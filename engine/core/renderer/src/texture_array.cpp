#include "renderer/texture_array.hpp"
#include <glad/gl.h>
#include <iostream>

namespace PyNovaGE {
namespace Renderer {

TextureArray::TextureArray() = default;

TextureArray::~TextureArray() {
    if (texture_id_ != 0) {
        glDeleteTextures(1, &texture_id_);
        texture_id_ = 0;
    }
}

TextureArray::TextureArray(TextureArray&& other) noexcept {
    texture_id_ = other.texture_id_;
    width_ = other.width_;
    height_ = other.height_;
    layers_ = other.layers_;
    format_ = other.format_;
    generate_mipmaps_ = other.generate_mipmaps_;
    other.texture_id_ = 0;
    other.width_ = 0;
    other.height_ = 0;
    other.layers_ = 0;
}

TextureArray& TextureArray::operator=(TextureArray&& other) noexcept {
    if (this != &other) {
        if (texture_id_ != 0) glDeleteTextures(1, &texture_id_);
        texture_id_ = other.texture_id_;
        width_ = other.width_;
        height_ = other.height_;
        layers_ = other.layers_;
        format_ = other.format_;
        generate_mipmaps_ = other.generate_mipmaps_;
        other.texture_id_ = 0;
        other.width_ = 0;
        other.height_ = 0;
        other.layers_ = 0;
    }
    return *this;
}

bool TextureArray::Create(int width, int height, int layers, TextureFormat format, bool generate_mipmaps) {
    if (width <= 0 || height <= 0 || layers <= 0) {
        std::cerr << "TextureArray::Create invalid dimensions/layers" << std::endl;
        return false;
    }

    if (texture_id_ != 0) {
        glDeleteTextures(1, &texture_id_);
        texture_id_ = 0;
    }

    width_ = width;
    height_ = height;
    layers_ = layers;
    format_ = format;
    generate_mipmaps_ = generate_mipmaps;

    glGenTextures(1, &texture_id_);
    if (texture_id_ == 0) {
        std::cerr << "Failed to generate GL texture array" << std::endl;
        return false;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);

    // Allocate storage (no data yet)
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, FormatToGL(format_), width_, height_, layers_, 0, FormatToGL(format_), GL_UNSIGNED_BYTE, nullptr);

    // Default parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, FilterToGL(TextureFilter::NearestMipmapLinear));
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, FilterToGL(TextureFilter::Linear));
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, WrapToGL(TextureWrap::Repeat));
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, WrapToGL(TextureWrap::Repeat));

    if (generate_mipmaps_) {
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    return true;
}

bool TextureArray::SetLayerData(int layer, TextureFormat src_format, TextureDataType data_type, const void* data) {
    if (texture_id_ == 0 || data == nullptr || layer < 0 || layer >= layers_) return false;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, width_, height_, 1, FormatToGL(src_format), DataTypeToGL(data_type), data);

    if (generate_mipmaps_) {
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    return true;
}

void TextureArray::Bind(unsigned int unit) const {
    if (texture_id_ == 0) return;
    if (unit > 31) return;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);
}

void TextureArray::Unbind(unsigned int unit) {
    if (unit > 31) return;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TextureArray::SetFilter(TextureFilter min_filter, TextureFilter mag_filter) {
    if (texture_id_ == 0) return;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, FilterToGL(min_filter));
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, FilterToGL(mag_filter));
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TextureArray::SetWrap(TextureWrap wrap_s, TextureWrap wrap_t) {
    if (texture_id_ == 0) return;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, WrapToGL(wrap_s));
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, WrapToGL(wrap_t));
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TextureArray::GenerateMipmaps() {
    if (texture_id_ == 0) return;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

bool TextureArray::LoadTextureFromFile(int layer, const std::string& filepath, bool flip_vertically) {
    if (texture_id_ == 0 || layer < 0 || layer >= layers_) {
        std::cerr << "TextureArray::LoadTextureFromFile - Invalid state or layer index" << std::endl;
        return false;
    }

    int img_width, img_height, channels;
    stbi_set_flip_vertically_on_load(flip_vertically);
    unsigned char* data = stbi_load(filepath.c_str(), &img_width, &img_height, &channels, STBI_rgb_alpha);

    if (!data) {
        std::cerr << "Failed to load texture from " << filepath << ": " << stbi_failure_reason() << std::endl;
        return false;
    }

    if (img_width != width_ || img_height != height_) {
        std::cerr << "Texture size mismatch: Expected " << width_ << "x" << height_ 
                  << ", got " << img_width << "x" << img_height << std::endl;
        stbi_image_free(data);
        return false;
    }

    // Upload image data to the specified layer
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id_);
    glTexSubImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,              // mipmap level
        0, 0, layer,    // x, y, layer offset
        width_, height_, 1,  // width, height, layer count
        GL_RGBA,        // format
        GL_UNSIGNED_BYTE,  // type
        data            // pixel data
    );

    if (generate_mipmaps_) {
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    stbi_image_free(data);
    return true;
}

bool TextureArray::LoadTexturesFromFiles(const std::vector<std::string>& filepaths, bool flip_vertically) {
    if (filepaths.empty()) return true;
    
    if (static_cast<int>(filepaths.size()) > layers_) {
        std::cerr << "Too many textures to load: Have " << layers_ << " layers, need " 
                  << filepaths.size() << std::endl;
        return false;
    }

    bool success = true;
    for (size_t i = 0; i < filepaths.size() && success; ++i) {
        success = LoadTextureFromFile(static_cast<int>(i), filepaths[i], flip_vertically);
    }
    return success;
}

unsigned int TextureArray::FilterToGL(TextureFilter filter) {
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

unsigned int TextureArray::WrapToGL(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Repeat: return GL_REPEAT;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
        default: return GL_REPEAT;
    }
}

unsigned int TextureArray::FormatToGL(TextureFormat format) {
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

unsigned int TextureArray::DataTypeToGL(TextureDataType data_type) {
    switch (data_type) {
        case TextureDataType::UnsignedByte: return GL_UNSIGNED_BYTE;
        case TextureDataType::Float: return GL_FLOAT;
        case TextureDataType::UnsignedInt: return GL_UNSIGNED_INT;
        default: return GL_UNSIGNED_BYTE;
    }
}

} // namespace Renderer
} // namespace PyNovaGE