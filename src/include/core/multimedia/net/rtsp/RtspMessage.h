#include <unordered_map>

#include <core/multimedia/net/media.h>
#include <core/multimedia/net/rtp/rtp.h>
#include <core/util/buffer/BufferReader.h>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

struct RtspMethod
{
  enum
  {
    NONE = 0,
    OPTIONS,
    DESCRIBE,
    SETUP,
    PLAY,
    TEARDOWN,
    GET_PARAMETER,
    RTCP,
    ANNOUNCE,
    RECORD,
  } method;

  static auto fromString(const std::string &method) -> RtspMethod {
    if (method == "OPTIONS") return {OPTIONS};
#define XX(x) else if (method == #x) return {x}
    XX(DESCRIBE);
    XX(PLAY);
    XX(TEARDOWN);
    XX(GET_PARAMETER);
    XX(RTCP);
    XX(ANNOUNCE);
    XX(RECORD);
#undef XX
    return {NONE};
  }

  auto toString() const -> std::string {
    switch (method)
    {
#define XX(x) \
  case x: return #x

      XX(OPTIONS);
      XX(DESCRIBE);
      XX(PLAY);
      XX(TEARDOWN);
      XX(GET_PARAMETER);
      XX(RTCP);
      XX(ANNOUNCE);
      XX(RECORD);

    default: return "NONE";
#undef XX
    }

    LY_UNREACHABLE();
  }
};


class RtspRequest
{
public:
  enum RtspRequestParseState
  {
    kParseRequestLine,
    kParseHeadersLine,
    // kParseBody,
    kGotAll,
  };

public:
  bool parse(BufferReader &buffer);
  auto isGotAll() const -> bool { return state_ == kGotAll; }
  void reset() {
    state_ = kParseRequestLine;
    request_line_param_.clear();
    header_line_param_.clear();
  }

  RtspMethod method() const { return method_; }
  uint32_t cseq() const;
  std::string rtspUrl() const;
  std::string rtspUrlSuffix() const;
  std::string ip() const;
	std::string authResponse() const;
  TransportMode transportMode() const { return transport_; }
  MediaChannelId channelId() const { return channel_id_; }
  uint8_t rtpChannel() const;
  uint8_t rtcpChannel() const;
  uint16_t rtpPort() const;
  uint16_t rtcpPort() const;

  int buildOptionRes(char *buf, int buf_size);
  int buildDescribeRes(char *buf, int buf_size, const char *sdp);
  int buildSetupMulticastRes(char *buf, int buf_size,
    const char *multicast_ip, uint16_t port, uint32_t session_id);
  int buildSetupTcpRes(char *buf, int buf_size, uint16_t rtp_chn,
    uint16_t rtcp_chn, uint32_t session_id);
  int buildSetupUdpRes(char *buf, int buf_size, uint16_t rtp_chn,
    uint16_t rtcp_chn, uint32_t session_id);
  int buildPlayRes(char *buf, int buf_size, const char *rtp_info, uint32_t session_id);
  int buildTeardownRes(char *buf, int buf_size, uint32_t session_id);
  int buildGetParameterRes(char *buf, int buf_size, uint32_t session_id);
  int buildNotFoundRes(char *buf, int buf_size);
  int buildServerErrorRes(char *buf, int buf_size);
  int buildUnsupportedRes(char *buf, int buf_size);
  int buildUnauthorizedRes(char *buf, int buf_size, const char *realm, const char *nonce);

private:
  bool parseRequestLine(std::string_view line);
  bool parseHeadersLine(std::string_view headers);
  bool parseCSeq(std::string_view message);
  bool parseAccept(std::string_view  message);
  bool parseTransport(std::string_view message);
  bool parseSessionId(std::string_view message);
  bool parseMediaChannel(std::string_view message);
  bool parseAuthorization(std::string_view message);

private:
  RtspMethod method_;
  MediaChannelId channel_id_;
  TransportMode transport_;
  std::string auth_response_;
  std::unordered_map<std::string, std::pair<std::string, uint32_t>>
    request_line_param_;
  std::unordered_map<std::string, std::pair<std::string, uint32_t>>
    header_line_param_;
  RtspRequestParseState state_{kParseRequestLine};
};

class RtspResponse
{
public:
  bool parse(BufferReader &buffer);

  RtspMethod method() const { return method_; }
  uint32_t cseq() const { return cseq_; }
  std::string session() const { return session_; }
  void setUserAgent(const char *user_agent) {
    user_agent_ = std::string(user_agent);
  }
  void setRtspUrl(const char *url) { rtsp_url_ = std::string(url); }

  int buildOptionReq(char *buf, int buf_size);
  int buildDescribeReq(char *buf, int buf_size);
  int buildAnnounceReq(char *buf, int buf_size, const char *sdp);
  int buildSetupTcpReq(char *buf, int buf_size, int track_id);
  int buildRecordReq(char *buf, int buf_size);

private:
  RtspMethod method_;
  uint32_t cseq_{0};
  std::string user_agent_;
  std::string rtsp_url_;
  std::string session_;
};


NAMESPACE_END(net)
LY_NAMESPACE_END
