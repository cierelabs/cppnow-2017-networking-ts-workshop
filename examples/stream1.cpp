#include <experimental/net>
#include <chrono>
#include <string>
#include <iostream>

using namespace std::chrono_literals;
namespace net = std::experimental::net;

int main()
{
   net::ip::tcp::iostream s;

   s.expires_after(5s);
   s.connect("www.boost.org", "https");
   
   if(!s)
   {
      std::cout << "error: " << s.error().message() << std::endl;
      return -1;
   }

   s << "GET / HTTP/1.0\r\n";
   s << "Host: www.boost.org\r\n";
   s << "Accept: */*\r\n";
   s << "Connection: close\r\n\r\n";

   std::string header;
   while(s && std::getline(s, header) && header != "\r")
      std::cout << header << "\n";

   std::cout << s.rdbuf();
}
