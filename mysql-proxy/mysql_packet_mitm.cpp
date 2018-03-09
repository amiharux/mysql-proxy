#include "mysql_packet_mitm.h"

#include "logger.h"
#include "common.h"

#include <string>
#include <functional>
#include <iomanip>


const unsigned OPCODE_COM_QUERY = 0x03;

const size_t HEADER_SIZE = 4;
size_t payload_length(const mysql_packet_mitm::mysql_packet &data) {
  return data[0] + (data[1] << 8) + (data[2] << 16);
}

unsigned sequence(const mysql_packet_mitm::mysql_packet &data) {
  return data[3];
}

unsigned opcode(const mysql_packet_mitm::mysql_packet &data) {
  return data[4];
}

std::string command(const mysql_packet_mitm::mysql_packet &data) {
  return std::string(data.begin() + 5, data.end());
}

bool has_header(const mysql_packet_mitm::mysql_packet &data) {
  return data.size() >= HEADER_SIZE;
}

bool has_body(const mysql_packet_mitm::mysql_packet &data) {
  return has_header(data) && data.size() >= payload_length(data) + HEADER_SIZE;
}

size_t header_bytes_left(const mysql_packet_mitm::mysql_packet &data) {
  return has_header(data) ? 0 : HEADER_SIZE - data.size();
}

size_t body_bytes_left(const mysql_packet_mitm::mysql_packet &data) {
  return has_body(data) ? 0 : payload_length(data) + HEADER_SIZE - data.size();
}


//////////////////////////////////////////////////////////////////////////
// class mysql_packet_mitm
//////////////////////////////////////////////////////////////////////////

void mysql_packet_mitm::on_client_data_impl(unsigned char *data, size_t bytes) {
  LOG_TRACE(identity(_client_socket)) << "Got " << bytes << " bytes from client";
}

void mysql_packet_mitm::on_server_data_impl(unsigned char *data, size_t bytes) {
  LOG_TRACE(identity(_client_socket)) << "Got " << bytes << " bytes from client";
}

//////////////////////////////////////////////////////////////////////////
// class mysql_client_packet_mitm
//////////////////////////////////////////////////////////////////////////

mysql_client_packet_mitm::~mysql_client_packet_mitm() {
  handle_client_packet(_client_packets);
  _client_packets.clear();
}

void mysql_client_packet_mitm::on_client_data_impl(unsigned char *data, size_t bytes) {
  if (bytes == 0) { return; }

  if (_client_packets.empty() || has_header(_client_packets.back()) && has_body(_client_packets.back())) {
    _client_packets.emplace_back();
  }

  auto &last_package = _client_packets.back();
  size_t can_be_added_bytes = has_header(last_package)
    ? std::min(bytes, body_bytes_left(last_package))      // has no body
    : std::min(bytes, header_bytes_left(last_package));   // has no header
  last_package.insert(last_package.end(), data, data + can_be_added_bytes);

  if (has_header(last_package) 
    && _client_packets.size() > 1 
    && sequence(last_package) != (sequence(*std::prev(std::prev(std::end(_client_packets)))) + 1)) {
    std::vector<mysql_packet> handled_packets(_client_packets.size() - 1);
    std::swap_ranges(handled_packets.begin(), handled_packets.end(), _client_packets.begin());
    handle_client_packet(handled_packets);
    std::vector<mysql_packet>({ last_package }).swap(_client_packets);
  }

  on_client_data_impl(data + can_be_added_bytes, bytes - can_be_added_bytes);
}

void mysql_client_packet_mitm::on_server_data_impl(unsigned char *data, size_t bytes) {
  if (_client_packets.empty()) { return; }

  auto &last_package = _client_packets.back();
  if (!has_body(last_package)) {
    std::vector<mysql_packet> handled_packets(_client_packets.size() - 1);
    std::swap_ranges(handled_packets.begin(), handled_packets.end(), _client_packets.begin());
    handle_client_packet(handled_packets);
    std::vector<mysql_packet>({ last_package }).swap(_client_packets);
  }
  else {
    handle_client_packet(_client_packets);
    _client_packets.clear();
  }
}

void mysql_client_packet_mitm::handle_client_packet(const std::vector<mysql_packet> &cmds) {
  LOG_TRACE(identity(_client_socket)) << "Got " << cmds.size() << " packets from client: ";
  for (auto &el : cmds) {
    LOG_TRACE(identity(_client_socket)) << "    " << payload_length(el) << " bytes";
  }
}

//////////////////////////////////////////////////////////////////////////
// class mysql_com_query_mitm
//////////////////////////////////////////////////////////////////////////
void mysql_com_query_mitm::handle_client_packet(const std::vector<mysql_packet> &cmds) {
  mysql_client_packet_mitm::handle_client_packet(cmds);
  if (cmds.size() > 0 && opcode(cmds.front()) == OPCODE_COM_QUERY) {
    LOG_INFO log{ identity(_client_socket) };
    log << "COM_QUERY: ";
    for (auto &el : cmds) { log << command(el); }
  }
}
