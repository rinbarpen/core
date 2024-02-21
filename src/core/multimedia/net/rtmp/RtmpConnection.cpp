#include <core/multimedia/net/rtmp/RtmpConnection.h>
#include <core/multimedia/net/rtmp/RtmpServer.h>
#include <core/multimedia/net/rtmp/RtmpClient.h>
#include <core/multimedia/net/rtmp/RtmpPublisher.h>
#include <core/util/buffer/BufferWriter.h>
#include <core/util/buffer/BufferReader.h>
#include <core/util/logger/Logger.h>
#include <core/util/timer/TimerTask.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
static auto g_rtmp_logger = GET_LOGGER("multimedia.rtmp");

RtmpConnection::RtmpConnection(std::shared_ptr<RtmpServer> rtmp_server,
  TaskScheduler *task_scheduler, sockfd_t sockfd)
  : RtmpConnection(task_scheduler, sockfd, rtmp_server.get()) {
  handshake_.reset(new RtmpHandshake(RtmpHandshake::HANDSHAKE_C0C1));
  rtmp_server_ = rtmp_server;
  connection_mode_ = ConnectionMode::RTMP_SERVER;
}

RtmpConnection::RtmpConnection(std::shared_ptr<RtmpPublisher> rtmp_publisher,
  TaskScheduler *task_scheduler, sockfd_t sockfd)
  : RtmpConnection(task_scheduler, sockfd, rtmp_publisher.get()) {
  handshake_.reset(new RtmpHandshake(RtmpHandshake::HANDSHAKE_S0S1S2));
  rtmp_publisher_ = rtmp_publisher;
  connection_mode_ = ConnectionMode::RTMP_PUBLISHER;
}

RtmpConnection::RtmpConnection(std::shared_ptr<RtmpClient> rtmp_client,
  TaskScheduler *task_scheduler, sockfd_t sockfd)
  : RtmpConnection(task_scheduler, sockfd, rtmp_client.get()) {
  handshake_.reset(new RtmpHandshake(RtmpHandshake::HANDSHAKE_S0S1S2));
  rtmp_client_ = rtmp_client;
  connection_mode_ = ConnectionMode::RTMP_CLIENT;
}

RtmpConnection::RtmpConnection(
  TaskScheduler *task_scheduler, sockfd_t sockfd, Rtmp *rtmp)
  : TcpConnection(task_scheduler, sockfd)
  , task_scheduler_(task_scheduler)
  , channel_(new FdChannel(sockfd))
  , rtmp_chunk_(new RtmpChunk{})
  , connection_status_(ConnectionStatus::HANDSHAKE) {
  peer_bandwidth_ = rtmp->getPeerBandwidth();
  acknowledgement_size_ = rtmp->getAcknowledgementSize();
  max_gop_cache_len_ = rtmp->getGopCacheLen();
  max_chunk_size_ = rtmp->getChunkSize();
  stream_path_ = rtmp->getStreamPath();
  stream_name_ = rtmp->getStreamName();
  app_ = rtmp->getApp();

  this->setReadCallback(
    [this](std::shared_ptr<TcpConnection> conn, BufferReader &buffer) {
      return this->onRead(buffer);
    });

  this->setCloseCallback(
    [this](std::shared_ptr<TcpConnection> conn) { this->onClose(); });
}

RtmpConnection::~RtmpConnection() {}

bool RtmpConnection::onRead(BufferReader &buffer) {
  bool r{true};

  if (handshake_->isCompleted()) {
    r = handleChunk(buffer);
    return r;
  }

  std::shared_ptr<char> res(new char[4096], std::default_delete<char[]>());
  int res_size = handshake_->parse(buffer, res.get(), 4096);
  if (res_size <= 0) {
    r = false;
  }

  if (res_size > 0) {
    this->send(res.get(), res_size);
  }

  if (handshake_->isCompleted()) {
    if (buffer.readableBytes() > 0) {
      r = handleChunk(buffer);
    }

    if (connection_mode_ == ConnectionMode::RTMP_PUBLISHER
        || connection_mode_ == ConnectionMode::RTMP_CLIENT) {
      this->setChunkSize();
      this->connect();
    }
  }

  return r;
}

void RtmpConnection::onClose() {
  if (connection_mode_ == ConnectionMode::RTMP_SERVER) {
    this->handleDeleteStream();
  }
  else if (connection_mode_ == ConnectionMode::RTMP_PUBLISHER) {
    this->deleteStream();
  }
}

bool RtmpConnection::handleChunk(BufferReader &buffer) {
  int r{-1};

  do {
    RtmpMessage rtmp_msg;
    r = rtmp_chunk_->parse(buffer, rtmp_msg);
    if (r < 0) {
      return false;
    }

    if (rtmp_msg.isCompleted()) {
      if (!handleMessage(rtmp_msg)) {
        return false;
      }
    }

    if (r == 0) {
      break;
    }
  } while (buffer.readableBytes() > 0);

  return true;
}

bool RtmpConnection::handleMessage(RtmpMessage &rtmp_msg) {
  bool r{true};
  switch (rtmp_msg.type_id) {
  case RTMP_VIDEO: r = handleVideo(rtmp_msg); break;
  case RTMP_AUDIO: r = handleAudio(rtmp_msg); break;
  case RTMP_INVOKE: r = handleInvoke(rtmp_msg); break;
  case RTMP_NOTIFY: r = handleNotify(rtmp_msg); break;
  case RTMP_FLEX_MESSAGE:
    ILOG_INFO(g_rtmp_logger) << "unsupported rtmp flex message.";
    r = false;
    break;
  case RTMP_SET_CHUNK_SIZE:
    rtmp_chunk_->setInChunkSize(readU32Forward(rtmp_msg.payload.get()));
    break;
  case RTMP_BANDWIDTH_SIZE: break;
  case RTMP_FLASH_VIDEO:
    ILOG_INFO(g_rtmp_logger) << "unsupported rtmp flash video.";
    r = false;
    break;
  case RTMP_ACK: break;
  case RTMP_ACK_SIZE: break;
  case RTMP_USER_EVENT: break;
  default:
    // TODO: print %d
    ILOG_INFO(g_rtmp_logger) << "unknown message type: " << rtmp_msg.type_id;
    break;
  }

  if (!r) {
    // TODO: print %x
    ILOG_INFO_FMT(g_rtmp_logger, "rtmp_msg.type_id: {:x}", rtmp_msg.type_id);
  }

  return r;
}

bool RtmpConnection::handleInvoke(RtmpMessage &rtmp_msg) {
  bool r = true;
  amf_decoder_.reset();

  int bytes_used = amf_decoder_.decode(
    (const char *) rtmp_msg.payload.get(), rtmp_msg.length, 1);
  if (bytes_used < 0) {
    return false;
  }

  std::string method = amf_decoder_.getString();
  // LOG_INFO("[Method] %s\n", method.c_str());

  if (connection_mode_ == ConnectionMode::RTMP_PUBLISHER
   || connection_mode_ == ConnectionMode::RTMP_CLIENT) {
    bytes_used += amf_decoder_.decode(
      rtmp_msg.payload.get() + bytes_used, rtmp_msg.length - bytes_used);
    if (method == "_result") {
      r = handleResult(rtmp_msg);
    }
    else if (method == "onStatus") {
      r = handleOnStatus(rtmp_msg);
    }
  }
  else if (connection_mode_ == ConnectionMode::RTMP_SERVER) {
    if (rtmp_msg.stream_id == 0) {
      bytes_used += amf_decoder_.decode(
        rtmp_msg.payload.get() + bytes_used, rtmp_msg.length - bytes_used);
      if (method == "connect") {
        r = handleConnect();
      }
      else if (method == "createStream") {
        r = handleCreateStream();
      }
    }
    else if (rtmp_msg.stream_id == stream_id_) {
      bytes_used +=
        amf_decoder_.decode((const char *) rtmp_msg.payload.get() + bytes_used,
          rtmp_msg.length - bytes_used, 3);
      stream_name_ = amf_decoder_.getString();
      stream_path_ = "/" + app_ + "/" + stream_name_;

      if ((int) rtmp_msg.length > bytes_used) {
        bytes_used += amf_decoder_.decode(
          (const char *) rtmp_msg.payload.get() + bytes_used,
          rtmp_msg.length - bytes_used);
      }

      if (method == "publish") {
        r = handlePublish();
      }
      else if (method == "play") {
        r = handlePlay();
      }
      else if (method == "play2") {
        r = handlePlay2();
      }
      else if (method == "DeleteStream") {
        r = handleDeleteStream();
      }
      else if (method == "releaseStream") {}
    }
  }

  return r;
}

bool RtmpConnection::handleNotify(RtmpMessage &rtmp_msg) {
  amf_decoder_.reset();
  int bytes_used = amf_decoder_.decode(
    (const char *) rtmp_msg.payload.get(), rtmp_msg.length, 1);
  if (bytes_used < 0) {
    return false;
  }

  if (amf_decoder_.getString() == "@setDataFrame") {
    amf_decoder_.reset();
    bytes_used =
      amf_decoder_.decode((const char *) rtmp_msg.payload.get() + bytes_used,
        rtmp_msg.length - bytes_used, 1);
    if (bytes_used < 0) {
      return false;
    }

    if (amf_decoder_.getString() == "onMetaData") {
      amf_decoder_.decode((const char *) rtmp_msg.payload.get() + bytes_used,
        rtmp_msg.length - bytes_used);
      meta_data_ = amf_decoder_.getObjectMap();

      auto server = rtmp_server_.lock();
      if (!server) {
        return false;
      }

      auto session = server->getSession(stream_path_);
      if (session) {
        session->setMetaData(meta_data_);
        session->sendMetaData(meta_data_);
      }
    }
  }

  return true;
}

bool RtmpConnection::handleVideo(RtmpMessage &rtmp_msg) {
  uint8_t type = RTMP_VIDEO;
  uint8_t *payload = (uint8_t *) rtmp_msg.payload.get();
  uint32_t length = rtmp_msg.length;
  uint8_t frame_type = (payload[0] >> 4) & 0x0f;
  uint8_t codec_id = payload[0] & 0x0f;

  if (connection_mode_ == ConnectionMode::RTMP_CLIENT) {
    if (is_playing_ && connection_status_ == ConnectionStatus::START_PLAY) {
      if (play_callback_) {
        play_callback_(payload, length, codec_id, (uint32_t) rtmp_msg.long_timestamp);
      }
    }
  }
  else if (connection_mode_ == ConnectionMode::RTMP_SERVER) {
    auto server = rtmp_server_.lock();
    if (!server) {
      return false;
    }

    auto session = server->getSession(stream_path_);
    if (session == nullptr) {
      return false;
    }

    if (frame_type == 1 && codec_id == RTMP_CODEC_ID_H264) {
      if (payload[1] == 0) {
        avc_sequence_header_size_ = length;
        avc_sequence_header_.reset(
          new char[length], std::default_delete<char[]>());
        memcpy(avc_sequence_header_.get(), rtmp_msg.payload.get(), length);
        session->setAvcSequenceHeader(
          avc_sequence_header_, avc_sequence_header_size_);
        type = RTMP_AVC_SEQUENCE_HEADER;
      }
    }

    session->sendMediaData(
      type, rtmp_msg.long_timestamp, rtmp_msg.payload, rtmp_msg.length);
  }

  return true;
}

bool RtmpConnection::handleAudio(RtmpMessage &rtmp_msg) {
  uint8_t type = RTMP_AUDIO;
  uint8_t *payload = (uint8_t *) rtmp_msg.payload.get();
  uint32_t length = rtmp_msg.length;
  uint8_t sound_format = (payload[0] >> 4) & 0x0f;
  // uint8_t sound_size = (payload[0] >> 1) & 0x01;
  // uint8_t sound_rate = (payload[0] >> 2) & 0x03;
  uint8_t codec_id = payload[0] & 0x0f;

  if (connection_mode_ == ConnectionMode::RTMP_CLIENT) {
    if (connection_status_ == ConnectionStatus::START_PLAY
     && is_playing_) {
      if (play_callback_) {
        play_callback_(payload, length, codec_id, (uint32_t) rtmp_msg.long_timestamp);
      }
    }
  }
  else {
    auto server = rtmp_server_.lock();
    if (!server) {
      return false;
    }

    auto session = server->getSession(stream_path_);
    if (session == nullptr) {
      return false;
    }

    if (sound_format == RTMP_CODEC_ID_AAC && payload[1] == 0) {
      aac_sequence_header_size_ = rtmp_msg.length;
      aac_sequence_header_.reset(
        new char[rtmp_msg.length], std::default_delete<char[]>());
      memcpy(
        aac_sequence_header_.get(), rtmp_msg.payload.get(), rtmp_msg.length);
      session->setAacSequenceHeader(
        aac_sequence_header_, aac_sequence_header_size_);
      type = RTMP_AAC_SEQUENCE_HEADER;
    }

    session->sendMediaData(
      type, rtmp_msg.long_timestamp, rtmp_msg.payload, rtmp_msg.length);
  }

  return true;
}


bool RtmpConnection::handshake() {
  uint32_t req_size = 1 + 1536;  // COC1
  std::shared_ptr<char> req(new char[req_size], std::default_delete<char[]>());
  handshake_->buildC0C1(req.get(), req_size);
  this->send(req.get(), req_size);
  return true;
}

bool RtmpConnection::connect() {
  AmfObjectMap objectMap;
  amf_encoder_.reset();

  amf_encoder_.encodeString("connect", 7);
  amf_encoder_.encodeNumber((double) (++number_));
  objectMap["app"] = app_;
  objectMap["type"] = "nonprivate";

  if (connection_mode_ == ConnectionMode::RTMP_PUBLISHER) {
    auto publisher = rtmp_publisher_.lock();
    if (!publisher) {
      return false;
    }
    objectMap["swfUrl"] = publisher->getSwfUrl();
    objectMap["tcUrl"] = publisher->getTcUrl();
  }
  else if (connection_mode_ == ConnectionMode::RTMP_CLIENT) {
    auto client = rtmp_client_.lock();
    if (!client) {
      return false;
    }
    objectMap["swfUrl"] = client->getSwfUrl();
    objectMap["tcUrl"] = client->getTcUrl();
  }

  amf_encoder_.encodeObjectMap(objectMap);
  connection_status_ = ConnectionStatus::START_CONNECT;
  sendInvokeMessage(
    RTMP_CHUNK_INVOKE_ID, amf_encoder_.data(), amf_encoder_.size());
  return true;
}

bool RtmpConnection::createStream() {
  AmfObjectMap objectMap;
  amf_encoder_.reset();

  amf_encoder_.encodeString("createStream", 12);
  amf_encoder_.encodeNumber((double) (++number_));
  amf_encoder_.encodeObjectMap(objectMap);

  connection_status_ = ConnectionStatus::START_CREATE_STREAM;
  sendInvokeMessage(
    RTMP_CHUNK_INVOKE_ID, amf_encoder_.data(), amf_encoder_.size());
  return true;
}

bool RtmpConnection::publish() {
  AmfObjectMap objectMap;
  amf_encoder_.reset();

  amf_encoder_.encodeString("publish", 7);
  amf_encoder_.encodeNumber((double) (++number_));
  amf_encoder_.encodeObjectMap(objectMap);
  amf_encoder_.encodeString(stream_name_.c_str(), (int) stream_name_.size());

  connection_status_ = ConnectionStatus::START_PUBLISH;
  sendInvokeMessage(
    RTMP_CHUNK_INVOKE_ID, amf_encoder_.data(), amf_encoder_.size());
  return true;
}

bool RtmpConnection::play() {
  AmfObjectMap objectMap;
  amf_encoder_.reset();

  amf_encoder_.encodeString("play", 4);
  amf_encoder_.encodeNumber((double) (++number_));
  amf_encoder_.encodeObjectMap(objectMap);
  amf_encoder_.encodeString(stream_name_.c_str(), (int) stream_name_.size());

  connection_status_ = ConnectionStatus::START_PLAY;
  sendInvokeMessage(
    RTMP_CHUNK_INVOKE_ID, amf_encoder_.data(), amf_encoder_.size());
  return true;
}

bool RtmpConnection::deleteStream() {
  AmfObjectMap objectMap;
  amf_encoder_.reset();

  amf_encoder_.encodeString("DeleteStream", 12);
  amf_encoder_.encodeNumber((double) (++number_));
  amf_encoder_.encodeObjectMap(objectMap);
  amf_encoder_.encodeNumber(stream_id_);

  connection_status_ = ConnectionStatus::START_DELETE_STREAM;
  sendInvokeMessage(
    RTMP_CHUNK_INVOKE_ID, amf_encoder_.data(), amf_encoder_.size());
  return true;
}

bool RtmpConnection::handleConnect() {
  if (!amf_decoder_.hasObject("app")) {
    return false;
  }

  AmfObject obj = amf_decoder_.getObject("app");
  app_ = obj.string();
  if (app_ == "") {
    return false;
  }

  sendAcknowledgement();
  setPeerBandwidth();
  setChunkSize();

  AmfObjectMap objectMap;
  amf_encoder_.reset();
  amf_encoder_.encodeString("_result", 7);
  amf_encoder_.encodeNumber(amf_decoder_.getNumber());

  objectMap["fmsVer"] = AmfObject(std::string("FMS/4,5,0,297"));
  objectMap["capabilities"] = AmfObject(255.0);
  objectMap["mode"] = AmfObject(1.0);
  amf_encoder_.encodeObjectMap(objectMap);
  objectMap.clear();
  objectMap["level"] = AmfObject(std::string("status"));
  objectMap["code"] = AmfObject(std::string("NetConnection.Connect.Success"));
  objectMap["description"] = AmfObject(std::string("Connection succeeded."));
  objectMap["objectEncoding"] = AmfObject(0.0);
  amf_encoder_.encodeObjectMap(objectMap);

  sendInvokeMessage(
    RTMP_CHUNK_INVOKE_ID, amf_encoder_.data(), amf_encoder_.size());
  return true;
}

bool RtmpConnection::handleCreateStream() {
  int stream_id = rtmp_chunk_->getStreamId();

  AmfObjectMap objectMap;
  amf_encoder_.reset();
  amf_encoder_.encodeString("_result", 7);
  amf_encoder_.encodeNumber(amf_decoder_.getNumber());
  amf_encoder_.encodeObjectMap(objectMap);
  amf_encoder_.encodeNumber(stream_id);

  sendInvokeMessage(
    RTMP_CHUNK_INVOKE_ID, amf_encoder_.data(), amf_encoder_.size());
  stream_id_ = stream_id;
  return true;
}

bool RtmpConnection::handlePublish() {
  ILOG_INFO_FMT(g_rtmp_logger, "[Publish] app: {}, stream name: {}, stream path: {}",
    app_, stream_name_, stream_path_);

  auto server = rtmp_server_.lock();
  if (!server) {
    return false;
  }

  AmfObjectMap objectMap;
  amf_encoder_.reset();
  amf_encoder_.encodeString("onStatus", 8);
  amf_encoder_.encodeNumber(0);
  amf_encoder_.encodeObjectMap(objectMap);

  bool is_error = false;

  if (server->hasPublisher(stream_path_)) {
    is_error = true;
    objectMap["level"] = AmfObject(std::string("error"));
    objectMap["code"] = AmfObject(std::string("NetStream.Publish.BadName"));
    objectMap["description"] =
      AmfObject(std::string("Stream already publishing."));
  }
  else if (connection_status_ == ConnectionStatus::START_PUBLISH) {
    is_error = true;
    objectMap["level"] = AmfObject(std::string("error"));
    objectMap["code"] = AmfObject(std::string("NetStream.Publish.BadConnection"));
    objectMap["description"] =
      AmfObject(std::string("Connection already publishing."));
  }
  /* else if(0)  {
      // 认证处理
  } */
  else {
    objectMap["level"] = AmfObject(std::string("status"));
    objectMap["code"] = AmfObject(std::string("NetStream.Publish.Start"));
    objectMap["description"] = AmfObject(std::string("Start publising."));
    server->addSession(stream_path_);
  }

  amf_encoder_.encodeObjectMap(objectMap);
  sendInvokeMessage(
    RTMP_CHUNK_INVOKE_ID, amf_encoder_.data(), amf_encoder_.size());

  if (is_error) {
    // Close ?
  }
  else {
    connection_status_ = ConnectionStatus::START_PUBLISH;
    is_publishing_ = true;
  }

  auto session = server->getSession(stream_path_);
  if (session) {
    session->setGopCache(max_gop_cache_len_);
    session->addRtmpClient(
      std::dynamic_pointer_cast<RtmpConnection>(shared_from_this()));
  }

  return true;
}

bool RtmpConnection::handlePlay() {
  ILOG_INFO_FMT(g_rtmp_logger, "[Play] app: {}, stream name: {}, stream path: {}",
    app_, stream_name_, stream_path_);

  auto server = rtmp_server_.lock();
  if (!server) {
    return false;
  }

  AmfObjectMap objectMap;
  amf_encoder_.reset();
  amf_encoder_.encodeString("onStatus", 8);
  amf_encoder_.encodeNumber(0);
  amf_encoder_.encodeObjectMap(objectMap);
  objectMap["level"] = AmfObject(std::string("status"));
  objectMap["code"] = AmfObject(std::string("NetStream.Play.Reset"));
  objectMap["description"] =
    AmfObject(std::string("Resetting and playing stream."));
  amf_encoder_.encodeObjectMap(objectMap);
  if (!sendInvokeMessage(
        RTMP_CHUNK_INVOKE_ID, amf_encoder_.data(), amf_encoder_.size())) {
    return false;
  }

  objectMap.clear();
  amf_encoder_.reset();
  amf_encoder_.encodeString("onStatus", 8);
  amf_encoder_.encodeNumber(0);
  amf_encoder_.encodeObjectMap(objectMap);
  objectMap["level"] = AmfObject(std::string("status"));
  objectMap["code"] = AmfObject(std::string("NetStream.Play.Start"));
  objectMap["description"] = AmfObject(std::string("Started playing."));
  amf_encoder_.encodeObjectMap(objectMap);
  if (!sendInvokeMessage(
        RTMP_CHUNK_INVOKE_ID, amf_encoder_.data(), amf_encoder_.size())) {
    return false;
  }

  amf_encoder_.reset();
  amf_encoder_.encodeString("|RtmpSampleAccess", 17);
  amf_encoder_.encodeBoolean(true);
  amf_encoder_.encodeBoolean(true);
  if (!this->sendNotifyMessage(
        RTMP_CHUNK_DATA_ID, amf_encoder_.data(), amf_encoder_.size())) {
    return false;
  }

  connection_status_ = ConnectionStatus::START_PLAY;

  auto session = server->getSession(stream_path_);
  if (session) {
    session->addRtmpClient(
      std::dynamic_pointer_cast<RtmpConnection>(shared_from_this()));
  }

  return true;
}

bool RtmpConnection::handlePlay2() {
  handlePlay();
  ILOG_INFO_FMT(g_rtmp_logger, "[Play2] stream path: {}", stream_path_);
  return false;
}

bool RtmpConnection::handleDeleteStream() {
  auto server = rtmp_server_.lock();
  if (!server) {
    return false;
  }

  if (stream_path_ != "") {
    auto session = server->getSession(stream_path_);
    if (session != nullptr) {
      auto conn = std::dynamic_pointer_cast<RtmpConnection>(shared_from_this());
      task_scheduler_->addTimer(TimerTask{
        [session, conn] {
          session->removeRtmpClient(conn);
          return false;
        }, 1ms});
    }

    is_playing_ = false;
    is_publishing_ = false;
    has_key_frame_ = false;
    rtmp_chunk_->clear();
  }

  return true;
}

bool RtmpConnection::handleResult(RtmpMessage &rtmp_msg) {
  bool r{false};

  if (connection_status_ == ConnectionStatus::START_CONNECT) {
    if (amf_decoder_.hasObject("code")) {
      AmfObject obj = amf_decoder_.getObject("code");
      if (obj.string() == "NetConnection.Connect.Success") {
        createStream();
        r = true;
      }
    }
  }
  else if (connection_status_ == ConnectionStatus::START_CREATE_STREAM) {
    if (amf_decoder_.getNumber() > 0) {
      stream_id_ = (uint32_t) amf_decoder_.getNumber();
      if (connection_mode_ == ConnectionMode::RTMP_PUBLISHER) {
        this->publish();
      }
      else if (connection_mode_ == ConnectionMode::RTMP_CLIENT) {
        this->play();
      }

      r = true;
    }
  }

  return r;
}

bool RtmpConnection::handleOnStatus(RtmpMessage &rtmp_msg) {
  bool r{true};

  if (connection_status_ == ConnectionStatus::START_PUBLISH
   || connection_status_ == ConnectionStatus::START_PLAY) {
    if (amf_decoder_.hasObject("code")) {
      AmfObject obj = amf_decoder_.getObject("code");
      status_ = obj.string();
      if (connection_mode_ == ConnectionMode::RTMP_PUBLISHER) {
        if (status_ == "NetStream.Publish.Start") {
          is_publishing_ = true;
        }
        else if (status_ == "NetStream.publish.Unauthorized"
              || status_ == "NetStream.Publish.BadConnection" /* "Connection already publishing" */
              || status_ == "NetStream.Publish.BadName")  /* Stream already publishing */
        {
          r = false;
        }
      }
      else if (connection_mode_ == ConnectionMode::RTMP_CLIENT) {
        if (/*obj.tring() == "NetStream.Play.Reset" || */
          status_ == "NetStream.Play.Start") {
          is_playing_ = true;
        }
        else if (status_ == "NetStream.play.Unauthorized"
              || status_ == "NetStream.Play.UnpublishNotify" /* "stream is now unpublished." */
              || status_ == "NetStream.Play.BadConnection") /* "Connection already playing" */
        {
          r = false;
        }
      }
    }
  }

  if (connection_status_ == ConnectionStatus::START_DELETE_STREAM) {
    if (amf_decoder_.hasObject("code")) {
      AmfObject obj = amf_decoder_.getObject("code");
      if (obj.string() != "NetStream.Unpublish.Success") {
        r = false;
      }
    }
  }

  return r;
}

bool RtmpConnection::sendMetaData(AmfObjectMap metaData) {
  if (!this->isRunning()) {
    return false;
  }
  if (metaData.empty()) {
    return false;
  }

  amf_encoder_.reset();
  amf_encoder_.encodeString("onMetaData", 10);
  amf_encoder_.encodeECMA(metaData);
  if (!this->sendNotifyMessage(
        RTMP_CHUNK_DATA_ID, amf_encoder_.data(), amf_encoder_.size())) {
    return false;
  }

  return true;
}

void RtmpConnection::setPeerBandwidth() {
  std::shared_ptr<char> data(new char[5], std::default_delete<char[]>());
  writeU32Forward(data.get(), peer_bandwidth_);

  data.get()[4] = 2;
  RtmpMessage rtmp_msg;
  rtmp_msg.type_id = RTMP_BANDWIDTH_SIZE;
  rtmp_msg.payload = data;
  rtmp_msg.length = 5;
  sendRtmpChunks(RTMP_CHUNK_CONTROL_ID, rtmp_msg);
}

void RtmpConnection::sendAcknowledgement() {
  std::shared_ptr<char> data(new char[4], std::default_delete<char[]>());
  writeU32Forward(data.get(), acknowledgement_size_);

  RtmpMessage rtmp_msg;
  rtmp_msg.type_id = RTMP_ACK_SIZE;
  rtmp_msg.payload = data;
  rtmp_msg.length = 4;
  sendRtmpChunks(RTMP_CHUNK_CONTROL_ID, rtmp_msg);
}

void RtmpConnection::setChunkSize() {
  rtmp_chunk_->setOutChunkSize(max_chunk_size_);
  std::shared_ptr<char> data(new char[4], std::default_delete<char[]>());
  writeU32Forward(data.get(), max_chunk_size_);

  RtmpMessage rtmp_msg;
  rtmp_msg.type_id = RTMP_SET_CHUNK_SIZE;
  rtmp_msg.payload = data;
  rtmp_msg.length = 4;
  sendRtmpChunks(RTMP_CHUNK_CONTROL_ID, rtmp_msg);
}

void RtmpConnection::setPlayCallBack(PlayCallback callback) {
  play_callback_ = callback;
}

bool RtmpConnection::sendInvokeMessage(
  uint32_t csid, std::shared_ptr<char> payload, uint32_t payload_size) {
  if (!this->isRunning()) {
    return false;
  }

  RtmpMessage rtmp_msg;
  rtmp_msg.type_id = RTMP_INVOKE;
  rtmp_msg.timestamp = 0;
  rtmp_msg.stream_id = stream_id_;
  rtmp_msg.payload = payload;
  rtmp_msg.length = payload_size;
  sendRtmpChunks(csid, rtmp_msg);
  return true;
}

bool RtmpConnection::sendNotifyMessage(
  uint32_t csid, std::shared_ptr<char> payload, uint32_t payload_size) {
  if (!this->isRunning()) {
    return false;
  }

  RtmpMessage rtmp_msg;
  rtmp_msg.type_id = RTMP_NOTIFY;
  rtmp_msg.timestamp = 0;
  rtmp_msg.stream_id = stream_id_;
  rtmp_msg.payload = payload;
  rtmp_msg.length = payload_size;
  sendRtmpChunks(csid, rtmp_msg);
  return true;
}

bool RtmpConnection::isKeyFrame(
  std::shared_ptr<char> payload, uint32_t payload_size) {
  uint8_t frame_type = (payload.get()[0] >> 4) & 0x0F;
  uint8_t codec_id = payload.get()[0] & 0x0F;
  return (frame_type == 1 && codec_id == RTMP_CODEC_ID_H264);
}

bool RtmpConnection::sendMediaData(uint8_t type, uint64_t timestamp,
  std::shared_ptr<char> payload, uint32_t payload_size) {
  if (!this->isRunning()) {
    return false;
  }
  if (payload_size == 0) {
    return false;
  }

  is_playing_ = true;

  if (type == RTMP_AVC_SEQUENCE_HEADER) {
    avc_sequence_header_ = payload;
    avc_sequence_header_size_ = payload_size;
  }
  else if (type == RTMP_AAC_SEQUENCE_HEADER) {
    aac_sequence_header_ = payload;
    aac_sequence_header_size_ = payload_size;
  }

  auto conn = std::dynamic_pointer_cast<RtmpConnection>(shared_from_this());
  task_scheduler_->addTriggerEvent(
    [conn, type, timestamp, payload, payload_size] {
      if (!conn->has_key_frame_ && conn->avc_sequence_header_size_ > 0
          && (type != RTMP_AVC_SEQUENCE_HEADER)
          && (type != RTMP_AAC_SEQUENCE_HEADER)) {
        if (conn->isKeyFrame(payload, payload_size)) {
          conn->has_key_frame_ = true;
        }
        else {
          return;
        }
      }

      RtmpMessage rtmp_msg;
      rtmp_msg.long_timestamp = timestamp;
      rtmp_msg.stream_id = conn->stream_id_;
      rtmp_msg.payload = payload;
      rtmp_msg.length = payload_size;

      if (type == RTMP_VIDEO || type == RTMP_AVC_SEQUENCE_HEADER) {
        rtmp_msg.type_id = RTMP_VIDEO;
        conn->sendRtmpChunks(RTMP_CHUNK_VIDEO_ID, rtmp_msg);
      }
      else if (type == RTMP_AUDIO || type == RTMP_AAC_SEQUENCE_HEADER) {
        rtmp_msg.type_id = RTMP_AUDIO;
        conn->sendRtmpChunks(RTMP_CHUNK_AUDIO_ID, rtmp_msg);
      }
    });

  return true;
}

bool RtmpConnection::sendVideoData(
  uint64_t timestamp, std::shared_ptr<char> payload, uint32_t payload_size) {
  if (payload_size == 0) {
    return false;
  }

  auto conn = std::dynamic_pointer_cast<RtmpConnection>(shared_from_this());
  task_scheduler_->addTriggerEvent([conn, timestamp, payload, payload_size] {
    RtmpMessage rtmp_msg;
    rtmp_msg.type_id = RTMP_VIDEO;
    rtmp_msg.long_timestamp = timestamp;
    rtmp_msg.stream_id = conn->stream_id_;
    rtmp_msg.payload = payload;
    rtmp_msg.length = payload_size;
    conn->sendRtmpChunks(RTMP_CHUNK_VIDEO_ID, rtmp_msg);
  });

  return true;
}

bool RtmpConnection::sendAudioData(
  uint64_t timestamp, std::shared_ptr<char> payload, uint32_t payload_size) {
  if (payload_size == 0) {
    return false;
  }

  auto conn = std::dynamic_pointer_cast<RtmpConnection>(shared_from_this());
  task_scheduler_->addTriggerEvent([conn, timestamp, payload, payload_size] {
    RtmpMessage rtmp_msg;
    rtmp_msg.type_id = RTMP_AUDIO;
    rtmp_msg.long_timestamp = timestamp;
    rtmp_msg.stream_id = conn->stream_id_;
    rtmp_msg.payload = payload;
    rtmp_msg.length = payload_size;
    conn->sendRtmpChunks(RTMP_CHUNK_VIDEO_ID, rtmp_msg);
  });
  return true;
}

void RtmpConnection::sendRtmpChunks(uint32_t csid, RtmpMessage &rtmp_msg) {
  uint32_t capacity =
    rtmp_msg.length + rtmp_msg.length / max_chunk_size_ * 5 + 1024;
  std::shared_ptr<char> buffer(
    new char[capacity], std::default_delete<char[]>());

  int size = rtmp_chunk_->createChunk(csid, rtmp_msg, buffer.get(), capacity);
  if (size > 0) {
    this->send(buffer.get(), size);
  }
}
NAMESPACE_END(net)
LY_NAMESPACE_END
