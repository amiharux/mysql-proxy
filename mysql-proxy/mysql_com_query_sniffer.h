#ifndef _MYSQL_COM_QUERY_SNIFFER_H_
#define _MYSQL_COM_QUERY_SNIFFER_H_

#include "mysql_command_sniffer.h"
#include "logger.h"

class mysql_com_query_sniffer : public mysql_command_sniffer {
public:
  mysql_com_query_sniffer(const socket_type &s)
    : mysql_command_sniffer(s) { }
  virtual ~mysql_com_query_sniffer() = default;

  virtual void handle_client_command(unsigned opcode, const std::string &s) override {
    if (opcode == 0x03) {
      LOG_TRACE(identity(_client_socket)) << "COM_QUERY FOUND: " << s;
    }
  }
};

using mysql_com_query_sniffer_factory = concrete_factory<
  mysql_com_query_sniffer,
  mysql_streaming_sniffer,
  factory<mysql_streaming_sniffer, const socket_type &>,
  const socket_type &>;

#endif // _MYSQL_COM_QUERY_SNIFFER_H_
