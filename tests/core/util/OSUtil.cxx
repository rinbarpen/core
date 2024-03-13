#include <gtest/gtest.h>
#include <core/util/OSUtil.h>

using namespace ly;

TEST(OSUtil, mk)
{
  os_api::rm("test");
  os_api::mk("test", true);
  os_api::mk("test/a/b", true);
  os_api::mk("test/a/b/x", false);
}
