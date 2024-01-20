#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include "core/multimedia/net/media.h"
#include "core/multimedia/net/rtp/rtp.h"
#include "core/net/NetAddress.h"
#include "core/net/platform.h"
#include "core/net/tcp/TcpConnection.h"
#include "core/util/marcos.h"

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class RtspConnection;

class RtpConnection
{
public:
  RtpConnection(std::weak_ptr<TcpConnection> rtspConnection);
  virtual ~RtpConnection();

  void setClockRate(MediaChannelId channelId, uint32_t clockRate) {
    media_channel_info_[channelId].clock_rate = clockRate;
  }
  void setPayloadType(MediaChannelId channelId, uint32_t payload) {
    media_channel_info_[channelId].rtp_header.payload = payload;
  }

  bool setupRtpOverTcp(
    MediaChannelId channelId, uint16_t rtpChannel, uint16_t rtcpChannel);
  bool setupRtpOverUdp(
    MediaChannelId channelId, uint16_t rtpPort, uint16_t rtcpPort);
  bool setupRtpOverMulticast(
    MediaChannelId channelId, const NetAddress &address);

  uint32_t getRtpSessionId() const { return reinterpret_cast<uintptr_t>(this); }
  uint16_t getRtpPort(MediaChannelId channelId) const {
    return local_rtp_port_[channelId];
  }

  uint16_t getRtcpPort(MediaChannelId channelId) const {
    return local_rtcp_port_[channelId];
  }

  sockfd_t getRtpSockfd(MediaChannelId channelId) const {
    return rtpfd_[channelId];
  }

  sockfd_t getRtcpSockfd(MediaChannelId channelId) const {
    return rtcpfd_[channelId];
  }

  auto getRtspAddress() const -> NetAddress { return rtsp_address_; }

  bool isMulticast() const { return multicasting_; }
  bool isSetup(MediaChannelId channelId) const {
    return media_channel_info_[channelId].is_setup;
  }

  std::string getMulticastIp(MediaChannelId channelId) const;

  void play();
  void record();
  void teardown();

  std::string getRtpInfo(const std::string &rtspUrl);
  int sendRtpPacket(MediaChannelId channelId, RtpPacket pkt);

  bool isRunning() const { return running_; }

  int getId() const { return rtsp_connection_->getId(); }

  bool hasKeyFrame() const { return has_key_frame_; }

private:
  friend class RtspConnection;
  friend class MediaSession;

  void setFrameType(uint8_t frameType = 0);
  void setRtpHeader(MediaChannelId channelId, RtpPacket pkt);
  int sendRtpOverTcp(MediaChannelId channelId, RtpPacket pkt);
  int sendRtpOverUdp(MediaChannelId channelId, RtpPacket pkt);

  std::weak_ptr<TcpConnection> rtsp_connection_;
  // rtsp_ip and rtsp_port
  NetAddress rtsp_address_;

  TransportMode transport_mode_;
  bool multicasting_{false};

  bool running_{true};
  bool has_key_frame_{false};

  uint8_t frame_type_ = 0;

  uint16_t local_rtp_port_[kMaxMediaChannel];
  uint16_t local_rtcp_port_[kMaxMediaChannel];
  sockfd_t rtpfd_[kMaxMediaChannel];
  sockfd_t rtcpfd_[kMaxMediaChannel];

  // struct sockaddr_in peer_addr_;
  // struct sockaddr_in peer_rtp_addr_[kMaxMediaChannel];
  // struct sockaddr_in peer_rtcp_sddr_[kMaxMediaChannel];
  NetAddress peer_addr_;
  NetAddress peer_rtp_addr_[kMaxMediaChannel];
  NetAddress peer_rtcp_sddr_[kMaxMediaChannel];
  MediaChannelInfo media_channel_info_[kMaxMediaChannel];
};

NAMESPACE_END(net)
LY_NAMESPACE_END
