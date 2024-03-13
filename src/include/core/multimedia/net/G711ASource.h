#pragma once

#include <core/multimedia/net/MediaSource.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
class G711ASource : public MediaSource
{
public:
	static G711ASource* create();
	virtual ~G711ASource();

	bool handleFrame(MediaChannelId channel_id, SimAVFrame frame) override;
	virtual std::string getMediaDescription(uint16_t port) const override;
	virtual std::string getAttribute() const override;

	static uint32_t getTimestamp();

	uint32_t getSampleRate() const
	{ return sample_rate_; }
	uint32_t getChannels() const
	{ return channels_; }
private:
	G711ASource();

private:
	uint32_t sample_rate_ = 8000;
	uint32_t channels_ = 1;
};
NAMESPACE_END(net)
LY_NAMESPACE_END
