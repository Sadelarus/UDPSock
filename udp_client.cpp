#include <iostream>
#include <boost/asio.hpp>
#include <boost/ref.hpp>
#include <boost/program_options.hpp>
#include "includes.h"

namespace {
    unData data;
    boost::asio::ip::udp::endpoint sender_endpoint;

    void read_handle(boost::asio::ip::udp::socket& socket,
                     const boost::system::error_code& err,
                     std::size_t bytes_transferred) {
        if (err) {
            std::cerr << "Error (receive): " << err.message() << std::endl;
            return;
        }

        std::cout << "Received result [" << data.res.res << "]" << std::endl;
    }

    void send_handle(boost::asio::ip::udp::socket& socket,
                     const boost::system::error_code& err,
                     const boost::asio::ip::udp::endpoint &destination,
                     std::size_t bytes_transferred) {
        if (err) {
            std::cerr << "Error (send): " << err.message() << std::endl;
            return;
        }

        socket.async_receive_from(boost::asio::buffer(&data.args, sizeof(data.args)),
                                  sender_endpoint,
                             [&socket](const boost::system::error_code& error,
                                                std::size_t bytes_transferred) {
                                    read_handle(boost::ref(socket), error, bytes_transferred);
                             });
    }

    void send_request(boost::asio::ip::udp::socket& socket,
                      const boost::asio::ip::udp::endpoint &destination) {

        std::cout << "Ready to send the data for processing ["
                  <<  data.args.a << " " << to_underlying(data.args.operation)  << " " << data.args.b << "]" << std::endl;

        socket.async_send_to(boost::asio::buffer(&data.args, sizeof(data.args)),
                             destination,
                          [&socket, &destination](const boost::system::error_code& error,
                                             std::size_t bytes_transferred) {
                              send_handle(boost::ref(socket),
                                          error,
                                          boost::ref(destination),
                                          bytes_transferred);
                           });
    }
}

int main(int argc, char** argv) {

    std::string serverAddr = "127.0.0.1";
    std::uint16_t port = 8080;
    namespace po = boost::program_options;
    po::options_description desc("Application options");
    std::string operation;
    desc.add_options()
    ("help,h", "Show help")
    ("arg-1,a", po::value<double>(&data.args.a), "Argument #1")
    ("arg-2,A", po::value<double>(&data.args.b), "Argument #2")
    ("operation,o", po::value<std::string>(&operation), "Operation")
    ("server-adr,s", po::value<std::string>(&serverAddr), "Server address")
    ("server-port,p", po::value<std::uint16_t>(&port), "Server port");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);


    if (vm.count("help") || argc == 1) {
        std::cout << desc << std::endl;
        return 1;
    }

    if (vm.count("arg-1") == 0 || vm.count("arg-2") == 0 || vm.count("operation") == 0) {
        std::cout << "Parameters were not passed" << std::endl;
        std::cout << desc << std::endl;
        return 1;
    }

    data.args.operation = operation_from_string(operation);

    boost::asio::io_service io_service;

    boost::asio::ip::udp::socket socket(io_service);
    boost::asio::ip::udp::endpoint destination(boost::asio::ip::make_address(serverAddr), port);
    boost::system::error_code ec;
    socket.open(destination.protocol(), ec);
    if (ec)
    {
        std::cerr << ec << std::endl;
    }

    send_request(socket, destination);

    io_service.run();
    return 0;
}
