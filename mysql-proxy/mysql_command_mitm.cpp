#include "mysql_command_mitm.h"

#include "logger.h"
#include "common.h"

#include <functional>

constexpr char hexmap[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

std::string hexStr(const unsigned char *data, size_t len)
{
  std::string s(3 * len, ' ');
  for (size_t i = 0; i < len; ++i) {
    s[3 * i] = hexmap[(data[i] & 0xF0) >> 4];
    s[3 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}

std::string reprStr(const unsigned char *data, size_t len)
{
  std::string s(len, ' ');
  for (size_t i = 0; i < len; ++i) {
    if (isgraph(data[i])) {
      s[i] = data[i];
    }
  }
  return s;
}


void feed_impl(unsigned char *data, size_t bytes, std::string &buf, unsigned &opcode, size_t &bytes_left, std::function<void(unsigned, const std::string &)> cb) {
  if (bytes_left == 0) {
    buf.clear();
    opcode = data[4];
    bytes_left = data[0] + (data[1] << 2) + (data[2] << 4);
    bytes_left -= bytes - 4;
    buf.append((const char*)data + 5, bytes - 5);
  }
  else {
    bytes_left -= bytes;
    buf.append((const char*)data, bytes);
  }

  if (bytes_left == 0) {
    cb(opcode, buf);
    opcode = 0;
  }
}

//////////////////////////////////////////////////////////////////////////
// class mysql_command_mitm
//////////////////////////////////////////////////////////////////////////

void mysql_command_mitm::on_client_data_impl(unsigned char *data, size_t bytes) {
  feed_impl(data, bytes, _client_data, _client_opcode, _client_bytes_left, [&](unsigned opcode, const std::string &data) {
    handle_client_command(opcode, data);
  });
}

void mysql_command_mitm::handle_client_command(unsigned opcode, const std::string &s) {
  LOG_TRACE(identity(_client_socket)) << ">>>>>>";
  LOG_TRACE(identity(_client_socket)) << hexStr((const unsigned char *)s.data(), s.size());
  LOG_TRACE(identity(_client_socket)) << ">>>>>>";
  LOG_TRACE(identity(_client_socket)) << reprStr((const unsigned char *)s.data(), s.size());
  LOG_TRACE(identity(_client_socket)) << ">>>>>>";
}

void mysql_command_mitm::on_server_data_impl(unsigned char *data, size_t bytes) {
  feed_impl(data, bytes, _server_data, _server_opcode, _server_bytes_left, [&](unsigned opcode, const std::string &data) {
    handle_server_command(opcode, data);
  });
}

//////////////////////////////////////////////////////////////////////////
// class mysql_com_query_mitm
//////////////////////////////////////////////////////////////////////////

void mysql_com_query_mitm::handle_client_command(unsigned opcode, const std::string &s) {
  if (opcode == 0x03) {
    LOG_TRACE(identity(_client_socket)) << "COM_QUERY FOUND: " << s;
  }
}
