#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string>

#include <memory>
#include <mutex>

#include <asio.hpp>

#include "logger.h"


class bridge : public std::enable_shared_from_this<bridge> {
public:

  using socket_type = asio::ip::tcp::socket;
  using ptr_type = std::shared_ptr<bridge>;

  bridge(asio::io_service& ios)
    : client_socket_(ios)
    , server_socket_(ios)
  {}

  socket_type& client_socket() {
    return client_socket_;
  }

  socket_type& server_socket() {
    return server_socket_;
  }

  std::string identity() const {
    std::stringstream ss;
    try {
      auto le = client_socket_.remote_endpoint();
      ss << le.address().to_string() << "::" << le.port();
    } 
    catch (const std::system_error &err) {
      ss << "<socket_closed:" << err.code() << ">";
    }
    return ss.str();
  }

  void start(const std::string& server_host, unsigned short server_port) {
    LOG_ID(identity()) << "bridge::start";
    server_socket_.async_connect(
      asio::ip::tcp::endpoint(asio::ip::address::from_string(server_host), server_port),
      [self = shared_from_this()](const asio::error_code& error) {
        self->handle_server_connect(error);
      }
    );
  }

  void handle_server_connect(const asio::error_code& error) {
    if (error) { return close(); }
    LOG_ID(identity()) << "bridge::handle_server_connect";

    async_read_from_server(error);
    async_read_from_client(error);
  }

private:
  void async_write_to_client(const asio::error_code& error, size_t bytes) {
    if (error) { return close(); }
    LOG_ID(identity()) << "bridge::async_write_to_client";

    async_write(client_socket_, asio::buffer(server_data_, bytes),
      [self = shared_from_this()](const asio::error_code& error, size_t) {
        self->async_read_from_server(error);
      }
    );
  }

  void async_read_from_server(const asio::error_code& error) {
    if (error) { return close(); }
    LOG_ID(identity()) << "bridge::async_read_from_server";

    server_socket_.async_read_some(asio::buffer(server_data_, max_data_length),
      [self = shared_from_this()](const asio::error_code& error, size_t bytes) {
        self->async_write_to_client(error, bytes);
      }
    );
  }

  void async_write_to_server(const asio::error_code& error, size_t bytes) {
    if (error) { return close(); }
    LOG_ID(identity()) << "bridge::async_write_to_server";

    async_write(server_socket_, asio::buffer(client_data_, bytes),
      [self = shared_from_this()](const asio::error_code& error, size_t) {
        self->async_read_from_client(error);
      }
    );
  }

  void async_read_from_client(const asio::error_code& error) {
    if (error) { return close(); }
    LOG_ID(identity()) << "bridge::async_read_from_client";

    client_socket_.async_read_some(asio::buffer(client_data_, max_data_length),
      [self = shared_from_this()](const asio::error_code& error, size_t bytes) {
        self->async_write_to_server(error, bytes);
      }
    );
  }

  void close() {
    std::lock_guard<std::mutex> lock(mutex_);

    LOG_ID(identity()) << "bridge::close";

    if (client_socket_.is_open()) {
      client_socket_.close();
    }

    if (server_socket_.is_open()) {
      server_socket_.close();
    }
  }

  socket_type client_socket_;
  socket_type server_socket_;

  enum { max_data_length = 8192 }; //8KB
  unsigned char client_data_[max_data_length];
  unsigned char server_data_[max_data_length];

  std::mutex mutex_;

public:

  class acceptor {
  public:

    acceptor(asio::io_service& io_service,
      const std::string& listen_host, unsigned short listen_port,
      const std::string& server_host, unsigned short server_port)
      : io_service_(io_service)
      , listenhost_address(asio::ip::address_v4::from_string(listen_host))
      , acceptor_(io_service_, asio::ip::tcp::endpoint(listenhost_address, listen_port))
      , server_port_(server_port)
      , server_host_(server_host)
    {}

    bool accept_connections() {
      LOG << "bridge::acceptor::accept_connections";
      try {
        session_ = std::make_shared<bridge>(io_service_);

        acceptor_.async_accept(session_->client_socket(),
          [&](const asio::error_code& error) {
            handle_accept(error);
          });
      } 
      catch (std::exception& e) {
        LOG << "acceptor exception: " << e.what() << std::endl;
        return false;
      }

      return true;
    }

  private:

    void handle_accept(const asio::error_code& error) {
      LOG_ID(session_->identity()) << "bridge::acceptor::handle_accept";
      if (!error) {
        session_->start(server_host_, server_port_);

        if (!accept_connections()) {
          LOG << "Failure during call to accept." << std::endl;
        }
      } else {
        LOG << "Error: " << error.message() << std::endl;
      }
    }

    asio::io_service& io_service_;
    asio::ip::address_v4 listenhost_address;
    asio::ip::tcp::acceptor acceptor_;
    ptr_type session_;
    unsigned short server_port_;
    std::string server_host_;
  };
};

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

  try {
    LOG << "starting server on " << listen_host << "::" << listen_port;
    bridge::acceptor acceptor(ios,
      listen_host, listen_port,
      forward_host, forward_port);

    acceptor.accept_connections();

    ios.run();
  } 
  catch (std::exception& e) {
    LOG << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
