#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string>

#include "tcp_bridge.h"
#include "logger.h"


int main(int argc, char* argv[]) {
  Logger_RAII logger("proxy.log");

  if (argc != 5) {
    LOG << "usage: mysql-proxy <listen host ip> <listen port> <forward host ip> <forward port>" << std::endl;
    return 1;
  }

  const auto listen_port = static_cast<unsigned short>(::atoi(argv[2]));
  const auto forward_port = static_cast<unsigned short>(::atoi(argv[4]));
  const std::string listen_host = argv[1];
  const std::string forward_host = argv[3];

  asio::io_service ios;

  auto on_client_data = [](unsigned char*, size_t bytes) {
    LOG << "Got " << bytes << " bytes from client";
  };

  auto on_server_data = [](unsigned char*, size_t bytes) {
    LOG << "Got " << bytes << " bytes from server";
  };

  try {
    LOG << "starting server on " << listen_host << "::" << listen_port;
    tcp_bridge::acceptor acceptor(ios,
      listen_host, listen_port,
      forward_host, forward_port,
      on_client_data, on_server_data);

    acceptor.accept_connections();

    ios.run();
  } 
  catch (std::exception& e) {
    LOG << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
