#include "../../include/geometry/path.hpp"
#include <benchmark/benchmark.h>

namespace pynovage {
namespace math {
namespace tests {

namespace {
// Helper to create a path with N points in a circle
Path createCircularPath(size_t numPoints, bool closed = false) {
    Path path;
    const float angleStep = 2.0f * 3.14159f / static_cast<float>(numPoints);
    
    for (size_t i = 0; i < numPoints; ++i) {
        float angle = angleStep * static_cast<float>(i);
        path.addPoint(Vector3(std::cos(angle), std::sin(angle), 0.0f));
    }
    
    path.setClosed(closed);
    return path;
}

// Helper to create a path with N points in a wave pattern
Path createWavePath(size_t numPoints, float amplitude = 1.0f) {
    Path path;
    const float step = 4.0f * 3.14159f / static_cast<float>(numPoints);
    
    for (size_t i = 0; i < numPoints; ++i) {
        float x = step * static_cast<float>(i);
        path.addPoint(Vector3(x, amplitude * std::sin(x), 0.0f));
    }
    
    return path;
}
} // anonymous namespace

// Basic operations
static void BM_PathConstruction(benchmark::State& state) {
    for (auto _ : state) {
        Path path;
        for (int i = 0; i < state.range(0); ++i) {
            float t = static_cast<float>(i) / static_cast<float>(state.range(0));
            path.addPoint(Vector3(t, std::sin(t * 6.28f), 0.0f));
        }
        benchmark::DoNotOptimize(path);
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_PathConstruction)->Range(8, 512);

// Point evaluation
static void BM_PathEvaluation(benchmark::State& state) {
    Path path = createCircularPath(state.range(0));
    std::vector<float> params;
    params.reserve(100);
    for (int i = 0; i < 100; ++i) {
        params.push_back(static_cast<float>(i) / 99.0f);
    }
    
    for (auto _ : state) {
        for (float t : params) {
            Vector3 pos = path.getPosition(t);
            benchmark::DoNotOptimize(pos);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * params.size());
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_PathEvaluation)->Range(8, 512);

// Frame computation
static void BM_PathFrameComputation(benchmark::State& state) {
    Path path = createWavePath(state.range(0));
    std::vector<float> params;
    params.reserve(100);
    for (int i = 0; i < 100; ++i) {
        params.push_back(static_cast<float>(i) / 99.0f);
    }
    
    for (auto _ : state) {
        for (float t : params) {
            auto [pos, tan, norm, binorm] = path.getFrame(t);
            benchmark::DoNotOptimize(pos);
            benchmark::DoNotOptimize(tan);
            benchmark::DoNotOptimize(norm);
            benchmark::DoNotOptimize(binorm);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * params.size());
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_PathFrameComputation)->Range(8, 512);

// Path type switching - individual benchmarks per type
static void BM_PathCatmullRom(benchmark::State& state) {
    Path path = createCircularPath(32);  // Fixed size for type tests
    path.setType(PathType::CatmullRom);
    for (auto _ : state) {
        Vector3 pos = path.getPosition(0.5f);
        benchmark::DoNotOptimize(pos);
    }
}
BENCHMARK(BM_PathCatmullRom);

static void BM_PathBezier(benchmark::State& state) {
    Path path = createCircularPath(32);
    path.setType(PathType::Bezier);
    for (auto _ : state) {
        Vector3 pos = path.getPosition(0.5f);
        benchmark::DoNotOptimize(pos);
    }
}
BENCHMARK(BM_PathBezier);

static void BM_PathBSpline(benchmark::State& state) {
    Path path = createCircularPath(32);
    path.setType(PathType::BSpline);
    for (auto _ : state) {
        Vector3 pos = path.getPosition(0.5f);
        benchmark::DoNotOptimize(pos);
    }
}
BENCHMARK(BM_PathBSpline);

static void BM_PathLinear(benchmark::State& state) {
    Path path = createCircularPath(32);
    path.setType(PathType::Linear);
    for (auto _ : state) {
        Vector3 pos = path.getPosition(0.5f);
        benchmark::DoNotOptimize(pos);
    }
}
BENCHMARK(BM_PathLinear);

// Arc length parameterization
static void BM_PathArcLength(benchmark::State& state) {
    Path path = createWavePath(state.range(0));
    std::vector<float> distances;
    distances.reserve(100);
    
    float length = path.getLength();
    for (int i = 0; i < 100; ++i) {
        distances.push_back(length * static_cast<float>(i) / 99.0f);
    }
    
    for (auto _ : state) {
        for (float d : distances) {
            float t = path.getParameterAtDistance(d);
            benchmark::DoNotOptimize(t);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * distances.size());
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_PathArcLength)->Range(8, 512);

// Test different tension values
static void BM_PathTensionAdjustment(benchmark::State& state) {
    Path path = createCircularPath(state.range(0));
    const std::vector<float> tensions = { 0.5f, 1.0f, 2.0f, 4.0f };
    
    for (auto _ : state) {
        for (float tension : tensions) {
            path.setTension(tension);
            Vector3 pos = path.getPosition(0.5f);
            benchmark::DoNotOptimize(pos);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * tensions.size());
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_PathTensionAdjustment)->Range(8, 512);

// Closed path operations
static void BM_PathClosedEvaluation(benchmark::State& state) {
    Path path = createCircularPath(state.range(0), true);
    std::vector<float> params;
    params.reserve(100);
    for (int i = 0; i < 100; ++i) {
        params.push_back(static_cast<float>(i) / 99.0f);
    }
    
    for (auto _ : state) {
        for (float t : params) {
            Vector3 pos = path.getPosition(t);
            Vector3 tan = path.getTangent(t);
            benchmark::DoNotOptimize(pos);
            benchmark::DoNotOptimize(tan);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * params.size());
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_PathClosedEvaluation)->Range(8, 512);

} // namespace tests
} // namespace math
} // namespace pynovage