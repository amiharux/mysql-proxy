#include "tcp_size_logger.h"
#include "logger.h"

#include <cctype>


tcp_size_logger::tcp_size_logger(const socket_type &client_socket) 
  : tcp_mitm(client_socket)
{
  LOG_ID(identity(_client_socket)) << "[tcp_size_logger::construct]";
}

tcp_size_logger::~tcp_size_logger()
{
  LOG_ID(identity(_client_socket)) << "[tcp_size_logger::destruct]";
}

constexpr char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

std::string hexStr(unsigned char *data, int len)
{
  std::string s(3*len, ' ');
  for (int i = 0; i < len; ++i) {
    s[3 * i] = hexmap[(data[i] & 0xF0) >> 4];
    s[3 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}

std::string reprStr(unsigned char *data, int len)
{
  std::string s(len, ' ');
  for (int i = 0; i < len; ++i) {
    if (isgraph(data[i])) {
      s[i] = data[i];
    }
  }
  return s;
}

// Looking for COM_QUERY(0x03)
void tcp_size_logger::on_client_data_impl(unsigned char* data, size_t bytes)
{
  LOG_ID(identity(_client_socket)) << "[tcp_size_logger::on_client_data_impl] Got " << bytes << " bytes";
  LOG << ">>>>>>";
  LOG << hexStr(data, bytes);
  LOG << ">>>>>>";
  LOG << reprStr(data, bytes);
  LOG << ">>>>>>";
}

void tcp_size_logger::on_server_data_impl(unsigned char* data, size_t bytes)
{
  LOG_ID(identity(_client_socket)) << "[tcp_size_logger::on_server_data_impl] Got " << bytes << " bytes";
}
