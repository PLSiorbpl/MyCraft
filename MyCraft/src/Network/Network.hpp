#pragma once

#include <asio.hpp>
#include <thread>
#include <atomic>

class Network {
    public:
    enum class PacketType : uint8_t {
        HELLO = 1,
        WELCOME = 2,
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

    template<typename T, size_t N>
    struct RingQueue {
        std::array<T, N> buf;
        std::atomic<size_t> head = 0;
        std::atomic<size_t> tail = 0;

        bool push(const T& v) {
            size_t h = head.load();
            size_t next = (h + 1) % N;
            if (next == tail.load()) return false;
            buf[h] = v;
            head.store(next);
            return true;
        }

        bool pop(T& out) {
            size_t t = tail.load();
            if (t == head.load()) return false;
            out = buf[t];
            tail.store((t + 1) % N);
            return true;
        }
    };

    // Queue for 256 packets
    RingQueue<Packet, 256> In;
    RingQueue<Packet, 256> Out;

    asio::io_context context;
    asio::ip::udp::socket socket;
    std::thread netThread;
    asio::ip::udp::endpoint server;

    void InitClient(const std::string& Ip, const uint16_t Port);
    void InitServer(uint16_t Port);

    void ReciveHeader();
    void RecivePayLoad(const PacketHeader& header);
};