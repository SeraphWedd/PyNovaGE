#pragma once

namespace PyNovaGE {
namespace Renderer {

/**
 * @brief Batch renderer for efficient sprite rendering
 * 
 * Batches multiple sprites into fewer draw calls for better performance.
 * For now, this is a placeholder implementation.
 */
class BatchRenderer {
public:
    /**
     * @brief Constructor
     * @param max_sprites Maximum sprites per batch
     * @param max_textures Maximum textures per batch
     */
    BatchRenderer(int max_sprites, int max_textures);
    
    /**
     * @brief Destructor
     */
    ~BatchRenderer();
    
    // Non-copyable but movable
    BatchRenderer(const BatchRenderer&) = delete;
    BatchRenderer& operator=(const BatchRenderer&) = delete;
    BatchRenderer(BatchRenderer&&) = default;
    BatchRenderer& operator=(BatchRenderer&&) = default;
};

} // namespace Renderer
} // namespace PyNovaGE