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
   net::steady_timer timer(io_context);

   {
      auto work = net::make_work_guard(io_context);

      t1 = std::thread([&io_context](){ io_context.run(); });
      t2 = std::thread([&io_context](){ io_context.run(); });

      timer.expires_after(2s);
      timer.async_wait([](auto ec)
                       {
                          std::cout << "timer handle 1 : " << !!ec << "\n";
                       }
         );

      std::cout << "sleeping..." << std::endl;
      std::this_thread::sleep_for(1s);

      timer.async_wait([](auto ec)
                       {
                          std::cout << "timer handle 2 : " << !!ec << "\n";
                       }
         );

      
      net::post(io_context,
                []()
                {
                   std::cout << "hi C++Now 2017!\n";
                   std::this_thread::sleep_for(1s);
                }
         );

      net::post(io_context,
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
