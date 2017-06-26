#include <experimental/net>
#include <string>
#include <iostream>
#include <thread>
#include <future>


using namespace std::chrono_literals;
using namespace std::literals::string_literals;
namespace net = std::experimental::net;
using net::ip::tcp;

int main()
{
   net::io_context io_context;

   net::post(io_context,
             []()
             {
                std::cout << "hi C++Now 2017!\n";
             }
      );
}
