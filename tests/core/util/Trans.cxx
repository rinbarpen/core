#include <gtest/gtest.h>

#include <functional>
#include <string_view>
#include <core/util/StringUtil.h>
#include <core/util/Trans.h>

using namespace ly;
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
  Translator::qstrCallView(x, [](std::string_view s){
    std::printf("%s\n", s.data());
  });
  Translator::qstrCallConstRef(x, [](const std::string &s){
    std::printf("%s\n", s.data());
  });
  // auto z = string_util::split(x.toStdString(), ",");
  // auto z = Translator::qstrCall(x, &string_util::split, ",");
  // EXPECT_STREQ("hello", z[0].c_str());
  // EXPECT_STREQ(" world!", z[1].c_str());
}
