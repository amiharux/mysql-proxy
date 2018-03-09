#include "logger.h"

#include <memory>
#include <stdexcept>

#include <chrono>
#include <ctime>
#include <iomanip>

std::unique_ptr<Logger> logger_instance;

void Logger::initialize(const std::string &path /*= "log.log"*/) {
  logger_instance.reset(new Logger(path));
}

Logger &Logger::instance() {
  if (!logger_instance) { Logger::initialize(); }
  if (logger_instance) { return *logger_instance; }
  throw std::runtime_error("Cannot access the logger");
}

void Logger::destroy() {
  if (logger_instance) { logger_instance.reset(); }
}

Logger::Logger(const std::string &path /*= "log.log"*/)
  : _file(path, std::ios_base::app) { }

Logger& Logger::operator<<(const Logger_Entry &entry)
{
  std::string str = entry.str();
  std::cout << str << std::endl;
  if (entry.level() <= 4) {
    _file << str << std::endl;
  }
  return *this;
}

std::string formatted_time()
{
  using namespace std::chrono;
  auto now = system_clock::now();
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  auto timer = system_clock::to_time_t(now);

  std::tm bt;
  localtime_s(&bt, &timer);

  std::ostringstream oss;

  oss << std::put_time(&bt, "%T");
  oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

  return oss.str();
}

Logger_Entry::Logger_Entry(int level) 
  : _level(level) 
{
  *this << formatted_time() << ": ";
}

Logger_Entry::~Logger_Entry() {
  Logger::instance() << *this;
}