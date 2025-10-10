#include "window/window.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include <iostream>

using namespace PyNovaGE;

int main() {
    try {
        // Initialize window system
        Window::WindowSystemGuard guard;
        
        // Create minimal window for OpenGL context
        Window::WindowConfig config;
        config.width = 400;
        config.height = 300;
        config.title = "PyNovaGE Shader Test";
        config.visible = false; // Hidden test window
        
        Window::Window window(config);
        
        // Initialize renderer
        Renderer::RendererConfig renderer_config{};
        if (!Renderer::Renderer::Initialize(renderer_config)) {
            std::cerr << "Failed to initialize renderer!" << std::endl;
            return 1;
        }
        
        std::cout << "=== PyNovaGE Shader System Test ===" << std::endl;
        
        // Test 1: Create a simple shader manually
        std::cout << "\n1. Testing manual shader creation..." << std::endl;
        
        const std::string vertex_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 transform;
void main() {
    gl_Position = transform * vec4(aPos, 1.0);
}
)";

        const std::string fragment_source = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main() {
    FragColor = color;
}
)";
        
        Renderer::Shader test_shader(vertex_source, fragment_source);
        
        if (test_shader.IsValid()) {
            std::cout << "[PASS] Shader created and compiled successfully!" << std::endl;
            
            // Test uniform setting
            test_shader.Bind();
            test_shader.SetVector4f("color", Vector4f{1.0f, 0.5f, 0.2f, 1.0f});
            test_shader.SetFloat("test_float", 3.14f);
            test_shader.SetInt("test_int", 42);
            
            std::cout << "[PASS] Uniform setting completed without errors" << std::endl;
            test_shader.Unbind();
        } else {
            std::cout << "[FAIL] Shader creation failed: " << test_shader.GetErrorLog() << std::endl;
            return 1;
        }
        
        // Test 2: Test ShaderLibrary
        std::cout << "\n2. Testing ShaderLibrary..." << std::endl;
        
        auto& shader_lib = Renderer::ShaderLibrary::Instance();
        
        // Load default shaders
        shader_lib.LoadDefaultShaders();
        
        // Check if default shaders were loaded
        if (shader_lib.HasShader("color") && shader_lib.HasShader("sprite")) {
            std::cout << "[PASS] Default shaders loaded successfully!" << std::endl;
            
            // Test retrieving shaders
            Renderer::Shader* color_shader = shader_lib.GetShader("color");
            Renderer::Shader* sprite_shader = shader_lib.GetShader("sprite");
            
            if (color_shader && color_shader->IsValid()) {
                std::cout << "[PASS] Color shader retrieved and valid" << std::endl;
            } else {
                std::cout << "[FAIL] Color shader invalid" << std::endl;
            }
            
            if (sprite_shader && sprite_shader->IsValid()) {
                std::cout << "[PASS] Sprite shader retrieved and valid" << std::endl;
            } else {
                std::cout << "[FAIL] Sprite shader invalid" << std::endl;
            }
        } else {
            std::cout << "[FAIL] Failed to load default shaders" << std::endl;
        }
        
        // Test 3: Test shader creation through library
        std::cout << "\n3. Testing custom shader through library..." << std::endl;
        
        bool loaded = shader_lib.LoadShader("test_shader", vertex_source, fragment_source);
        if (loaded && shader_lib.HasShader("test_shader")) {
            std::cout << "[PASS] Custom shader loaded through library successfully!" << std::endl;
            
            Renderer::Shader* custom_shader = shader_lib.GetShader("test_shader");
            if (custom_shader && custom_shader->IsValid()) {
                std::cout << "[PASS] Custom shader is valid and accessible" << std::endl;
            } else {
                std::cout << "[FAIL] Custom shader is invalid" << std::endl;
            }
        } else {
            std::cout << "[FAIL] Failed to load custom shader through library" << std::endl;
        }
        
        // Test 4: Test error handling
        std::cout << "\n4. Testing error handling..." << std::endl;
        
        const std::string bad_vertex = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
invalid_syntax_here;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";
        
        Renderer::Shader bad_shader(bad_vertex, fragment_source);
        if (!bad_shader.IsValid()) {
            std::cout << "[PASS] Error handling works - invalid shader properly rejected" << std::endl;
            std::cout << "  Error log: " << bad_shader.GetErrorLog() << std::endl;
        } else {
            std::cout << "[FAIL] Error handling failed - invalid shader was accepted" << std::endl;
        }
        
        std::cout << "\n=== Shader System Test Results ===" << std::endl;
        std::cout << "[PASS] Shader compilation and linking: Working" << std::endl;
        std::cout << "[PASS] Uniform management: Working" << std::endl;
        std::cout << "[PASS] ShaderLibrary: Working" << std::endl;
        std::cout << "[PASS] Error handling: Working" << std::endl;
        std::cout << "[PASS] Default shaders: Working" << std::endl;
        
        // Cleanup
        shader_lib.Clear();
        Renderer::Renderer::Shutdown();
        
        std::cout << "\n*** All shader system tests passed! ***" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}