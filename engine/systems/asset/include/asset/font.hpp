#pragma once

#include "asset_manager.hpp"
#include <vectors/vector2.hpp>
#include <memory>
#include <unordered_map>

struct stbtt_fontinfo;

namespace PyNovaGE {
namespace Asset {

/**
 * @brief Glyph information for text rendering
 */
struct Glyph {
    int width = 0;
    int height = 0;
    int bearing_x = 0;  // Left side bearing
    int bearing_y = 0;  // Top bearing
    int advance = 0;    // How far to advance cursor
    
    // Texture coordinates in font atlas (if using atlas)
    float tex_x0 = 0.0f, tex_y0 = 0.0f;
    float tex_x1 = 1.0f, tex_y1 = 1.0f;
    
    // Bitmap data (if not using atlas)
    std::unique_ptr<unsigned char[]> bitmap;
};

/**
 * @brief Font asset for text rendering
 */
class Font : public Asset {
public:
    Font(const std::string& path, float size);
    ~Font();

    /**
     * @brief Load font from file
     * @param path Path to font file (.ttf, .otf)
     * @return true if successful
     */
    bool LoadFromFile(const std::string& path);

    /**
     * @brief Get glyph for character
     * @param codepoint Unicode codepoint
     * @return Glyph data, or null if not found
     */
    const Glyph* GetGlyph(int codepoint);

    /**
     * @brief Get kerning between two characters
     * @param char1 First character codepoint
     * @param char2 Second character codepoint
     * @return Kerning offset in pixels
     */
    int GetKerning(int char1, int char2) const;

    /**
     * @brief Calculate text dimensions
     * @param text Text string to measure
     * @return Width and height in pixels
     */
    Vector2i MeasureText(const std::string& text) const;

    /**
     * @brief Get font size
     */
    float GetSize() const { return size_; }

    /**
     * @brief Get font ascent (baseline to top)
     */
    int GetAscent() const { return ascent_; }

    /**
     * @brief Get font descent (baseline to bottom) 
     */
    int GetDescent() const { return descent_; }

    /**
     * @brief Get line gap (spacing between lines)
     */
    int GetLineGap() const { return line_gap_; }

    /**
     * @brief Check if font is loaded
     */
    bool IsLoaded() const { return loaded_; }

private:
    void GenerateGlyph(int codepoint);
    void CalculateFontMetrics();

    bool loaded_ = false;
    float size_;
    
    // Font metrics
    int ascent_ = 0;
    int descent_ = 0; 
    int line_gap_ = 0;
    
    // stb_truetype data
    std::unique_ptr<stbtt_fontinfo> font_info_;
    std::unique_ptr<unsigned char[]> font_data_;
    float scale_ = 1.0f;
    
    // Glyph cache
    std::unordered_map<int, std::unique_ptr<Glyph>> glyphs_;
};

} // namespace Asset
} // namespace PyNovaGE