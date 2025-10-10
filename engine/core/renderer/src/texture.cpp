#include "renderer/texture.hpp"
#include <glad/gl.h>
#include <iostream>
#include <cstring>

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

// TextureAtlas implementation (placeholder)
TextureAtlas::TextureAtlas(int width, int height) : width_(width), height_(height) {
    std::cout << "TextureAtlas created " << width << "x" << height << " (placeholder)" << std::endl;
}

TextureAtlas::~TextureAtlas() {
    std::cout << "TextureAtlas destroyed" << std::endl;
}

TextureAtlas::TextureAtlas(TextureAtlas&& other) noexcept : texture_(std::move(other.texture_)), width_(other.width_), height_(other.height_), root_(std::move(other.root_)), regions_(std::move(other.regions_)) {
    std::cout << "TextureAtlas moved" << std::endl;
}

TextureAtlas& TextureAtlas::operator=(TextureAtlas&& other) noexcept {
    if (this != &other) {
        texture_ = std::move(other.texture_);
        width_ = other.width_;
        height_ = other.height_;
        root_ = std::move(other.root_);
        regions_ = std::move(other.regions_);
    }
    return *this;
}

const TextureAtlasRegion* TextureAtlas::AddRegion(const std::string& name, int width, int height, const unsigned char* data) {
    (void)name;
    (void)width;
    (void)height;
    (void)data;
    return nullptr;
}

const TextureAtlasRegion* TextureAtlas::GetRegion(const std::string& name) const {
    (void)name;
    return nullptr;
}

TextureAtlas::AtlasNode* TextureAtlas::FindNode(AtlasNode* node, int width, int height) {
    (void)node;
    (void)width;
    (void)height;
    return nullptr;
}

TextureAtlas::AtlasNode* TextureAtlas::SplitNode(AtlasNode* node, int width, int height) {
    (void)node;
    (void)width;
    (void)height;
    return nullptr;
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