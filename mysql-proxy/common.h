#ifndef _COMMON_H_
#define _COMMON_H_

#include <asio.hpp>

using socket_type = asio::ip::tcp::socket;
std::string identity(const socket_type &s);

template <typename Base, typename ...Args>
class factory {
public:
  virtual ~factory() = default;
  virtual std::unique_ptr<Base> build(Args ...args) = 0;
};

template <typename T, typename TBase, typename Parent, typename ...Args>
class concrete_factory : public Parent {
public:
  virtual ~concrete_factory() override = default;

  virtual std::unique_ptr<TBase> build(Args ...args) override {
    return std::make_unique<T>(args...);
  }
};

#endif // _COMMON_H_
