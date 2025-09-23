#include <iostream>
#include <asio.hpp>
#include <deque>
#include <glm/glm.hpp>


using asio::ip::tcp;

struct Client {
    tcp::socket socket;
    std::vector<char> buffer;
    glm::vec3 Pos;

    Client(asio::io_context& io) : socket(io), buffer(1024) {}
};

class Server {
    private:
    tcp::acceptor acceptor;
    asio::io_context& io;
    short port;
    std::deque<Client> Players;

    public:
        Server(asio::io_context& io, short port) : acceptor(io, tcp::endpoint(tcp::v4(), port)), io(io), port(port) {}

        void TryConnect() {
            Players.emplace_back(io);
            Client& player = Players.back();

            acceptor.async_accept(player.socket, 
                [this, &player](std::error_code er) {
                    if (!er) {
                        std::cout << "New Client Connected \n";
                        Client_Loop(player);
                    } else {
                        std::cout << "Accept Failed \n";
                        Players.pop_back();
                    }
                    // Wait for New Client
                    TryConnect();
                }
            );
        }

        void Client_Loop(Client& Player) {
            Player.socket.async_read_some(asio::buffer(&Player.Pos, sizeof(Player.Pos)),
                [this, &Player](std::error_code er, std::size_t len) {
                    if (!er) {
                        std::cout << "Got " << Player.Pos.x << " " <<
                        Player.Pos.y << " " << Player.Pos.z << "\n";

                        Player.socket.async_write_some(asio::buffer(&Player.Pos, sizeof(Player.Pos)),
                            [this, &Player](std::error_code er, std::size_t len) {
                                if (!er) {
                                    std::cout << "Send Pos \n";
                                } else {
                                    std::cout << "Send failed: " << er.message() << "\n";
                                }
                            }
                        );
                        Client_Loop(Player);

                    } else {
                        // Handle Disconect
                        std::cout << "Client Disconected \n";
                    }
                }
            );
        }
};

int main() {
    try {
        asio::io_context io;
        Server server(io, 25565);
        server.TryConnect();
        io.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}