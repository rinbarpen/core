#pragma once

#include <string_view>
#include <string>

#ifdef ENABLE_OPENSSL
#include <openssl/md5.h>
#else
#error "Need to use openssl!"
#endif
namespace ly
{

class XHash
{
public:
  static auto md5(std::string_view raw) -> std::string
  {
#ifdef ENABLE_OPENSSL
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(raw.data()), raw.size(), digest);

    // 将 digest 转换为字符串表示
    char hexBuffer[2 * MD5_DIGEST_LENGTH + 1];
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        std::snprintf(hexBuffer + 2 * i, 2, "%02x", digest[i]);
    }

    return std::string(hexBuffer, 2 * MD5_DIGEST_LENGTH);
#else
    return "";
#endif
  }
};

}  // namespace ly
