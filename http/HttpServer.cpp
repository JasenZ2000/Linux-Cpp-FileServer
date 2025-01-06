#include "HttpServer.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "TcpServer.h"
#include "TcpConnection.h"
#include "CurrentThread.h"
#include "Buffer.h"

#include <memory>
#include <iostream>
#include <arpa/inet.h>

void HttpServer::DefaultHttpCallback(const HttpRequest &req, HttpResponse *resp){
    resp->SetStatusCode(HttpResponse::k404NotFound);
    resp->SetStatusMessage("Not Found");
    resp->SetCloseConnection(true);
}

HttpServer::HttpServer(EventLoop *loop, const char *ip, const int port) : main_reactor_(loop){
    tcp_server_ = std::make_unique<TcpServer>(main_reactor_, ip, port);
    tcp_server_->set_connection_callback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));    
    tcp_server_->set_message_callback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1));
    set_http_callback(std::bind(&HttpServer::DefaultHttpCallback, this, std::placeholders::_1, std::placeholders::_2));
};

HttpServer::~HttpServer(){};

void HttpServer::onConnection(const conn_ptr &conn){
    int clnt_fd = conn->fd();
    struct sockaddr_in peeraddr;
    socklen_t peer_addrlength = sizeof(peeraddr);
    getpeername(clnt_fd, (struct sockaddr *)&peeraddr, &peer_addrlength);

    std::cout << CurrentThread::tid()
              << " HttpServer::OnNewConnection : new connection "
              << "[fd#" << clnt_fd << "]"
              << " from " << inet_ntoa(peeraddr.sin_addr) << ":" << ntohs(peeraddr.sin_port)
              << std::endl;
};

void HttpServer::onMessage(const conn_ptr &conn){
    std::cout << CurrentThread::tid() << " HttpServer::onMessage" << std::endl;
    if (conn->state() == TcpConnection::ConnectionState::Connected)
    {
        // connection一次读取可能读不完一个请求，因此需要多次读取。
        HttpContext *context = conn->context();
        if (!context->ParseRequest(conn->read_buf()->c_str(), conn->read_buf()->Size()))
        {
            conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conn->HandleClose();
        }
        if (context->IsComplete())
        {
            std::cout << "HttpServer::onMessage : Completed" << std::endl;
            onRequest(conn, *context->GetRequest());
            context->Reset();
        }
    }
}

void HttpServer::onRequest(const conn_ptr &conn, const HttpRequest &req){
    std::cout << "HttpServer::onRequest" << std::endl;
    const std::string &connection = req.GetHeaderString("Connection");
    bool close = connection == "close" ||
                 (req.GetVersion() == HttpRequest::kHTTP_1_0 &&
                  connection != "Keep-Alive");
    HttpResponse response(close);
    on_request_(req, &response);

    std::cout << "HttpServer::onRequest : Send" << std::endl;
    conn->Send(response.message().c_str());
    if (response.close_connection())
    {
        std::cout << "HttpServer::onRequest : HandleClose" << std::endl;
        conn->HandleClose();
    }
}

void HttpServer::Start(){
    tcp_server_->Start();
}

void HttpServer::SetThreadNums(int thread_nums){
    tcp_server_->SetThreadNum(thread_nums);
}