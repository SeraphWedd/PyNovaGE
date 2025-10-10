#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"
#include "renderer/sprite_renderer.hpp"
#include "renderer/batch_renderer.hpp"
#include <glad/gl.h>
#include <iostream>
#include <sstream>
#include <chrono>

namespace PyNovaGE {
namespace Renderer {

// Static member definitions
bool Renderer::s_initialized_ = false;
RendererConfig Renderer::s_config_;
RenderStats Renderer::s_stats_;
std::unique_ptr<SpriteRenderer> Renderer::s_sprite_renderer_;
std::unique_ptr<BatchRenderer> Renderer::s_batch_renderer_;

bool Renderer::Initialize(const RendererConfig& config) {
    if (s_initialized_) {
        std::cerr << "Renderer already initialized!" << std::endl;
        return true;
    }
    
    // Load OpenGL functions
    if (!gladLoaderLoadGL()) {
        std::cerr << "Failed to initialize OpenGL function pointers!" << std::endl;
        return false;
    }
    
    // Store configuration
    s_config_ = config;
    
    // Get OpenGL information
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    
    std::cout << "OpenGL initialized:" << std::endl;
    std::cout << "  Version: " << (version ? version : "Unknown") << std::endl;
    std::cout << "  Vendor: " << (vendor ? vendor : "Unknown") << std::endl;
    std::cout << "  Renderer: " << (renderer ? renderer : "Unknown") << std::endl;
    
    // Set initial OpenGL state
    if (s_config_.enable_depth_test) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }
    
    if (s_config_.enable_blend) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    if (s_config_.enable_culling) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }
    
    // Set clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Check for errors
    if (!CheckGLError("Initial setup")) {
        std::cerr << "OpenGL errors during initialization!" << std::endl;
        return false;
    }
    
    // Initialize subsystems
    try {
        s_sprite_renderer_ = std::make_unique<SpriteRenderer>();
        s_batch_renderer_ = std::make_unique<BatchRenderer>(s_config_.max_sprites_per_batch, s_config_.max_textures_per_batch);
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize renderer subsystems: " << e.what() << std::endl;
        return false;
    }
    
    s_initialized_ = true;
    std::cout << "Renderer initialized successfully!" << std::endl;
    return true;
}

void Renderer::Shutdown() {
    if (!s_initialized_) {
        return;
    }
    
    // Cleanup subsystems
    s_batch_renderer_.reset();
    s_sprite_renderer_.reset();
    
    s_initialized_ = false;
    std::cout << "Renderer shut down." << std::endl;
}

bool Renderer::IsInitialized() {
    return s_initialized_;
}

void Renderer::BeginFrame() {
    if (!s_initialized_) {
        std::cerr << "Renderer not initialized!" << std::endl;
        return;
    }
    
    // Reset statistics
    s_stats_.Reset();
    
    // Start frame timer
    static auto start_time = std::chrono::high_resolution_clock::now();
    start_time = std::chrono::high_resolution_clock::now();
}

void Renderer::EndFrame() {
    if (!s_initialized_) {
        return;
    }
    
    // Calculate frame time
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    s_stats_.frame_time_ms = duration.count() / 1000.0;
}

void Renderer::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
    CheckGLError("SetViewport");
}

void Renderer::Clear(const Vector4f& color) {
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    CheckGLError("Clear");
}

void Renderer::SetClearColor(const Vector4f& color) {
    glClearColor(color.x, color.y, color.z, color.w);
    CheckGLError("SetClearColor");
}

void Renderer::SetDepthTest(bool enabled) {
    if (enabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    s_config_.enable_depth_test = enabled;
    CheckGLError("SetDepthTest");
}

void Renderer::SetBlending(bool enabled) {
    if (enabled) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
    s_config_.enable_blend = enabled;
    CheckGLError("SetBlending");
}

void Renderer::SetCulling(bool enabled) {
    if (enabled) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    } else {
        glDisable(GL_CULL_FACE);
    }
    s_config_.enable_culling = enabled;
    CheckGLError("SetCulling");
}

void Renderer::SetWireframe(bool enabled) {
    if (enabled) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    CheckGLError("SetWireframe");
}

const RenderStats& Renderer::GetStats() {
    return s_stats_;
}

SpriteRenderer* Renderer::GetSpriteRenderer() {
    return s_sprite_renderer_.get();
}

BatchRenderer* Renderer::GetBatchRenderer() {
    return s_batch_renderer_.get();
}

std::string Renderer::GetRendererInfo() {
    if (!s_initialized_) {
        return "Renderer not initialized";
    }
    
    std::ostringstream info;
    info << "PyNovaGE Renderer\n";
    info << "API: OpenGL\n";
    
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* shading_version = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    if (version) info << "Version: " << version << "\n";
    if (vendor) info << "Vendor: " << vendor << "\n";
    if (renderer) info << "Renderer: " << renderer << "\n";
    if (shading_version) info << "Shading Language: " << shading_version << "\n";
    
    // Get limits
    int max_texture_size, max_texture_units, max_vertex_attribs;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_texture_units);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);
    
    info << "Max Texture Size: " << max_texture_size << "\n";
    info << "Max Texture Units: " << max_texture_units << "\n";
    info << "Max Vertex Attributes: " << max_vertex_attribs << "\n";
    
    return info.str();
}

bool Renderer::CheckGLError(const std::string& operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::string error_string;
        switch (error) {
            case GL_INVALID_ENUM:
                error_string = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error_string = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error_string = "GL_INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error_string = "GL_OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error_string = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            default:
                error_string = "Unknown error (" + std::to_string(error) + ")";
                break;
        }
        
        std::cerr << "OpenGL Error";
        if (!operation.empty()) {
            std::cerr << " (" << operation << ")";
        }
        std::cerr << ": " << error_string << std::endl;
        return false;
    }
    return true;
}

// RendererGuard implementation
RendererGuard::RendererGuard(const RendererConfig& config) {
    initialized_ = Renderer::Initialize(config);
}

RendererGuard::~RendererGuard() {
    if (initialized_) {
        Renderer::Shutdown();
    }
}

} // namespace Renderer
} // namespace PyNovaGE