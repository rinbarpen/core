#pragma once

#include <core/util/marcos.h>
#include <cstdint>

LY_NAMESPACE_BEGIN
class ByteConverter
{
public:
  enum {
    T_INT8 = 1,
    T_INT16 = 2,
    T_INT24 = 3,
    T_INT32 = 4,
  };

  static uint8_t readU8Forward(const char *p)
  {
    uint8_t val{0};
    val |= p[0] << 8;
    return val;
  }
  static uint8_t readU8Reverse(const char *p)
  {
    uint8_t val{0};
    val |= p[0] << 8;
    return val;
  }
  static uint16_t readU16Forward(const char *p)
  {
    uint16_t val{0};
    val |= p[0] << 8;
    val |= p[1] << 0;
    return val;
  }
  static uint16_t readU16Reverse(const char *p)
  {
    uint16_t val{0};
    val |= p[1] << 8;
    val |= p[0] << 0;
    return val;
  }
  static uint32_t readU24Forward(const char *p)
  {
    uint32_t val{0};
    val |= p[0] << 16;
    val |= p[1] << 8;
    val |= p[2] << 0;
    return val;
  }
  static uint32_t readU24Reverse(const char *p)
  {
    uint32_t val{0};
    val |= p[2] << 16;
    val |= p[1] << 8;
    val |= p[0] << 0;
    return val;
  }
  static uint32_t readU32Forward(const char *p)
  {
    uint32_t val{0};
    val |= p[0] << 24;
    val |= p[1] << 16;
    val |= p[2] << 8;
    val |= p[3] << 0;
    return val;
  }
  static uint32_t readU32Reverse(const char *p)
  {
    uint32_t val{0};
    val |= p[3] << 24;
    val |= p[2] << 16;
    val |= p[1] << 8;
    val |= p[0] << 0;
    return val;
  }
  static uint32_t readForward(const char *p, int intType)
  {
    uint32_t val{0};
    for (int i = 0; i < intType; ++i) {
      val |= p[i] << ((intType - i - 1) * 8);
    }
    return val;
  }
  static uint32_t readReverse(const char *p, int intType)
  {
    uint32_t val{0};
    for (int i = 0; i < intType; ++i) {
      val |= p[i] << (i * 8);
    }
    return val;
  }
  static uint32_t read(bool isForward, const char *p, int intType)
  {
    if (isForward) return readForward(p, intType);
    else return readReverse(p, intType);
  }

  static void writeU8Forward(char *p, uint8_t value)
  {
    p[0] = value & 0xFF;
  }
  static void writeU8Reverse(char *p, uint8_t value)
  {
    p[0] = value & 0xFF;
  }
  static void writeU16Forward(char *p, uint16_t value)
  {
    p[0] = value >> 8;
    p[1] = value & 0xFF;
  }
  static void writeU16Reverse(char *p, uint16_t value)
  {
    p[1] = value >> 8;
    p[0] = value & 0xFF;
  }
  static void writeU24Forward(char *p, uint32_t value)
  {
    p[0] = value >> 16;
    p[1] = value >> 8;
    p[2] = value & 0xFF;
  }
  static void writeU24Reverse(char *p, uint32_t value)
  {
    p[2] = value >> 16;
    p[1] = value >> 8;
    p[0] = value & 0xFF;
  }
  static void writeU32Forward(char *p, uint32_t value)
  {
    p[0] = value >> 24;
    p[1] = value >> 16;
    p[2] = value >> 8;
    p[3] = value & 0xFF;
  }
  static void writeU32Reverse(char *p, uint32_t value)
  {
    p[3] = value >> 24;
    p[2] = value >> 16;
    p[1] = value >> 8;
    p[0] = value & 0xFF;
  }
  static void writeForward(char *p, uint32_t value, int intType)
  {
    for (int i = 0; i < intType; ++i) {
      p[i] |= ((value >> ((intType - i - 1) * 8)) & 0xFF);
    }
  }
  static void writeReverse(char *p, uint32_t value, int intType)
  {
    for (int i = 0; i < intType; ++i) {
      p[i] |= ((value >> (i * 8)) & 0xFF);
    }
  }
  static void write(bool isForward, char *p, uint32_t value, int intType)
  {
    if (isForward) writeForward(p, value, intType);
    else writeReverse(p, value, intType);
  }

  static void reverse(char *p, uint32_t nbytes)
  {
    uint32_t mid = nbytes / 2;
    for (uint32_t i = 0; i < mid; ++i) {
      std::swap(p[i], p[nbytes - i - 1]);
    }
  }
};
LY_NAMESPACE_END
