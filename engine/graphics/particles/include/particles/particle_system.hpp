#pragma once

#include "particles/particle.hpp"
#include "particles/particle_emitter.hpp"
#include <memory/object_pool.h>
#include <renderer/batch_renderer.hpp>
#include <memory>
#include <vector>
#include <unordered_set>
#include <chrono>

namespace PyNovaGE {
namespace Particles {

/**
 * @brief Particle system statistics
 */
struct ParticleSystemStats {
    size_t active_particles = 0;        ///< Currently active particles
    size_t total_particles_spawned = 0; ///< Total particles spawned
    size_t peak_active_particles = 0;   ///< Peak active particles
    size_t active_emitters = 0;         ///< Currently active emitters
    size_t pool_size = 0;              ///< Total pool size
    size_t pool_free = 0;              ///< Free pool slots
    float update_time_ms = 0.0f;       ///< Last update time in milliseconds
    float render_time_ms = 0.0f;       ///< Last render time in milliseconds
    
    void Reset() {
        active_particles = 0;
        total_particles_spawned = 0;
        peak_active_particles = 0;
        active_emitters = 0;
        update_time_ms = 0.0f;
        render_time_ms = 0.0f;
    }
};

/**
 * @brief Particle system configuration
 */
struct ParticleSystemConfig {
    size_t max_particles = 10000;       ///< Maximum particles in pool
    bool enable_sorting = false;        ///< Enable depth sorting for particles
    bool enable_culling = false;        ///< Enable frustum culling
    Vector4f culling_bounds{-1000.0f, -1000.0f, 2000.0f, 2000.0f}; ///< Culling rectangle (x, y, width, height)
};

/**
 * @brief Main particle system class
 * 
 * Manages particle lifecycle, memory allocation, updates, and rendering.
 * Integrates with ObjectPool for efficient memory management and
 * BatchRenderer for efficient rendering.
 */
class ParticleSystem {
public:
    /**
     * @brief Constructor
     * @param config System configuration
     */
    explicit ParticleSystem(const ParticleSystemConfig& config = ParticleSystemConfig{});
    
    /**
     * @brief Destructor
     */
    ~ParticleSystem();
    
    // Non-copyable but movable
    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;
    ParticleSystem(ParticleSystem&&) = default;
    ParticleSystem& operator=(ParticleSystem&&) = default;
    
    /**
     * @brief Initialize the particle system
     * @return true if successful
     */
    bool Initialize();
    
    /**
     * @brief Shutdown and cleanup
     */
    void Shutdown();
    
    /**
     * @brief Check if system is initialized
     */
    bool IsInitialized() const { return initialized_; }
    
    /**
     * @brief Update all particles and emitters
     * @param dt Delta time in seconds
     */
    void Update(float dt);
    
    /**
     * @brief Render all active particles
     * @param batch_renderer BatchRenderer to use for rendering
     */
    void Render(Renderer::BatchRenderer& batch_renderer);
    
    /**
     * @brief Create and add a new emitter
     * @param config Emitter configuration
     * @return Shared pointer to the created emitter
     */
    std::shared_ptr<ParticleEmitter> CreateEmitter(const EmitterConfig& config);
    
    /**
     * @brief Remove an emitter
     * @param emitter Emitter to remove
     */
    void RemoveEmitter(std::shared_ptr<ParticleEmitter> emitter);
    
    /**
     * @brief Remove all emitters
     */
    void ClearEmitters();
    
    /**
     * @brief Manually spawn a particle
     * @param init_data Particle initialization data
     * @return Pointer to spawned particle, or nullptr if pool is full
     */
    Particle* SpawnParticle(const ParticleInitData& init_data);
    
    /**
     * @brief Manually destroy a particle
     * @param particle Particle to destroy
     */
    void DestroyParticle(Particle* particle);
    
    /**
     * @brief Clear all active particles
     */
    void ClearParticles();
    
    /**
     * @brief Apply force to all active particles
     * @param force Force vector to apply
     */
    void ApplyGlobalForce(const Vector2f& force);
    
    /**
     * @brief Apply force to particles within a radius
     * @param position Center position
     * @param radius Effect radius
     * @param force Force vector to apply
     * @param falloff Whether force should falloff with distance
     */
    void ApplyRadialForce(const Vector2f& position, float radius, const Vector2f& force, bool falloff = true);
    
    /**
     * @brief Set system configuration
     */
    void SetConfig(const ParticleSystemConfig& config);
    
    /**
     * @brief Get system configuration
     */
    const ParticleSystemConfig& GetConfig() const { return config_; }
    
    /**
     * @brief Get system statistics
     */
    const ParticleSystemStats& GetStats() const { return stats_; }
    
    /**
     * @brief Reset statistics
     */
    void ResetStats() { stats_.Reset(); }
    
    /**
     * @brief Get number of active particles
     */
    size_t GetActiveParticleCount() const { return active_particles_.size(); }
    
    /**
     * @brief Get number of active emitters
     */
    size_t GetActiveEmitterCount() const { return active_emitters_.size(); }
    
    /**
     * @brief Get maximum particle capacity
     */
    size_t GetMaxParticles() const { return config_.max_particles; }
    
    /**
     * @brief Check if particle pool is full
     */
    bool IsPoolFull() const;

private:
    /**
     * @brief Callback for particle emission from emitters
     */
    void OnParticleEmitted(const ParticleInitData& init_data);
    
    /**
     * @brief Update active particles
     */
    void UpdateParticles(float dt);
    
    /**
     * @brief Update active emitters
     */
    void UpdateEmitters(float dt);
    
    /**
     * @brief Remove dead particles
     */
    void RemoveDeadParticles();
    
    /**
     * @brief Convert particle to BatchVertex for rendering
     */
    Renderer::BatchVertex ParticleToVertex(const Particle& particle, size_t vertex_index);
    
    /**
     * @brief Check if particle is within culling bounds
     */
    bool IsParticleVisible(const Particle& particle) const;
    
    /**
     * @brief Sort particles by depth (if enabled)
     */
    void SortParticles();
    
    ParticleSystemConfig config_;
    ParticleSystemStats stats_;
    bool initialized_ = false;
    
    // Memory management
    std::unique_ptr<ObjectPool<Particle>> particle_pool_;
    
    // Active particles and emitters
    std::unordered_set<Particle*> active_particles_;
    std::vector<std::shared_ptr<ParticleEmitter>> active_emitters_;
    
    // Rendering cache
    std::vector<Particle*> render_cache_;  ///< Cache for particles to render
    
    // Timing for statistics
    std::chrono::high_resolution_clock::time_point last_update_time_;
    std::chrono::high_resolution_clock::time_point last_render_time_;
};

} // namespace Particles
} // namespace PyNovaGE