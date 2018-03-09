#ifndef _MYSQL_COMMAND_SNIFFER_H_
#define _MYSQL_COMMAND_SNIFFER_H_

#include "mysql_streaming_sniffer.h"
#include "common.h"

class mysql_command_sniffer : public mysql_streaming_sniffer {
public:
  mysql_command_sniffer(const socket_type &s)
    : mysql_streaming_sniffer(s) { }
  virtual ~mysql_command_sniffer() = default;

  virtual void feed_client(unsigned char *data, size_t bytes) override;
  virtual void feed_server(unsigned char*, size_t) override;

  virtual void handle_client_command(unsigned opcode, const std::string &s);
  virtual void handle_server_command(unsigned opcode, const std::string &s) { }

private:
  unsigned _client_opcode = 0;
  size_t _client_bytes_left = 0;
  std::string _client_data;

  unsigned _server_opcode = 0;
  size_t _server_bytes_left = 0;
  std::string _server_data;
};

using mysql_command_sniffer_factory = concrete_factory<
  mysql_command_sniffer, 
  mysql_streaming_sniffer,
  factory<mysql_streaming_sniffer, const socket_type &>, 
  const socket_type &>;

#endif // _MYSQL_COMMAND_SNIFFER_H_