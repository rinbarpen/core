#pragma once
#include <string>

class Authentication
{
public:
  Authentication(std::string realm, std::string username, std::string password);
  ~Authentication() = default;

  std::string getRealm() const { return realm_; }
  std::string getUsername() const { return username_; }
  std::string getPassword() const { return password_; }

  std::string getNonce();
  std::string getResponse(std::string nonce, std::string cmd, std::string url) const;

private:
  std::string realm_;
  std::string username_;
  std::string password_;
};

