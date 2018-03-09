#include "mysql_request_mitm.h"
#include "logger.h"

#include <cctype>


mysql_request_mitm::mysql_request_mitm(const socket_type &client_socket, std::unique_ptr<mysql_streaming_sniffer> &&sniffer)
  : tcp_mitm(client_socket), _sniffer(std::move(sniffer))
{
  LOG_TRACE(identity(_client_socket)) << "[mysql_request_mitm::construct]";
}

mysql_request_mitm::~mysql_request_mitm()
{
  LOG_TRACE(identity(_client_socket)) << "[mysql_request_mitm::destruct]";
}

void mysql_request_mitm::on_client_data_impl(unsigned char* data, size_t bytes)
{
  LOG_TRACE(identity(_client_socket)) << "[mysql_request_mitm::on_client_data_impl] Got " << bytes << " bytes";
  if (_sniffer) { _sniffer->feed_client(data, bytes); }
}

void mysql_request_mitm::on_server_data_impl(unsigned char* data, size_t bytes)
{
  LOG_TRACE(identity(_client_socket)) << "[mysql_request_mitm::on_server_data_impl] Got " << bytes << " bytes";
  if (_sniffer) { _sniffer->feed_server(data, bytes); }
}
