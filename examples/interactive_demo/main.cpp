#include "window/window.hpp"
#include "window/input.hpp"
#include "renderer/renderer.hpp"
#include <iostream>
#include <iomanip>

int main() {
    try {
        // Initialize window system
        PyNovaGE::Window::WindowSystemGuard guard;
        
        // Configure window
        PyNovaGE::Window::WindowConfig config;
        config.width = 800;
        config.height = 600;
        config.title = "PyNovaGE Interactive Demo";
        config.resizable = true;
        config.vsync = true;
        
        // Create window
        PyNovaGE::Window::Window window(config);
        
        // Initialize renderer
        PyNovaGE::Renderer::RendererConfig renderer_config{};
        PyNovaGE::Renderer::Renderer::Initialize(renderer_config);
        
        // Set viewport to match window
        auto size = window.GetSize();
        PyNovaGE::Renderer::Renderer::SetViewport(0, 0, size.x, size.y);
        
        // Initialize input manager
        PyNovaGE::Window::InputManager input(window.GetNativeWindow());
        
        // Set up event callbacks
        window.SetEventCallback([&](const PyNovaGE::Window::WindowEvent& event) {
            switch (event.type) {
                case PyNovaGE::Window::WindowEventType::Close:
                    std::cout << "Window close requested\n";
                    break;
                case PyNovaGE::Window::WindowEventType::Resize:
                    std::cout << "Window resized to " << event.width << "x" << event.height << "\n";
                    PyNovaGE::Renderer::Renderer::SetViewport(0, 0, event.width, event.height);
                    break;
                case PyNovaGE::Window::WindowEventType::Focus:
                    std::cout << "Window gained focus\n";
                    break;
                case PyNovaGE::Window::WindowEventType::Unfocus:
                    std::cout << "Window lost focus\n";
                    break;
                case PyNovaGE::Window::WindowEventType::Minimize:
                    std::cout << "Window minimized\n";
                    break;
                case PyNovaGE::Window::WindowEventType::Maximize:
                    std::cout << "Window maximized\n";
                    break;
                case PyNovaGE::Window::WindowEventType::Restore:
                    std::cout << "Window restored\n";
                    break;
            }
        });
        
        input.SetInputCallback([&](const PyNovaGE::Window::InputEvent& event) {
            static int move_count = 0; // Move static variable outside switch
            switch (event.type) {
                case PyNovaGE::Window::InputEventType::KeyPress:
                    std::cout << "Key pressed: " << static_cast<int>(event.key);
                    if (event.shift) std::cout << " +Shift";
                    if (event.control) std::cout << " +Ctrl";
                    if (event.alt) std::cout << " +Alt";
                    std::cout << "\n";
                    break;
                case PyNovaGE::Window::InputEventType::KeyRelease:
                    std::cout << "Key released: " << static_cast<int>(event.key) << "\n";
                    break;
                case PyNovaGE::Window::InputEventType::MousePress:
                    std::cout << "Mouse button pressed: " << static_cast<int>(event.mouse_button) 
                             << " at (" << event.mouse_position.x << ", " << event.mouse_position.y << ")\n";
                    break;
                case PyNovaGE::Window::InputEventType::MouseRelease:
                    std::cout << "Mouse button released: " << static_cast<int>(event.mouse_button) << "\n";
                    break;
                case PyNovaGE::Window::InputEventType::MouseMove:
                    // Only log occasionally to avoid spam
                    if (++move_count % 50 == 0) {
                        std::cout << "Mouse at (" << event.mouse_position.x << ", " << event.mouse_position.y << ")\n";
                    }
                    break;
                case PyNovaGE::Window::InputEventType::MouseScroll:
                    std::cout << "Mouse scroll: (" << event.scroll_offset.x << ", " << event.scroll_offset.y << ")\n";
                    break;
            }
        });
        
        std::cout << "=== PyNovaGE Interactive Demo ===\n";
        std::cout << "Controls:\n";
        std::cout << "  WASD/Arrow Keys - Change background color\n";
        std::cout << "  Mouse - Move around and click\n";
        std::cout << "  Scroll wheel - Test scroll events\n";
        std::cout << "  ESC - Close window\n";
        std::cout << "  F11 - Toggle fullscreen\n";
        std::cout << "  Space - Toggle VSync\n";
        std::cout << "  C - Toggle cursor visibility\n";
        std::cout << "  1-9 - Test gamepad (if connected)\n";
        std::cout << "================================\n\n";
        
        // Color and state variables
        float red = 0.2f, green = 0.3f, blue = 0.4f;
        bool cursor_visible = true;
        double start_time = window.GetTime();
        
        // Main loop
        while (!window.ShouldClose()) {
            double current_time = window.GetTime();
            double delta_time = current_time - start_time;
            
            // Poll window events
            window.PollEvents();
            
            // Update input state
            input.Update();
            
            // Handle input
            if (input.IsKeyJustPressed(PyNovaGE::Window::Key::Escape)) {
                window.SetShouldClose(true);
            }
            
            if (input.IsKeyJustPressed(PyNovaGE::Window::Key::F11)) {
                window.SetFullscreen(!window.IsFullscreen());
                std::cout << "Fullscreen: " << (window.IsFullscreen() ? "ON" : "OFF") << "\n";
            }
            
            if (input.IsKeyJustPressed(PyNovaGE::Window::Key::Space)) {
                bool vsync = !window.IsVSyncEnabled();
                window.SetVSync(vsync);
                std::cout << "VSync: " << (vsync ? "ON" : "OFF") << "\n";
            }
            
            if (input.IsKeyJustPressed(PyNovaGE::Window::Key::C)) {
                cursor_visible = !cursor_visible;
                input.SetMouseCursorVisible(cursor_visible);
                std::cout << "Cursor: " << (cursor_visible ? "VISIBLE" : "HIDDEN") << "\n";
            }
            
            // Color controls
            float color_speed = 0.5f * static_cast<float>(delta_time);
            if (input.IsKeyPressed(PyNovaGE::Window::Key::W) || input.IsKeyPressed(PyNovaGE::Window::Key::Up)) {
                red = std::min(1.0f, red + color_speed);
            }
            if (input.IsKeyPressed(PyNovaGE::Window::Key::S) || input.IsKeyPressed(PyNovaGE::Window::Key::Down)) {
                red = std::max(0.0f, red - color_speed);
            }
            if (input.IsKeyPressed(PyNovaGE::Window::Key::A) || input.IsKeyPressed(PyNovaGE::Window::Key::Left)) {
                green = std::min(1.0f, green + color_speed);
            }
            if (input.IsKeyPressed(PyNovaGE::Window::Key::D) || input.IsKeyPressed(PyNovaGE::Window::Key::Right)) {
                green = std::max(0.0f, green - color_speed);
            }
            
            // Mouse influence on blue channel
            auto mouse_pos = input.GetMousePosition();
            auto window_size = window.GetSize();
            if (window_size.x > 0 && window_size.y > 0) {
                blue = mouse_pos.y / static_cast<float>(window_size.y);
            }
            
            // Test gamepad input
            if (input.IsGamepadConnected(0)) {
                static bool gamepad_logged = false;
                if (!gamepad_logged) {
                    const auto& gamepad_state = input.GetGamepadState(0);
                    std::cout << "Gamepad connected: " << gamepad_state.name << "\n";
                    gamepad_logged = true;
                }
                
                // Use gamepad left stick to influence red channel
                float left_stick_x = input.GetGamepadAxis(0, PyNovaGE::Window::GamepadAxis::LeftX);
                red += left_stick_x * color_speed;
                red = std::max(0.0f, std::min(1.0f, red));
                
                // Test gamepad buttons
                for (int i = 1; i <= 9; ++i) {
                    if (input.IsKeyJustPressed(static_cast<PyNovaGE::Window::Key>(48 + i))) {  // Keys 1-9
                        std::cout << "Testing gamepad button " << i << "\n";
                    }
                }
            }
            
            // Render
            PyNovaGE::Renderer::Renderer::Clear(PyNovaGE::Vector4f{red, green, blue, 1.0f});
            
            // Display current status every 2 seconds
            static double last_status_time = 0.0;
            if (current_time - last_status_time > 2.0) {
                std::cout << std::fixed << std::setprecision(2)
                         << "Status: RGB(" << red << ", " << green << ", " << blue << ") "
                         << "Mouse(" << mouse_pos.x << ", " << mouse_pos.y << ") "
                         << "FPS: ~" << static_cast<int>(1.0 / delta_time) << "\n";
                last_status_time = current_time;
            }
            
            // Swap buffers
            window.SwapBuffers();
            
            start_time = current_time;
        }
        
        std::cout << "Demo completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}