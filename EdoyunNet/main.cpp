#include <iostream>
#include "EventLoop.h"
#include "TcpServer.h"


int main() {
    uint32_t cnt = std::thread::hardware_concurrency();
    EventLoop event_loop(cnt * 0.9);
    TcpServer* tcp_server = new TcpServer(&event_loop);
    
    tcp_server->Start("10.6.0.11", 8260);   // port取0628反过来

    std::cout << "tcp server start !" << std::endl;

    getchar();
    tcp_server->Stop();


    std::cout << "Hello, EdoyunNet!" << std::endl;
    return 0;
}