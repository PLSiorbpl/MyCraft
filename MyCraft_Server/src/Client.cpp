#include <iostream>
#include <asio.hpp>
#include <thread>
#include <chrono>

using asio::ip::tcp;

int main() {
    try {
        asio::io_context io;
        tcp::socket socket(io);

        socket.connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), 25565));
        std::cout << "Connected to server.\n";

        float pos[3] = {0, 0, 0};

        while (true) {
            // symulacja ruchu
            pos[0] += 0.1f;
            pos[1] = 5.0f;
            pos[2] += 0.05f;

            asio::error_code ec;

            // wyślij pozycję
            asio::write(socket, asio::buffer(pos, sizeof(pos)), ec);
            if (ec) break;

            // odbierz echo
            float recv[3];
            size_t len = socket.read_some(asio::buffer(recv, sizeof(recv)), ec);
            if (ec) break;

            std::cout << "Server says pos: " << recv[0] << " " << recv[1] << " " << recv[2] << "\n";

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
