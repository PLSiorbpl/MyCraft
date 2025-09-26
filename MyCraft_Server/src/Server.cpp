#include <iostream>
#include <asio.hpp>
#include <deque>
#include <glm/glm.hpp>


using asio::ip::tcp;

struct Client {
    tcp::socket socket;
    std::string Name = "";
    char buf[1024];

    Client(asio::io_context& io) : socket(io) {}
};

class Server {
    private:
    tcp::acceptor acceptor;
    asio::io_context& io;
    short port;
    std::deque<std::shared_ptr<Client>> Players;


    public:
        Server(asio::io_context& io, short port) : acceptor(io, tcp::endpoint(tcp::v4(), port)), io(io), port(port) {}

        void TryConnect() {
            auto player = std::make_shared<Client>(io);
            Players.push_back(player);

            acceptor.async_accept(player->socket, 
                [this, player](std::error_code er) {
                    if (!er) {
                        Client_Loop(player);
                    } else {
                        std::cout << "Server: Accept Failed \n";
                        Players.pop_back();
                    }
                    // Wait for New Client
                    TryConnect();
                }
            );
        }

        void Client_Loop(std::shared_ptr<Client> Player) {
            Player->socket.async_read_some(asio::buffer(Player->buf),
                [this, Player](std::error_code er, std::size_t len) {
                    if (!er) {
                        if (Player->Name != "") {
                            // Normal Loop
                            BroadCast(Player, Player->Name + ": " + std::string(Player->buf, len));
                            Client_Loop(Player);
                        } else {
                            // Set Name
                            Player->Name = std::string(Player->buf, len);
                            std::string Welcome = "Server: " + Player->Name + " Joined!";
                            std::cout << Welcome << "\n";
                            BroadCast(Player, Welcome);
                            Client_Loop(Player);
                        }
                    } else {
                        std::string Leave = "Server: " + Player->Name + " Left";
                        std::cout << Leave << "\n";
                        BroadCast(Player, Leave);
                        std::error_code ec;
                        Player->socket.close(ec);
                        RemoveClient(Player);
                    }
                }
            );
        }

        void Write(std::shared_ptr<Client> Player, std::string Message) {
            if (Message.size() < 1025) {
                auto msg = std::make_shared<std::string>(Message);
                Player->socket.async_write_some(asio::buffer(*msg),
                    [](std::error_code er, std::size_t) {
                    }
                );
            }
        }

        void BroadCast(std::shared_ptr<Client> sender, std::string Message) {
                for (auto& p : Players) {
                    if (p != sender || !sender) {
                        Write(p, Message);
                    }
                }
        }

        void RemoveClient(std::shared_ptr<Client> Player) {
            auto it = std::find(Players.begin(), Players.end(), Player);
            if (it != Players.end()) {
                Players.erase(it);
            }
        }
};

int main() {
    try {
        short port;
        std::cout << "Port: ";
        std::cin >> port;
        asio::io_context io;
        Server server(io, port);
        server.TryConnect();
        io.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}