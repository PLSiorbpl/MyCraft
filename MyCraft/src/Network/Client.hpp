#ifndef CLIENT_H
#define CLIENT_H

#pragma once

#include <asio.hpp>
#include <thread>
#include <optional>

#include "TsQueue.hpp"
#include "Net_Common.hpp"

class Net_Client {
public:
    Net_Client(): work(asio::make_work_guard(context)), resolver(context) {
    }

    ~Net_Client();

    TsQueue<Packet, 255> queue_In;
    TsQueue<Packet, 255> queue_Out;

    asio::io_context context;
    asio::executor_work_guard<asio::io_context::executor_type> work;
    asio::ip::tcp::resolver resolver;
    std::thread netThread;
    std::atomic<bool> sending = false;
    std::atomic<int> gen = 0;

    std::shared_ptr<Client> client;
    //Packet packet;

    bool Init_Client(const std::string &Ip, uint16_t Port, const std::string &Name);
    void Stop_Client();
    void Request_Stop();
    void Connect_Client(const std::string &Name);
    void GetHeader();
    void GetPayload(std::shared_ptr<Packet> packet);
    void Send();
    void SendPackets();
    void Disconnect_Client();
};

#endif //CLIENT_H
