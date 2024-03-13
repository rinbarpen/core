#pragma once

#include <core/util/marcos.h>
#include <core/util/Mutex.h>
#include <core/util/time/Timestamp.h>
#include <core/net/EventLoop.h>
#include <core/net/tcp/TcpSocket.h>
#include <core/multimedia/net/rtmp/RtmpConnection.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class RtmpPublisher : public Rtmp, public std::enable_shared_from_this<RtmpPublisher>
{
	friend class RtmpConnection;
public:
	static std::shared_ptr<RtmpPublisher> create(EventLoop* event_loop);
	~RtmpPublisher();

	bool setMediaInfo(MediaInfo media_info);

	bool openUrl(std::string url, int msec, std::string& status);
	bool openUrl(std::string url, std::chrono::milliseconds msec, std::string& status);
	void close();

	bool isConnected() const;

	bool pushVideoFrame(uint8_t *data, uint32_t size); /* (sps pps)idr frame or p frame */
	bool pushAudioFrame(uint8_t *data, uint32_t size);

private:
	RtmpPublisher(EventLoop *event_loop);
	bool isKeyFrame(uint8_t* data, uint32_t size) const;

private:
	mutable Mutex::type mutex_;
	EventLoop *event_loop_ = nullptr;
	TaskScheduler *task_scheduler_ = nullptr;
	std::shared_ptr<RtmpConnection> rtmp_conn_;
	std::unique_ptr<TcpSocket> tcp_socket_;

	MediaInfo media_info_;
	std::shared_ptr<char> avc_sequence_header_;
	std::shared_ptr<char> aac_sequence_header_;
	uint32_t avc_sequence_header_size_ = 0;
	uint32_t aac_sequence_header_size_ = 0;
	uint8_t audio_tag_ = 0;
	bool has_key_frame_ = false;
	Timestamp<T_steady_clock> timestamp_;
	uint64_t video_timestamp_ = 0;
	uint64_t audio_timestamp_ = 0;

	static constexpr uint32_t kSamplingFrequency[16] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0};
};

NAMESPACE_END(net)
LY_NAMESPACE_END
