#pragma once

#include <vectors/vector2.hpp>
#include <vectors/vector3.hpp>
#include <vectors/vector4.hpp>
// Matrix headers temporarily commented out - will add when matrices are implemented
// #include <matrix/matrix3.hpp>
// #include <matrix/matrix4.hpp>
#include <string>
#include <unordered_map>
#include <memory>

namespace PyNovaGE {
namespace Renderer {

/**
 * @brief Shader types supported by the engine
 */
enum class ShaderType {
    Vertex,
    Fragment,
    Geometry
};

/**
 * @brief OpenGL shader wrapper class
 * 
 * Provides functionality to load, compile, and use OpenGL shaders
 * with automatic uniform location caching and type-safe uniform setting.
 */
class Shader {
    friend class ShaderLibrary;
public:
    /**
     * @brief Default constructor creates invalid shader
     */
    Shader();
    
    /**
     * @brief Constructor with vertex and fragment shader source
     * @param vertex_source Vertex shader source code
     * @param fragment_source Fragment shader source code
     */
    Shader(const std::string& vertex_source, const std::string& fragment_source);
    
    /**
     * @brief Constructor with vertex, fragment, and geometry shader source
     * @param vertex_source Vertex shader source code
     * @param fragment_source Fragment shader source code
     * @param geometry_source Geometry shader source code
     */
    Shader(const std::string& vertex_source, const std::string& fragment_source, const std::string& geometry_source);
    
    /**
     * @brief Destructor automatically cleans up OpenGL resources
     */
    ~Shader();
    
    // Non-copyable but movable
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;
    
    /**
     * @brief Load shader from source strings
     * @param vertex_source Vertex shader source
     * @param fragment_source Fragment shader source
     * @return true if successful, false otherwise
     */
    bool LoadFromSource(const std::string& vertex_source, const std::string& fragment_source);
    
    /**
     * @brief Load shader from source strings including geometry shader
     * @param vertex_source Vertex shader source
     * @param fragment_source Fragment shader source
     * @param geometry_source Geometry shader source
     * @return true if successful, false otherwise
     */
    bool LoadFromSource(const std::string& vertex_source, const std::string& fragment_source, const std::string& geometry_source);
    
    /**
     * @brief Load shader from files
     * @param vertex_path Path to vertex shader file
     * @param fragment_path Path to fragment shader file
     * @return true if successful, false otherwise
     */
    bool LoadFromFile(const std::string& vertex_path, const std::string& fragment_path);
    
    /**
     * @brief Load shader from files including geometry shader
     * @param vertex_path Path to vertex shader file
     * @param fragment_path Path to fragment shader file
     * @param geometry_path Path to geometry shader file
     * @return true if successful, false otherwise
     */
    bool LoadFromFile(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path);
    
    /**
     * @brief Bind this shader for use
     */
    void Bind() const;
    
    /**
     * @brief Unbind shader (bind 0)
     */
    static void Unbind();
    
    /**
     * @brief Check if shader is valid and ready to use
     */
    bool IsValid() const { return program_id_ != 0; }
    
    /**
     * @brief Get OpenGL program ID
     */
    unsigned int GetProgramID() const { return program_id_; }
    
    // Uniform setting methods
    /**
     * @brief Set int uniform
     */
    void SetInt(const std::string& name, int value);
    
    /**
     * @brief Set float uniform
     */
    void SetFloat(const std::string& name, float value);
    
    /**
     * @brief Set Vector2f uniform
     */
    void SetVector2f(const std::string& name, const Vector2f& value);
    
    /**
     * @brief Set Vector3f uniform
     */
    void SetVector3f(const std::string& name, const Vector3f& value);
    
    /**
     * @brief Set Vector4f uniform
     */
    void SetVector4f(const std::string& name, const Vector4f& value);
    
    /**
     * @brief Set Matrix3f uniform (temporarily disabled)
     */
    // void SetMatrix3f(const std::string& name, const Matrix3f& value);
    
    /**
     * @brief Set Matrix4f uniform (temporarily disabled)
     */
    // void SetMatrix4f(const std::string& name, const Matrix4f& value);
    
    /**
     * @brief Set int array uniform
     */
    void SetIntArray(const std::string& name, const int* values, int count);
    
    /**
     * @brief Set float array uniform
     */
    void SetFloatArray(const std::string& name, const float* values, int count);
    
    /**
     * @brief Get uniform location (cached)
     */
    int GetUniformLocation(const std::string& name);
    
    /**
     * @brief Get shader compilation/linking error log
     */
    const std::string& GetErrorLog() const { return error_log_; }

    /**
     * @brief Create shader from embedded source strings
     * @param name Shader name for debugging
     * @param vertex_source Vertex shader source
     * @param fragment_source Fragment shader source
     * @return Shader instance
     */
    static Shader Create(const std::string& name, const std::string& vertex_source, const std::string& fragment_source);

private:
    /**
     * @brief Compile individual shader stage
     * @param type Shader type (vertex, fragment, geometry)
     * @param source Shader source code
     * @return Compiled shader ID or 0 on failure
     */
    unsigned int CompileShader(ShaderType type, const std::string& source);
    
    /**
     * @brief Link shader program
     * @param vertex_id Vertex shader ID
     * @param fragment_id Fragment shader ID
     * @param geometry_id Geometry shader ID (0 if not used)
     * @return true if successful, false otherwise
     */
    bool LinkProgram(unsigned int vertex_id, unsigned int fragment_id, unsigned int geometry_id = 0);
    
    /**
     * @brief Read file contents
     * @param filepath Path to file
     * @return File contents or empty string on error
     */
    std::string ReadFile(const std::string& filepath);
    
    /**
     * @brief Cleanup OpenGL resources
     */
    void Cleanup();
    
    unsigned int program_id_ = 0;
    std::unordered_map<std::string, int> uniform_locations_;
    std::string error_log_;
    std::string name_; // For debugging
};

/**
 * @brief Shader library for managing common shaders
 */
class ShaderLibrary {
public:
    /**
     * @brief Get the singleton instance
     */
    static ShaderLibrary& Instance();
    
    /**
     * @brief Load and store a shader
     * @param name Shader name
     * @param vertex_source Vertex shader source
     * @param fragment_source Fragment shader source
     * @return true if successful, false otherwise
     */
    bool LoadShader(const std::string& name, const std::string& vertex_source, const std::string& fragment_source);
    
    /**
     * @brief Load shader from files
     * @param name Shader name
     * @param vertex_path Vertex shader file path
     * @param fragment_path Fragment shader file path
     * @return true if successful, false otherwise
     */
    bool LoadShaderFromFile(const std::string& name, const std::string& vertex_path, const std::string& fragment_path);
    
    /**
     * @brief Get a shader by name
     * @param name Shader name
     * @return Shader pointer or nullptr if not found
     */
    Shader* GetShader(const std::string& name);
    
    /**
     * @brief Check if shader exists
     * @param name Shader name
     * @return true if exists, false otherwise
     */
    bool HasShader(const std::string& name) const;
    
    /**
     * @brief Remove a shader
     * @param name Shader name
     */
    void RemoveShader(const std::string& name);
    
    /**
     * @brief Clear all shaders
     */
    void Clear();
    
    /**
     * @brief Load default engine shaders
     */
    void LoadDefaultShaders();

private:
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders_;
};

} // namespace Renderer
} // namespace PyNovaGE
