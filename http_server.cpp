#include <iostream>
#include "HttpServer.h"
// #include "HttpRequest.h"
// #include "HttpResponse.h"
#include "EventLoop.h"
#include "Logger.h"
#include "AsyncLogger.h"
#include <thread>
#include <string>

#include "FileSystem.h"

std::unique_ptr<AsyncLogger> asynclog;

extern void HttpResponseCallback(const HttpRequest &request, HttpResponse *response);

void AsyncOutputFunc(const char *data, int len)
{
    asynclog->Append(data, len);
}

void AsyncFlushFunc() {
    asynclog->Flush();
}

int main(int argc, char *argv[]){
    int port;
    if (argc <= 1)
    {
        port = 1234;
    }else if (argc == 2){
        port = atoi(argv[1]);
    }else{
        printf("error");
        exit(0);
    }

    // // 好糟糕啊，这东西独立于服务器建立，但其工作流在服务器内访问
    // asynclog = std::make_unique<AsyncLogger>();
    // Logger::setOutput(AsyncOutputFunc);
    // Logger::setFlush(AsyncFlushFunc);

    // asynclog->Start();

    int size = 10; // std::thread::hardware_concurrency();
    EventLoop *loop = new EventLoop();
    HttpServer *server = new HttpServer(loop, "127.0.0.1", port, 60.0);
    server->set_http_callback(HttpResponseCallback);
    server->SetThreadNums(size);
    server->Start();
    
    //delete loop;
    //delete server;
    return 0;
}