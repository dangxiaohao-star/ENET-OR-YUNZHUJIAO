#include <iostream>
#include "EventLoop.h"
#include "TcpServer.h"


int main() {
    uint32_t cnt = std::thread::hardware_concurrency();

    EventLoop event_loop(cnt);
    TcpServer* tcp_server = new TcpServer(&event_loop);
    
    tcp_server->Start("0.0.0.0", 8260);   // port取0628反过来
    std::cout << "tcp server start !" << std::endl;

    getchar();
    
    tcp_server->Stop();
    std::cout << "tcp server terminate !" << std::endl;
    return 0;
}