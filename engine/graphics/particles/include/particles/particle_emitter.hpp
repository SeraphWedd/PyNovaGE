#pragma once

#include "particles/particle.hpp"
#include <vectors/vector2.hpp>
#include <vectors/vector4.hpp>
#include <functional>
#include <random>

namespace PyNovaGE {
namespace Particles {

/**
 * @brief Emission shape types
 */
enum class EmissionShape {
    Point,      ///< Emit from a single point
    Circle,     ///< Emit from circle perimeter
    Box,        ///< Emit from rectangular area
    Line        ///< Emit along a line
};

/**
 * @brief Emission burst configuration
 */
struct EmissionBurst {
    float time = 0.0f;          ///< Time when burst should occur
    int count = 10;             ///< Number of particles to emit
    float probability = 1.0f;   ///< Probability of burst occurring (0.0-1.0)
    
    EmissionBurst() = default;
    EmissionBurst(float t, int c, float p = 1.0f) 
        : time(t), count(c), probability(p) {}
};

/**
 * @brief Particle emitter configuration
 * 
 * Defines all properties for particle emission including rates,
 * initial values, randomization ranges, and animation curves.
 */
struct EmitterConfig {
    // Emission
    Vector2f position{0.0f, 0.0f};      ///< Emitter position
    float emission_rate = 10.0f;        ///< Particles per second
    bool auto_emit = true;              ///< Automatically emit particles
    bool looping = true;                ///< Loop emission
    float duration = 5.0f;              ///< Duration of emission cycle
    
    // Shape
    EmissionShape shape = EmissionShape::Point;
    Vector2f shape_data{1.0f, 1.0f};    ///< Shape-specific data (radius for circle, size for box/line)
    
    // Bursts
    std::vector<EmissionBurst> bursts;
    
    // Initial particle properties
    struct {
        Vector2f position_min{0.0f, 0.0f};
        Vector2f position_max{0.0f, 0.0f};
        
        Vector2f velocity_min{-1.0f, -1.0f};
        Vector2f velocity_max{1.0f, 1.0f};
        
        float rotation_min = 0.0f;
        float rotation_max = 0.0f;
        
        float angular_velocity_min = 0.0f;
        float angular_velocity_max = 0.0f;
        
        Vector2f size_min{1.0f, 1.0f};
        Vector2f size_max{1.0f, 1.0f};
        
        Vector4f color_min{1.0f, 1.0f, 1.0f, 1.0f};
        Vector4f color_max{1.0f, 1.0f, 1.0f, 1.0f};
        
        float lifetime_min = 1.0f;
        float lifetime_max = 1.0f;
        
        Vector2f acceleration_min{0.0f, 0.0f};
        Vector2f acceleration_max{0.0f, 0.0f};
        
        float mass_min = 1.0f;
        float mass_max = 1.0f;
        
        float drag_min = 0.0f;
        float drag_max = 0.0f;
    } initial;
    
    // Animation over lifetime
    struct {
        float size_start = 1.0f;
        float size_end = 1.0f;
        
        Vector4f color_start{1.0f, 1.0f, 1.0f, 1.0f};
        Vector4f color_end{1.0f, 1.0f, 1.0f, 1.0f};
    } animation;
    
    // Global forces
    Vector2f gravity{0.0f, -9.81f};
    
    // Texture
    std::shared_ptr<Renderer::Texture> texture;
    Vector4f uv_rect{0.0f, 0.0f, 1.0f, 1.0f};
};

/**
 * @brief Particle emitter class
 * 
 * Handles particle generation based on configuration.
 * Uses callback pattern to decouple from particle system.
 */
class ParticleEmitter {
public:
    using EmitCallback = std::function<void(const ParticleInitData&)>;
    
    /**
     * @brief Constructor
     * @param config Emitter configuration
     */
    explicit ParticleEmitter(const EmitterConfig& config = EmitterConfig{});
    
    /**
     * @brief Set emitter configuration
     */
    void SetConfig(const EmitterConfig& config) { config_ = config; }
    
    /**
     * @brief Get emitter configuration
     */
    const EmitterConfig& GetConfig() const { return config_; }
    
    /**
     * @brief Set emission callback
     * @param callback Function called when particles need to be emitted
     */
    void SetEmitCallback(EmitCallback callback) { emit_callback_ = callback; }
    
    /**
     * @brief Start emission
     */
    void Start();
    
    /**
     * @brief Stop emission
     */
    void Stop();
    
    /**
     * @brief Pause/resume emission
     */
    void SetPaused(bool paused) { paused_ = paused; }
    
    /**
     * @brief Check if emitter is active
     */
    bool IsActive() const { return active_ && !paused_; }
    
    /**
     * @brief Check if emitter is looping
     */
    bool IsLooping() const { return config_.looping; }
    
    /**
     * @brief Update emitter
     * @param dt Delta time in seconds
     */
    void Update(float dt);
    
    /**
     * @brief Manually emit a single particle
     */
    void EmitParticle();
    
    /**
     * @brief Manually emit multiple particles
     * @param count Number of particles to emit
     */
    void EmitBurst(int count);
    
    /**
     * @brief Set emitter position
     */
    void SetPosition(const Vector2f& position) { config_.position = position; }
    
    /**
     * @brief Get emitter position
     */
    const Vector2f& GetPosition() const { return config_.position; }
    
    /**
     * @brief Reset emitter to initial state
     */
    void Reset();
    
    /**
     * @brief Get current emission time
     */
    float GetEmissionTime() const { return emission_time_; }

private:
    /**
     * @brief Generate position within emission shape
     */
    Vector2f GeneratePosition();
    
    /**
     * @brief Generate random value between min and max
     */
    template<typename T>
    T RandomRange(const T& min, const T& max);
    
    /**
     * @brief Check and process bursts
     */
    void ProcessBursts(float dt);
    
    /**
     * @brief Create particle initialization data
     */
    ParticleInitData CreateParticleData();
    
    EmitterConfig config_;
    EmitCallback emit_callback_;
    
    // State
    bool active_ = false;
    bool paused_ = false;
    float emission_time_ = 0.0f;        ///< Total emission time
    float next_emission_time_ = 0.0f;   ///< Time until next particle emission
    
    // Burst tracking
    std::vector<bool> burst_triggered_;  ///< Track which bursts have been triggered
    
    // Random number generation
    std::mt19937 rng_;
    std::uniform_real_distribution<float> unit_dist_{0.0f, 1.0f};
};

} // namespace Particles
} // namespace PyNovaGE