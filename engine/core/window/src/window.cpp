#include "window/window.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

namespace PyNovaGE {
namespace Window {

// Static member for error handling
static bool g_glfw_initialized = false;
static int g_window_count = 0;

static void GLFWErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

// WindowSystemGuard implementation
WindowSystemGuard::WindowSystemGuard() {
    initialized_ = InitializeWindowSystem();
}

WindowSystemGuard::~WindowSystemGuard() {
    if (initialized_) {
        ShutdownWindowSystem();
    }
}

bool InitializeWindowSystem() {
    if (g_glfw_initialized) {
        return true;
    }
    
    glfwSetErrorCallback(GLFWErrorCallback);
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    g_glfw_initialized = true;
    return true;
}

void ShutdownWindowSystem() {
    if (g_glfw_initialized && g_window_count == 0) {
        glfwTerminate();
        g_glfw_initialized = false;
    }
}

// Window implementation
Window::Window(const WindowConfig& config) : config_(config), window_(nullptr) {
    if (!g_glfw_initialized) {
        throw std::runtime_error("Window system not initialized. Call InitializeWindowSystem() first.");
    }
    
    // Set window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, config.visible ? GLFW_TRUE : GLFW_FALSE);
    
    if (config.samples > 0) {
        glfwWindowHint(GLFW_SAMPLES, config.samples);
    }
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    
    // Create window
    GLFWmonitor* monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    window_ = glfwCreateWindow(config.width, config.height, config.title.c_str(), monitor, nullptr);
    
    if (!window_) {
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    g_window_count++;
    
    // Store this instance in the window user pointer
    glfwSetWindowUserPointer(window_, this);
    
    // Make context current and setup callbacks
    glfwMakeContextCurrent(window_);

    // Initialize glad
    if (!gladLoadGL(glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize OpenGL context");
    }

    SetVSync(config.vsync);
    SetupCallbacks();
}

Window::~Window() {
    if (window_) {
        glfwDestroyWindow(window_);
        g_window_count--;
    }
}

Window::Window(Window&& other) noexcept
    : window_(other.window_)
    , config_(std::move(other.config_))
    , event_callback_(std::move(other.event_callback_)) {
    other.window_ = nullptr;
    
    if (window_) {
        glfwSetWindowUserPointer(window_, this);
    }
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        if (window_) {
            glfwDestroyWindow(window_);
            g_window_count--;
        }
        
        window_ = other.window_;
        config_ = std::move(other.config_);
        event_callback_ = std::move(other.event_callback_);
        
        other.window_ = nullptr;
        
        if (window_) {
            glfwSetWindowUserPointer(window_, this);
        }
    }
    return *this;
}

bool Window::ShouldClose() const {
    return window_ ? glfwWindowShouldClose(window_) : true;
}

void Window::SetShouldClose(bool should_close) {
    if (window_) {
        glfwSetWindowShouldClose(window_, should_close ? GLFW_TRUE : GLFW_FALSE);
    }
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::SwapBuffers() {
    if (window_) {
        glfwSwapBuffers(window_);
    }
}

PyNovaGE::Vector2i Window::GetSize() const {
    if (!window_) return {0, 0};
    
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    return {width, height};
}

void Window::SetSize(int width, int height) {
    if (window_) {
        glfwSetWindowSize(window_, width, height);
        config_.width = width;
        config_.height = height;
    }
}

PyNovaGE::Vector2i Window::GetFramebufferSize() const {
    if (!window_) return {0, 0};
    
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    return {width, height};
}

PyNovaGE::Vector2i Window::GetPosition() const {
    if (!window_) return {0, 0};
    
    int x, y;
    glfwGetWindowPos(window_, &x, &y);
    return {x, y};
}

void Window::SetPosition(int x, int y) {
    if (window_) {
        glfwSetWindowPos(window_, x, y);
    }
}

const std::string& Window::GetTitle() const {
    return config_.title;
}

void Window::SetTitle(const std::string& title) {
    if (window_) {
        glfwSetWindowTitle(window_, title.c_str());
        config_.title = title;
    }
}

bool Window::IsFullscreen() const {
    return window_ ? (glfwGetWindowMonitor(window_) != nullptr) : false;
}

void Window::SetFullscreen(bool fullscreen) {
    if (!window_) return;
    
    if (fullscreen == IsFullscreen()) return;
    
    if (fullscreen) {
        // Store windowed mode size and position
        glfwGetWindowPos(window_, &config_.width, &config_.height);
        glfwGetWindowSize(window_, &config_.width, &config_.height);
        
        // Switch to fullscreen
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(window_, glfwGetPrimaryMonitor(), 0, 0, 
                           mode->width, mode->height, mode->refreshRate);
    } else {
        // Switch to windowed mode
        glfwSetWindowMonitor(window_, nullptr, 100, 100, 
                           config_.width, config_.height, 0);
    }
    
    config_.fullscreen = fullscreen;
}

bool Window::IsMinimized() const {
    return window_ ? glfwGetWindowAttrib(window_, GLFW_ICONIFIED) : false;
}

bool Window::IsMaximized() const {
    return window_ ? glfwGetWindowAttrib(window_, GLFW_MAXIMIZED) : false;
}

bool Window::IsFocused() const {
    return window_ ? glfwGetWindowAttrib(window_, GLFW_FOCUSED) : false;
}

void Window::Minimize() {
    if (window_) {
        glfwIconifyWindow(window_);
    }
}

void Window::Maximize() {
    if (window_) {
        glfwMaximizeWindow(window_);
    }
}

void Window::Restore() {
    if (window_) {
        glfwRestoreWindow(window_);
    }
}

void Window::Show() {
    if (window_) {
        glfwShowWindow(window_);
        config_.visible = true;
    }
}

void Window::Hide() {
    if (window_) {
        glfwHideWindow(window_);
        config_.visible = false;
    }
}

bool Window::IsVSyncEnabled() const {
    return config_.vsync;
}

void Window::SetVSync(bool enabled) {
    if (window_) {
        glfwSwapInterval(enabled ? 1 : 0);
        config_.vsync = enabled;
    }
}

void Window::SetEventCallback(EventCallback callback) {
    event_callback_ = std::move(callback);
}

double Window::GetTime() const {
    return glfwGetTime();
}

void Window::MakeContextCurrent() {
    if (window_) {
        glfwMakeContextCurrent(window_);
    }
}

void Window::SetupCallbacks() {
    if (!window_) return;
    
    glfwSetWindowCloseCallback(window_, WindowCloseCallback);
    glfwSetWindowSizeCallback(window_, WindowSizeCallback);
    glfwSetWindowFocusCallback(window_, WindowFocusCallback);
    glfwSetWindowIconifyCallback(window_, WindowIconifyCallback);
    glfwSetWindowMaximizeCallback(window_, WindowMaximizeCallback);
}

void Window::TriggerEvent(const WindowEvent& event) {
    if (event_callback_) {
        event_callback_(event);
    }
}

// Static GLFW callbacks
void Window::WindowCloseCallback(GLFWwindow* window) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        WindowEvent event{};
        event.type = WindowEventType::Close;
        win->TriggerEvent(event);
    }
}

void Window::WindowSizeCallback(GLFWwindow* window, int width, int height) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        win->config_.width = width;
        win->config_.height = height;
        
        WindowEvent event{};
        event.type = WindowEventType::Resize;
        event.width = width;
        event.height = height;
        win->TriggerEvent(event);
    }
}

void Window::WindowFocusCallback(GLFWwindow* window, int focused) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        WindowEvent event{};
        event.type = focused ? WindowEventType::Focus : WindowEventType::Unfocus;
        event.focused = focused != 0;
        win->TriggerEvent(event);
    }
}

void Window::WindowIconifyCallback(GLFWwindow* window, int iconified) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        WindowEvent event{};
        event.type = iconified ? WindowEventType::Minimize : WindowEventType::Restore;
        win->TriggerEvent(event);
    }
}

void Window::WindowMaximizeCallback(GLFWwindow* window, int maximized) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        WindowEvent event{};
        event.type = maximized ? WindowEventType::Maximize : WindowEventType::Restore;
        win->TriggerEvent(event);
    }
}

} // namespace Window
} // namespace PyNovaGE