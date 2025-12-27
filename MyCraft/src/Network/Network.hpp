#pragma once

#include <asio.hpp>
#include <thread>
#include <array>
#include <atomic>
#include <mutex>

#include "TsQueue.hpp"

class Network {
    public:
    enum class PacketType : uint8_t {
        HELLO = 1,
        WELCOME = 2,
        CHAT = 3
    };

    struct PacketHeader {
        PacketType type;
        uint16_t size = 0;
    };

    struct Packet {
        PacketHeader h;
        std::vector<uint8_t> data;
        size_t size() const {
            return sizeof(PacketHeader) + data.size();
        }
    };

    struct Client {
        asio::ip::tcp::socket socket;
        std::string Name;
        Packet packet;

        Client(asio::io_context& io, std::string& name) : socket(io), Name(name) {}
    };

    // Queue for 256 packets
    TsQueue<Packet, 256> In;
    TsQueue<Packet, 256> Out;
    // List of Clients alive
    std::vector<std::shared_ptr<Client>> clients;

    // Client
    asio::io_context context;
    asio::ip::tcp::resolver resolver;
    std::thread netThread;
    // Server
    asio::ip::tcp::acceptor acceptor;
    asio::io_context context_serv;
    std::thread servThread;

    // Client
    void InitClient(const std::string& Ip, const uint16_t Port, std::string& Name);
    void Disconect(std::shared_ptr<Client> player);
    void Connect_To_Server(std::shared_ptr<Client> player);
    void ReciveHeader(std::shared_ptr<Client> player);
    void RecivePayLoad(std::shared_ptr<Client> player);
    // Server
    void Disconect_Player(std::shared_ptr<Client> player);
    void InitServer(uint16_t Port);
    void Accept_Players();
    void Verify_Player(std::shared_ptr<Client> player);
};