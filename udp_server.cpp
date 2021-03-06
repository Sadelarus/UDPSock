 //importing libraries
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <iostream>
#include <cstdint>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind/bind.hpp>
#include <boost/program_options.hpp>
#include <boost/function.hpp>
#include <stdexcept>

#include "includes.h"

#if BOOST_VERSION >= 107000
#define GET_IO_SERVICE(s) ((boost::asio::io_context&)(s).get_executor().context())
#else
#define GET_IO_SERVICE(s) ((s).get_io_service())
#endif

using namespace boost::asio;
using ip::udp;
using std::cout;
using std::endl;

class con_handler : public boost::enable_shared_from_this<con_handler>
{
private:
    enum { max_length = sizeof(CalcData) };
    unData data_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    boost::shared_ptr<udp::socket> io_sock_;
public:
    typedef boost::shared_ptr<con_handler> pointer;

    con_handler(boost::shared_ptr<udp::socket> io_sock)
    :io_sock_(io_sock){
    }

    con_handler(const con_handler&) = delete;
    con_handler(con_handler&&) = delete;
    ~con_handler() = default;

    static pointer create(boost::shared_ptr<udp::socket> io_sock)
    {
        return pointer(new con_handler(io_sock));
    }

    void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
    {
        if (err) {
            std::cerr << "error: " << err.message() << std::endl;
        } else {
            std::cout << "Processing & data send operation both are over." << std::endl;
        }
    }

    static void handle_receive(con_handler::pointer connection,
                               boost::function<void ()> f,
                               std::size_t bytes_transferred,
                               const boost::system::error_code& err)
    {
        if (err) {
            std::cerr << "error: " << err.message() << std::endl;
            return;
        }
        f();
        auto& data = connection->data_;
        std::cout << "[" << bytes_transferred << "] bytes have been received" << std::endl;
        double r = std::numeric_limits<double>::quiet_NaN();
        if(data.args.operation == Operation::Plus) {
            r = data.args.a + data.args.b;
        } else if(data.args.operation == Operation::Minus) {
            r = data.args.a - data.args.b;
        } else if(data.args.operation == Operation::Mult) {
            r = data.args.a * data.args.b;
        } else if(data.args.operation == Operation::Div) {
            r = data.args.a / data.args.b;
        }

        printf("Result: %f %c %f = %f\n",
               data.args.a,
               to_underlying(data.args.operation),
               data.args.b,
               r);

        data.res.res = r;

        connection->io_sock_->async_send_to(boost::asio::buffer(&data.res, sizeof(data.res)),
                               connection->sender_endpoint_,
                            [connection](const boost::system::error_code& error,
                                    std::size_t bytes_transferred) {
                                connection->handle_write(error, bytes_transferred);
                            });

    }

    void run(boost::function<void ()> f) {
        auto p = shared_from_this();
        io_sock_->async_receive_from(boost::asio::buffer(&data_.args, sizeof(data_.args)),
                                     sender_endpoint_,
                                     [p, f](const boost::system::error_code& error,
                                         std::size_t bytes_transferred) {
                                         handle_receive(p, f, bytes_transferred, error);
                                     });
    }
};

class Server
{
private:
    boost::shared_ptr<udp::socket> io_sock_;

    void start_accept()
    {
        con_handler::pointer connection = con_handler::create(io_sock_);
        connection->run([this]() {
            start_accept();
        });
    }

public:
//constructor for accepting connection from client
    Server(boost::asio::io_service& io_service, const std::string &addr, const std::uint16_t port)
    : io_sock_(new udp::socket(io_service,
                               boost::asio::ip::udp::endpoint(boost::asio::ip::make_address(addr.c_str()),
                                                              port)))
    {
        start_accept();
    }

};

int main(int argc, char *argv[])
{
    std::string serverAddr = "127.0.0.1";
    std::uint16_t port = 8080;
    namespace po = boost::program_options;
    po::options_description desc("Application options");
    desc.add_options()
    ("help,h", "Show help")
    ("server-adr,s", po::value<std::string>(&serverAddr), "Server address")
    ("server-port,p", po::value<std::uint16_t>(&port), "Server port");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);


    if (vm.count("help") || argc == 1) {
        std::cout << desc << std::endl;
        return 1;
    }

    try
    {
        boost::asio::io_service io_service;
        Server server(io_service, serverAddr, port);

        io_service.run();
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << endl;
    }

    return 0;
}
