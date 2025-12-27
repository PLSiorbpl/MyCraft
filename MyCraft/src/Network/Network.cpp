#include "Network.hpp"

// I see you Imated
// dont skul me
// rust > c++ fr

void Network::InitClient(const std::string& Ip, const uint16_t Port, std::string& Name) {
    auto player = std::make_shared<Client>(context, Name);
    resolver = asio::ip::tcp::resolver(context);
    auto Server = resolver.resolve(Ip, std::to_string(Port));
    connect(player->socket, Server);
    Connect_To_Server(player);
    netThread = std::thread([this]{ context.run(); });
}

void Network::Connect_To_Server(std::shared_ptr<Client> player) {
    static int tried = 0;
    // Header
    PacketHeader header = {PacketType::HELLO, static_cast<uint16_t>(player->Name.size())};
    asio::write(player->socket, asio::buffer(&header, sizeof(header)));
    // Payload
    asio::write(player->socket, asio::buffer(&player->Name, player->Name.size()));
    // Read Header
    Packet packet;
    asio::read(player->socket, asio::buffer(&packet.h, sizeof(PacketHeader)));
    // Read Payload
    asio::read(player->socket, asio::buffer(&packet.data, packet.h.size));
    if (player->Name.size() == packet.data.size()) {
        ReciveHeader(player);
    } else {
        tried += 1;
        if (tried >= 3) {
            return;
        }
        Connect_To_Server(player);
    }
}   

void Network::ReciveHeader(std::shared_ptr<Client> player) {
    asio::async_read(player->socket, asio::buffer(&player->packet.h, sizeof(PacketHeader)), [this, player](asio::error_code ec, size_t len) {
        if (!ec && len == sizeof(PacketHeader)) {
            if (player->packet.h.size > 1024) Disconect(player);
            RecivePayLoad(player);
        } else {
            Disconect(player);
        }
    });
}

void Network::RecivePayLoad(std::shared_ptr<Client> player) {
    player->packet.data.resize(player->packet.h.size);
    asio::async_read(player->socket, asio::buffer(player->packet.data), [this, player](asio::error_code ec, size_t len) {
        if (!ec && len == player->packet.h.size) {
            In.push(player->packet);
            ReciveHeader(player);
        } else {
            Disconect(player);
        }
    });
}

void Network::Disconect(std::shared_ptr<Client> player) {
    asio::post(context, [player] {
        asio::error_code ec;
        player->socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        player->socket.close(ec);
    });
}

void Network::Disconect_Player(std::shared_ptr<Client> player) {
    asio::post(context, [this, player]() {
        if (player->socket.is_open()) {
            asio::error_code ec;
            player->socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            player->socket.close(ec);
            auto it = std::find(clients.begin(), clients.end(), player);
            if (it != clients.end()) {
                clients.erase(it);
            }
        }
    });
}

void Network::Accept_Players() {
    auto client = std::make_shared<Client>(context_serv, "");
    acceptor.async_accept(client->socket, [this, client](asio::error_code ec) {
        if (!ec) {
            Verify_Player(client);
        } else {

        }
        Accept_Players();
    });
}

void Network::Verify_Player(std::shared_ptr<Client> player) {
    asio::async_read(player->socket, asio::buffer(&player->packet.h, sizeof(PacketHeader)), [this, player](asio::error_code ec, size_t len) {
        if (!ec && len == sizeof(PacketHeader)) {
            if (player->packet.h.size > 1024) Disconect(player);
            // get players name and stuff
            // i will do it soon
            clients.push_back(player);
            ReciveHeader(player);
        } else {
            Disconect(player);
        }
    });
}

void Network::InitServer(uint16_t Port) {
    asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), Port);
    acceptor = asio::ip::tcp::acceptor(context_serv, ep);
    Accept_Players();
    servThread = std::thread( [this] { context_serv.run(); });
}