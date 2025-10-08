#include "../../include/geometry/bspline.hpp"
#include "../../include/simd_utils.hpp"
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
struct BSplinePointsSoA {
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
std::vector<AlignedVector3> createAlignedControlPoints(size_t count) {
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
BSplinePointsSoA createSoAControlPoints(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

    BSplinePointsSoA points;
    points.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        points.x.push_back(dist(gen));
        points.y.push_back(dist(gen));
        points.z.push_back(dist(gen));
    }
    return points;
}

// Benchmark cache performance with large curves
static void BM_BSplineCachePerformance(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const int degree = 3;  // Fixed cubic degree
    
    // Create control points
    auto points = std::vector<Vector3>();
    points.reserve(numPoints);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
    
    for (size_t i = 0; i < numPoints; ++i) {
        points.emplace_back(dist(gen), dist(gen), dist(gen));
    }
    
    BSpline spline(points, degree);
    
    // Create evaluation points
    std::vector<float> params;
    params.reserve(1000);
    std::uniform_real_distribution<float> paramDist(0.0f, 1.0f);
    for (int i = 0; i < 1000; ++i) {
        params.push_back(paramDist(gen));
    }
    
    // Benchmark evaluation
    for (auto _ : state) {
        auto result = spline.evaluateMultiple(params);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetComplexityN(numPoints);
}

// Benchmark memory layout impact (SoA vs AoS)
static void BM_BSplineMemoryLayout(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const int numEvals = 1000;
    const int degree = 3;

    // Create standard AoS points and SoA points
    auto aosPoints = std::vector<Vector3>();
    auto soaPoints = BSplinePointsSoA();
    
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
    
    // Create BSpline with AoS layout
    BSpline aosSpline(aosPoints, degree);
    
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
        auto spline = aosSpline;  // Create copy to ensure fair comparison
        state.ResumeTiming();
        
        for (float t : params) {
            Vector3 result = spline.evaluate(t);
            benchmark::DoNotOptimize(result);
        }
    }
    
    state.SetComplexityN(numPoints);
}

// Benchmark SIMD vs non-SIMD operations
static void BM_BSplineSIMDComparison(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const int numEvals = 1000;
    const int degree = 3;
    
    // Create aligned points for SIMD
    auto alignedPoints = createAlignedControlPoints(numPoints);
    std::vector<Vector3> standardPoints;
    standardPoints.reserve(numPoints);
    for (const auto& p : alignedPoints) {
        standardPoints.emplace_back(p.x, p.y, p.z);
    }
    
    BSpline spline(standardPoints, degree);
    
    // Create evaluation parameters
    std::vector<float> params;
    params.reserve(numEvals);
    std::uniform_real_distribution<float> paramDist(0.0f, 1.0f);
    std::random_device rd;
    std::mt19937 gen(rd());
    for (int i = 0; i < numEvals; ++i) {
        params.push_back(paramDist(gen));
    }
    
    // Warmup via public API to ensure any lazy initialization is done
    if (state.thread_index() == 0) {
        for (float t : params) {
            benchmark::DoNotOptimize(spline.evaluate(t));
        }
    }
    
    // Benchmark evaluation via public API only
    for (auto _ : state) {
        for (float t : params) {
            Vector3 result = spline.evaluate(t);
            benchmark::DoNotOptimize(result);
        }
    }
    
    state.SetComplexityN(numPoints * params.size());
}

// Benchmark memory usage for different operations
static void BM_BSplineMemoryBehavior(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const int degree = 3;
    
    // Create initial spline
    auto points = std::vector<Vector3>();
    points.reserve(numPoints);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
    
    for (size_t i = 0; i < numPoints; ++i) {
        points.emplace_back(dist(gen), dist(gen), dist(gen));
    }
    
    BSpline spline(points, degree);
    
    // Test memory behavior under various operations
    for (auto _ : state) {
        state.PauseTiming();
        auto tempSpline = spline;  // Create copy for modification
        state.ResumeTiming();
        
        // Series of operations that stress memory
        tempSpline.insertKnot(0.5f);
        benchmark::DoNotOptimize(tempSpline.derivative());
        tempSpline.elevateDegree();
        
        // Force evaluation to ensure operations complete
        benchmark::DoNotOptimize(tempSpline.evaluate(0.5f));
    }
    
    state.SetComplexityN(numPoints);
}

// Register benchmarks with various input sizes
BENCHMARK(BM_BSplineCachePerformance)
    ->RangeMultiplier(4)
    ->Range(64, 16384)  // Test with larger curves
    ->Complexity();

BENCHMARK(BM_BSplineMemoryLayout)
    ->RangeMultiplier(4)
    ->Range(64, 16384)
    ->Complexity();

BENCHMARK(BM_BSplineSIMDComparison)
    ->RangeMultiplier(4)
    ->Range(64, 16384)
    ->Complexity();

BENCHMARK(BM_BSplineMemoryBehavior)
    ->RangeMultiplier(4)
    ->Range(64, 16384)
    ->Complexity();

} // namespace tests
} // namespace math
} // namespace pynovage