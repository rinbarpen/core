#include <core/args/ArgsParser.h>
#include <cstdio>

using namespace ly;

void calc_exe(int argc, const char *argv[])
{
  ArgsParser args;
  args << Option("add", OptionValue{}, "add a number", true)
       << Option("minus", OptionValue{}, "minus a number", false);
  args.print(std::cout);
  args.parse(argc, argv);
  args.info(std::cout);
  args.verbose(std::cout);

  auto x = args.match("add");
}

int main()
{
  const char *argv[] = {"calc_exe", "-add=200", "-minus=300", NULL};
  calc_exe(3, argv);
}
