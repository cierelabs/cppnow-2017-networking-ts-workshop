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

   std::thread t([&io_context](){ io_context.run(); });

   std::cout << "sleeping..." << std::endl;
   std::this_thread::sleep_for(2s);

   net::post(io_context,
             []()
             {
                std::cout << "hi C++Now 2017!\n";
             }
      );

   t.join();
   std::cout << "thread joined...\n";
}
