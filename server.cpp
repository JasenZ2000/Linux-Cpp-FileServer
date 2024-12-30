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
#include "src/Buffer.h"

int main() {
    Server *server = new Server("127.0.0.1", 1234);

    server->setOnConnectCallback([](Connection* conn) {
        // conn->connRead();
        if (conn->getState() == ConnState::Closed) {
            conn->close();
            return;
        }
        std::cout << "Message from client " << conn->getId() << ": " << conn->getReadBuffer()->c_str() << std::endl;
        conn->connSend(conn->getReadBuffer()->c_str());
    });
    
    server->start(); 
    delete server;
    
    return 0;
}