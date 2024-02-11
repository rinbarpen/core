#pragma once

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include <core/util/StringUtil.h>
#include <core/util/marcos.h>

#ifdef __LINUX__
# include <dirent.h>
# include <fcntl.h>
# include <signal.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>
#elif defined(__WIN__)
# include <direct.h>
#endif

LY_NAMESPACE_BEGIN
NAMESPACE_BEGIN(os_api)
namespace detail
{
static int lstat(const char *file, struct stat *st = nullptr) {
  struct stat lst;
  int r = ::lstat(file, &lst);
  if (st) *st = lst;
  return r;
}
static bool touch(const std::string &filename, int oflag = 0644) {
  int fd;
  if ((fd = ::open(filename.c_str(), O_CREAT | O_WRONLY, oflag)) == 0) {
    ::close(fd);
    return true;
  }
  return false;
}
static int mkdir(const std::string &dirname) {
  if (::access(dirname.c_str(), F_OK) == 0) {
    return 0;
  }
#ifdef __LINUX__
  return ::mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#elif defined(__WIN__)
  return ::_mkdir(dirname.c_str());
#endif
  return -1;
}
static void list_all_file(std::vector<std::string> &files,
  const std::string &path, const std::string &suffix) {
  if (::access(path.data(), 0) != 0) return;

  DIR *dir = ::opendir(path.data());
  if (dir == nullptr) return;

  struct dirent *dp = nullptr;
  while ((dp = ::readdir(dir)) != nullptr) {
    if (dp->d_type == DT_DIR) {
      if (!std::strcmp(dp->d_name, ".") || !std::strcmp(dp->d_name, "..")) {
        continue;
      }
      detail::list_all_file(files, path + "/" + dp->d_name, suffix);
    }
    else if (dp->d_type == DT_REG) {
      std::string filename(dp->d_name);
      if (suffix.empty()) {
        files.push_back(path + "/" + filename);
        continue;
      }
      if (filename.size() < suffix.size()) {
        continue;
      }
      if (filename.substr(filename.length() - suffix.size()) == suffix) {
        files.push_back(path + "/" + filename);
      }
    }
  }
  ::closedir(dir);
}
}  // namespace detail

static bool exist(const std::string &path, bool is_dir) {
  struct stat st;
  if (detail::lstat(path.c_str(), &st) == 0) {
    if (is_dir && !S_ISDIR(st.st_mode)) {
      return false;
    }
    return true;
  }
  return false;
}
static bool touch(const std::string &filename, int oflag = 0644) {
  if (exist(filename.c_str(), false)) {
    return false;
  }

  char *path = ::strdup(filename.c_str());
  char *ptr = ::strchr(path + 1, '/');
  for (; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
    *ptr = '\0';
    if (detail::mkdir(path) != 0) {
      ::free(path);
      return false;
    }
  }
  if (ptr != nullptr) {
    ::free(path);
    return false;
  }
  else if (detail::touch(filename, oflag)) {
    ::free(path);
    return false;
  }
  ::free(path);
  return true;
}
static bool mkdir(const std::string &dirname) {
  if (exist(dirname.c_str(), true) == 0) {
    return true;
  }

  char *path = ::strdup(dirname.c_str());
  char *ptr = ::strchr(path + 1, '/');
  for (; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
    *ptr = '\0';
    if (detail::mkdir(path) != 0) {
      ::free(path);
      return false;
    }
  }
  if (ptr != nullptr) {
    ::free(path);
    return false;
  }
  else if (detail::mkdir(path) != 0) {
    ::free(path);
    return false;
  }
  ::free(path);
  return true;
}
static bool mk(const std::string &path, bool is_dir) {
  if (is_dir) {
    return mkdir(path);
  }

  return touch(path);
}

static std::vector<std::string> list_all_file(
  const std::string &path, const std::string &suffix = "") {
  std::vector<std::string> files;
  if (::access(path.data(), 0) != 0) return files;

  DIR *dir = ::opendir(path.data());
  if (dir == nullptr) return files;

  struct dirent *dp = nullptr;
  while ((dp = ::readdir(dir)) != nullptr) {
    if (dp->d_type == DT_DIR) {
      if (!std::strcmp(dp->d_name, ".") || !std::strcmp(dp->d_name, "..")) {
        continue;
      }
      detail::list_all_file(files, path + "/" + dp->d_name, suffix);
    }
    else if (dp->d_type == DT_REG) {
      std::string filename(dp->d_name);
      if (suffix.empty()) {
        files.push_back(path + "/" + filename);
        continue;
      }
      if (filename.size() < suffix.size()) {
        continue;
      }
      if (filename.substr(filename.length() - suffix.size()) == suffix) {
        files.push_back(path + "/" + filename);
      }
    }
  }
  ::closedir(dir);
  return files;
}
static bool is_executing_file(const std::string &pidfile) {
  if (!exist(pidfile, false)) {
    return false;
  }

  std::ifstream ifs(pidfile);
  std::string line;
  if (!ifs || !std::getline(ifs, line)) {
    return false;
  }
  if (line.empty()) {
    return false;
  }

  pid_t pid = atoi(line.c_str());
  if (pid <= 1) {
    return false;
  }

  if (::kill(pid, 0) != 0) {
    return false;
  }
  return true;
}
static bool unlink(const std::string &filename) {
  if (exist(filename, false)) return true;

  return ::unlink(filename.c_str()) == 0;
}
static bool rm(const std::string &path) {
  struct stat st;
  if (detail::lstat(path.c_str(), &st)) {
    return true;
  }
  // symlink file
  if (!(st.st_mode & S_IFDIR)) {
    return unlink(path);
  }

  DIR *dir = ::opendir(path.c_str());
  if (nullptr == dir) {
    return false;
  }

  bool r = false;
  struct dirent *dp = nullptr;
  while ((dp = readdir(dir)) != nullptr) {
    if (!std::strcmp(dp->d_name, ".") || !std::strcmp(dp->d_name, "..")) {
      continue;
    }
    std::string dirname = path + "/" + dp->d_name;
    r = rm(dirname);
  }
  ::closedir(dir);
  if (0 == ::rmdir(path.c_str())) {
    r = true;
  }
  return r;
}
static bool move(const std::string &from, const std::string &to) {
  if (!rm(to)) {
    return false;
  }
  return 0 == ::rename(from.c_str(), to.c_str());
}
static bool realpath(const std::string &path, std::string &rpath) {
  if (0 != detail::lstat(path.c_str())) {
    return false;
  }

  char *p = ::realpath(path.c_str(), nullptr);
  if (nullptr == p) {
    return false;
  }

  std::string(p).swap(rpath);
  ::free(p);
  return true;
}
static bool symlink(const std::string &from, const std::string &to) {
  if (!rm(to)) {
    return false;
  }
  return 0 == ::symlink(from.c_str(), to.c_str());
}
/**
 * @brief
 *
 * @param filename
 * @return std::string
 * @note The file must exist
 *
 */
static std::string dirname(const std::string &filename) {
  if (filename.empty()) {
    return ".";
  }

  auto pos = filename.rfind('/');
  if (pos == 0) {
    return "/";
  }
  else if (pos == std::string::npos) {
    return ".";
  }
  return filename.substr(0, pos);
}
static std::string basename(const std::string &filename) {
  if (filename.empty()) {
    return filename;
  }

  auto pos = filename.rfind('/');
  if (pos == std::string::npos) {
    return filename;
  }
  return filename.substr(pos + 1);
}

NAMESPACE_END(os_api)
LY_NAMESPACE_END
