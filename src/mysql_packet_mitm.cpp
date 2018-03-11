#include "mysql_packet_mitm.h"

#include "logger.h"
#include "common.h"

#include <string>
#include <functional>
#include <iomanip>

namespace mysql_proxy {

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

  if (_client_packets.empty() || _client_packets.back().has_header() && _client_packets.back().has_body()) {
    _client_packets.emplace_back();
  }

  auto &last_package = _client_packets.back();
  size_t can_be_added_bytes = last_package.has_header()
    ? std::min(bytes, last_package.body_bytes_left())      // has no body
    : std::min(bytes, last_package.header_bytes_left());   // has no header
  last_package.insert(last_package.end(), data, data + can_be_added_bytes);

  if (last_package.has_header()
    && _client_packets.size() > 1 
    && last_package.sequence() != std::prev(std::prev(std::end(_client_packets)))->sequence() + 1) {
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
  if (!last_package.has_body()) {
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
    LOG_TRACE(identity(_client_socket)) << "    " << el.payload_length() << " bytes";
  }
}

//////////////////////////////////////////////////////////////////////////
// class mysql_com_query_mitm
//////////////////////////////////////////////////////////////////////////
void mysql_com_query_mitm::handle_client_packet(const std::vector<mysql_packet> &cmds) {
  mysql_client_packet_mitm::handle_client_packet(cmds);
  if (cmds.size() > 0 && cmds.front().opcode() == mysql_packet::OPCODE_COM_QUERY) {
    LOG_INFO log{ identity(_client_socket) };
    log << "COM_QUERY: ";
    for (auto &el : cmds) { log << el.command(); }
  }
}

}
