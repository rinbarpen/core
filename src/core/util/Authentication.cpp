#include <core/util/Authentication.h>
// #include <openssl/md5.h>
#include <openssl/rand.h>
#include <core/util/crypt/md5.h>

Authentication::Authentication(std::string realm, std::string username, std::string password) :
  realm_(realm), username_(username), password_(password)
{
}

std::string Authentication::getNonce()
{
  unsigned char random_data[16]; // 16 bytes for MD5
  RAND_bytes(random_data, sizeof(random_data));

  char md5_hash[MD5_DIGEST_LENGTH];
  MD5(random_data, sizeof(random_data), (unsigned char *)md5_hash);

  char md5_hash_hex[MD5_DIGEST_LENGTH * 2 + 1];
  for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
    sprintf(&md5_hash_hex[i * 2], "%02x", (unsigned int)md5_hash[i]);
  }
  md5_hash_hex[MD5_DIGEST_LENGTH * 2] = '\0';

  return std::string(md5_hash_hex);
}

std::string Authentication::getResponse(std::string nonce, std::string cmd, std::string url) const
{
  //md5(md5(<username>: <realm> : <password>) :<nonce> : md5(<cmd>:<url>))
  std::string md5_hash1;
  std::string md5_hash2;
  std::string md5_hash3;
  std::string s1 = username_ + ":" + realm_ + ":" + password_;
  std::string s2 = cmd + ":" + url;
  MD5((unsigned char*)s1.c_str(), s1.size(), (unsigned char *)md5_hash1.data());
  MD5((unsigned char *)s2.c_str(), s2.size(), (unsigned char *)md5_hash2.data());
  std::string s3 = md5_hash1 + ":" + nonce + ":" + md5_hash2;
  MD5((unsigned char *)s3.c_str(), s3.size(), (unsigned char *)md5_hash3.data());

  return md5_hash3;
}
