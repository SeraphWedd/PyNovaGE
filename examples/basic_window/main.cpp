#include <window/window.hpp>
#include <renderer/renderer.hpp>
#include <vectors/vector4.hpp>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "PyNovaGE - Basic Window Demo\n";
    std::cout << "=============================\n";
    
    try {
        // Initialize window system
        std::cout << "Initializing window system...\n";
        PyNovaGE::Window::WindowSystemGuard window_system_guard;
        
        if (!window_system_guard.IsInitialized()) {
            std::cerr << "Failed to initialize window system!\n";
            return -1;
        }
        
        std::cout << "Window system initialized successfully!\n";
        
        // Create window configuration
        PyNovaGE::Window::WindowConfig window_config;
        window_config.title = "PyNovaGE - Basic Window Test";
        window_config.width = 800;
        window_config.height = 600;
        window_config.resizable = true;
        window_config.vsync = true;
        
        std::cout << "Creating window: " << window_config.width << "x" << window_config.height << "\n";
        
        // Create window
        PyNovaGE::Window::Window window(window_config);
        std::cout << "Window created successfully!\n";
        
        // Initialize renderer
        PyNovaGE::Renderer::RendererConfig renderer_config;
        renderer_config.enable_vsync = true;
        renderer_config.enable_depth_test = true;
        renderer_config.enable_blend = true;
        
        std::cout << "Initializing renderer...\n";
        PyNovaGE::Renderer::RendererGuard renderer_guard(renderer_config);
        
        if (!renderer_guard.IsInitialized()) {
            std::cerr << "Failed to initialize renderer!\n";
            return -1;
        }
        
        std::cout << "Renderer initialized successfully!\n";
        std::cout << "Renderer Info: " << PyNovaGE::Renderer::Renderer::GetRendererInfo() << "\n";
        
        // Set viewport to match window size
        auto window_size = window.GetFramebufferSize();
        PyNovaGE::Renderer::Renderer::SetViewport(0, 0, window_size.x, window_size.y);
        std::cout << "Viewport set to: " << window_size.x << "x" << window_size.y << "\n";
        
        // Colors to cycle through
        std::vector<PyNovaGE::Vector4f> colors = {
            {0.2f, 0.3f, 0.8f, 1.0f},  // Blue
            {0.8f, 0.2f, 0.3f, 1.0f},  // Red
            {0.3f, 0.8f, 0.2f, 1.0f},  // Green
            {0.8f, 0.8f, 0.2f, 1.0f},  // Yellow
            {0.8f, 0.2f, 0.8f, 1.0f},  // Magenta
            {0.2f, 0.8f, 0.8f, 1.0f},  // Cyan
        };
        
        int color_index = 0;
        auto start_time = std::chrono::steady_clock::now();
        
        std::cout << "\n=== Starting Render Loop ===\n";
        std::cout << "The window should now be visible and cycling through colors!\n";
        std::cout << "Close the window to exit, or it will automatically close after 10 seconds.\n\n";
        
        // Main render loop
        int frame_count = 0;
        while (!window.ShouldClose()) {
            // Poll window events
            window.PollEvents();
            
            // Calculate elapsed time
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
            
            // Change color every second
            int new_color_index = static_cast<int>((elapsed.count() / 1000) % colors.size());
            if (new_color_index != color_index) {
                color_index = new_color_index;
                std::cout << "Frame " << frame_count << ": Switching to color " << color_index << "\n";
            }
            
            // Begin frame
            PyNovaGE::Renderer::Renderer::BeginFrame();
            
            // Clear with current color
            PyNovaGE::Renderer::Renderer::Clear(colors[color_index]);
            
            // End frame
            PyNovaGE::Renderer::Renderer::EndFrame();
            
            // Swap buffers
            window.SwapBuffers();
            
            frame_count++;
            
            // Auto-close after 10 seconds for automated testing
            if (elapsed.count() > 10000) {
                std::cout << "Auto-closing after 10 seconds...\n";
                window.SetShouldClose(true);
            }
            
            // Small sleep to prevent excessive CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }
        
        auto final_stats = PyNovaGE::Renderer::Renderer::GetStats();
        std::cout << "\n=== Render Loop Finished ===\n";
        std::cout << "Total frames rendered: " << frame_count << "\n";
        std::cout << "Final render stats:\n";
        std::cout << "  Draw calls: " << final_stats.draw_calls << "\n";
        std::cout << "  Frame time: " << final_stats.frame_time_ms << " ms\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return -1;
    }
    
    std::cout << "\n=== Demo completed successfully! ===\n";
    return 0;
}