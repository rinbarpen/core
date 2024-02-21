#include <core/multimedia/net/rtmp/RtmpSession.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
RtmpSession::RtmpSession() {}
RtmpSession::~RtmpSession() {}

void RtmpSession::sendMetaData(AmfObjectMap &metaData) {
  Mutex::lock locker(mutex_);

  for (auto iter = rtmp_clients_.begin(); iter != rtmp_clients_.end();) {
    auto conn = iter->second.lock();
    if (conn == nullptr) {
      rtmp_clients_.erase(iter++);
    }
    else {
      if (conn->isPlayer()) {
        conn->sendMetaData(metaData);
      }
      iter++;
    }
  }
}

void RtmpSession::sendMediaData(
  uint8_t type, uint64_t timestamp, std::shared_ptr<char> data, uint32_t size) {
  Mutex::lock locker(mutex_);

  if (this->max_gop_cache_len_ > 0) {
    this->saveGop(type, timestamp, data, size);
  }

  for (auto iter = rtmp_clients_.begin(); iter != rtmp_clients_.end();) {
    auto conn = iter->second.lock();
    if (conn == nullptr) {
      rtmp_clients_.erase(iter++);
    }
    else {
      if (conn->isPlayer()) {
        if (!conn->isPlaying()) {
          conn->sendMetaData(meta_data_);
          conn->sendMediaData(RTMP_AVC_SEQUENCE_HEADER, 0,
            this->avc_sequence_header_, this->avc_sequence_header_size_);
          conn->sendMediaData(RTMP_AAC_SEQUENCE_HEADER, 0,
            this->aac_sequence_header_, this->aac_sequence_header_size_);

          if (gop_cache_.size() > 0) {
            auto gop = gop_cache_.begin()->second;
            for (auto iter : *gop) {
              if (iter->type == RTMP_VIDEO) {
                conn->sendVideoData(iter->timestamp, iter->data, iter->size);
              }
              else if (iter->type == RTMP_AUDIO) {
                conn->sendAudioData(iter->timestamp, iter->data, iter->size);
              }
            }
          }
        }
        conn->sendMediaData(type, timestamp, data, size);
      }
      iter++;
    }
  }

  for (auto iter = http_clients_.begin(); iter != http_clients_.end();) {
    auto conn = iter->second.lock();
    if (conn == nullptr) {  // conn disconect
      http_clients_.erase(iter++);
    }
    else {
      if (!conn->isPlaying()) {
        conn->sendMediaData(RTMP_AVC_SEQUENCE_HEADER, 0, avc_sequence_header_,
          avc_sequence_header_size_);
        conn->sendMediaData(RTMP_AAC_SEQUENCE_HEADER, 0, aac_sequence_header_,
          aac_sequence_header_size_);

        if (gop_cache_.size() > 0) {
          auto gop = gop_cache_.begin()->second;
          for (auto iter : *gop) {
            if (iter->type == RTMP_VIDEO) {
              conn->sendMediaData(
                RTMP_VIDEO, iter->timestamp, iter->data, iter->size);
            }
            else if (iter->type == RTMP_AUDIO) {
              conn->sendMediaData(
                RTMP_AUDIO, iter->timestamp, iter->data, iter->size);
            }
          }

          conn->resetKeyFrame();
        }
      }

      conn->sendMediaData(type, timestamp, data, size);
      iter++;
    }
  }

  return;
}

void RtmpSession::saveGop(
  uint8_t type, uint64_t timestamp, std::shared_ptr<char> data, uint32_t size) {
  uint8_t *payload = (uint8_t *) data.get();
  uint8_t frame_type = 0;
  uint8_t codec_id = 0;
  std::shared_ptr<AVFrame> av_frame = nullptr;
  std::shared_ptr<std::list<std::shared_ptr<AVFrame>>> gop = nullptr;
  if (gop_cache_.size() > 0) {
    gop = gop_cache_[gop_index_];
  }

  if (type == RTMP_VIDEO) {
    frame_type = (payload[0] >> 4) & 0x0F;
    codec_id = payload[0] & 0x0f;
    if (frame_type == 1 && codec_id == RTMP_CODEC_ID_H264) {
      if (payload[1] == 1) {
        if (max_gop_cache_len_ > 0) {
          if (gop_cache_.size() == 2) {
            gop_cache_.erase(gop_cache_.begin());
          }
          gop_index_ += 1;
          gop.reset(new std::list<std::shared_ptr<AVFrame>>);
          gop_cache_[gop_index_] = gop;
          av_frame.reset(new AVFrame);
        }
      }
    }
    else if (codec_id == RTMP_CODEC_ID_H264 && gop != nullptr) {
      if (max_gop_cache_len_ > 0 && gop->size() >= 1
          && gop->size() < max_gop_cache_len_) {
        av_frame.reset(new AVFrame);
      }
    }
  }
  else if (type == RTMP_AUDIO && gop != nullptr) {
    uint8_t sound_format = (payload[0] >> 4) & 0x0F;
    // uint8_t sound_size = (payload[0] >> 1) & 0x01;
    // uint8_t sound_rate = (payload[0] >> 2) & 0x03;

    if (sound_format == RTMP_CODEC_ID_AAC) {
      if (max_gop_cache_len_ > 0 && gop->size() >= 2
          && gop->size() < max_gop_cache_len_) {
        if (timestamp > 0) {
          av_frame.reset(new AVFrame);
        }
      }
    }
  }

  if (av_frame != nullptr && gop != nullptr) {
    av_frame->type = type;
    av_frame->timestamp = timestamp;
    av_frame->size = size;
    av_frame->data.reset(new char[size], std::default_delete<char[]>());
    memcpy(av_frame->data.get(), data.get(), size);
    gop->push_back(av_frame);
  }
}

void RtmpSession::addRtmpClient(std::shared_ptr<RtmpConnection> conn) {
  Mutex::lock locker(mutex_);
  rtmp_clients_[conn->getSockfd()] = conn;
  if (conn->isPublisher()) {
    avc_sequence_header_ = nullptr;
    aac_sequence_header_ = nullptr;
    avc_sequence_header_size_ = 0;
    aac_sequence_header_size_ = 0;
    gop_cache_.clear();
    gop_index_ = 0;
    has_publisher_ = true;
    publisher_ = conn;
  }
  return;
}

void RtmpSession::removeRtmpClient(std::shared_ptr<RtmpConnection> conn) {
  Mutex::lock locker(mutex_);
  if (conn->isPublisher()) {
    avc_sequence_header_ = nullptr;
    aac_sequence_header_ = nullptr;
    avc_sequence_header_size_ = 0;
    aac_sequence_header_size_ = 0;
    gop_cache_.clear();
    gop_index_ = 0;
    has_publisher_ = false;
  }
  rtmp_clients_.erase(conn->getSockfd());
}

void RtmpSession::addHttpClient(std::shared_ptr<HttpFlvConnection> conn) {
  Mutex::lock locker(mutex_);
  http_clients_[conn->getSockfd()] = conn;
}

void RtmpSession::removeHttpClient(std::shared_ptr<HttpFlvConnection> conn) {
  Mutex::lock locker(mutex_);
  http_clients_.erase(conn->getSockfd());
}

int RtmpSession::getClients() {
  Mutex::lock locker(mutex_);

  int clients = 0;
  for (auto iter : rtmp_clients_) {
    auto conn = iter.second.lock();
    if (conn != nullptr) {
      clients += 1;
    }
  }

  for (auto iter : http_clients_) {
    auto conn = iter.second.lock();
    if (conn != nullptr) {
      clients += 1;
    }
  }

  return clients;
}

std::shared_ptr<RtmpConnection> RtmpSession::getPublisher() {
  Mutex::lock locker(mutex_);
  return publisher_.lock();
}

NAMESPACE_END(net)
LY_NAMESPACE_END
