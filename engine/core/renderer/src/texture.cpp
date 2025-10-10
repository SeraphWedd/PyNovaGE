#include "renderer/texture.hpp"
#include <glad/gl.h>
#include <iostream>
#include <cstring>
#include <climits>
#include <algorithm>
#include <vector>

// For image loading - we'll implement a simple approach for now
// In a full implementation, you'd use stb_image or similar
#ifdef STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

namespace PyNovaGE {
namespace Renderer {

// Texture implementation
Texture::Texture() {
    // Default constructor creates invalid texture
}

Texture::Texture(int width, int height, TextureFormat format, const TextureConfig& config) {
    CreateEmpty(width, height, format, config);
}

Texture::~Texture() {
    Cleanup();
}

Texture::Texture(Texture&& other) noexcept
    : texture_id_(other.texture_id_)
    , width_(other.width_)
    , height_(other.height_)
    , format_(other.format_)
    , config_(other.config_)
    , filepath_(std::move(other.filepath_))
    , name_(std::move(other.name_)) {
    other.texture_id_ = 0;
    other.width_ = 0;
    other.height_ = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        Cleanup();
        
        texture_id_ = other.texture_id_;
        width_ = other.width_;
        height_ = other.height_;
        format_ = other.format_;
        config_ = other.config_;
        filepath_ = std::move(other.filepath_);
        name_ = std::move(other.name_);
        
        other.texture_id_ = 0;
        other.width_ = 0;
        other.height_ = 0;
    }
    return *this;
}

// Placeholder methods
bool Texture::LoadFromFile(const std::string& filepath, const TextureConfig& config) {
    (void)filepath;
    (void)config;
    return false;
}

bool Texture::CreateFromData(int width, int height, TextureFormat format, TextureDataType data_type, const void* data, const TextureConfig& config) {
    if (width <= 0 || height <= 0) {
        std::cerr << "Invalid texture dimensions: " << width << "x" << height << std::endl;
        return false;
    }
    
    Cleanup();
    
    // Store properties
    width_ = width;
    height_ = height;
    format_ = format;
    config_ = config;
    
    // Generate OpenGL texture
    glGenTextures(1, &texture_id_);
    if (texture_id_ == 0) {
        std::cerr << "Failed to generate OpenGL texture" << std::endl;
        return false;
    }
    
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    
    // Upload texture data
    unsigned int gl_format = FormatToGL(format);
    unsigned int gl_data_type = DataTypeToGL(data_type);
    
    glTexImage2D(GL_TEXTURE_2D, 0, gl_format, width, height, 0, gl_format, gl_data_type, data);
    
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error creating texture: " << error << std::endl;
        glDeleteTextures(1, &texture_id_);
        texture_id_ = 0;
        return false;
    }
    
    // Apply configuration
    ApplyConfig();
    
    return true;
}

bool Texture::CreateEmpty(int width, int height, TextureFormat format, const TextureConfig& config) {
    return CreateFromData(width, height, format, TextureDataType::UnsignedByte, nullptr, config);
}

void Texture::UpdateData(int x, int y, int width, int height, TextureFormat format, TextureDataType data_type, const void* data) {
    if (texture_id_ == 0 || data == nullptr) {
        return;
    }
    
    if (x < 0 || y < 0 || width <= 0 || height <= 0 || 
        x + width > width_ || y + height > height_) {
        std::cerr << "Invalid texture update region" << std::endl;
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, 
                   FormatToGL(format), DataTypeToGL(data_type), data);
    
    // Regenerate mipmaps if they were enabled
    if (config_.generate_mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Bind(unsigned int unit) const {
    if (texture_id_ == 0) return;
    
    if (unit > 31) {  // OpenGL guarantees at least 32 texture units
        std::cerr << "Invalid texture unit: " << unit << std::endl;
        return;
    }
    
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
}

void Texture::Unbind(unsigned int unit) {
    if (unit > 31) {
        std::cerr << "Invalid texture unit: " << unit << std::endl;
        return;
    }
    
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::GenerateMipmaps() {
    if (texture_id_ == 0) return;
    
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Update config to reflect mipmaps were generated
    config_.generate_mipmaps = true;
}

void Texture::SetFilter(TextureFilter min_filter, TextureFilter mag_filter) {
    config_.min_filter = min_filter;
    config_.mag_filter = mag_filter;
    
    if (texture_id_ == 0) return;
    
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilterToGL(min_filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilterToGL(mag_filter));
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetWrap(TextureWrap wrap_s, TextureWrap wrap_t) {
    config_.wrap_s = wrap_s;
    config_.wrap_t = wrap_t;
    
    if (texture_id_ == 0) return;
    
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapToGL(wrap_s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapToGL(wrap_t));
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture Texture::Create(const std::string& name, int width, int height, const unsigned char* data, const TextureConfig& config) {
    Texture texture;
    texture.name_ = name;
    
    if (texture.CreateFromData(width, height, TextureFormat::RGBA, TextureDataType::UnsignedByte, data, config)) {
        return texture;
    }
    
    // Return invalid texture on failure
    return Texture();
}

unsigned int Texture::FilterToGL(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::Nearest: return GL_NEAREST;
        case TextureFilter::Linear: return GL_LINEAR;
        case TextureFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case TextureFilter::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
        case TextureFilter::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
        case TextureFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
        default: return GL_LINEAR;
    }
}

unsigned int Texture::WrapToGL(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Repeat: return GL_REPEAT;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
        default: return GL_REPEAT;
    }
}

unsigned int Texture::FormatToGL(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGB: return GL_RGB;
        case TextureFormat::RGBA: return GL_RGBA;
        case TextureFormat::R: return GL_RED;
        case TextureFormat::RG: return GL_RG;
        case TextureFormat::DepthComponent: return GL_DEPTH_COMPONENT;
        case TextureFormat::DepthStencil: return GL_DEPTH_STENCIL;
        default: return GL_RGBA;
    }
}

unsigned int Texture::DataTypeToGL(TextureDataType data_type) {
    switch (data_type) {
        case TextureDataType::UnsignedByte: return GL_UNSIGNED_BYTE;
        case TextureDataType::Float: return GL_FLOAT;
        case TextureDataType::UnsignedInt: return GL_UNSIGNED_INT;
        default: return GL_UNSIGNED_BYTE;
    }
}

void Texture::Cleanup() {
    if (texture_id_ != 0) {
        glDeleteTextures(1, &texture_id_);
        texture_id_ = 0;
    }
    width_ = 0;
    height_ = 0;
}

void Texture::ApplyConfig() {
    if (texture_id_ == 0) return;
    
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    
    // Set filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilterToGL(config_.min_filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilterToGL(config_.mag_filter));
    
    // Set wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapToGL(config_.wrap_s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapToGL(config_.wrap_t));
    
    // Generate mipmaps if requested
    if (config_.generate_mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

// TextureAtlas implementation
TextureAtlas::TextureAtlas(int width, int height) : width_(width), height_(height) {
    // Initialize with one large free rectangle covering entire atlas
    free_rectangles_.emplace_back(0, 0, width, height);
    
    // Only create OpenGL texture if we have a valid context
    // For unit tests, we can skip this part
    try {
        unsigned char* empty_data = new unsigned char[width * height * 4];
        std::memset(empty_data, 0, width * height * 4);
        
        texture_ = Texture::Create("atlas_" + std::to_string(width) + "x" + std::to_string(height), width, height, empty_data);
        delete[] empty_data;
    } catch (...) {
        // If texture creation fails (no OpenGL context), continue without it
        // The atlas can still function for testing the packing algorithm
    }
}

TextureAtlas::~TextureAtlas() {
    std::cout << "TextureAtlas destroyed" << std::endl;
}

TextureAtlas::TextureAtlas(TextureAtlas&& other) noexcept : texture_(std::move(other.texture_)), width_(other.width_), height_(other.height_), free_rectangles_(std::move(other.free_rectangles_)), regions_(std::move(other.regions_)) {
    std::cout << "TextureAtlas moved" << std::endl;
}

TextureAtlas& TextureAtlas::operator=(TextureAtlas&& other) noexcept {
    if (this != &other) {
        texture_ = std::move(other.texture_);
        width_ = other.width_;
        height_ = other.height_;
        free_rectangles_ = std::move(other.free_rectangles_);
        regions_ = std::move(other.regions_);
    }
    return *this;
}

const TextureAtlasRegion* TextureAtlas::AddRegion(const std::string& name, int width, int height, const unsigned char* data) {
    if (!data || width <= 0 || height <= 0) {
        return nullptr;
    }
    
    // Check if region already exists
    if (regions_.find(name) != regions_.end()) {
        std::cerr << "Region '" << name << "' already exists in atlas" << std::endl;
        return nullptr;
    }
    
    // Find best position using maximal rectangles algorithm
    int best_x, best_y;
    if (!FindBestPosition(width, height, best_x, best_y)) {
        std::cerr << "No space available in atlas for region '" << name << "' (" << width << "x" << height << ")" << std::endl;
        return nullptr;
    }
    
    // Place the rectangle and update free space
    PlaceRectangle(best_x, best_y, width, height);
    
    // Update atlas texture with region data (only if texture is valid)
    if (texture_.IsValid()) {
        texture_.UpdateData(best_x, best_y, width, height, TextureFormat::RGBA, TextureDataType::UnsignedByte, data);
    }
    
    // Create region info
    TextureAtlasRegion region;
    region.position = {best_x, best_y};
    region.size = {width, height};
    region.name = name;
    
    // Calculate UV coordinates
    float inv_width = 1.0f / static_cast<float>(width_);
    float inv_height = 1.0f / static_cast<float>(height_);
    region.uv_min = {static_cast<float>(best_x) * inv_width, static_cast<float>(best_y) * inv_height};
    region.uv_max = {static_cast<float>(best_x + width) * inv_width, static_cast<float>(best_y + height) * inv_height};
    
    // Store region
    auto [iter, success] = regions_.emplace(name, region);
    return success ? &iter->second : nullptr;
}

const TextureAtlasRegion* TextureAtlas::GetRegion(const std::string& name) const {
    auto iter = regions_.find(name);
    return (iter != regions_.end()) ? &iter->second : nullptr;
}

bool TextureAtlas::FindBestPosition(int width, int height, int& best_x, int& best_y) {
    int best_short_side_fit = INT_MAX;
    int best_long_side_fit = INT_MAX;
    best_x = -1;
    best_y = -1;
    
    for (const auto& rect : free_rectangles_) {
        if (!rect.CanFit(width, height)) {
            continue;
        }
        
        // Calculate fit quality
        int leftover_horizontal = rect.width - width;
        int leftover_vertical = rect.height - height;
        int short_side_fit = std::min(leftover_horizontal, leftover_vertical);
        int long_side_fit = std::max(leftover_horizontal, leftover_vertical);
        
        // Best fit is the one with least short side waste, then least long side waste
        if (short_side_fit < best_short_side_fit || 
            (short_side_fit == best_short_side_fit && long_side_fit < best_long_side_fit)) {
            best_x = rect.x;
            best_y = rect.y;
            best_short_side_fit = short_side_fit;
            best_long_side_fit = long_side_fit;
        }
    }
    
    return best_x != -1;
}

void TextureAtlas::PlaceRectangle(int x, int y, int width, int height) {
    // Split free rectangles that intersect with the placed rectangle
    SplitFreeRectangles(x, y, width, height);
    
    // Remove redundant free rectangles
    PruneFreeRectangles();
}

void TextureAtlas::SplitFreeRectangles(int placed_x, int placed_y, int placed_width, int placed_height) {
    std::vector<FreeRectangle> new_rectangles;
    
    for (const auto& rect : free_rectangles_) {
        // Skip rectangles that don't intersect with the placed rectangle
        if (rect.x >= placed_x + placed_width || rect.x + rect.width <= placed_x ||
            rect.y >= placed_y + placed_height || rect.y + rect.height <= placed_y) {
            new_rectangles.push_back(rect);
            continue;
        }
        
        // Split the intersecting rectangle
        // Left side
        if (rect.x < placed_x) {
            new_rectangles.emplace_back(rect.x, rect.y, placed_x - rect.x, rect.height);
        }
        
        // Right side
        if (rect.x + rect.width > placed_x + placed_width) {
            new_rectangles.emplace_back(placed_x + placed_width, rect.y, 
                                       (rect.x + rect.width) - (placed_x + placed_width), rect.height);
        }
        
        // Bottom side
        if (rect.y < placed_y) {
            new_rectangles.emplace_back(rect.x, rect.y, rect.width, placed_y - rect.y);
        }
        
        // Top side
        if (rect.y + rect.height > placed_y + placed_height) {
            new_rectangles.emplace_back(rect.x, placed_y + placed_height, rect.width,
                                       (rect.y + rect.height) - (placed_y + placed_height));
        }
    }
    
    free_rectangles_ = std::move(new_rectangles);
}

void TextureAtlas::PruneFreeRectangles() {
    // Remove rectangles that are contained within other rectangles
    for (size_t i = 0; i < free_rectangles_.size(); ++i) {
        for (size_t j = i + 1; j < free_rectangles_.size();) {
            if (IsContainedIn(free_rectangles_[i], free_rectangles_[j])) {
                // Rectangle i is contained in j, remove i
                free_rectangles_.erase(free_rectangles_.begin() + i);
                --i;
                break;
            } else if (IsContainedIn(free_rectangles_[j], free_rectangles_[i])) {
                // Rectangle j is contained in i, remove j
                free_rectangles_.erase(free_rectangles_.begin() + j);
            } else {
                ++j;
            }
        }
    }
}

bool TextureAtlas::IsContainedIn(const FreeRectangle& a, const FreeRectangle& b) const {
    return a.x >= b.x && a.y >= b.y && 
           a.x + a.width <= b.x + b.width && 
           a.y + a.height <= b.y + b.height;
}

// TextureManager implementation
TextureManager& TextureManager::Instance() {
    static TextureManager instance;
    return instance;
}

Texture* TextureManager::LoadTexture(const std::string& name, const std::string& filepath, const TextureConfig& config) {
    (void)name;
    (void)filepath;
    (void)config;
    return nullptr;
}

Texture* TextureManager::CreateTexture(const std::string& name, int width, int height, const unsigned char* data, const TextureConfig& config) {
    (void)name;
    (void)width;
    (void)height;
    (void)data;
    (void)config;
    return nullptr;
}

Texture* TextureManager::GetTexture(const std::string& name) {
    (void)name;
    return nullptr;
}

bool TextureManager::HasTexture(const std::string& name) const {
    (void)name;
    return false;
}

void TextureManager::RemoveTexture(const std::string& name) {
    (void)name;
}

void TextureManager::Clear() {
    textures_.clear();
}

} // namespace Renderer
} // namespace PyNovaGE