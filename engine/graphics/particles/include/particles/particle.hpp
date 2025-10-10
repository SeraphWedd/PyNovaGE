#pragma once

#include <vectors/vector2.hpp>
#include <vectors/vector3.hpp>
#include <vectors/vector4.hpp>
#include <memory>

namespace PyNovaGE {

// Forward declarations
namespace Renderer {
    class Texture;
}

namespace Particles {

/**
 * @brief Core particle data structure
 * 
 * Optimized for cache efficiency and batch rendering.
 * Size is kept minimal for better memory usage with ObjectPool.
 */
struct Particle {
    // Transform
    Vector2f position{0.0f, 0.0f};      ///< Current position (x, y)
    Vector2f velocity{0.0f, 0.0f};      ///< Velocity vector (dx/dt, dy/dt)
    float rotation = 0.0f;              ///< Rotation in radians
    float angular_velocity = 0.0f;      ///< Angular velocity (rad/s)
    
    // Visual properties
    Vector2f size{1.0f, 1.0f};          ///< Particle size (width, height)
    Vector4f color{1.0f, 1.0f, 1.0f, 1.0f}; ///< RGBA color
    
    // Lifetime
    float lifetime = 1.0f;              ///< Total lifetime in seconds
    float age = 0.0f;                   ///< Current age in seconds
    
    // Physics
    Vector2f acceleration{0.0f, 0.0f};  ///< Acceleration vector
    float mass = 1.0f;                  ///< Mass for physics calculations
    float drag = 0.0f;                  ///< Air resistance coefficient
    
    // Animation
    float size_over_time = 1.0f;        ///< Size multiplier based on age
    Vector4f color_over_time{1.0f, 1.0f, 1.0f, 1.0f}; ///< Color multiplier based on age
    
    // Texture
    std::shared_ptr<Renderer::Texture> texture; ///< Particle texture
    Vector4f uv_rect{0.0f, 0.0f, 1.0f, 1.0f};  ///< UV rectangle (x, y, width, height)
    
    /**
     * @brief Check if particle is alive
     */
    bool IsAlive() const {
        return age < lifetime;
    }
    
    /**
     * @brief Get normalized age (0.0 to 1.0)
     */
    float GetNormalizedAge() const {
        return lifetime > 0.0f ? (age / lifetime) : 1.0f;
    }
    
    /**
     * @brief Update particle by delta time
     * @param dt Delta time in seconds
     */
    void Update(float dt);
    
    /**
     * @brief Reset particle to default state
     */
    void Reset();
    
    /**
     * @brief Apply force to particle
     * @param force Force vector to apply
     */
    void ApplyForce(const Vector2f& force);
};

/**
 * @brief Particle initialization data
 * 
 * Used by emitters to configure new particles.
 * Separate from Particle to avoid bloating the particle structure.
 */
struct ParticleInitData {
    // Initial transform
    Vector2f position{0.0f, 0.0f};
    Vector2f velocity{0.0f, 0.0f};
    float rotation = 0.0f;
    float angular_velocity = 0.0f;
    
    // Initial visual properties
    Vector2f size{1.0f, 1.0f};
    Vector4f color{1.0f, 1.0f, 1.0f, 1.0f};
    
    // Lifetime
    float lifetime = 1.0f;
    
    // Physics
    Vector2f acceleration{0.0f, 0.0f};
    float mass = 1.0f;
    float drag = 0.0f;
    
    // Animation curves (applied over lifetime)
    float size_over_time = 1.0f;
    Vector4f color_over_time{1.0f, 1.0f, 1.0f, 1.0f};
    
    // Texture
    std::shared_ptr<Renderer::Texture> texture;
    Vector4f uv_rect{0.0f, 0.0f, 1.0f, 1.0f};
};

} // namespace Particles
} // namespace PyNovaGE