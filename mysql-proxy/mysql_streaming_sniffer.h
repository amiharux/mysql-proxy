#ifndef _MYSQL_STREAMING_SNIFFER_H_
#define _MYSQL_STREAMING_SNIFFER_H_

#include <functional>
#include <string>
#include <memory>

#include "common.h"

class mysql_streaming_sniffer {
public:
  mysql_streaming_sniffer(const socket_type &client_socket)
    : _client_socket(client_socket) { }
  virtual ~mysql_streaming_sniffer() = default;

  virtual void feed_client(unsigned char*, size_t) = 0;
  virtual void feed_server(unsigned char*, size_t) = 0;

protected:
  const socket_type &_client_socket;
};

using mysql_streaming_sniffer_factory = factory<mysql_streaming_sniffer, const socket_type &>;

#endif // _MYSQL_STREAMING_SNIFFER_H_