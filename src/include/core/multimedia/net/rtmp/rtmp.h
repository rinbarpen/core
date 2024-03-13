#pragma once

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
static constexpr int RTMP_VERSION = 0x3;
static constexpr int RTMP_SET_CHUNK_SIZE = 0x1; /* 设置块大小 */
static constexpr int RTMP_ABORT_MESSAGE = 0X2;  /* 终止消息 */
static constexpr int RTMP_ACK = 0x3;            /* 确认 */
static constexpr int RTMP_USER_EVENT = 0x4;     /* 用户控制消息 */
static constexpr int RTMP_ACK_SIZE = 0x5;       /* 窗口大小确认 */
static constexpr int RTMP_BANDWIDTH_SIZE = 0x6; /* 设置对端带宽 */
static constexpr int RTMP_AUDIO = 0x08;
static constexpr int RTMP_VIDEO = 0x09;
static constexpr int RTMP_FLEX_MESSAGE = 0x11;  // amf3
static constexpr int RTMP_NOTIFY = 0x12;
static constexpr int RTMP_INVOKE = 0x14;  // amf0
static constexpr int RTMP_FLASH_VIDEO = 0x16;

static constexpr int RTMP_CHUNK_TYPE_0 = 0;  // 11
static constexpr int RTMP_CHUNK_TYPE_1 = 1;  // 7
static constexpr int RTMP_CHUNK_TYPE_2 = 2;  // 3
static constexpr int RTMP_CHUNK_TYPE_3 = 3;  // 0

static constexpr int RTMP_CHUNK_CONTROL_ID = 2;
static constexpr int RTMP_CHUNK_INVOKE_ID = 3;
static constexpr int RTMP_CHUNK_AUDIO_ID = 4;
static constexpr int RTMP_CHUNK_VIDEO_ID = 5;
static constexpr int RTMP_CHUNK_DATA_ID = 6;

static constexpr int RTMP_CODEC_ID_H264 = 7;
static constexpr int RTMP_CODEC_ID_AAC = 10;
static constexpr int RTMP_CODEC_ID_G711A = 7;
static constexpr int RTMP_CODEC_ID_G711U = 8;

static constexpr int RTMP_AVC_SEQUENCE_HEADER = 0x18;
static constexpr int RTMP_AAC_SEQUENCE_HEADER = 0x19;

struct MediaInfo
{
  uint8_t video_codec_id = RTMP_CODEC_ID_H264;
  uint8_t video_frame_rate = 0;
  uint32_t video_width = 0;
  uint32_t video_height = 0;
  std::shared_ptr<uint8_t> sps;
  std::shared_ptr<uint8_t> pps;
  std::shared_ptr<uint8_t> sei;
  uint32_t sps_size = 0;
  uint32_t pps_size = 0;
  uint32_t sei_size = 0;

  uint8_t audio_codec_id = RTMP_CODEC_ID_AAC;
  uint32_t audio_channel = 0;
  uint32_t audio_sample_rate = 0;
  uint32_t audio_frame_len = 0;
  std::shared_ptr<uint8_t> audio_specific_config;
  uint32_t audio_specific_config_size = 0;
};

class Rtmp
{
public:
  Rtmp() = default;
  virtual ~Rtmp() = default;

  void setChunkSize(uint32_t size) {
    if (size > 0 && size <= 60000) {
      max_chunk_size_ = size;
    }
  }

  void setGopCache(uint32_t len = 10000) { max_gop_cache_len_ = len; }
  void setPeerBandwidth(uint32_t size) { peer_bandwidth_ = size; }

  uint32_t getChunkSize() const { return max_chunk_size_; }
  uint32_t getGopCacheLen() const { return max_gop_cache_len_; }
  uint32_t getAcknowledgementSize() const { return acknowledgement_size_; }
  uint32_t getPeerBandwidth() const { return peer_bandwidth_; }

  virtual bool parseRtmpUrl(const std::string &url) {
    char ip[100] = {0};
    char streamPath[500] = {0};
    char app[100] = {0};
    char streamName[400] = {0};
    uint16_t port = 0;

    if (sscanf(url.c_str(), "rtmp://%[^:]:%hu/%s", ip, &port, streamPath) == 3) {
    }
    else if (sscanf(url.c_str(), "rtmp://%[^/]/%s", ip, streamPath) == 2) {
      port = 1935;
    }
    else {
      return false;
    }

    ip_ = ip;
    port_ = port;
    stream_path_ = "/";
    stream_path_ += streamPath;
    url_ = url;

    if (sscanf(stream_path_.c_str(), "/%[^/]/%s", app, streamName) != 2) {
      return false;
    }

    app_ = app;
    stream_name_ = streamName;
    swf_url_ = tc_url_ =
      std::string("rtmp://") + ip_ + ":" + std::to_string(port_) + "/" + app_;
    return true;
  }

  std::string getUrl() const { return url_; }
  std::string getStreamPath() const { return stream_path_; }
  std::string getApp() const { return app_; }
  std::string getStreamName() const { return stream_name_; }
  std::string getSwfUrl() const { return swf_url_; }
  std::string getTcUrl() const { return tc_url_; }

protected:
  std::string url_;
  std::string tc_url_, swf_url_;
  std::string ip_;
  uint16_t port_{1935};
  std::string stream_path_;
  std::string app_;
  std::string stream_name_;

  uint32_t peer_bandwidth_{5000 * 1000};
  uint32_t acknowledgement_size_{5000 * 1000};
  uint32_t max_chunk_size_{128};
  uint32_t max_gop_cache_len_{0};
};
NAMESPACE_END(net)
LY_NAMESPACE_END
