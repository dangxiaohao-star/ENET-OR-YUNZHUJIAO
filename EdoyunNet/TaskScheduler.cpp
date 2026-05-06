#include "TaskScheduler.h"

TaskScheduler::TaskScheduler(int id)
    : id_(id), is_shutdown_(false)
{
}

TaskScheduler::~TaskScheduler()
{
}

void TaskScheduler::Start()
{
    is_shutdown_ = false;
    while (!is_shutdown_) {
        // 处理定时事件
        this->timer_queue_.HandleTimerEvent();
        // 处理 I/O 事件
        this->HandleEvent();
    }
}

void TaskScheduler::Stop()
{
    is_shutdown_ = true;
}

TimerId TaskScheduler::AddTimer(const TimerEvent &event, uint32_t msec)
{
    TimerId timer_id = timer_queue_.AddTimer(event, msec);
    return timer_id;
}

void TaskScheduler::RemoveTimer(TimerId id)
{
    timer_queue_.RemoveTimer(id);
}
