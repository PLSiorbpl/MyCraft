#include "Server.hpp"
#include <iostream>

Net_Server::~Net_Server() {
    Stop_Server();
}

void Net_Server::Stop_Server() {
    asio::post(context, [this] {
        asio::error_code ec;
        acceptor->close(ec);

        for (auto& c : clients) {
            c->socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            c->socket.close(ec);
        }
        clients.clear();
    });

    work.reset();
    context.stop();

    if (netThread.joinable())
        netThread.join();
}

void Net_Server::Init_Server(const uint16_t Port) {
    if (running) {
        std::cout << "Server already running\n";
        return;
    }
    acceptor.emplace(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), Port));
    Run_Server();
    netThread = std::thread([this] { context.run(); });
}

void Net_Server::Run_Server() {
    auto client = std::make_shared<Client>(context);
    std::cout << "Looking for client\n";
    acceptor->async_accept(client->socket, [this, client](const asio::error_code &ec) {
        if (!ec) {
            VerifyClient(client);
        } else {
            std::cout << "Client Failed to Connected: " << ec.message() << "\n";
        }
        Run_Server();
    });
}

void Net_Server::VerifyClient(std::shared_ptr<Client> client) {
    asio::async_read(client->socket, asio::buffer(&client->packet.h, sizeof(PacketHeader)), [this, client](asio::error_code ec, size_t len) {
        if (!ec && len == sizeof(PacketHeader) && client->packet.h.type == PacketType::HELLO) {
            if (client->packet.h.size > 1024) {
                std::cout << "PayLoad too Big\n";
                Disconnect(client);
                return;
            }
            client->packet.data.resize(client->packet.h.size);
            asio::async_read(client->socket, asio::buffer(client->packet.data, client->packet.h.size), [this, client](asio::error_code ec, size_t len) {
                if (!ec && len == client->packet.h.size) {
                    client->Name = std::string(client->packet.data.begin(), client->packet.data.end());
                    client->packet.h.type = PacketType::WELCOME;
                    asio::async_write(client->socket, asio::buffer(&client->packet.h, sizeof(PacketHeader)), [this, client](asio::error_code ec, size_t len) {
                        if (!ec) {
                            asio::async_write(client->socket, asio::buffer(client->packet.data, client->packet.h.size), [this, client](asio::error_code ec, size_t len) {
                                if (!ec) {
                                    clients.push_back(client);
                                    std::cout << "Server: (" << client->Name << ") connected\n";
                                    GetHeader(client);
                                } else {
                                    std::cout << "Error Sending Payload\n";
                                    Disconnect(client);
                                }
                            });
                        } else {
                            std::cout << "Error Sending WELCOME: " << ec.message() << "\n";
                            Disconnect(client);
                        }
                    });
                } else {
                    std::cout << "Payload error: " << ec.message() << "\n";
                    Disconnect(client);
                }
            });
        } else {
            std::cout << "Header error: " << ec.message() << "\n";
            Disconnect(client);
        }
    });
}

void Net_Server::GetHeader(std::shared_ptr<Client> client) {
    asio::async_read(client->socket, asio::buffer(&client->packet.h, sizeof(PacketHeader)), [this, client](asio::error_code ec, size_t len) {
       if (!ec && len == sizeof(PacketHeader)) {
            if (client->packet.h.size > 1024) {
                std::cout << "Payload too big\n";
                Disconnect(client);
                return;
            }
           GetPayload(client);
       } else {
           std::cout << "Header error: " << ec.message() << "\n";
           Disconnect(client);
       }
    });
}

void Net_Server::GetPayload(std::shared_ptr<Client> client) {
    client->packet.data.resize(client->packet.h.size);
    asio::async_read(client->socket, asio::buffer(client->packet.data, client->packet.h.size), [this, client](asio::error_code ec, size_t len) {
        if (!ec && len == client->packet.h.size) {
            std::cout << "Server: Got PayLoad from: (" << client->Name << ")\n";
            queue_In.push(client->packet);
            GetHeader(client);
        } else {
            std::cout << "Payload error: " << ec.message() << "\n";
            Disconnect(client);
        }
    });
}

void Net_Server::SendPackets(std::shared_ptr<Client> client) {
    if (client->writing) return;

    client->writing = true;

    auto pkt = std::make_shared<Packet>();
    if (!client->queue_Out.pop(*pkt)) {
        client->writing = false;
        return;
    }
    asio::async_write(client->socket, asio::buffer(&pkt->h, sizeof(PacketHeader)),
                      [this, client, pkt](asio::error_code ec, size_t) {
            if (!ec) {
                asio::async_write(client->socket, asio::buffer(pkt->data, pkt->h.size), [this, client](asio::error_code ec, size_t len) {
                    if (!ec) {
                        std::cout << "Server: Packet Sent to (" << client->Name << ")\n";
                        client->writing = false;
                        SendPackets(client);
                    } else {
                        Disconnect(client);
                    }
                });
            } else {
                Disconnect(client);
            }
        }
    );
}

void Net_Server::BroadcastPackets(const Packet &pkt, const std::shared_ptr<Client> &except) {
    std::cout << "Server: Broadcasting\n";
    for (auto& client : clients) {
        if (client == except) continue;
        client->queue_Out.push(pkt);
        SendPackets(client);
    }
}

void Net_Server::Disconnect(std::shared_ptr<Client> client) {
    asio::post(context, [this, client] {
        if (!client->socket.is_open())
            return;

        asio::error_code ec;
        client->socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        client->socket.close();
        client->writing = false;
        clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
        std::cout << "Server: (" << client->Name << ") disconnected\n";
        if (ec) {
            std::cout << "Error disconnecting from server: " << ec.message() << "\n";
        }
    });
}