#include <iostream>
#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "EventLoop.h"
#include <string>

const std::string html = " <font color=\"red\">This is html!</font> ";
void HttpResponseCallback(const HttpRequest &request, HttpResponse *response)
{
    if(request.GetMethod() != HttpRequest::Method::kGET){
        response->SetStatusCode(HttpResponse::StatusCode::k400BadRequest);
        response->SetStatusMessage("Bad Request");
        response->SetCloseConnection(true);
    }
    else
    {
        std::string url = request.GetUrl();
        if(url == "/"){
            response->SetStatusCode(HttpResponse::StatusCode::k200OK);
            response->SetBody(html);
            response->SetContentType("text/html");
        }else if(url == "/hello"){
            response->SetStatusCode(HttpResponse::StatusCode::k200OK);
            response->SetBody("hello world\n");
            response->SetContentType("text/plain");
        }else if(url == "/favicon.ico"){
            response->SetStatusCode(HttpResponse::StatusCode::k200OK);
        }else{
            response->SetStatusCode(HttpResponse::StatusCode::k404NotFound);
            response->SetStatusMessage("Not Found");
            response->SetBody("Sorry Not Found\n");
            response->SetCloseConnection(true);
        }
    }
    return;
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
    int size = 10; // std::thread::hardware_concurrency();
    EventLoop *loop = new EventLoop();
    HttpServer *server = new HttpServer(loop, "127.0.0.1", port, 10.0);
    server->set_http_callback(HttpResponseCallback);
    server->SetThreadNums(size);
    server->Start();
    
    //delete loop;
    //delete server;
    return 0;
}