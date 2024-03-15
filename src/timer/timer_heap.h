#ifndef TIMER_HEAP_H_
#define TIMER_HEAP_H_

#include <cstddef>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <vector>

using TimeoutCallback = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using MS = std::chrono::milliseconds;
using Timestamp = Clock::time_point; // 可与Clock::now() 进行比较

struct TimerNode {
    int id;
    Timestamp expires;
    TimeoutCallback cb;

    bool operator<(const TimerNode& t) {
        return expires < t.expires;
    }
};

class TimerHeap {
public:
    TimerHeap() {
        heap_.reserve(64);
    }

    ~TimerHeap() {
        Clear();
    }

    int GetNextTick();
    void Add(int id, int timeout, const TimeoutCallback& cb);
    void Adjust(int id, int new_expires);
    void DoWork(int id);
    void Clear();
    void Tick();
    void Pop();

private:
    void Delete(std::size_t index);
    void ShiftUp(std::size_t i);
    bool ShiftDown(std::size_t index, std::size_t n);
    void SwapNode(std::size_t i, std::size_t j);

private:
    std::vector<TimerNode> heap_;
    // 存储定时器节点的id和它们在堆中的索引之间的映射
    std::unordered_map<int, std::size_t> ref_;
};

#endif /* TIMER_HEAP_H_ */
