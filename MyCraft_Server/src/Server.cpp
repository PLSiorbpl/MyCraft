#include <iostream>
#include <asio.hpp>

using asio::ip::tcp;

void TryConnect(asio::io_context &io, tcp::acceptor &acceptor, tcp::socket &socket) {
    std::cout << "Waiting for client...\n";
    acceptor.accept(socket);  // Waits for Client
    std::cout << "Client connected!\n";
}

void MainLoop() {
    asio::io_context io;

    tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 25565));
    std::cout << "Server running on port 25565...\n";

    tcp::socket socket(io);

    TryConnect(io, acceptor, socket);

    while (true) {
            float pos[3]; // x, y, z
            asio::error_code ec;

            // Reciving
            size_t len = asio::read(socket, asio::buffer(pos, sizeof(pos)), ec);
            if (ec) {
                std::cout << "Client disconnected.\n";
                socket.close();
                socket = tcp::socket(io);
                TryConnect(io, acceptor, socket);
                continue;
            }

            std::cout << "Got pos: " << pos[0] << " " << pos[1] << " " << pos[2] << "\n";

            // Sending Back
            asio::write(socket, asio::buffer(pos, sizeof(pos)), ec);
            if (ec) {
                socket.close();
                socket = tcp::socket(io);
                TryConnect(io, acceptor, socket);
                continue;
            }
        }
}

int main() {
    MainLoop();
}