#include <experimental/net>
#include <string>
#include <iostream>
#include <thread>
#include <future>

using namespace std::chrono_literals;
namespace net = std::experimental::net;


int main()
{
   std::thread t1;
   std::thread t2;
   net::io_context io_context;
   net::strand<net::io_context::executor_type> strand(io_context.get_executor());

   {
      auto work = net::make_work_guard(io_context);

      t1 = std::thread([&io_context](){ io_context.run(); });
      t2 = std::thread([&io_context](){ io_context.run(); });

      std::cout << "sleeping..." << std::endl;
      std::this_thread::sleep_for(1s);

      net::post(strand,
                []()
                {
                   std::cout << "hi C++Now 2017!\n";
                   std::this_thread::sleep_for(1s);
                }
         );

      net::post(strand,
                []()
                {
                   std::cout << "Lightning Talks!\n";
                   std::this_thread::sleep_for(1s);
                }
         );
   }
   std::cout << "thread joining...\n";
   t1.join();
   t2.join();
   std::cout << "thread joined...\n";
}
