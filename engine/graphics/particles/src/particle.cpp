#include "particles/particle.hpp"
#include <algorithm>
#include <cmath>

namespace PyNovaGE {
namespace Particles {

void Particle::Update(float dt) {
    if (!IsAlive()) {
        return;
    }
    
    // Update age
    age += dt;
    
    // Apply drag to velocity
    if (drag > 0.0f) {
        float drag_factor = 1.0f - (drag * dt);
        drag_factor = std::max(0.0f, drag_factor); // Prevent negative velocity
        velocity = velocity * drag_factor;
    }
    
    // Update velocity from acceleration
    velocity = velocity + (acceleration * dt);
    
    // Update position from velocity
    position = position + (velocity * dt);
    
    // Update rotation from angular velocity
    rotation += angular_velocity * dt;
    
    // Animation properties are applied during rendering
    // (size_over_time and color_over_time are used by the renderer)
    
    // Apply color animation (linear interpolation for now)
    // color_over_time acts as a multiplier to the base color
    // This allows for effects like fade-out (alpha from 1.0 to 0.0)
}

void Particle::Reset() {
    position = Vector2f{0.0f, 0.0f};
    velocity = Vector2f{0.0f, 0.0f};
    rotation = 0.0f;
    angular_velocity = 0.0f;
    
    size = Vector2f{1.0f, 1.0f};
    color = Vector4f{1.0f, 1.0f, 1.0f, 1.0f};
    
    lifetime = 1.0f;
    age = 0.0f;
    
    acceleration = Vector2f{0.0f, 0.0f};
    mass = 1.0f;
    drag = 0.0f;
    
    size_over_time = 1.0f;
    color_over_time = Vector4f{1.0f, 1.0f, 1.0f, 1.0f};
    
    texture.reset();
    uv_rect = Vector4f{0.0f, 0.0f, 1.0f, 1.0f};
}

void Particle::ApplyForce(const Vector2f& force) {
    if (mass > 0.0f) {
        // F = ma, therefore a = F/m
        Vector2f accel = force * (1.0f / mass);
        acceleration = acceleration + accel;
    }
}

} // namespace Particles
} // namespace PyNovaGE