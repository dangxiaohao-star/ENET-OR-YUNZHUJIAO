#include "BufferWriter.h"
#include <cstring>          // memcpy函数需要包含这个头文件
#include <sys/socket.h>     // send函数需要包含这个头文件
#include <errno.h>          // errno变量需要包含这个头文件

BufferWriter::BufferWriter(int capacity)
    : max_queue_length_(capacity)
{
}

// 不拷贝数据，直接把外部数据的智能指针放入发送队列，生命周期由外部管理
// 业务层已经有一块数据，并且用 shared_ptr 管理生命周期，那么直接挂进发送队列即可。
bool BufferWriter::Append(std::shared_ptr<char> data, uint32_t size, uint32_t index)
{
    if (size < index) {
        return false;
    }

    if (buffer_.size() >= max_queue_length_) {
        return false;
    }

    Packet pkt = { data, size, index };
    buffer_.emplace(std::move(pkt));
    return true;
}

// 自己拷贝一份数据，生命周期更安全。
bool BufferWriter::Append(const char *data, uint32_t size, uint32_t index)
{
    if (size < index) {
        return false;
    }

    if (buffer_.size() >= max_queue_length_) {
        return false;
    }

    Packet pkt;
    // 这句写法我还是第一次见到，std::default_delete<char[]>()是一个函数对象，用于删除动态分配的数组内存
    // 它会调用delete[]运算符来释放内存
    // 分配一块新内存，自己保存待发送数据
    pkt.data.reset(new char[size + 512], std::default_delete<char[]>());  // 分配内存并设置智能指针的删除器
    // 把外部数据拷贝进自己的缓冲区
    memcpy(pkt.data.get(), data, size);
    pkt.size = size;
    pkt.writeIndex = index;
    buffer_.emplace(std::move(pkt));
    return true;
}

int BufferWriter::Send(int sockfd)
{
    int ret = 0;
    int count = 1;
    do {
        if (buffer_.empty()) {
            return 0;
        }
        count -= 1;
        Packet& pkt = buffer_.front();
         // ::send防御性编程,确保调用全局send函数
        ret = ::send(sockfd, pkt.data.get() + pkt.writeIndex, pkt.size - pkt.writeIndex, 0);
        if (ret > 0) {
            pkt.writeIndex += ret;
            if (pkt.size == pkt.writeIndex) {
                count += 1;
                buffer_.pop();
            }
        }
        else if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN) {
                ret = 0;
            }
        }
    } while (count > 0);
    
    return ret;
}
