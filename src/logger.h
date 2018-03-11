#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <initializer_list>

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class logger_entry : public std::stringstream {
public:
  logger_entry(const std::string &v ="");
  virtual ~logger_entry() override;
};

class logger_queue {
public:
  logger_queue() : _is_running(true) { }

  std::string pop();
  void push(std::string&& item);

  void deactivate() { _is_running.store(false); }

private:
  std::queue<std::string> _queue;
  std::mutex _mutex;
  std::condition_variable _cond;
  std::atomic<bool> _is_running;
};

class logger {
  logger(const std::string &path = "log.log");

public:
  virtual ~logger();

public:
  static void initialize(const std::string &path = "log.log");
  static logger &instance();
  static void destroy();

public:
  logger& operator<< (const logger_entry &entry);

private:
  void run();

  std::ofstream _file;
  logger_queue _queue;

  std::atomic<bool> _is_running;
  std::unique_ptr<std::thread> _worker;
};

#ifdef DEBUG
using LOG_TRACE = logger_entry;
#else
using LOG_TRACE = std::stringstream;
#endif // DEBUG

using LOG_INFO = logger_entry;

class Logger_RAII {
public:
  Logger_RAII(const std::string &path = "log.log") {
    logger::initialize(path);
  }
  ~Logger_RAII() {
    logger::destroy();
  }
};

#endif // _LOGGER_H_
