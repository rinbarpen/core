#pragma once

#include <core/util/marcos.h>
#include <core/net/EventLoop.h>
#include <core/net/tcp/TcpConnection.h>
#include <core/multimedia/net/rtmp/amf.h>
#include <core/multimedia/net/rtmp/rtmp.h>
#include <core/multimedia/net/rtmp/RtmpHandshake.h>
#include <core/multimedia/net/rtmp/RtmpChunk.h>
#include "core/util/buffer/BufferReader.h"

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class RtmpServer;
class RtmpPublisher;
class RtmpClient;

class RtmpConnection : public TcpConnection
{
  friend class RtmpSession;
  friend class RtmpServer;
  friend class RtmpPublisher;
  friend class RtmpClient;

public:
  using PlayCallback = std::function<void(
    uint8_t *payload, uint32_t length, uint8_t codec_id, uint32_t timestamp)>;

  enum class ConnectionStatus
  {
    HANDSHAKE,
    START_CONNECT,
    START_CREATE_STREAM,
    START_DELETE_STREAM,
    START_PLAY,
    START_PUBLISH,
  };
  enum class ConnectionMode
  {
    RTMP_SERVER,
    RTMP_PUBLISHER,
    RTMP_CLIENT,
  };

  RtmpConnection(std::shared_ptr<RtmpServer> server, TaskScheduler *scheduler,
    sockfd_t sockfd);
  RtmpConnection(std::shared_ptr<RtmpPublisher> publisher,
    TaskScheduler *scheduler, sockfd_t sockfd);
  RtmpConnection(std::shared_ptr<RtmpClient> client, TaskScheduler *scheduler,
    sockfd_t sockfd);
  virtual ~RtmpConnection();

  std::string getStreamPath() const { return stream_path_; }
  std::string getStreamName() const { return stream_name_; }
  std::string getApp() const { return app_; }
  AmfObjectMap getMetaData() const { return meta_data_; }

  bool isPlayer() const {
    return connection_status_ == ConnectionStatus::START_PLAY;
  }
  bool isPublisher() const {
    return connection_status_ == ConnectionStatus::START_PUBLISH;
  }
  bool isPlaying() const { return is_playing_; }
  bool isPublishing() const { return is_publishing_; }

  std::string getStatus() const {
    if (status_ == "") {
      return "unknown error";
    }
    return status_;
  }

private:
  RtmpConnection(TaskScheduler *scheduler, sockfd_t sockfd, Rtmp *rtmp);

  bool handleRead(BufferReader &buffer);
  void handleClose();

  bool handleChunk(BufferReader &buffer);
  bool handleMessage(RtmpMessage &rtmp_msg);
  bool handleInvoke(RtmpMessage &rtmp_msg);
  bool handleNotify(RtmpMessage &rtmp_msg);
  bool handleVideo(RtmpMessage &rtmp_msg);
  bool handleAudio(RtmpMessage &rtmp_msg);
  bool handleConnect();
  bool handleCreateStream();
  bool handlePublish();
  bool handlePlay();
  bool handlePlay2();
  bool handleDeleteStream();
  bool handleResult(RtmpMessage &rtmp_msg);
  bool handleOnStatus(RtmpMessage &rtmp_msg);

  bool handshake();
  bool connect();
  bool publish();
  bool play();
  bool createStream();
  bool deleteStream();

  void setPeerBandwidth();
  void sendAcknowledgement();
  void setChunkSize();
  void setPlayCallBack(PlayCallback callback);

  bool sendInvokeMessage(
    uint32_t csid, std::shared_ptr<char> payload, uint32_t payload_size);
  bool sendNotifyMessage(
    uint32_t csid, std::shared_ptr<char> payload, uint32_t payload_size);
  virtual bool sendMetaData(AmfObjectMap metaData);
  virtual bool sendMediaData(uint8_t type, uint64_t timestamp,
    std::shared_ptr<char> payload, uint32_t payload_size);
  virtual bool sendVideoData(
    uint64_t timestamp, std::shared_ptr<char> payload, uint32_t payload_size);
  virtual bool sendAudioData(
    uint64_t timestamp, std::shared_ptr<char> payload, uint32_t payload_size);
  void sendRtmpChunks(uint32_t csid, RtmpMessage &rtmp_msg);
  bool isKeyFrame(std::shared_ptr<char> payload, uint32_t payload_size);

private:
  std::weak_ptr<RtmpServer> rtmp_server_;
  std::weak_ptr<RtmpPublisher> rtmp_publisher_;
  std::weak_ptr<RtmpClient> rtmp_client_;

  std::shared_ptr<RtmpHandshake> handshake_;
  std::shared_ptr<RtmpChunk> rtmp_chunk_;
  ConnectionMode connection_mode_;
  ConnectionStatus connection_status_;

  TaskScheduler *task_scheduler_;
  std::shared_ptr<FdChannel> channel_;

  uint32_t peer_bandwidth_ = 5000000;
  uint32_t acknowledgement_size_ = 5000000;
  uint32_t max_chunk_size_ = 128;
  uint32_t max_gop_cache_len_ = 0;
  uint32_t stream_id_ = 0;
  uint32_t number_ = 0;
  std::string app_;
  std::string stream_name_;
  std::string stream_path_;
  std::string status_;

  AmfObjectMap meta_data_;
  AmfDecoder amf_decoder_;
  AmfEncoder amf_encoder_;

  bool is_playing_ = false;
  bool is_publishing_ = false;
  bool has_key_frame_ = false;
  std::shared_ptr<char> avc_sequence_header_;
  std::shared_ptr<char> aac_sequence_header_;
  uint32_t avc_sequence_header_size_ = 0;
  uint32_t aac_sequence_header_size_ = 0;
  PlayCallback play_callback_;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
