#ifndef _TCP_MITM_H_
#define _TCP_MITM_H_

#include "common.h"

namespace mysql_proxy {

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

  virtual void on_client_data_impl(unsigned char*, size_t) { /* do nothing */ }
  virtual void on_server_data_impl(unsigned char*, size_t) { /* do nothing */ }
};

using tcp_mitm_factory = factory<tcp_mitm, const socket_type &>;

}
#endif // _TCP_MITM_H_