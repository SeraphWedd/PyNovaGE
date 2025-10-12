#pragma once

#include <window/window.hpp>
#include <renderer/sprite_renderer.hpp>
#include <renderer/frame_buffer.hpp>
#include <renderer/screen_quad.hpp>
#include <physics/physics.hpp>
#include <scene/scene.hpp>
#include <audio/audio.hpp>
#include <particles/particle_system.hpp>
#include <window/input.hpp>
#include "player.hpp"
#include "platform.hpp"

class Game {
public:
    Game();
    ~Game();

    void run();

private:
    void initialize();
    void processInput();
    void update(float deltaTime);
    void render();
    void cleanUp();
    void createPlatforms(); // Add platform creation method

    // Window system lifetime guard (must outlive window and renderer)
    PyNovaGE::Window::WindowSystemGuard window_guard;

    // Window size
    int window_width = 800;
    int window_height = 600;
    
    // Virtual resolution for game rendering
    static const int virtual_width = 800;
    static const int virtual_height = 600;
    
    // Framebuffer for fixed resolution rendering
    PyNovaGE::Renderer::FrameBuffer* framebuffer;
    PyNovaGE::Renderer::ScreenQuad* screen_quad;

    // World boundaries (in world units)
    float world_width = 800.0f;
    float world_height = 600.0f;
    
    // Event callbacks
    void OnWindowResized(int width, int height);
    
    // Calculate orthographic projection that preserves world coordinates
    void UpdateProjection();

    // Window and rendering
    PyNovaGE::Window::Window* window;
    std::unique_ptr<PyNovaGE::Window::InputManager> input;
    PyNovaGE::Renderer::SpriteRenderer* renderer;
    
    // Scene management
    PyNovaGE::Scene::Scene* scene;
    
    // Physics
    PyNovaGE::Physics::PhysicsWorld* physicsWorld;
    
    // Audio
    PyNovaGE::Audio::AudioSystem* audioSystem;
    
    // Particle system
    PyNovaGE::Particles::ParticleSystem* particleSystem;
    
    // Game objects
    Player* player;
    std::vector<Platform*> platforms;
    
    // Game state
    bool isRunning;
    bool show_debug = false;
    float score;
};