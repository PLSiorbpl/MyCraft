#ifndef NET_COMMON_H
#define NET_COMMON_H

#include <stdint.h>
#include <vector>
#include "TsQueue.hpp"

enum class PacketType : uint8_t {
    HELLO = 1,
    WELCOME = 2,
    CHAT = 3
};

struct PacketHeader {
    PacketType type = PacketType::HELLO;
    uint16_t size = 0;
};

struct Packet {
    PacketHeader h;
    std::vector<uint8_t> data;
    [[nodiscard]] size_t size() const {
        return sizeof(PacketHeader) + data.size();
    }
};

struct Client {
    asio::ip::tcp::socket socket;
    std::string Name;
    Packet packet;
    TsQueue<Packet, 31> queue_Out;
    bool writing = false;

    explicit Client(asio::io_context& io) : socket(io) {}
};

#endif //NET_COMMON_H
