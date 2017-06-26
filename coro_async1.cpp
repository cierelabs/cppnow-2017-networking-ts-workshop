// This is the example that Gor Nishanov put together during the workshop
// that used Coroutines.
//
// Thank you Gor for sharing this with the group!
//
// Compile with VS2017: extra options:
//     /await /std:c++latest -D_WIN32_WINNT=0x0600 -DNET_TS_HAS_CHRONO
//     and /I<where your networking TS includes are>
#include <cstdio>
#include <iostream>
#include <future>
#include <chrono>
#include <string_view>
#include <experimental/timer>
#include <experimental/net>

using namespace std::experimental::net;
using namespace std;
using namespace std::chrono;
namespace net = std::experimental::net;
using net::ip::tcp;

// ---------------------------------------------------------------------- //
// begin coroutine adapters:
template <typename Timer, typename R, typename P>
auto async_await(Timer &t, std::chrono::duration<R, P> d) {
  struct Awaiter {
    Timer &t;
    std::chrono::duration<R, P> d;
    std::error_code ec;

    bool await_ready() { return d.count() == 0; }
    void await_resume() {
      if (ec)
        throw std::system_error(ec);
    }
    void await_suspend(std::experimental::coroutine_handle<> coro) {
      t.expires_after(d);
      t.async_wait([this, coro](auto ec) {
        this->ec = ec;
        coro.resume();
      });
    }
  };
  return Awaiter{ t, d };
}

template <typename Socket, typename EndpointSequence>
auto async_connect(Socket &s, const EndpointSequence &endpoints) {
  struct Awaiter {
    Socket &s;
    const EndpointSequence &endpoints;
    std::error_code ec;

    bool await_ready() { return false; }
    void await_resume() {
      if (ec)
        throw std::system_error(ec);
    }
    void await_suspend(std::experimental::coroutine_handle<> coro) {
      net::async_connect(s, endpoints, [this, coro](auto ec, auto end_point) {
        this->ec = ec;
        coro.resume();
      });
    }
  };
  return Awaiter{s, endpoints};
}

template <typename Socket, typename ConstBufferSequence>
auto async_send(Socket &s, const ConstBufferSequence& buffers)
{
  struct Awaiter {
    Socket &s;
    const ConstBufferSequence& buffers;
    std::error_code ec;
    size_t sz;

    bool await_ready() { return false; }
    auto await_resume() {
      if (ec)
        throw std::system_error(ec);
      return sz;
    }
    void await_suspend(std::experimental::coroutine_handle<> coro) {
      s.async_send(buffers, [this, coro](auto ec, auto sz) {
        this->ec = ec;
        this->sz = sz;
        coro.resume();
      });
    }
  };
  return Awaiter{ s, buffers };
}

template <typename SyncReadStream, typename DynamicBuffer>
auto async_read_until(SyncReadStream &s, DynamicBuffer &&buffers,
                      string delim) {
  struct Awaiter {
    SyncReadStream &s;
    DynamicBuffer &&buffers;
    string delim;

    std::error_code ec;
    size_t sz;

    bool await_ready() { return false; }
    auto await_resume() {
      if (ec)
        throw std::system_error(ec);
      return sz;
    }
    void await_suspend(std::experimental::coroutine_handle<> coro) {
      net::async_read_until(s, std::move(buffers), delim,
                            [this, coro](auto ec, auto sz) {
                              this->ec = ec;
                              this->sz = sz;
                              coro.resume();
                            });
    }
  };
  return Awaiter{ s, std::forward<DynamicBuffer>(buffers), delim };
}
// end coroutine adapters:
// ---------------------------------------------------------------------- //

// User code:

future<void> sleeper(steady_timer & timer) {
  puts("sleeping...");
  co_await async_await(timer, 1ms);
  puts("coroutine, yay!");
}
future<void> poster(io_context &io, string const &host, string const &port) {
  tcp::socket socket(io);
  tcp::resolver resolver(io);

  co_await async_connect(socket, resolver.resolve(host, port));

  puts("connected");

  auto content = "{\"name\":\"Gor Nishanov\",\"psk\":\"hidden\"}"s;

  auto request =
    "POST /register HTTP/1.1\r\n"
    "Host: cpp.ciere.cloud:8000\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: "s + std::to_string(content.size()) + "\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "\r\n"s + content;

  co_await async_send(socket, net::buffer(request));

  puts("sent");
  std::string header;
  co_await async_read_until(socket, net::dynamic_buffer(header), "\r\n\r\n");

  puts("response:");
  puts(header.c_str());
}



int main() {
  std::cout << "{\"name\":\"Gor Nishanov\",\"psk\":\"hidden\"}"s.length() << "\n";

  io_context io;
  steady_timer timer(io);
  auto fut1 = sleeper(timer);
  auto fut2 = poster(io, "cpp.ciere.cloud", "8000");

  io.run();

  try {
    fut1.get();
    fut2.get();
  }
  catch (std::exception const& e) {
    cout << "oops: " << e.what() << "\n";
  }
  return 0;
}


