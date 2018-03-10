#include "logger.h"

#include <memory>
#include <stdexcept>

#include <chrono>
#include <ctime>
#include <iomanip>

std::unique_ptr<logger> logger_instance;

void logger::initialize(const std::string &path /*= "log.log"*/) {
  logger_instance.reset(new logger(path));
}

logger &logger::instance() {
  if (!logger_instance) { logger::initialize(); }
  if (logger_instance) { return *logger_instance; }
  throw std::runtime_error("Cannot access the logger");
}

void logger::destroy() {
  if (logger_instance) { logger_instance.reset(); }
}

logger::logger(const std::string &path /*= "log.log"*/)
  : _file(path, std::ios_base::app), _is_running(true)
{
  _worker = std::make_unique<std::thread>([&]() { run(); });
}

logger::~logger() {
  _is_running.store(false);
  _queue.deactivate();
  if (_worker) {
    _worker->join();
  }
}

logger& logger::operator<<(const logger_entry &entry)
{
  _queue.push(entry.str());
  return *this;
}

void logger::run() {
  while (_is_running.load()) {
    auto str = _queue.pop();
#ifdef DEBUG
    std::cout << str << std::endl;
#endif // DEBUG
    if (_is_running.load()) {
      _file << str << std::endl;
    }
  }
}

logger_entry::logger_entry(const std::string &v)
{
  *this << v << " ";
}

logger_entry::~logger_entry() {
  logger::instance() << *this;
}

std::string logger_queue::pop() {
  std::unique_lock<std::mutex> mlock(_mutex);
  while (_is_running.load() && _queue.empty())
  {
    _cond.wait(mlock);
  }
  if (!_queue.empty()) {
    auto item = _queue.front();
    _queue.pop();
    return item;
  }
  return "";
}

void logger_queue::push(std::string&& item) {
  std::unique_lock<std::mutex> mlock(_mutex);
  _queue.push(std::move(item));
  mlock.unlock();
  _cond.notify_one();
}
