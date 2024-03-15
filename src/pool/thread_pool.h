#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <memory>

class ThreadPool {
public:
    explicit ThreadPool(std::size_t pool_size) {
        is_close_ = false; // 线程池开始运行
        for (std::size_t i = 0; i < pool_size; i++) {
            pool_.emplace_back([this] {
                // 不断从tasks队列中取出一个任务执行
                while (true) {
                    Task task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        // 当 tasks队列 为空，或者 stop_ 为true时不再执行wait操作
                        // wait操作解释：https://zh.cppreference.com/w/cpp/thread/condition_variable/wait
                        this->cond_.wait(lock, [this] {
                            return this->is_close_ || !this->tasks_.empty();
                        });
                        if (this->is_close_ || this->tasks_.empty())
                            return;
                        task = std::move(tasks_.front());
                        this->tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            is_close_ = true;
        }
        // 让还位于线程池中的线程执行完
        cond_.notify_all();
        for (auto& work : pool_) {
            work.join();
        }
    }

    template <typename F, typename... Args>
    auto AddTask(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using RetType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<RetType> future = task->get_future();
        // 往任务队列中添加一个任务，当线程池中有空闲线程时即可取出执行
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.emplace([task] { (*task)(); });
        }
        cond_.notify_one();
        return future;
    }

private:
    using Task = std::function<void()>;

    std::vector<std::thread> pool_; // 线程池
    std::queue<Task> tasks_;        // 任务队列
    std::mutex mutex_;
    std::condition_variable cond_;
    bool is_close_;
};

#endif

#endif /* THREAD_POOL_H_ */
