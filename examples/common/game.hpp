#pragma once

#include <window/window.hpp>
#include <memory>
#include <string>
#include <renderer/renderer.hpp>

namespace PyNovaGE {

class Game {
public:
    Game(const std::string& title = "Game")
        : title_(title)
        , window_guard_()
        , window_(nullptr) {}

    virtual ~Game() = default;

    int Run() {
        // Create window
        Window::WindowConfig config;
        config.title = title_;
        window_ = new Window::Window(config);
        if (window_ == nullptr) {
            return 1;
        }

        // Initialize renderer
        if (!Renderer::Renderer::Initialize()) {
            return 1;
        }

        // Set callbacks
        window_->SetEventCallback([this](const Window::WindowEvent& event) {
            if (event.type == Window::WindowEventType::Resize) {
                this->OnWindowResize(event.width, event.height);
            }
        });

        // Initialize game
        if (!OnInit()) {
            return 1;
        }

        // Game loop
        while (!window_->ShouldClose()) {
            window_->PollEvents();
            OnUpdate();
            OnRender();
            window_->SwapBuffers();
        }

        // Cleanup
        OnCleanup();
        delete window_;
        Renderer::Renderer::Shutdown();

        return 0;
    }

protected:
    virtual bool OnInit() { return true; }
    virtual void OnUpdate() {}
    virtual void OnRender() {}
    virtual void OnCleanup() {}
    virtual void OnWindowResize(int width, int height) { (void)width; (void)height; }

    Window::Window* GetWindow() { return window_; }
    const Window::Window* GetWindow() const { return window_; }

private:
    std::string title_;
    Window::WindowSystemGuard window_guard_;
    Window::Window* window_;
};

} // namespace PyNovaGE