#include <experimental/net>
#include <string>
#include <iostream>
#include <thread>
#include <system_error>
#include <future>
#include <tuple>

using namespace std::chrono_literals;
using namespace std::literals::string_literals;
namespace net = std::experimental::net;
using net::ip::tcp;


int main()
{
   net::io_context io_context;
   std::thread t([&io_context](){io_context.run();});

   auto resolver = tcp::resolver(io_context);
   auto resolve = resolver.async_resolve("www.boost.org", "http",
                                         net::use_future);

   tcp::socket socket(io_context);
   auto connect = net::async_connect(socket, resolve.get(),
                                     net::use_future);

   auto request = "GET / HTTP/1.0\r\nHost: www.boost.org\r\n"
                  "Accept: */*\r\nConnection: close\r\n\r\n"s;

   socket.async_send(net::buffer(request), [](auto, auto){});

   std::string header;
   auto header_read = net::async_read_until(socket, net::dynamic_buffer(header),
                                            "\r\n\r\n",
                                            net::user_future);
   // do some stuff ....
   if(header_read.get() <= 2) { std::cout << "no header\n"; }

   std::string body;
   auto body_read = net::async_read(socket, net::dynamic_buffer(body),
                                    net::transfer_all(),
                                    net::use_future);
   // do some stuff ....
   body_read.get();
   std::cout << "Header:\n" << header << "Body:\n" << body << "\n";
   t.join();
}

