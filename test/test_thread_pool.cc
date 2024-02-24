#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <sstream>
#include "thread_pool.h" // 确保包含线程池的头文件

void TestThreadPool() {
    // 创建一个包含5个线程的线程池
    ThreadPool pool(5);

    // 添加10个任务到线程池
    for (int i = 0; i < 10; ++i) {
        // 使用lambda表达式作为任务，打印数字
        auto future = pool.AddTask([i] {
            std::stringstream ss;
            ss << "Task " << i << " is running on thread " << std::this_thread::get_id() << std::endl;
            std::cout << ss.str();
        });
    }

    // 等待所有任务完成
    std::cout << "All tasks have been added to the thread pool. Waiting for completion...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 等待1秒，让任务有时间执行

    // 关闭线程池并等待所有线程完成
    std::cout << "ThreadPool has been destroyed.\n";
}

int main() {
    TestThreadPool();
}