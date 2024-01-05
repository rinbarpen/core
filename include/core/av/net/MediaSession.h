#pragma once
#include <functional>
#include <map>
#include <string>

#include "marcos.h"
#include "net/net.h"
#include "media/MediaSource.h"
#include "proto/rtp/RtpConnection.h"
#include "util/buffer/RingBuffer.h"

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

  void addNotifyConnectedCallback(const NotifyConnectedCallback &fn);
  void addNotifyDisconnectedCallback(const NotifyDisconnectedCallback &fn);

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
  std::string getMulticastIp() const { return multicast_ip_; }
  uint16_t getMulticastPort(MediaChannelId channel_id) const;
private:
  MediaSession(std::string url_suffix);

private:
  MediaSessionId session_id_{0};
  std::string suffix_;
  std::string sdp_;

  std::vector<std::unique_ptr<MediaSource>> media_sources_;
  std::vector<RingBuffer<SimAVFrame>> buffers_;

  std::vector<NotifyConnectedCallback> notify_connected_cbs_;
  std::vector<NotifyDisconnectedCallback> notify_disconnected_cbs_;

  std::map<sockfd_t, std::weak_ptr<RtpConnection>> clients_;
  std::atomic_bool has_new_client_;

  bool multicasting_{false};
  uint16_t multicast_port_[kMaxMediaChannel];
  std::string multicast_ip_;

  Mutex::type mutex_;
  Mutex::type map_mutex_;

  static inline std::atomic<MediaSessionId> next_session_id_ = 1;
};

