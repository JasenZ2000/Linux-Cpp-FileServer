/**
 * @file HttpServer.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 结合Http请求解析器与响应器，以及TCP服务器，实现了一个简单的HTTP服务器
 * @version 0.1
 * @date 2025-01-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#pragma once
#include <functional>
#include <memory>
#include "common.h"
#include "TimerStamp.h"

class HttpRequest;
class HttpResponse;
class TcpServer;
class TcpConnection;
class EventLoop;

class HttpServer
{
public:
    typedef std::shared_ptr<TcpConnection> conn_ptr;
    typedef std::function<void(const HttpRequest &, HttpResponse *)> http_callback;

    DISALLOW_COPY_AND_MOVE(HttpServer);
    HttpServer(EventLoop*, const char *ip, int port, double timeout = 0.0);
    ~HttpServer();

    void set_http_callback(const http_callback &fn) { on_request_ = std::move(fn);};
    void DefaultHttpCallback(const HttpRequest &req, HttpResponse *resp);

    void HandleActiveClose(std::weak_ptr<TcpConnection>&  conn);

    void Start();

    void onConnection(const conn_ptr &conn);
    void onMessage(const conn_ptr &conn);
    void onRequest(const conn_ptr &conn, const HttpRequest &req);

    void SetThreadNums(int thread_nums);

    void Refresh() const;

private:
    EventLoop *main_reactor_;
    std::unique_ptr<TcpServer> tcp_server_;

    std::function<void(const HttpRequest&, HttpResponse*)> on_request_;
    double timeout_;
    bool auto_close_conn_;
};