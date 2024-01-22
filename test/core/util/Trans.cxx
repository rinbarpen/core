#include <gtest/gtest.h>
#include <core/util/Trans.h>
#include <core/util/string_util.h>

class QString
{
public:
  QString() : x_("") {}
  QString(std::string x) : x_(x) {}
  static QString fromStdString(const std::string &s) {return{s};}
  static QString fromStdString(std::string &&s) {return{s};}

  std::string toStdString() const { return x_; }


private:
  std::string x_;
};


TEST(Trans, Trans)
{
  QString x("hello, world!");
  auto y = ly::Translator::qstrCall(x, &ly::string_util::split, ",");

  EXPECT_STREQ("hello", y[0]);
  EXPECT_STREQ(" world!", y[1]);
}
