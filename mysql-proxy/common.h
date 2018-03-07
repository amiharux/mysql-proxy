#ifndef _COMMON_H_
#define _COMMON_H_

#include <asio.hpp>

using socket_type = asio::ip::tcp::socket;
std::string identity(const socket_type &s);

#endif // _COMMON_H_
