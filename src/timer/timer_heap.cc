#include "timer_heap.h"

#include <bits/chrono.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>

#include "log.h"

void TimerHeap::Add(int id, int timeout, const TimeoutCallback& cb) {
    assert(id >= 0);
    std::size_t i;
    if (ref_.count(id) == 0) {
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeout), cb});
        ShiftUp(i);
    } else {
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeout);
        heap_[i].cb = cb;
        // 将节点下沉到正确的位置后，如果下沉操作没有成功（即节点仍然大于其子节点），则需要将该节点向上调整以恢复最小堆的性质
        if (!ShiftDown(i, heap_.size())) {
            ShiftUp(i);
        }
    }
}

// 获取下一个定时器的超时时间
int TimerHeap::GetNextTick() {
    Tick();
    std::size_t ret = -1;
    if (!heap_.empty()) {
        ret = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if (ret < 0) {
            ret = 0;
        }
    }
    return ret;
}

void TimerHeap::Adjust(int id, int new_expires) {
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(new_expires);
    ShiftDown(ref_[id], heap_.size());
}

void TimerHeap::DoWork(int id) {
    if (heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    std::size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.cb();
    Delete(i);
}

void TimerHeap::Clear() {
    ref_.clear();
    heap_.clear();
}

void TimerHeap::Tick() {
    // if (heap_.empty()) {
    //     return;
    // }
    while (!heap_.empty()) {
        TimerNode node = heap_.front();
        if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {
            break; // 说明当前没有到期的定时器
        }
        node.cb();
        Pop();
    }
}

void TimerHeap::Pop() {
    assert(!heap_.empty());
    Delete(0);
}

void TimerHeap::Delete(std::size_t index) {
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    std::size_t end = heap_.size() - 1;
    assert(index <= end);
    if (index < end) {
        SwapNode(index, end);
        if (!ShiftDown(index, end)) {
            ShiftUp(index);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void TimerHeap::ShiftUp(std::size_t i) {
    assert(i >= 0 && i < heap_.size());
    std::size_t j = (i - 1) / 2;
    while (j >= 0) {
        if (heap_[j] < heap_[i]) {
            break; // 小根堆
        }
        SwapNode(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

bool TimerHeap::ShiftDown(std::size_t index, std::size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    std::size_t i = index, j = i * 2 + 1;
    while (j < n) {
        if (j + 1 < n && heap_[j + 1] < heap_[j]) {
            j++;
        }
        if (heap_[i] < heap_[j]) {
            break;
        }
        SwapNode(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void TimerHeap::SwapNode(std::size_t i, std::size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}
