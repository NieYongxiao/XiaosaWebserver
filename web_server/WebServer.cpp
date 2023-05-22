#include"WebServer.h"
#include<iostream>
#include<errno.h>  //引入全局变量errno
#include<fcntl.h>  //引入fcntl函数
#include<unistd.h>
#include<signal.h>

//本来想实现成员函数的func，但是thread pool的submit函数提交时出现了奇怪的bug，见question文件夹

using namespace std;
#define BUF_SIZE 1024
void func(int clnt_sock,int epfd);
void complete_request(char *buf,int clnt_sock);
void send_null_file(char* buf,int clnt_sock);
void send_html_file(char* buf,int clnt_sock,FILE* file_reader);
void send_txt_file(char* buf,int clnt_sock,FILE* file_reader);
void send_jpg_file(char* buf,int clnt_sock,FILE* file_reader);
void send_gif_file(char* buf,int clnt_sock,FILE* file_reader);

WebServer::WebServer():port(8090) {
    serv_sock = socket(AF_INET, SOCK_STREAM,0);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sock_addr_size) == -1){
        error_handler("bind error!");
    }
    if(listen(serv_sock,10)<0){
        error_handler("listen error!");
    }
}

WebServer::WebServer(int set_port):port(set_port) {
    serv_sock = socket(AF_INET, SOCK_STREAM,0);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sock_addr_size) == -1){
        error_handler("bind error!");
    }
    if(listen(serv_sock,10)<0){
        error_handler("listen error!");
    }
}

WebServer::~WebServer(){
    close(serv_sock);
}

void WebServer::epoll_add(epoll_event& event,int epfd){
    
}

//边缘触发方式必须用非阻塞read write ，否则可能引起服务器端的长时间停顿
void WebServer::start(int epoll_size){   //主线程工作场景
    int clnt_sock;
    struct sockaddr_in clnt_addr;

    struct epoll_event event; //工具人用于注册文件描述符
    struct epoll_event *ep_events;
    int epfd,event_cnt;
    epfd=epoll_create(epoll_size); //创建保存epoll文件描述符的空间，成功时返回文件描述符，失败时返回-1
    //手动分配空间，返回分配空间的首地址 此处空间为了保存发生情况的events  
    ep_events= (struct epoll_event*)(malloc(sizeof(struct epoll_event)*epoll_size)); 

    set_non_block_mode(serv_sock);

    //以下三行为了注册serv_sock
    event.data.fd=serv_sock;
    event.events=EPOLLIN; // 设定监测的信息为发生需要读取数据的情况
    epoll_ctl(epfd,EPOLL_CTL_ADD,serv_sock,&event); //向内部注册并注销文件描述符
    int epoll_wait_num=0;
    while(true){
        epoll_wait_num++;
        //等待文件描述符发生变化，成功时返回发生事件的文件描述符数，失败时返回-1
        event_cnt = epoll_wait(epfd,ep_events,epoll_size,-1); 
        //cout<<"epoll_wait "<< epoll_wait_num <<" times "<<endl;
        if(event_cnt < 0)
        {
            error_handler("epoll_wait error!");
        }
        for(int i=0;i<event_cnt;i++)
        {
            if(ep_events[i].data.fd==serv_sock) //当client发起连接请求时
            {
                clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &sock_addr_size);
                set_non_block_mode(clnt_sock);
                //向epoll_event 注册 clnt_sock
                event.data.fd=clnt_sock;
                event.events=EPOLLIN|EPOLLET; // 将触发方式设置为边缘触发
                epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_sock,&event);
                //printf("clnt_sock %d  connected \n",clnt_sock);
            }
            else  //当有请求发送到clnt_sock时
            {
                int temp_sock = ep_events[i].data.fd;
                this->thread_pool.submit(func,temp_sock,epfd);
            }
        }
    }
}


void WebServer::set_non_block_mode(int fd)
{
    int flag = fcntl(fd,F_GETFL,0);  //fcntl根据文件描述词来操作文件的特性
    fcntl(fd,F_SETFL,flag | O_NONBLOCK);  //打开文件之后设置为非阻塞 
}


void WebServer::error_handler(string&& message){
    cout<<message<<endl;
    exit(1);
}


void func(int clnt_sock,int epfd){
    //cout<<"start to read "<<endl;
    char buf[BUF_SIZE];
    memset(buf,0,sizeof(buf));
    while(1)  //为保证完全读取完全 需循环read
    {
        int str_len = read(clnt_sock,buf,BUF_SIZE);
        if(str_len == 0)
        {
            epoll_ctl(epfd,EPOLL_CTL_DEL,clnt_sock,NULL);
            close(clnt_sock);
            //cout<<"clne sock "<<clnt_sock<<" disconnected!"<<endl;
            break;
        }
        else if(str_len <0)
        {
            if(errno==EAGAIN){
                break;   //当两个条件都满足时，说明读取了输入缓冲的全部数据
            }
        }
        else
        {
            read(clnt_sock, buf, BUF_SIZE); // 从客户端读取 HTTP 请求
            //puts(buf);
        }
    }
    if(strlen(buf) ==0) return;
    complete_request(buf,clnt_sock);
}

void complete_request(char *buf,int clnt_sock){
    char method[10], pathname[100], protocol[10];
    sscanf(buf, "%s %s %s", method, pathname, protocol); 
    // printf("method is %s\n", method);
    // printf("pathname is %s\n", pathname);
    // printf("protocol is %s\n", protocol);

    char path[BUF_SIZE];
    //处理 GET 方法请求
    if(strcmp(method, "GET") == 0)
    {
        if(strcmp(pathname, "/")==0){ //此时输入路径只有“/”
            strcpy(path,"/home/xiaosa/desktop/xiaosa_webserver/web_source/default.html");
        }
        else {
            strcpy(path,pathname);
        }
        path[strlen(path)] = '\0';

        //printf("Open file path is %s\n", path);
        FILE* file_reader = fopen(path, "rb"); 
        if(file_reader == NULL){
            send_null_file(buf,clnt_sock);
        } 
        else{
            char *file_type = strrchr(path, '.'); //此时file_type是.http .txt .jpg .mp3
            //strrchr用于在一个字符串中查找最后一个匹配给定字符（或者字符的ASCII码值）的位置，并返回该位置的指针。
            if(strcmp(file_type, ".html") == 0) send_html_file(buf, clnt_sock, file_reader);
            else if(strcmp(file_type, ".txt") == 0) send_txt_file(buf, clnt_sock, file_reader);
            else if(strcmp(file_type, ".jpg") == 0)  send_jpg_file(buf, clnt_sock, file_reader);
            else if(strcmp(file_type, ".gif") == 0)  send_gif_file(buf, clnt_sock, file_reader);
        }
        cout<<"message send successful"<<endl;
    }
    else cout<<"method is not GET"<<endl;
    close(clnt_sock);
}


void send_null_file(char* buf,int clnt_sock){
    sprintf(buf, "HTTP/1.1 404 Not Found\r\n");  //将数据打印到字符串中。
    sprintf(buf, "%sContent-Type:text/html\r\n\r\n",buf);
    send(clnt_sock, buf, strlen(buf),MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文头部
    sprintf(buf, "<html><body><h1>404 Not Found  文件打开失败</h1></body></html>\r\n");       
}

void send_html_file(char* buf,int clnt_sock,FILE* file_reader) {
    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    send(clnt_sock, buf, strlen(buf),MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文头部
    // 发送长度 连接关闭等有关信息
    // sprintf(buf, "Content-Length : %d\r\n",file_size(path)); 
    // printf("file size : %d \n",file_size(path));
    // send(clnt_sock, buf, strlen(buf), MSG_WAITALL | MSG_NOSIGNAL);
    // sprintf(buf, "Connection: close\r\n");
    // send(clnt_sock, buf, strlen(buf),MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文头部
    sprintf(buf, "Content-Type: text/html; charset=utf-8\r\n\r\n");
    send(clnt_sock, buf, strlen(buf),MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文头部
    while(fgets(buf, BUF_SIZE, file_reader) != NULL)
    {
        send(clnt_sock, buf, strlen(buf),MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文体
    }
}
void send_txt_file(char* buf,int clnt_sock,FILE* file_reader){
    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    send(clnt_sock, buf, strlen(buf),MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文头部
    sprintf(buf, "Content-Type: text/plain; charset=utf-8\r\n\r\n");
    send(clnt_sock, buf, strlen(buf),MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文头部
    while(fgets(buf, BUF_SIZE, file_reader) != NULL)
    {
        send(clnt_sock, buf, strlen(buf),MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文体
    }
}
//两个大坑，首先要使用fread不能用fgets，fread读取二进制文件，fget读取文本文件 其次不能用strlen函数算出buf的长度，因为后面短的没法完全覆盖
void send_jpg_file(char* buf,int clnt_sock,FILE* file_reader) {
    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    send(clnt_sock, buf, strlen(buf),MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文头部
    sprintf(buf, "Content-Type: image/jpeg\r\n\r\n");
    send(clnt_sock, buf, strlen(buf),MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文头部
    int bytes_read;
    while((bytes_read = fread(buf, sizeof(unsigned char),BUF_SIZE, file_reader)) >0)
    {
        send(clnt_sock, buf, bytes_read,MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文体
    }
}

void send_gif_file(char* buf,int clnt_sock,FILE* file_reader) {
    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    send(clnt_sock, buf, strlen(buf),MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文头部
    sprintf(buf, "Content-Type: image/gif\r\n\r\n");
    send(clnt_sock, buf, strlen(buf),MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文头部
    int bytes_read;
    while((bytes_read = fread(buf, sizeof(unsigned char),BUF_SIZE, file_reader)) >0)
    {
        send(clnt_sock, buf, bytes_read,MSG_WAITALL | MSG_NOSIGNAL); // 发送 HTTP 响应报文体
    }
}
