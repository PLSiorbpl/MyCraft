#include <iostream>
#include <asio.hpp>
#include <deque>
#include <thread>
#include <glm/glm.hpp>
#include <chrono>


using asio::ip::tcp;

void print_slow(const std::string& text, int ms_delay) {
    for (char c : text) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms_delay));
    }
    std::cout << "\n";
}

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
                        print_slow("Server: Accept Failed", 25);
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
                            print_slow(Welcome, 25);
                            BroadCast(Player, Welcome);
                            Client_Loop(Player);
                        }
                    } else {
                        std::string Leave = "Server: " + Player->Name + " Left";
                        print_slow(Leave, 25);
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
        print_slow("|========================================================================================================================================|", 1);
        std::cout << " /$$      /$$            /$$$$$$                      /$$$$$$   /$$           /$$$$$$\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "| $$$    /$$$           /$$__  $$                    /$$__  $$ | $$          /$$__  $$\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        std::cout << "| $$$$  /$$$$ /$$   /$$| $$  \\__/  /$$$$$$  /$$$$$$ | $$  \\__//$$$$$$       | $$  \\__/  /$$$$$$   /$$$$$$  /$$    /$$ /$$$$$$   /$$$$$$\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "| $$ $$/$$ $$| $$  | $$| $$       /$$__  $$|____  $$| $$$$   |_  $$_//$$$$$$|  $$$$$$  /$$__  $$ /$$__  $$|  $$  /$$//$$__  $$ /$$__  $$\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        std::cout << "| $$  $$$| $$| $$  | $$| $$      | $$  \\__/ /$$$$$$$| $$_/     | $$ |______/ \\____  $$| $$$$$$$$| $$  \\__/ \\  $$/$$/| $$$$$$$$| $$  \\__/\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "| $$\\  $ | $$| $$  | $$| $$    $$| $$      /$$__  $$| $$       | $$ /$$      /$$  \\ $$| $$_____/| $$        \\  $$$/ | $$_____/| $$\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        std::cout << "| $$ \\/  | $$|  $$$$$$$|  $$$$$$/| $$     |  $$$$$$$| $$       |  $$$$/     |  $$$$$$/|  $$$$$$$| $$         \\  $/  |  $$$$$$$| $$\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "|__/     |__/ \\____  $$ \\______/ |__/      \\_______/|__/        \\___/        \\______/  \\_______/|__/          \\_/    \\_______/|__/\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        std::cout << "              /$$  | $$\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        std::cout << "             |  $$$$$$/\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        std::cout << "              \\______/\n";
        print_slow("|========================================================================================================================================|\n", 1);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        print_slow("Enter Port: ", 50);
        std::cin >> port;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        print_slow("\nReady to accept players!\n", 50);
        print_slow("|=====================================================================|\n", 1);
        asio::io_context io;
        Server server(io, port);
        server.TryConnect();
        io.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}