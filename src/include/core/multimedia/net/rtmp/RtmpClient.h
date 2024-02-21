#pragma once

#include <core/util/marcos.h>
#include <core/util/Mutex.h>
#include <core/util/time/Timestamp.h>
#include <core/net/EventLoop.h>
#include <core/multimedia/net/rtmp/RtmpConnection.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

class RtmpClient : public Rtmp, public std::enable_shared_from_this<RtmpClient>
{
	friend class RtmpConnection;
public:
	using FrameCallback = std::function<void(uint8_t* payload, uint32_t length, uint8_t codec_id, uint32_t timestamp)>;

	static std::shared_ptr<RtmpClient> create(EventLoop* event_loop);
	RtmpClient(EventLoop *event_loop);
	~RtmpClient();

	void setFrameCallBack(FrameCallback callback);
	bool openUrl(std::string url, int msec, std::string& status);
	bool openUrl(std::string url, std::chrono::milliseconds msec, std::string& status);
	void close();
	bool isConnected();

private:
	mutable Mutex::type mutex_;
	EventLoop* event_loop_;
	TaskScheduler* task_scheduler_;
	std::shared_ptr<RtmpConnection> rtmp_conn_;
	FrameCallback frame_callback_;
};

NAMESPACE_END(net)
LY_NAMESPACE_END
