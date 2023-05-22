#include<iostream>
#include</home/xiaosa/desktop/xiaosa_webserver/web_server/WebServer.cpp>


// xiaosa@xiaosa-virtual-machine:~$ webbench -c 10000 -t 10 http://127.0.0.1:8090/
// Webbench - Simple Web Benchmark 1.5
// Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

// Benchmarking: GET http://127.0.0.1:8090/
// 10000 clients, running 10 sec.

// Speed=1337940 pages/min, 22614698 bytes/sec.
// Requests: 222675 susceed, 315 failed.


int main(){
    WebServer web_server;
    web_server.start(10);
    return 0;
}