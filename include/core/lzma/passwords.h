#pragma once

#include <fstream>
#include <vector>

#include <core/util/string_util.h>

LY_NAMESPACE_BEGIN

static std::vector<std::string> g_passwords;

static void read_passwords() 
{
  std::ifstream ifs("passwd");
  std::string buf;
  ifs >> buf;

  g_passwords = string_util::split(buf, "\n");
}

static void write_passwords() 
{
  std::ofstream ofs("passwd");
  std::string buf;
  for (auto &passwd : g_passwords)
  {
    buf += passwd + "\n";
  }
  ofs << buf;
  ofs.flush();
}

LY_NAMESPACE_END
