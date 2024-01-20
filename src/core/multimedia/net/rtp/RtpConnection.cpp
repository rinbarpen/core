#include <arpa/inet.h>
#include <chrono>
#include <core/multimedia/net/rtp/RtpConnection.h>
#include <core/multimedia/net/rtsp/RtspConnection.h>
#include "core/multimedia/net/media.h"
#include "core/net/NetAddress.h"
#include "core/net/SocketUtil.h"
#include "core/util/time/Timestamp.h"
#include "core/util/time/time.h"
#include "fmt/core.h"

#include <memory>
#include <netinet/in.h>
#include <random>
#include <string>
#include <sys/socket.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

RtpConnection::RtpConnection(std::weak_ptr<TcpConnection> rtspConnection)
  : rtsp_connection_(rtspConnection) {
  std::random_device rd;

  for (int i = 0; i < kMaxMediaChannel; i++)
  {
    rtpfd_[i] = kInvalidSockfd;
    rtcpfd_[i] = kInvalidSockfd;
    // std::uninitialized_fill_n(&media_channel_info_[i],
    // sizeof(media_channel_info_[i]), 0);
    memset(&media_channel_info_[i], 0, sizeof(media_channel_info_[i]));

    media_channel_info_[i].rtp_header.version = RTP_VERSION;
    media_channel_info_[i].packet_seq = rd();
    media_channel_info_[i].rtp_header.seq = 0;  // htons(1);
    media_channel_info_[i].rtp_header.ts = ::htonl(rd());
    media_channel_info_[i].rtp_header.ssrc = ::htonl(rd());
  }

  auto conn = rtsp_connection_.lock();
  rtsp_address_ = conn->getAddress();
}

RtpConnection::~RtpConnection() {
  for (int i = 0; i < kMaxMediaChannel; i++)
  {
    if (rtpfd_[i] != kInvalidSockfd)
    {
      socket_api::close(rtpfd_[i]);
    }

    if (rtcpfd_[i] != kInvalidSockfd)
    {
      socket_api::close(rtcpfd_[i]);
    }
  }
}

int RtpConnection::getId() const {
  auto conn = rtsp_connection_.lock();
  if (!conn)
  {
    return -1;
  }

  auto rtspConn = std::dynamic_pointer_cast<RtspConnection *>(conn);
  return rtspConn->getId();
}

bool RtpConnection::setupRtpOverTcp(
  MediaChannelId channelId, uint16_t rtpChannel, uint16_t rtcpChannel) {
  auto conn = rtsp_connection_.lock();
  if (!conn)
  {
    return false;
  }

  media_channel_info_[channelId].rtp_channel = rtpChannel;
  media_channel_info_[channelId].rtcp_channel = rtcpChannel;
  rtpfd_[channelId] = conn->getSockfd();
  rtcpfd_[channelId] = conn->getSockfd();
  media_channel_info_[channelId].is_setup = true;
  transport_mode_ = RTP_OVER_TCP;

  return true;
}

bool RtpConnection::setupRtpOverUdp(
  MediaChannelId channelId, uint16_t rtpPort, uint16_t rtcpPort) {
  auto conn = rtsp_connection_.lock();
  if (!conn)
  {
    return false;
  }

  peer_addr_ = socket_api::getPeerAddr(conn->getSockfd());
  if (peer_addr_.isNull())
  {
    return false;
  }

  media_channel_info_[channelId].rtp_port = rtpPort;
  media_channel_info_[channelId].rtcp_port = rtcpPort;

  std::random_device rd;
  for (int n = 0; n <= 10; n++)
  {
    if (n == 10)
    {
      return false;
    }

    local_rtp_port_[channelId] = rd() & 0xFFFE;
    local_rtcp_port_[channelId] = local_rtp_port_[channelId] + 1;

    rtpfd_[channelId] =
      socket_api::socket_udp_bind("0.0.0.0", local_rtp_port_[channelId]);
    if (rtpfd_[channelId] == kInvalidSockfd)
    {
      continue;
    }

    rtcpfd_[channelId] =
      socket_api::socket_udp_bind("0.0.0.0", local_rtcp_port_[channelId]);
    if (rtcpfd_[channelId] == kInvalidSockfd)
    {
      socket_api::close(rtpfd_[channelId]);
      continue;
    }

    break;
  }

  // TODO: Replace magic number
  socket_api::setSendBufSize(rtpfd_[channelId], 50 * 1024);

  peer_rtp_addr_[channelId] = {
    peer_addr_.ip, media_channel_info_[channelId].rtp_port};
  peer_rtcp_sddr_[channelId] = {
    peer_addr_.ip, media_channel_info_[channelId].rtcp_port};

  transport_mode_ = RTP_OVER_UDP;
  media_channel_info_[channelId].is_setup = true;

  return true;
}

bool RtpConnection::setupRtpOverMulticast(MediaChannelId channelId, const NetAddress &address) {
  std::random_device rd;
  for (int n = 0; n <= 10; n++)
  {
    if (n == 10)
    {
      return false;
    }

    local_rtp_port_[channelId] = rd() & 0xFFFE;
    rtpfd_[channelId] = socket_api::socket_udp_bind("0.0.0.0", local_rtp_port_[channelId]);
    if (rtpfd_[channelId] < 0)
    {
      continue;
    }

    break;
  }

  media_channel_info_[channelId].rtp_port = address.port;

  // peer_rtp_addr_[channelId].sin_family = AF_INET;
  // peer_rtp_addr_[channelId].sin_addr.s_addr = inet_addr(ip.c_str());
  // peer_rtp_addr_[channelId].sin_port = htons(port);

  peer_rtp_addr_[channelId] = address;

  multicasting_ = true;
  transport_mode_ = RTP_OVER_MULTICAST;
  media_channel_info_[channelId].is_setup = true;
  return true;
}

void RtpConnection::play() {
  for (int i = 0; i < kMaxMediaChannel; i++)
  {
    if (media_channel_info_[i].is_setup)
    {
      media_channel_info_[i].is_play = true;
    }
  }
}

void RtpConnection::record() {
  for (int i = 0; i < kMaxMediaChannel; i++)
  {
    if (media_channel_info_[i].is_setup)
    {
      media_channel_info_[i].is_record = true;
      media_channel_info_[i].is_play = true;
    }
  }
}

void RtpConnection::teardown() {
  if (running_)
  {
    running_ = false;
    for (int i = 0; i < kMaxMediaChannel; i++)
    {
      media_channel_info_[i].is_play = false;
      media_channel_info_[i].is_record = false;
    }
  }
}

auto RtpConnection::getMulticastIp(MediaChannelId channelId) const -> std::string {
  return peer_rtp_addr_[channelId].ip;
}

std::string RtpConnection::getRtpInfo(const std::string &rtspUrl) {
  char buf[2048] = {0};
  snprintf(buf, 1024, "RTP-Info: ");

  int num_channel = 0;

  auto ts = Timestamp<T_steady_clock>::now<std::chrono::milliseconds>();
  for (int i = 0; i < kMaxMediaChannel; i++)
  {
    uint32_t rtpTime =
      (uint32_t) (ts * media_channel_info_[i].clock_rate / 1000);
    if (media_channel_info_[i].is_setup)
    {
      if (num_channel != 0)
      {
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), ",");
      }

      snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
        "url=%s/track%d;seq=0;rtptime=%u", rtspUrl.c_str(), i, rtpTime);
      ++num_channel;
    }
  }

  return std::string(buf);
}

void RtpConnection::setFrameType(uint8_t frame_type) {
  frame_type_ = frame_type;
  if (!has_key_frame_ && (frame_type == 0 || frame_type == VIDEO_FRAME_I))
  {
    has_key_frame_ = true;
  }
}

void RtpConnection::setRtpHeader(MediaChannelId channelId, RtpPacket pkt) {
  if ((media_channel_info_[channelId].is_play
      || media_channel_info_[channelId].is_record)
      && has_key_frame_)
  {
    media_channel_info_[channelId].rtp_header.marker = pkt.last;
    media_channel_info_[channelId].rtp_header.ts = htonl(pkt.timestamp);
    media_channel_info_[channelId].rtp_header.seq =
      htons(media_channel_info_[channelId].packet_seq++);
    memcpy(pkt.data.get() + 4, &media_channel_info_[channelId].rtp_header,
      RTP_HEADER_SIZE);
    // pkt.data.fill((const char*)&media_channel_info_[channelId].rtp_header,
    //   RTP_HEADER_SIZE, 4);
  }
}

bool RtpConnection::sendRtpPacket(MediaChannelId channelId, RtpPacket pkt) {
  if (!running_)
  {
    return false;
  }

  auto conn = rtsp_connection_.lock();
  if (!conn)
  {
    return false;
  }
  auto rtsp_conn = std::dynamic_pointer_cast<RtspConnection>(conn);
  bool ret =
    rtsp_conn->task_scheduler_->addTriggerEvent([this, channelId, pkt] {
      this->setFrameType(pkt.type);
      this->setRtpHeader(channelId, pkt);
      if ((media_channel_info_[channelId].is_play
          || media_channel_info_[channelId].is_record)
          && has_key_frame_)
      {
        if (transport_mode_ == RTP_OVER_TCP)
        {
          this->sendRtpOverTcp(channelId, pkt);
        }
        else
        {
          this->sendRtpOverUdp(channelId, pkt);
        }

        // NOTE: Maybe remove
        media_channel_info_[channelId].octet_count  += pkt.data.size();
        media_channel_info_[channelId].packet_count += 1;
      }
    });

  return ret;
}

bool RtpConnection::sendRtpOverTcp(MediaChannelId channelId, RtpPacket pkt) {
  auto conn = rtsp_connection_.lock();
  if (!conn)
  {
    return -1;
  }

  auto rtpPktPtr = pkt.data.get();
  rtpPktPtr[0] = '$';
  rtpPktPtr[1] = (char) media_channel_info_[channelId].rtp_channel;
  rtpPktPtr[2] = (char) (((pkt.data.size() - 4) & 0xFF00) >> 8);
  rtpPktPtr[3] = (char) ((pkt.data.size() - 4) & 0xFF);

  conn->send(rtpPktPtr, pkt.data.size());
  return pkt.data.size();
}

int RtpConnection::sendRtpOverUdp(MediaChannelId channelId, RtpPacket pkt) {
  int ret = socket_api::sendto(rtpfd_[channelId], pkt.data.get() + 4,
    pkt.data.size() - 4, 0, peer_rtp_addr_[channelId].ip.c_str(), peer_rtp_addr_[channelId].port);

  if (ret < 0)
  {
    this->teardown();
    return -1;
  }

  return ret;
}


NAMESPACE_END(net)
LY_NAMESPACE_END
