#include "Network.hpp"

// I see you Imated

void Network::InitClient(const std::string& Ip, const uint16_t Port) {
    server = asio::ip::udp::endpoint(asio::ip::make_address(Ip), Port);
    socket.open(asio::ip::udp::v4());
    ReciveHeader();
    netThread = std::thread([&]{ context.run(); });
}

void Network::ReciveHeader() {
    PacketHeader header;
    socket.async_receive_from(asio::buffer(&header, sizeof(header)), server, [&](asio::error_code ec, size_t len) {
        if (!ec && len == sizeof(PacketHeader)) {
            RecivePayLoad(header);
        }
    });
}

void Network::RecivePayLoad(const PacketHeader& header) {
    auto payload = std::make_unique<uint8_t[]>(header.size);
    socket.async_receive_from(asio::buffer(payload.get(), header.size), server, [this, payload = std::move(payload), header](asio::error_code ec, size_t len) {
        if (!ec && len == header.size) {
            Packet P;
            P.h = header;
            std::memcpy(P.data.data(), payload.get(), header.size);
            In.push(P);
        }
        ReciveHeader();
    });
}

void Network::InitServer(uint16_t Port) {
    // soon fr
}