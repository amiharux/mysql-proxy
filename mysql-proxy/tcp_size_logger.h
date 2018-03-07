#ifndef _TCP_SIZE_LOGGER_H_
#define _TCP_SIZE_LOGGER_H_

#include "tcp_mitm.h"

class tcp_size_logger : public tcp_mitm {
public:
  tcp_size_logger(const socket_type &client_socket);
  virtual ~tcp_size_logger() override;

private:
  virtual void on_client_data_impl(unsigned char*, size_t) final;
  virtual void on_server_data_impl(unsigned char*, size_t) final;
};

using tcp_size_logger_factory = tcp_mitm_concrete_factory<tcp_size_logger>;

#endif // _TCP_SIZE_LOGGER_H_