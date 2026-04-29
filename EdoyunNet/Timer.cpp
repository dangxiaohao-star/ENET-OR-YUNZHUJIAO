#include "Timer.h"

TimerId TimerQueue::AddTimer(const TimerEvent &event, uint32_t msec)
{
    int64_t timer_point = GetTimeNow();
    TimerId timer_id = ++last_timer_id_;

    auto timer = std::make_shared<Timer>(event, msec);
    timer->SetNextTimerOut(timer_point);
    timers_.emplace(timer_id, timer);
    events_.emplace(std::pair<int64_t, TimerId>(timer->GetNextTimeout(), timer_id), timer);
    return timer_id;
}

void TimerQueue::RemoveTimer(TimerId timer_id)
{
    auto iter = timers_.find(timer_id);
    if (iter != timers_.end()) {
        int64_t timeout = iter->second->GetNextTimeout();
        events_.erase(std::pair<int64_t, TimerId>(timeout, timer_id));
        timers_.erase(timer_id);
    }
}

void TimerQueue::HandleTimerEvent()
{
    if (!timers_.empty()) {
        int64_t time_point = GetTimeNow();
        while (!timers_.empty() && events_.begin()->first.first <= time_point) {
            TimerId timer_id = events_.begin()->first.second;
            bool flag = events_.begin()->second->event_callback_();
            if (flag) { // 反复执行
                events_.begin()->second->SetNextTimerOut(time_point);
                auto timePtr = std::move(events_.begin()->second);
                events_.erase(events_.begin());
                events_.emplace(std::pair<int64_t, TimerId>(timePtr->GetNextTimeout(), timer_id), timePtr);
            }
            else {
                events_.erase(events_.begin());
                timers_.erase(timer_id);
            }
        }
    }
}

int64_t TimerQueue::GetTimeNow()
{
    auto time_point = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch()).count();
    return 0;
}
