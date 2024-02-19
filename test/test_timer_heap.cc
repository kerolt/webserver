#include "timer_heap.h"
#include <iostream>
#include <chrono>
#include <thread>

// 定义一个简单的回调函数
void TestCallback(int id) {
    std::cout << "Callback for timer " << id << " is triggered at " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() << "ms" << std::endl;
}

int main() {
    // 创建HeapTimer实例
    TimerHeap timer;

    // 添加定时器
    timer.Add(1, 1000, [=] { TestCallback(1); }); // 1秒后触发
    timer.Add(2, 2000, [=] { TestCallback(2); }); // 2秒后触发

    // 等待一段时间，模拟时间流逝
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 调整定时器2的超时时间
    timer.Adjust(2, 3000); // 将定时器2的超时时间调整为3秒后

    // 获取下一个定时器的超时时间
    int nextTick = timer.GetNextTick();
    std::cout << "Next tick in " << nextTick << "ms" << std::endl;

    // 再次等待一段时间，直到下一个定时器到期
    std::this_thread::sleep_for(std::chrono::milliseconds(nextTick));

    // 再次获取下一个定时器的超时时间
    nextTick = timer.GetNextTick();
    std::cout << "Next tick in " << nextTick << "ms" << std::endl;

    // 等待所有定时器执行完毕
    std::this_thread::sleep_for(std::chrono::seconds(3));

    return 0;
}