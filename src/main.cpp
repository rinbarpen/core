#include <iostream>
#include <core/util/marcos.h>
#include <core/util/string_util.h>
#include <core/util/logger/Logger.h>
#include <core/util/time/Timestamp.h>
#include <core/util/time/TimestampDuration.h>
#include <core/util/time/Clock.h>

//#include "core/time/Timestamp.cpp"
//#include "core/time/TimestampDuration.cpp"

#include "core/lzma/zip.h"
#include "core/util/Library.h"
#include "core/util/thread/ThreadPool.h"

using namespace ly;


void test_zip()
{
  Zipper zipper;
  // zipper.setZipType(Zipper::_7Z);
  zipper.setPassword("lpout");

#define PREFIX_DIR "E:\\Program\\Projects\\ResourcesManager\\core\\bin\\Debug"
  zipper.compress(PREFIX_DIR, PREFIX_DIR"\\myzip.7z");

  zipper.decompress(PREFIX_DIR "\\myzip.7z", PREFIX_DIR "");

  ThreadPool pool;
  auto compress_r = pool.submit([&](){
    return zipper.compress(PREFIX_DIR, PREFIX_DIR "\\myzip.7z");
  });
  auto decompress_r = pool.submit([&]() {
    return zipper.decompress(PREFIX_DIR "\\myzip.7z", PREFIX_DIR "");
  });

  pool.stop();
}

void test_logger()
{
  using namespace std::literals;
  using namespace ly::literals;

  LOG_ROOT()->setLevel(ly::LogLevel::LTRACE);
  LOG_ROOT()->setFormatter("$DATETIME{%Y-%m-%d %H:%M:%S}"
                           "$CHAR:\t$LOG_NAME$CHAR:[$LOG_LEVEL$CHAR:]"
                           "$CHAR:\t$FUNCTION_NAME"
                           "$CHAR:: $MESSAGE$CHAR:\n");
  LOG_ROOT()->clearAppenders();
  LOG_ROOT()->addAppender(std::make_shared<StdoutLogAppender>());

  LOG_TRACE_FMT("{}", "x"sv * 3);
  LOG_DEBUG_FMT("{}", "x"sv * 3);
  LOG_INFO_FMT("{}", "x"sv * 3);
  LOG_CRITICAL_FMT("{}", "x"sv * 3);
  LOG_WARN_FMT("{}", "x"sv * 3);
  LOG_ERROR_FMT("{}", "x"sv * 3);
  LOG_FATAL_FMT("{}", "x"sv * 3);
}

void test_time()
{
  using namespace std::literals;
  using namespace ly::literals;

  auto t = Clock<T_system_clock>::now();
  auto ts = Timestamp<T_system_clock>();

  LOG_DEBUG_FMT("Current: {}", time2datetime(ts));
  LOG_DEBUG_FMT("Current: {} <=> {}", t, ts.count());

  ILOG_DEBUG(GET_LOGGER("root")) << "{}";

  std::this_thread::sleep_for(1000ms);

  auto duration = TimestampDuration<>::castTo<T_system_clock>(
    ts.current(), Clock<T_system_clock>::tp());
  auto h = TimestampDuration(1h);
  auto s = h.cast<std::chrono::seconds>();
  LOG_DEBUG_FMT(
    "Duration: {} -> {} -> {}", duration.count(), h.count(), s.count());

  std::cout << LogManager::instance()->toYamlString() << std::endl;
}

int main()
{
  // test_zip();
  test_logger();
  test_time();

}
