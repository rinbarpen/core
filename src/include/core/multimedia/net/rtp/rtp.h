#pragma once

#include "core/util/ds/SharedString.h"
#include <memory>
#include <cstdint>

#include <core/util/marcos.h>

static constexpr int RTP_HEADER_SIZE = 12;
static constexpr int MAX_RTP_PAYLOAD_SIZE = 1420; //1460  1500-20-12-8
static constexpr int RTP_VERSION = 2;
static constexpr int RTP_TCP_HEAD_SIZE = 4;

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)

enum TransportMode
{
	RTP_OVER_TCP = 1,
	RTP_OVER_UDP = 2,
	RTP_OVER_MULTICAST = 3,
};

struct RtpHeader
{
	/* Little Eden */
	unsigned char csrc:4;
	unsigned char extension:1;
	unsigned char padding:1;
	unsigned char version:2;
	unsigned char payload:7;
	unsigned char marker:1;

	unsigned short seq;
	unsigned int   ts;
	unsigned int   ssrc;
};

struct MediaChannelInfo
{
	RtpHeader rtp_header;

	// tcp
	uint16_t rtp_channel;
	uint16_t rtcp_channel;

	// udp
	uint16_t rtp_port;
	uint16_t rtcp_port;
	uint16_t packet_seq;
	uint32_t clock_rate;

	// rtcp
	uint64_t packet_count;
	uint64_t octet_count;
	uint64_t last_rtcp_ntp_time;

	bool is_setup;
	bool is_play;
	bool is_record;
};

static constexpr int kRtpPacketBufferSize = 1600;

struct RtpPacket
{
  SharedString data{kRtpPacketBufferSize};
	uint32_t timestamp;
	uint8_t  type{0};
	uint8_t  last;
};


NAMESPACE_END(net)
LY_NAMESPACE_END
