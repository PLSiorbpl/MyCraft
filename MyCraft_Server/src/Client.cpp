#include <iostream>
#include <asio.hpp>
#include <thread>
#include <chrono>
#include <glm/glm.hpp>

using asio::ip::tcp;

int main() {
    std::string ip;
    std::string Name;
    short port;
    std::cout << "Ip Address: ";
    std::cin >> ip;
    std::cout << "Port: ";
    std::cin >> port;
    try {
        asio::io_context io;
        tcp::socket socket(io);
        tcp::resolver resolver(io);
        auto endpoints = resolver.resolve(ip, std::to_string(port));
        asio::connect(socket, endpoints);

        std::cout << "Enter Name: ";
        std::cin >> Name;
        if (Name.empty()) return 0;
        asio::write(socket, asio::buffer(Name));
        std::cout << "Connected to server\n";

        char ReciveMesg[1024];
        
        std::thread receive_thread([&]() {
            asio::error_code ec;
            while (true) {
                size_t len = socket.read_some(asio::buffer(ReciveMesg), ec);
                if (ec) {
                    std::cout << "Disconnected from server\n";
                    break;
                }
                    std::cout << std::string(ReciveMesg, len) << "\n> ";
                    std::cout.flush();
            }
        });

        while (true) {
            std::string Message;
            std::cout << "> ";
            std::getline(std::cin, Message);
        
            asio::error_code ec;
            asio::write(socket, asio::buffer(Message), ec);
            if (ec) {
                std::cout << "Disconnected from server\n";
            }
        }

        receive_thread.join();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
