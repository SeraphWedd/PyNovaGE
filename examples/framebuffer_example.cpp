#include "common/game.hpp"
#include <memory>
#include <renderer/renderer.hpp>
#include <renderer/frame_buffer.hpp>
#include <renderer/screen_quad.hpp>

using namespace PyNovaGE;

class FramebufferExample : public Game {
public:
    FramebufferExample() : Game("Framebuffer Example") {
        // Set virtual resolution
        virtual_width_ = 800;
        virtual_height_ = 600;
        stretch_mode_ = false;
    }

    void SetStretch(bool stretch) { stretch_mode_ = stretch; }
    bool IsStretch() const { return stretch_mode_; }

protected:
    bool OnInit() override {
        if (!Game::OnInit()) {
            return false;
        }

        // Create framebuffer
        framebuffer_ = std::make_unique<Renderer::FrameBuffer>(virtual_width_, virtual_height_);

        // Create screen quad
        screen_quad_ = std::make_unique<Renderer::ScreenQuad>();
        screen_quad_->Initialize();

        return true;
    }

    void OnRender() override {
        // First render to framebuffer
        framebuffer_->Bind();
        Renderer::Renderer::Clear({0.2f, 0.3f, 0.3f, 1.0f}); // Different background color for framebuffer
        Game::OnRender(); // Render game content
        framebuffer_->Unbind();

        // Then render framebuffer to screen
        Renderer::Renderer::Clear({0.1f, 0.1f, 0.1f, 1.0f}); // Different background color for main window

        auto window_size = GetWindow()->GetSize();
        int viewport_x = 0;
        int viewport_y = 0;
        int viewport_width = window_size.x;
        int viewport_height = window_size.y;

        if (!stretch_mode_) {
            // Calculate scaled dimensions maintaining aspect ratio
            float window_width = static_cast<float>(window_size.x);
            float window_height = static_cast<float>(window_size.y);
            float target_aspect = static_cast<float>(virtual_width_) / virtual_height_;
            float current_aspect = window_width / window_height;

            if (current_aspect > target_aspect) {
                viewport_width = static_cast<int>(window_height * target_aspect);
                viewport_x = static_cast<int>((window_width - viewport_width) / 2);
            } else {
                viewport_height = static_cast<int>(window_width / target_aspect);
                viewport_y = static_cast<int>((window_height - viewport_height) / 2);
            }
        }

        Renderer::Renderer::SetViewport(viewport_x, viewport_y, viewport_width, viewport_height);
        screen_quad_->Render(framebuffer_->GetTextureHandle());
    }

    void OnWindowResize(int width, int height) override {
        Game::OnWindowResize(width, height);
    }

private:
    std::unique_ptr<Renderer::FrameBuffer> framebuffer_;
    std::unique_ptr<Renderer::ScreenQuad> screen_quad_;
    int virtual_width_;
    int virtual_height_;
    bool stretch_mode_;
};