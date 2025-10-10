#include <benchmark/benchmark.h>
#include "particles/particle_system.hpp"
#include "particles/particle_emitter.hpp"
#include <vectors/vector2.hpp>
#include <vectors/vector4.hpp>
#include <memory>
#include <random>

using namespace PyNovaGE;
using namespace PyNovaGE::Particles;

// ========== Benchmark Fixtures ==========

class ParticleSystemBenchmark : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override {
        config_.max_particles = state.range(0);
        config_.enable_sorting = false;
        config_.enable_culling = false;
        
        system_ = std::make_unique<ParticleSystem>(config_);
        system_->Initialize();
    }
    
    void TearDown(const ::benchmark::State& state) override {
        system_->Shutdown();
        system_.reset();
    }
    
protected:
    ParticleSystemConfig config_;
    std::unique_ptr<ParticleSystem> system_;
    std::mt19937 rng_{std::random_device{}()};
    std::uniform_real_distribution<float> dist_{-10.0f, 10.0f};
};

// ========== Core Performance Benchmarks ==========

BENCHMARK_DEFINE_F(ParticleSystemBenchmark, ParticleSpawning)(benchmark::State& state) {
    ParticleInitData init_data;
    init_data.lifetime = 1.0f;
    init_data.velocity = Vector2f(dist_(rng_), dist_(rng_));
    init_data.acceleration = Vector2f(0.0f, -9.81f);
    
    for (auto _ : state) {
        state.PauseTiming();
        system_->ClearParticles();
        state.ResumeTiming();
        
        // Spawn particles up to capacity
        for (int i = 0; i < state.range(0) && !system_->IsPoolFull(); ++i) {
            init_data.position = Vector2f(dist_(rng_), dist_(rng_));
            init_data.velocity = Vector2f(dist_(rng_), dist_(rng_));
            system_->SpawnParticle(init_data);
        }
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * state.range(0));
    state.SetBytesProcessed(int64_t(state.iterations()) * state.range(0) * sizeof(Particle));
}
BENCHMARK_REGISTER_F(ParticleSystemBenchmark, ParticleSpawning)
    ->Range(100, 10000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_DEFINE_F(ParticleSystemBenchmark, ParticleUpdate)(benchmark::State& state) {
    // Pre-fill with particles
    ParticleInitData init_data;
    init_data.lifetime = 10.0f; // Long lifetime
    init_data.acceleration = Vector2f(0.0f, -9.81f);
    
    for (int i = 0; i < state.range(0) && !system_->IsPoolFull(); ++i) {
        init_data.position = Vector2f(dist_(rng_), dist_(rng_));
        init_data.velocity = Vector2f(dist_(rng_), dist_(rng_));
        system_->SpawnParticle(init_data);
    }
    
    const float dt = 1.0f / 60.0f; // 60 FPS
    
    for (auto _ : state) {
        system_->Update(dt);
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * system_->GetActiveParticleCount());
}
BENCHMARK_REGISTER_F(ParticleSystemBenchmark, ParticleUpdate)
    ->Range(100, 10000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_DEFINE_F(ParticleSystemBenchmark, EmitterUpdate)(benchmark::State& state) {
    // Create multiple emitters
    const int num_emitters = 10;
    std::vector<std::shared_ptr<ParticleEmitter>> emitters;
    
    EmitterConfig emitter_config;
    emitter_config.emission_rate = static_cast<float>(state.range(0)) / num_emitters;
    emitter_config.initial.lifetime_min = 0.5f;
    emitter_config.initial.lifetime_max = 2.0f;
    emitter_config.gravity = Vector2f(0.0f, -9.81f);
    
    for (int i = 0; i < num_emitters; ++i) {
        emitter_config.position = Vector2f(dist_(rng_), dist_(rng_));
        auto emitter = system_->CreateEmitter(emitter_config);
        emitter->Start();
        emitters.push_back(emitter);
    }
    
    const float dt = 1.0f / 60.0f;
    
    for (auto _ : state) {
        system_->Update(dt);
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * system_->GetActiveParticleCount());
}
BENCHMARK_REGISTER_F(ParticleSystemBenchmark, EmitterUpdate)
    ->Range(100, 5000)
    ->Unit(benchmark::kMicrosecond);

// ========== Force Application Benchmarks ==========

BENCHMARK_DEFINE_F(ParticleSystemBenchmark, GlobalForceApplication)(benchmark::State& state) {
    // Pre-fill with particles
    ParticleInitData init_data;
    init_data.lifetime = 10.0f;
    init_data.mass = 1.0f;
    
    for (int i = 0; i < state.range(0) && !system_->IsPoolFull(); ++i) {
        init_data.position = Vector2f(dist_(rng_), dist_(rng_));
        system_->SpawnParticle(init_data);
    }
    
    Vector2f force(1.0f, 0.0f);
    
    for (auto _ : state) {
        system_->ApplyGlobalForce(force);
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * system_->GetActiveParticleCount());
}
BENCHMARK_REGISTER_F(ParticleSystemBenchmark, GlobalForceApplication)
    ->Range(100, 10000)
    ->Unit(benchmark::kNanosecond);

BENCHMARK_DEFINE_F(ParticleSystemBenchmark, RadialForceApplication)(benchmark::State& state) {
    // Pre-fill with particles in a grid
    ParticleInitData init_data;
    init_data.lifetime = 10.0f;
    init_data.mass = 1.0f;
    
    int side = static_cast<int>(std::sqrt(state.range(0)));
    for (int y = 0; y < side; ++y) {
        for (int x = 0; x < side && system_->GetActiveParticleCount() < state.range(0); ++x) {
            init_data.position = Vector2f(x * 2.0f - side, y * 2.0f - side);
            system_->SpawnParticle(init_data);
        }
    }
    
    Vector2f center(0.0f, 0.0f);
    float radius = side * 0.5f;
    Vector2f force(0.0f, 10.0f);
    
    for (auto _ : state) {
        system_->ApplyRadialForce(center, radius, force, true);
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * system_->GetActiveParticleCount());
}
BENCHMARK_REGISTER_F(ParticleSystemBenchmark, RadialForceApplication)
    ->Range(100, 10000)
    ->Unit(benchmark::kMicrosecond);

// ========== Memory Management Benchmarks ==========

BENCHMARK_DEFINE_F(ParticleSystemBenchmark, ParticlePoolChurn)(benchmark::State& state) {
    ParticleInitData init_data;
    init_data.lifetime = 0.01f; // Very short lifetime for rapid turnover
    init_data.velocity = Vector2f(dist_(rng_), dist_(rng_));
    
    const float dt = 1.0f / 60.0f;
    
    for (auto _ : state) {
        // Spawn some particles
        for (int i = 0; i < 10 && !system_->IsPoolFull(); ++i) {
            init_data.position = Vector2f(dist_(rng_), dist_(rng_));
            system_->SpawnParticle(init_data);
        }
        
        // Update to trigger cleanup
        system_->Update(dt);
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * 10);
}
BENCHMARK_REGISTER_F(ParticleSystemBenchmark, ParticlePoolChurn)
    ->Range(1000, 10000)
    ->Unit(benchmark::kMicrosecond);

// ========== Sorting and Culling Benchmarks ==========

class ParticleSystemSortingBenchmark : public ParticleSystemBenchmark {
public:
    void SetUp(const ::benchmark::State& state) override {
        ParticleSystemBenchmark::SetUp(state);
        
        ParticleSystemConfig sorting_config = config_;
        sorting_config.enable_sorting = true;
        system_->SetConfig(sorting_config);
    }
};

BENCHMARK_DEFINE_F(ParticleSystemSortingBenchmark, SortedUpdate)(benchmark::State& state) {
    // Pre-fill with particles at random positions
    ParticleInitData init_data;
    init_data.lifetime = 10.0f;
    
    for (int i = 0; i < state.range(0) && !system_->IsPoolFull(); ++i) {
        init_data.position = Vector2f(dist_(rng_), dist_(rng_));
        init_data.velocity = Vector2f(dist_(rng_), dist_(rng_));
        system_->SpawnParticle(init_data);
    }
    
    const float dt = 1.0f / 60.0f;
    
    for (auto _ : state) {
        system_->Update(dt);
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * system_->GetActiveParticleCount());
}
BENCHMARK_REGISTER_F(ParticleSystemSortingBenchmark, SortedUpdate)
    ->Range(100, 5000)
    ->Unit(benchmark::kMicrosecond);

// ========== Realistic Scenario Benchmarks ==========

BENCHMARK_DEFINE_F(ParticleSystemBenchmark, ExplosionEffect)(benchmark::State& state) {
    // Create explosion emitter
    EmitterConfig explosion_config;
    explosion_config.emission_rate = 0.0f; // Burst only
    explosion_config.auto_emit = false;
    explosion_config.shape = EmissionShape::Circle;
    explosion_config.shape_data = Vector2f(2.0f, 0.0f);
    explosion_config.initial.velocity_min = Vector2f(-20.0f, -20.0f);
    explosion_config.initial.velocity_max = Vector2f(20.0f, 20.0f);
    explosion_config.initial.lifetime_min = 0.5f;
    explosion_config.initial.lifetime_max = 2.0f;
    explosion_config.initial.drag_min = 1.0f;
    explosion_config.initial.drag_max = 3.0f;
    explosion_config.gravity = Vector2f(0.0f, -9.81f);
    
    auto emitter = system_->CreateEmitter(explosion_config);
    
    const float dt = 1.0f / 60.0f;
    
    for (auto _ : state) {
        state.PauseTiming();
        system_->ClearParticles();
        state.ResumeTiming();
        
        // Trigger explosion
        emitter->EmitBurst(state.range(0));
        
        // Simulate for a short time
        for (int i = 0; i < 30; ++i) { // 0.5 seconds
            system_->Update(dt);
        }
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * state.range(0));
}
BENCHMARK_REGISTER_F(ParticleSystemBenchmark, ExplosionEffect)
    ->Range(50, 1000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_DEFINE_F(ParticleSystemBenchmark, ContinuousEmission)(benchmark::State& state) {
    // Create continuous emitter (like fire or smoke)
    EmitterConfig continuous_config;
    continuous_config.emission_rate = static_cast<float>(state.range(0)) / 2.0f; // Half the target count per second
    continuous_config.initial.lifetime_min = 1.0f;
    continuous_config.initial.lifetime_max = 3.0f;
    continuous_config.initial.velocity_min = Vector2f(-1.0f, 0.0f);
    continuous_config.initial.velocity_max = Vector2f(1.0f, 5.0f);
    continuous_config.gravity = Vector2f(0.0f, 1.0f); // Upward drift
    continuous_config.initial.drag_min = 0.5f;
    continuous_config.initial.drag_max = 2.0f;
    
    auto emitter = system_->CreateEmitter(continuous_config);
    emitter->Start();
    
    const float dt = 1.0f / 60.0f;
    
    // Let it build up particles first
    for (int i = 0; i < 120; ++i) { // 2 seconds
        system_->Update(dt);
    }
    
    for (auto _ : state) {
        system_->Update(dt);
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * system_->GetActiveParticleCount());
}
BENCHMARK_REGISTER_F(ParticleSystemBenchmark, ContinuousEmission)
    ->Range(100, 2000)
    ->Unit(benchmark::kMicrosecond);

// ========== Standalone Function Benchmarks ==========

static void BM_ParticlePhysicsUpdate(benchmark::State& state) {
    Particle particle;
    particle.position = Vector2f(0.0f, 0.0f);
    particle.velocity = Vector2f(10.0f, 5.0f);
    particle.acceleration = Vector2f(0.0f, -9.81f);
    particle.drag = 0.1f;
    particle.lifetime = 10.0f;
    
    const float dt = 1.0f / 60.0f;
    
    for (auto _ : state) {
        particle.Update(dt);
        benchmark::DoNotOptimize(particle.position);
        benchmark::DoNotOptimize(particle.velocity);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ParticlePhysicsUpdate)->Unit(benchmark::kNanosecond);

static void BM_ParticleForceApplication(benchmark::State& state) {
    Particle particle;
    particle.mass = 1.0f;
    Vector2f force(5.0f, -2.0f);
    
    for (auto _ : state) {
        particle.ApplyForce(force);
        benchmark::DoNotOptimize(particle.acceleration);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ParticleForceApplication)->Unit(benchmark::kNanosecond);

// ========== Summary and Custom Counters ==========

static void CustomCounters(benchmark::State& state) {
    ParticleSystemConfig config;
    config.max_particles = 1000;
    
    ParticleSystem system(config);
    system.Initialize();
    
    // Create emitter
    EmitterConfig emitter_config;
    emitter_config.emission_rate = 100.0f;
    emitter_config.initial.lifetime_min = 1.0f;
    emitter_config.initial.lifetime_max = 2.0f;
    
    auto emitter = system.CreateEmitter(emitter_config);
    emitter->Start();
    
    const float dt = 1.0f / 60.0f;
    
    // Build up particles
    for (int i = 0; i < 60; ++i) {
        system.Update(dt);
    }
    
    for (auto _ : state) {
        system.Update(dt);
    }
    
    const auto& stats = system.GetStats();
    
    state.counters["ActiveParticles"] = stats.active_particles;
    state.counters["PeakParticles"] = stats.peak_active_particles;
    state.counters["TotalSpawned"] = stats.total_particles_spawned;
    state.counters["PoolUtilization"] = benchmark::Counter(
        static_cast<double>(stats.active_particles) / config.max_particles * 100.0,
        benchmark::Counter::kDefaults, "%");
    state.counters["UpdateTimeMs"] = stats.update_time_ms;
}
BENCHMARK(CustomCounters)->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();