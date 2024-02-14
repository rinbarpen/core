#include <cstring>
#include <string_view>

#include <core/util/marcos.h>
#include <core/util/ContainerUtil.h>
#include <core/util/buffer/BufferReader.h>
#include <core/multimedia/net/rtsp/RtspMessage.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
auto RtspRequest::parse(BufferReader &buffer) -> bool {
  if (*buffer.peek() == '$')
  {
    method_.method = RtspMethod::RTCP;
    return true;
  }

  bool r = false;
  for (;;)
  {
    if (state_ == kParseRequestLine)
    {
      int distance = buffer.findFirst("\r\n");
      if (distance >= 0)
      {
        r = parseRequestLine(std::string_view(buffer.peek(), distance));
        buffer.advance(distance + 2);
      }

      if (state_ != kParseHeadersLine)
      {
        break;
      }
    }
    else if (state_ == kParseHeadersLine)
    {
      int distance = buffer.findLast("\r\n");
      if (distance >= 0)
      {
        r = this->parseHeadersLine(std::string_view(buffer.peek(), distance));
        buffer.advance(distance + 2);
      }
      break;
    }
    else if (state_ == kGotAll)
    {
      buffer.advance(buffer.readableBytes());
      return true;
    }
  }

  return r;
}

bool RtspRequest::parseRequestLine(std::string_view line) {
  char method[64] = {0};
  char url[512] = {0};
  char version[64] = {0};

  if (std::sscanf(line.data(), "%s %s %s", method, url, version) != 3)
  {
    return false;
  }

  if (0 == std::strcmp("OPTIONS", method))
  {
    method_.method = RtspMethod::OPTIONS;
  }
#define XX(x) \
  else if (0 == std::strcmp(#x, method)) method_.method = RtspMethod::x
  XX(DESCRIBE);
  XX(SETUP);
  XX(PLAY);
  XX(TEARDOWN);
  XX(GET_PARAMETER);
#undef XX
  else {
    // Unsupported method
    method_.method = RtspMethod::NONE;
    return false;
  }

  if (std::strncmp(url, "rtsp://", 7) != 0)
  {
    return false;
  }

  // parse url
  uint16_t port = 0;
  char ip[64] = {0};
  char suffix[64] = {0};

  if (3 == std::sscanf(url + 7, "%[^:]:%hu/%s", ip, &port, suffix))
  {}
  // match rtsp://{ip}/{suffix}
  else if (2 == std::sscanf(url, "%[^/]/%s", ip, suffix))
  {
    port = 554;
  }
  // unsupported
  // // match rtsp://[username:password@]{ip[:port]}/{suffix}
  // else if (5 == std::sscanf(url + 7, "%[^:]:%[^@]@%[^:]:%hu/%s", username,
  // password, ip, &port, suffix)) {
  // }
  // // match rtsp://[username:password@]{ip}/{suffix}
  // else if (4 == std::sscanf(url + 7, "%[^:]:%[^@]@%[^/]/%s", username,
  // password, ip, suffix)) {
  //   port = 554;
  // }
  else
  {
    return false;
  }

  request_line_param_.emplace("url", std::make_pair(std::string(url), 0));
  request_line_param_.emplace("url_ip", std::make_pair(std::string(ip), 0));
  request_line_param_.emplace("url_port", std::make_pair("", (uint32_t) port));
  request_line_param_.emplace(
    "url_suffix", std::make_pair(std::string(suffix), 0));
  request_line_param_.emplace(
    "version", std::make_pair(std::string(version), 0));
  request_line_param_.emplace("method", std::make_pair(std::string(method), 0));

  state_ = kParseHeadersLine;
  return true;
}

bool RtspRequest::parseHeadersLine(std::string_view headers) {
  if (!this->parseCSeq(headers))
  {
    if (!container_util::contain(header_line_param_, "cseq"))
    {
      return false;
    }
  }

  if (method_.method == RtspMethod::DESCRIBE
      || method_.method == RtspMethod::SETUP
      || method_.method == RtspMethod::PLAY)
  {
    this->parseAuthorization(headers);
  }

  if (method_.method == RtspMethod::OPTIONS)
  {
    state_ = kGotAll;
    return true;
  }

  if (method_.method == RtspMethod::DESCRIBE)
  {
    if (this->parseAccept(headers))
    {
      state_ = kGotAll;
    }
    return true;
  }

  if (method_.method == RtspMethod::SETUP)
  {
    if (this->parseTransport(headers))
    {
      this->parseMediaChannel(headers);
      state_ = kGotAll;
    }

    return true;
  }

  if (method_.method == RtspMethod::PLAY)
  {
    if (this->parseSessionId(headers))
    {
      state_ = kGotAll;
    }
    return true;
  }

  if (method_.method == RtspMethod::TEARDOWN)
  {
    state_ = kGotAll;
    return true;
  }

  if (method_.method == RtspMethod::GET_PARAMETER)
  {
    state_ = kGotAll;
    return true;
  }

  return true;
}

bool RtspRequest::parseCSeq(std::string_view message) {
  if (std::size_t pos = message.find("CSeq"); pos != std::string::npos)
  {
    uint32_t cseq = 0;
    std::sscanf(message.data() + pos, "%*[^:]: %u", &cseq);
    header_line_param_.emplace("cseq", std::make_pair("", cseq));
    return true;
  }

  return false;
}

bool RtspRequest::parseAccept(std::string_view message) {
  if ((message.rfind("Accept") == std::string::npos)
      || (message.rfind("sdp") == std::string::npos))
  {
    return false;
  }

  return true;
}

bool RtspRequest::parseTransport(std::string_view message) {
  if (std::size_t pos = message.find("Transport"); pos != std::string::npos)
  {
    if ((pos = message.find("RTP/AVP/TCP")) != std::string::npos)
    {
      transport_ = RTP_OVER_TCP;
      uint16_t rtpChannel = 0, rtcpChannel = 0;
      if (2 != std::sscanf(message.data() + pos, "%*[^;];%*[^;];%*[^=]=%hu-%hu",
            &rtpChannel, &rtcpChannel))
      {
        return false;
      }
      header_line_param_.emplace("rtp_channel", std::make_pair("", rtpChannel));
      header_line_param_.emplace("rtcp_channel", std::make_pair("", rtcpChannel));
    }
    else if (pos = message.find("RTP/AVP");
             pos != std::string::npos)
    {
      uint16_t rtp_port = 0, rtcpPort = 0;
      if (((message.find("unicast", pos)) != std::string::npos))
      {
        transport_ = RTP_OVER_UDP;
        if (2 != std::sscanf(message.data() + pos, "%*[^;];%*[^;];%*[^=]=%hu-%hu",
              &rtp_port, &rtcpPort))
        {
          return false;
        }
      }
      else if ((message.find("multicast", pos)) != std::string::npos)
      {
        transport_ = RTP_OVER_MULTICAST;
      }
      else
      {
        return false;
      }

      header_line_param_.emplace("rtp_port", std::make_pair("", rtp_port));
      header_line_param_.emplace("rtcp_port", std::make_pair("", rtcpPort));
    }
    else
    {
      return false;
    }

    return true;
  }

  return false;
}

bool RtspRequest::parseSessionId(std::string_view message) {
  if (std::size_t pos = message.find("Session"); pos != std::string::npos)
  {
    uint32_t session_id = 0;
    if (1 != std::sscanf(message.data() + pos, "%*[^:]: %u", &session_id))
    {
      return false;
    }
    return true;
  }

  return false;
}

bool RtspRequest::parseMediaChannel(std::string_view message) {
  channel_id_ = channel_0;

  if (auto iter = request_line_param_.find("url");
      iter != request_line_param_.end())
  {
    if (std::size_t pos = iter->second.first.find("track1");
        pos != std::string::npos)
    {
      channel_id_ = channel_1;
    }
  }

  return true;
}

bool RtspRequest::parseAuthorization(std::string_view message) {
  if (std::size_t pos = message.find("Authorization"); pos != std::string::npos)
  {
    if (pos = message.find("response="); pos != std::string::npos)
    {
      auth_response_ = message.substr(pos + 10, 32);
      if (auth_response_.size() == 32)
      {
        return true;
      }
    }
  }

  auth_response_.clear();
  return false;
}

uint32_t RtspRequest::cseq() const {
  uint32_t cseq = 0;
  auto iter = header_line_param_.find("cseq");
  if (iter != header_line_param_.end())
  {
    cseq = iter->second.second;
  }

  return cseq;
}

std::string RtspRequest::ip() const {
  auto iter = request_line_param_.find("url_ip");
  if (iter != request_line_param_.end())
  {
    return iter->second.first;
  }

  return "";
}

std::string RtspRequest::rtspUrl() const {
  auto iter = request_line_param_.find("url");
  if (iter != request_line_param_.end())
  {
    return iter->second.first;
  }

  return "";
}

std::string RtspRequest::rtspUrlSuffix() const {
  auto iter = request_line_param_.find("url_suffix");
  if (iter != request_line_param_.end())
  {
    return iter->second.first;
  }

  return "";
}

std::string RtspRequest::authResponse() const {
  return auth_response_;
}

uint8_t RtspRequest::rtpChannel() const {
  auto iter = header_line_param_.find("rtp_channel");
  if (iter != header_line_param_.end())
  {
    return iter->second.second;
  }

  return 0;
}

uint8_t RtspRequest::rtcpChannel() const {
  auto iter = header_line_param_.find("rtcp_channel");
  if (iter != header_line_param_.end())
  {
    return iter->second.second;
  }

  return 0;
}

uint16_t RtspRequest::rtpPort() const {
  auto iter = header_line_param_.find("rtp_port");
  if (iter != header_line_param_.end())
  {
    return iter->second.second;
  }

  return 0;
}

uint16_t RtspRequest::rtcpPort() const {
  auto iter = header_line_param_.find("rtcp_port");
  if (iter != header_line_param_.end())
  {
    return iter->second.second;
  }

  return 0;
}

int RtspRequest::buildOptionRes(char *buf, int buf_size)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: %u\r\n"
			"Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY\r\n"
			"\r\n",
			this->cseq());

	return (int)strlen(buf);
}
int RtspRequest::buildDescribeRes(char *buf, int buf_size, const char *sdp)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: %u\r\n"
			"Content-Length: %d\r\n"
			"Content-Type: application/sdp\r\n"
			"\r\n"
			"%s",
			this->cseq(),
			(int)strlen(sdp),
			sdp);

	return (int)strlen(buf);
}
int RtspRequest::buildSetupMulticastRes(char *buf, int buf_size, const char *multicast_ip, uint16_t port, uint32_t session_id)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: %u\r\n"
			"Transport: RTP/AVP;multicast;destination=%s;source=%s;port=%u-0;ttl=255\r\n"
			"Session: %u\r\n"
			"\r\n",
			this->cseq(),
			multicast_ip,
			this->ip().c_str(),
			port,
			session_id);

	return (int)strlen(buf);
}
int RtspRequest::buildSetupTcpRes(char *buf, int buf_size, uint16_t rtp_chn, uint16_t rtcp_chn, uint32_t session_id)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: %u\r\n"
			"Transport: RTP/AVP;unicast;client_port=%hu-%hu;server_port=%hu-%hu\r\n"
			"Session: %u\r\n"
			"\r\n",
			this->cseq(),
			this->rtpPort(),
			this->rtcpPort(),
			rtp_chn,
			rtcp_chn,
			session_id);

	return (int)strlen(buf);
}
int RtspRequest::buildSetupUdpRes(char *buf, int buf_size, uint16_t rtp_chn, uint16_t rtcp_chn, uint32_t session_id)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: %u\r\n"
			"Transport: RTP/AVP/TCP;unicast;interleaved=%d-%d\r\n"
			"Session: %u\r\n"
			"\r\n",
			this->cseq(),
			rtp_chn, rtcp_chn,
			session_id);

	return (int)strlen(buf);
}
int RtspRequest::buildPlayRes(char *buf, int buf_size, const char *rtp_info, uint32_t session_id)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: %d\r\n"
			"Range: npt=0.000-\r\n"
			"Session: %u; timeout=60\r\n",
			this->cseq(),
			session_id);

	if (rtp_info != nullptr) {
		snprintf(buf + strlen(buf), buf_size - strlen(buf), "%s\r\n", rtp_info);
	}

	snprintf(buf + strlen(buf), buf_size - strlen(buf), "\r\n");
	return (int)strlen(buf);
}
int RtspRequest::buildTeardownRes(char *buf, int buf_size, uint32_t session_id)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: %d\r\n"
			"Session: %u\r\n"
			"\r\n",
			this->cseq(),
			session_id);

	return (int)strlen(buf);
}
int RtspRequest::buildGetParameterRes(char *buf, int buf_size, uint32_t session_id)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: %d\r\n"
			"Session: %u\r\n"
			"\r\n",
			this->cseq(),
			session_id);

	return (int)strlen(buf);
}
int RtspRequest::buildNotFoundRes(char *buf, int buf_size)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RTSP/1.0 404 Stream Not Found\r\n"
			"CSeq: %u\r\n"
			"\r\n",
			this->cseq());

	return (int)strlen(buf);
}
int RtspRequest::buildServerErrorRes(char *buf, int buf_size)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RTSP/1.0 500 Internal Server Error\r\n"
			"CSeq: %u\r\n"
			"\r\n",
			this->cseq());

	return (int)strlen(buf);
}
int RtspRequest::buildUnsupportedRes(char *buf, int buf_size)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RTSP/1.0 461 Unsupported transport\r\n"
			"CSeq: %d\r\n"
			"\r\n",
			this->cseq());

	return (int)strlen(buf);
}
int RtspRequest::buildUnauthorizedRes(char *buf, int buf_size, const char *realm, const char *nonce)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RTSP/1.0 401 Unauthorized\r\n"
			"CSeq: %d\r\n"
			"WWW-Authenticate: Digest realm=\"%s\", nonce=\"%s\"\r\n"
			"\r\n",
			this->cseq(),
			realm,
			nonce);

	return (int)strlen(buf);
}


// RtspResponse
bool RtspResponse::parse(BufferReader &buffer)
{
	if (buffer.findLast("\r\n\r\n") > 0) {
		if (buffer.findFirst("OK") > 0) {
			return false;
		}

    // get session_id
		int distance = buffer.findFirst("Session");
		if (distance > 0) {
			char session_id[50] = {0};
			if (1 == std::sscanf(buffer.peek() + distance, "%*[^:]: %s", session_id))
				session_ = session_id;
		}

		cseq_++;
		buffer.advanceTo("\r\n\r\n");
	}

	return true;
}
int RtspResponse::buildOptionReq(char *buf, int buf_size)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"OPTIONS %s RTSP/1.0\r\n"
			"CSeq: %u\r\n"
			"User-Agent: %s\r\n"
			"\r\n",
			rtsp_url_.c_str(),
			this->cseq() + 1,
			user_agent_.c_str());

	method_.method = RtspMethod::OPTIONS;
	return (int)strlen(buf);
}
int RtspResponse::buildDescribeReq(char *buf, int buf_size)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"DESCRIBE %s RTSP/1.0\r\n"
			"CSeq: %u\r\n"
			"Accept: application/sdp\r\n"
			"User-Agent: %s\r\n"
			"\r\n",
			rtsp_url_.c_str(),
			this->cseq() + 1,
			user_agent_.c_str());

	method_.method = RtspMethod::DESCRIBE;
	return (int)strlen(buf);
}
int RtspResponse::buildAnnounceReq(char *buf, int buf_size, const char *sdp)
{
  memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"ANNOUNCE %s RTSP/1.0\r\n"
			"Content-Type: application/sdp\r\n"
			"CSeq: %u\r\n"
			"User-Agent: %s\r\n"
			"Session: %s\r\n"
			"Content-Length: %d\r\n"
			"\r\n"
			"%s",
			rtsp_url_.c_str(),
			this->cseq() + 1,
			user_agent_.c_str(),
			this->session().c_str(),
			(int)strlen(sdp),
			sdp);

	method_.method = RtspMethod::ANNOUNCE;
	return (int)strlen(buf);
}
int RtspResponse::buildSetupTcpReq(char *buf, int buf_size, int track_id)
{
	int interleaved[2] = { 0, 1 };
	if (track_id == 1) {
		interleaved[0] = 2;
		interleaved[1] = 3;
	}

	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"SETUP %s/track%d RTSP/1.0\r\n"
			"Transport: RTP/AVP/TCP;unicast;mode=record;interleaved=%d-%d\r\n"
			"CSeq: %u\r\n"
			"User-Agent: %s\r\n"
			"Session: %s\r\n"
			"\r\n",
			rtsp_url_.c_str(),
			track_id,
			interleaved[0],
			interleaved[1],
			this->cseq() + 1,
			user_agent_.c_str(),
			this->session().c_str());

	method_.method = RtspMethod::SETUP;
	return (int)strlen(buf);
}
int RtspResponse::buildRecordReq(char *buf, int buf_size)
{
	memset(buf, 0, buf_size);
	snprintf(buf, buf_size,
			"RECORD %s RTSP/1.0\r\n"
			"Range: npt=0.000-\r\n"
			"CSeq: %u\r\n"
			"User-Agent: %s\r\n"
			"Session: %s\r\n"
			"\r\n",
			rtsp_url_.c_str(),
			this->cseq() + 1,
			user_agent_.c_str(),
			this->session().c_str());

	method_.method = RtspMethod::RECORD;
	return (int)strlen(buf);
}

NAMESPACE_END(net)
LY_NAMESPACE_END
