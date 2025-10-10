#include <benchmark/benchmark.h>
#include "physics/physics.hpp"
#include <random>
#include <vector>

using namespace PyNovaGE::Physics;

//------------------------------------------------------------------------------
// Random data generation utilities
//------------------------------------------------------------------------------

class RandomDataGenerator {
public:
    RandomDataGenerator() : rng_(std::random_device{}()) {}
    
    Vector2<float> randomPosition() {
        return Vector2<float>(pos_dist_(rng_), pos_dist_(rng_));
    }
    
    Vector2<float> randomVelocity() {
        return Vector2<float>(vel_dist_(rng_), vel_dist_(rng_));
    }
    
    float randomRadius() {
        return radius_dist_(rng_);
    }
    
    Vector2<float> randomSize() {
        return Vector2<float>(size_dist_(rng_), size_dist_(rng_));
    }

private:
    std::mt19937 rng_;
    std::uniform_real_distribution<float> pos_dist_{-50.0f, 50.0f};
    std::uniform_real_distribution<float> vel_dist_{-10.0f, 10.0f};
    std::uniform_real_distribution<float> radius_dist_{0.5f, 3.0f};
    std::uniform_real_distribution<float> size_dist_{1.0f, 4.0f};
};

static RandomDataGenerator g_generator;

//------------------------------------------------------------------------------
// Collision Shape Benchmarks (SIMD vs Scalar comparison)
//------------------------------------------------------------------------------

static void BM_Physics_RectangleVsRectangle_SIMD(benchmark::State& state) {
    const int num_tests = state.range(0);
    
    std::vector<RectangleShape> rectangles;
    std::vector<Vector2<float>> positions1, positions2;
    
    rectangles.reserve(num_tests);
    positions1.reserve(num_tests);
    positions2.reserve(num_tests);
    
    for (int i = 0; i < num_tests; ++i) {
        rectangles.emplace_back(g_generator.randomSize());
        positions1.push_back(g_generator.randomPosition());
        positions2.push_back(g_generator.randomPosition());
    }
    
    for (auto _ : state) {
        size_t collisions = 0;
        for (int i = 0; i < num_tests; ++i) {
            // Uses SIMD AABB intersection under the hood
            if (CollisionDetection::intersects(rectangles[i], positions1[i], rectangles[i], positions2[i])) {
                collisions++;
            }
        }
        benchmark::DoNotOptimize(collisions);
    }
    
    state.SetItemsProcessed(state.iterations() * num_tests);
}
BENCHMARK(BM_Physics_RectangleVsRectangle_SIMD)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Physics_CircleVsCircle_SIMD(benchmark::State& state) {
    const int num_tests = state.range(0);
    
    std::vector<CircleShape> circles;
    std::vector<Vector2<float>> positions1, positions2;
    
    circles.reserve(num_tests);
    positions1.reserve(num_tests);
    positions2.reserve(num_tests);
    
    for (int i = 0; i < num_tests; ++i) {
        circles.emplace_back(g_generator.randomRadius());
        positions1.push_back(g_generator.randomPosition());
        positions2.push_back(g_generator.randomPosition());
    }
    
    for (auto _ : state) {
        size_t collisions = 0;
        for (int i = 0; i < num_tests; ++i) {
            // Uses SIMD Sphere intersection under the hood
            if (CollisionDetection::intersects(circles[i], positions1[i], circles[i], positions2[i])) {
                collisions++;
            }
        }
        benchmark::DoNotOptimize(collisions);
    }
    
    state.SetItemsProcessed(state.iterations() * num_tests);
}
BENCHMARK(BM_Physics_CircleVsCircle_SIMD)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Physics_RectangleVsCircle_SIMD(benchmark::State& state) {
    const int num_tests = state.range(0);
    
    std::vector<RectangleShape> rectangles;
    std::vector<CircleShape> circles;
    std::vector<Vector2<float>> rect_positions, circle_positions;
    
    rectangles.reserve(num_tests);
    circles.reserve(num_tests);
    rect_positions.reserve(num_tests);
    circle_positions.reserve(num_tests);
    
    for (int i = 0; i < num_tests; ++i) {
        rectangles.emplace_back(g_generator.randomSize());
        circles.emplace_back(g_generator.randomRadius());
        rect_positions.push_back(g_generator.randomPosition());
        circle_positions.push_back(g_generator.randomPosition());
    }
    
    for (auto _ : state) {
        size_t collisions = 0;
        for (int i = 0; i < num_tests; ++i) {
            // Uses SIMD AABB vs Sphere intersection under the hood
            if (CollisionDetection::intersects(rectangles[i], rect_positions[i], circles[i], circle_positions[i])) {
                collisions++;
            }
        }
        benchmark::DoNotOptimize(collisions);
    }
    
    state.SetItemsProcessed(state.iterations() * num_tests);
}
BENCHMARK(BM_Physics_RectangleVsCircle_SIMD)->Arg(1000)->Arg(10000)->Arg(100000);

//------------------------------------------------------------------------------
// Containment Tests (Point-in-Shape) - Leveraging SIMD
//------------------------------------------------------------------------------

static void BM_Physics_RectangleContainment_SIMD(benchmark::State& state) {
    const int num_tests = state.range(0);
    
    RectangleShape rectangle(Vector2<float>(10.0f, 10.0f));
    Vector2<float> rect_pos(0.0f, 0.0f);
    
    std::vector<Vector2<float>> test_points;
    test_points.reserve(num_tests);
    
    for (int i = 0; i < num_tests; ++i) {
        test_points.push_back(g_generator.randomPosition());
    }
    
    for (auto _ : state) {
        size_t contained = 0;
        for (int i = 0; i < num_tests; ++i) {
            // Uses SIMD AABB containment test under the hood
            if (CollisionDetection::contains(rectangle, rect_pos, test_points[i])) {
                contained++;
            }
        }
        benchmark::DoNotOptimize(contained);
    }
    
    state.SetItemsProcessed(state.iterations() * num_tests);
}
BENCHMARK(BM_Physics_RectangleContainment_SIMD)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Physics_CircleContainment_SIMD(benchmark::State& state) {
    const int num_tests = state.range(0);
    
    CircleShape circle(5.0f);
    Vector2<float> circle_pos(0.0f, 0.0f);
    
    std::vector<Vector2<float>> test_points;
    test_points.reserve(num_tests);
    
    for (int i = 0; i < num_tests; ++i) {
        test_points.push_back(g_generator.randomPosition());
    }
    
    for (auto _ : state) {
        size_t contained = 0;
        for (int i = 0; i < num_tests; ++i) {
            // Uses SIMD Sphere containment test under the hood
            if (CollisionDetection::contains(circle, circle_pos, test_points[i])) {
                contained++;
            }
        }
        benchmark::DoNotOptimize(contained);
    }
    
    state.SetItemsProcessed(state.iterations() * num_tests);
}
BENCHMARK(BM_Physics_CircleContainment_SIMD)->Arg(1000)->Arg(10000)->Arg(100000);

//------------------------------------------------------------------------------
// Collision Manifold Generation Benchmarks  
//------------------------------------------------------------------------------

static void BM_Physics_ManifoldGeneration_RectVsRect(benchmark::State& state) {
    const int num_tests = state.range(0);
    
    std::vector<RectangleShape> rectangles1, rectangles2;
    std::vector<Vector2<float>> positions1, positions2;
    
    rectangles1.reserve(num_tests);
    rectangles2.reserve(num_tests);
    positions1.reserve(num_tests);
    positions2.reserve(num_tests);
    
    for (int i = 0; i < num_tests; ++i) {
        rectangles1.emplace_back(g_generator.randomSize());
        rectangles2.emplace_back(g_generator.randomSize());
        positions1.push_back(g_generator.randomPosition());
        positions2.push_back(g_generator.randomPosition());
    }
    
    for (auto _ : state) {
        size_t manifolds_generated = 0;
        for (int i = 0; i < num_tests; ++i) {
            auto manifold = CollisionDetection::generateManifold(rectangles1[i], positions1[i], rectangles2[i], positions2[i]);
            if (manifold.hasCollision) {
                manifolds_generated++;
            }
        }
        benchmark::DoNotOptimize(manifolds_generated);
    }
    
    state.SetItemsProcessed(state.iterations() * num_tests);
}
BENCHMARK(BM_Physics_ManifoldGeneration_RectVsRect)->Arg(1000)->Arg(10000);

static void BM_Physics_ManifoldGeneration_CircleVsCircle(benchmark::State& state) {
    const int num_tests = state.range(0);
    
    std::vector<CircleShape> circles1, circles2;
    std::vector<Vector2<float>> positions1, positions2;
    
    circles1.reserve(num_tests);
    circles2.reserve(num_tests);
    positions1.reserve(num_tests);
    positions2.reserve(num_tests);
    
    for (int i = 0; i < num_tests; ++i) {
        circles1.emplace_back(g_generator.randomRadius());
        circles2.emplace_back(g_generator.randomRadius());
        positions1.push_back(g_generator.randomPosition());
        positions2.push_back(g_generator.randomPosition());
    }
    
    for (auto _ : state) {
        size_t manifolds_generated = 0;
        for (int i = 0; i < num_tests; ++i) {
            auto manifold = CollisionDetection::generateManifold(circles1[i], positions1[i], circles2[i], positions2[i]);
            if (manifold.hasCollision) {
                manifolds_generated++;
            }
        }
        benchmark::DoNotOptimize(manifolds_generated);
    }
    
    state.SetItemsProcessed(state.iterations() * num_tests);
}
BENCHMARK(BM_Physics_ManifoldGeneration_CircleVsCircle)->Arg(1000)->Arg(10000);

//------------------------------------------------------------------------------
// RigidBody Integration Benchmarks
//------------------------------------------------------------------------------

static void BM_Physics_RigidBodyIntegration(benchmark::State& state) {
    const int num_bodies = state.range(0);
    const float deltaTime = 1.0f / 60.0f;
    
    std::vector<std::shared_ptr<RigidBody>> bodies;
    bodies.reserve(num_bodies);
    
    // Create mix of rectangle and circle bodies
    for (int i = 0; i < num_bodies; ++i) {
        std::shared_ptr<CollisionShape> shape;
        if (i % 2 == 0) {
            shape = std::make_shared<RectangleShape>(g_generator.randomSize());
        } else {
            shape = std::make_shared<CircleShape>(g_generator.randomRadius());
        }
        
        auto body = std::make_shared<RigidBody>(shape, BodyType::Dynamic);
        body->setPosition(g_generator.randomPosition());
        body->setLinearVelocity(g_generator.randomVelocity());
        body->applyForce(Vector2<float>(0.0f, -9.81f) * body->getMass()); // Gravity
        
        bodies.push_back(body);
    }
    
    for (auto _ : state) {
        for (auto& body : bodies) {
            body->integrate(deltaTime);
        }
        benchmark::DoNotOptimize(bodies[0]->getPosition());
    }
    
    state.SetItemsProcessed(state.iterations() * num_bodies);
}
BENCHMARK(BM_Physics_RigidBodyIntegration)->Arg(100)->Arg(1000)->Arg(10000);

//------------------------------------------------------------------------------
// Broad Phase Collision Detection (SIMD AABB tests)
//------------------------------------------------------------------------------

static void BM_Physics_BroadPhaseCollisionAABB(benchmark::State& state) {
    const int num_bodies = state.range(0);
    
    std::vector<std::shared_ptr<RigidBody>> bodies;
    bodies.reserve(num_bodies);
    
    // Create bodies with rectangle shapes (for AABB tests)
    for (int i = 0; i < num_bodies; ++i) {
        auto shape = std::make_shared<RectangleShape>(g_generator.randomSize());
        auto body = std::make_shared<RigidBody>(shape, BodyType::Dynamic);
        body->setPosition(g_generator.randomPosition());
        bodies.push_back(body);
    }
    
    for (auto _ : state) {
        size_t potential_collisions = 0;
        
        // Broad phase: test AABB overlaps using SIMD
        for (int i = 0; i < num_bodies; ++i) {
            auto bounds1 = bodies[i]->getWorldBounds();
            
            for (int j = i + 1; j < num_bodies; ++j) {
                auto bounds2 = bodies[j]->getWorldBounds();
                
                // This uses SIMD AABB intersection test
                if (bounds1.intersects(bounds2)) {
                    potential_collisions++;
                }
            }
        }
        
        benchmark::DoNotOptimize(potential_collisions);
    }
    
    state.SetItemsProcessed(state.iterations() * (num_bodies * (num_bodies - 1) / 2));
}
BENCHMARK(BM_Physics_BroadPhaseCollisionAABB)->Arg(50)->Arg(100)->Arg(200);

//------------------------------------------------------------------------------
// Memory Performance Tests
//------------------------------------------------------------------------------

static void BM_Physics_ShapeCreationDestruction(benchmark::State& state) {
    const int num_shapes = state.range(0);
    
    for (auto _ : state) {
        std::vector<std::shared_ptr<CollisionShape>> shapes;
        shapes.reserve(num_shapes);
        
        for (int i = 0; i < num_shapes; ++i) {
            if (i % 2 == 0) {
                shapes.push_back(std::make_shared<RectangleShape>(g_generator.randomSize()));
            } else {
                shapes.push_back(std::make_shared<CircleShape>(g_generator.randomRadius()));
            }
        }
        
        benchmark::DoNotOptimize(shapes);
    }
    
    state.SetItemsProcessed(state.iterations() * num_shapes);
}
BENCHMARK(BM_Physics_ShapeCreationDestruction)->Arg(1000)->Arg(10000);

//------------------------------------------------------------------------------
// Cache Performance Tests
//------------------------------------------------------------------------------

static void BM_Physics_CacheFriendlyCollisionTest(benchmark::State& state) {
    const int num_tests = state.range(0);
    
    // Structure of Arrays (SoA) layout for better cache performance
    std::vector<RectangleShape> rectangles(num_tests, RectangleShape(Vector2<float>(2.0f, 2.0f)));
    std::vector<Vector2<float>> positions1(num_tests);
    std::vector<Vector2<float>> positions2(num_tests);
    std::vector<bool> results(num_tests);
    
    // Initialize data
    for (int i = 0; i < num_tests; ++i) {
        positions1[i] = g_generator.randomPosition();
        positions2[i] = g_generator.randomPosition();
    }
    
    for (auto _ : state) {
        // Process all collision tests in sequence (cache-friendly)
        for (int i = 0; i < num_tests; ++i) {
            results[i] = CollisionDetection::intersects(rectangles[i], positions1[i], rectangles[i], positions2[i]);
        }
        benchmark::DoNotOptimize(results);
    }
    
    state.SetItemsProcessed(state.iterations() * num_tests);
}
BENCHMARK(BM_Physics_CacheFriendlyCollisionTest)->Arg(1000)->Arg(10000)->Arg(100000);

//------------------------------------------------------------------------------
// Comparison with existing math benchmarks
//------------------------------------------------------------------------------

static void BM_Physics_CompareWithExistingSIMD_AABB(benchmark::State& state) {
    const int num_tests = state.range(0);
    
    // Test the same AABB intersection that the existing math benchmarks use
    std::vector<AABB<float>> aabbs;
    aabbs.reserve(num_tests * 2);
    
    for (int i = 0; i < num_tests * 2; ++i) {
        Vector2<float> pos = g_generator.randomPosition();
        Vector2<float> size = g_generator.randomSize();
        Vector2<float> half_size = size * 0.5f;
        
        SIMD::Vector<float, 3> min(pos.x - half_size.x, pos.y - half_size.y, 0.0f);
        SIMD::Vector<float, 3> max(pos.x + half_size.x, pos.y + half_size.y, 0.0f);
        aabbs.emplace_back(min, max);
    }
    
    for (auto _ : state) {
        size_t collisions = 0;
        for (int i = 0; i < num_tests; i += 2) {
            // Same SIMD AABB test as in existing benchmarks
            if (aabbs[i].intersects(aabbs[i + 1])) {
                collisions++;
            }
        }
        benchmark::DoNotOptimize(collisions);
    }
    
    state.SetItemsProcessed(state.iterations() * (num_tests / 2));
}
BENCHMARK(BM_Physics_CompareWithExistingSIMD_AABB)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();