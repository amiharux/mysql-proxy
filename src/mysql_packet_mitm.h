#ifndef _MYSQL_PACKET_MITM_H_
#define _MYSQL_PACKET_MITM_H_

#include "tcp_mitm.h"
#include "common.h"

namespace mysql_proxy {

class mysql_packet : public std::vector<unsigned char> {
public:
  static const unsigned OPCODE_COM_QUERY = 0x03;
  static const size_t HEADER_SIZE = 4;

public:
  size_t payload_length() const {
    assert(has_header());
    return (*this)[0] + ((*this)[1] << 8) + ((*this)[2] << 16);
  }

  unsigned sequence() const {
    assert(has_header());
    return (*this)[3];
  }

  unsigned opcode() const {
    assert(size() > HEADER_SIZE);
    return (*this)[HEADER_SIZE];
  }

  std::string command() const {
    assert(size() > HEADER_SIZE + 1);
    return std::string(begin() + HEADER_SIZE + 1, end());
  }

public:
  bool has_header() const {
    return size() >= HEADER_SIZE;
  }

  size_t header_bytes_left() const {
    return has_header() ? 0 : HEADER_SIZE - size();
  }

  bool has_body() const {
    return has_header() && size() >= payload_length() + HEADER_SIZE;
  }

  size_t body_bytes_left() const {
    return has_body() ? 0 : payload_length() + HEADER_SIZE - size();
  }
};

class mysql_packet_mitm : public tcp_mitm {
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

}
#endif // _MYSQL_PACKET_MITM_H_