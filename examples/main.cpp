#include "framebuffer_example.cpp"
#include <memory>

int main() {
    auto game = std::make_unique<FramebufferExample>();
    return game->Run();
}