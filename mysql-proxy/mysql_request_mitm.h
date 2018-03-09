#ifndef _TCP_SIZE_LOGGER_H_
#define _TCP_SIZE_LOGGER_H_

#include "tcp_mitm.h"
#include "mysql_streaming_sniffer.h"

class mysql_request_mitm : public tcp_mitm {
public:
  mysql_request_mitm(const socket_type &client_socket, std::unique_ptr<mysql_streaming_sniffer> &&sniffer = nullptr);
  virtual ~mysql_request_mitm() override;

private:
  virtual void on_client_data_impl(unsigned char*, size_t) final;
  virtual void on_server_data_impl(unsigned char*, size_t) final;

protected:
  std::unique_ptr<mysql_streaming_sniffer> _sniffer;
};

class mysql_request_mitm_factory 
  : public concrete_factory
    < mysql_request_mitm
    , tcp_mitm
    , factory<tcp_mitm, const socket_type &>
    , const socket_type &
    >
{
public:
  mysql_request_mitm_factory(mysql_streaming_sniffer_factory &stream_sniffer_factory)
    : _stream_sniffer_factory(stream_sniffer_factory) { }

  virtual std::unique_ptr<tcp_mitm> build(const socket_type &s) override {
    return std::make_unique<mysql_request_mitm>(s, _stream_sniffer_factory.build(s));
  }

protected:
  mysql_streaming_sniffer_factory &_stream_sniffer_factory;
};

#endif // _TCP_SIZE_LOGGER_H_