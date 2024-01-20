#include <core/util/Authentication.h>
#include <core/util/crypt/md5.h>


LY_NAMESPACE_BEGIN
Authentication::Authentication(
  std::string_view realm, std::string_view username, std::string_view password)
  : realm_(realm), username_(username), password_(password) {}

std::string Authentication::getNonce() {
  return XHash::md5(XHash::random(MD5_DIGEST_LENGTH));
}

std::string Authentication::getResponse(
  std::string nonce, std::string cmd, std::string url) const {
  // md5(md5(<username>: <realm> : <password>) :<nonce> : md5(<cmd>:<url>))
  return XHash::md5(XHash::md5(username_ + ":" + realm_ + ":" + password_) + ":"
                    + nonce + ":" + XHash::md5(cmd + ":" + url));
}
LY_NAMESPACE_END
