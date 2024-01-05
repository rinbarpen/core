/**
***  LOG_ROOT
***  GET_LOGGER
***  Ixx_ -> with my logger
***  xx   -> with root logger
***  xx_FMT -> print by using format if having libfmt, otherwise c-style
***
**/

#pragma once

#include <chrono>
#include <cstdarg>
#include <fstream>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <core/util/marcos.h>

#include <fmt/core.h>
#include <yaml-cpp/yaml.h>

#define __LogEventGen(level, timestamp) \
  std::make_shared<::ly::LogEvent>(level, __FILE__, __LINE__, __PRETTY_FUNCTION__, timestamp)

#define __LogEventGen2(level) \
  std::make_shared<::ly::LogEvent>(level, __FILE__, __LINE__, __PRETTY_FUNCTION__, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())

#define __LogEventWrapperGen(pLogger, level, timestamp) \
  std::make_shared<::ly::LogEventWrapper>(__LogEventGen(level, timestamp), pLogger)

#define __LogEventWrapperGen2(pLogger, level) \
  std::make_shared<::ly::LogEventWrapper>(__LogEventGen2(level), pLogger)

#define __LOG_STREAM(pLogger, level) \
  __LogEventWrapperGen2(pLogger, level)->getSS()

#define LOG_ROOT() ::ly::LogManager::instance()->getRoot()
#define GET_LOGGER(name) ::ly::LogManager::instance()->getLogger(name)

#define ILOG_TRACE_FMT(pLogger, fmt, ...) \
  __LogEventWrapperGen2(pLogger, ::ly::LogLevel::LTRACE)->getEvent()->format(fmt, ##__VA_ARGS__)
#define ILOG_DEBUG_FMT(pLogger, fmt, ...) \
  __LogEventWrapperGen2(pLogger, ::ly::LogLevel::LDEBUG)->getEvent()->format(fmt, ##__VA_ARGS__)
#define ILOG_INFO_FMT(pLogger, fmt, ...) \
  __LogEventWrapperGen2(pLogger, ::ly::LogLevel::LINFO)->getEvent()->format(fmt, ##__VA_ARGS__)
#define ILOG_CRITICAL_FMT(pLogger, fmt, ...) \
  __LogEventWrapperGen2(pLogger, ::ly::LogLevel::LCRITICAL)->getEvent()->format(fmt, ##__VA_ARGS__)
#define ILOG_WARN_FMT(pLogger, fmt, ...) \
  __LogEventWrapperGen2(pLogger, ::ly::LogLevel::LWARN)->getEvent()->format(fmt, ##__VA_ARGS__)
#define ILOG_ERROR_FMT(pLogger, fmt, ...) \
  __LogEventWrapperGen2(pLogger, ::ly::LogLevel::LERROR)->getEvent()->format(fmt, ##__VA_ARGS__)
#define ILOG_FATAL_FMT(pLogger, fmt, ...) \
  __LogEventWrapperGen2(pLogger, ::ly::LogLevel::LFATAL)->getEvent()->format(fmt, ##__VA_ARGS__)


#define ILOG_TRACE(pLogger) \
  __LOG_STREAM(pLogger, ::ly::LogLevel::LTRACE)
#define ILOG_DEBUG(pLogger) \
  __LOG_STREAM(pLogger, ::ly::LogLevel::LDEBUG)
#define ILOG_INFO(pLogger) \
  __LOG_STREAM(pLogger, ::ly::LogLevel::LINFO)
#define ILOG_CRITICAL(pLogger) \
  __LOG_STREAM(pLogger, ::ly::LogLevel::LCRITICAL)
#define ILOG_WARN(pLogger) \
  __LOG_STREAM(pLogger, ::ly::LogLevel::LWARN)
#define ILOG_ERROR(pLogger) \
  __LOG_STREAM(pLogger, ::ly::LogLevel::LERROR)
#define ILOG_FATAL(pLogger) \
  __LOG_STREAM(pLogger, ::ly::LogLevel::LFATAL)


#define LOG_TRACE_FMT(fmt, ...) \
  __LogEventWrapperGen2(LOG_ROOT(), ::ly::LogLevel::LTRACE)->getEvent()->format(fmt, ##__VA_ARGS__)
#define LOG_DEBUG_FMT(fmt, ...) \
  __LogEventWrapperGen2(LOG_ROOT(), ::ly::LogLevel::LDEBUG)->getEvent()->format(fmt, ##__VA_ARGS__)
#define LOG_INFO_FMT(fmt, ...) \
  __LogEventWrapperGen2(LOG_ROOT(), ::ly::LogLevel::LINFO)->getEvent()->format(fmt, ##__VA_ARGS__)
#define LOG_CRITICAL_FMT(fmt, ...) \
  __LogEventWrapperGen2(LOG_ROOT(), ::ly::LogLevel::LCRITICAL)->getEvent()->format(fmt, ##__VA_ARGS__)
#define LOG_WARN_FMT(fmt, ...) \
  __LogEventWrapperGen2(LOG_ROOT(), ::ly::LogLevel::LWARN)->getEvent()->format(fmt, ##__VA_ARGS__)
#define LOG_ERROR_FMT(fmt, ...) \
  __LogEventWrapperGen2(LOG_ROOT(), ::ly::LogLevel::LERROR)->getEvent()->format(fmt, ##__VA_ARGS__)
#define LOG_FATAL_FMT(fmt, ...) \
  __LogEventWrapperGen2(LOG_ROOT(), ::ly::LogLevel::LFATAL)->getEvent()->format(fmt, ##__VA_ARGS__)


#define LOG_TRACE() \
  __LOG_STREAM(LOG_ROOT(), ::ly::LogLevel::LTRACE)
#define LOG_DEBUG() \
  __LOG_STREAM(LOG_ROOT(), ::ly::LogLevel::LDEBUG)
#define LOG_INFO() \
  __LOG_STREAM(LOG_ROOT(), ::ly::LogLevel::LINFO)
#define LOG_CRITICAL() \
  __LOG_STREAM(LOG_ROOT(), ::ly::LogLevel::LCRITICAL)
#define LOG_WARN() \
  __LOG_STREAM(LOG_ROOT(), ::ly::LogLevel::LWARN)
#define LOG_ERROR() \
  __LOG_STREAM(LOG_ROOT(), ::ly::LogLevel::LERROR)
#define LOG_FATAL() \
  __LOG_STREAM(LOG_ROOT(), ::ly::LogLevel::LFATAL)


LY_NAMESPACE_BEGIN

// 2023-03-01    root[DEBUG]    Logger.h:121    main    thanks for using lycore
inline constexpr const char *kDefaultFormatPattern =
  "$DATETIME{%Y-%m-%d %H:%M:%S}"
  "$CHAR:\t$LOG_NAME$CHAR:[$LOG_LEVEL$CHAR:]"
  "$CHAR:\t$FILENAME$CHAR::$LINE"
  "$CHAR:\t$FUNCTION_NAME"
  "$CHAR:\t$MESSAGE$CHAR:\n";
// 2023-03-01    Logger.h:121    [DEBUG]    main    thanks for using lycore
inline constexpr const char *kCommonFormatPattern =
  "$DATETIME{%Y-%m-%d %H:%M:%S}"
  "$CHAR:\t$FILENAME$CHAR::$LINE"
  "$CHAR:\t$CHAR:[$LOG_LEVEL$CHAR:]"
  "$CHAR:\t$FUNCTION_NAME"
  "$CHAR:\t$MESSAGE$CHAR:\n";
// 2023-03-01    [DEBUG]    main    thanks for using lycore
inline constexpr const char *kCommon2FormatPattern =
  "$DATETIME{%Y-%m-%d %H:%M:%S}"
  "$CHAR:\t$CHAR:[$LOG_LEVEL$CHAR:]"
  "$CHAR:\t$FUNCTION_NAME"
  "$CHAR:\t$MESSAGE$CHAR:\n";
// 2023-03-01    [DEBUG]    thanks for using lycore
inline constexpr const char *kBriefFormatPattern =
  "$DATETIME{%Y-%m-%d %H:%M:%S}"
  "$CHAR:\t$CHAR:[$LOG_LEVEL$CHAR:]"
  "$CHAR:\t$MESSAGE$CHAR:\n";


/**
 * $MESSAGE
 * $LOG_LEVEL
 * $LOG_NAME
 * $CHAR:\n
 * $CHAR:\t
 * $CHAR:[
 * $CHAR:]
 * $DATETIME
 * $LINE
 * $FILENAME
 *
 *
 *  "$DATETIME{%Y-%m-%d %H:%M:%S}"
 *  "$CHAR:\t$THREAD_NAME$CHAR:[$THREAD_ID:%FIBER_ID$CHAR:]"
 *  "$CHAR:\t$LOG_NAME$CHAR:[$LOG_LEVEL$CHAR:]"
 *  "$CHAR:\t$FILENAME$CHAR::$LINE"
 *  "$CHAR:\t$FUNCTION_NAME"
 *  "$CHAR:\t$MESSAGE$CHAR:\n"
 **/
class LogLevel
{
public:
  enum Level : uint8_t
  {
    LUNKNOWN = 0,
    LTRACE = 1,
    LDEBUG = 2,
    LINFO = 3,
    LWARN = 4,
    LERROR = 5,
    LCRITICAL = 6,
    LFATAL = 7,
    LCLOSE = 8,
    /* CUSTOM */
  };

  LogLevel(LogLevel::Level level = LUNKNOWN)
    : level_(level)
  {}

  Level level() const { return level_; }
	std::string toString() const
	{
    switch (level_)
    {
#define XX(x) \
    case LogLevel::Level::L##x: \
      return #x;

    XX(TRACE)
    XX(DEBUG)
    XX(INFO)
    XX(CRITICAL)
    XX(WARN)
    XX(ERROR)
    XX(FATAL)
#undef XX

    case LogLevel::Level::LUNKNOWN:
    default:
      return "NONE";
    }

    LY_UNREACHABLE();
	}

  bool operator<(const LogLevel &rhs) const { return level_ < rhs.level_; }
  bool operator>(const LogLevel &rhs) const { return !(*this <= rhs); }
  bool operator<=(const LogLevel &rhs) const { return level_ <= rhs.level_; }
  bool operator>=(const LogLevel &rhs) const { return !(*this < rhs); }
  bool operator==(const LogLevel &rhs) const { return level_ == rhs.level_; }
  bool operator!=(const LogLevel &rhs) const { return !(*this == rhs); }

	static LogLevel fromString(const std::string &str)
	{
#define XX(x)                 \
  if (str == #x) { return LogLevel(L##x); }

    XX(TRACE)
    XX(DEBUG)
    XX(INFO)
    XX(CRITICAL)
    XX(WARN)
    XX(ERROR)
    XX(FATAL)
#undef XX

    return LogLevel(LUNKNOWN);
	}

private:
  Level level_;
};

struct LogColorConfig
{
  enum ColorType : int
  {
    END = 0,
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    BLUE = 4,
    PURPLE = 5,
    LIGHT_BLUE = 6,
    DEEP_RED = 7,
  };

  const char * const colors[8] = {
    "\033[0m",  // END
    "\033[31m", // Red
    "\033[32m", // Green
    "\033[33m", // Yellow
    "\033[34m", // Blue
    "\033[35m", // Purple
    "\033[36m", // Cyan
    "\033[31;2m", // Deep Red
  };

  int LOG_END = END;
  int LOG_LEVEL_TRACE = LIGHT_BLUE;
  int LOG_LEVEL_DEBUG = BLUE;
  int LOG_LEVEL_INFO = GREEN;
  int LOG_LEVEL_CRITICAL = PURPLE;
  int LOG_LEVEL_WARN = YELLOW;
  int LOG_LEVEL_ERROR = RED;
  int LOG_LEVEL_FATAL = DEEP_RED;

  const char *getColor(int type) const
  {
    return colors[type];
  }
};

class Logger;

class LogEvent
{
public:
  using ptr = std::shared_ptr<LogEvent>;

  LogEvent(LogLevel level,
    const std::string &filename, int32_t line, const std::string &functionName,
    int64_t timestamp,
    LogColorConfig config = LogColorConfig());

  std::string getFilename() const { return filename_; }
  std::string getFunctionName() const { return function_name_; }
  int32_t getLine() const { return line_; }
  int64_t getTimestamp() const { return timestamp_; }
  std::string getContent() const { return ss_.str(); }
  // TODO: add thread id and process id
  //std::thread::id getThreadId() const { return std::this_thread::get_id(); }
  //int getProcessId() const {}

  LogLevel getLevel() const { return level_; }
  LogColorConfig getColorConfig() const { return color_config_; }
  std::stringstream &getSS() { return ss_; }

#ifdef FMT_VERSION
  // fmt-style
  template <typename... Args>
  void format(::fmt::string_view fmt, Args&&... args)
  {
    ss_ << ::fmt::format(fmt, std::forward<Args>(args)...);
  }
#else
  // c-style
  template <typename... Args>
  void format(::std::string_view fmt, Args&&... args)
  {
    char buf[256];
    snprintf(buf, 256, fmt.data(), std::forward<Args>(args)...);
    ss_ << std::string(buf);
  }
#endif
private:
  std::string filename_;
  std::string function_name_;
  int32_t line_ = 0;
  int64_t timestamp_ = 0;
  std::stringstream ss_;
  LogLevel level_;
  LogColorConfig color_config_;
};

struct LogFormatterItem {
  using ptr = std::shared_ptr<LogFormatterItem>;

  virtual ~LogFormatterItem() = default;

  virtual void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) = 0;
};

class LogFormatter
{
protected:
  static constexpr char ID_TOKEN = '$';
  static constexpr int FORMAT_ID_LOC = 0;
  static constexpr int FORMAT_FN_ARG_LOC = 1;
  static constexpr int STATUS_CODE_LOC = 2;

  enum {
    PARSE_OK = 0,
    PARSE_ERROR = 1,
  };

  using PatArgsWrapper = std::tuple<std::string, std::string, int>;
public:
  using ptr = std::shared_ptr<LogFormatter>;

  LogFormatter(const std::string &pattern = kDefaultFormatPattern);

  std::string format(LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger);
  std::ostream &format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger);

  bool hasError() const { return hasError_; }
  std::string lastError() const { return error_; }
  std::string getPattern() const { return pattern_; }

  auto toYamlString() const -> std::string;

private:
  PatArgsWrapper parsePatToken(const std::string& patToken);

  void init();

private:
  std::string pattern_;
  std::vector<LogFormatterItem::ptr> items_;
  std::string error_;
  bool hasError_{false};
};

class LogAppender
{
public:
  using ptr = std::shared_ptr<LogAppender>;

  LogAppender() = default;
  virtual ~LogAppender() = default;

  virtual void log(LogEvent::ptr pEvent, std::shared_ptr<Logger> pLogger) = 0;

  void setFormatter(LogFormatter::ptr pFormatter);

  LogFormatter::ptr getFormatter();

  LogLevel getLevel() const { return level_; }
  void setLevel(LogLevel level) { level_ = level; }

  virtual auto toYamlString() const -> std::string = 0;
protected:
  LogLevel level_{LogLevel::LTRACE};
  bool hasFormatter_{false};
  std::mutex mutex_;
  LogFormatter::ptr pFormatter_;
};

class FileLogAppender : public LogAppender
{
public:
  using ptr = std::shared_ptr<FileLogAppender>;

	FileLogAppender(const std::string &filename);
  virtual ~FileLogAppender() override = default;

  virtual void log(LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override;

  bool reopen();

  auto toYamlString() const -> std::string override;

private:
  std::string getWholeFilename();

protected:
  // TODO:
  // can be defined in 'LogEnv' by ini or other config file
  static constexpr uint64_t kMaxLines = 100000;
private:
  /* real filename: filename_ + "_" + current_day + "_" + cnt{02d} + ".log" */
  std::string filename_;
  std::ofstream filestream_;
  uint64_t lastAccessTime_{0};
  uint64_t lines_{0};
  uint8_t cnt_{0};  // cnt_ incr when lines_ encounters kMaxLines
  int today_;
};
class AsyncFileLogAppender : public LogAppender {
public:
  using ptr = std::shared_ptr<AsyncFileLogAppender>;

  AsyncFileLogAppender(const std::string &filename);
  virtual ~AsyncFileLogAppender() override = default;

  virtual void log(LogEvent::ptr pLogEvent,
                   std::shared_ptr<Logger> pLogger) override;

  auto toYamlString() const -> std::string override;
private:
  bool reopen(struct tm* tm);

  std::string getWholeFilename(struct tm *tm) const;

  void reopenNewFileIfShould(int64_t timestamp);

protected:
  // TODO:
  // can be defined in 'LogEnv' by ini or other config file
  static constexpr uint64_t kMaxLines = 100000;

private:
  /* real filename: filename_ + "_" + current_day + "_" + cnt{02d} + ".log" */
  std::string filename_;
  std::ofstream filestream_;
  uint64_t lines_{0};
  uint8_t cnt_{0}; // cnt_ incr when lines_ encounters kMaxLines
  int today_{0};
};
class StdoutLogAppender : public LogAppender
{
public:
  using ptr = std::shared_ptr<StdoutLogAppender>;

	StdoutLogAppender() = default;
  virtual ~StdoutLogAppender() override = default;

  virtual void log(LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override;
  auto toYamlString() const -> std::string override;

};

class Logger : public std::enable_shared_from_this<Logger>
{
public:
	using ptr = std::shared_ptr<Logger>;

  Logger(const std::string &name = "root");
  Logger(const std::string &name, LogLevel level, std::string pattern);
  ~Logger() = default;

  void log(LogEvent::ptr pLogEvent);

  void addAppender(LogAppender::ptr pAppender);
  void removeAppender(LogAppender::ptr pAppender);
  void clearAppenders();

  auto toYamlString() const -> std::string;


  LogLevel getLevel() const { return level_; }
  void setLevel(LogLevel level) { level_ = level; }
  const std::string &getName() const { return name_; }

  void setFormatter(LogFormatter::ptr pFormatter);
  void setFormatter(const std::string &pattern);
  LogFormatter::ptr getFormatter() const;

  Logger::ptr getLogger() { return root_; }
  void setLogger(Logger::ptr pLogger) { root_ = pLogger; }
private:
  std::string name_;
  LogLevel level_{LogLevel::Level::LDEBUG};
  mutable std::mutex mutex_;
  std::list<LogAppender::ptr> appenders_;
  LogFormatter::ptr pFormatter_;
  Logger::ptr root_;
};


class LogEventWrapper {
public:
  using ptr = std::shared_ptr<LogEventWrapper>;

  LogEventWrapper(LogEvent::ptr pEvent, Logger::ptr pLogger);

  ~LogEventWrapper()
  {
    pLogger_->log(pEvent_);
  }

  LY_NODISCARD LogEvent::ptr getEvent() const { return pEvent_; }
  LY_NODISCARD Logger::ptr getLogger() const { return pLogger_; }
  std::stringstream &getSS() { return pEvent_->getSS(); }

private:
  LogEvent::ptr pEvent_;
  Logger::ptr pLogger_;
};

class LogManager
{
public:
  using ptr = std::shared_ptr<LogManager>;

  static LogManager *instance()
  {
    static LogManager *manager = new LogManager();
    return manager;
  }

  ~LogManager() = default;

  Logger::ptr getLogger(const std::string &name);
  bool putLogger(Logger::ptr pLogger);

  // TODO: For future do
  void init() {}
  LY_NODISCARD Logger::ptr getRoot() const { return root_; }
  LY_NODISCARD auto toYamlString() const -> std::string;

private:
  LogManager();

  std::mutex mutex_;
  std::unordered_map<std::string, Logger::ptr> loggers_;
  Logger::ptr root_;
};

class LogIniter
{
public:
  /* the appender's formatter is the same as the logger */
  static Logger::ptr getLogger(
    /* logger */
    const std::string &logName, LogLevel logLevel,
    /* formatter */
    const std::string &formatPattern = kDefaultFormatPattern,
    /* appender */
    bool write2file = true, const std::string &filename = "x", bool async = false);

  // TODO: Test me!!
  static void loadYamlFile(std::string_view filename);

  static void loadYamlNode(YAML::Node node);

};

/* LogIniter::getLogger("sample", "LogLevel::Level::LDEBUG",
 *                      kDefaultFormatPattern, true,
 *                      "sample")
 *
 * sample_${DATE}_${COUNT}.log
 */

LY_NAMESPACE_END
