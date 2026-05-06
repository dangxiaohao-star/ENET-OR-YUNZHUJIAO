#include "EventLoop.h"

EventLoop::EventLoop(uint32_t num_threads)
    : num_threads_(num_threads), index_(0)
{
    this->StartLoop();  // 有隐患,尽量手动显式启动EventLoop（TODO）
}

EventLoop::~EventLoop()
{
    this->QuitLoop();
}

std::shared_ptr<TaskScheduler> EventLoop::GetTaskScheduler()
{
    if (task_schedulers_.empty()) {
        return nullptr;
    }
    
    // 单线程版
    if (task_schedulers_.size() == 1) {
        return task_schedulers_.at(0);
    }

    // Main-Sub主从模式
    uint32_t idx = index_.fetch_add(1);
    uint32_t sub_idx = 1 + (idx % (task_schedulers_.size() - 1)); // 取模计算，保证返回的 Scheduler 一定在 1 ~ (size - 1) 之间

    return task_schedulers_.at(sub_idx);
}

TimerId EventLoop::AddTimer(const TimerEvent &event, uint32_t msec)
{
    if (task_schedulers_.size() > 0) {
        task_schedulers_[0]->AddTimer(event, msec); // 为什么是task_schedulers_[0]?
    }
    return 0;
}

void EventLoop::RemoveTimer(TimerId id)
{
    if (task_schedulers_.size() > 0) {
        task_schedulers_[0]->RemoveTimer(id); // 为什么是task_schedulers_[0]?
    }
    return;
}

// TODO:这个【0】有点搞
void EventLoop::UpdateChannel(ChannelPtr channel)
{
    if (task_schedulers_.size() > 0) {
        task_schedulers_[0]->UpdateChannel(channel); // 为什么是task_schedulers_[0]?
    }
}

void EventLoop::RemoveChannel(ChannelPtr channel)
{
    if (task_schedulers_.size() > 0) {
        task_schedulers_[0]->RemoveChannel(channel); // 为什么是task_schedulers_[0]?
    }
}

void EventLoop::StartLoop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!task_schedulers_.empty()) {
        return;
    }

    for (uint32_t i = 0; i < num_threads_; ++i) {
        std::shared_ptr<TaskScheduler> task_scheduler_ptr = std::make_shared<TaskScheduler>(i);
        task_schedulers_.push_back(task_scheduler_ptr);

        threads_.push_back(std::make_shared<std::thread>(&TaskScheduler::Start, task_scheduler_ptr));
    }
}

void EventLoop::QuitLoop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (task_schedulers_.empty() && threads_.empty()) {
        return;
    }
    for (auto& iter : task_schedulers_) {
        iter->Stop();
    }

    for (auto& thread : threads_) {
        if (thread && thread->joinable()) {
            thread->join();
        }
    }
    
    task_schedulers_.clear();
    threads_.clear();
}
