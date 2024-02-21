#include <random>
#include <core/multimedia/net/rtmp/RtmpHandshake.h>
#include <core/multimedia/net/rtmp/rtmp.h>
#include <core/util/logger/Logger.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
static auto g_rtmp_logger = GET_LOGGER("multimedia.rtmp");

RtmpHandshake::RtmpHandshake(Status status)
  : handshake_status_(status)
{}
RtmpHandshake::~RtmpHandshake() {}

int RtmpHandshake::parse(BufferReader &in, char *res_buf, uint32_t res_buf_size) {
	uint8_t *buf = (uint8_t*)in.peek();
	uint32_t buf_size = in.readableBytes();
	uint32_t offset = 0;
	uint32_t res_size = 0;
	std::random_device rd;

  switch (handshake_status_) {
  case HANDSHAKE_S0S1S2:
  {
    if (buf_size < (1 + 1536 + 1536)) { //S0S1S2
			return res_size;
		}

		if (buf[0] != RTMP_VERSION) {
			ILOG_ERROR_FMT(g_rtmp_logger, "unsupported rtmp version {:x}", buf[0]);
			return -1;
		}

		offset += 1 + 1536 + 1536;
		res_size = 1536;
		memcpy(res_buf, buf + 1, 1536); //C2
		handshake_status_ = HANDSHAKE_COMPLETE;
    break;
  }
  case HANDSHAKE_C0C1:
  {
		if (buf_size < 1537) { //C0C1
			return res_size;
		}
    if (buf[0] != RTMP_VERSION) {
      return -1;
    }

    offset += 1537;
    res_size = 1 + 1536 + 1536;
    memset(res_buf, 0, 1537); //S0 S1 S2
    res_buf[0] = RTMP_VERSION;

    char *p = res_buf; p += 9;
    for (int i = 0; i < 1528; i++) {
      *p++ = rd();
    }
    memcpy(p, buf + 1, 1536);
    handshake_status_ = HANDSHAKE_C2;
    break;
  }
  case HANDSHAKE_C2:
  {
		if (buf_size < 1536) { // C2
			return res_size;
		}

    offset = 1536;
    handshake_status_ = HANDSHAKE_COMPLETE;
    break;
  }
  default: return -1;
  }

	in.advance(offset);
	return res_size;
}
bool RtmpHandshake::buildC0C1(char *buf, uint32_t buf_size) {
	uint32_t size = 1 + 1536; // COC1
	::memset(buf, 0, 1537);
	buf[0] = RTMP_VERSION;

	std::random_device rd;
	uint8_t *p = (uint8_t *)buf; p += 9;
	for (int i = 0; i < 1537 - 9; i++) {
		*p++ = rd();
	}

	return size;
}
NAMESPACE_END(net)
LY_NAMESPACE_END
