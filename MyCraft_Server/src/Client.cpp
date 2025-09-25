#include <iostream>
#include <asio.hpp>
#include <thread>
#include <chrono>
#include <glm/glm.hpp>

using asio::ip::tcp;

int main() {
    std::string ip;
    short port;
    std::cout << "Gibe ip";
    std::cin >> ip;
    std::cout << "Gibe port";
    std::cin >> port;
    try {
        asio::io_context io;
        tcp::socket socket(io);

        tcp::resolver resolver(io);
        auto results = resolver.resolve(ip, std::to_string(port));
        asio::connect(socket, results);
        //socket.connect(tcp::endpoint(asio::ip::make_address(ip), port)); // LocalHost rn
        std::cout << "Connected to server.\n";

        glm::vec3 Pos;

        while (true) {
            // symulacja ruchu
            Pos += glm::vec3(1.0f, 0.5f, 2.5f);

            asio::error_code ec;

            // wyślij pozycję
            asio::write(socket, asio::buffer(&Pos, sizeof(Pos)), ec);
            if (ec) break;

            // odbierz echo
            glm::vec3 RecvPos;
            size_t len = socket.read_some(asio::buffer(&RecvPos, sizeof(RecvPos)), ec);
            if (ec) break;

            std::cout << "Server says pos: " << RecvPos.x << " " << RecvPos.y << " " << RecvPos.z << "\n";

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
