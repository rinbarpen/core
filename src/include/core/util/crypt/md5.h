#pragma once

#include <string_view>
#include <string>

#include <core/util/marcos.h>

#ifdef ENABLE_OPENSSL
#include <openssl/md5.h>
#include <openssl/rand.h>
#else
#error "Need to use openssl!"
#endif

LY_NAMESPACE_BEGIN
class XHash
{
public:
  static auto md5(std::string_view raw) -> std::string
  {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(raw.data()), raw.size(), digest);

    // 将 digest 转换为字符串表示
    char hexBuffer[2 * MD5_DIGEST_LENGTH];
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
      std::snprintf(hexBuffer + 2 * i, 2, "%02x", digest[i]);
    }

    return std::string(hexBuffer, 2 * MD5_DIGEST_LENGTH);
  }

  static auto random(size_t nbytes) -> std::string
  {
    char *buffer = new char[nbytes];
    ::RAND_bytes(reinterpret_cast<unsigned char *>(buffer), nbytes);
    std::string buf = std::string(buffer, nbytes);
    delete[] buffer;
    return buf;
  }
};

LY_NAMESPACE_END
