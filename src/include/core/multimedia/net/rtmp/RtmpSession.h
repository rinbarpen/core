#pragma once

#include <list>
#include <unordered_map>
#include <map>
#include <core/util/marcos.h>
#include <core/util/Mutex.h>
#include <core/multimedia/net/rtmp/amf.h>
#include <core/multimedia/net/rtmp/RtmpConnection.h>
#include <core/multimedia/net/http/HttpFlvConnection.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class RtmpConnection;
class HttpFlvConnection;

class RtmpSession
{
public:
  SHARED_PTR_USING(RtmpSession, ptr);

	RtmpSession();
	virtual ~RtmpSession();

	void setMetaData(AmfObjectMap metaData)
	{
		Mutex::lock lock(mutex_);
		meta_data_ = metaData;
	}
	void setAvcSequenceHeader(std::shared_ptr<char> avcSequenceHeader, uint32_t avcSequenceHeaderSize)
	{
		Mutex::lock lock(mutex_);
		avc_sequence_header_ = avcSequenceHeader;
		avc_sequence_header_size_ = avcSequenceHeaderSize;
	}
	void setAacSequenceHeader(std::shared_ptr<char> aacSequenceHeader, uint32_t aacSequenceHeaderSize)
	{
		Mutex::lock lock(mutex_);
		aac_sequence_header_ = aacSequenceHeader;
		aac_sequence_header_size_ = aacSequenceHeaderSize;
	}
	AmfObjectMap getMetaData() const
	{
		Mutex::lock lock(mutex_);
		return meta_data_;
	}

	void addRtmpClient(std::shared_ptr<RtmpConnection> conn);
	void removeRtmpClient(std::shared_ptr<RtmpConnection> conn);
	void addHttpClient(std::shared_ptr<HttpFlvConnection> conn);
	void removeHttpClient(std::shared_ptr<HttpFlvConnection> conn);
	int  getClients();

	void sendMetaData(AmfObjectMap& metaData);
	void sendMediaData(uint8_t type, uint64_t timestamp, std::shared_ptr<char> data, uint32_t size);

	std::shared_ptr<RtmpConnection> getPublisher();

	void setGopCache(uint32_t cacheLen)
	{
		Mutex::lock lock(mutex_);
		max_gop_cache_len_ = cacheLen;
	}
	void saveGop(uint8_t type, uint64_t timestamp, std::shared_ptr<char> data, uint32_t size);

private:
  AmfObjectMap meta_data_;
  bool has_publisher_ = false;
	std::weak_ptr<RtmpConnection> publisher_;
  std::unordered_map<sockfd_t, std::weak_ptr<RtmpConnection>> rtmp_clients_;
	std::unordered_map<sockfd_t, std::weak_ptr<HttpFlvConnection>> http_clients_;

	std::shared_ptr<char> avc_sequence_header_;
	std::shared_ptr<char> aac_sequence_header_;
	uint32_t avc_sequence_header_size_ = 0;
	uint32_t aac_sequence_header_size_ = 0;
	uint64_t gop_index_ = 0;
	uint32_t max_gop_cache_len_ = 0;

	struct AVFrame {
		uint8_t  type = 0;
		uint64_t timestamp = 0;
		uint32_t size = 0;
		std::shared_ptr<char> data = nullptr;
	};
	std::map<uint64_t, std::shared_ptr<std::list<std::shared_ptr<AVFrame>>>> gop_cache_;
  mutable Mutex::type mutex_;
};

NAMESPACE_END(net)
LY_NAMESPACE_END
