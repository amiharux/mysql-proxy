#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string>

#include "tcp_bridge.h"
#include "logger.h"

#include "mysql_request_mitm.h"


int main(int argc, char* argv[]) {
  Logger_RAII logger("proxy.log");

  if (argc != 5) {
    LOG_TRACE() << "usage: mysql-proxy <listen host ip> <listen port> <forward host ip> <forward port>" << std::endl;
    return 1;
  }

  const auto listen_port = static_cast<unsigned short>(::atoi(argv[2]));
  const auto forward_port = static_cast<unsigned short>(::atoi(argv[4]));
  const std::string listen_host = argv[1];
  const std::string forward_host = argv[3];

  asio::io_service ios;

  auto on_client_data = [](unsigned char*, size_t bytes) {
    LOG_TRACE() << "Got " << bytes << " bytes from client";
  };

  auto on_server_data = [](unsigned char*, size_t bytes) {
    LOG_TRACE() << "Got " << bytes << " bytes from server";
  };

  try {
    LOG_TRACE() << "starting server on " << listen_host << "::" << listen_port;

    mysql_request_mitm_factory com_query_request_logger;
    tcp_bridge::acceptor acceptor(ios,
      listen_host, listen_port,
      forward_host, forward_port,
      com_query_request_logger);

    acceptor.accept_connections();

    ios.run();
  } 
  catch (std::exception& e) {
    LOG_TRACE() << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
