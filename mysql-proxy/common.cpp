#include "common.h"

#include <string>
#include <sstream>
#include <stdexcept>

std::string identity(const socket_type &s)
{
  std::stringstream ss;
  try {
    auto le = s.remote_endpoint();
    ss << le.address().to_string() << "::" << le.port();
  }
  catch (const std::system_error &err) {
    ss << "<" << err.code() << ">";
  }
  return ss.str();
}
