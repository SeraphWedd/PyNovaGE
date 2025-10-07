#include "../../include/geometry/bspline.hpp"
#include <benchmark/benchmark.h>
#include <random>
#include <vector>

namespace pynovage {
namespace math {
namespace tests {

// Helper function to create random control points
std::vector<Vector3> createRandomControlPoints(size_t count) {
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

// Helper function to create random parameters
std::vector<float> createRandomParameters(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::vector<float> params;
    params.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        params.push_back(dist(gen));
    }
    return params;
}

// Benchmark B-spline construction
static void BM_BSplineConstruction(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const int degree = state.range(1);
    auto points = createRandomControlPoints(numPoints);

    for (auto _ : state) {
        BSpline spline(points, degree);
        benchmark::DoNotOptimize(spline);
    }

    state.SetComplexityN(numPoints);
}

// Benchmark single point evaluation
static void BM_BSplineEvaluate(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const int degree = state.range(1);
    auto points = createRandomControlPoints(numPoints);
    BSpline spline(points, degree);
    float t = 0.5f;

    for (auto _ : state) {
        Vector3 result = spline.evaluate(t);
        benchmark::DoNotOptimize(result);
    }

    state.SetComplexityN(numPoints);
}

// Benchmark multiple point evaluation
static void BM_BSplineEvaluateMultiple(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const size_t numEvals = state.range(1);
    const int degree = 3;  // Fixed cubic degree
    
    auto points = createRandomControlPoints(numPoints);
    auto params = createRandomParameters(numEvals);
    BSpline spline(points, degree);

    for (auto _ : state) {
        auto result = spline.evaluateMultiple(params);
        benchmark::DoNotOptimize(result);
    }

    state.SetComplexityN(numPoints * numEvals);
}

// Benchmark knot insertion
static void BM_BSplineKnotInsertion(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const int degree = state.range(1);
    auto points = createRandomControlPoints(numPoints);
    BSpline spline(points, degree);
    float t = 0.5f;

    for (auto _ : state) {
        BSpline tempSpline = spline;  // Create copy for each iteration
        benchmark::DoNotOptimize(tempSpline.insertKnot(t));
    }

    state.SetComplexityN(numPoints);
}

// Benchmark degree elevation
static void BM_BSplineDegreeElevation(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const int degree = state.range(1);
    auto points = createRandomControlPoints(numPoints);
    BSpline spline(points, degree);

    for (auto _ : state) {
        BSpline tempSpline = spline;  // Create copy for each iteration
        benchmark::DoNotOptimize(tempSpline.elevateDegree());
    }

    state.SetComplexityN(numPoints);
}

// Benchmark derivative computation
static void BM_BSplineDerivative(benchmark::State& state) {
    const size_t numPoints = state.range(0);
    const int degree = state.range(1);
    auto points = createRandomControlPoints(numPoints);
    BSpline spline(points, degree);

    for (auto _ : state) {
        auto derivative = spline.derivative();
        benchmark::DoNotOptimize(derivative);
    }

    state.SetComplexityN(numPoints);
}

// Register benchmarks with various input sizes and curve degrees
BENCHMARK(BM_BSplineConstruction)
    ->Args({8, 2})    // Small curve, quadratic
    ->Args({16, 3})   // Medium curve, cubic
    ->Args({32, 4})   // Large curve, quartic
    ->Args({64, 5})   // Very large curve, quintic
    ->Complexity();

BENCHMARK(BM_BSplineEvaluate)
    ->Args({8, 2})
    ->Args({16, 3})
    ->Args({32, 4})
    ->Args({64, 5})
    ->Complexity();

BENCHMARK(BM_BSplineEvaluateMultiple)
    ->Args({8, 100})    // Small curve, many evaluations
    ->Args({16, 1000})  // Medium curve, more evaluations
    ->Args({32, 500})   // Large curve, moderate evaluations
    ->Args({64, 250})   // Very large curve, fewer evaluations
    ->Complexity();

BENCHMARK(BM_BSplineKnotInsertion)
    ->Args({8, 2})
    ->Args({16, 3})
    ->Args({32, 4})
    ->Args({64, 5})
    ->Complexity();

BENCHMARK(BM_BSplineDegreeElevation)
    ->Args({8, 2})
    ->Args({16, 3})
    ->Args({32, 4})
    ->Args({64, 5})
    ->Complexity();

BENCHMARK(BM_BSplineDerivative)
    ->Args({8, 2})
    ->Args({16, 3})
    ->Args({32, 4})
    ->Args({64, 5})
    ->Complexity();

} // namespace tests
} // namespace math
} // namespace pynovage
