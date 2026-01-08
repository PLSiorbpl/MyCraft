#ifndef SERVER_H
#define SERVER_H

#include <asio.hpp>
#include <thread>
#include <memory>
#include <optional>

#include "TsQueue.hpp"
#include "Net_Common.hpp"

class Net_Server {
public:
    explicit Net_Server() : work(asio::make_work_guard(context)) {}
    ~Net_Server();

    TsQueue<Packet, 255> queue_In;
    std::vector<std::shared_ptr<Client>> clients;

    std::atomic<bool> running = false;
    asio::io_context context;
    std::optional<asio::ip::tcp::acceptor> acceptor;
    std::thread netThread;
    asio::executor_work_guard<asio::io_context::executor_type> work;

    void Init_Server(uint16_t Port);
    void Stop_Server();
    void Run_Server();
    void VerifyClient(std::shared_ptr<Client> client);
    void GetHeader(std::shared_ptr<Client> client);
    void GetPayload(std::shared_ptr<Client> client);
    void SendPackets(std::shared_ptr<Client> client);
    void BroadcastPackets(const Packet& pkt, const std::shared_ptr<Client> &except);
    void Disconnect(std::shared_ptr<Client> client);
};



#endif //SERVER_H
