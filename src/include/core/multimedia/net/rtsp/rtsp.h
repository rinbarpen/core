#pragma once
#include <cstdint>
#include <cstdio>
#include <memory>

#include "core/multimedia/net/media.h"
#include "core/util/Authentication.h"
#include "core/util/logger/Logger.h"
#include "core/util/marcos.h"
#include <core/multimedia/net/MediaSession.h>
#include <core/multimedia/net/MediaSource.h>
#include <string>
#include <string_view>

LY_NAMESPACE_BEGIN

NAMESPACE_BEGIN(net)

struct RtspUrlInfo
{
  std::string url;
  std::string ip;
  uint16_t port;
  std::string suffix;
};

class Rtsp : public std::enable_shared_from_this<Rtsp>
{
  friend class RtspConnection;
public:
  SHARED_REG(Rtsp);

  Rtsp() = default;
  virtual ~Rtsp() = default;

  virtual void setAuthConfig(std::string_view realm, std::string_view username, std::string_view passwd)
  {
    auth_ = Authentication(realm, username, passwd);
  }
  auto auth() const -> const Authentication& { return auth_; }
  auto hasAuthInfo() const -> bool { return auth_.hasAuthInfo(); }

  // sdp session name
  virtual void setVersion(std::string_view version) { version_ = version; }
  virtual auto version() const -> std::string { return version_; }
  virtual auto rtspUrl() const -> std::string { return rtsp_url_info_.url; }

  virtual auto parseRtspUrl(std::string_view url) -> bool
  {
    static auto rtsp_logger = GET_LOGGER("multimedia");
    char ip[100]{};
    char suffix[100]{};
    uint16_t port{};
    char username[100]{};
    char password[100]{};

    // skip rtsp://
    // match rtsp://{ip:[port]}/{suffix}
    if (3 == std::sscanf(url.data() + 7, "%[^:]:%hu/%s", ip, &port, suffix)) {
      ILOG_DEBUG_FMT(rtsp_logger, "{} matches rtsp://{ip:[port]}/{suffix}", url);
    }
    // match rtsp://{ip}/{suffix}
    else if (2 == std::sscanf(url.data(), "%[^/]/%s", ip, suffix)) {
      port = 554;
      ILOG_DEBUG_FMT(rtsp_logger, "{} matches rtsp://{ip}/{suffix}", url);
    }
    // match rtsp://[username:password@]{ip[:port]}/{suffix}
    else if (5 == std::sscanf(url.data() + 7, "%[^:]:%[^@]@%[^:]:%hu/%s", username, password, ip, &port, suffix)) {
      ILOG_DEBUG_FMT(rtsp_logger, "{} matches rtsp://[username:password@]{ip[:port]}/{suffix}", url);
    }
    // match rtsp://[username:password@]{ip}/{suffix}
    else if (4 == std::sscanf(url.data() + 7, "%[^:]:%[^@]@%[^/]/%s", username, password, ip, suffix)) {
      port = 554;
      ILOG_DEBUG_FMT(rtsp_logger, "{} matches rtsp://[username:password@]{ip}/{suffix}", url);
    }
    else {
      ILOG_WARN_FMT(rtsp_logger, "RTSP url[{}] is INVALID", url);
      return false;
    }

    rtsp_url_info_.ip = ip;
    rtsp_url_info_.port = port;
    rtsp_url_info_.suffix = suffix;
    rtsp_url_info_.url = url;

    return true;
  }

protected:
  virtual auto lookMediaSession(std::string_view suffix) -> MediaSession::ptr { return nullptr; }
  virtual auto lookMediaSession(MediaSessionId session_id) -> MediaSession::ptr { return nullptr; }
  // virtual auto lookMediaSession(std::variant<std::string_view, MediaSessionId> suffix_or_session_id) -> MediaSession::ptr;

  Authentication auth_{"", "", ""};
  std::string version_;
  struct RtspUrlInfo rtsp_url_info_;
};

NAMESPACE_END(net)
LY_NAMESPACE_END
