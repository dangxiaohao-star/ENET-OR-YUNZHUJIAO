#include "EpollTaskScheduler.h"
#include <sys/epoll.h>
#include <errno.h>
#include <iostream>

EpollTaskScheduler::EpollTaskScheduler(int id)
    : TaskScheduler(id)
{
    // 创建epoll
    epollfd_ = epoll_create(1024);
}

EpollTaskScheduler::~EpollTaskScheduler()
{
}

void EpollTaskScheduler::UpdateChannel(ChannelPtr channel)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int fd = channel->GetSockfd();
    if (channels_.find(fd) !=  channels_.end()) {
        if (channel->IsNoneEvent()) {
            Update(EPOLL_CTL_DEL, channel);
            channels_.erase(fd);
        }
        else {
            Update(EPOLL_CTL_MOD, channel);
        }
    }
    else {
        if (!channel->IsNoneEvent()) {
            channels_.emplace(fd, channel);
            Update(EPOLL_CTL_ADD, channel);
        }
    }
}

void EpollTaskScheduler::RemoveChannel(ChannelPtr channel)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int fd = channel->GetSockfd();
    if (channels_.find(fd) != channels_.end()) {
        Update(EPOLL_CTL_DEL, channel);
        channels_.erase(fd);
    }
}

bool EpollTaskScheduler::HandleEvent()
{
    struct epoll_event epoll_events[1024] = { 0 };
    int num_events = -1;

    num_events = epoll_wait(epollfd_, epoll_events, 1024, 0); // timeout = 0代表什么？
    if (num_events < 0) {
        if (errno != EINTR) return false;
    }

    for (int i = 0; i < num_events; ++i) {
        if (epoll_events[i].data.ptr) {
            static_cast<Channel*>(epoll_events[i].data.ptr)->HandleEvent(epoll_events[i].events);
        }
    }
    return true;
}

void EpollTaskScheduler::Update(int operation, ChannelPtr &channel)
{
    struct epoll_event event = { 0 };
    if (operation == EPOLL_CTL_ADD || operation == EPOLL_CTL_MOD) {
        event.data.ptr = channel.get();
        event.events = channel->GetEvents();
    }

    if (::epoll_ctl(epollfd_, operation, channel->GetSockfd(), &event) < 0) {
        std::cout << "修改epoll失败" << std::endl;
    }
}
