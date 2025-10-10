#include "renderer/shader.hpp"
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace PyNovaGE {
namespace Renderer {

// Shader implementation
Shader::Shader() {
    // Default constructor creates invalid shader
}

Shader::Shader(const std::string& vertex_source, const std::string& fragment_source) {
    LoadFromSource(vertex_source, fragment_source);
}

Shader::Shader(const std::string& vertex_source, const std::string& fragment_source, const std::string& geometry_source) {
    LoadFromSource(vertex_source, fragment_source, geometry_source);
}

Shader::~Shader() {
    Cleanup();
}

Shader::Shader(Shader&& other) noexcept 
    : program_id_(other.program_id_)
    , uniform_locations_(std::move(other.uniform_locations_))
    , error_log_(std::move(other.error_log_))
    , name_(std::move(other.name_)) {
    other.program_id_ = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        Cleanup();
        
        program_id_ = other.program_id_;
        uniform_locations_ = std::move(other.uniform_locations_);
        error_log_ = std::move(other.error_log_);
        name_ = std::move(other.name_);
        
        other.program_id_ = 0;
    }
    return *this;
}

bool Shader::LoadFromSource(const std::string& vertex_source, const std::string& fragment_source) {
    return LoadFromSource(vertex_source, fragment_source, "");
}

bool Shader::LoadFromSource(const std::string& vertex_source, const std::string& fragment_source, const std::string& geometry_source) {
    Cleanup();
    error_log_.clear();
    
    if (vertex_source.empty() || fragment_source.empty()) {
        error_log_ = "Vertex and fragment shader sources cannot be empty";
        return false;
    }
    
    // Compile vertex shader
    unsigned int vertex_id = CompileShader(ShaderType::Vertex, vertex_source);
    if (vertex_id == 0) {
        return false;
    }
    
    // Compile fragment shader
    unsigned int fragment_id = CompileShader(ShaderType::Fragment, fragment_source);
    if (fragment_id == 0) {
        glDeleteShader(vertex_id);
        return false;
    }
    
    // Compile geometry shader if provided
    unsigned int geometry_id = 0;
    if (!geometry_source.empty()) {
        geometry_id = CompileShader(ShaderType::Geometry, geometry_source);
        if (geometry_id == 0) {
            glDeleteShader(vertex_id);
            glDeleteShader(fragment_id);
            return false;
        }
    }
    
    // Link program
    bool success = LinkProgram(vertex_id, fragment_id, geometry_id);
    
    // Clean up individual shaders (they're now linked into the program)
    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);
    if (geometry_id != 0) {
        glDeleteShader(geometry_id);
    }
    
    return success;
}

bool Shader::LoadFromFile(const std::string& vertex_path, const std::string& fragment_path) {
    return LoadFromFile(vertex_path, fragment_path, "");
}

bool Shader::LoadFromFile(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path) {
    std::string vertex_source = ReadFile(vertex_path);
    if (vertex_source.empty()) {
        error_log_ = "Failed to read vertex shader file: " + vertex_path;
        return false;
    }
    
    std::string fragment_source = ReadFile(fragment_path);
    if (fragment_source.empty()) {
        error_log_ = "Failed to read fragment shader file: " + fragment_path;
        return false;
    }
    
    std::string geometry_source;
    if (!geometry_path.empty()) {
        geometry_source = ReadFile(geometry_path);
        if (geometry_source.empty()) {
            error_log_ = "Failed to read geometry shader file: " + geometry_path;
            return false;
        }
    }
    
    return LoadFromSource(vertex_source, fragment_source, geometry_source);
}

void Shader::Bind() const {
    if (program_id_ != 0) {
        glUseProgram(program_id_);
    }
}

void Shader::Unbind() {
    glUseProgram(0);
}

void Shader::SetInt(const std::string& name, int value) {
    int location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform1i(location, value);
    }
}

void Shader::SetFloat(const std::string& name, float value) {
    int location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform1f(location, value);
    }
}

void Shader::SetVector2f(const std::string& name, const Vector2f& value) {
    int location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform2f(location, value.x, value.y);
    }
}

void Shader::SetVector3f(const std::string& name, const Vector3f& value) {
    int location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform3f(location, value.x, value.y, value.z);
    }
}

void Shader::SetVector4f(const std::string& name, const Vector4f& value) {
    int location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }
}

// Matrix methods temporarily disabled until matrix classes are implemented
// void Shader::SetMatrix3f(const std::string& name, const Matrix3f& value) {
//     (void)name;
//     (void)value;
// }
// 
// void Shader::SetMatrix4f(const std::string& name, const Matrix4f& value) {
//     (void)name;
//     (void)value;
// }

void Shader::SetIntArray(const std::string& name, const int* values, int count) {
    int location = GetUniformLocation(name);
    if (location >= 0 && values != nullptr && count > 0) {
        glUniform1iv(location, count, values);
    }
}

void Shader::SetFloatArray(const std::string& name, const float* values, int count) {
    int location = GetUniformLocation(name);
    if (location >= 0 && values != nullptr && count > 0) {
        glUniform1fv(location, count, values);
    }
}

int Shader::GetUniformLocation(const std::string& name) {
    if (program_id_ == 0) {
        return -1;
    }
    
    // Check cache first
    auto it = uniform_locations_.find(name);
    if (it != uniform_locations_.end()) {
        return it->second;
    }
    
    // Get location from OpenGL and cache it
    int location = glGetUniformLocation(program_id_, name.c_str());
    uniform_locations_[name] = location;
    
    if (location == -1) {
        static std::unordered_map<std::string, bool> warned_uniforms;
        if (warned_uniforms.find(name) == warned_uniforms.end()) {
            std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
            warned_uniforms[name] = true;
        }
    }
    
    return location;
}

Shader Shader::Create(const std::string& name, const std::string& vertex_source, const std::string& fragment_source) {
    Shader shader(vertex_source, fragment_source);
    shader.name_ = name;
    return shader;
}

unsigned int Shader::CompileShader(ShaderType type, const std::string& source) {
    GLenum shader_type;
    std::string type_name;
    
    switch (type) {
        case ShaderType::Vertex:
            shader_type = GL_VERTEX_SHADER;
            type_name = "vertex";
            break;
        case ShaderType::Fragment:
            shader_type = GL_FRAGMENT_SHADER;
            type_name = "fragment";
            break;
        case ShaderType::Geometry:
#ifdef GL_GEOMETRY_SHADER
            shader_type = GL_GEOMETRY_SHADER;
#else
            shader_type = 0x8DD9; // GL_GEOMETRY_SHADER value
#endif
            type_name = "geometry";
            break;
        default:
            error_log_ = "Unknown shader type";
            return 0;
    }
    
    // Create shader
    unsigned int shader_id = glCreateShader(shader_type);
    if (shader_id == 0) {
        error_log_ = "Failed to create " + type_name + " shader";
        return 0;
    }
    
    // Compile shader
    const char* source_ptr = source.c_str();
    glShaderSource(shader_id, 1, &source_ptr, nullptr);
    glCompileShader(shader_id);
    
    // Check compilation status
    GLint success;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        GLint log_length;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
        
        std::vector<char> log(log_length);
        glGetShaderInfoLog(shader_id, log_length, nullptr, log.data());
        
        error_log_ = "Failed to compile " + type_name + " shader:\n" + std::string(log.data());
        
        glDeleteShader(shader_id);
        return 0;
    }
    
    return shader_id;
}

bool Shader::LinkProgram(unsigned int vertex_id, unsigned int fragment_id, unsigned int geometry_id) {
    // Create program
    program_id_ = glCreateProgram();
    if (program_id_ == 0) {
        error_log_ = "Failed to create shader program";
        return false;
    }
    
    // Attach shaders
    glAttachShader(program_id_, vertex_id);
    glAttachShader(program_id_, fragment_id);
    if (geometry_id != 0) {
        glAttachShader(program_id_, geometry_id);
    }
    
    // Link program
    glLinkProgram(program_id_);
    
    // Check linking status
    GLint success;
    glGetProgramiv(program_id_, GL_LINK_STATUS, &success);
    
    if (!success) {
        GLint log_length;
        glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &log_length);
        
        std::vector<char> log(log_length);
        glGetProgramInfoLog(program_id_, log_length, nullptr, log.data());
        
        error_log_ = "Failed to link shader program:\n" + std::string(log.data());
        
        glDeleteProgram(program_id_);
        program_id_ = 0;
        return false;
    }
    
    // Note: glDetachShader calls removed due to compatibility issues
    // Shaders are automatically cleaned up when the program is deleted
    
    return true;
}

std::string Shader::ReadFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return "";
    }
    
    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();
    
    return contents.str();
}

void Shader::Cleanup() {
    if (program_id_ != 0) {
        glDeleteProgram(program_id_);
        program_id_ = 0;
    }
    uniform_locations_.clear();
}

// ShaderLibrary implementation
ShaderLibrary& ShaderLibrary::Instance() {
    static ShaderLibrary instance;
    return instance;
}

bool ShaderLibrary::LoadShader(const std::string& name, const std::string& vertex_source, const std::string& fragment_source) {
    if (name.empty() || vertex_source.empty() || fragment_source.empty()) {
        return false;
    }
    
    auto shader = std::make_unique<Shader>(vertex_source, fragment_source);
    if (!shader->IsValid()) {
        std::cerr << "Failed to create shader '" << name << "': " << shader->GetErrorLog() << std::endl;
        return false;
    }
    
    shader->name_ = name;
    shaders_[name] = std::move(shader);
    return true;
}

bool ShaderLibrary::LoadShaderFromFile(const std::string& name, const std::string& vertex_path, const std::string& fragment_path) {
    if (name.empty() || vertex_path.empty() || fragment_path.empty()) {
        return false;
    }
    
    auto shader = std::make_unique<Shader>();
    if (!shader->LoadFromFile(vertex_path, fragment_path)) {
        std::cerr << "Failed to load shader '" << name << "' from files: " << shader->GetErrorLog() << std::endl;
        return false;
    }
    
    shader->name_ = name;
    shaders_[name] = std::move(shader);
    return true;
}

Shader* ShaderLibrary::GetShader(const std::string& name) {
    auto it = shaders_.find(name);
    return (it != shaders_.end()) ? it->second.get() : nullptr;
}

bool ShaderLibrary::HasShader(const std::string& name) const {
    return shaders_.find(name) != shaders_.end();
}

void ShaderLibrary::RemoveShader(const std::string& name) {
    shaders_.erase(name);
}

void ShaderLibrary::Clear() {
    shaders_.clear();
}

void ShaderLibrary::LoadDefaultShaders() {
    // Basic 2D sprite shader
    const std::string sprite_vertex = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 model;

void main()
{
    gl_Position = projection * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";

    const std::string sprite_fragment = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform vec4 color;

void main()
{
    FragColor = texture(texture1, TexCoord) * color;
}
)";

    // Basic color shader (no texture)
    const std::string color_vertex = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 model;

void main()
{
    gl_Position = projection * model * vec4(aPos, 1.0);
}
)";

    const std::string color_fragment = R"(
#version 330 core
out vec4 FragColor;

uniform vec4 color;

void main()
{
    FragColor = color;
}
)";

    // Load shaders
    LoadShader("sprite", sprite_vertex, sprite_fragment);
    LoadShader("color", color_vertex, color_fragment);
    
    std::cout << "Default shaders loaded: sprite, color" << std::endl;
}

} // namespace Renderer
} // namespace PyNovaGE