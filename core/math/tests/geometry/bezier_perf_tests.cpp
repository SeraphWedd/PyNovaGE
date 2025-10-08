#include "../../include/geometry/bezier.hpp"
#include <benchmark/benchmark.h>
#include <random>

namespace pynovage {
namespace math {
namespace tests {

// Helper to create random control points
std::vector<Vector3> createRandomPoints(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
    
    std::vector<Vector3> points;
    points.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        points.emplace_back(dist(gen), dist(gen), dist(gen));
    }
    return points;
}

// Benchmark curve construction
static void BM_BezierConstruction(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    auto points = createRandomPoints(numPoints);
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(Bezier(points));
    }
    
    state.SetComplexityN(numPoints);
}

// Benchmark single point evaluation
static void BM_BezierEvaluate(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    auto points = createRandomPoints(numPoints);
    Bezier curve(points);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    for (auto _ : state) {
        float t = dist(gen);
        benchmark::DoNotOptimize(curve.evaluate(t));
    }
    
    state.SetComplexityN(numPoints);
}

// Benchmark batch evaluation
static void BM_BezierEvaluateMultiple(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const size_t numEvals = state.range(1);
    
    auto points = createRandomPoints(numPoints);
    Bezier curve(points);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    std::vector<float> params;
    params.reserve(numEvals);
    for (size_t i = 0; i < numEvals; ++i) {
        params.push_back(dist(gen));
    }
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(curve.evaluateMultiple(params));
    }
    
    state.SetComplexityN(numPoints * numEvals);
}

// Benchmark derivative computation
static void BM_BezierDerivative(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    auto points = createRandomPoints(numPoints);
    Bezier curve(points);
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(curve.derivative());
    }
    
    state.SetComplexityN(numPoints);
}

// Compare De Casteljau vs direct evaluation
static void BM_BezierEvaluationMethods(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    auto points = createRandomPoints(numPoints);
    Bezier curve(points);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float t = dist(gen);
    
    if (state.thread_index() == 0) {
        state.SetLabel(numPoints <= 4 ? "De Casteljau" : "SIMD/Direct");
    }
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(curve.evaluate(t));
    }
    
    state.SetComplexityN(numPoints);
}

// Register benchmarks with various input sizes
BENCHMARK(BM_BezierConstruction)
    ->RangeMultiplier(2)
    ->Range(2, 32)
    ->Complexity();

BENCHMARK(BM_BezierEvaluate)
    ->RangeMultiplier(2)
    ->Range(2, 32)
    ->Complexity();

BENCHMARK(BM_BezierEvaluateMultiple)
    ->Args({2, 100})   // Small curve, many points
    ->Args({8, 1000})  // Medium curve, lots of points
    ->Args({16, 500})  // Larger curve, moderate points
    ->Args({32, 250})  // Large curve, fewer points
    ->Complexity();

BENCHMARK(BM_BezierDerivative)
    ->RangeMultiplier(2)
    ->Range(2, 32)
    ->Complexity();

BENCHMARK(BM_BezierEvaluationMethods)
    ->RangeMultiplier(2)
    ->Range(2, 32)
    ->Complexity();

} // namespace tests
} // namespace math
} // namespace pynovage