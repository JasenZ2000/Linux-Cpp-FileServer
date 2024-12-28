/**
 * @file server.cpp
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 把业务逻辑放在这了，底层越来越像一个库了
 * @version 0.1
 * @date 2024-12-28
 * @details 1. server create (main sub reactor)
 *          2. acceptor default create (main reactor)
 *          3. connection create with callback defined in server.cpp 
 * @copyright Copyright (c) 2024
 * 
 */
#include <iostream>
#include "src/Server.h"
#include "src/EventLoop.h"
#include "src/Connection.h"
// #include "src/Buffer.h"
#include "src/Socket.h"

int main() {
    EventLoop* loop = new EventLoop();
    Server* server = new Server(loop);
    server->setOnConnectCallback([](Connection* conn) {
        conn->connRead();
        if (conn->getState() == ConnState::Closed) {
            conn->close();
            return;
        }
        std::cout << "Message from client " << conn->getSocket()->getFd() << ": " << conn->ReadBuffer() << std::endl;
        conn->setSendBuffer(conn->ReadBuffer());
        conn->connWrite();
    });
    loop->loop();
    delete server;
    delete loop;
    return 0;
}