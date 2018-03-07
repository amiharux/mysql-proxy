#include "tcp_bridge.h"

#include "logger.h"

//////////////////////////////////////////////////////////////////////////
// class tcp_bridge
//////////////////////////////////////////////////////////////////////////

tcp_bridge::tcp_bridge(
      asio::io_service& ios
    , on_data_cb on_client_data /*= dummy_cb*/
    , on_data_cb on_server_data /*= dummy_cb*/) 
  : client_socket_(ios)
  , server_socket_(ios)
  , on_client_data_(on_client_data)
  , on_server_data_(on_server_data)
{
  LOG_ID(identity()) << "bridge::construct";
}

tcp_bridge::~tcp_bridge()
{
  LOG_ID(identity()) << "bridge::destruct";
}

tcp_bridge::socket_type& tcp_bridge::client_socket()
{
  return client_socket_;
}

tcp_bridge::socket_type& tcp_bridge::server_socket()
{
  return server_socket_;
}

std::string tcp_bridge::identity() const
{
  std::stringstream ss;
  try {
    auto le = client_socket_.remote_endpoint();
    ss << le.address().to_string() << "::" << le.port();
  }
  catch (const std::system_error &err) {
    ss << "<" << err.code() << ">";
  }
  return ss.str();
}

void tcp_bridge::start(const std::string& server_host, unsigned short server_port)
{
  LOG_ID(identity()) << "bridge::start";
  server_socket_.async_connect(
    asio::ip::tcp::endpoint(asio::ip::address::from_string(server_host), server_port),
    [self = shared_from_this()](const asio::error_code& error) {
      self->handle_server_connect(error);
    }
  );
}

void tcp_bridge::handle_server_connect(const asio::error_code& error)
{
  if (error) { return close(); }
  LOG_ID(identity()) << "bridge::handle_server_connect";

  async_read_from_server(error);
  async_read_from_client(error);
}

void tcp_bridge::async_write_to_client(const asio::error_code& error, size_t bytes)
{
  if (error) { return close(); }
  LOG_ID(identity()) << "bridge::async_write_to_client";

  async_write(client_socket_, asio::buffer(server_data_, bytes),
    [self = shared_from_this()](const asio::error_code& error, size_t) {
      self->async_read_from_server(error);
    }
  );
}

void tcp_bridge::async_read_from_server(const asio::error_code& error)
{
  if (error) { return close(); }
  LOG_ID(identity()) << "bridge::async_read_from_server";

  server_socket_.async_read_some(asio::buffer(server_data_, max_data_length),
    [self = shared_from_this()](const asio::error_code& error, size_t bytes) {
      self->on_server_data_(self->server_data_, bytes);
      self->async_write_to_client(error, bytes);
    }
  );
}

void tcp_bridge::async_write_to_server(const asio::error_code& error, size_t bytes)
{
  if (error) { return close(); }
  LOG_ID(identity()) << "bridge::async_write_to_server";

  async_write(server_socket_, asio::buffer(client_data_, bytes),
    [self = shared_from_this()](const asio::error_code& error, size_t) {
      self->async_read_from_client(error);
    }
  );
}

void tcp_bridge::async_read_from_client(const asio::error_code& error)
{
  if (error) { return close(); }
  LOG_ID(identity()) << "bridge::async_read_from_client";

  client_socket_.async_read_some(asio::buffer(client_data_, max_data_length),
    [self = shared_from_this()](const asio::error_code& error, size_t bytes) {
      self->on_client_data_(self->client_data_, bytes);
      self->async_write_to_server(error, bytes);
    }
  );
}

void tcp_bridge::close()
{
  std::lock_guard<std::mutex> lock(mutex_);

  LOG_ID(identity()) << "bridge::close";

  if (client_socket_.is_open()) {
    client_socket_.close();
  }

  if (server_socket_.is_open()) {
    server_socket_.close();
  }
}


//////////////////////////////////////////////////////////////////////////
// class tcp_bridge::acceptor
//////////////////////////////////////////////////////////////////////////

tcp_bridge::acceptor::acceptor(
      asio::io_service& io_service
    , const std::string& listen_host
    , unsigned short listen_port
    , const std::string& server_host
    , unsigned short server_port
    , on_data_cb on_client_data /*= dummy_cb*/
    , on_data_cb on_server_data /*= dummy_cb*/) 
  : io_service_(io_service)
  , listenhost_address(asio::ip::address_v4::from_string(listen_host))
  , acceptor_(io_service_, asio::ip::tcp::endpoint(listenhost_address, listen_port))
  , server_port_(server_port)
  , server_host_(server_host)
  , on_client_data_(on_client_data)
  , on_server_data_(on_server_data)
{

}

bool tcp_bridge::acceptor::accept_connections()
{
  LOG << "bridge::acceptor::accept_connections";
  try {
    session_ = std::make_shared<tcp_bridge>(io_service_, on_client_data_, on_server_data_);

    acceptor_.async_accept(session_->client_socket(),
      [&](const asio::error_code& error) {
        handle_accept(error);
      }
    );
  }
  catch (std::exception& e) {
    LOG << "acceptor exception: " << e.what() << std::endl;
    return false;
  }

  return true;
}

void tcp_bridge::acceptor::handle_accept(const asio::error_code& error)
{
  LOG_ID(session_->identity()) << "bridge::acceptor::handle_accept";
  if (!error) {
    session_->start(server_host_, server_port_);

    if (!accept_connections()) {
      LOG << "Failure during call to accept." << std::endl;
    }
  }
  else {
    LOG << "Error: " << error.message() << std::endl;
  }
}

