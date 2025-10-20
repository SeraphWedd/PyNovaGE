#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>

namespace PyNovaGE {
namespace Threading {

/**
 * @brief High-performance thread pool for MMO-scale parallel processing
 * 
 * Optimized for:
 * - Batch physics updates for many players/NPCs
 * - Parallel voxel meshing
 * - Concurrent AI updates
 * - Background asset streaming
 */
class ThreadPool {
public:
    /**
     * @brief Create thread pool with specified number of worker threads
     * @param threads Number of worker threads (0 = auto-detect)
     */
    explicit ThreadPool(size_t threads = 0);
    
    /**
     * @brief Destructor - waits for all tasks to complete
     */
    ~ThreadPool();

    /**
     * @brief Enqueue a task for execution
     * @param f Function to execute
     * @param args Arguments to pass to function
     * @return Future that will contain the result
     */
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    /**
     * @brief Get number of worker threads
     */
    size_t size() const { return workers_.size(); }

    /**
     * @brief Get number of pending tasks
     */
    size_t pending_tasks() const;

    /**
     * @brief Wait for all current tasks to complete
     */
    void wait_for_all();

    /**
     * @brief Check if any tasks are currently running or queued
     */
    bool is_busy() const;

private:
    // Worker threads
    std::vector<std::thread> workers_;
    
    // Task queue
    std::queue<std::function<void()>> tasks_;
    
    // Synchronization
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::condition_variable finished_;
    
    // Control flags
    std::atomic<bool> stop_;
    std::atomic<size_t> busy_threads_;
    std::atomic<size_t> total_tasks_;
};

/**
 * @brief Parallel for loop implementation
 * @param start Start index (inclusive)
 * @param end End index (exclusive)  
 * @param func Function to execute for each index
 * @param pool Thread pool to use (nullptr = create temporary pool)
 */
template<typename Func>
void parallel_for(size_t start, size_t end, Func func, ThreadPool* pool = nullptr);

/**
 * @brief Parallel for each implementation
 * @param container Container to iterate over
 * @param func Function to execute for each element
 * @param pool Thread pool to use (nullptr = create temporary pool)
 */
template<typename Container, typename Func>
void parallel_for_each(Container& container, Func func, ThreadPool* pool = nullptr);

/**
 * @brief Batch parallel execution - optimal for MMO scenarios
 * @param items Vector of items to process
 * @param batch_size Number of items per batch
 * @param func Function to execute on each batch
 * @param pool Thread pool to use (nullptr = create temporary pool)
 */
template<typename T, typename Func>
void parallel_batch(const std::vector<T>& items, size_t batch_size, Func func, ThreadPool* pool = nullptr);

} // namespace Threading
} // namespace PyNovaGE

// Template implementations
namespace PyNovaGE {
namespace Threading {

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::invoke_result<F, Args...>::type>
{
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        // Don't allow enqueueing after stopping the pool
        if (stop_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        tasks_.emplace([task]() { (*task)(); });
        ++total_tasks_;
    }
    
    condition_.notify_one();
    return res;
}

template<typename Func>
void parallel_for(size_t start, size_t end, Func func, ThreadPool* pool) {
    if (start >= end) return;
    
    bool owns_pool = false;
    if (!pool) {
        pool = new ThreadPool();
        owns_pool = true;
    }
    
    const size_t range = end - start;
    const size_t num_threads = std::min(range, pool->size());
    const size_t chunk_size = range / num_threads;
    
    std::vector<std::future<void>> futures;
    futures.reserve(num_threads);
    
    for (size_t i = 0; i < num_threads; ++i) {
        const size_t chunk_start = start + i * chunk_size;
        const size_t chunk_end = (i == num_threads - 1) ? end : chunk_start + chunk_size;
        
        futures.emplace_back(pool->enqueue([chunk_start, chunk_end, func]() {
            for (size_t idx = chunk_start; idx < chunk_end; ++idx) {
                func(idx);
            }
        }));
    }
    
    // Wait for all chunks to complete
    for (auto& future : futures) {
        future.wait();
    }
    
    if (owns_pool) {
        delete pool;
    }
}

template<typename Container, typename Func>
void parallel_for_each(Container& container, Func func, ThreadPool* pool) {
    parallel_for(0, container.size(), [&container, func](size_t i) {
        func(container[i]);
    }, pool);
}

template<typename T, typename Func>
void parallel_batch(const std::vector<T>& items, size_t batch_size, Func func, ThreadPool* pool) {
    bool owns_pool = false;
    if (!pool) {
        pool = new ThreadPool();
        owns_pool = true;
    }
    
    std::vector<std::future<void>> futures;
    
    for (size_t start = 0; start < items.size(); start += batch_size) {
        const size_t end = std::min(start + batch_size, items.size());
        
        futures.emplace_back(pool->enqueue([&items, start, end, func]() {
            std::vector<T> batch(items.begin() + start, items.begin() + end);
            func(batch);
        }));
    }
    
    // Wait for all batches to complete
    for (auto& future : futures) {
        future.wait();
    }
    
    if (owns_pool) {
        delete pool;
    }
}

} // namespace Threading  
} // namespace PyNovaGE