#include "particles/particle_emitter.hpp"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace PyNovaGE {
namespace Particles {

ParticleEmitter::ParticleEmitter(const EmitterConfig& config)
    : config_(config)
    , rng_(std::random_device{}()) {
    
    // Initialize burst tracking
    burst_triggered_.resize(config_.bursts.size(), false);
}

void ParticleEmitter::Start() {
    active_ = true;
    paused_ = false;
    
    // Reset emission timing
    if (config_.emission_rate > 0.0f) {
        next_emission_time_ = 1.0f / config_.emission_rate;
    }
}

void ParticleEmitter::Stop() {
    active_ = false;
    paused_ = false;
}

void ParticleEmitter::Update(float dt) {
    if (!IsActive()) {
        return;
    }
    
    emission_time_ += dt;
    
    // Handle continuous emission
    if (config_.auto_emit && config_.emission_rate > 0.0f) {
        next_emission_time_ -= dt;
        
        while (next_emission_time_ <= 0.0f) {
            EmitParticle();
            next_emission_time_ += 1.0f / config_.emission_rate;
        }
    }
    
    // Handle bursts
    ProcessBursts(dt);
    
    // Handle duration and looping
    if (!config_.looping && emission_time_ >= config_.duration) {
        Stop();
    } else if (config_.looping && emission_time_ >= config_.duration) {
        Reset();
    }
}

void ParticleEmitter::EmitParticle() {
    if (emit_callback_) {
        ParticleInitData init_data = CreateParticleData();
        emit_callback_(init_data);
    }
}

void ParticleEmitter::EmitBurst(int count) {
    for (int i = 0; i < count; ++i) {
        EmitParticle();
    }
}

void ParticleEmitter::Reset() {
    emission_time_ = 0.0f;
    next_emission_time_ = config_.emission_rate > 0.0f ? (1.0f / config_.emission_rate) : 0.0f;
    
    // Reset burst tracking
    std::fill(burst_triggered_.begin(), burst_triggered_.end(), false);
}

Vector2f ParticleEmitter::GeneratePosition() {
    Vector2f base_pos = config_.position;
    
    switch (config_.shape) {
        case EmissionShape::Point:
            return base_pos;
            
        case EmissionShape::Circle: {
            float radius = config_.shape_data.x;
            float angle = unit_dist_(rng_) * 2.0f * static_cast<float>(M_PI);
            float r = unit_dist_(rng_) * radius;
            
            return base_pos + Vector2f{
                r * std::cos(angle),
                r * std::sin(angle)
            };
        }
        
        case EmissionShape::Box: {
            Vector2f size = config_.shape_data;
            Vector2f half_size = size * 0.5f;
            
            return base_pos + Vector2f{
                (unit_dist_(rng_) - 0.5f) * size.x,
                (unit_dist_(rng_) - 0.5f) * size.y
            };
        }
        
        case EmissionShape::Line: {
            float length = config_.shape_data.x;
            float angle = config_.shape_data.y; // Line angle in radians
            float t = (unit_dist_(rng_) - 0.5f) * length;
            
            return base_pos + Vector2f{
                t * std::cos(angle),
                t * std::sin(angle)
            };
        }
        
        default:
            return base_pos;
    }
}

template<>
float ParticleEmitter::RandomRange(const float& min, const float& max) {
    if (min >= max) return min;
    return min + unit_dist_(rng_) * (max - min);
}

template<>
Vector2f ParticleEmitter::RandomRange(const Vector2f& min, const Vector2f& max) {
    return Vector2f{
        RandomRange(min.x, max.x),
        RandomRange(min.y, max.y)
    };
}

template<>
Vector4f ParticleEmitter::RandomRange(const Vector4f& min, const Vector4f& max) {
    return Vector4f{
        RandomRange(min.x, max.x),
        RandomRange(min.y, max.y),
        RandomRange(min.z, max.z),
        RandomRange(min.w, max.w)
    };
}

void ParticleEmitter::ProcessBursts(float /* dt */) {
    for (size_t i = 0; i < config_.bursts.size(); ++i) {
        const auto& burst = config_.bursts[i];
        
        if (!burst_triggered_[i] && emission_time_ >= burst.time) {
            // Check probability
            if (unit_dist_(rng_) <= burst.probability) {
                EmitBurst(burst.count);
            }
            burst_triggered_[i] = true;
        }
    }
}

ParticleInitData ParticleEmitter::CreateParticleData() {
    ParticleInitData data;
    
    // Position (shape + offset randomization)
    Vector2f shape_pos = GeneratePosition();
    Vector2f pos_offset = RandomRange(config_.initial.position_min, config_.initial.position_max);
    data.position = shape_pos + pos_offset;
    
    // Velocity
    data.velocity = RandomRange(config_.initial.velocity_min, config_.initial.velocity_max);
    
    // Rotation
    data.rotation = RandomRange(config_.initial.rotation_min, config_.initial.rotation_max);
    data.angular_velocity = RandomRange(config_.initial.angular_velocity_min, config_.initial.angular_velocity_max);
    
    // Visual properties
    data.size = RandomRange(config_.initial.size_min, config_.initial.size_max);
    data.color = RandomRange(config_.initial.color_min, config_.initial.color_max);
    
    // Lifetime
    data.lifetime = RandomRange(config_.initial.lifetime_min, config_.initial.lifetime_max);
    
    // Physics
    data.acceleration = RandomRange(config_.initial.acceleration_min, config_.initial.acceleration_max);
    data.acceleration = data.acceleration + config_.gravity; // Add global gravity
    
    data.mass = RandomRange(config_.initial.mass_min, config_.initial.mass_max);
    data.drag = RandomRange(config_.initial.drag_min, config_.initial.drag_max);
    
    // Animation curves
    data.size_over_time = config_.animation.size_end / config_.animation.size_start;
    data.color_over_time = Vector4f{
        config_.animation.color_end.x / config_.animation.color_start.x,
        config_.animation.color_end.y / config_.animation.color_start.y,
        config_.animation.color_end.z / config_.animation.color_start.z,
        config_.animation.color_end.w / config_.animation.color_start.w
    };
    
    // Texture
    data.texture = config_.texture;
    data.uv_rect = config_.uv_rect;
    
    return data;
}

} // namespace Particles
} // namespace PyNovaGE