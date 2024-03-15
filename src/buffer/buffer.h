#ifndef BUFFER_H_
#define BUFFER_H_

#include <stddef.h>
#include <sys/types.h>

#include <atomic>
#include <string>
#include <vector>

/*                  buffer
-------------------------------------------------
| prependable |   readable   |    writable      |
-------------------------------------------------
^             ^              ^                  ^
0          read_pos      write_pos        buffer.size()

- [0, read_pos) --- 预留空间大小
- [read_pos, write_pos) --- 可读空间大小
- [write_pos, n) --- 可写入空间的大小
*/

class Buffer {
public:
    Buffer(size_t inital_size = 1024);
    ~Buffer() = default;

    // 可读空间大小
    size_t ReadableBytes() const;

    // 可写空间大小
    size_t WritableBytes() const;

    // 预留空间大小
    size_t PrependableBytes() const;

    // 返回可读空间的地址
    const char* Peek() const;

    // 可写空间的起始处
    char* BeginWrite();

    // 向缓冲区中添加数据
    void Append(const char*, size_t);
    void Append(const std::string&);
    void Append(const Buffer&);

    // 已写入len长度的数据，更新 write_pos
    void HasWritten(size_t len);

    // 向fd中写入数据
    ssize_t WriteToFd(int fd, int* err_no);

    // 将fd中的数据读出后写入buffer中
    ssize_t ReadFromFd(int fd, int* err_no);

    // 复位buffer或取出其中的数据
    void Retrieve(size_t len);
    void RetrieveAll();
    std::string RetrieveAllToStr();

private:
    // 返回buffer的起始地址
    const char* Begin() const;
    char* Begin();

    // 为buffer开辟（更多）空间
    void MakeSpace(size_t len);

    // 确保buffer可写入len长度的数据
    void EnsureWritable(size_t len);

private:
    std::vector<char> buffer_;
    std::atomic_size_t read_pos_;
    std::atomic_size_t write_pos_;
};

#endif /* BUFFER_H_ */
