#include "Client.hpp"
#include <iostream>
#include <memory>

Net_Client::~Net_Client() {
    Disconnect_Client();
}

void Net_Client::Stop_Client() {
    work.reset();
    context.stop();
    if (netThread.joinable())
        netThread.join();
}

void Net_Client::Request_Stop() {
    if (!client) return;
    asio::post(context, [this]() {
        if (client->socket.is_open()) {
            asio::error_code ec;
            client->socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            client->socket.close(ec);
            if (ec)
                std::cout << "Client: Error closing socket: " << ec.message() << "\n";
        }
    });
}


bool Net_Client::Init_Client(const std::string &Ip, const uint16_t Port, const std::string &Name) {
    if (!client)
        netThread = std::thread([this]() { context.run(); });
    Request_Stop();
    if (client)
        client.reset();
    asio::error_code ec;
    const auto endpoint = resolver.resolve(Ip, std::to_string(Port), ec);
    if (ec) {
        std::cout << "Client: Cant Connect " << ec.message() << "\n";
        return false;
    }

    client = std::make_shared<Client>(context);
    sending = false;

    asio::connect(client->socket, endpoint, ec);
    if (ec) {
        std::cout << "Error connecting to server: " << ec.message() << "\n";
        return false;
    }

    std::cout << "Client: Connected to " << Ip << ":" << Port << "\n";
    gen.store(gen.load() + 1);
    Connect_Client(Name);

    return true;
}

void Net_Client::Connect_Client(const std::string &Name) {
    int mgen = gen.load();
    // In future Async
    // Header
    PacketHeader header = {PacketType::HELLO, static_cast<uint16_t>(Name.size())};
    asio::write(client->socket, asio::buffer(&header, sizeof(header)));
    if (mgen != gen.load()) return;
    // Payload
    asio::write(client->socket, asio::buffer(Name.data(), Name.size()));
    // Read Header
    Packet packet;
    if (mgen != gen.load()) return;
    asio::read(client->socket, asio::buffer(&packet.h, sizeof(PacketHeader)));
    // Read Payload
    packet.data.resize(packet.h.size);
    if (mgen != gen.load()) return;
    asio::read(client->socket, asio::buffer(packet.data.data(), packet.h.size));
    if (Name.size() == packet.data.size()) {
        if (mgen != gen.load()) return;
        GetHeader();
        std::cout << "Client: Successfully connected\n";
    }
}

void Net_Client::GetHeader() {
    int mgen = gen.load();
    auto packet = std::make_shared<Packet>();
    std::cout << "Client: Waiting for packet\n";
    asio::async_read(client->socket, asio::buffer(&packet->h, sizeof(PacketHeader)), [this, packet, mgen](const asio::error_code ec, const size_t len) {
        if (mgen != gen.load()) return;
        if (!ec && len == sizeof(PacketHeader)) {
            if (packet->h.size > 1024) {
                std::cout << "Client: PayLoad too Big\n";
                Disconnect_Client();
                return;
            } // PayLoad limit
            std::cout << "Client: Got Header\n";
            GetPayload(packet);
        } else {
            if (ec == asio::error::operation_aborted) {
                std::cout << "Client: Read aborted (likely due to disconnect)\n";
                return;
            }
            std::cout << "Client: Header Error: " << ec.message() << "\n";
            Disconnect_Client();
        }
    });
}

void Net_Client::GetPayload(std::shared_ptr<Packet> packet) {
    int mgen = gen.load();
    packet->data.resize(packet->h.size);
    asio::async_read(client->socket, asio::buffer(packet->data, packet->h.size), [this, packet, mgen](const asio::error_code ec, const size_t len) {
        if (mgen != gen.load()) return;
        if (!ec && len == packet->h.size) {
            std::cout << "Client: Got Payload\n";
            queue_In.push(*packet);
            GetHeader();
        } else {
            std::cout << "Client: PayLoad Error\n";
            Disconnect_Client();
        }
    });
}

void Net_Client::Send() {
    asio::post(context, [this]() {
        if (!sending)
            SendPackets();
    });
}

void Net_Client::SendPackets() {
    int mgen = gen.load();
    if (sending) return;
    sending = true;
    auto pkt = std::make_shared<Packet>();
    if (!queue_Out.pop(*pkt)) {
        sending = false;
        return;
    }
    asio::async_write(client->socket, asio::buffer(&pkt->h, sizeof(PacketHeader)), [this, pkt, mgen](const asio::error_code& ec, const size_t len) {
        if (mgen != gen.load()) return;
        if (!ec) {
            asio::async_write(client->socket, asio::buffer(pkt->data.data(), pkt->h.size), [this, pkt, mgen](const asio::error_code& ec, const size_t len) {
                if (mgen != gen.load()) return;
                if (!ec) {
                   std::cout << "Client: Sending successful: " << std::string(pkt->data.begin(), pkt->data.end()) << "\n";
                   sending = false;
                   SendPackets();
               } else {
                   std::cout << "Client: Sending PayLoad error\n";
                   Disconnect_Client();
               }
            });
        } else {
            std::cout << "Client: Sending Header error\n";
            Disconnect_Client();
        }
    });
}

void Net_Client::Disconnect_Client() {
    std::cout << "Client: Stoping..\n";
    Request_Stop();
}