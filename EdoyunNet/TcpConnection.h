#include "BufferReader.h"
#include "BufferWriter.h"
#include "TcpSocket.h"
#include "Channel.h"
#include "TaskScheduler.h"

// std::enable_shared_from_this<TcpConnection>在干嘛？
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using TcpConnPtr = std::shared_ptr<TcpConnection>;
    using DisConnectCallback = std::function<void(std::shared_ptr<TcpConnection> conn)>;
    using CloseCallback = std::function<void(std::shared_ptr<TcpConnection> conn)>;
    using ReadCallback = std::function<bool(std::shared_ptr<TcpConnection> conn, BufferReader& r_buffer)>;
    
    // 通过外部传入的sockfd创建Channel,并添加到TaskScheduler对象中进行事件监听和处理
    TcpConnection(TaskScheduler* task_scheduler, int sockfd);
    virtual ~TcpConnection();

    inline TaskScheduler* GetTaskScheduler() const { return task_scheduler_; }

    // 分别设置读、写回调函数
    // 补充：C++ 17类内定义的函数 默认就是 inline。但是明确写出对读者友好
    inline void SetReadCallback(const ReadCallback &cb) { read_cb_ = cb; }
    inline void SetCloseCallback(const CloseCallback &cb) { close_cb_ = cb; }

    // 关于inline的补充：它们的真实目的不是“加速”，而是：允许多个翻译单元中有相同定义
    // 允许这些函数在头文件（.h）中定义，而不违反 ODR（一次定义规则）
    inline bool IsClosed() const { return is_closed_; }
    inline int GetSocket() const { return channel_->GetSockfd(); }
    
    void Send(std::shared_ptr<char> data, uint32_t size);
    void Send(const char* data, uint32_t size);

    void Disconnect();

protected:
    friend class TcpServer;

    virtual void HandleRead();      // 处理读事件, 内核 Socket 接收缓冲区 → r_buffer_（应用层读缓冲区）
    virtual void HandleWrite();     // 处理写事件, w_buffer_（应用层写缓冲区）→ 内核 Socket 发送缓冲区
    virtual void HandleError();
    virtual void HandleClose();

    void SetDisconnectCallback(const DisConnectCallback &cb) { disconnect_cb_ = cb; }

    TaskScheduler* task_scheduler_;
    std::unique_ptr<BufferReader> r_buffer_;
    std::unique_ptr<BufferWriter> w_buffer_;
    bool is_closed_ = true;

private:
    void Close();
    std::shared_ptr<Channel> channel_ = nullptr;
    DisConnectCallback disconnect_cb_ {};
    CloseCallback close_cb_ {};
    ReadCallback read_cb_ {};
};