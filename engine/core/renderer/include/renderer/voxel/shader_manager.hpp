#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vectors/vector2.hpp>
#include <vectors/vector3.hpp>
#include <matrices/matrix4.hpp>

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

// Type aliases for convenience
using Vector2f = Vector2<float>;
using Vector3f = Vector3<float>;
using Matrix4f = Matrix4<float>;

/**
 * @brief OpenGL shader wrapper
 */
class Shader {
public:
    /**
     * @brief Shader types
     */
    enum class Type {
        Vertex,
        Fragment,
        Geometry,
        Compute
    };

    /**
     * @brief Create shader from source code
     * @param source Shader source code
     * @param type Shader type
     */
    Shader(const std::string& source, Type type);

    /**
     * @brief Destructor - cleans up OpenGL resources
     */
    ~Shader();

    /**
     * @brief Move constructor
     */
    Shader(Shader&& other) noexcept;

    /**
     * @brief Move assignment operator
     */
    Shader& operator=(Shader&& other) noexcept;

    // Disable copy operations
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    /**
     * @brief Get OpenGL shader ID
     */
    uint32_t GetID() const { return shader_id_; }

    /**
     * @brief Check if shader compiled successfully
     */
    bool IsValid() const { return is_valid_; }

    /**
     * @brief Get compilation error message (if any)
     */
    const std::string& GetErrorMessage() const { return error_message_; }

private:
    uint32_t shader_id_ = 0;
    bool is_valid_ = false;
    std::string error_message_;
};

/**
 * @brief OpenGL shader program wrapper
 */
class ShaderProgram {
public:
    /**
     * @brief Create empty shader program
     */
    ShaderProgram();

    /**
     * @brief Destructor - cleans up OpenGL resources
     */
    ~ShaderProgram();

    /**
     * @brief Move constructor
     */
    ShaderProgram(ShaderProgram&& other) noexcept;

    /**
     * @brief Move assignment operator
     */
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

    // Disable copy operations
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    /**
     * @brief Attach shader to program
     * @param shader Shader to attach
     */
    void AttachShader(const Shader& shader);

    /**
     * @brief Link the shader program
     * @return True if linking succeeded
     */
    bool Link();

    /**
     * @brief Use this shader program for rendering
     */
    void Use() const;

    /**
     * @brief Stop using any shader program
     */
    static void Unuse();

    /**
     * @brief Get OpenGL program ID
     */
    uint32_t GetID() const { return program_id_; }

    /**
     * @brief Check if program linked successfully
     */
    bool IsValid() const { return is_valid_; }

    /**
     * @brief Get linking error message (if any)
     */
    const std::string& GetErrorMessage() const { return error_message_; }

    // Uniform setting methods
    void SetUniform(const std::string& name, int value);
    void SetUniform(const std::string& name, float value);
    void SetUniform(const std::string& name, bool value);
    void SetUniform(const std::string& name, const Vector2f& value);
    void SetUniform(const std::string& name, const Vector3f& value);
    void SetUniform(const std::string& name, const Matrix4f& value);
    void SetUniform(const std::string& name, const float* values, int count);

    /**
     * @brief Get uniform location
     * @param name Uniform name
     * @return Uniform location (-1 if not found)
     */
    int GetUniformLocation(const std::string& name);

private:
    uint32_t program_id_ = 0;
    bool is_valid_ = false;
    std::string error_message_;
    std::unordered_map<std::string, int> uniform_cache_;
};

/**
 * @brief Uniform buffer object wrapper
 */
class UniformBuffer {
public:
    /**
     * @brief Create uniform buffer
     * @param size Buffer size in bytes
     * @param binding_point Binding point index
     */
    UniformBuffer(size_t size, uint32_t binding_point);

    /**
     * @brief Destructor
     */
    ~UniformBuffer();

    /**
     * @brief Move constructor
     */
    UniformBuffer(UniformBuffer&& other) noexcept;

    /**
     * @brief Move assignment operator
     */
    UniformBuffer& operator=(UniformBuffer&& other) noexcept;

    // Disable copy operations
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

    /**
     * @brief Update buffer data
     * @param data Data to upload
     * @param size Size of data in bytes
     * @param offset Offset in buffer
     */
    void UpdateData(const void* data, size_t size, size_t offset = 0);

    /**
     * @brief Bind buffer to its binding point
     */
    void Bind() const;

    /**
     * @brief Get OpenGL buffer ID
     */
    uint32_t GetID() const { return buffer_id_; }

private:
    uint32_t buffer_id_ = 0;
    uint32_t binding_point_;
    size_t size_;
};

/**
 * @brief Voxel shader manager
 * 
 * Manages loading, compilation, and caching of voxel rendering shaders.
 * Provides convenience methods for common voxel shader operations.
 */
class VoxelShaderManager {
public:
    /**
     * @brief Predefined shader configurations
     */
    enum class ShaderPreset {
        Standard,      // Basic voxel rendering
        Lit,          // With advanced lighting
        Transparent,  // For transparent voxels
        Emissive,     // For glowing voxels
        Wireframe,    // Wireframe debugging
        Shadow        // Shadow mapping
    };

    /**
     * @brief Camera matrices uniform buffer layout
     */
    struct CameraMatrices {
        Matrix4f view_matrix;
        Matrix4f projection_matrix;
        Matrix4f view_projection_matrix;
        Vector3f camera_position;
        float near_plane;
        float far_plane;
        float fov;
        Vector2f viewport_size;
    };

    /**
     * @brief Lighting data uniform buffer layout
     */
    struct LightingData {
        Vector3f sun_direction;
        Vector3f sun_color;
        float sun_intensity;
        Vector3f ambient_color;
        float ambient_intensity;
        float gamma;
        bool enable_fog;
        Vector3f fog_color;
        float fog_density;
        float fog_start;
        float fog_end;
    };

    /**
     * @brief Constructor - initializes shader manager
     * @param shader_directory Base directory for shader files
     */
    explicit VoxelShaderManager(const std::string& shader_directory = "shaders/voxel/");

    /**
     * @brief Destructor
     */
    ~VoxelShaderManager() = default;

    /**
     * @brief Initialize shader manager and load default shaders
     * @return True if initialization succeeded
     */
    bool Initialize();

    /**
     * @brief Load shader program from files
     * @param name Shader program name
     * @param vertex_path Path to vertex shader
     * @param fragment_path Path to fragment shader
     * @param geometry_path Path to geometry shader (optional)
     * @return True if loading succeeded
     */
    bool LoadShaderProgram(const std::string& name, 
                          const std::string& vertex_path,
                          const std::string& fragment_path,
                          const std::string& geometry_path = "");

    /**
     * @brief Load predefined shader preset
     * @param preset Shader preset to load
     * @return True if loading succeeded
     */
    bool LoadShaderPreset(ShaderPreset preset);

    /**
     * @brief Get shader program by name
     * @param name Shader program name
     * @return Pointer to shader program (null if not found)
     */
    ShaderProgram* GetShaderProgram(const std::string& name);

    /**
     * @brief Get shader program by preset
     * @param preset Shader preset
     * @return Pointer to shader program (null if not found)
     */
    ShaderProgram* GetShaderProgram(ShaderPreset preset);

    /**
     * @brief Reload all shader programs
     * @return True if reload succeeded
     */
    bool ReloadShaders();

    /**
     * @brief Update camera matrices uniform buffer
     * @param matrices Camera matrices data
     */
    void UpdateCameraMatrices(const CameraMatrices& matrices);

    /**
     * @brief Update lighting data uniform buffer
     * @param lighting Lighting data
     */
    void UpdateLightingData(const LightingData& lighting);

    /**
     * @brief Get camera matrices uniform buffer
     */
    UniformBuffer* GetCameraMatricesBuffer() { return &camera_matrices_buffer_; }

    /**
     * @brief Get lighting data uniform buffer
     */
    UniformBuffer* GetLightingDataBuffer() { return &lighting_data_buffer_; }

    /**
     * @brief Enable/disable hot reloading of shaders
     * @param enable True to enable hot reloading
     */
    void SetHotReloadEnabled(bool enable) { hot_reload_enabled_ = enable; }

    /**
     * @brief Check if hot reloading is enabled
     */
    bool IsHotReloadEnabled() const { return hot_reload_enabled_; }

    /**
     * @brief Update shader manager (call once per frame)
     * Handles hot reloading if enabled
     */
    void Update();

private:
    /**
     * @brief Load shader source from file
     * @param filepath Path to shader file
     * @return Shader source code (empty if failed)
     */
    std::string LoadShaderSource(const std::string& filepath);

    /**
     * @brief Get preset shader paths
     * @param preset Shader preset
     * @param vertex_path Output vertex shader path
     * @param fragment_path Output fragment shader path
     * @param geometry_path Output geometry shader path
     */
    void GetPresetPaths(ShaderPreset preset,
                       std::string& vertex_path,
                       std::string& fragment_path,
                       std::string& geometry_path);

    /**
     * @brief Get preset name string
     * @param preset Shader preset
     * @return Preset name
     */
    std::string GetPresetName(ShaderPreset preset);

private:
    std::string shader_directory_;
    std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> shader_programs_;
    std::unordered_map<ShaderPreset, std::string> preset_mapping_;
    
    // Uniform buffers
    UniformBuffer camera_matrices_buffer_;
    UniformBuffer lighting_data_buffer_;
    
    // Hot reloading
    bool hot_reload_enabled_ = false;
    std::unordered_map<std::string, uint64_t> file_timestamps_;
    
    // Initialization state
    bool initialized_ = false;
};

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE