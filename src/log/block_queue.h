#ifndef BLOCK_QUEUE_H_
#define BLOCK_QUEUE_H_

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <deque>

/*
    经典的生产者消费者模型
*/
template <class T>
class BlockQueue {
public:
    BlockQueue(size_t max_size = 1000)
        : capacity_(max_size)
        , is_close_(false) {}

    ~BlockQueue() {
        Close();
    }

    void Push(const T& data);
    bool Pop(T& data);
    T Front();
    void Flush();
    void Close();
    bool Empty();
    bool Full();

private:
    std::deque<T> queue_;
    size_t capacity_;
    bool is_close_;
    std::condition_variable produce_cond_;
    std::condition_variable consume_cond_;
    std::mutex mutex_;
};

template <class T>
void BlockQueue<T>::Push(const T& data) {
    std::unique_lock<std::mutex> lock(mutex_);
    // 当前队列容量小于max_size则不阻塞
    // “生产者”阻塞
    produce_cond_.wait(lock, [this] { return queue_.size() < capacity_; });
    queue_.push_back(data);
    consume_cond_.notify_one(); // 通知“消费者”
}

template <class T>
bool BlockQueue<T>::Pop(T& data) {
    std::unique_lock<std::mutex> lock(mutex_);

    // 当队列中无数据时“消费者”阻塞
    // 需要注意的是wait的第二个参数，当其返回 false 时才会继续阻塞
    // 当阻塞队列被关闭时，!queue_.empty() || is_close_ = true，此时不会再阻塞
    consume_cond_.wait(lock, [this] { return !queue_.empty() || is_close_; });
    if (is_close_) {
        return false;
    }

    data = queue_.front();
    queue_.pop_front();
    produce_cond_.notify_one(); // 通知“生产者”
    return true;
}

template <class T>
void BlockQueue<T>::Flush() {
    // 唤醒消费者, 即可以从队列中取出数据了
    consume_cond_.notify_one();
}

template <class T>
void BlockQueue<T>::Close() {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
    is_close_ = true;
    consume_cond_.notify_all();
    produce_cond_.notify_all();
}

template <class T>
T BlockQueue<T>::Front() {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.front();
}

template <class T>
bool BlockQueue<T>::Empty() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

template <class T>
bool BlockQueue<T>::Full() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size() >= capacity_;
}

#endif /* BLOCK_QUEUE_H_ */
