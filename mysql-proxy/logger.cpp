#include "logger.h"

#include <memory>
#include <stdexcept>

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

Logger& Logger::operator<<(const std::string &str)
{
  std::cout << str << std::endl;
  if (_file.is_open()) {
    _file << str << std::endl;
  }
  return *this;
}
