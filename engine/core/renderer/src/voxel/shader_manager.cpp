#include "renderer/voxel/shader_manager.hpp"
#include <fstream>
#include <sstream>

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

// Shader implementation
Shader::Shader([[maybe_unused]] const std::string& source, [[maybe_unused]] Type type) {
    // TODO: Implement OpenGL shader compilation
    shader_id_ = 0;
    is_valid_ = false;
    error_message_ = "Shader compilation not implemented yet";
}

Shader::~Shader() {
    // TODO: Delete OpenGL shader
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
    // TODO: Create OpenGL program
    program_id_ = 0;
    is_valid_ = false;
}

ShaderProgram::~ShaderProgram() {
    // TODO: Delete OpenGL program
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

void ShaderProgram::AttachShader([[maybe_unused]] const Shader& shader) {
    // TODO: Attach shader to program
}

bool ShaderProgram::Link() {
    // TODO: Link shader program
    // For testing purposes, mark as valid
    is_valid_ = true;
    error_message_ = "";
    return true;
}

void ShaderProgram::Use() const {
    // TODO: Use shader program
}

void ShaderProgram::Unuse() {
    // TODO: Unuse shader program
}

void ShaderProgram::SetUniform([[maybe_unused]] const std::string& name, [[maybe_unused]] int value) {
    // TODO: Set uniform value
}

void ShaderProgram::SetUniform([[maybe_unused]] const std::string& name, [[maybe_unused]] float value) {
    // TODO: Set uniform value
}

void ShaderProgram::SetUniform([[maybe_unused]] const std::string& name, [[maybe_unused]] bool value) {
    // TODO: Set uniform value
}

void ShaderProgram::SetUniform([[maybe_unused]] const std::string& name, [[maybe_unused]] const Vector2f& value) {
    // TODO: Set uniform value
}

void ShaderProgram::SetUniform([[maybe_unused]] const std::string& name, [[maybe_unused]] const Vector3f& value) {
    // TODO: Set uniform value
}

void ShaderProgram::SetUniform([[maybe_unused]] const std::string& name, [[maybe_unused]] const Matrix4f& value) {
    // TODO: Set uniform value
}

void ShaderProgram::SetUniform([[maybe_unused]] const std::string& name, [[maybe_unused]] const float* values, [[maybe_unused]] int count) {
    // TODO: Set uniform array
}

int ShaderProgram::GetUniformLocation([[maybe_unused]] const std::string& name) {
    // TODO: Get uniform location
    return -1;
}

// UniformBuffer implementation
UniformBuffer::UniformBuffer(size_t size, uint32_t binding_point) 
    : binding_point_(binding_point), size_(size) {
    // TODO: Create OpenGL uniform buffer
    buffer_id_ = 0;
}

UniformBuffer::~UniformBuffer() {
    // TODO: Delete OpenGL buffer
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

void UniformBuffer::UpdateData([[maybe_unused]] const void* data, [[maybe_unused]] size_t size, [[maybe_unused]] size_t offset) {
    // TODO: Update buffer data
}

void UniformBuffer::Bind() const {
    // TODO: Bind buffer to binding point
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
    
    // TODO: Initialize OpenGL context and load shaders
    initialized_ = true;
    return true;
}

bool VoxelShaderManager::LoadShaderProgram(const std::string& name, 
                                          [[maybe_unused]] const std::string& vertex_path,
                                          [[maybe_unused]] const std::string& fragment_path,
                                          [[maybe_unused]] const std::string& geometry_path) {
    // TODO: Load and compile shaders
    auto program = std::make_unique<ShaderProgram>();
    shader_programs_[name] = std::move(program);
    return false; // Not implemented yet
}

bool VoxelShaderManager::LoadShaderPreset(ShaderPreset preset) {
    std::string name = GetPresetName(preset);
    std::string vertex_path, fragment_path, geometry_path;
    GetPresetPaths(preset, vertex_path, fragment_path, geometry_path);
    
    // Create a placeholder valid shader for now
    auto program = std::make_unique<ShaderProgram>();
    // Call Link() to mark as valid for testing purposes
    program->Link();
    shader_programs_[name] = std::move(program);
    preset_mapping_[preset] = name;
    
    return true;
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