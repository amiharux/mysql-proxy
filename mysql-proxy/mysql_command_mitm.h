#ifndef _MYSQL_COMMAND_MITM_H_
#define _MYSQL_COMMAND_MITM_H_

#include "tcp_mitm.h"
#include "common.h"

class mysql_command_mitm : public tcp_mitm {
public:
  mysql_command_mitm(const socket_type &s)
    : tcp_mitm(s) { }
  virtual ~mysql_command_mitm() = default;

  virtual void on_client_data_impl(unsigned char *data, size_t bytes) override;
  virtual void on_server_data_impl(unsigned char*, size_t) override;

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

using mysql_command_mitm_factory = concrete_factory<
  mysql_command_mitm, 
  tcp_mitm,
  const socket_type &>;


class mysql_com_query_mitm : public mysql_command_mitm {
public:
  mysql_com_query_mitm(const socket_type &s)
    : mysql_command_mitm(s) { }
  virtual ~mysql_com_query_mitm() = default;

  virtual void handle_client_command(unsigned opcode, const std::string &s) override;
};

using mysql_com_query_mitm_factory = concrete_factory<
  mysql_com_query_mitm,
  tcp_mitm,
  const socket_type &>;

#endif // _MYSQL_COMMAND_MITM_H_