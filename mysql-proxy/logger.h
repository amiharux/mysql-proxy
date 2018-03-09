#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <initializer_list>

class Logger_Entry : public std::stringstream {
public:
  Logger_Entry(int level = 0);
  virtual ~Logger_Entry() override;

  int level() const { return _level; }

private:
  int _level;
};

class Logger {
  Logger(const std::string &path = "log.log");
public:
  static void initialize(const std::string &path = "log.log");
  static Logger &instance();
  static void destroy();

public:
  Logger& operator<< (const Logger_Entry &entry);

private:
  std::ofstream _file;
};

class LOG_TRACE : public Logger_Entry {
public:
  LOG_TRACE() : Logger_Entry(6) { }
  LOG_TRACE(const std::string &v) : LOG_TRACE() { *this << v << " "; }
};

class LOG_INFO : public Logger_Entry {
public:
  LOG_INFO() : Logger_Entry(3) { }
  LOG_INFO(const std::string &v) : LOG_INFO() { *this << v << " "; }
};

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
