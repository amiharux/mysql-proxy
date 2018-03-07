#ifndef _TCP_BRIDGE_H_
#define _TCP_BRIDGE_H_

#include <asio.hpp>

#include <functional>
#include <mutex>

#include "common.h"
#include "tcp_mitm.h"


using on_data_cb = std::function<void(unsigned char*, size_t)>;
const on_data_cb dummy_cb = [](unsigned char*, size_t) {};

class tcp_bridge : public std::enable_shared_from_this<tcp_bridge> {
public:
  tcp_bridge(asio::io_service& ios, tcp_mitm_factory &mitm_factory);
  ~tcp_bridge();

  socket_type& client_socket();
  socket_type& server_socket();
  std::string client_id() const;

  void start(const std::string& server_host, unsigned short server_port);
  void handle_server_connect(const asio::error_code& error);

private:
  void async_write_to_client(const asio::error_code& error, size_t bytes);
  void async_read_from_server(const asio::error_code& error);
  void async_write_to_server(const asio::error_code& error, size_t bytes);
  void async_read_from_client(const asio::error_code& error);
  void close();

  socket_type client_socket_;
  socket_type server_socket_;

  enum { max_data_length = 8192 }; //8KB
  unsigned char client_data_[max_data_length];
  unsigned char server_data_[max_data_length];

  tcp_mitm_factory &mitm_factory_;
  std::unique_ptr<tcp_mitm> mitm_;

  std::mutex mutex_;

public:
  class acceptor {
  public:

    acceptor(
      asio::io_service& io_service, 
      const std::string& listen_host, unsigned short listen_port, 
      const std::string& server_host, unsigned short server_port, 
      tcp_mitm_factory& mitm_factory);

    bool accept_connections();

  private:

    void handle_accept(const asio::error_code& error);

    asio::io_service& io_service_;
    asio::ip::address_v4 listenhost_address;
    asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<tcp_bridge> session_;
    unsigned short server_port_;
    std::string server_host_;

    tcp_mitm_factory &mitm_factory_;
  };
};

#endif // _TCP_BRIDGE_H_
