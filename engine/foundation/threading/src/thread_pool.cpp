#include "threading/thread_pool.hpp"
#include <algorithm>

namespace PyNovaGE {
namespace Threading {

ThreadPool::ThreadPool(size_t threads)
    : stop_(false), busy_threads_(0), total_tasks_(0)
{
    // Auto-detect optimal thread count if not specified
    if (threads == 0) {
        threads = std::thread::hardware_concurrency();
        if (threads == 0) threads = 4; // Fallback for older systems
    }
    
    // Reserve one thread for main game logic, use rest for parallel tasks
    threads = std::max(size_t(1), threads - 1);
    
    workers_.reserve(threads);
    
    // Create worker threads
    for (size_t i = 0; i < threads; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    
                    // Wait for task or stop signal
                    condition_.wait(lock, [this] {
                        return stop_ || !tasks_.empty();
                    });
                    
                    if (stop_ && tasks_.empty()) {
                        return; // Exit thread
                    }
                    
                    // Get next task
                    task = std::move(tasks_.front());
                    tasks_.pop();
                    ++busy_threads_;
                }
                
                // Execute task
                task();
                
                // Mark thread as no longer busy
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    --busy_threads_;
                    --total_tasks_;
                    
                    // Notify if all tasks completed
                    if (busy_threads_ == 0 && tasks_.empty()) {
                        finished_.notify_all();
                    }
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    
    condition_.notify_all();
    
    for (std::thread &worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

size_t ThreadPool::pending_tasks() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return tasks_.size() + busy_threads_;
}

void ThreadPool::wait_for_all() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    finished_.wait(lock, [this] {
        return tasks_.empty() && busy_threads_ == 0;
    });
}

bool ThreadPool::is_busy() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return !tasks_.empty() || busy_threads_ > 0;
}

} // namespace Threading
} // namespace PyNovaGE