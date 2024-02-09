#pragma once

#include "core/util/marcos.h"
#include "core/util/string_util.h"
#include <string>
#ifdef __LINUX__
#include <fcntl.h>
#include <unistd.h>
# include <sys/stat.h>
#elif defined(__WIN__)
# include <direct.h>
#endif

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(os_api)
static bool rm(const char *filename)
{
#ifdef __LINUX__
#elif defined(__WIN__)
#endif
  if (::remove(filename) == 0) {
    return true;
  }
  return false;
}
static bool touch(const char *filename, int oflag = 0644)
{
  int fd;

  if ((fd = ::open(filename, O_CREAT | O_WRONLY, oflag)) == 0) {
    ::close(fd);
#ifdef __WIN__
    struct _utimbuf times;
    times.actime = times.modtime = time(NULL);
    if (_utime(file_name, &times) != 0) {
      rm(filename);
      return false;
    }
#endif
    return true;
  }
  return false;
}
static bool mkdir(const char *dir_path, __mode_t mode = 0777)
{
#ifdef __LINUX__
  if (::mkdir(dir_path, mode) == 0) {
    return true;
  }
#elif defined(__WIN__)
  if (::_mkdir(directory_name) == 0) {
    return true;
  }
#endif
  return false;
}
static bool exist(const char *path, bool is_dir = true)
{
#ifdef __LINUX__
  struct stat st;
  if (::stat(path, &st) == 0) {
    if (is_dir) {
      if (!S_ISDIR(st.st_mode)) {
        return false;
      }
    }
    return true;
  }
#elif defined(__WIN__)
  if (::_access(path, 0) == 0) {
    return true;
  }
#endif
  return false;
}

// advance functions
static void mk(const char *path, bool is_dir = true)
{
  auto l = string_util::split(path, "/");
  if (l.size() == 1) {
    if (is_dir) {
      mkdir(path);
    } else {
      touch(path);
    }
    return;
  }

  std::string current_path = l.front();
  for (size_t i = 1; i < l.size(); ++i) {
    current_path += "/" + l[i];
    if (i == l.size() - 1) {
      if (is_dir) {
        mkdir(current_path.c_str());
      } else {
        touch(current_path.c_str());
      }
      return;
    }
    if (!exist(current_path.c_str(), true)) {
      mkdir(current_path.c_str());
    }
  }
}


NAMESPACE_END(os_api)
LY_NAMESPACE_END
