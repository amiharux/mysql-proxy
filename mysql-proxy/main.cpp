#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string>

#include <memory>
#include <mutex>

#include <asio.hpp>


namespace tcp_proxy {

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

  void start(const std::string& server_host, unsigned short server_port) {
    // Attempt connection to remote server (server side)
    server_socket_.async_connect(
      asio::ip::tcp::endpoint(asio::ip::address::from_string(server_host), server_port),
      [self = shared_from_this()](const asio::error_code& error) {
        self->handle_server_connect(error);
      }
    );
  }

  void handle_server_connect(const asio::error_code& error) {
    if (error) { return close(); }

    // Setup async read from remote server (server)
    server_socket_.async_read_some(asio::buffer(server_data_, max_data_length),
      [self = shared_from_this()](const asio::error_code& error, size_t bytes) {
        self->handle_server_read(error, bytes);
      }
    );

    // Setup async read from client (client)
    client_socket_.async_read_some(asio::buffer(client_data_, max_data_length),
      [self = shared_from_this()](const asio::error_code& error, size_t bytes) {
        self->handle_client_read(error, bytes);
      }
    );
  }

private:
  void handle_server_read(const asio::error_code& error, size_t bytes) {
    if (error) { return close(); }

    async_write(client_socket_, asio::buffer(server_data_, bytes),
      [self = shared_from_this()](const asio::error_code& error, size_t bytes) {
        self->handle_client_write(error, bytes);
      }
    );
  }

  void handle_client_write(const asio::error_code& error, size_t bytes) {
    if (error) { return close(); }

    server_socket_.async_read_some(asio::buffer(server_data_, max_data_length),
      [self = shared_from_this()](const asio::error_code& error, size_t bytes) {
        self->handle_server_read(error, bytes);
      }
    );
  }

  void handle_client_read(const asio::error_code& error, size_t bytes) {
    if (error) { return close(); }

    async_write(server_socket_, asio::buffer(client_data_, bytes),
      [self = shared_from_this()](const asio::error_code& error, size_t bytes) {
        self->handle_server_write(error, bytes);
      }
    );
  }

  void handle_server_write(const asio::error_code& error, size_t bytes) {
    if (error) { return close(); }

    client_socket_.async_read_some(asio::buffer(client_data_, max_data_length),
      [self = shared_from_this()](const asio::error_code& error, size_t bytes) {
        self->handle_client_read(error, bytes);
      }
    );
  }

  void close() {
    std::lock_guard<std::mutex> lock(mutex_);

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
      try {
        session_ = std::make_shared<bridge>(io_service_);

        acceptor_.async_accept(session_->client_socket(),
          [&](const asio::error_code& error) {
            handle_accept(error);
          });
      } 
      catch (std::exception& e) {
        std::cerr << "acceptor exception: " << e.what() << std::endl;
        return false;
      }

      return true;
    }

  private:

    void handle_accept(const asio::error_code& error) {
      if (!error) {
        session_->start(server_host_, server_port_);

        if (!accept_connections()) {
          std::cerr << "Failure during call to accept." << std::endl;
        }
      } else {
        std::cerr << "Error: " << error.message() << std::endl;
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

}

int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr << "usage: mysql-proxy <listen host ip> <listen port> <forward host ip> <forward port>" << std::endl;
    return 1;
  }

  const auto listen_port = static_cast<unsigned short>(::atoi(argv[2]));
  const auto forward_port = static_cast<unsigned short>(::atoi(argv[4]));
  const std::string listen_host = argv[1];
  const std::string forward_host = argv[3];

  asio::io_service ios;

  try {
    tcp_proxy::bridge::acceptor acceptor(ios,
      listen_host, listen_port,
      forward_host, forward_port);

    acceptor.accept_connections();

    ios.run();
  } 
  catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
