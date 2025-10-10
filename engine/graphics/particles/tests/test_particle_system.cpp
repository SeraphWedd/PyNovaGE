#include <gtest/gtest.h>
#include "particles/particle.hpp"
#include "particles/particle_emitter.hpp"
#include "particles/particle_system.hpp"
#include <vectors/vector2.hpp>
#include <vectors/vector4.hpp>
#include <memory>
#include <chrono>
#include <thread>

using namespace PyNovaGE;
using namespace PyNovaGE::Particles;

class ParticleSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Default system configuration
        config_.max_particles = 100;
        config_.enable_sorting = false;
        config_.enable_culling = false;
        
        system_ = std::make_unique<ParticleSystem>(config_);
        ASSERT_TRUE(system_->Initialize());
    }
    
    void TearDown() override {
        system_->Shutdown();
    }
    
    ParticleSystemConfig config_;
    std::unique_ptr<ParticleSystem> system_;
};

// ========== Particle Tests ==========

TEST(ParticleTest, DefaultConstruction) {
    Particle particle;
    
    EXPECT_EQ(particle.position, Vector2f(0.0f, 0.0f));
    EXPECT_EQ(particle.velocity, Vector2f(0.0f, 0.0f));
    EXPECT_EQ(particle.rotation, 0.0f);
    EXPECT_EQ(particle.angular_velocity, 0.0f);
    EXPECT_EQ(particle.size, Vector2f(1.0f, 1.0f));
    EXPECT_EQ(particle.color, Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
    EXPECT_EQ(particle.lifetime, 1.0f);
    EXPECT_EQ(particle.age, 0.0f);
    EXPECT_TRUE(particle.IsAlive());
    EXPECT_EQ(particle.GetNormalizedAge(), 0.0f);
}

TEST(ParticleTest, LifetimeManagement) {
    Particle particle;
    particle.lifetime = 2.0f;
    
    EXPECT_TRUE(particle.IsAlive());
    EXPECT_EQ(particle.GetNormalizedAge(), 0.0f);
    
    // Update to halfway through lifetime
    particle.age = 1.0f;
    EXPECT_TRUE(particle.IsAlive());
    EXPECT_NEAR(particle.GetNormalizedAge(), 0.5f, 0.001f);
    
    // Update past lifetime
    particle.age = 2.5f;
    EXPECT_FALSE(particle.IsAlive());
    EXPECT_NEAR(particle.GetNormalizedAge(), 1.25f, 0.001f);
}

TEST(ParticleTest, PhysicsUpdate) {
    Particle particle;
    particle.position = Vector2f(0.0f, 0.0f);
    particle.velocity = Vector2f(10.0f, 5.0f);
    particle.acceleration = Vector2f(0.0f, -9.81f); // Gravity
    
    float dt = 0.1f;
    particle.Update(dt);
    
    // Check position update (Euler integration: update velocity first, then position)
    EXPECT_NEAR(particle.position.x, 1.0f, 0.001f);
    EXPECT_NEAR(particle.position.y, 0.4019f, 0.001f); // velocity updated to (5 - 0.981), position += 4.019 * 0.1
    
    // Check velocity update from acceleration
    EXPECT_NEAR(particle.velocity.x, 10.0f, 0.001f);
    EXPECT_NEAR(particle.velocity.y, 5.0f - 0.981f, 0.001f);
}

TEST(ParticleTest, DragApplication) {
    Particle particle;
    particle.velocity = Vector2f(10.0f, 10.0f);
    particle.drag = 5.0f; // High drag
    
    float dt = 0.1f;
    particle.Update(dt);
    
    // Velocity should be reduced by drag
    EXPECT_LT(particle.velocity.x, 10.0f);
    EXPECT_LT(particle.velocity.y, 10.0f);
    EXPECT_GT(particle.velocity.x, 0.0f);
    EXPECT_GT(particle.velocity.y, 0.0f);
}

TEST(ParticleTest, ForceApplication) {
    Particle particle;
    particle.mass = 2.0f;
    particle.acceleration = Vector2f(0.0f, 0.0f);
    
    Vector2f force(20.0f, 10.0f);
    particle.ApplyForce(force);
    
    // F = ma, so a = F/m
    EXPECT_NEAR(particle.acceleration.x, 10.0f, 0.001f);
    EXPECT_NEAR(particle.acceleration.y, 5.0f, 0.001f);
}

TEST(ParticleTest, Reset) {
    Particle particle;
    particle.position = Vector2f(10.0f, 20.0f);
    particle.velocity = Vector2f(5.0f, -3.0f);
    particle.age = 1.5f;
    
    particle.Reset();
    
    EXPECT_EQ(particle.position, Vector2f(0.0f, 0.0f));
    EXPECT_EQ(particle.velocity, Vector2f(0.0f, 0.0f));
    EXPECT_EQ(particle.age, 0.0f);
    EXPECT_TRUE(particle.IsAlive());
}

// ========== ParticleEmitter Tests ==========

TEST(ParticleEmitterTest, DefaultConfiguration) {
    ParticleEmitter emitter;
    const auto& config = emitter.GetConfig();
    
    EXPECT_EQ(config.emission_rate, 10.0f);
    EXPECT_TRUE(config.auto_emit);
    EXPECT_TRUE(config.looping);
    EXPECT_EQ(config.shape, EmissionShape::Point);
}

TEST(ParticleEmitterTest, EmissionControl) {
    ParticleEmitter emitter;
    
    EXPECT_FALSE(emitter.IsActive());
    
    emitter.Start();
    EXPECT_TRUE(emitter.IsActive());
    
    emitter.SetPaused(true);
    EXPECT_FALSE(emitter.IsActive());
    
    emitter.SetPaused(false);
    EXPECT_TRUE(emitter.IsActive());
    
    emitter.Stop();
    EXPECT_FALSE(emitter.IsActive());
}

TEST(ParticleEmitterTest, EmissionCallback) {
    EmitterConfig config;
    config.emission_rate = 100.0f; // High rate for testing
    config.auto_emit = false; // Manual control
    
    ParticleEmitter emitter(config);
    
    int particles_emitted = 0;
    emitter.SetEmitCallback([&particles_emitted](const ParticleInitData& /* data */) {
        particles_emitted++;
    });
    
    emitter.EmitParticle();
    EXPECT_EQ(particles_emitted, 1);
    
    emitter.EmitBurst(5);
    EXPECT_EQ(particles_emitted, 6);
}

TEST(ParticleEmitterTest, ShapeEmission) {
    EmitterConfig config;
    config.shape = EmissionShape::Circle;
    config.shape_data = Vector2f(5.0f, 0.0f); // Radius = 5
    
    ParticleEmitter emitter(config);
    
    bool callback_called = false;
    Vector2f emitted_position;
    
    emitter.SetEmitCallback([&callback_called, &emitted_position](const ParticleInitData& data) {
        callback_called = true;
        emitted_position = data.position;
    });
    
    emitter.EmitParticle();
    
    EXPECT_TRUE(callback_called);
    
    // Check that position is within circle radius
    float distance = std::sqrt(emitted_position.x * emitted_position.x + 
                              emitted_position.y * emitted_position.y);
    EXPECT_LE(distance, 5.0f);
}

TEST(ParticleEmitterTest, BurstEmission) {
    EmitterConfig config;
    config.auto_emit = false;
    config.bursts.push_back(EmissionBurst(0.5f, 10, 1.0f)); // Burst at 0.5 seconds
    
    ParticleEmitter emitter(config);
    emitter.Start();
    
    int particles_emitted = 0;
    emitter.SetEmitCallback([&particles_emitted](const ParticleInitData& /* data */) {
        particles_emitted++;
    });
    
    // Update before burst time
    emitter.Update(0.25f);
    EXPECT_EQ(particles_emitted, 0);
    
    // Update past burst time
    emitter.Update(0.5f);
    EXPECT_EQ(particles_emitted, 10);
}

// ========== ParticleSystem Tests ==========

TEST_F(ParticleSystemTest, Initialization) {
    EXPECT_TRUE(system_->IsInitialized());
    EXPECT_EQ(system_->GetActiveParticleCount(), 0);
    EXPECT_EQ(system_->GetActiveEmitterCount(), 0);
    EXPECT_EQ(system_->GetMaxParticles(), 100);
}

TEST_F(ParticleSystemTest, ManualParticleSpawning) {
    ParticleInitData init_data;
    init_data.position = Vector2f(10.0f, 20.0f);
    init_data.velocity = Vector2f(1.0f, 2.0f);
    init_data.lifetime = 2.0f;
    
    Particle* particle = system_->SpawnParticle(init_data);
    
    ASSERT_NE(particle, nullptr);
    EXPECT_EQ(system_->GetActiveParticleCount(), 1);
    EXPECT_EQ(particle->position, Vector2f(10.0f, 20.0f));
    EXPECT_EQ(particle->velocity, Vector2f(1.0f, 2.0f));
    EXPECT_EQ(particle->lifetime, 2.0f);
}

TEST_F(ParticleSystemTest, ParticleDestruction) {
    ParticleInitData init_data;
    init_data.lifetime = 1.0f;
    
    Particle* particle = system_->SpawnParticle(init_data);
    ASSERT_NE(particle, nullptr);
    EXPECT_EQ(system_->GetActiveParticleCount(), 1);
    
    system_->DestroyParticle(particle);
    EXPECT_EQ(system_->GetActiveParticleCount(), 0);
}

TEST_F(ParticleSystemTest, AutomaticParticleCleanup) {
    ParticleInitData init_data;
    init_data.lifetime = 0.1f; // Very short lifetime
    
    system_->SpawnParticle(init_data);
    EXPECT_EQ(system_->GetActiveParticleCount(), 1);
    
    // Update past particle lifetime
    system_->Update(0.2f);
    
    // Particle should be automatically cleaned up
    EXPECT_EQ(system_->GetActiveParticleCount(), 0);
}

TEST_F(ParticleSystemTest, EmitterIntegration) {
    EmitterConfig emitter_config;
    emitter_config.emission_rate = 50.0f;
    emitter_config.auto_emit = false; // Manual control
    emitter_config.initial.lifetime_min = 1.0f;
    emitter_config.initial.lifetime_max = 1.0f;
    
    auto emitter = system_->CreateEmitter(emitter_config);
    EXPECT_EQ(system_->GetActiveEmitterCount(), 1);
    
    emitter->Start();
    emitter->EmitBurst(5);
    
    EXPECT_EQ(system_->GetActiveParticleCount(), 5);
    
    system_->RemoveEmitter(emitter);
    EXPECT_EQ(system_->GetActiveEmitterCount(), 0);
}

TEST_F(ParticleSystemTest, GlobalForceApplication) {
    ParticleInitData init_data;
    init_data.mass = 1.0f;
    
    Particle* particle1 = system_->SpawnParticle(init_data);
    Particle* particle2 = system_->SpawnParticle(init_data);
    
    Vector2f initial_accel1 = particle1->acceleration;
    Vector2f initial_accel2 = particle2->acceleration;
    
    Vector2f global_force(0.0f, 10.0f);
    system_->ApplyGlobalForce(global_force);
    
    EXPECT_NE(particle1->acceleration, initial_accel1);
    EXPECT_NE(particle2->acceleration, initial_accel2);
    EXPECT_EQ(particle1->acceleration.y, initial_accel1.y + 10.0f);
    EXPECT_EQ(particle2->acceleration.y, initial_accel2.y + 10.0f);
}

TEST_F(ParticleSystemTest, RadialForceApplication) {
    ParticleInitData init_data;
    init_data.mass = 1.0f;
    init_data.position = Vector2f(0.0f, 0.0f);
    
    Particle* close_particle = system_->SpawnParticle(init_data);
    
    init_data.position = Vector2f(20.0f, 0.0f);
    Particle* far_particle = system_->SpawnParticle(init_data);
    
    Vector2f center(0.0f, 0.0f);
    float radius = 10.0f;
    Vector2f force(0.0f, 5.0f);
    
    Vector2f initial_accel_close = close_particle->acceleration;
    Vector2f initial_accel_far = far_particle->acceleration;
    
    system_->ApplyRadialForce(center, radius, force, false);
    
    // Close particle should be affected
    EXPECT_NE(close_particle->acceleration, initial_accel_close);
    
    // Far particle should not be affected
    EXPECT_EQ(far_particle->acceleration, initial_accel_far);
}

TEST_F(ParticleSystemTest, PoolExhaustion) {
    ParticleInitData init_data;
    init_data.lifetime = 10.0f; // Long lifetime to keep particles alive
    
    // Fill the pool
    for (size_t i = 0; i < config_.max_particles; ++i) {
        Particle* particle = system_->SpawnParticle(init_data);
        EXPECT_NE(particle, nullptr);
    }
    
    EXPECT_EQ(system_->GetActiveParticleCount(), config_.max_particles);
    EXPECT_TRUE(system_->IsPoolFull());
    
    // Try to spawn one more - should fail
    Particle* overflow_particle = system_->SpawnParticle(init_data);
    EXPECT_EQ(overflow_particle, nullptr);
    EXPECT_EQ(system_->GetActiveParticleCount(), config_.max_particles);
}

TEST_F(ParticleSystemTest, Statistics) {
    ParticleInitData init_data;
    init_data.lifetime = 0.1f;
    
    // Spawn particles
    system_->SpawnParticle(init_data);
    system_->SpawnParticle(init_data);
    
    // Update to populate stats
    system_->Update(0.01f); // Small update to populate initial stats
    
    const auto& stats_after_spawn = system_->GetStats();
    EXPECT_EQ(stats_after_spawn.total_particles_spawned, 2);
    EXPECT_EQ(stats_after_spawn.active_particles, 2);
    
    // Update to let particles die
    system_->Update(0.2f);
    
    const auto& stats_after_death = system_->GetStats();
    EXPECT_EQ(stats_after_death.active_particles, 0);
    EXPECT_EQ(stats_after_death.peak_active_particles, 2);
    EXPECT_EQ(stats_after_death.total_particles_spawned, 2);
    
    // Check that timing statistics are populated
    EXPECT_GE(stats_after_death.update_time_ms, 0.0f);
}

TEST_F(ParticleSystemTest, ConfigurationChanges) {
    ParticleSystemConfig new_config;
    new_config.max_particles = 50;
    new_config.enable_sorting = true;
    new_config.enable_culling = true;
    
    system_->SetConfig(new_config);
    
    const auto& config = system_->GetConfig();
    EXPECT_EQ(config.max_particles, 50);
    EXPECT_TRUE(config.enable_sorting);
    EXPECT_TRUE(config.enable_culling);
    EXPECT_EQ(system_->GetMaxParticles(), 50);
}

// ========== Integration Tests ==========

TEST_F(ParticleSystemTest, FullWorkflowIntegration) {
    // Create emitter with specific configuration
    EmitterConfig emitter_config;
    emitter_config.position = Vector2f(0.0f, 0.0f);
    emitter_config.emission_rate = 20.0f;
    emitter_config.duration = 1.0f;
    emitter_config.looping = false;
    emitter_config.shape = EmissionShape::Circle;
    emitter_config.shape_data = Vector2f(5.0f, 0.0f);
    
    // Set particle properties
    emitter_config.initial.lifetime_min = 0.5f;
    emitter_config.initial.lifetime_max = 1.5f;
    emitter_config.initial.velocity_min = Vector2f(-2.0f, -2.0f);
    emitter_config.initial.velocity_max = Vector2f(2.0f, 2.0f);
    emitter_config.gravity = Vector2f(0.0f, -5.0f);
    
    // Animation over time
    emitter_config.animation.color_start = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
    emitter_config.animation.color_end = Vector4f(1.0f, 0.0f, 0.0f, 0.0f); // Fade to transparent red
    
    auto emitter = system_->CreateEmitter(emitter_config);
    emitter->Start();
    
    // Simulate for several frames
    const float dt = 1.0f / 60.0f; // 60 FPS
    const int num_frames = 120; // 2 seconds
    
    size_t max_particles = 0;
    
    for (int frame = 0; frame < num_frames; ++frame) {
        system_->Update(dt);
        
        size_t current_particles = system_->GetActiveParticleCount();
        max_particles = std::max(max_particles, current_particles);
        
        // Apply some external forces occasionally
        if (frame % 30 == 0) {
            system_->ApplyGlobalForce(Vector2f(1.0f, 0.0f)); // Wind effect
        }
        
        // Radial explosion at frame 60
        if (frame == 60) {
            system_->ApplyRadialForce(Vector2f(0.0f, 0.0f), 10.0f, Vector2f(0.0f, 20.0f));
        }
    }
    
    const auto& stats = system_->GetStats();
    
    // Verify the simulation ran correctly
    EXPECT_GT(stats.total_particles_spawned, 0);
    EXPECT_GT(stats.peak_active_particles, 0);
    EXPECT_GE(stats.update_time_ms, 0.0f);
    EXPECT_GT(max_particles, 0);
    
    // By end of simulation, most particles should have died off
    EXPECT_LT(system_->GetActiveParticleCount(), stats.peak_active_particles);
}

// ========== Performance Test ==========

TEST_F(ParticleSystemTest, PerformanceStressTest) {
    // Reconfigure for large particle count
    ParticleSystemConfig perf_config;
    perf_config.max_particles = 5000;
    system_->SetConfig(perf_config);
    
    // Create high-rate emitter
    EmitterConfig emitter_config;
    emitter_config.emission_rate = 500.0f;
    emitter_config.initial.lifetime_min = 2.0f;
    emitter_config.initial.lifetime_max = 4.0f;
    emitter_config.initial.velocity_min = Vector2f(-10.0f, -10.0f);
    emitter_config.initial.velocity_max = Vector2f(10.0f, 10.0f);
    emitter_config.gravity = Vector2f(0.0f, -9.81f);
    
    auto emitter = system_->CreateEmitter(emitter_config);
    emitter->Start();
    
    // Run for a period to build up particles
    const float dt = 1.0f / 60.0f;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 300; ++i) { // 5 seconds at 60 FPS
        system_->Update(dt);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    const auto& stats = system_->GetStats();
    
    // Performance assertions
    EXPECT_GT(stats.peak_active_particles, 100); // Should have built up significant particles
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
    EXPECT_LT(stats.update_time_ms, 10.0f); // Individual updates should be fast
    
    std::cout << "Performance Results:\n";
    std::cout << "  Peak Particles: " << stats.peak_active_particles << "\n";
    std::cout << "  Total Runtime: " << duration.count() << "ms\n";
    std::cout << "  Avg Update Time: " << stats.update_time_ms << "ms\n";
    std::cout << "  Total Spawned: " << stats.total_particles_spawned << "\n";
}