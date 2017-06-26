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


class web_page_getter
{
public:
   using page_type = std::tuple<std::string, std::string>;

   web_page_getter(net::io_context & io_context)
      : io_context_(io_context)
      , socket_(io_context_)
   {}

   std::future<page_type> get_page(std::string const & host,
                                   std::string const & port)
   {
      promise_ = std::promise<page_type>{};
      socket_ = tcp::socket{io_context_};
      header_ = std::string{};
      body_ = std::string{};

      connect(host, port);

      return promise_.get_future();
   }

private:
   net::io_context & io_context_;
   tcp::socket socket_;
   std::string header_;
   std::string body_;
   std::promise<page_type> promise_;


   void connect(std::string const & host,
                std::string const & port)
   {
      tcp::resolver resolver(io_context_);
      net::async_connect(socket_,
                         resolver.resolve(host, port),
                         [this](auto ec, auto end_point)
                         {
                            if(!ec)
                            {
                               send_request();
                               read_header();
                            }
                            else
                            {
                               promise_.set_exception(
                                  std::make_exception_ptr(ec)
                                  );
                            }
                         });
   }
   
   void read_header()
   {
      net::async_read_until(socket_,
                            net::dynamic_buffer(header_),
                            "\r\n\r\n",
                            [this](auto ec, auto bytes_trans)
                            {
                               if(!ec)
                               {
                                  read_body();
                               }
                               else
                               {
                                  promise_.set_exception(
                                     std::make_exception_ptr(ec)
                                     );
                               }
                            });
   }

   void read_body()
   {
      net::async_read(socket_,
                      net::dynamic_buffer(body_),
                      net::transfer_all(),
                      [this](auto ec, auto bytes_trans)
                      {
                         if(!ec || ec == net::stream_errc::eof )
                         {
                            promise_.set_value(
                               page_type{std::move(header_),
                                         std::move(body_)}
                               );
                         }
                         else
                         {
                            promise_.set_exception(
                               std::make_exception_ptr(ec)
                               );
                         }
                         std::error_code close_ec;
                         socket_.close(close_ec);
                      });
   }


   void send_request()
   {
      auto request = "GET / HTTP/1.0\r\n"
                     "Host: www.boost.org\r\n"
                     "Accept: */*\r\n"
                     "Connection: close\r\n\r\n"s;

      socket_.async_send(net::buffer(request),
                         [](auto, auto){} );
   }
};


int main()
{
   net::io_context io_context;
   auto work = net::make_work_guard(io_context);
   std::thread t([&io_context](){io_context.run();});

   web_page_getter wpg(io_context);

   auto f = wpg.get_page("www.boost.org", "http");

   try
   {
      auto result = f.get();
      std::cout << "Header:\n" << std::get<0>(result)
                << "Body:\n" << std::get<1>(result) << std::endl;
   }
   catch(std::error_code const & ec)
   {
      std::cout << "Error: " << ec.message() << std::endl;
   }
   
   io_context.stop();
   t.join();
}
