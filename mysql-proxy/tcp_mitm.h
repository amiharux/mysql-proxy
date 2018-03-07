#ifndef _TCP_MITM_H_
#define _TCP_MITM_H_

#include "common.h"

class tcp_mitm {
public:
  tcp_mitm(const socket_type &client_socket)
    : _client_socket(client_socket) { }
  virtual ~tcp_mitm() = default;

  void on_client_data(unsigned char* data, size_t bytes) {
    if (bytes) { on_client_data_impl(data, bytes); }
  }
  void on_server_data(unsigned char* data, size_t bytes) {
    if (bytes) { on_server_data_impl(data, bytes); }
  }

protected:
  const socket_type &_client_socket;

  virtual void on_client_data_impl(unsigned char*, size_t) = 0;
  virtual void on_server_data_impl(unsigned char*, size_t) = 0;
};

using p_tcp_mitm = std::unique_ptr<tcp_mitm>;

class tcp_mitm_factory {
public:
  virtual ~tcp_mitm_factory() = default;
  virtual std::unique_ptr<tcp_mitm> get_mitm(const socket_type &client_socket) = 0;
};

template <typename T>
class tcp_mitm_concrete_factory : public tcp_mitm_factory {
public:
  virtual ~tcp_mitm_concrete_factory() override = default;

  virtual p_tcp_mitm get_mitm(const socket_type &client_socket) override {
    return std::make_unique<T>(client_socket);
  }
};


#endif // _TCP_MITM_H_