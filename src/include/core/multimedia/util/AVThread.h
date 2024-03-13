#pragma once

#include <thread>
#include <atomic>
#include <queue>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>
#include <core/multimedia/ffmpeg/Encoder.h>
#include <core/multimedia/util/AVClock.h>
#include <core/multimedia/util/AVQueue.h>
#include <core/multimedia/util/AACEncoder.h>

LY_NAMESPACE_BEGIN
#if 0
class AVThread
{
public:
  virtual bool open(EncoderConfig config) = 0;
  virtual bool close() = 0;
  virtual void run() = 0;

private:

};
class AVEncodeThread : public AVThread
{
public:
  AVEncodeThread(AVQueue<ffmpeg::AVPacketPtr> &in, AVQueue<ffmpeg::AVPacketPtr> &out);
  ~AVEncodeThread() = default;

  bool open(EncoderConfig config) override;
  bool close() override;

  void run() override {
    while (launched_) {
      auto frame_or_not = in_frames_.pop();
      ffmpeg::AVFramePtr frame;
      if (frame_or_not.has_value()) {
        frame = frame_or_not.value();
      }
      else {
        // no frame
        continue;
      }

      auto pkt = encoder_->encode(frame, encoder_config_);
      if (pkt) {
        out_packets_.push(pkt);
      }
    }
  }

private:
  std::atomic_bool launched_{false};

  std::unique_ptr<Encoder> encoder_;
  AVQueue<ffmpeg::AVFramePtr> &in_frames_;
  AVQueue<ffmpeg::AVPacketPtr> &out_packets_;
  EncoderConfig encoder_config_;
};
class AVDecodeThread : public AVThread
{
public:
  AVDecodeThread(AVQueue<ffmpeg::AVPacketPtr> &in, AVQueue<ffmpeg::AVPacketPtr> &out);
  ~AVDecodeThread() = default;

  bool open(EncoderConfig config) override;
  bool close() override;

  void run() override {
    while (launched_) {
      auto packet_or_not = in_packets_.pop();
      ffmpeg::AVPacketPtr packet;
      if (packet_or_not.has_value()) {
        packet = packet_or_not.value();
      }
      else {
        // no packet
        continue;
      }

      auto frame = decoder_->decode(packet, decoder_config_);
      if (frame) {
        out_frames_.push(frame);
      }
    }
  }

private:
  std::atomic_bool launched_{false};

  // std::unique_ptr<Decoder> decoder_;
  AVQueue<ffmpeg::AVPacketPtr> &in_packets_;
  AVQueue<ffmpeg::AVFramePtr> &out_frames_;
  DecoderConfig decoder_config_;

};
#endif
LY_NAMESPACE_END
