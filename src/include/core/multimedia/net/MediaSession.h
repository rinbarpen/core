#pragma once
#include <functional>
#include <map>
#include <string>

#include <core/util/marcos.h>
#include <core/net/SocketUtil.h>
#include <core/util/buffer/RingBuffer.h>
#include <core/util/Generic.h>
#include "core/multimedia/net/media.h"
#include <core/multimedia/net/MediaSource.h>
#include <core/multimedia/net/rtp/RtpConnection.h>
#include <core/net/multicast/MulticastAddress.h>


LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class RtpConnection;
class MediaSession
{
public:
  SHARED_PTR_USING(MediaSession, ptr);

  using NotifyConnectedCallback = std::function<void(MediaSessionId, std::string peer_ip, uint16_t peer_port)>;
  using NotifyDisconnectedCallback = std::function<void(MediaSessionId, std::string peer_ip, uint16_t peer_port)>;

  static MediaSession* create(std::string suffix = "live");
  virtual ~MediaSession();

  bool addSource(MediaChannelId channel_id, MediaSource *source);
  bool removeSource(MediaChannelId channel_id);

  bool startMulticast();

  void addNotifyConnectedCallback(const NotifyConnectedCallback &callback);
  void addNotifyDisconnectedCallback(const NotifyDisconnectedCallback &callback);

  std::string getRtspUrlSuffix() const { return suffix_; }
  void setRtspUrlSuffix(std::string suffix) { suffix_ = suffix; }

  std::string getSdpMessage(const std::string &ip, std::string session_name = "");

  MediaSource *getMediaSource(MediaSessionId channel_id);

  bool handleFrame(MediaChannelId channel_id, SimAVFrame frame);

  bool addClient(sockfd_t rtspfd, std::shared_ptr<RtpConnection> rtpConn);
  bool removeClient(sockfd_t rtspfd);
  uint32_t getClientCount() const { return clients_.size(); }

  MediaSessionId getMediaSessionId() const { return session_id_; }

  bool isMulticasting() const { return multicasting_; }
  std::string getMulticastIp() const { return multicast_address_.ip(); }
  uint16_t getMulticastPort(MediaChannelId channel_id) const;
private:
  MediaSession(std::string url_suffix);

private:
  MediaSessionId session_id_{0};
  std::string suffix_;
  std::string sdp_;

  std::vector<std::unique_ptr<MediaSource>> media_sources_;
  std::vector<RingBuffer<SimAVFrame>> buffers_;

  std::vector<NotifyConnectedCallback> notify_connected_callbacks_;
  std::vector<NotifyDisconnectedCallback> notify_disconnected_callbacks_;

  std::map<sockfd_t, std::weak_ptr<RtpConnection>> clients_;
  std::atomic_bool has_new_client_{false};

  bool multicasting_{false};
  MulticastAddress multicast_address_;
  // uint16_t multicast_port_[kMaxMediaChannel];
  // std::string multicast_ip_;

  Mutex::type mutex_;
  Mutex::type map_mutex_;

  static inline std::atomic<ForwardSequenceGeneric<MediaSessionId>> generator_;
  // static inline std::atomic<MediaSessionId> next_session_id_ = 1;
};
NAMESPACE_END(net)
LY_NAMESPACE_END