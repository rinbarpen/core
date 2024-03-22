#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(net)
enum AMF0DataType {
  AMF0_NUMBER = 0,
  AMF0_BOOLEAN,
  AMF0_STRING,
  AMF0_OBJECT,
  AMF0_MOVIECLIP, /* reserved, not used */
  AMF0_NULL,
  AMF0_UNDEFINED,
  AMF0_REFERENCE,
  AMF0_ECMA_ARRAY,
  AMF0_OBJECT_END,
  AMF0_STRICT_ARRAY,
  AMF0_DATE,
  AMF0_LONG_STRING,
  AMF0_UNSUPPORTED,
  AMF0_RECORDSET, /* reserved, not used */
  AMF0_XML_DOC,
  AMF0_TYPED_OBJECT,
  AMF0_AVMPLUS, /* switch to AMF3 */
  AMF0_INVALID = 0xFF
};
enum AMF3DataType {
  AMF3_UNDEFINED = 0,
  AMF3_NULL,
  AMF3_FALSE,
  AMF3_TRUE,
  AMF3_INTEGER,
  AMF3_DOUBLE,
  AMF3_STRING,
  AMF3_XML_DOC,
  AMF3_DATE,
  AMF3_ARRAY,
  AMF3_OBJECT,
  AMF3_XML,
  AMF3_BYTE_ARRAY
};
enum AmfObjectType {
  AMF_NONE,
  AMF_NUMBER,
  AMF_BOOLEAN,
  AMF_STRING,
};

struct AmfObject
{
  AmfObjectType type;
  std::tuple<std::string, double, bool> data;

  AmfObject()
    : type(AmfObjectType::AMF_NONE)
  {}
  AmfObject(std::string str)
    : type(AmfObjectType::AMF_STRING)
  {
    std::get<std::string>(this->data) = str;
  }
  AmfObject(double number)
    : type(AmfObjectType::AMF_NUMBER)
  {
    std::get<double>(this->data) = number;
  }
  AmfObject(bool boolean)
    : type(AmfObjectType::AMF_BOOLEAN)
  {
    std::get<bool>(this->data) = boolean;
  }

  std::string& string() { return std::get<std::string>(this->data); }
  double& number() { return std::get<double>(this->data); }
  bool& boolean() { return std::get<bool>(this->data); }

  std::string string() const { return std::get<std::string>(this->data); }
  double number() const { return std::get<double>(this->data); }
  bool boolean() const { return std::get<bool>(this->data); }

  void clear() {
    type = AmfObjectType::AMF_NONE;
    data = std::make_tuple("", 0, false);
  }
};

using AmfObjectMap = std::unordered_map<std::string, AmfObject>;
class AmfDecoder
{
public:
  /**
   * @brief
   *
   * @param data
   * @param size
   * @param n The number of decoding
   * @return int
   */
  int decode(const char *data, int size, int n = -1);

  void reset() {
    obj_.clear();
    obj_map_.clear();
  }

  std::string getString() const { return obj_.string(); }
  double getNumber() const { return obj_.number(); }
  bool getBoolean() const { return obj_.boolean(); }

  bool hasObject(std::string key) const {
    return (obj_map_.find(key) != obj_map_.end());
  }

  AmfObject getObject(std::string key) { return obj_map_[key]; }
  AmfObject getObject() { return obj_; }
  AmfObjectMap getObjectMap() { return obj_map_; }

private:
  static int decodeBoolean(const char *data, int size, bool &amf_boolean);
  static int decodeNumber(const char *data, int size, double &amf_number);
  static int decodeString(const char *data, int size, std::string &amf_string);
  static int decodeObject(const char *data, int size, AmfObjectMap &amf_obj_map);
  static uint16_t decodeInt16(const char *data, int size);
  static uint32_t decodeInt24(const char *data, int size);
  static uint32_t decodeInt32(const char *data, int size);

  AmfObject obj_;
  AmfObjectMap obj_map_;
};

class AmfEncoder
{
public:
  AmfEncoder(uint32_t size = 1024);
  virtual ~AmfEncoder();

  void encodeString(const char *str, int len, bool isObject = true);
  void encodeNumber(double value);
  void encodeBoolean(int value);
  void encodeObjectMap(AmfObjectMap &objMap);
  void encodeECMA(AmfObjectMap &objMap);

  void reset() { index_ = 0; }
  std::shared_ptr<char> data() { return data_; }
  uint32_t size() const { return index_; }

private:
  void encodeInt8(int8_t value);
  void encodeInt16(int16_t value);
  void encodeInt24(int32_t value);
  void encodeInt32(int32_t value);
  void realloc(uint32_t size);

  std::shared_ptr<char> data_;
  uint32_t size_{0};
  uint32_t index_{0};
};
NAMESPACE_END(net)
LY_NAMESPACE_END
