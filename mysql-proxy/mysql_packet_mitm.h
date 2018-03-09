#ifndef _MYSQL_PACKET_MITM_H_
#define _MYSQL_PACKET_MITM_H_

#include "tcp_mitm.h"
#include "common.h"

class mysql_packet_mitm : public tcp_mitm {
public:
  using mysql_packet = std::vector<unsigned char>;

public:
  mysql_packet_mitm(const socket_type &s)
    : tcp_mitm(s) { }
  virtual ~mysql_packet_mitm() = default;

  virtual void on_client_data_impl(unsigned char *data, size_t bytes) override;
  virtual void on_server_data_impl(unsigned char*, size_t) override;
};

class mysql_client_packet_mitm : public mysql_packet_mitm {
public:
  mysql_client_packet_mitm(const socket_type &s)
    : mysql_packet_mitm(s) { }
  virtual ~mysql_client_packet_mitm();

  virtual void on_client_data_impl(unsigned char *data, size_t bytes) override;
  virtual void on_server_data_impl(unsigned char*, size_t) override;

  virtual void handle_client_packet(const std::vector<mysql_packet> &cmds);

private:
  std::vector<mysql_packet> _client_packets;
};


class mysql_com_query_mitm : public mysql_client_packet_mitm {
public:
  mysql_com_query_mitm(const socket_type &s)
    : mysql_client_packet_mitm(s) { }
  virtual ~mysql_com_query_mitm() = default;

  virtual void handle_client_packet(const std::vector<mysql_packet> &cmds) override;
};

using mysql_com_query_mitm_factory = concrete_factory<
  mysql_com_query_mitm,
  tcp_mitm,
  const socket_type &>;

#endif // _MYSQL_PACKET_MITM_H_