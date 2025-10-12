#include "game.hpp"
#include <chrono>
#include <algorithm>
#include <glad/gl.h>
#include <iostream>
#include <vector>
#include <renderer/renderer.hpp>
#include <scene/components.hpp>

Game::Game()
    : window_guard() // Initialize the window system guard first
    , window(nullptr)
    , renderer(nullptr)
    , scene(nullptr)
    , physicsWorld(nullptr)
    , audioSystem(nullptr)
    , particleSystem(nullptr)
    , player(nullptr)
    , isRunning(false)
    , score(0.0f)
{
    initialize();
}

Game::~Game() {
    cleanUp();
}

void Game::initialize() {
    std::cout << "Starting game initialization..." << std::endl;
    
    // Initialize window system via member window_guard (RAII)
    std::cout << "Creating window..." << std::endl;
    window = new PyNovaGE::Window::Window({"PyNovaGE Platformer", window_width, window_height, false, true, true, 0, true});

    // Set window resize callback
    window->SetEventCallback([this](const PyNovaGE::Window::WindowEvent& event) {
        if (event.type == PyNovaGE::Window::WindowEventType::Resize) {
            OnWindowResized(event.width, event.height);
        }
    });
    std::cout << "Window created successfully" << std::endl;

    // Create input manager for this window
    input = std::make_unique<PyNovaGE::Window::InputManager>(window->GetNativeWindow());

    // Initialize renderer
    std::cout << "Initializing renderer..." << std::endl;
    PyNovaGE::Renderer::RendererConfig renderer_config{};
    renderer_config.enable_vsync = true;
    renderer_config.enable_depth_test = true;
    renderer_config.enable_blend = true;
    if (!PyNovaGE::Renderer::Renderer::Initialize(renderer_config)) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
        throw std::runtime_error("Renderer initialization failed");
    }
    std::cout << "Renderer initialized successfully" << std::endl;

    // Get sprite renderer from the engine
    std::cout << "Getting sprite renderer..." << std::endl;
    renderer = PyNovaGE::Renderer::Renderer::GetSpriteRenderer();
    if (!renderer) {
        std::cerr << "Failed to get sprite renderer" << std::endl;
        throw std::runtime_error("Sprite renderer not available");
    }
    std::cout << "Got sprite renderer" << std::endl;

    // Create fixed resolution framebuffer
    framebuffer = new PyNovaGE::Renderer::FrameBuffer(virtual_width, virtual_height);
    
    // Create screen quad for rendering framebuffer to window
    screen_quad = new PyNovaGE::Renderer::ScreenQuad();
    screen_quad->Initialize();
    
    // Set initial viewport based on window size
    OnWindowResized(window_width, window_height);
    // Set initial projection scale for sprite renderer
    UpdateProjection();

    // Initialize physics with default gravity
    std::cout << "Creating physics world..." << std::endl;
    physicsWorld = new PyNovaGE::Physics::PhysicsWorld(PyNovaGE::Physics::PhysicsConfig{});
    std::cout << "Physics world created" << std::endl;

    // Initialize scene
    std::cout << "Creating scene..." << std::endl;
    scene = new PyNovaGE::Scene::Scene();
    std::cout << "Scene created" << std::endl;

    // Initialize particle system (optional for now)
    std::cout << "Creating particle system..." << std::endl;
    particleSystem = new PyNovaGE::Particles::ParticleSystem();
    if (!particleSystem->Initialize()) {
        std::cerr << "Failed to initialize particle system" << std::endl;
        throw std::runtime_error("Particle system initialization failed");
    }
    std::cout << "Particle system initialized" << std::endl;

    // Create player entity and initialize components via Player wrapper
    std::cout << "Creating player..." << std::endl;
    auto playerEntity = scene->CreateEntity("Player");
    player = new Player(playerEntity);
    player->Initialize(scene);
    std::cout << "Player created and initialized" << std::endl;

    // Create platforms
    createPlatforms();

    // Register rigid bodies with physics world
    if (auto* pBodyComp = scene->GetComponent<PyNovaGE::Scene::RigidBody2DComponent>(player->GetEntityID())) {
        if (pBodyComp->body) physicsWorld->addBody(pBodyComp->body);
    }
    for (auto* plat : platforms) {
        if (auto* rb = scene->GetComponent<PyNovaGE::Scene::RigidBody2DComponent>(plat->GetEntityID())) {
            if (rb->body) physicsWorld->addBody(rb->body);
        }
    }

    isRunning = true;
    std::cout << "Game initialization complete" << std::endl;
}

void Game::createPlatforms() {
    // Ground platform (centered at bottom of screen)
    auto groundEntity = scene->CreateEntity("Ground");
    auto* ground = new Platform(groundEntity, PyNovaGE::Vector2<float>(virtual_width / 2.0f, 20.0f), PyNovaGE::Vector2<float>(virtual_width * 0.8f, 40.0f));
    ground->Initialize(scene);
    platforms.push_back(ground);

    // Floating platforms at various heights
    auto e1 = scene->CreateEntity("Plat1");
    auto* p1 = new Platform(e1, PyNovaGE::Vector2<float>(virtual_width * 0.25f, virtual_height * 0.33f), PyNovaGE::Vector2<float>(100.0f, 20.0f));
    p1->Initialize(scene);
    platforms.push_back(p1);

    auto e2 = scene->CreateEntity("Plat2");
    auto* p2 = new Platform(e2, PyNovaGE::Vector2<float>(virtual_width * 0.5f, virtual_height * 0.5f), PyNovaGE::Vector2<float>(100.0f, 20.0f));
    p2->Initialize(scene);
    platforms.push_back(p2);

    auto e3 = scene->CreateEntity("Plat3");
    auto* p3 = new Platform(e3, PyNovaGE::Vector2<float>(virtual_width * 0.75f, virtual_height * 0.66f), PyNovaGE::Vector2<float>(100.0f, 20.0f));
    p3->Initialize(scene);
    platforms.push_back(p3);
}

void Game::run() {
    std::cout << "Starting game loop..." << std::endl;
    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    int frame_count = 0;

    while (isRunning && !window->ShouldClose()) {
        // Calculate delta time
        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentFrameTime - lastFrameTime).count();
        lastFrameTime = currentFrameTime;

        // Update input state
        input->Update();

        processInput();
        update(deltaTime);
        render();

        // Every 60 frames, compute hash of frame buffer
        if (frame_count % 60 == 0) {
            // Get current framebuffer size
            int width = window_width, height = window_height;
            std::vector<unsigned char> pixels(width * height * 4);
            
            // Read frame buffer
            PyNovaGE::Renderer::Renderer::ReadPixels(0, 0, width, height, pixels.data());
            
            std::cout << "Reading frame buffer: " << width << "x" << height << std::endl;
            
            // Compute simple hash of pixels
            unsigned int hash = 0;
            for (size_t i = 0; i < pixels.size(); i++) {
                hash = (hash * 31 + pixels[i]) & 0xFFFFFFFF;
            }
            
            std::cout << "Frame " << frame_count << " buffer hash: 0x" << std::hex << hash << std::dec << std::endl;
        }
        
        frame_count++;
        
        window->SwapBuffers();
        window->PollEvents();
    }
}

void Game::UpdateProjection() {
    // Calculate aspect ratios
    float window_aspect = static_cast<float>(window_width) / static_cast<float>(window_height);
    float world_aspect = world_width / world_height;
    
    // Calculate scale factors to maintain world coordinates regardless of window shape
    float scale_x, scale_y;
    if (window_aspect > world_aspect) {
        // Window is wider than world - fit to height and center horizontally
        scale_y = 2.0f / world_height;
        scale_x = scale_y;  // Same scale for both axes
    } else {
        // Window is taller than world - fit to width and center vertically
        scale_x = 2.0f / world_width;
        scale_y = scale_x;  // Same scale for both axes
    }
    
    // Apply aspect correction to maintain square pixels
    if (window_aspect > world_aspect) {
        // Correct for wider window
        scale_x /= window_aspect;
        scale_y /= world_aspect;
    } else {
        // Correct for taller window
        scale_x /= window_aspect;
        scale_y /= world_aspect;
    }
    
    // Set projection scale in renderer (will be picked up by sprite renderer)
    PyNovaGE::Vector2f scale(scale_x, scale_y);
    PyNovaGE::Renderer::Renderer::SetProjectionScale(scale);
}

void Game::OnWindowResized(int width, int height) {
    window_width = width;
    window_height = height;
    
    // Calculate letterbox/pillarbox dimensions
    float window_aspect = static_cast<float>(width) / static_cast<float>(height);
    float target_aspect = static_cast<float>(virtual_width) / static_cast<float>(virtual_height);
    
    int viewport_x = 0;
    int viewport_y = 0;
    int viewport_width = width;
    int viewport_height = height;
    
    if (window_aspect > target_aspect) {
        // Window is wider - letterbox left/right
        viewport_width = static_cast<int>(height * target_aspect);
        viewport_x = (width - viewport_width) / 2;
    } else {
        // Window is taller - letterbox top/bottom
        viewport_height = static_cast<int>(width / target_aspect);
        viewport_y = (height - viewport_height) / 2;
    }
    
    // Set viewport for scaled rendering
    PyNovaGE::Renderer::Renderer::SetViewport(viewport_x, viewport_y, viewport_width, viewport_height);
}

void Game::processInput() {
    // Get input state from window
    if (!player) return;

    // Horizontal movement
    if (input->IsKeyPressed(PyNovaGE::Window::Key::Left) || input->IsKeyPressed(PyNovaGE::Window::Key::A)) {
        player->MoveLeft();
    } else if (input->IsKeyPressed(PyNovaGE::Window::Key::Right) || input->IsKeyPressed(PyNovaGE::Window::Key::D)) {
        player->MoveRight();
    } else {
        player->Stop();
    }

    // Jump (space to jump; use edge trigger to avoid continuous reapply)
    if (input->IsKeyJustPressed(PyNovaGE::Window::Key::Space)) {
        player->Jump();
    }

    // ESC to quit
    if (input->IsKeyJustPressed(PyNovaGE::Window::Key::Escape)) {
        window->SetShouldClose(true);
    }
}

void Game::update(float deltaTime) {
    // Update physics first
    physicsWorld->step(deltaTime);

    // Update scene entities
    scene->Update(deltaTime);

    // Update particle system
    particleSystem->Update(deltaTime);
}

void Game::render() {
    // Bind virtual resolution framebuffer
    framebuffer->Bind();
    
    // Clear virtual resolution framebuffer with nice blue background
    PyNovaGE::Vector4f clear_color(0.2f, 0.3f, 0.8f, 1.0f);
    PyNovaGE::Renderer::Renderer::Clear(clear_color);
    
    // Update viewport for virtual resolution
    PyNovaGE::Renderer::Renderer::SetViewport(0, 0, virtual_width, virtual_height);
    
    // Begin rendering frame
    PyNovaGE::Renderer::Renderer::BeginFrame();
    
    // Render platforms
    for (const auto& platform : platforms) {
        if (auto* sprite_comp = scene->GetComponent<PyNovaGE::Scene::SpriteComponent>(platform->GetEntityID())) {
            PyNovaGE::Renderer::Sprite sprite;
            // Copy sprite component data
            sprite.texture = sprite_comp->texture;
            sprite.color = sprite_comp->color;
            sprite.size = sprite_comp->size;
            // Set origin to center (0.5, 0.5) if not specified
            sprite.origin = sprite_comp->pivot.x == 0.0f && sprite_comp->pivot.y == 0.0f ?
                PyNovaGE::Vector2<float>(0.5f, 0.5f) : sprite_comp->pivot;
            
            // Copy transform data
            if (auto* transform = scene->GetComponent<PyNovaGE::Scene::Transform2DComponent>(platform->GetEntityID())) {
                sprite.position = transform->GetPosition();
                sprite.scale = transform->GetScale();
                sprite.rotation = transform->GetRotation();
            } else {
                sprite.scale = PyNovaGE::Vector2<float>(1.0f, 1.0f);
                sprite.rotation = 0.0f;
            }
            
            renderer->RenderSprite(sprite);
        }
    }
    
    // Render player
    if (player) {
        if (auto* sprite_comp = scene->GetComponent<PyNovaGE::Scene::SpriteComponent>(player->GetEntityID())) {
            PyNovaGE::Renderer::Sprite sprite;
            // Copy sprite component data
            sprite.texture = sprite_comp->texture;
            sprite.color = sprite_comp->color;
            sprite.size = sprite_comp->size;
            // Set origin to center (0.5, 0.5) if not specified
            sprite.origin = sprite_comp->pivot.x == 0.0f && sprite_comp->pivot.y == 0.0f ?
                PyNovaGE::Vector2<float>(0.5f, 0.5f) : sprite_comp->pivot;
            
            // Copy transform data
            if (auto* transform = scene->GetComponent<PyNovaGE::Scene::Transform2DComponent>(player->GetEntityID())) {
                sprite.position = transform->GetPosition();
                sprite.scale = transform->GetScale();
                sprite.rotation = transform->GetRotation();
            } else {
                sprite.scale = PyNovaGE::Vector2<float>(1.0f, 1.0f);
                sprite.rotation = 0.0f;
            }
            
            renderer->RenderSprite(sprite);
        }
    }
    
    // Render particles
    if (particleSystem) {
        // TODO: Add particle rendering once system supports it
    }
    
    // End rendering frame
    PyNovaGE::Renderer::Renderer::EndFrame();
    
    // Unbind virtual resolution framebuffer
    framebuffer->Unbind();
    
    // Clear window framebuffer to black
    PyNovaGE::Renderer::Renderer::SetViewport(0, 0, window_width, window_height);
    PyNovaGE::Renderer::Renderer::Clear(PyNovaGE::Vector4f(0.0f, 0.0f, 0.0f, 1.0f));
    
    // Calculate letterbox/pillarbox dimensions to maintain aspect ratio
    float window_aspect = static_cast<float>(window_width) / static_cast<float>(window_height);
    float target_aspect = static_cast<float>(virtual_width) / static_cast<float>(virtual_height);
    
    int viewport_x = 0;
    int viewport_y = 0;
    int viewport_width = window_width;
    int viewport_height = window_height;
    
    if (window_aspect > target_aspect) {
        // Window is wider - letterbox left/right
        viewport_width = static_cast<int>(window_height * target_aspect);
        viewport_x = (window_width - viewport_width) / 2;
    } else {
        // Window is taller - letterbox top/bottom
        viewport_height = static_cast<int>(window_width / target_aspect);
        viewport_y = (window_height - viewport_height) / 2;
    }
    
    // Set viewport for scaled rendering
    PyNovaGE::Renderer::Renderer::SetViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    // Update projection when window size changes
    UpdateProjection();
    
    // Render virtual resolution framebuffer to screen quad
    screen_quad->Render(framebuffer->GetTextureHandle());
    
    // Finish rendering
    PyNovaGE::Renderer::Renderer::EndFrame();
}

void Game::cleanUp() {
    // Clean up game objects
    delete player;
    for (auto platform : platforms) {
        delete platform;
    }
    platforms.clear();

    // Clean up systems in reverse initialization order
    if (particleSystem) { particleSystem->Shutdown(); delete particleSystem; }
    if (audioSystem) { audioSystem->Shutdown(); delete audioSystem; }
    if (scene) { scene->Shutdown(); delete scene; }
    if (physicsWorld) { physicsWorld->clear(); delete physicsWorld; }
    renderer = nullptr; // Don't delete - owned by engine

    // Delete framebuffer and screen quad
    if (screen_quad) { delete screen_quad; screen_quad = nullptr; }
    if (framebuffer) { delete framebuffer; framebuffer = nullptr; }

    // Shut down core renderer after sprite renderer
    PyNovaGE::Renderer::Renderer::Shutdown();
    // Delete window last since it holds the OpenGL context
    if (window) { delete window; }
}
