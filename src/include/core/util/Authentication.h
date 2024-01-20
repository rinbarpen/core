#pragma once
#include <string>
#include <core/util/marcos.h>

LY_NAMESPACE_BEGIN
class Authentication
{
public:
  Authentication(std::string_view realm, std::string_view username, std::string_view password);
  ~Authentication() = default;

  std::string getRealm() const { return realm_; }
  std::string getUsername() const { return username_; }
  std::string getPassword() const { return password_; }

  std::string getNonce();
  std::string getResponse(std::string nonce, std::string cmd, std::string url) const;

  auto hasAuthInfo() const -> bool { return !(realm_.empty() || username_.empty()); }
private:
  std::string realm_;
  std::string username_;
  std::string password_;
};
LY_NAMESPACE_END
