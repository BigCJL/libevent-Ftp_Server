#include <iostream>
#include "XThreadPool.h"
#include "XFtpServerCMD.h"
#include <event2/listener.h>
#include <event2/event.h>
#ifndef _WIN32
#include <signal.h>
#include <string.h>
#endif
#include "XFtpFactory.h"

using namespace std;

#define SPORT 21

void listen_cb(struct evconnlistener* e, evutil_socket_t s, struct sockaddr* a, int socklen, void* arg)
{
    cout << "listen_cb"<<endl;
    XTask* task = XFtpFactory::Get()->CreateTask();
    task->sock = s;
    XThreadPool::Get()->Dispatch(task);
}

int main()
{
#ifdef _WIN32
    //初始化socket库,这是windows特有的操作
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#else
    //忽略管道信号 发送数据给已关闭的socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif
    //1. 初始化线程池
    XThreadPool::Get()->Init(10);








    std::cout << "test_thread_pool_server\n";
    //创建libevent的上下文
    event_base* base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }
    //监听端口
    //socket,bind,listen 三步全部包含在内了
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(SPORT);
    
    evconnlistener* ev = evconnlistener_new_bind(base,   //libevent的上下文 
        listen_cb,               //接收到连接的回调函数
        base,                    //回调函数获取的参数
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,  //地址重用，listen关闭socket
        10,                       //连接队列大小，对应listen函数
        (sockaddr*)&sin,                          //绑定的地址和端口
        sizeof(sin)
    );
    //事件分发处理
    if(base)
        event_base_dispatch(base);
    if(ev)
        evconnlistener_free(ev);
    if(base)
        event_base_free(base);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

