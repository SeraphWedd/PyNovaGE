/**
 * @file main.cpp
 * @brief Interactive Voxel World Demo
 * 
 * Showcases the PyNovaGE voxel rendering system with:
 * - Real-time 3D voxel world rendering
 * - Greedy meshing optimization
 * - Frustum culling performance
 * - Interactive camera controls
 * - Live performance statistics
 * - Multiple world generation patterns
 */

#include <iostream>
#include <memory>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <deque>

// Engine includes
#include <window/window.hpp>
#include <renderer/renderer.hpp>
#include <renderer/voxel/voxel_renderer.hpp>
#include <renderer/voxel/camera.hpp>
#include <renderer/voxel/voxel_types.hpp>
#include <vectors/vector4.hpp>

// GLFW for input handling
#include <GLFW/glfw3.h>

using namespace PyNovaGE::Renderer::Voxel;

/**
 * @brief Interactive Voxel Demo Application
 */
class VoxelDemo {
public:
    VoxelDemo() : window_(nullptr), camera_(), renderer_("shaders/voxel/"), world_(8) {
        last_frame_time_ = std::chrono::high_resolution_clock::now();
        
        // Configure camera for nice viewing
        camera_.SetPosition(Vector3f(32.0f, 40.0f, 32.0f));
        camera_.SetRotation(45.0f, -20.0f);
        camera_.SetPerspective(75.0f, 16.0f/9.0f, 0.1f, 500.0f);
        camera_.SetMovementSpeed(25.0f);
        camera_.SetMouseSensitivity(0.2f);
    }
    
    ~VoxelDemo() {
        Cleanup();
    }
    
    bool Initialize() {
        std::cout << "🎮 PyNovaGE Voxel Demo - Initializing..." << std::endl;
        
        // Initialize window system
        window_system_guard_ = std::make_unique<PyNovaGE::Window::WindowSystemGuard>();
        if (!window_system_guard_->IsInitialized()) {
            std::cerr << "❌ Failed to initialize window system!" << std::endl;
            return false;
        }
        
        // Create window
        PyNovaGE::Window::WindowConfig window_config;
        window_config.width = 1600;
        window_config.height = 900;
        window_config.title = "PyNovaGE Voxel Demo - High Performance 3D Voxel Engine";
        window_config.resizable = true;
        window_config.vsync = true;
        
        window_ = std::make_unique<PyNovaGE::Window::Window>(window_config);
        std::cout << "✅ Window created successfully!" << std::endl;
        
        // Make OpenGL context current
        window_->MakeContextCurrent();
        
        // Initialize main renderer system
        PyNovaGE::Renderer::RendererConfig renderer_config;
        renderer_config.enable_vsync = true;
        renderer_config.enable_depth_test = true;
        renderer_config.enable_blend = true;
        
        renderer_guard_ = std::make_unique<PyNovaGE::Renderer::RendererGuard>(renderer_config);
        if (!renderer_guard_->IsInitialized()) {
            std::cerr << "❌ Failed to initialize main renderer!" << std::endl;
            return false;
        }
        
        std::cout << "✅ Main renderer initialized!" << std::endl;
        std::cout << "Renderer Info: " << PyNovaGE::Renderer::Renderer::GetRendererInfo() << std::endl;
        
        // Set viewport to match window size
        auto window_size = window_->GetFramebufferSize();
        PyNovaGE::Renderer::Renderer::SetViewport(0, 0, window_size.x, window_size.y);
        std::cout << "Viewport set to: " << window_size.x << "x" << window_size.y << std::endl;
        
        // Setup input callbacks
        SetupInputCallbacks();
        
        // Initialize voxel renderer
        std::cout << "Initializing voxel renderer..." << std::endl;
        if (!renderer_.Initialize()) {
            std::cerr << "❌ Failed to initialize voxel renderer!" << std::endl;
            return false;
        }
        std::cout << "✅ Voxel renderer initialized!" << std::endl;
        
        // Configure renderer for best performance
        VoxelRenderConfig render_config;
        render_config.enable_frustum_culling = true;
        render_config.enable_multithreaded_meshing = false;  // Disable for now to test
        render_config.max_render_distance = 200.0f;
        render_config.max_remesh_per_frame = 4;
        render_config.max_upload_per_frame = 2;
        
        renderer_.SetConfig(render_config);
        renderer_.SetWorld(&world_);
        
        // Generate world patterns
        GenerateWorld();
        
        std::cout << "✅ Voxel demo initialized successfully!" << std::endl;
        std::cout << std::endl;
        PrintControls();
        
        return true;
    }
    
    void Run() {
        std::cout << "🚀 Starting voxel demo main loop..." << std::endl;
        
        int frame_count = 0;
        while (!window_->ShouldClose()) {
            if (frame_count == 0) {
                std::cout << "Entering first frame..." << std::endl;
            }
            
            auto current_time = std::chrono::high_resolution_clock::now();
            float delta_time = std::chrono::duration<float>(current_time - last_frame_time_).count();
            last_frame_time_ = current_time;
            
            if (frame_count == 0) {
                std::cout << "Processing input..." << std::endl;
            }
            
            // Handle input
            ProcessInput(delta_time);
            
            if (frame_count == 0) {
                std::cout << "Updating systems..." << std::endl;
            }
            
            // Update systems
            Update(delta_time);
            
            if (frame_count == 0) {
                std::cout << "Rendering frame..." << std::endl;
            }
            
            // Render frame
            Render();
            
            if (frame_count == 0) {
                std::cout << "Swapping buffers..." << std::endl;
            }
            
            // Swap buffers and poll events
            window_->SwapBuffers();
            window_->PollEvents();
            
            // Update performance stats
            UpdatePerformanceStats(delta_time);
            
            frame_count++;
            if (frame_count == 1) {
                std::cout << "First frame completed successfully!" << std::endl;
            }
            if (frame_count == 2) {
                std::cout << "Second frame completed successfully!" << std::endl;
            }
            if (frame_count % 60 == 0) {
                std::cout << "Frame " << frame_count << " completed" << std::endl;
            }
        }
        
        std::cout << "👋 Voxel demo shutting down..." << std::endl;
    }

private:
    void GenerateWorld() {
        std::cout << "🌍 Generating voxel world (8x8 chunks)..." << std::endl;
        
        // The SimpleVoxelWorld already generates terrain in constructor
        // Let's add some variety to make it more interesting
        
        // Add some structures and patterns
        for (int cx = 0; cx < 8; ++cx) {
            for (int cz = 0; cz < 8; ++cz) {
                // Add pillars at corners
                if ((cx == 0 || cx == 7) && (cz == 0 || cz == 7)) {
                    for (int y = 3; y < 15; ++y) {
                        world_.SetVoxel(Vector3f(static_cast<float>(cx * 16 + 8), static_cast<float>(y), static_cast<float>(cz * 16 + 8)), VoxelType::WOOD);
                    }
                }
                
                // Add some variety to the terrain
                if ((cx + cz) % 3 == 0) {
                    // Raise terrain
                    for (int lx = 0; lx < 16; ++lx) {
                        for (int lz = 0; lz < 16; ++lz) {
                            if ((lx + lz) % 2 == 0) {
                                world_.SetVoxel(Vector3f(static_cast<float>(cx * 16 + lx), 3.0f, static_cast<float>(cz * 16 + lz)), VoxelType::STONE);
                                world_.SetVoxel(Vector3f(static_cast<float>(cx * 16 + lx), 4.0f, static_cast<float>(cz * 16 + lz)), VoxelType::DIRT);
                            }
                        }
                    }
                }
            }
        }
        
        std::cout << "✅ World generation complete!" << std::endl;
    }
    
    void SetupInputCallbacks() {
        // Set user pointer for callbacks
        glfwSetWindowUserPointer(window_->GetNativeWindow(), this);
        
        // Mouse callback for camera rotation
        glfwSetCursorPosCallback(window_->GetNativeWindow(), 
            [](GLFWwindow* window, double xpos, double ypos) {
                auto* demo = static_cast<VoxelDemo*>(glfwGetWindowUserPointer(window));
                demo->OnMouseMove(xpos, ypos);
            });
        
        // Key callback for commands
        glfwSetKeyCallback(window_->GetNativeWindow(),
            [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                auto* demo = static_cast<VoxelDemo*>(glfwGetWindowUserPointer(window));
                demo->OnKeyPress(key, scancode, action, mods);
            });
    }
    
    void OnMouseMove(double xpos, double ypos) {
        if (first_mouse_) {
            last_mouse_x_ = xpos;
            last_mouse_y_ = ypos;
            first_mouse_ = false;
        }
        
        float xoffset = static_cast<float>(xpos - last_mouse_x_);
        float yoffset = static_cast<float>(last_mouse_y_ - ypos); // Reversed for Y
        
        last_mouse_x_ = xpos;
        last_mouse_y_ = ypos;
        
        if (mouse_captured_) {
            camera_.Rotate(xoffset, yoffset);
        }
    }
    
    void OnKeyPress(int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) {
        if (action != GLFW_PRESS) return;
        
        switch (key) {
            case GLFW_KEY_ESCAPE:
                window_->SetShouldClose(true);
                break;
            case GLFW_KEY_TAB:
                ToggleMouseCapture();
                break;
            case GLFW_KEY_F1:
                show_performance_stats_ = !show_performance_stats_;
                std::cout << "Performance stats: " << (show_performance_stats_ ? "ON" : "OFF") << std::endl;
                break;
            case GLFW_KEY_F2:
                wireframe_mode_ = !wireframe_mode_;
                std::cout << "Wireframe mode: " << (wireframe_mode_ ? "ON" : "OFF") << std::endl;
                break;
            case GLFW_KEY_F3:
                // Toggle frustum culling
                {
                    auto config = renderer_.GetConfig();
                    config.enable_frustum_culling = !config.enable_frustum_culling;
                    renderer_.SetConfig(config);
                    std::cout << "Frustum culling: " << (config.enable_frustum_culling ? "ON" : "OFF") << std::endl;
                }
                break;
            case GLFW_KEY_R:
                ResetCamera();
                break;
        }
    }
    
    void ProcessInput(float delta_time) {
        GLFWwindow* window = window_->GetNativeWindow();
        
        // Camera movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera_.MoveForward(camera_.GetMovementSpeed() * delta_time);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera_.MoveForward(-camera_.GetMovementSpeed() * delta_time);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera_.MoveRight(-camera_.GetMovementSpeed() * delta_time);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera_.MoveRight(camera_.GetMovementSpeed() * delta_time);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera_.MoveUp(camera_.GetMovementSpeed() * delta_time);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera_.MoveUp(-camera_.GetMovementSpeed() * delta_time);
            
        // Speed adjustment
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            camera_.SetMovementSpeed(50.0f); // Fast mode
        } else {
            camera_.SetMovementSpeed(25.0f); // Normal speed
        }
    }
    
    void Update(float delta_time) {
        // Update renderer
        renderer_.Update(delta_time, camera_);
        
        // Update window title with FPS
        frame_count_++;
        fps_timer_ += delta_time;
        
        if (fps_timer_ >= 1.0f) {
            current_fps_ = frame_count_ / fps_timer_;
            frame_count_ = 0;
            fps_timer_ = 0.0f;
            
            // Update window title
            std::stringstream title;
            title << "PyNovaGE Voxel Demo - " << std::fixed << std::setprecision(1) 
                  << current_fps_ << " FPS - Pos: " << std::setprecision(0)
                  << camera_.GetPosition().x << ", " << camera_.GetPosition().y << ", " << camera_.GetPosition().z;
            
            window_->SetTitle(title.str());
        }
    }
    
    void Render() {
        // Begin frame
        PyNovaGE::Renderer::Renderer::BeginFrame();
        
        // Clear screen with sky blue
        PyNovaGE::Vector4f sky_color(0.53f, 0.81f, 0.98f, 1.0f);
        PyNovaGE::Renderer::Renderer::Clear(sky_color);
        
        // Wireframe mode
        if (wireframe_mode_) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        // Render voxel world
        renderer_.Render(camera_);
        
        // End frame
        PyNovaGE::Renderer::Renderer::EndFrame();
        
        // Print performance stats to console periodically
        if (show_performance_stats_ && fps_timer_ >= 0.9f) {
            PrintPerformanceStats();
        }
    }
    
    void UpdatePerformanceStats(float delta_time) {
        frame_times_.push_back(delta_time * 1000.0f); // Convert to ms
        if (frame_times_.size() > 60) { // Keep last 60 frames
            frame_times_.pop_front();
        }
    }
    
    void PrintPerformanceStats() {
        auto stats = renderer_.GetStats();
        
        // Calculate average frame time
        float avg_frame_time = 0.0f;
        for (float time : frame_times_) {
            avg_frame_time += time;
        }
        avg_frame_time /= frame_times_.size();
        
        std::cout << "\n📊 Performance Stats:" << std::endl;
        std::cout << "  FPS: " << current_fps_ << " | Frame: " << avg_frame_time << "ms" << std::endl;
        std::cout << "  Chunks - Total: " << stats.total_chunks 
                  << " | Visible: " << stats.visible_chunks 
                  << " | Culled: " << stats.culled_chunks << std::endl;
        std::cout << "  Culling ratio: " << (stats.culling_ratio * 100.0f) << "%" << std::endl;
        std::cout << "  Render time: " << stats.render_time_ms << "ms" << std::endl;
        std::cout << "  Memory: " << (stats.cpu_memory_used / 1024) << "KB CPU" << std::endl;
    }
    
    void ToggleMouseCapture() {
        mouse_captured_ = !mouse_captured_;
        
        if (mouse_captured_) {
            glfwSetInputMode(window_->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            std::cout << "🖱️  Mouse captured - move to look around" << std::endl;
        } else {
            glfwSetInputMode(window_->GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            std::cout << "🖱️  Mouse released" << std::endl;
        }
    }
    
    void ResetCamera() {
        camera_.SetPosition(Vector3f(32.0f, 40.0f, 32.0f));
        camera_.SetRotation(45.0f, -20.0f);
        std::cout << "📷 Camera reset to default position" << std::endl;
    }
    
    void PrintControls() {
        std::cout << "🎮 Controls:" << std::endl;
        std::cout << "  WASD      - Move around" << std::endl;
        std::cout << "  Space     - Move up" << std::endl;
        std::cout << "  Shift     - Move down" << std::endl;
        std::cout << "  Ctrl      - Move faster" << std::endl;
        std::cout << "  Mouse     - Look around (when captured)" << std::endl;
        std::cout << "  Tab       - Toggle mouse capture" << std::endl;
        std::cout << "  F1        - Toggle performance stats" << std::endl;
        std::cout << "  F2        - Toggle wireframe mode" << std::endl;
        std::cout << "  F3        - Toggle frustum culling" << std::endl;
        std::cout << "  R         - Reset camera position" << std::endl;
        std::cout << "  Escape    - Exit demo" << std::endl;
        std::cout << std::endl;
    }
    
    void Cleanup() {
        // Resources will be cleaned up automatically by RAII
    }

private:
    // Core systems
    std::unique_ptr<PyNovaGE::Window::WindowSystemGuard> window_system_guard_;
    std::unique_ptr<PyNovaGE::Window::Window> window_;
    std::unique_ptr<PyNovaGE::Renderer::RendererGuard> renderer_guard_;
    Camera camera_;
    VoxelRenderer renderer_;
    SimpleVoxelWorld world_;
    
    // Timing
    std::chrono::high_resolution_clock::time_point last_frame_time_;
    float fps_timer_ = 0.0f;
    int frame_count_ = 0;
    float current_fps_ = 0.0f;
    std::deque<float> frame_times_;
    
    // Input state
    bool first_mouse_ = true;
    double last_mouse_x_ = 0.0;
    double last_mouse_y_ = 0.0;
    bool mouse_captured_ = false;
    
    // Display options
    bool show_performance_stats_ = false;
    bool wireframe_mode_ = false;
};

int main() {
    std::cout << "🎮 PyNovaGE Voxel Demo" << std::endl;
    std::cout << "High-Performance 3D Voxel Rendering Engine" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    try {
        VoxelDemo demo;
        
        if (!demo.Initialize()) {
            std::cerr << "❌ Failed to initialize voxel demo!" << std::endl;
            return -1;
        }
        
        demo.Run();
        
    } catch (const std::exception& e) {
        std::cerr << "💥 Exception: " << e.what() << std::endl;
        return -1;
    }
    
    std::cout << "✅ Voxel demo completed successfully!" << std::endl;
    return 0;
}