#pragma once

#include <atomic>
#include <core/multimedia/ffmpeg/FFmpegUtil.h>
#include <core/multimedia/ffmpeg/Encoder.h>
#include <core/multimedia/util/AVClock.h>
#include <core/multimedia/util/AVQueue.h>
#include <core/multimedia/ffmpeg/Decoder.h>
#include "core/util/thread/Thread.h"

LY_NAMESPACE_BEGIN
#if 1
class AVThread
{
public:
  AVThread(std::string_view threadName) : thread_(threadName) {}
  virtual ~AVThread() = default;
  virtual void open() { launched_ = true; thread_.dispatch(&AVThread::run, this); }
  virtual void close() { launched_ = false; thread_.destroy(); }
  virtual void run() = 0;

protected:
  std::atomic_bool launched_{false};
  Thread thread_{""};
};
class AVEncodeThread : public AVThread
{
public:
  AVEncodeThread(AVFrameQueue &in, AVPacketQueue &out)
    : AVThread("AVEncodeThread"),
      in_frames_(in), out_packets_(out)
  {}
  ~AVEncodeThread() = default;

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

      auto pkt = encoder_->encode(frame);
      if (pkt) {
        out_packets_.push(pkt);
      }
    }
  }

private:
  std::atomic_bool launched_{false};

  std::unique_ptr<ffmpeg::Encoder> encoder_;
  AVFrameQueue &in_frames_;
  AVPacketQueue &out_packets_;
};
class AVDecodeThread : public AVThread
{
public:
  AVDecodeThread(AVPacketQueue &in, AVFrameQueue &out)
    : AVThread("AVDecodeThread"),
      in_packets_(in), out_frames_(out)
  {}
  ~AVDecodeThread() = default;

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

      auto frame = decoder_->decode(packet);
      if (frame) {
        out_frames_.push(frame);
      }
    }
  }

private:
  std::atomic_bool launched_{false};

  std::unique_ptr<ffmpeg::Decoder> decoder_;
  AVPacketQueue &in_packets_;
  AVFrameQueue &out_frames_;
};
#endif
LY_NAMESPACE_END
