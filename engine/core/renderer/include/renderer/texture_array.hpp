#pragma once

#include <string>
#include <vector>
#include <vectors/vector2.hpp>
#include "renderer/texture.hpp"
#include "stb_image.h"

namespace PyNovaGE {
namespace Renderer {

class TextureArray {
public:
    TextureArray();
    ~TextureArray();

    // Non-copyable but movable
    TextureArray(const TextureArray&) = delete;
    TextureArray& operator=(const TextureArray&) = delete;
    TextureArray(TextureArray&& other) noexcept;
    TextureArray& operator=(TextureArray&& other) noexcept;

    bool Create(int width, int height, int layers, TextureFormat format = TextureFormat::RGBA, bool generate_mipmaps = true);

    // Set entire layer data (expects width*height pixels)
    bool SetLayerData(int layer, TextureFormat src_format, TextureDataType data_type, const void* data);

    void Bind(unsigned int unit = 0) const;
    static void Unbind(unsigned int unit = 0);

    void SetFilter(TextureFilter min_filter, TextureFilter mag_filter);
    void SetWrap(TextureWrap wrap_s, TextureWrap wrap_t);
    void GenerateMipmaps();

    unsigned int GetTextureID() const { return texture_id_; }
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    int GetLayerCount() const { return layers_; }

    /**
     * @brief Load a PNG image file into a specific layer
     * @param layer Layer index to load into
     * @param filepath Path to PNG file
     * @param flip_vertically Whether to flip the image vertically on load
     * @return True if loading succeeded
     */
    bool LoadTextureFromFile(int layer, const std::string& filepath, bool flip_vertically = true);

    /**
     * @brief Load multiple PNG files into consecutive layers
     * @param filepaths List of PNG file paths to load
     * @param flip_vertically Whether to flip images vertically on load
     * @return True if all images loaded successfully
     */
    bool LoadTexturesFromFiles(const std::vector<std::string>& filepaths, bool flip_vertically = true);

private:
    static unsigned int FilterToGL(TextureFilter filter);
    static unsigned int WrapToGL(TextureWrap wrap);
    static unsigned int FormatToGL(TextureFormat format);
    static unsigned int DataTypeToGL(TextureDataType data_type);

    unsigned int texture_id_ = 0;
    int width_ = 0;
    int height_ = 0;
    int layers_ = 0;
    TextureFormat format_ = TextureFormat::RGBA;
    bool generate_mipmaps_ = true;
};

} // namespace Renderer
} // namespace PyNovaGE