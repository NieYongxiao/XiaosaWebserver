#pragma once
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include</home/xiaosa/desktop/xiaosa_webserver/thread_pool/thread_pool.h>
#include</home/xiaosa/desktop/xiaosa_webserver/thread_pool/thread_pool.cpp>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<string>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<sys/socket.h>

using namespace std;

class WebServer{
public:
    WebServer();
    WebServer(int port);
    ~WebServer();
    void error_handler(string&&);
    void start(int epoll_size);
private:
    void set_non_block_mode(int fd);
    void epoll_add(epoll_event& event,int epfd);
private:
    int serv_sock;
    int port;
    struct sockaddr_in serv_addr;
    socklen_t sock_addr_size = sizeof(struct sockaddr_in);
    ThreadPool thread_pool;
};


#endif