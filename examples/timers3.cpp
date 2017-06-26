#include <experimental/net>
#include <string>
#include <iostream>
#include <thread>
#include <future>

using namespace std::chrono_literals;
namespace net = std::experimental::net;


int main()
{
   std::thread t;
   net::io_context io_context;

   auto work = net::make_work_guard(io_context);

   t = std::thread([&io_context](){ io_context.run(); });

   std::cout << "sleeping..." << std::endl;
   std::this_thread::sleep_for(1s);

   net::post(io_context,
             []()
             {
                std::cout << "hi C++Now 2017!\n";
             }
      );

   std::cout << "thread joining...\n";
   t.join();
   std::cout << "thread joined...\n";
}
