#ifndef _LOGGER_H_
#define LOGGER_H_

#include <iostream>
#include <fstream>
#include <sstream>

class Logger {
  Logger(const std::string &path = "log.log");
public:
  static void initialize(const std::string &path = "log.log");
  static Logger &instance();
  static void destroy();

public:
  Logger& operator<< (const std::string &str);

private:
  std::ofstream _file;
};

class Logger_Entry : public std::stringstream {
public:
  virtual ~Logger_Entry() override {
    Logger::instance() << this->str();
  }
};

#define LOG Logger_Entry{}
#define LOG_ID(identity) Logger_Entry{} << (identity) << ": "

class Logger_RAII {
public:
  Logger_RAII(const std::string &path = "log.log") {
    Logger::initialize(path);
  }
  ~Logger_RAII() {
    Logger::destroy();
  }
};

#endif // _LOGGER_H_
