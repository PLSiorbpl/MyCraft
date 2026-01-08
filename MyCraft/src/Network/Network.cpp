#include "Network.hpp"

void Net::Start_Client(const std::string &Ip, uint16_t Port, const std::string &Name) {
    client.Init_Client(Ip, Port, Name);
}

void Net::Start_Server(const uint16_t Port) {
    server.Init_Server(Port);
}

bool Net::Client_Read(Packet& packet) {
    return client.queue_In.pop(packet);
}

bool Net::Client_Send(const Packet& packet) {
    return client.queue_Out.push(packet);
}

bool Net::Server_Read(Packet& packet) {
    return server.queue_In.pop(packet);
}

bool Net::Server_Send(const Packet& packet, std::shared_ptr<Client> &client) {
    return client->queue_Out.push(packet);
}

