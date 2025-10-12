#pragma once

#include <scene/components.hpp>
#include <scene/scene.hpp>
#include <renderer/sprite_renderer.hpp>
#include <audio/audio.hpp>
#include <asset/audio_clip.hpp>
#include <particles/particle_emitter.hpp>
#include <unordered_map>

class Player : public PyNovaGE::Scene::Component {
public:
    explicit Player(PyNovaGE::Scene::EntityID entity_id);
    ~Player();

    void Initialize(PyNovaGE::Scene::Scene* scene);
    void OnUpdate(float deltaTime);
    void OnRender(PyNovaGE::Renderer::SpriteRenderer* renderer);
    
// Movement controls
    void MoveLeft();
    void MoveRight();
    void Jump();
    void Stop();

    // State queries
    bool IsGrounded() const;
    const PyNovaGE::Vector2<float>& GetPosition() const;
    PyNovaGE::Scene::EntityID GetEntityID() const { return entity_id_; }

private:
    void UpdateParticles();
    void PlayAudio(const std::string& soundName);

    // Cached scene pointer
    PyNovaGE::Scene::Scene* scene_ = nullptr;

    // Movement properties
    float move_speed_ = 200.0f;
    float jump_force_ = 300.0f;
    bool facing_right_ = true;

    // Audio clips
    std::unordered_map<std::string, std::shared_ptr<PyNovaGE::Audio::AudioClip>> sounds_;

    // Entity management
    PyNovaGE::Scene::EntityID entity_id_;
};
