#ifndef _TCP_SIZE_LOGGER_H_
#define _TCP_SIZE_LOGGER_H_

#include "tcp_mitm.h"

class mysql_request_mitm : public tcp_mitm {
public:
  mysql_request_mitm(const socket_type &client_socket);
  virtual ~mysql_request_mitm() override;

private:
  virtual void on_client_data_impl(unsigned char*, size_t) final;
  virtual void on_server_data_impl(unsigned char*, size_t) final;
};

using mysql_request_mitm_factory = concrete_factory<mysql_request_mitm, tcp_mitm, const socket_type &>;

#endif // _TCP_SIZE_LOGGER_H_