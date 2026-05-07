#pragma once
#include <map>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <functional>
#include <memory>
#include <cstdint>

typedef std::function<bool (void)> TimerEvent;
typedef uint32_t TimerId;

class Timer {
public:
    Timer(const TimerEvent& event, uint32_t msec)
        : event_callback_(event), interval_(msec)
    {
    }

    ~Timer() = default;
    
    static void Sleep(uint32_t msec) {
        std::this_thread::sleep_for(std::chrono::milliseconds(msec));
    }


private:
    void SetNextTimerOut(int64_t time_point)
    {
        next_timeout_ = time_point + interval_;
    }

    int64_t GetNextTimeout() const {
        return next_timeout_;
    }

private:
    friend class TimerQueue;    // 1.这里为什么用friend class？答：如果不使用friend class，TimerQueue就无法访问Timer的私有成员变量event_callback_和interval_，这可能会导致TimerQueue无法正确管理和执行定时器事件。

    TimerEvent event_callback_ = [] { return false; }; // 定时器事件的回调函数，默认返回false
    uint32_t interval_ = 0; // 定时器的时间间隔，单位为毫秒
    int64_t next_timeout_ = 0; // 定时器的下一个超时时间点，单位为毫秒
};

class TimerQueue {
public:
    TimerId AddTimer(const TimerEvent& event, uint32_t msec);
    void RemoveTimer(TimerId timer_id);
    void HandleTimerEvent();

private:
    int64_t GetTimeNow();
    std::unordered_map<TimerId, std::shared_ptr<Timer>> timers_; // 定时器ID与定时器对象的映射关系
    std::map<std::pair<int64_t, TimerId>, std::shared_ptr<Timer>> events_; // 定时器超时时间点与定时器ID的映射关系
    
private:
    uint32_t last_timer_id_ = 0; // 上一个定时器ID，用于生成新的定时器ID
};

// 关于为什么pair<int64_t, TimerId>作为map的key：
// 多个定时器可能在完全相同的时间到期，
// 而 std::map 要求 Key 唯一。所以这里使用 (时间, ID) 作为复合键，确保每个键都是唯一的，
// int64_t 存储的是绝对时间戳（毫秒），表示定时器应该被触发的时间点，即超时时间点 。
//使用 std::map 自动按这个时间排序，确保我们能快速找到并处理到期的定时器
