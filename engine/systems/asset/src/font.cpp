#include "asset/font.hpp"
#include "../../../third_party/stb/stb_truetype.h"

#include <fstream>
#include <iostream>
#include <algorithm>

namespace PyNovaGE {
namespace Asset {

Font::Font(const std::string& path, float size) 
    : Asset(path, AssetType::Font), size_(size) {
    font_info_ = std::make_unique<stbtt_fontinfo>();
}

Font::~Font() {
    // Cleanup handled by unique_ptr destructors
}

bool Font::LoadFromFile(const std::string& path) {
    if (loaded_) {
        return true;
    }

    // Read font file
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open font file: " << path << std::endl;
        return false;
    }

    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    font_data_ = std::make_unique<unsigned char[]>(file_size);
    file.read(reinterpret_cast<char*>(font_data_.get()), file_size);
    file.close();

    // Initialize font info
    if (!stbtt_InitFont(font_info_.get(), font_data_.get(), 0)) {
        std::cerr << "Failed to initialize font: " << path << std::endl;
        return false;
    }

    // Calculate font metrics
    CalculateFontMetrics();
    
    loaded_ = true;
    std::cout << "Font loaded: " << path << " (size: " << size_ << ")" << std::endl;
    return true;
}

const Glyph* Font::GetGlyph(int codepoint) {
    if (!loaded_) {
        return nullptr;
    }

    // Check if glyph is cached
    auto it = glyphs_.find(codepoint);
    if (it != glyphs_.end()) {
        return it->second.get();
    }

    // Generate glyph
    GenerateGlyph(codepoint);
    
    // Return the newly generated glyph
    auto new_it = glyphs_.find(codepoint);
    if (new_it != glyphs_.end()) {
        return new_it->second.get();
    }

    return nullptr;
}

int Font::GetKerning(int char1, int char2) const {
    if (!loaded_) {
        return 0;
    }

    return static_cast<int>(stbtt_GetCodepointKernAdvance(font_info_.get(), char1, char2) * scale_);
}

Vector2i Font::MeasureText(const std::string& text) const {
    if (!loaded_ || text.empty()) {
        return Vector2i(0, 0);
    }

    int width = 0;
    int height = static_cast<int>(size_);
    
    const char* str = text.c_str();
    while (*str) {
        int codepoint = *str; // Simple ASCII for now
        
        int advance, lsb;
        stbtt_GetCodepointHMetrics(font_info_.get(), codepoint, &advance, &lsb);
        width += static_cast<int>(advance * scale_);
        
        // Add kerning if there's a next character
        if (*(str + 1)) {
            width += GetKerning(codepoint, *(str + 1));
        }
        
        str++;
    }

    return Vector2i(width, height);
}

void Font::GenerateGlyph(int codepoint) {
    if (!loaded_) {
        return;
    }

    auto glyph = std::make_unique<Glyph>();

    // Get glyph metrics
    int advance, lsb;
    stbtt_GetCodepointHMetrics(font_info_.get(), codepoint, &advance, &lsb);
    
    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBox(font_info_.get(), codepoint, scale_, scale_, &x0, &y0, &x1, &y1);
    
    glyph->width = x1 - x0;
    glyph->height = y1 - y0;
    glyph->bearing_x = x0;
    glyph->bearing_y = -y0; // stb_truetype uses different coordinate system
    glyph->advance = static_cast<int>(advance * scale_);

    // Generate bitmap if glyph has dimensions
    if (glyph->width > 0 && glyph->height > 0) {
        glyph->bitmap = std::make_unique<unsigned char[]>(glyph->width * glyph->height);
        
        stbtt_MakeCodepointBitmap(
            font_info_.get(),
            glyph->bitmap.get(),
            glyph->width,
            glyph->height,
            glyph->width, // stride
            scale_,
            scale_,
            codepoint
        );
    }

    // Cache the glyph
    glyphs_[codepoint] = std::move(glyph);
}

void Font::CalculateFontMetrics() {
    if (!loaded_) {
        return;
    }

    // Get unscaled metrics
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(font_info_.get(), &ascent, &descent, &line_gap);
    
    // Calculate scale for desired font size
    scale_ = stbtt_ScaleForPixelHeight(font_info_.get(), size_);
    
    // Scale metrics
    ascent_ = static_cast<int>(ascent * scale_);
    descent_ = static_cast<int>(descent * scale_);
    line_gap_ = static_cast<int>(line_gap * scale_);
}

} // namespace Asset
} // namespace PyNovaGE