#pragma once

#include "Client.hpp"
#include "Server.hpp"
#include "Net_Common.hpp"

class Net {
public:
    enum class As : uint8_t {
        Client = 0,
        Server = 1
    };

    Net_Client client;
    Net_Server server;

    void Start_Server(uint16_t Port);
    void Start_Client(const std::string &Ip, uint16_t Port, const std::string &Name);

    bool Client_Read(Packet&);
    bool Client_Send(const Packet&);

    bool Server_Read(Packet&);
    bool Server_Send(const Packet&, std::shared_ptr<Client> &client);

};