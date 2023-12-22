#include <iostream>
#include <core/util/logger/Logger.h>


LY_NAMESPACE_BEGIN

/********************************************* LogEvent *********************************************/

LogEvent::LogEvent(LogLevel::Level level,
  const std::string &filename, int32_t line, const std::string &functionName,
  int64_t timestamp,
  LogColorConfig config)
  : filename_(filename)
  , function_name_(functionName)
  , line_(line)
  , timestamp_(timestamp)
  , level_(level)
{
}

#if 0
void LogEvent::format(const char *fmt, ...)
{
  va_list args;
  char buf[256]{0};

  va_start(args, fmt);
  int len = vsnprintf(&buf[0], sizeof(buf), fmt, args);
  if (len < 0) {
    va_end(args);
    return;
  }

  ss_ << std::string(buf, len);
  va_end(args);
}
#endif

/***************************************** LogFormatterItem *****************************************/
class MessageFormatterItem final : public LogFormatterItem
{
public:
  MessageFormatterItem(const std::string &str = "") {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override
  {
    os << pLogEvent->getContent();
  }
};
class LogLevelFormatterItem final : public LogFormatterItem
{
public:
  LogLevelFormatterItem(const std::string &str = "") {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override
  {
    os << LogLevel::toString(pLogEvent->getLevel());
  }
};
class LogNameFormatterItem final : public LogFormatterItem
{
public:
  LogNameFormatterItem(const std::string &str = "") {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override
  {
    os << pLogger->getName();
  }
};
class DateTimeFormatterItem final : public LogFormatterItem
{
public:
  DateTimeFormatterItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
    : timefmt_(format)
  {
    if (timefmt_.empty()) {
      timefmt_ = "%Y-%m-%d %H:%M:%S";
    }
  }
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override
  {
    const auto timestamp = pLogEvent->getTimestamp();
    const auto tp = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(timestamp));
    time_t t = std::chrono::system_clock::to_time_t(tp);
#ifdef __WIN__
    struct tm tm;
    localtime_s(&tm, &t);
#elif defined(__LINUX__)
    struct tm tm;
    localtime_r(&t, &tm);
#else
    struct tm tm = *localtime(&t);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), timefmt_.c_str(), &tm);
    os << buf;
  }

private:
  std::string timefmt_;
};
class FilenameFormatterItem final : public LogFormatterItem
{
public:
  FilenameFormatterItem(const std::string &str = "") {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override
  {
    os << pLogEvent->getFilename();
  }
};
class LineFormatterItem final : public LogFormatterItem
{
public:
  LineFormatterItem(const std::string &str = "") {}
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override
  {
    os << pLogEvent->getLine();
  }
};
class StringFormatterItem final : public LogFormatterItem
{
public:
  StringFormatterItem(const std::string &str)
    : str_(str)
  {
  }
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override
  {
    os << str_;
  }

private:
  std::string str_;
};
class CharFormatterItem final : public LogFormatterItem
{
public:
  CharFormatterItem(const std::string &str = "")
    : str_(str)
  {
  }
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override
  {
    os << str_;
  }

private:
  std::string str_;
};
class FunctionNameFormatterItem final : public LogFormatterItem
{
public:
  FunctionNameFormatterItem(const std::string &str)
    : str_(str)
  {
  }
  void format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) override
  {
    os << pLogEvent->getFunctionName();
  }

private:
  std::string str_;
};

/******************************************* LogFormatter *******************************************/
LogFormatter::LogFormatter(const std::string &pattern)
  : pattern_(pattern) {
  init();
}

std::string LogFormatter::format(LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) {
  std::stringstream ss;
  for (auto &item : items_) {
    item->format(ss, pLogEvent, pLogger);
  }
  return ss.str();
}

std::ostream& LogFormatter::format(std::ostream &os, LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger) {
  std::stringstream ss;
  for (auto &item : items_) {
    item->format(ss, pLogEvent, pLogger);
  }
  os << ss.str();
  os.flush();
  return os;
}

LogFormatter::PatArgsWrapper LogFormatter::parsePatToken(const std::string &patToken) {
  if (patToken.find("CHAR:") == 0) {
    if (patToken.length() <= 5) return std::make_tuple("CHAR", "", PARSE_ERROR);
    auto ch = patToken.substr(5);
    return std::make_tuple("CHAR", ch, PARSE_OK);
  }
  if (patToken.find("DATETIME") == 0) {
    if (patToken.length() > 8 && patToken[8] == '{') {
      size_t timefmt_len = patToken.rfind('}');
      if (timefmt_len >= 9) {
        timefmt_len -= 9;
        auto timefmt = patToken.substr(9, timefmt_len);
        return std::make_tuple("DATETIME", timefmt, PARSE_OK);
      }
      else {
        // error timefmt
        return std::make_tuple(patToken, "", PARSE_ERROR);
      }
    }
    else {
      // Default DATETIME format
      return std::make_tuple("DATETIME", "%Y-%m-%d %H:%M:%S", PARSE_OK);
    }
  }
  // NO PARAM ARG
  return { patToken, "", PARSE_OK };
}

void LogFormatter::init() {
  std::vector<PatArgsWrapper> vec;
  std::string nstr;
  size_t start_pos = 0, len = 0;
  for (size_t i = 0; i < pattern_.size(); ++i) {
    if (pattern_[i] == ID_TOKEN) {
      if (len != 0) {
        nstr = pattern_.substr(start_pos, len);
        vec.push_back(parsePatToken(nstr));
      }

      start_pos = i + 1;
      len = 0;
      continue;
    }

    ++len;
  }

  if (len != 0) {
    nstr = pattern_.substr(start_pos, len);
    vec.push_back(parsePatToken(nstr));
  }
  else {
    // $
    vec.push_back(std::make_tuple("", "", PARSE_ERROR));
  }

  static std::unordered_map<std::string, std::function<LogFormatterItem::ptr(const std::string &str)> >
    s_format_items = {
#define XX(STR, ID) \
      { STR, [](const std::string& str) -> LogFormatterItem::ptr { return std::make_shared<ID>(str);} }
      XX("LOG_LEVEL"    , LogLevelFormatterItem),
      XX("MESSAGE"      , MessageFormatterItem),
      XX("LOG_NAME"     , LogNameFormatterItem),
      XX("DATETIME"     , DateTimeFormatterItem),
      XX("FILENAME"     , FilenameFormatterItem),
      XX("LINE"         , LineFormatterItem),
      XX("CHAR"         , CharFormatterItem),
      XX("FUNCTION_NAME", FunctionNameFormatterItem),
#undef XX
    };

  hasError_ = false;
  for (const auto &wrapper : vec) {
    const auto &[id, arg, status] = wrapper;
    if (status != PARSE_OK) {
      items_.push_back(std::make_shared<StringFormatterItem>(id));
      continue;
    }

    auto it = s_format_items.find(id);
    if (it == s_format_items.end()) {
      hasError_ = true;
      error_.clear();
      error_.append("<<PATTERN ERROR: UNSUPPORTED FORMAT $");
      error_.append(id);
      error_.append(">>");
      items_.push_back(std::make_shared<StringFormatterItem>(error_));
    }
    else {
      items_.push_back(it->second(arg));
    }
  }
}

auto LogFormatter::toYamlString() const -> std::string 
{
  YAML::Node node;
  node["pattern"] = pattern_;
  return (std::ostringstream{} << node).str();
}


/******************************************* LogAppender ********************************************/
void LogAppender::setFormatter(LogFormatter::ptr pFormatter)
{
  std::lock_guard<std::mutex> locker(mutex_);
  pFormatter_ = pFormatter;
  if (pFormatter_) {
    hasFormatter_ = true;
  } else {
    hasFormatter_ = false;
  }
}

LogFormatter::ptr LogAppender::getFormatter()
{
  std::lock_guard<std::mutex> locker(mutex_);
  return pFormatter_;
}

/***************************************** FileLogAppender ******************************************/
FileLogAppender::FileLogAppender(const std::string &filename)
  : filename_(filename)
{
  reopen();
}

void FileLogAppender::log(LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger)
{
  if (pLogEvent->getLevel() >= level_) {
    uint64_t now = pLogEvent->getTimestamp();
    if (now >= (lastAccessTime_ + 3)) {
      reopen();
      lastAccessTime_ = now;
    }

    std::lock_guard<std::mutex> locker(mutex_);
    if (!pFormatter_->format(filestream_, pLogEvent, pLogger)) {
      std::cerr << "error in "
                << "FileLogAppender::log"
                << " with Formatter format" << std::endl;
      std::cerr << "log file cannot be created" << std::endl;
    } else {
      lines_++;
      if (lines_ >= kMaxLines) {
        cnt_++;
        lines_ = 0;
      }
    }
  }
}

bool FileLogAppender::reopen()
{
  std::lock_guard<std::mutex> locker(mutex_);
  if (filestream_) {
    filestream_.close();
  }

  // filestream_.open(filename_, std::ios::app);
  filestream_.open(getWholeFilename(), std::ios::app);
  return filestream_.is_open();
}

auto FileLogAppender::toYamlString() const -> std::string
{
  YAML::Node node;
  node["type"] = "SyncFileLogAppender";
  node["filename"] = filename_;
  node["lines"] = lines_;
  node["count"] = cnt_;
  node["today"] = today_;

  node["level"] = LogLevel::toString(level_);
  if (hasFormatter_)
  {
    node["formatter"] = pFormatter_->toYamlString();
  }

  return (std::ostringstream{} << node).str();
}

std::string FileLogAppender::getWholeFilename()
{
  std::string wholeFilename;
  time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
#ifdef __WIN__
  struct tm tm;
  localtime_s(&tm, &t);
#elif defined(__LINUX__)
  struct tm tm;
  localtime_r(&t, &tm);
#else
  struct tm tm = *localtime(&t);
#endif

  char todayStr[30];
  std::strftime(todayStr, 20, "%Y-%m-%d", &tm);
  const int today = tm.tm_yday;
  if (today != today_) {
    today_ = today;
    cnt_ = 0;
  }
  wholeFilename.append(filename_);
  wholeFilename.append("_");
  wholeFilename.append(todayStr);
  wholeFilename.append("_");
  if (cnt_ < 10)
    wholeFilename.append("0");
  wholeFilename.append(std::to_string(cnt_));
  wholeFilename.append(".log");

  return wholeFilename;
}

/*************************************** AsyncFileLogAppender ***************************************/
AsyncFileLogAppender::AsyncFileLogAppender(const std::string &filename)
  : filename_(filename)
{
}
void AsyncFileLogAppender::log(LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger)
{
  if (pLogEvent->getLevel() >= level_) {
    std::async(std::launch::async, [&]() {
      std::unique_lock<std::mutex> locker(mutex_);

      int64_t now = pLogEvent->getTimestamp();
      reopenNewFileIfShould(now);
      if (!pFormatter_->format(filestream_, pLogEvent, pLogger)) {
        std::cerr << "error in "
                  << "AsyncFileLogAppender::log"
                  << " with Formatter format" << std::endl;
        std::cerr << "log file cannot be created" << std::endl;
      } else {
        lines_++;
      }
    });
  }
}

auto AsyncFileLogAppender::toYamlString() const -> std::string
{
  YAML::Node node;
  node["type"] = "AsyncFileLogAppender";
  node["filename"] = filename_;
  node["lines"] = lines_;
  node["count"] = cnt_;
  node["today"] = today_;

  node["level"] = LogLevel::toString(level_);
  if (hasFormatter_)
  {
    node["formatter"] = pFormatter_->toYamlString();
  }

  return (std::ostringstream{} << node).str();
}

bool AsyncFileLogAppender::reopen(tm *tm)
{
  if (filestream_) {
    filestream_.close();
  }

  filestream_.open(getWholeFilename(tm), std::ios::app);
  return filestream_.is_open();
}

std::string AsyncFileLogAppender::getWholeFilename(tm *tm) const
{
  std::string wholeFilename;

  char todayStr[20];
  strftime(todayStr, 20, "%Y-%m-%d", tm);
  wholeFilename.append(filename_);
  wholeFilename.append("_");
  wholeFilename.append(todayStr);
  wholeFilename.append("_");
  if (cnt_ < 10)
    wholeFilename.append("0");
  wholeFilename.append(std::to_string(cnt_));
  wholeFilename.append(".log");

  return wholeFilename;
}

void AsyncFileLogAppender::reopenNewFileIfShould(int64_t timestamp)
{
  time_t t = timestamp / 1000;
#ifdef __WIN__
  struct tm tm;
  localtime_s(&tm, &t);
#elif defined(__LINUX__)
  struct tm tm;
  localtime_r(&t, &tm);
#else
  struct tm tm = *localtime(&t);
#endif

  const int today = tm.tm_yday;
  if (today != today_) {
    today_ = today;
    // switch to new file
    cnt_ = 0;
    lines_ = 0;
    reopen(&tm);
    return;
  }

  if (lines_ >= kMaxLines) {
    // switch to new file
    cnt_++;
    lines_ = 0;
    reopen(&tm);
    return;
  }

  // donothing
}

/**************************************** StdoutLogAppender *****************************************/
void StdoutLogAppender::log(LogEvent::ptr pLogEvent, std::shared_ptr<Logger> pLogger)
{
  if (pLogEvent->getLevel() >= level_) {
    std::lock_guard<std::mutex> locker(mutex_);

    LogColorConfig conf = pLogEvent->getColorConfig();
    switch (pLogEvent->getLevel()) {
#define XX(x)       \
      case LogLevel::L##x: \
        std::cout << conf.getColor(conf.LOG_LEVEL_##x); \
      break;

      XX(TRACE)
      XX(DEBUG)
      XX(INFO)
      XX(CRITICAL)
      XX(WARN)
      XX(ERROR)
      XX(FATAL)
#undef XX
      default: ;
    }

    pFormatter_->format(std::cout, pLogEvent, pLogger);
    if (pLogEvent->getLevel() > LogLevel::LUNKNOWN)
      std::cout << conf.getColor(conf.LOG_END);
  }
}

auto StdoutLogAppender::toYamlString() const -> std::string
{
  YAML::Node node;
  node["type"] = "StdoutLogAppender";

  node["level"] = LogLevel::toString(level_);
  if (hasFormatter_)
  {
    node["formatter"] = pFormatter_->toYamlString();
  }

  return (std::ostringstream{} << node).str();
}

/********************************************** Logger **********************************************/
Logger::Logger(const std::string &name)
  : name_(name), pFormatter_(new LogFormatter())
{
}
Logger::Logger(const std::string &name, LogLevel::Level level, std::string pattern)
  : name_(name)
  , level_(level)
  , pFormatter_(new LogFormatter(pattern))
{
}

void Logger::log(LogEvent::ptr pLogEvent)
{
  if (pLogEvent->getLevel() >= level_) {
    auto self = shared_from_this();
    std::lock_guard<std::mutex> locker(mutex_);
    if (!appenders_.empty()) {
      for (auto &pAppender : appenders_) {
        pAppender->log(pLogEvent, self);
      }
    } else if (root_)
      root_->log(pLogEvent);
  }
}

void Logger::addAppender(LogAppender::ptr pAppender)
{
  std::lock_guard<std::mutex> locker(mutex_);

  if (!pAppender->getFormatter()) {
    pAppender->setFormatter(pFormatter_);
  }
  appenders_.push_back(pAppender);
}
void Logger::removeAppender(LogAppender::ptr pAppender)
{
  std::lock_guard<std::mutex> locker(mutex_);
  auto it = std::find(appenders_.begin(), appenders_.end(), pAppender);
  if (it != appenders_.end()) {
    appenders_.erase(it);
  }
}
void Logger::clearAppenders()
{
  std::lock_guard<std::mutex> locker(mutex_);
  appenders_.clear();
}

void Logger::setFormatter(LogFormatter::ptr pFormatter)
{
  std::lock_guard<std::mutex> locker(mutex_);
  pFormatter_ = pFormatter;
}
void Logger::setFormatter(const std::string &pattern)
{
  std::lock_guard<std::mutex> locker(mutex_);
  pFormatter_ = std::make_shared<LogFormatter>(pattern);
}
LogFormatter::ptr Logger::getFormatter() const
{
  std::lock_guard<std::mutex> locker(mutex_);
  return pFormatter_;
}

auto Logger::toYamlString() const -> std::string 
{
  YAML::Node node;

  node["name"] = this->name_;
  node["level"] = LogLevel::toString(this->level_);
  if (!appenders_.empty()) {
    for (auto appender : appenders_) {
      node["appenders"].push_back(appender->toYamlString());
    }
  }
  node["formatter"] = pFormatter_->toYamlString();

  return (std::stringstream{} << node).str();
}

/***************************************** LogEventWrapper ******************************************/

LogEventWrapper::LogEventWrapper(LogEvent::ptr pEvent, Logger::ptr pLogger)
  : pEvent_(pEvent)
  , pLogger_(pLogger)
{
}

/******************************************** LogManager ********************************************/
Logger::ptr LogManager::getLogger(const std::string &name)
{
  std::lock_guard<std::mutex> locker(mutex_);
  auto it = loggers_.find(name);
  if (it != loggers_.end()) {
    return it->second;
  }

  auto pLogger = std::make_shared<Logger>(name);
  pLogger->setLogger(root_);
  loggers_[name] = pLogger;
  return pLogger;
}
bool LogManager::putLogger(Logger::ptr pLogger) 
{
  if (auto it = loggers_.find(pLogger->getName()); 
      it != loggers_.end()) {
    return false;
  }
  loggers_.emplace(pLogger->getName(), pLogger);
  return true;
}

LogManager::LogManager() {
  root_.reset(new Logger());
  auto pAppender = std::make_shared<StdoutLogAppender>();
  // pAppender->setFormatter(root_->getFormatter());
  root_->addAppender(pAppender);

  loggers_[root_->getName()] = root_;

  init();
}

auto LogManager::toYamlString() const -> std::string 
{
  YAML::Node node;
  
  for (auto &[k, v] : loggers_) {
    node["logger"].push_back(v->toYamlString());
  }

  return (std::ostringstream{} << node).str();
}

LY_NAMESPACE_END
