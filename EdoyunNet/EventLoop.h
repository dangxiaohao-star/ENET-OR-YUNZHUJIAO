#include "EpollTaskScheduler.h"
#include <vector>


class EventLoop{
public:
    explicit EventLoop(uint32_t num_threads = -1);
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    std::shared_ptr<TaskScheduler> GetTaskScheduler();
    
    // 从 TaskScheduler.h 复制来的
    TimerId AddTimer(const TimerEvent& event, uint32_t msec);
    void RemoveTimer(TimerId id);
    
    void UpdateChannel(ChannelPtr channel);
    void RemoveChannel(ChannelPtr channel);

    void StartLoop();
    void QuitLoop();


private:
    uint32_t num_threads_ = 1;
    std::atomic<uint32_t> index_ { 0 };
    std::vector<std::shared_ptr<TaskScheduler>> task_schedulers_;
    std::vector<std::shared_ptr<std::thread>> threads_;     // TODO：vector<thread>
    std::mutex mutex_;
};