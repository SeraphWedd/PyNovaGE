#pragma once

#include <vectors/vector2.hpp>
#include <string>
#include <memory>
#include <unordered_map>

namespace PyNovaGE {
namespace Renderer {

/**
 * @brief Texture filtering modes
 */
enum class TextureFilter {
    Nearest,    // GL_NEAREST
    Linear,     // GL_LINEAR
    NearestMipmapNearest,   // GL_NEAREST_MIPMAP_NEAREST
    LinearMipmapNearest,    // GL_LINEAR_MIPMAP_NEAREST
    NearestMipmapLinear,    // GL_NEAREST_MIPMAP_LINEAR
    LinearMipmapLinear      // GL_LINEAR_MIPMAP_LINEAR
};

/**
 * @brief Texture wrapping modes
 */
enum class TextureWrap {
    Repeat,         // GL_REPEAT
    MirroredRepeat, // GL_MIRRORED_REPEAT
    ClampToEdge,    // GL_CLAMP_TO_EDGE
    ClampToBorder   // GL_CLAMP_TO_BORDER
};

/**
 * @brief Texture format types
 */
enum class TextureFormat {
    RGB,        // GL_RGB
    RGBA,       // GL_RGBA
    R,          // GL_RED
    RG,         // GL_RG
    DepthComponent, // GL_DEPTH_COMPONENT
    DepthStencil    // GL_DEPTH_STENCIL
};

/**
 * @brief Texture data type
 */
enum class TextureDataType {
    UnsignedByte,   // GL_UNSIGNED_BYTE
    Float,          // GL_FLOAT
    UnsignedInt,    // GL_UNSIGNED_INT
};

/**
 * @brief Texture configuration parameters
 */
struct TextureConfig {
    TextureFilter min_filter = TextureFilter::Linear;
    TextureFilter mag_filter = TextureFilter::Linear;
    TextureWrap wrap_s = TextureWrap::Repeat;
    TextureWrap wrap_t = TextureWrap::Repeat;
    bool generate_mipmaps = true;
    bool flip_on_load = true;
};

/**
 * @brief OpenGL texture wrapper class
 * 
 * Provides functionality to load, create, and manage OpenGL textures
 * with support for various formats and configurations.
 */
class Texture {
public:
    /**
     * @brief Default constructor creates invalid texture
     */
    Texture();
    
    /**
     * @brief Constructor with width, height, and format
     * @param width Texture width
     * @param height Texture height
     * @param format Texture format
     * @param config Texture configuration
     */
    Texture(int width, int height, TextureFormat format = TextureFormat::RGBA, const TextureConfig& config = {});
    
    /**
     * @brief Destructor automatically cleans up OpenGL resources
     */
    ~Texture();
    
    // Non-copyable but movable
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
    
    /**
     * @brief Load texture from file
     * @param filepath Path to texture file
     * @param config Texture configuration
     * @return true if successful, false otherwise
     */
    bool LoadFromFile(const std::string& filepath, const TextureConfig& config = {});
    
    /**
     * @brief Create texture from raw data
     * @param width Texture width
     * @param height Texture height
     * @param format Texture format
     * @param data_type Data type
     * @param data Raw pixel data (can be nullptr for empty texture)
     * @param config Texture configuration
     * @return true if successful, false otherwise
     */
    bool CreateFromData(int width, int height, TextureFormat format, TextureDataType data_type, const void* data, const TextureConfig& config = {});
    
    /**
     * @brief Create empty texture
     * @param width Texture width
     * @param height Texture height
     * @param format Texture format
     * @param config Texture configuration
     * @return true if successful, false otherwise
     */
    bool CreateEmpty(int width, int height, TextureFormat format = TextureFormat::RGBA, const TextureConfig& config = {});
    
    /**
     * @brief Update texture data
     * @param x X offset
     * @param y Y offset
     * @param width Width of region to update
     * @param height Height of region to update
     * @param format Data format
     * @param data_type Data type
     * @param data Raw pixel data
     */
    void UpdateData(int x, int y, int width, int height, TextureFormat format, TextureDataType data_type, const void* data);
    
    /**
     * @brief Bind texture to specified texture unit
     * @param unit Texture unit (0-31)
     */
    void Bind(unsigned int unit = 0) const;
    
    /**
     * @brief Unbind texture from specified unit
     * @param unit Texture unit (0-31)
     */
    static void Unbind(unsigned int unit = 0);
    
    /**
     * @brief Check if texture is valid and ready to use
     */
    bool IsValid() const { return texture_id_ != 0; }
    
    /**
     * @brief Get OpenGL texture ID
     */
    unsigned int GetTextureID() const { return texture_id_; }
    
    /**
     * @brief Get texture width
     */
    int GetWidth() const { return width_; }
    
    /**
     * @brief Get texture height
     */
    int GetHeight() const { return height_; }
    
    /**
     * @brief Get texture size as Vector2i
     */
    Vector2i GetSize() const { return {width_, height_}; }
    
    /**
     * @brief Get texture format
     */
    TextureFormat GetFormat() const { return format_; }
    
    /**
     * @brief Get texture configuration
     */
    const TextureConfig& GetConfig() const { return config_; }
    
    /**
     * @brief Get file path (if loaded from file)
     */
    const std::string& GetFilePath() const { return filepath_; }
    
    /**
     * @brief Get texture name (for debugging)
     */
    const std::string& GetName() const { return name_; }
    
    /**
     * @brief Generate mipmaps for this texture
     */
    void GenerateMipmaps();
    
    /**
     * @brief Set texture filtering
     * @param min_filter Minification filter
     * @param mag_filter Magnification filter
     */
    void SetFilter(TextureFilter min_filter, TextureFilter mag_filter);
    
    /**
     * @brief Set texture wrapping
     * @param wrap_s S-coordinate wrapping
     * @param wrap_t T-coordinate wrapping
     */
    void SetWrap(TextureWrap wrap_s, TextureWrap wrap_t);
    
    /**
     * @brief Create texture from embedded data
     * @param name Texture name for debugging
     * @param width Texture width
     * @param height Texture height
     * @param data Raw pixel data
     * @param config Texture configuration
     * @return Texture instance
     */
    static Texture Create(const std::string& name, int width, int height, const unsigned char* data, const TextureConfig& config = {});

private:
    /**
     * @brief Convert TextureFilter to OpenGL constant
     */
    static unsigned int FilterToGL(TextureFilter filter);
    
    /**
     * @brief Convert TextureWrap to OpenGL constant
     */
    static unsigned int WrapToGL(TextureWrap wrap);
    
    /**
     * @brief Convert TextureFormat to OpenGL constant
     */
    static unsigned int FormatToGL(TextureFormat format);
    
    /**
     * @brief Convert TextureDataType to OpenGL constant
     */
    static unsigned int DataTypeToGL(TextureDataType data_type);
    
    /**
     * @brief Cleanup OpenGL resources
     */
    void Cleanup();
    
    /**
     * @brief Apply texture configuration
     */
    void ApplyConfig();
    
    unsigned int texture_id_ = 0;
    int width_ = 0;
    int height_ = 0;
    TextureFormat format_ = TextureFormat::RGBA;
    TextureConfig config_;
    std::string filepath_; // For debugging and reloading
    std::string name_;     // For debugging
};

/**
 * @brief Texture atlas for combining multiple small textures
 */
struct TextureAtlasRegion {
    Vector2i position;
    Vector2i size;
    Vector2f uv_min;
    Vector2f uv_max;
    std::string name;
};

/**
 * @brief Simple texture atlas implementation
 */
class TextureAtlas {
public:
    /**
     * @brief Constructor with atlas size
     * @param width Atlas width
     * @param height Atlas height
     */
    TextureAtlas(int width, int height);
    
    /**
     * @brief Destructor
     */
    ~TextureAtlas();
    
    // Non-copyable but movable
    TextureAtlas(const TextureAtlas&) = delete;
    TextureAtlas& operator=(const TextureAtlas&) = delete;
    TextureAtlas(TextureAtlas&& other) noexcept;
    TextureAtlas& operator=(TextureAtlas&& other) noexcept;
    
    /**
     * @brief Add a texture region to the atlas
     * @param name Region name
     * @param width Region width
     * @param height Region height
     * @param data Raw pixel data
     * @return Pointer to region info or nullptr if failed
     */
    const TextureAtlasRegion* AddRegion(const std::string& name, int width, int height, const unsigned char* data);
    
    /**
     * @brief Get region by name
     * @param name Region name
     * @return Pointer to region info or nullptr if not found
     */
    const TextureAtlasRegion* GetRegion(const std::string& name) const;
    
    /**
     * @brief Get the atlas texture
     */
    const Texture& GetTexture() const { return texture_; }
    
    /**
     * @brief Check if atlas is valid
     */
    bool IsValid() const { return texture_.IsValid(); }
    
    /**
     * @brief Get atlas size
     */
    Vector2i GetSize() const { return {width_, height_}; }
    
    /**
     * @brief Get number of regions
     */
    size_t GetRegionCount() const { return regions_.size(); }

private:
    struct Shelf {
        int y;          // Y position of shelf
        int height;     // Height of shelf (tallest item on shelf)
        int width_used; // How much width is used on this shelf
        
        Shelf(int y_pos, int shelf_height) : y(y_pos), height(shelf_height), width_used(0) {}
    };
    
    struct FreeRectangle {
        int x, y, width, height;
        
        FreeRectangle(int x_, int y_, int w_, int h_) : x(x_), y(y_), width(w_), height(h_) {}
        
        bool CanFit(int w, int h) const {
            return width >= w && height >= h;
        }
        
        // Calculate waste when fitting rectangle of given size
        int GetWaste(int w, int h) const {
            return (width * height) - (w * h);
        }
    };
    
    /**
     * @brief Find best position using Best Fit algorithm
     * @param width Required width
     * @param height Required height
     * @param best_x Output: best x position
     * @param best_y Output: best y position
     * @return true if position found, false if no space
     */
    bool FindBestPosition(int width, int height, int& best_x, int& best_y);
    
    /**
     * @brief Place rectangle and update free rectangles
     * @param x X position
     * @param y Y position
     * @param width Rectangle width
     * @param height Rectangle height
     */
    void PlaceRectangle(int x, int y, int width, int height);
    
    /**
     * @brief Split free rectangles that intersect with placed rectangle
     * @param placed_x X position of placed rectangle
     * @param placed_y Y position of placed rectangle
     * @param placed_width Width of placed rectangle
     * @param placed_height Height of placed rectangle
     */
    void SplitFreeRectangles(int placed_x, int placed_y, int placed_width, int placed_height);
    
    /**
     * @brief Remove redundant free rectangles that are contained within others
     */
    void PruneFreeRectangles();
    
    /**
     * @brief Check if rectangle A is inside rectangle B
     */
    bool IsContainedIn(const FreeRectangle& a, const FreeRectangle& b) const;
    
    Texture texture_;
    int width_;
    int height_;
    std::vector<FreeRectangle> free_rectangles_;
    std::unordered_map<std::string, TextureAtlasRegion> regions_;
};

/**
 * @brief Texture manager for loading and caching textures
 */
class TextureManager {
public:
    /**
     * @brief Get the singleton instance
     */
    static TextureManager& Instance();
    
    /**
     * @brief Load and cache a texture
     * @param name Texture name
     * @param filepath Path to texture file
     * @param config Texture configuration
     * @return Pointer to texture or nullptr if failed
     */
    Texture* LoadTexture(const std::string& name, const std::string& filepath, const TextureConfig& config = {});
    
    /**
     * @brief Create and cache a texture from data
     * @param name Texture name
     * @param width Texture width
     * @param height Texture height
     * @param data Raw pixel data
     * @param config Texture configuration
     * @return Pointer to texture or nullptr if failed
     */
    Texture* CreateTexture(const std::string& name, int width, int height, const unsigned char* data, const TextureConfig& config = {});
    
    /**
     * @brief Get a texture by name
     * @param name Texture name
     * @return Pointer to texture or nullptr if not found
     */
    Texture* GetTexture(const std::string& name);
    
    /**
     * @brief Check if texture exists
     * @param name Texture name
     * @return true if exists, false otherwise
     */
    bool HasTexture(const std::string& name) const;
    
    /**
     * @brief Remove a texture
     * @param name Texture name
     */
    void RemoveTexture(const std::string& name);
    
    /**
     * @brief Clear all textures
     */
    void Clear();
    
    /**
     * @brief Get number of loaded textures
     */
    size_t GetTextureCount() const { return textures_.size(); }

private:
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
};

} // namespace Renderer
} // namespace PyNovaGE