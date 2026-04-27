#include "BufferReader.h"
#include <sys/socket.h>     // recv函数需要包含这个头文件

BufferReader::BufferReader(uint32_t initial_size)
{
    buffer_.resize(initial_size);
}

BufferReader::~BufferReader()
{
}

int BufferReader::Read(int sockfd)
{
    // 简单扩容
    uint32_t size = WriteableBytes();
    if (size < MAX_BYTES_PER_READ) {
        uint32_t bufferReadSize = buffer_.size();
        if (bufferReadSize >= MAX_BUFFER_SIZE) {
            return -1;  // 超过最大缓冲区大小，返回错误   
        }
        buffer_.resize(bufferReadSize + MAX_BYTES_PER_READ);
    }

    // 读数据 从sock接收缓存 -> buffer
    int bytes_read = ::recv(sockfd, BeginWrite(), MAX_BYTES_PER_READ, 0); //::recv防御性编程,确保调用全局recv函数
    if (bytes_read > 0) {
        writer_index_ += bytes_read;
    }
    return bytes_read;
}

uint32_t BufferReader::ReadAll(std::string &data)
{
    uint32_t size = ReadableBytes();
    if (size > 0) {
        data.assign(Peek(), size);
        RetrieveAll();  // 读取完所有数据后,重置读写索引
    }
    return size;
}


