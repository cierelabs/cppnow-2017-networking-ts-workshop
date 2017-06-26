#include <experimental/net>
#include <string>
#include <iostream>
#include <system_error>

using namespace std::literals::string_literals;
namespace net = std::experimental::net;
using net::ip::tcp;

int main()
{
   net::io_context io_context;
   tcp::socket socket(io_context);
   tcp::resolver resolver(io_context);

   net::connect(socket,
                resolver.resolve("www.boost.org", "http"));

   for(auto v : { "GET / HTTP/1.0\r\n"s
                , "Host: www.boost.org\r\n"s
                , "Accept: */*\r\n"s
                , "Connection: close\r\n\r\n"s } )
   {
      net::write(socket, net::buffer(v));
   }

   std::string header;
   net::read_until(socket,
                   net::dynamic_buffer(header),
                   "\r\n\r\n");
   
   std::error_code e;
   std::string body;

   net::read(socket,
             net::dynamic_buffer(body),
             e);

   std::cout << "Header:\n" << header 
             << "Body:\n" << body
             << "Error code: " << e.message() << std::endl;
}
