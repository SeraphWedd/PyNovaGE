#include "particles/particle_system.hpp"
#include <renderer/sprite_renderer.hpp>
#include <algorithm>
#include <cmath>

namespace PyNovaGE {
namespace Particles {

ParticleSystem::ParticleSystem(const ParticleSystemConfig& config)
    : config_(config) {
    
    // Reserve space for render cache based on max particles
    render_cache_.reserve(config_.max_particles);
}

ParticleSystem::~ParticleSystem() {
    Shutdown();
}

bool ParticleSystem::Initialize() {
    if (initialized_) {
        return true;
    }
    
    try {
        // Create particle pool
        particle_pool_ = std::make_unique<ObjectPool<Particle>>(config_.max_particles);
        
        // Initialize statistics
        stats_.pool_size = config_.max_particles;
        stats_.pool_free = config_.max_particles;
        
        initialized_ = true;
        return true;
    }
    catch (const std::exception&) {
        Shutdown();
        return false;
    }
}

void ParticleSystem::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    // Clear all emitters and particles
    ClearEmitters();
    ClearParticles();
    
    // Reset pools
    particle_pool_.reset();
    
    initialized_ = false;
}

void ParticleSystem::Update(float dt) {
    if (!initialized_) {
        return;
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Update emitters
    UpdateEmitters(dt);
    
    // Update particles
    UpdateParticles(dt);
    
    // Remove dead particles
    RemoveDeadParticles();
    
    // Update statistics
    stats_.active_particles = active_particles_.size();
    stats_.active_emitters = active_emitters_.size();
    stats_.pool_free = particle_pool_->getFreeCount();
    
    if (stats_.active_particles > stats_.peak_active_particles) {
        stats_.peak_active_particles = stats_.active_particles;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    stats_.update_time_ms = duration.count() / 1000.0f;
}

void ParticleSystem::Render(Renderer::BatchRenderer& batch_renderer) {
    if (!initialized_ || active_particles_.empty()) {
        return;
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Build render cache with visible particles
    render_cache_.clear();
    
    for (Particle* particle : active_particles_) {
        if (particle && particle->IsAlive()) {
            if (!config_.enable_culling || IsParticleVisible(*particle)) {
                render_cache_.push_back(particle);
            }
        }
    }
    
    // Sort particles if enabled
    if (config_.enable_sorting) {
        SortParticles();
    }
    
    // Convert particles to sprites and render through batch system
    std::vector<Renderer::Sprite> sprites;
    sprites.reserve(render_cache_.size());
    
    for (const Particle* particle : render_cache_) {
        if (particle && particle->IsAlive()) {
            Renderer::Sprite sprite;
            
            // Transform
            sprite.position = Vector2f{particle->position.x, particle->position.y};
            sprite.rotation = particle->rotation;
            sprite.scale = Vector2f{
                particle->size.x * particle->size_over_time,
                particle->size.y * particle->size_over_time
            };
            sprite.size = sprite.scale;
            
            // Visual properties
            float t = particle->GetNormalizedAge();
            Vector4f current_color = Vector4f{
                particle->color.x * (particle->color_over_time.x * t + (1.0f - t)),
                particle->color.y * (particle->color_over_time.y * t + (1.0f - t)),
                particle->color.z * (particle->color_over_time.z * t + (1.0f - t)),
                particle->color.w * (particle->color_over_time.w * t + (1.0f - t))
            };
            sprite.color = current_color;
            
            // Texture
            sprite.texture = particle->texture;
            if (particle->texture) {
                sprite.SetTextureRegionNormalized(
                    particle->uv_rect.x, 
                    particle->uv_rect.y,
                    particle->uv_rect.x + particle->uv_rect.z, 
                    particle->uv_rect.y + particle->uv_rect.w
                );
            }
            
            sprites.push_back(std::move(sprite));
        }
    }
    
    // Render through batch system
    if (!sprites.empty()) {
        batch_renderer.RenderSprites(sprites);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    stats_.render_time_ms = duration.count() / 1000.0f;
}

std::shared_ptr<ParticleEmitter> ParticleSystem::CreateEmitter(const EmitterConfig& config) {
    auto emitter = std::make_shared<ParticleEmitter>(config);
    
    // Set the emission callback to this system
    emitter->SetEmitCallback([this](const ParticleInitData& data) {
        OnParticleEmitted(data);
    });
    
    active_emitters_.push_back(emitter);
    return emitter;
}

void ParticleSystem::RemoveEmitter(std::shared_ptr<ParticleEmitter> emitter) {
    auto it = std::find(active_emitters_.begin(), active_emitters_.end(), emitter);
    if (it != active_emitters_.end()) {
        active_emitters_.erase(it);
    }
}

void ParticleSystem::ClearEmitters() {
    active_emitters_.clear();
}

Particle* ParticleSystem::SpawnParticle(const ParticleInitData& init_data) {
    if (!initialized_ || !particle_pool_) {
        return nullptr;
    }
    
    // Acquire particle from pool
    Particle* particle = particle_pool_->acquire();
    if (!particle) {
        return nullptr; // Pool full
    }
    
    // Initialize particle from data
    particle->position = init_data.position;
    particle->velocity = init_data.velocity;
    particle->rotation = init_data.rotation;
    particle->angular_velocity = init_data.angular_velocity;
    particle->size = init_data.size;
    particle->color = init_data.color;
    particle->lifetime = init_data.lifetime;
    particle->age = 0.0f;
    particle->acceleration = init_data.acceleration;
    particle->mass = init_data.mass;
    particle->drag = init_data.drag;
    particle->size_over_time = init_data.size_over_time;
    particle->color_over_time = init_data.color_over_time;
    particle->texture = init_data.texture;
    particle->uv_rect = init_data.uv_rect;
    
    // Add to active particles
    active_particles_.insert(particle);
    
    // Update statistics
    stats_.total_particles_spawned++;
    
    return particle;
}

void ParticleSystem::DestroyParticle(Particle* particle) {
    if (!particle || !particle_pool_->ownsPointer(particle)) {
        return;
    }
    
    // Remove from active set
    active_particles_.erase(particle);
    
    // Return to pool
    particle_pool_->release(particle);
}

void ParticleSystem::ClearParticles() {
    // Destroy all active particles
    for (Particle* particle : active_particles_) {
        if (particle && particle_pool_->ownsPointer(particle)) {
            particle_pool_->release(particle);
        }
    }
    
    active_particles_.clear();
}

void ParticleSystem::ApplyGlobalForce(const Vector2f& force) {
    for (Particle* particle : active_particles_) {
        if (particle && particle->IsAlive()) {
            particle->ApplyForce(force);
        }
    }
}

void ParticleSystem::ApplyRadialForce(const Vector2f& position, float radius, const Vector2f& force, bool falloff) {
    float radius_sq = radius * radius;
    
    for (Particle* particle : active_particles_) {
        if (!particle || !particle->IsAlive()) {
            continue;
        }
        
        Vector2f diff = particle->position - position;
        float dist_sq = diff.x * diff.x + diff.y * diff.y;
        
        if (dist_sq <= radius_sq) {
            Vector2f applied_force = force;
            
            if (falloff && dist_sq > 0.0f) {
                float falloff_factor = 1.0f - (std::sqrt(dist_sq) / radius);
                applied_force = applied_force * falloff_factor;
            }
            
            particle->ApplyForce(applied_force);
        }
    }
}

void ParticleSystem::SetConfig(const ParticleSystemConfig& config) {
    if (config.max_particles != config_.max_particles) {
        // Need to recreate pool if size changed
        bool was_initialized = initialized_;
        if (was_initialized) {
            Shutdown();
        }
        
        config_ = config;
        
        if (was_initialized) {
            Initialize();
        }
    } else {
        config_ = config;
    }
    
    render_cache_.reserve(config_.max_particles);
}

bool ParticleSystem::IsPoolFull() const {
    return particle_pool_ ? (particle_pool_->getFreeCount() == 0) : true;
}

void ParticleSystem::OnParticleEmitted(const ParticleInitData& init_data) {
    SpawnParticle(init_data);
}

void ParticleSystem::UpdateParticles(float dt) {
    for (Particle* particle : active_particles_) {
        if (particle && particle->IsAlive()) {
            particle->Update(dt);
        }
    }
}

void ParticleSystem::UpdateEmitters(float dt) {
    for (auto& emitter : active_emitters_) {
        if (emitter) {
            emitter->Update(dt);
        }
    }
}

void ParticleSystem::RemoveDeadParticles() {
    std::vector<Particle*> to_remove;
    to_remove.reserve(active_particles_.size() / 10); // Estimate
    
    for (Particle* particle : active_particles_) {
        if (particle && !particle->IsAlive()) {
            to_remove.push_back(particle);
        }
    }
    
    for (Particle* particle : to_remove) {
        DestroyParticle(particle);
    }
}

Renderer::BatchVertex ParticleSystem::ParticleToVertex(const Particle& particle, size_t vertex_index) {
    // This method is for future direct batch vertex generation if needed
    // Currently we use the Sprite conversion approach for simplicity
    
    Renderer::BatchVertex vertex;
    vertex.position = Vector3f{particle.position.x, particle.position.y, 0.0f};
    
    // Calculate texture coordinates based on vertex index (0-3 for quad)
    switch (vertex_index) {
        case 0: vertex.texCoords = Vector2f{particle.uv_rect.x, particle.uv_rect.y}; break;
        case 1: vertex.texCoords = Vector2f{particle.uv_rect.x + particle.uv_rect.z, particle.uv_rect.y}; break;
        case 2: vertex.texCoords = Vector2f{particle.uv_rect.x + particle.uv_rect.z, particle.uv_rect.y + particle.uv_rect.w}; break;
        case 3: vertex.texCoords = Vector2f{particle.uv_rect.x, particle.uv_rect.y + particle.uv_rect.w}; break;
    }
    
    // Calculate current color with animation
    float t = particle.GetNormalizedAge();
    vertex.color = Vector4f{
        particle.color.x * (particle.color_over_time.x * t + (1.0f - t)),
        particle.color.y * (particle.color_over_time.y * t + (1.0f - t)),
        particle.color.z * (particle.color_over_time.z * t + (1.0f - t)),
        particle.color.w * (particle.color_over_time.w * t + (1.0f - t))
    };
    
    vertex.textureIndex = 0.0f; // Will be set by batch renderer
    
    return vertex;
}

bool ParticleSystem::IsParticleVisible(const Particle& particle) const {
    if (!config_.enable_culling) {
        return true;
    }
    
    // Simple AABB culling
    Vector2f half_size = particle.size * 0.5f * particle.size_over_time;
    
    return !(particle.position.x + half_size.x < config_.culling_bounds.x ||
             particle.position.x - half_size.x > config_.culling_bounds.x + config_.culling_bounds.z ||
             particle.position.y + half_size.y < config_.culling_bounds.y ||
             particle.position.y - half_size.y > config_.culling_bounds.y + config_.culling_bounds.w);
}

void ParticleSystem::SortParticles() {
    if (!config_.enable_sorting || render_cache_.empty()) {
        return;
    }
    
    // Sort by Y position (back to front)
    std::sort(render_cache_.begin(), render_cache_.end(), 
        [](const Particle* a, const Particle* b) {
            return a->position.y > b->position.y;
        });
}

} // namespace Particles
} // namespace PyNovaGE