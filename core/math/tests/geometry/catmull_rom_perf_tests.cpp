#include "../../include/geometry/catmull_rom.hpp"
#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include <memory>

namespace pynovage {
namespace math {
namespace tests {

// Helper to create random points
struct TestData {
    std::vector<Vector3> points;
    CatmullRom::Parameterization param;
    float tension;

    static TestData random(size_t point_count) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(-10.0f, 10.0f);
        std::uniform_int_distribution<int> param_dist(0, 2);
        std::uniform_real_distribution<float> tension_dist(0.1f, 5.0f);

        std::vector<Vector3> points;
        points.reserve(point_count);
        for (size_t i = 0; i < point_count; ++i) {
            points.emplace_back(dist(gen), dist(gen), dist(gen));
        }

        return {
            std::move(points),
            static_cast<CatmullRom::Parameterization>(param_dist(gen)),
            tension_dist(gen)
        };
    }
};

// Basic construction and parameterization
static void BM_CatmullRomConstruction(benchmark::State& state) {
    auto data = TestData::random(state.range(0));
    
    for (auto _ : state) {
        CatmullRom spline(data.points, data.param, data.tension);
        benchmark::DoNotOptimize(spline);
    }
    
    state.SetComplexityN(state.range(0));
}

// Evaluation performance
static void BM_CatmullRomEvaluation(benchmark::State& state) {
    auto data = TestData::random(state.range(0));
    CatmullRom spline(data.points, data.param, data.tension);
    
    std::uniform_real_distribution<float> t_dist(0.0f, 1.0f);
    std::mt19937 gen(std::random_device{}());
    
    for (auto _ : state) {
        float t = t_dist(gen);
        auto point = spline.evaluate(t);
        benchmark::DoNotOptimize(point);
    }
    
    state.SetComplexityN(state.range(0));
}

// Batch evaluation performance
static void BM_CatmullRomBatchEvaluation(benchmark::State& state) {
    const size_t num_points = 32;  // Fixed number of control points
    const size_t num_evals = state.range(0);  // Varying number of evaluations
    
    auto data = TestData::random(num_points);
    CatmullRom spline(data.points, data.param, data.tension);
    
    std::vector<float> params;
    params.reserve(num_evals);
    std::uniform_real_distribution<float> t_dist(0.0f, 1.0f);
    std::mt19937 gen(std::random_device{}());
    
    for (size_t i = 0; i < num_evals; ++i) {
        params.push_back(t_dist(gen));
    }
    
    for (auto _ : state) {
        auto points = spline.evaluateMultiple(params);
        benchmark::DoNotOptimize(points);
    }
    
    state.SetComplexityN(num_evals);
}

// Parameter computation performance
static void BM_CatmullRomParameterization(benchmark::State& state) {
    auto data = TestData::random(state.range(0));
    CatmullRom spline(data.points, CatmullRom::Parameterization::Uniform);
    
    for (auto _ : state) {
        state.PauseTiming();
        spline = CatmullRom(data.points, CatmullRom::Parameterization::Centripetal);
        state.ResumeTiming();
        
        auto point = spline.evaluate(0.5f);
        benchmark::DoNotOptimize(point);
    }
    
    state.SetComplexityN(state.range(0));
}

// Point manipulation performance
static void BM_CatmullRomPointManipulation(benchmark::State& state) {
    auto data = TestData::random(state.range(0));
    CatmullRom spline(data.points);
    
    std::uniform_real_distribution<float> coord_dist(-10.0f, 10.0f);
    std::uniform_int_distribution<size_t> index_dist(0, data.points.size() - 1);
    std::mt19937 gen(std::random_device{}());
    
    for (auto _ : state) {
        state.PauseTiming();
        Vector3 new_point(coord_dist(gen), coord_dist(gen), coord_dist(gen));
        size_t index = index_dist(gen);
        state.ResumeTiming();
        
        spline.insertPoint(new_point, index);
        spline.removePoint(index);
        benchmark::DoNotOptimize(spline);
    }
    
    state.SetComplexityN(state.range(0));
}

// Memory behavior with large point counts
static void BM_CatmullRomMemoryBehavior(benchmark::State& state) {
    const size_t num_points = state.range(0);
    auto data = TestData::random(num_points);
    CatmullRom spline(data.points);
    
    std::uniform_real_distribution<float> t_dist(0.0f, 1.0f);
    std::mt19937 gen(std::random_device{}());
    
    for (auto _ : state) {
        float t = t_dist(gen);
        auto point = spline.evaluate(t);
        benchmark::DoNotOptimize(point);
    }
    
    state.SetComplexityN(num_points);
}

// Performance across different parameterizations
static void BM_CatmullRomParameterizationTypes(benchmark::State& state) {
    auto data = TestData::random(32);  // Fixed size for consistent comparison
    CatmullRom spline(data.points, 
                      static_cast<CatmullRom::Parameterization>(state.range(0)));
    
    std::uniform_real_distribution<float> t_dist(0.0f, 1.0f);
    std::mt19937 gen(std::random_device{}());
    
    for (auto _ : state) {
        float t = t_dist(gen);
        auto point = spline.evaluate(t);
        benchmark::DoNotOptimize(point);
    }
}

// Register benchmarks
BENCHMARK(BM_CatmullRomConstruction)
    ->RangeMultiplier(2)
    ->Range(4, 1024)
    ->Complexity();

BENCHMARK(BM_CatmullRomEvaluation)
    ->RangeMultiplier(2)
    ->Range(4, 1024)
    ->Complexity();

BENCHMARK(BM_CatmullRomBatchEvaluation)
    ->RangeMultiplier(2)
    ->Range(4, 1024)
    ->Complexity();

BENCHMARK(BM_CatmullRomParameterization)
    ->RangeMultiplier(2)
    ->Range(4, 1024)
    ->Complexity();

BENCHMARK(BM_CatmullRomPointManipulation)
    ->RangeMultiplier(2)
    ->Range(4, 1024)
    ->Complexity();

BENCHMARK(BM_CatmullRomMemoryBehavior)
    ->RangeMultiplier(2)
    ->Range(4, 1024)
    ->Complexity();

BENCHMARK(BM_CatmullRomParameterizationTypes)
    ->DenseRange(0, 2);  // Test all parameterization types

} // namespace tests
} // namespace math
} // namespace pynovage