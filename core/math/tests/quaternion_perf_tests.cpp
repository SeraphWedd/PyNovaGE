#include "quaternion.hpp"
#include "matrix3.hpp"
#include "vector3.hpp"
#include "math_constants.hpp"
#include <benchmark/benchmark.h>

using namespace pynovage::math;
using namespace pynovage::math::constants;

// Benchmark creation of rotation representations
static void BM_QuaternionCreation(benchmark::State& state) {
    Vector3 axis(1.0f, 0.0f, 0.0f);
    float angle = half_pi;
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(Quaternion(axis, angle));
    }
}
BENCHMARK(BM_QuaternionCreation);

static void BM_Matrix3Creation(benchmark::State& state) {
    Vector3 axis(1.0f, 0.0f, 0.0f);
    float angle = half_pi;
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(Matrix3::fromAxisAngle(axis, angle));
    }
}
BENCHMARK(BM_Matrix3Creation);

// Benchmark rotation composition
static void BM_QuaternionComposition(benchmark::State& state) {
    Quaternion q1(Vector3(1.0f, 0.0f, 0.0f), half_pi);
    Quaternion q2(Vector3(0.0f, 1.0f, 0.0f), quarter_pi);
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(q1 * q2);
    }
}
BENCHMARK(BM_QuaternionComposition);

static void BM_Matrix3Composition(benchmark::State& state) {
    Matrix3 m1 = Matrix3::fromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), half_pi);
    Matrix3 m2 = Matrix3::fromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), quarter_pi);
    Matrix3 m3;
    for (auto _ : state) {
        benchmark::DoNotOptimize(m1 * m2);
    }
}
BENCHMARK(BM_Matrix3Composition);

// Benchmark vector rotation
static void BM_QuaternionVectorRotation(benchmark::State& state) {
    Quaternion q(Vector3(1.0f, 0.0f, 0.0f), half_pi);
    Vector3 v(1.0f, 1.0f, 1.0f);
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(q.RotateVector(v));
    }
}
BENCHMARK(BM_QuaternionVectorRotation);

static void BM_Matrix3VectorRotation(benchmark::State& state) {
    Matrix3 m = Matrix3::fromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), half_pi);
    Vector3 v(1.0f, 1.0f, 1.0f);
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(m * v);
    }
}
BENCHMARK(BM_Matrix3VectorRotation);

// Benchmark multiple sequential rotations
static void BM_QuaternionMultipleRotations(benchmark::State& state) {
    Quaternion qx(Vector3(1.0f, 0.0f, 0.0f), quarter_pi);
    Quaternion qy(Vector3(0.0f, 1.0f, 0.0f), half_pi);
    Quaternion qz(Vector3(0.0f, 0.0f, 1.0f), quarter_pi);
    Vector3 v(1.0f, 1.0f, 1.0f);
    
    for (auto _ : state) {
        Quaternion combined = qz * qy * qx;
        benchmark::DoNotOptimize(combined.RotateVector(v));
    }
}
BENCHMARK(BM_QuaternionMultipleRotations);

static void BM_Matrix3MultipleRotations(benchmark::State& state) {
    Matrix3 mx = Matrix3::fromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), quarter_pi);
    Matrix3 my = Matrix3::fromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), half_pi);
    Matrix3 mz = Matrix3::fromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), quarter_pi);
    Vector3 v(1.0f, 1.0f, 1.0f);
    
    for (auto _ : state) {
        Matrix3 combined = mz * my * mx;
        benchmark::DoNotOptimize(combined * v);
    }
}
BENCHMARK(BM_Matrix3MultipleRotations);

// Benchmark memory usage
// Note: This is more to demonstrate the size difference
static void BM_QuaternionMemoryUsage(benchmark::State& state) {
    for (auto _ : state) {
        // Quaternion uses 4 floats = 16 bytes
        Quaternion q;
        benchmark::DoNotOptimize(q);
    }
}
BENCHMARK(BM_QuaternionMemoryUsage);

static void BM_Matrix3MemoryUsage(benchmark::State& state) {
    for (auto _ : state) {
        // Matrix3 uses 9 floats = 36 bytes
        Matrix3 m;
        benchmark::DoNotOptimize(m);
    }
}
BENCHMARK(BM_Matrix3MemoryUsage);

// Benchmark interpolation
static void BM_QuaternionSlerp(benchmark::State& state) {
    Quaternion start(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
    Quaternion end(Vector3(1.0f, 0.0f, 0.0f), half_pi);
    float t = 0.5f;
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(Quaternion::Slerp(start, end, t));
    }
}
BENCHMARK(BM_QuaternionSlerp);

static void BM_QuaternionLerp(benchmark::State& state) {
    Quaternion start(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
    Quaternion end(Vector3(1.0f, 0.0f, 0.0f), half_pi);
    float t = 0.5f;
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(Quaternion::Lerp(start, end, t));
    }
}
BENCHMARK(BM_QuaternionLerp);

// Benchmark normalization/orthogonalization
static void BM_QuaternionNormalization(benchmark::State& state) {
    Quaternion q(1.5f, 2.5f, 3.5f, 4.5f);
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(q.Normalized());
    }
}
BENCHMARK(BM_QuaternionNormalization);

static void BM_Matrix3Orthogonalization(benchmark::State& state) {
    // Create a slightly non-orthogonal matrix
    Matrix3 m(
        1.1f, 0.1f, 0.2f,
        0.1f, 1.2f, 0.1f,
        0.2f, 0.1f, 1.1f
    );
    
    for (auto _ : state) {
        // Gram-Schmidt orthogonalization
        Vector3 x(m.m00, m.m01, m.m02);
        Vector3 y(m.m10, m.m11, m.m12);
        Vector3 z(m.m20, m.m21, m.m22);
        
        x = x.normalized();
        y = (y - x * x.dot(y)).normalized();
        z = (z - x * x.dot(z) - y * y.dot(z)).normalized();
        
        benchmark::DoNotOptimize(Matrix3(
            x.x, x.y, x.z,
            y.x, y.y, y.z,
            z.x, z.y, z.z
        ));
    }
}
BENCHMARK(BM_Matrix3Orthogonalization);