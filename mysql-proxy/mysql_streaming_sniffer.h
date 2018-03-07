#ifndef _TCP_SIZE_LOGGER_H_
#define _TCP_SIZE_LOGGER_H_

#include <functional>
#include <string>
#include <memory>

#include "common.h"

class mysql_streaming_sniffer {
public:
  mysql_streaming_sniffer(std::function<void(std::string)> on_data_found)
    : on_data_found_(on_data_found) { }
  virtual ~mysql_streaming_sniffer() = default;

  virtual void feed(unsigned char*, size_t) = 0;

protected:
  std::function<void(std::string)> on_data_found_;
};

using tcp_mitm_factory = factory<mysql_streaming_sniffer, const socket_type &>;

#endif // _TCP_SIZE_LOGGER_H_