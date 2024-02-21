#include <cstring>
#include <memory>
#include <core/util/buffer/BufferReader.h>
#include <core/util/buffer/BufferWriter.h>
#include <core/multimedia/net/rtmp/amf.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
// AmfDecoder
int AmfDecoder::decode(const char *data, int size, int n) {
  int bytesUsed = 0;
  while (size > bytesUsed) {
    int r{0};
    char type = data[bytesUsed];
    ++bytesUsed;

    switch (type) {
    case AMF0DataType::AMF0_NUMBER:
      obj_.type = AmfObjectType::AMF_NUMBER;
      r = decodeNumber(data + bytesUsed, size - bytesUsed, obj_.number());
      break;
    case AMF0DataType::AMF0_BOOLEAN:
      obj_.type = AmfObjectType::AMF_BOOLEAN;
      r = decodeBoolean(data + bytesUsed, size - bytesUsed, obj_.boolean());
      break;
    case AMF0DataType::AMF0_STRING:
      obj_.type = AmfObjectType::AMF_STRING;
      r = decodeString(data + bytesUsed, size - bytesUsed, obj_.string());
      break;
    case AMF0DataType::AMF0_OBJECT:
      r = decodeObject(data + bytesUsed, size - bytesUsed, obj_map_);
      break;
    case AMF0DataType::AMF0_OBJECT_END: break;
    case AMF0DataType::AMF0_ECMA_ARRAY: break;
    case AMF0DataType::AMF0_NULL: break;
    default: break;
    }

    if (r < 0) {
      break;
    }
    bytesUsed += r;
    --n;
    if (n == 0) {
      break;
    }
  }

  return bytesUsed;
}
int AmfDecoder::decodeBoolean(const char *data, int size, bool &amf_boolean) {
  if (size < 1) {
    return 0;
  }

  amf_boolean = (data[0] != 0);
  return 1;
}
int AmfDecoder::decodeNumber(const char *data, int size, double &amf_number) {
  if (size < 8) {
    return 0;
  }

  char *ci = (char *) data;
  char *co = (char *) &amf_number;
  co[0] = ci[7];
  co[1] = ci[6];
  co[2] = ci[5];
  co[3] = ci[4];
  co[4] = ci[3];
  co[5] = ci[2];
  co[6] = ci[1];
  co[7] = ci[0];

  return 8;
}
int AmfDecoder::decodeString(
  const char *data, int size, std::string &amf_string) {
  if (size < 2) {
    return 0;
  }

  int bytesUsed = 0;
  int strSize = decodeInt16(data, size);
  bytesUsed += 2;
  if (strSize > (size - bytesUsed)) {
    return -1;
  }

  amf_string = std::string(&data[bytesUsed], 0, strSize);
  bytesUsed += strSize;
  return bytesUsed;
}
int AmfDecoder::decodeObject(
  const char *data, int size, AmfObjectMap &amf_obj_map) {
  amf_obj_map.clear();

  int bytesUsed = 0;
  while (size > 0) {
    int strSize = decodeInt16(data + bytesUsed, size);
    size -= 2;
    if (strSize > size) {
      return bytesUsed;
    }

    std::string key(data + bytesUsed + 2, 0, strSize);
    size -= strSize;

    AmfDecoder dec;
    int r = dec.decode(data + bytesUsed + 2 + strSize, size, 1);
    bytesUsed += 2 + strSize + r;
    if (r <= 1) {
      break;
    }

    amf_obj_map.emplace(key, dec.getObject());
  }

  return bytesUsed;
}
uint16_t AmfDecoder::decodeInt16(const char *data, int size) {
  uint16_t val = readU16Forward((char *) data);
  return val;
}
uint32_t AmfDecoder::decodeInt24(const char *data, int size) {
  uint32_t val = readU24Forward((char *) data);
  return val;
}
uint32_t AmfDecoder::decodeInt32(const char *data, int size) {
  uint32_t val = readU32Forward((char *) data);
  return val;
}

// AmfEncoder
AmfEncoder::AmfEncoder(uint32_t size)
  : data_(new char[size], std::default_delete<char[]>()), size_(size) {}
AmfEncoder::~AmfEncoder() {}

void AmfEncoder::encodeBoolean(int value) {
  if ((size_ - index_) < 2) {
    this->realloc(size_ + 1024);
  }

  data_.get()[index_++] = AMF0DataType::AMF0_BOOLEAN;
  data_.get()[index_++] = value ? 0x01 : 0x00;
}
void AmfEncoder::encodeNumber(double value) {
  if ((size_ - index_) < 9) {
    this->realloc(size_ + 1024);
  }

  data_.get()[index_++] = AMF0DataType::AMF0_NUMBER;

  char *ci = (char *) &value;
  char *co = data_.get();
  co[index_ + 0] = ci[7];
  co[index_ + 1] = ci[6];
  co[index_ + 2] = ci[5];
  co[index_ + 3] = ci[4];
  co[index_ + 4] = ci[3];
  co[index_ + 5] = ci[2];
  co[index_ + 6] = ci[1];
  co[index_ + 7] = ci[0];
  index_ += 8;
}
void AmfEncoder::encodeString(const char *str, int len, bool isObject) {
  if ((int) (size_ - index_) < (len + 1 + 2 + 2)) {
    this->realloc(size_ + len + 5);
  }

  if (len < 65536) {
    if (isObject) {
      data_.get()[index_++] = AMF0DataType::AMF0_STRING;
    }
    encodeInt16(len);
  }
  else {
    if (isObject) {
      data_.get()[index_++] = AMF0DataType::AMF0_LONG_STRING;
    }
    encodeInt32(len);
  }

  memcpy(data_.get() + index_, str, len);
  index_ += len;
}
void AmfEncoder::encodeObjectMap(AmfObjectMap &objMap) {
  if (objMap.empty()) {
    encodeInt8(AMF0DataType::AMF0_NULL);
    return;
  }

  encodeInt8(AMF0DataType::AMF0_OBJECT);
  for (auto [id, obj] : objMap) {
    encodeString(id.c_str(), (int) id.size(), false);
    switch (obj.type) {
    case AMF_NUMBER:
      encodeNumber(obj  .number());
      break;
    case AMF_STRING:
      encodeString(
        obj.string().c_str(), (int) obj.string().size());
      break;
    case AMF_BOOLEAN:
      encodeBoolean(obj.boolean());
      break;
    default: break;
    }
  }

  encodeString("", 0, false);
  encodeInt8(AMF0DataType::AMF0_OBJECT_END);
}
void AmfEncoder::encodeECMA(AmfObjectMap &objMap) {
  encodeInt8(AMF0DataType::AMF0_ECMA_ARRAY);
  encodeInt32(0);

  for (auto [id, obj] : objMap) {
    encodeString(id.c_str(), (int) id.size(), false);
    switch (obj.type) {
    case AMF_NUMBER:
      encodeNumber(obj.number());
      break;
    case AMF_STRING:
      encodeString(
        obj.string().c_str(), (int) obj.string().size());
      break;
    case AMF_BOOLEAN:
      encodeBoolean(obj.boolean());
      break;
    default: break;
    }
  }

  encodeString("", 0, false);
  encodeInt8(AMF0DataType::AMF0_OBJECT_END);
}

void AmfEncoder::encodeInt8(int8_t value) {
  if ((size_ - index_) < 1) {
    this->realloc(size_ + 1024);
  }

  data_.get()[index_++] = value;
}
void AmfEncoder::encodeInt16(int16_t value) {
  if ((size_ - index_) < 2) {
    this->realloc(size_ + 1024);
  }

  writeU16Forward(data_.get() + index_, value);
  index_ += 2;
}
void AmfEncoder::encodeInt24(int32_t value) {
  if ((size_ - index_) < 3) {
    this->realloc(size_ + 1024);
  }

  writeU24Forward(data_.get() + index_, value);
  index_ += 3;
}
void AmfEncoder::encodeInt32(int32_t value) {
  if ((size_ - index_) < 4) {
    this->realloc(size_ + 1024);
  }

  writeU32Forward(data_.get() + index_, value);
  index_ += 4;
}
void AmfEncoder::realloc(uint32_t size) {
  if (size <= size_) {
    return;
  }

  std::shared_ptr<char> data(new char[size], std::default_delete<char[]>());
  ::memcpy(data.get(), data_.get(), index_);
  size_ = size;
  data_ = data;
}
NAMESPACE_END(net)
LY_NAMESPACE_END
