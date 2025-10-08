#include "../../include/geometry/hermite.hpp"
#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include <memory>

namespace pynovage {
namespace math {
namespace tests {

// Helper to create random points and tangents
struct TestData {
    Vector3 p0, p1, t0, t1;
    float tension;

    static TestData random() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
        std::uniform_real_distribution<float> tension_dist(0.1f, 5.0f);

        return {
            Vector3(dist(gen), dist(gen), dist(gen)),
            Vector3(dist(gen), dist(gen), dist(gen)),
            Vector3(dist(gen), dist(gen), dist(gen)),
            Vector3(dist(gen), dist(gen), dist(gen)),
            tension_dist(gen)
        };
    }
};

// Basic construction and evaluation
static void BM_HermiteConstruction(benchmark::State& state) {
    auto data = TestData::random();
    
    for (auto _ : state) {
        Hermite curve(data.p0, data.p1, data.t0, data.t1, data.tension);
        benchmark::DoNotOptimize(curve);
    }
}

static void BM_HermiteEvaluation(benchmark::State& state) {
    auto data = TestData::random();
    Hermite curve(data.p0, data.p1, data.t0, data.t1, data.tension);
    std::uniform_real_distribution<float> t_dist(0.0f, 1.0f);
    std::mt19937 gen(std::random_device{}());
    
    for (auto _ : state) {
        float t = t_dist(gen);
        auto point = curve.evaluate(t);
        benchmark::DoNotOptimize(point);
    }
}

// Batch evaluation performance
static void BM_HermiteBatchEvaluation(benchmark::State& state) {
    const size_t num_points = state.range(0);
    auto data = TestData::random();
    Hermite curve(data.p0, data.p1, data.t0, data.t1, data.tension);
    
    std::vector<float> params;
    params.reserve(num_points);
    std::uniform_real_distribution<float> t_dist(0.0f, 1.0f);
    std::mt19937 gen(std::random_device{}());
    
    for (size_t i = 0; i < num_points; ++i) {
        params.push_back(t_dist(gen));
    }
    
    for (auto _ : state) {
        auto points = curve.evaluateMultiple(params);
        benchmark::DoNotOptimize(points);
    }
    
    state.SetComplexityN(num_points);
}

// SIMD vs Scalar comparison
static void BM_HermiteEvaluationMethods(benchmark::State& state) {
    auto data = TestData::random();
    Hermite curve(data.p0, data.p1, data.t0, data.t1, data.tension);
    std::uniform_real_distribution<float> t_dist(0.0f, 1.0f);
    std::mt19937 gen(std::random_device{}());
    
    const size_t batch_size = state.range(0);
    std::vector<float> params;
    params.reserve(batch_size);
    
    for (size_t i = 0; i < batch_size; ++i) {
        params.push_back(t_dist(gen));
    }
    
    for (auto _ : state) {
        auto points = curve.evaluateMultiple(params);
        benchmark::DoNotOptimize(points);
    }
    
    state.SetComplexityN(batch_size);
}

// Memory behavior under varying tension
static void BM_HermiteTensionBehavior(benchmark::State& state) {
    auto data = TestData::random();
    Hermite curve(data.p0, data.p1, data.t0, data.t1, 1.0f);
    const float tension = static_cast<float>(state.range(0)) / 100.0f;
    
    for (auto _ : state) {
        state.PauseTiming();
        curve.setTension(tension);
        state.ResumeTiming();
        
        auto point = curve.evaluate(0.5f);
        benchmark::DoNotOptimize(point);
    }
}

// Derivative computation performance
static void BM_HermiteDerivative(benchmark::State& state) {
    auto data = TestData::random();
    Hermite curve(data.p0, data.p1, data.t0, data.t1, data.tension);
    std::uniform_real_distribution<float> t_dist(0.0f, 1.0f);
    std::mt19937 gen(std::random_device{}());
    
    for (auto _ : state) {
        auto deriv = curve.derivative();
        float t = t_dist(gen);
        auto point = deriv.evaluate(t);
        benchmark::DoNotOptimize(point);
    }
}

// Cache performance test
static void BM_HermiteCachePerformance(benchmark::State& state) {
    const size_t num_curves = state.range(0);
    std::vector<Hermite> curves;
    curves.reserve(num_curves);
    
    for (size_t i = 0; i < num_curves; ++i) {
        auto data = TestData::random();
        curves.emplace_back(data.p0, data.p1, data.t0, data.t1, data.tension);
    }
    
    std::uniform_real_distribution<float> t_dist(0.0f, 1.0f);
    std::uniform_int_distribution<size_t> curve_dist(0, num_curves - 1);
    std::mt19937 gen(std::random_device{}());
    
    for (auto _ : state) {
        size_t idx = curve_dist(gen);
        float t = t_dist(gen);
        auto point = curves[idx].evaluate(t);
        benchmark::DoNotOptimize(point);
    }
    
    state.SetComplexityN(num_curves);
}

// Register benchmarks
BENCHMARK(BM_HermiteConstruction);

BENCHMARK(BM_HermiteEvaluation);

BENCHMARK(BM_HermiteBatchEvaluation)
    ->RangeMultiplier(4)
    ->Range(4, 1024)
    ->Complexity();

BENCHMARK(BM_HermiteEvaluationMethods)
    ->RangeMultiplier(4)
    ->Range(4, 1024)
    ->Complexity();

BENCHMARK(BM_HermiteTensionBehavior)
    ->DenseRange(10, 500, 50);  // Test tensions from 0.1 to 5.0

BENCHMARK(BM_HermiteDerivative);

BENCHMARK(BM_HermiteCachePerformance)
    ->RangeMultiplier(4)
    ->Range(4, 1024)
    ->Complexity();

} // namespace tests
} // namespace math
} // namespace pynovage