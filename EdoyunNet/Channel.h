#include <functional>
#include <memory>

enum EventType {
    EVENT_NONE  = 0,
    EVENT_IN    = 1,
    EVENT_PRI   = 2,
    EVENT_OUT   = 4,
    EVENT_ERR   = 8,
    EVENT_HUP   = 16,
};

class Channel {
public:
    typedef std::function<void()> EventCallback;

    Channel(int sockfd) : sockfd_(sockfd) {}
    ~Channel() {};

    // 1. 注册各类事件的回调函数
    inline void SetReadCallback(const EventCallback& cb) { 
        read_callback_ = cb; 
    }

    inline void SetWriteCallback(const EventCallback& cb) {
        write_callback_ = cb;
    }

    inline void SetCloseCallback(const EventCallback& cb) {
        close_callback_ = cb;
    }

    inline void SetErrorCallback(const EventCallback& cb) {
        error_callback_ = cb;
    }

    // 2. 获取和设置成员变量
    inline int GetSockfd() const { return sockfd_; }
    inline int GetEvents() const { return events_; }
    inline void SetEvents(int events) { events_ = events; }

    // 3. 注册事件
    inline void EnableReading() { events_ |= EVENT_IN; }
    inline void EnableWriting() { events_ |= EVENT_OUT; }

    // 移除事件
    inline void DisableReading() { events_ &= ~EVENT_IN; }
    inline void DisableWriting() { events_ &= ~EVENT_OUT; }

    // 4. 判断是否属于某类事件
    inline bool IsNoneEvent() const { return events_ == EVENT_NONE; }
    inline bool IsReading() const { return events_ & EVENT_IN; }
    inline bool IsWriting() const { return events_ & EVENT_OUT; }

    // 5. 根据事件触发对应回调函数
    void HandleEvent(int events) {
        if (events & (EVENT_PRI | EVENT_IN)) {
            read_callback_();
        }

        if (events & EVENT_OUT) {
            write_callback_();
        }

        if (events & EVENT_HUP) {
            close_callback_();
            return;
        }

        if (events & EVENT_ERR) {
            error_callback_();
        }
    }

private:
    EventCallback read_callback_ = []{};    // 这是什么写法？
    EventCallback write_callback_ = []{};   // []{}是一个 “空的匿名函数”，即一个 无参、无返回值、无操作的 Lambda
    EventCallback close_callback_ = []{};   // 这种写法的好处是，避免了回调函数未设置时，调用时出现空指针异常（nullptr）
    EventCallback error_callback_ = []{};   // 而是直接调用一个空的函数对象，不会有任何副作用
    int sockfd_ = 0;    // channel封装的fd
    int events_ = 0;    // 该fd感兴趣的事件
};

typedef std::shared_ptr<Channel> ChannelPtr;