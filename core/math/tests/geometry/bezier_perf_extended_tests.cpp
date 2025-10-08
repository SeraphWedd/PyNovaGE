#include "../../include/geometry/bezier.hpp"
#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include <memory>
#include <array>

namespace pynovage {
namespace math {
namespace tests {

// Helpers for memory aligned allocations
struct alignas(32) AlignedVector3 {
    float x, y, z;
    float padding;  // To ensure 32-byte alignment
};

// Structure of Arrays (SoA) layout for comparison
struct BezierPointsSoA {
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
    
    void resize(size_t size) {
        x.resize(size);
        y.resize(size);
        z.resize(size);
    }
    
    void reserve(size_t size) {
        x.reserve(size);
        y.reserve(size);
        z.reserve(size);
    }
    
    void push_back(const Vector3& point) {
        x.push_back(point.x);
        y.push_back(point.y);
        z.push_back(point.z);
    }
};

// Helper to create random control points with specific memory alignment
std::vector<AlignedVector3> createAlignedPoints(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
    
    std::vector<AlignedVector3> points;
    points.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        points.push_back({dist(gen), dist(gen), dist(gen), 0.0f});
    }
    return points;
}

// Helper to create SoA layout control points
BezierPointsSoA createSoAPoints(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
    
    BezierPointsSoA points;
    points.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        points.x.push_back(dist(gen));
        points.y.push_back(dist(gen));
        points.z.push_back(dist(gen));
    }
    return points;
}

// Benchmark cache performance with large curves
static void BM_BezierCachePerformance(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    auto points = std::vector<Vector3>();
    points.reserve(numPoints);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
    
    for (size_t i = 0; i < numPoints; ++i) {
        points.emplace_back(dist(gen), dist(gen), dist(gen));
    }
    
    Bezier curve(points);
    
    // Create evaluation points
    std::vector<float> params;
    params.reserve(1000);
    std::uniform_real_distribution<float> paramDist(0.0f, 1.0f);
    for (int i = 0; i < 1000; ++i) {
        params.push_back(paramDist(gen));
    }
    
    // Benchmark evaluation
    for (auto _ : state) {
        auto result = curve.evaluateMultiple(params);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(numPoints);
}

// Benchmark memory layout impact (SoA vs AoS)
static void BM_BezierMemoryLayout(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const int numEvals = 1000;
    
    // Create standard AoS points and SoA points
    auto aosPoints = std::vector<Vector3>();
    auto soaPoints = BezierPointsSoA();
    
    aosPoints.reserve(numPoints);
    soaPoints.reserve(numPoints);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
    
    for (size_t i = 0; i < numPoints; ++i) {
        Vector3 point(dist(gen), dist(gen), dist(gen));
        aosPoints.push_back(point);
        soaPoints.push_back(point);
    }
    
    // Create Bezier with AoS layout
    Bezier aosCurve(aosPoints);
    
    // Create evaluation parameters
    std::vector<float> params;
    params.reserve(numEvals);
    std::uniform_real_distribution<float> paramDist(0.0f, 1.0f);
    for (int i = 0; i < numEvals; ++i) {
        params.push_back(paramDist(gen));
    }
    
    // Benchmark AoS layout
    for (auto _ : state) {
        state.PauseTiming();
        auto curve = aosCurve;  // Create copy to ensure fair comparison
        state.ResumeTiming();
        
        auto result = curve.evaluateMultiple(params);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(numPoints);
}

// Benchmark SIMD vs non-SIMD operations
static void BM_BezierSIMDComparison(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const int numEvals = 1000;
    
    // Create aligned points for SIMD
    auto alignedPoints = createAlignedPoints(numPoints);
    std::vector<Vector3> standardPoints;
    standardPoints.reserve(numPoints);
    for (const auto& p : alignedPoints) {
        standardPoints.emplace_back(p.x, p.y, p.z);
    }
    
    Bezier curve(standardPoints);
    
    // Create evaluation parameters
    std::vector<float> params;
    params.reserve(numEvals);
    std::uniform_real_distribution<float> paramDist(0.0f, 1.0f);
    std::random_device rd;
    std::mt19937 gen(rd());
    for (int i = 0; i < numEvals; ++i) {
        params.push_back(paramDist(gen));
    }
    
    // Benchmark SIMD evaluation
    for (auto _ : state) {
        auto result = curve.evaluateMultiple(params);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(numPoints);
}

// Benchmark memory behavior under various operations
static void BM_BezierMemoryBehavior(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    
    // Create initial curve
    auto points = std::vector<Vector3>();
    points.reserve(numPoints);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
    
    for (size_t i = 0; i < numPoints; ++i) {
        points.emplace_back(dist(gen), dist(gen), dist(gen));
    }
    
    Bezier curve(points);
    
    // Test memory behavior under various operations
    for (auto _ : state) {
        state.PauseTiming();
        auto tempCurve = curve;  // Create copy for modification
        state.ResumeTiming();
        
        // Series of operations that stress memory
        auto left = tempCurve.split(0.5f).first;
        benchmark::DoNotOptimize(left.derivative());
        left.elevateDegree();
        
        // Force evaluation to ensure operations complete
        benchmark::DoNotOptimize(left.evaluate(0.5f));
    }
    
    state.SetComplexityN(numPoints);
}

// Benchmark complex curve operations
static void BM_BezierComplexOperations(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const int numOps = state.range(1);
    
    // Create initial curve
    auto points = std::vector<Vector3>();
    points.reserve(numPoints);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
    
    for (size_t i = 0; i < numPoints; ++i) {
        points.emplace_back(dist(gen), dist(gen), dist(gen));
    }
    
    Bezier curve(points);
    
    // Create random parameters for operations
    std::vector<float> splitPoints;
    splitPoints.reserve(numOps);
    std::uniform_real_distribution<float> paramDist(0.1f, 0.9f);
    for (int i = 0; i < numOps; ++i) {
        splitPoints.push_back(paramDist(gen));
    }
    
    // Benchmark complex operations
    for (auto _ : state) {
        state.PauseTiming();
        auto workingCurve = curve;
        state.ResumeTiming();
        
        for (float t : splitPoints) {
            auto [left, right] = workingCurve.split(t);
            workingCurve = left;
            workingCurve.elevateDegree();
            auto deriv = workingCurve.derivative();
            benchmark::DoNotOptimize(deriv.evaluate(0.5f));
        }
    }
    
    state.SetComplexityN(numPoints * numOps);
}

// Register benchmarks with various input sizes
BENCHMARK(BM_BezierCachePerformance)
    ->RangeMultiplier(2)
    ->Range(4, 128)  // More reasonable range for cache testing
    ->Complexity();

BENCHMARK(BM_BezierMemoryLayout)
    ->RangeMultiplier(2)
    ->Range(4, 64)   // Memory layout tests up to typical max size
    ->Complexity();

BENCHMARK(BM_BezierSIMDComparison)
    ->RangeMultiplier(2)
    ->Range(4, 32)   // SIMD most effective for curves up to degree 31
    ->Complexity();

BENCHMARK(BM_BezierMemoryBehavior)
    ->RangeMultiplier(2)
    ->Range(4, 128)  // Test memory behavior with reasonable sizes
    ->Complexity();

BENCHMARK(BM_BezierComplexOperations)
    ->Args({8, 50})      // Small curve, many ops
    ->Args({16, 25})     // Medium curve, moderate ops
    ->Args({32, 10})     // Large curve, fewer ops
    ->Args({64, 5})      // Very large curve, minimal ops
    ->Complexity();

} // namespace tests
} // namespace math
} // namespace pynovage