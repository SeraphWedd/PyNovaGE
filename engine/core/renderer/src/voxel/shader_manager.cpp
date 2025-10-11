#include "renderer/voxel/shader_manager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <glad/gl.h>

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

// Shader implementation
Shader::Shader(const std::string& source, Type type) {
    // Create shader
    GLenum shader_type;
    switch (type) {
        case Type::Vertex:
            shader_type = GL_VERTEX_SHADER;
            break;
        case Type::Fragment:
            shader_type = GL_FRAGMENT_SHADER;
            break;
        case Type::Geometry:
            shader_type = GL_GEOMETRY_SHADER;
            break;
        default:
            shader_id_ = 0;
            is_valid_ = false;
            error_message_ = "Unknown shader type";
            return;
    }
    
    shader_id_ = glCreateShader(shader_type);
    if (shader_id_ == 0) {
        is_valid_ = false;
        error_message_ = "Failed to create shader";
        return;
    }
    
    // Compile shader
    const char* source_cstr = source.c_str();
    glShaderSource(shader_id_, 1, &source_cstr, nullptr);
    glCompileShader(shader_id_);
    
    // Check compilation status
    GLint success;
    glGetShaderiv(shader_id_, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint log_length;
        glGetShaderiv(shader_id_, GL_INFO_LOG_LENGTH, &log_length);
        
        std::vector<char> log(log_length);
        glGetShaderInfoLog(shader_id_, log_length, nullptr, log.data());
        
        error_message_ = "Shader compilation failed: " + std::string(log.data());
        
        glDeleteShader(shader_id_);
        shader_id_ = 0;
        is_valid_ = false;
        return;
    }
    
    is_valid_ = true;
    error_message_ = "";
}

Shader::~Shader() {
    if (shader_id_ != 0) {
        glDeleteShader(shader_id_);
        shader_id_ = 0;
    }
}

Shader::Shader(Shader&& other) noexcept 
    : shader_id_(other.shader_id_), is_valid_(other.is_valid_), error_message_(std::move(other.error_message_)) {
    other.shader_id_ = 0;
    other.is_valid_ = false;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        shader_id_ = other.shader_id_;
        is_valid_ = other.is_valid_;
        error_message_ = std::move(other.error_message_);
        
        other.shader_id_ = 0;
        other.is_valid_ = false;
    }
    return *this;
}

// ShaderProgram implementation
ShaderProgram::ShaderProgram() {
    program_id_ = glCreateProgram();
    is_valid_ = (program_id_ != 0);
    if (!is_valid_) {
        error_message_ = "Failed to create OpenGL program";
    }
}

ShaderProgram::~ShaderProgram() {
    if (program_id_ != 0) {
        glDeleteProgram(program_id_);
        program_id_ = 0;
    }
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept 
    : program_id_(other.program_id_), is_valid_(other.is_valid_), 
      error_message_(std::move(other.error_message_)), uniform_cache_(std::move(other.uniform_cache_)) {
    other.program_id_ = 0;
    other.is_valid_ = false;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    if (this != &other) {
        program_id_ = other.program_id_;
        is_valid_ = other.is_valid_;
        error_message_ = std::move(other.error_message_);
        uniform_cache_ = std::move(other.uniform_cache_);
        
        other.program_id_ = 0;
        other.is_valid_ = false;
    }
    return *this;
}

void ShaderProgram::AttachShader(const Shader& shader) {
    if (program_id_ != 0 && shader.IsValid()) {
        glAttachShader(program_id_, shader.GetID());
    }
}

bool ShaderProgram::Link() {
    if (program_id_ == 0) {
        is_valid_ = false;
        error_message_ = "Cannot link invalid program";
        return false;
    }
    
    glLinkProgram(program_id_);
    
    // Check linking status
    GLint success;
    glGetProgramiv(program_id_, GL_LINK_STATUS, &success);
    if (!success) {
        GLint log_length;
        glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &log_length);
        
        std::vector<char> log(log_length);
        glGetProgramInfoLog(program_id_, log_length, nullptr, log.data());
        
        error_message_ = "Program linking failed: " + std::string(log.data());
        is_valid_ = false;
        return false;
    }
    
    is_valid_ = true;
    error_message_ = "";
    return true;
}

void ShaderProgram::Use() const {
    if (is_valid_ && program_id_ != 0) {
        glUseProgram(program_id_);
    }
}

void ShaderProgram::Unuse() {
    glUseProgram(0);
}

void ShaderProgram::SetUniform(const std::string& name, int value) {
    int location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform1i(location, value);
    }
}

void ShaderProgram::SetUniform(const std::string& name, float value) {
    int location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform1f(location, value);
    }
}

void ShaderProgram::SetUniform(const std::string& name, bool value) {
    int location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform1i(location, value ? 1 : 0);
    }
}

void ShaderProgram::SetUniform(const std::string& name, const Vector2f& value) {
    int location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform2f(location, value.x, value.y);
    }
}

void ShaderProgram::SetUniform(const std::string& name, const Vector3f& value) {
    int location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform3f(location, value.x, value.y, value.z);
    }
}

void ShaderProgram::SetUniform(const std::string& name, const Matrix4f& value) {
    int location = GetUniformLocation(name);
    if (location >= 0) {
        // Matrix4f is stored in row-major order; OpenGL expects column-major by default.
        // Pass GL_TRUE to transpose on upload so the shader receives the correct matrices.
        glUniformMatrix4fv(location, 1, GL_TRUE, value.data.data());
    }
}

void ShaderProgram::SetUniform(const std::string& name, const float* values, int count) {
    int location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform1fv(location, count, values);
    }
}

int ShaderProgram::GetUniformLocation(const std::string& name) {
    if (!is_valid_ || program_id_ == 0) {
        return -1;
    }
    
    // Check cache first
    auto it = uniform_cache_.find(name);
    if (it != uniform_cache_.end()) {
        return it->second;
    }
    
    // Get location from OpenGL
    int location = glGetUniformLocation(program_id_, name.c_str());
    uniform_cache_[name] = location;
    
    return location;
}

// UniformBuffer implementation
UniformBuffer::UniformBuffer(size_t size, uint32_t binding_point) 
    : binding_point_(binding_point), size_(size) {
    // Don't create OpenGL resources immediately - defer until first use
    buffer_id_ = 0;
}

UniformBuffer::~UniformBuffer() {
    if (buffer_id_ != 0) {
        glDeleteBuffers(1, &buffer_id_);
        buffer_id_ = 0;
    }
}

UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    : buffer_id_(other.buffer_id_), binding_point_(other.binding_point_), size_(other.size_) {
    other.buffer_id_ = 0;
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept {
    if (this != &other) {
        buffer_id_ = other.buffer_id_;
        binding_point_ = other.binding_point_;
        size_ = other.size_;
        other.buffer_id_ = 0;
    }
    return *this;
}

void UniformBuffer::UpdateData(const void* data, size_t size, size_t offset) {
    // Lazy initialization - create buffer if it doesn't exist yet
    if (buffer_id_ == 0) {
        glGenBuffers(1, &buffer_id_);
        if (buffer_id_ != 0) {
            glBindBuffer(GL_UNIFORM_BUFFER, buffer_id_);
            glBufferData(GL_UNIFORM_BUFFER, size_, nullptr, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_, buffer_id_);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
    }
    
    if (buffer_id_ != 0 && data != nullptr && (offset + size) <= size_) {
        glBindBuffer(GL_UNIFORM_BUFFER, buffer_id_);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

void UniformBuffer::Bind() const {
    // For const correctness, we can't do lazy initialization here
    // The buffer should be initialized through UpdateData first
    if (buffer_id_ != 0) {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_, buffer_id_);
    }
}

// VoxelShaderManager implementation
VoxelShaderManager::VoxelShaderManager(const std::string& shader_directory)
    : shader_directory_(shader_directory)
    , camera_matrices_buffer_(sizeof(CameraMatrices), 0)
    , lighting_data_buffer_(sizeof(LightingData), 1) {
}

bool VoxelShaderManager::Initialize() {
    if (initialized_) {
        return true;
    }
    
    std::cout << "VoxelShaderManager::Initialize() - Starting initialization..." << std::endl;
    
    initialized_ = true;
    std::cout << "VoxelShaderManager::Initialize() - Initialization complete!" << std::endl;
    return true;
}

bool VoxelShaderManager::LoadShaderProgram(const std::string& name, 
                                          const std::string& vertex_path,
                                          const std::string& fragment_path,
                                          const std::string& geometry_path) {
    // Load shader sources
    std::string vertex_source = LoadShaderSource(vertex_path);
    std::string fragment_source = LoadShaderSource(fragment_path);
    
    if (vertex_source.empty()) {
        std::cerr << "Failed to load vertex shader: " << vertex_path << std::endl;
        return false;
    }
    
    if (fragment_source.empty()) {
        std::cerr << "Failed to load fragment shader: " << fragment_path << std::endl;
        return false;
    }
    
    // Create and compile shaders
    Shader vertex_shader(vertex_source, Shader::Type::Vertex);
    if (!vertex_shader.IsValid()) {
        std::cerr << "Vertex shader compilation failed: " << vertex_shader.GetErrorMessage() << std::endl;
        return false;
    }
    
    Shader fragment_shader(fragment_source, Shader::Type::Fragment);
    if (!fragment_shader.IsValid()) {
        std::cerr << "Fragment shader compilation failed: " << fragment_shader.GetErrorMessage() << std::endl;
        return false;
    }
    
    // Create program and attach shaders
    auto program = std::make_unique<ShaderProgram>();
    program->AttachShader(vertex_shader);
    program->AttachShader(fragment_shader);
    
    // Handle geometry shader if provided
    std::unique_ptr<Shader> geometry_shader;
    if (!geometry_path.empty()) {
        std::string geometry_source = LoadShaderSource(geometry_path);
        if (!geometry_source.empty()) {
            geometry_shader = std::make_unique<Shader>(geometry_source, Shader::Type::Geometry);
            if (geometry_shader->IsValid()) {
                program->AttachShader(*geometry_shader);
            }
        }
    }
    
    // Link program
    if (!program->Link()) {
        std::cerr << "Shader program linking failed: " << program->GetErrorMessage() << std::endl;
        return false;
    }
    
    shader_programs_[name] = std::move(program);
    return true;
}

bool VoxelShaderManager::LoadShaderPreset(ShaderPreset preset) {
    std::string name = GetPresetName(preset);
    std::string vertex_path, fragment_path, geometry_path;
    GetPresetPaths(preset, vertex_path, fragment_path, geometry_path);
    
    std::cout << "Loading shader preset: " << name << std::endl;
    std::cout << "  Vertex: " << vertex_path << std::endl;
    std::cout << "  Fragment: " << fragment_path << std::endl;
    
    bool success = LoadShaderProgram(name, vertex_path, fragment_path, geometry_path);
    if (success) {
        preset_mapping_[preset] = name;
        std::cout << "  Shader preset '" << name << "' loaded successfully!" << std::endl;
    } else {
        std::cout << "  Failed to load shader preset '" << name << "'!" << std::endl;
    }
    
    return success;
}

ShaderProgram* VoxelShaderManager::GetShaderProgram(const std::string& name) {
    auto it = shader_programs_.find(name);
    return it != shader_programs_.end() ? it->second.get() : nullptr;
}

ShaderProgram* VoxelShaderManager::GetShaderProgram(ShaderPreset preset) {
    auto it = preset_mapping_.find(preset);
    if (it != preset_mapping_.end()) {
        return GetShaderProgram(it->second);
    }
    return nullptr;
}

bool VoxelShaderManager::ReloadShaders() {
    // TODO: Reload all shaders from disk
    return false;
}

void VoxelShaderManager::UpdateCameraMatrices(const CameraMatrices& matrices) {
    camera_matrices_buffer_.UpdateData(&matrices, sizeof(matrices));
}

void VoxelShaderManager::UpdateLightingData(const LightingData& lighting) {
    lighting_data_buffer_.UpdateData(&lighting, sizeof(lighting));
}

void VoxelShaderManager::Update() {
    // TODO: Handle hot reloading if enabled
}

std::string VoxelShaderManager::LoadShaderSource(const std::string& filepath) {
    std::ifstream file(shader_directory_ + filepath);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void VoxelShaderManager::GetPresetPaths(ShaderPreset preset,
                                       std::string& vertex_path,
                                       std::string& fragment_path,
                                       [[maybe_unused]] std::string& geometry_path) {
    switch (preset) {
        case ShaderPreset::Standard:
            vertex_path = "voxel.vert";
            fragment_path = "voxel.frag";
            break;
        case ShaderPreset::Lit:
            vertex_path = "voxel_lit.vert";
            fragment_path = "voxel_lit.frag";
            break;
        case ShaderPreset::Transparent:
            vertex_path = "voxel_transparent.vert";
            fragment_path = "voxel_transparent.frag";
            break;
        case ShaderPreset::Emissive:
            vertex_path = "voxel_emissive.vert";
            fragment_path = "voxel_emissive.frag";
            break;
        case ShaderPreset::Wireframe:
            vertex_path = "voxel_wireframe.vert";
            fragment_path = "voxel_wireframe.frag";
            break;
        case ShaderPreset::Shadow:
            vertex_path = "voxel_shadow.vert";
            fragment_path = "voxel_shadow.frag";
            break;
    }
}

std::string VoxelShaderManager::GetPresetName(ShaderPreset preset) {
    switch (preset) {
        case ShaderPreset::Standard: return "standard";
        case ShaderPreset::Lit: return "lit";
        case ShaderPreset::Transparent: return "transparent";
        case ShaderPreset::Emissive: return "emissive";
        case ShaderPreset::Wireframe: return "wireframe";
        case ShaderPreset::Shadow: return "shadow";
        default: return "unknown";
    }
}

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE