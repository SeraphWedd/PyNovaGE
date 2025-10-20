#include <benchmark/benchmark.h>
#include "threading/thread_pool.hpp"
#include <vector>
#include <numeric>
#include <chrono>
#include <random>

using namespace PyNovaGE::Threading;

// Simulate CPU-intensive work (like AI updates or physics calculations)
static void CPUIntensiveWork(int workload) {
    volatile int result = 0;
    for (int i = 0; i < workload; ++i) {
        result += i * i;
    }
}

// Benchmark single-threaded vs multi-threaded execution
static void BM_SingleThreaded_AIUpdates(benchmark::State& state) {
    const auto num_objects = static_cast<int>(state.range(0));
    const int work_per_object = 10000;
    
    for (auto _ : state) {
        for (int i = 0; i < num_objects; ++i) {
            CPUIntensiveWork(work_per_object);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * num_objects);
    state.SetLabel("SingleThread");
}

static void BM_MultiThreaded_AIUpdates(benchmark::State& state) {
    const auto num_objects = static_cast<int>(state.range(0));
    const int work_per_object = 10000;
    
    ThreadPool pool;
    
    for (auto _ : state) {
        std::vector<std::future<void>> futures;
        futures.reserve(num_objects);
        
        for (int i = 0; i < num_objects; ++i) {
            futures.emplace_back(pool.enqueue([work_per_object]() {
                CPUIntensiveWork(work_per_object);
            }));
        }
        
        // Wait for all to complete
        for (auto& future : futures) {
            future.wait();
        }
    }
    
    state.SetItemsProcessed(state.iterations() * num_objects);
    state.SetLabel("MultiThread");
}

// Benchmark parallel_for implementation
static void BM_ParallelFor_Updates(benchmark::State& state) {
    const auto num_objects = static_cast<size_t>(state.range(0));
    const int work_per_object = 10000;
    
    ThreadPool pool;
    
    for (auto _ : state) {
        parallel_for(0, num_objects, [work_per_object](size_t /*i*/) {
            CPUIntensiveWork(work_per_object);
        }, &pool);
    }
    
    state.SetItemsProcessed(state.iterations() * num_objects);
    state.SetLabel("ParallelFor");
}

// Benchmark batch processing (optimal for MMO scenarios)
static void BM_ParallelBatch_Updates(benchmark::State& state) {
    const auto num_objects = static_cast<size_t>(state.range(0));
    const int work_per_object = 10000;
    const int batch_size = 50; // Process 50 objects per batch
    
    std::vector<int> objects(num_objects);
    std::iota(objects.begin(), objects.end(), 0);
    
    ThreadPool pool;
    
    for (auto _ : state) {
        parallel_batch(objects, batch_size, [work_per_object](const std::vector<int>& batch) {
            for (size_t i = 0; i < batch.size(); ++i) {
                CPUIntensiveWork(work_per_object);
            }
        }, &pool);
    }
    
    state.SetItemsProcessed(state.iterations() * num_objects);
    state.SetLabel("ParallelBatch");
}

// Benchmark thread pool overhead
static void BM_ThreadPool_Overhead(benchmark::State& state) {
    const auto num_tasks = static_cast<int>(state.range(0));
    ThreadPool pool;
    
    for (auto _ : state) {
        std::vector<std::future<int>> futures;
        futures.reserve(num_tasks);
        
        for (int i = 0; i < num_tasks; ++i) {
            futures.emplace_back(pool.enqueue([i]() {
                return i * 2; // Minimal work
            }));
        }
        
        int total = 0;
        for (auto& future : futures) {
            total += future.get();
        }
        benchmark::DoNotOptimize(total);
    }
    
    state.SetItemsProcessed(state.iterations() * num_tasks);
    state.SetLabel("ThreadPoolOverhead");
}

// Memory allocation benchmark (simulate object creation in MMO)
static void BM_ParallelObjectCreation(benchmark::State& state) {
    const auto num_objects = static_cast<int>(state.range(0));
    ThreadPool pool;
    
    struct GameObject {
        float position[3];
        float velocity[3];
        int health;
        int id;
        
        GameObject(int _id) : health(100), id(_id) {
            for (int i = 0; i < 3; ++i) {
                position[i] = static_cast<float>(id * 0.1f);
                velocity[i] = 0.0f;
            }
        }
    };
    
    for (auto _ : state) {
        std::vector<std::future<std::unique_ptr<GameObject>>> futures;
        futures.reserve(num_objects);
        
        for (int i = 0; i < num_objects; ++i) {
            futures.emplace_back(pool.enqueue([i]() {
                return std::make_unique<GameObject>(i);
            }));
        }
        
        std::vector<std::unique_ptr<GameObject>> objects;
        objects.reserve(num_objects);
        
        for (auto& future : futures) {
            objects.push_back(future.get());
        }
        
        benchmark::DoNotOptimize(objects);
    }
    
    state.SetItemsProcessed(state.iterations() * num_objects);
    state.SetLabel("ParallelObjectCreation");
}

// Register benchmarks with different object counts (MMO scale)
BENCHMARK(BM_SingleThreaded_AIUpdates)
    ->RangeMultiplier(2)
    ->Range(100, 2000)
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_MultiThreaded_AIUpdates)
    ->RangeMultiplier(2)
    ->Range(100, 2000)
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_ParallelFor_Updates)
    ->RangeMultiplier(2)
    ->Range(100, 2000)
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_ParallelBatch_Updates)
    ->RangeMultiplier(2)
    ->Range(100, 2000)
    ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_ThreadPool_Overhead)
    ->RangeMultiplier(2)
    ->Range(1000, 16000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK(BM_ParallelObjectCreation)
    ->RangeMultiplier(2)
    ->Range(500, 4000)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();