#include "renderer/batch_renderer.hpp"
#include <iostream>

namespace PyNovaGE {
namespace Renderer {

BatchRenderer::BatchRenderer(int max_sprites, int max_textures) {
    std::cout << "BatchRenderer created (placeholder) - Max sprites: " << max_sprites 
              << ", Max textures: " << max_textures << std::endl;
}

BatchRenderer::~BatchRenderer() {
    std::cout << "BatchRenderer destroyed" << std::endl;
}

} // namespace Renderer
} // namespace PyNovaGE