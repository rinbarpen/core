#include <chrono>
#include <core/multimedia/net/rtmp/RtmpPublisher.h>
#include <memory>
#include "core/net/platform.h"
#include "core/net/tcp/TcpSocket.h"
#include "core/util/logger/Logger.h"

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
static auto g_rtmp_logger = GET_LOGGER("multimedia.rtmp");

RtmpPublisher::RtmpPublisher(EventLoop *event_loop) : event_loop_(event_loop) {}

RtmpPublisher::~RtmpPublisher() {}

std::shared_ptr<RtmpPublisher> RtmpPublisher::create(EventLoop *event_loop) {
  std::shared_ptr<RtmpPublisher> publisher(new RtmpPublisher(event_loop));
  return publisher;
}

bool RtmpPublisher::setMediaInfo(MediaInfo media_info) {
  Mutex::lock locker(mutex_);

  media_info_ = media_info;

  if (media_info_.audio_codec_id == RTMP_CODEC_ID_AAC) {
    if (media_info_.audio_specific_config_size > 0) {
      aac_sequence_header_size_ = media_info_.audio_specific_config_size + 2;
      aac_sequence_header_.reset(new char[aac_sequence_header_size_]);
      uint8_t *data = (uint8_t *) aac_sequence_header_.get();
      uint8_t sound_rate = 3;   // for aac awlays 3
      uint8_t soundz_size = 1;  // 0:8bit , 1:16bit
      uint8_t sound_type = 1;   // for aac awlays 1

      // audio tag data
      audio_tag_ = data[0] =
        (((RTMP_CODEC_ID_AAC & 0xf) << 4) | ((sound_rate & 0x3) << 2)
          | ((soundz_size & 0x1) << 1) | (sound_type & 0x1));

      // aac packet type
      data[1] = 0;  // 0: aac sequence header, 1: aac raw data

      memcpy(data + 2, media_info_.audio_specific_config.get(),
        media_info_.audio_specific_config_size);

      // 11 90 -- 48000 2, 12 10 -- 44100 2
      uint32_t samplingFrequencyIndex =
        ((data[2] & 0x07) << 1) | ((data[3] & 0x80) >> 7);
      uint32_t channel = (data[3] & 0x78) >> 3;
      media_info_.audio_channel = channel;
      media_info_.audio_sample_rate =
        kSamplingFrequency[samplingFrequencyIndex];
    }
    else {
      media_info_.audio_codec_id = 0;
    }
  }

  if (media_info_.video_codec_id == RTMP_CODEC_ID_H264) {
    if (media_info_.sps_size > 0 && media_info_.pps > 0) {
      avc_sequence_header_.reset(new char[4096], std::default_delete<char[]>());
      uint8_t *data = (uint8_t *) avc_sequence_header_.get();
      uint32_t index = 0;

      data[index++] = 0x17;  // 1:keyframe  7:avc
      data[index++] = 0;     // 0: avc sequence header

      data[index++] = 0;
      data[index++] = 0;
      data[index++] = 0;

      // AVCDecoderConfigurationRecord
      data[index++] = 0x01;                      // configurationVersion
      data[index++] = media_info_.sps.get()[1];  // AVCProfileIndication
      data[index++] = media_info_.sps.get()[2];  // profile_compatibility
      data[index++] = media_info_.sps.get()[3];  // AVCLevelIndication
      data[index++] = 0xff;                      // lengthSizeMinusOne

      // sps nums
      data[index++] = 0xE1;  //&0x1f

      // sps data length
      data[index++] = media_info_.sps_size >> 8;
      data[index++] = media_info_.sps_size & 0xff;
      // sps data
      memcpy(data + index, media_info_.sps.get(), media_info_.sps_size);
      index += media_info_.sps_size;

      // pps nums
      data[index++] = 0x01;  //&0x1f
                             // pps data length
      data[index++] = media_info_.pps_size >> 8;
      data[index++] = media_info_.pps_size & 0xff;
      // sps data
      memcpy(data + index, media_info_.pps.get(), media_info_.pps_size);
      index += media_info_.pps_size;

      avc_sequence_header_size_ = index;
    }
    else {
      media_info_.video_codec_id = 0;
    }
  }

  return true;
}
bool RtmpPublisher::openUrl(
  std::string url, std::chrono::milliseconds msec, std::string &status) {
  return this->openUrl(url, msec.count(), status);
}
bool RtmpPublisher::openUrl(std::string url, int msec, std::string &status) {
  Mutex::lock locker(mutex_);

  static Timestamp timestamp;
  int timeout = msec;
  if (timeout <= 0) {
    timeout = 10000;
  }

  timestamp.reset();
  if (!this->parseRtmpUrl(url)) {
    ILOG_INFO_FMT(
      g_rtmp_logger, "[RtmpPublisher] rtmp url({}) was illegal.", url);
    return false;
  }

  ILOG_INFO_FMT(g_rtmp_logger, "[RtmpPublisher] ip:{}, port:{}, stream path:{}",
    ip_, port_, stream_path_);
  if (rtmp_conn_ != nullptr) {
    std::shared_ptr<RtmpConnection> rtmpConn = rtmp_conn_;
    sockfd_t sockfd = rtmpConn->getSockfd();
    task_scheduler_->addTriggerEvent(
      [sockfd, rtmpConn]() { rtmpConn->disconnect(); });
    rtmp_conn_ = nullptr;
  }

  // FIXME: fix me
  std::unique_ptr<TcpSocket> tcp_socket = std::make_unique<TcpSocket>();
  if (!tcp_socket->connect({ip_, port_}, std::chrono::milliseconds(timeout))) {
    tcp_socket->close();
    return false;
  }

  task_scheduler_ = event_loop_->getTaskScheduler().get();
  rtmp_conn_.reset(new RtmpConnection(
    shared_from_this(), task_scheduler_, tcp_socket->getSockfd()));
  task_scheduler_->addTriggerEvent([this]() { rtmp_conn_->handshake(); });

  timeout -= timestamp.elapsed().count();
  if (timeout < 0) {
    timeout = 1000;
  }

  do {
    Timer::sleep(100ms);
    timeout -= 100;
  } while (
    rtmp_conn_->isRunning() && !rtmp_conn_->isPublishing() && timeout > 0);

  status = rtmp_conn_->getStatus();
  if (!rtmp_conn_->isPublishing()) {
    std::shared_ptr<RtmpConnection> rtmpConn = rtmp_conn_;
    sockfd_t sockfd = rtmpConn->getSockfd();
    task_scheduler_->addTriggerEvent(
      [sockfd, rtmpConn]() { rtmpConn->disconnect(); });
    rtmp_conn_ = nullptr;
    return false;
  }

  video_timestamp_ = 0;
  audio_timestamp_ = 0;
  has_key_frame_ = true;
  if (media_info_.video_codec_id == RTMP_CODEC_ID_H264) {
    has_key_frame_ = false;
  }
  return true;
}

void RtmpPublisher::close() {
  Mutex::lock locker(mutex_);

  if (rtmp_conn_ != nullptr) {
    std::shared_ptr<RtmpConnection> rtmp_conn = rtmp_conn_;
    sockfd_t sockfd = rtmp_conn->getSockfd();
    task_scheduler_->addTriggerEvent(
      [sockfd, rtmp_conn]() { rtmp_conn->disconnect(); });
    rtmp_conn_ = nullptr;
    video_timestamp_ = 0;
    audio_timestamp_ = 0;
    has_key_frame_ = false;
  }
}

bool RtmpPublisher::isConnected() const {
  Mutex::lock locker(mutex_);
  return rtmp_conn_ && rtmp_conn_->isRunning();
}

bool RtmpPublisher::isKeyFrame(uint8_t *data, uint32_t size) const {
  int startCode = 0;
  if (data[0] == 0 && data[1] == 0 && data[2] == 1) {
    startCode = 3;
  }
  else if (data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 1) {
    startCode = 4;
  }

  int type = data[startCode] & 0x1F;
  if (type == 5 || type == 7) {  // sps_pps_idr or idr
    return true;
  }

  return false;
}

bool RtmpPublisher::pushVideoFrame(uint8_t *data, uint32_t size) {
  Mutex::lock locker(mutex_);
  if (rtmp_conn_ == nullptr || (!rtmp_conn_->isRunning()) || size <= 5) {
    return false;
  }

  if (media_info_.video_codec_id == RTMP_CODEC_ID_H264) {
    if (!has_key_frame_) {
      if (!this->isKeyFrame(data, size)) {
        return true;
      }
      has_key_frame_ = true;
      timestamp_.reset();
      // task_scheduler_->addTriggerEvent([=]() {
      rtmp_conn_->sendVideoData(
        0, avc_sequence_header_, avc_sequence_header_size_);
      rtmp_conn_->sendAudioData(
        0, aac_sequence_header_, aac_sequence_header_size_);
      //});
    }

    auto timestamp = timestamp_.elapsed().count();
    // uint64_t timestamp_delta = 0;
    // if (timestamp < video_timestamp_)
    //{
    //	timestamp = video_timestamp_;
    // }
    // timestamp_delta = timestamp - video_timestamp_;
    // video_timestamp_ = timestamp;

    std::shared_ptr<char> payload(
      new char[size + 4096], std::default_delete<char[]>());
    uint32_t payload_size = 0;

    uint8_t *buffer = (uint8_t *) payload.get();
    uint32_t index = 0;
    buffer[index++] = this->isKeyFrame(data, size) ? 0x17 : 0x27;
    buffer[index++] = 1;

    buffer[index++] = 0;
    buffer[index++] = 0;
    buffer[index++] = 0;

    buffer[index++] = (size >> 24) & 0xFF;
    buffer[index++] = (size >> 16) & 0xFF;
    buffer[index++] = (size >> 8) & 0xFF;
    buffer[index++] = (size >> 0) & 0xFF;

    memcpy(buffer + index, data, size);
    index += size;
    payload_size = index;
    // task_scheduler_->addTriggerEvent([=]() {
    rtmp_conn_->sendVideoData(timestamp, payload, payload_size);
    //});
  }

  return true;
}

bool RtmpPublisher::pushAudioFrame(uint8_t *data, uint32_t size) {
  Mutex::lock locker(mutex_);
  if (rtmp_conn_ == nullptr || (!rtmp_conn_->isRunning()) || size <= 0) {
    return false;
  }

  if (has_key_frame_ && media_info_.audio_codec_id == RTMP_CODEC_ID_AAC) {
    auto timestamp = timestamp_.elapsed().count();
    // uint64_t timestamp_delta = 0;
    // if (timestamp < audio_timestamp_)
    //{
    //	timestamp = audio_timestamp_;
    // }
    // timestamp_delta = timestamp - audio_timestamp_;
    // audio_timestamp_ = timestamp;

    uint32_t payload_size = size + 2;
    std::shared_ptr<char> payload(
      new char[size + 2], std::default_delete<char[]>());
    payload.get()[0] = audio_tag_;
    payload.get()[1] = 1;  // 0: aac sequence header, 1: aac raw data
    memcpy(payload.get() + 2, data, size);
    // task_scheduler_->addTriggerEvent([=]() {
    rtmp_conn_->sendAudioData(timestamp, payload, payload_size);
    //});
  }

  return true;
}
NAMESPACE_END(net)
LY_NAMESPACE_END
