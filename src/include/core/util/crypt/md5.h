#pragma once

#include <string_view>
#include <string>

#include <core/util/marcos.h>

#ifdef ENABLE_OPENSSL
namespace openssl {
extern "C" {
# include <openssl/md5.h>
# include <openssl/rand.h>
}
}
#else
# error "Need to use openssl!"
#endif

LY_NAMESPACE_BEGIN
class XHash
{
public:
  static std::string md5(std::string_view raw)
  {
    unsigned char digest[MD5_DIGEST_LENGTH];
    openssl::MD5(reinterpret_cast<const unsigned char*>(raw.data()), raw.size(), digest);

    // 将 digest 转换为字符串表示
    char hexBuffer[2 * MD5_DIGEST_LENGTH];
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
      std::snprintf(hexBuffer + 2 * i, 2, "%02x", digest[i]);
    }

    return std::string(hexBuffer, 2 * MD5_DIGEST_LENGTH);
  }

  static std::string random(size_t nbytes)
  {
    char *buffer = new char[nbytes];
    openssl::RAND_bytes(reinterpret_cast<unsigned char *>(buffer), nbytes);
    std::string buf = std::string(buffer, nbytes);
    delete[] buffer;
    return buf;
  }
};

LY_NAMESPACE_END
