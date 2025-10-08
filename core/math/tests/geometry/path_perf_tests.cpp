#include "../../include/geometry/path.hpp"
#include "../../include/geometry/catmull_rom_path.hpp"
#include <benchmark/benchmark.h>

namespace pynovage {
namespace math {
namespace tests {

static void BM_PathConstruction(benchmark::State& state) {
    std::vector<Vector3> points = {
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.0f, 0.0f, 0.0f),
        Vector3(1.0f, 1.0f, 0.0f),
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(0.0f, 0.0f, 0.0f)
    };

    for (auto _ : state) {
        benchmark::DoNotOptimize(
            CatmullRomPath(points, Path::MovementMode::ConstantSpeed)
        );
    }
}
BENCHMARK(BM_PathConstruction);

static void BM_PathEvaluation(benchmark::State& state) {
    std::vector<Vector3> points = {
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.0f, 0.0f, 0.0f),
        Vector3(1.0f, 1.0f, 0.0f),
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(0.0f, 0.0f, 0.0f)
    };
    CatmullRomPath path(points, Path::MovementMode::ConstantSpeed);

    // Sample 100 points along path
    float step = 1.0f / 100.0f;
    float t = 0.0f;

    for (auto _ : state) {
        Path::State state = path.getState(t);
        benchmark::DoNotOptimize(state);
        t = (t + step < 1.0f) ? t + step : 0.0f;
    }
}
BENCHMARK(BM_PathEvaluation);

static void BM_PathConstantSpeed(benchmark::State& state) {
    std::vector<Vector3> points = {
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.0f, 0.0f, 0.0f),
        Vector3(1.0f, 1.0f, 0.0f),
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(0.0f, 0.0f, 0.0f)
    };
    CatmullRomPath path(points, Path::MovementMode::ConstantSpeed);

    Path::State currentState;
    currentState.position = points[0];
    currentState.time = 0.0f;
    currentState.distance = 0.0f;
    currentState.speed = 1.0f;
    float dt = 0.016f; // 60 FPS

    for (auto _ : state) {
        benchmark::DoNotOptimize(
            path.updateConstantSpeed(currentState, dt)
        );
        if (currentState.distance >= path.getLength()) {
            currentState.distance = 0.0f;
        }
    }
}
BENCHMARK(BM_PathConstantSpeed);

static void BM_PathClosestPoint(benchmark::State& state) {
    std::vector<Vector3> points = {
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.0f, 0.0f, 0.0f),
        Vector3(1.0f, 1.0f, 0.0f),
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(0.0f, 0.0f, 0.0f)
    };
    CatmullRomPath path(points, Path::MovementMode::ConstantSpeed);

    // Query points in a grid pattern
    std::vector<Vector3> queries;
    for (float x = -1.0f; x <= 2.0f; x += 0.5f) {
        for (float y = -1.0f; y <= 2.0f; y += 0.5f) {
            queries.emplace_back(x, y, 0.0f);
        }
    }
    size_t queryIndex = 0;

    for (auto _ : state) {
        benchmark::DoNotOptimize(
            path.getClosestPoint(queries[queryIndex])
        );
        queryIndex = (queryIndex + 1) % queries.size();
    }
}
BENCHMARK(BM_PathClosestPoint);

static void BM_PathBlending(benchmark::State& state) {
    std::vector<Vector3> points1 = {
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.0f, 0.0f, 0.0f),
        Vector3(1.0f, 1.0f, 0.0f),
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(0.0f, 0.0f, 0.0f)
    };
    std::vector<Vector3> points2 = {
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(2.0f, 0.0f, 0.0f),
        Vector3(2.0f, 2.0f, 0.0f),
        Vector3(0.0f, 2.0f, 0.0f),
        Vector3(0.0f, 0.0f, 0.0f)
    };

    CatmullRomPath path1(points1, Path::MovementMode::ConstantSpeed);
    CatmullRomPath path2(points2, Path::MovementMode::ConstantSpeed);
    float blendFactor = 0.0f;
    float blendStep = 0.1f;

    for (auto _ : state) {
        benchmark::DoNotOptimize(
            path1.blend(path2, blendFactor)
        );
        blendFactor += blendStep;
        if (blendFactor >= 1.0f) {
            blendFactor = 0.0f;
        }
    }
}
BENCHMARK(BM_PathBlending);

} // namespace tests
} // namespace math
} // namespace pynovage