#pragma once

namespace PyNovaGE {
namespace Renderer {

/**
 * @brief 2D sprite renderer
 * 
 * Handles individual sprite rendering operations.
 * For now, this is a placeholder implementation.
 */
class SpriteRenderer {
public:
    /**
     * @brief Constructor
     */
    SpriteRenderer();
    
    /**
     * @brief Destructor
     */
    ~SpriteRenderer();
    
    // Non-copyable but movable
    SpriteRenderer(const SpriteRenderer&) = delete;
    SpriteRenderer& operator=(const SpriteRenderer&) = delete;
    SpriteRenderer(SpriteRenderer&&) = default;
    SpriteRenderer& operator=(SpriteRenderer&&) = default;
};

} // namespace Renderer
} // namespace PyNovaGE