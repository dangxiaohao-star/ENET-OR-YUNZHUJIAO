#include <cstdint>
#include <vector>
#include <string>

class BufferReader {
public:
    BufferReader(uint32_t initial_size = 2048);
    virtual ~BufferReader();

    // 获取当前缓冲区可读数据大小
    uint32_t ReadableBytes() const { return (uint32_t)(writer_index_ - reader_index_); }
    // 获取当前缓冲区可写数据大小
    // 如果写入数据比可写空间大，需要对缓冲区进行扩容操作
    uint32_t WriteableBytes() const { return (uint32_t)(buffer_.size() - writer_index_); }

    // 获取可读数据的首地址，只看不取。
    char* Peek() { return Begin() + reader_index_; }
    const char* Peek() const { return Begin() + reader_index_; }

    // 重新初始化缓冲区读写索引
    void RetrieveAll() {
        writer_index_ = 0;
        reader_index_ = 0;
    }

    // 获取len字节数据后,更新缓冲区读写索引
    void Retrieve(size_t len) {
        if (len <= ReadableBytes()) {
            reader_index_ += len;
            // 如果可读数据全部消费完，就重置两个下标
            // 这样下次可以从 buffer_ 起点重新写，避免一直增长
            if (reader_index_ == writer_index_) {
                RetrieveAll();
            }
        } else {
            // 如果想取的数据比已有数据还多，直接清空
            // 这个行为比较粗暴，实际项目里更建议 assert 或返回错误
            RetrieveAll();
        }
    }

    int Read(int sockfd);

    // 获取缓冲区所有数据
    uint32_t ReadAll(std::string& data);

    uint32_t Size() const { return (uint32_t)buffer_.size(); }

private:
    char* Begin() { return &*buffer_.begin(); }
    const char* Begin() const { return &*buffer_.begin(); }

    char* BeginWrite() { return Begin() + writer_index_; }
    const char* BeginWrite() const { return Begin() + writer_index_; }

    std::vector<char> buffer_;
    size_t reader_index_ = 0;
    size_t writer_index_ = 0;
    static const uint32_t MAX_BYTES_PER_READ = 4096;
    static const uint32_t MAX_BUFFER_SIZE = 1024 * 100000;  // 100MB
};