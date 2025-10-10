#pragma once

#include <memory>
#include <string>
#include <functional>
#include <vectors/vector2.hpp>

struct GLFWwindow;

namespace PyNovaGE {
namespace Window {

/**
 * @brief Window creation and configuration parameters
 */
struct WindowConfig {
    std::string title = "PyNovaGE Window";
    int width = 800;
    int height = 600;
    bool fullscreen = false;
    bool resizable = true;
    bool vsync = true;
    int samples = 0; // MSAA samples, 0 = disabled
    bool visible = true;
};

/**
 * @brief Window events that can be handled
 */
enum class WindowEventType {
    Close,
    Resize,
    Focus,
    Unfocus,
    Minimize,
    Maximize,
    Restore
};

/**
 * @brief Window event data
 */
struct WindowEvent {
    WindowEventType type;
    int width = 0;      // For resize events
    int height = 0;     // For resize events
    bool focused = false; // For focus events
};

/**
 * @brief Cross-platform window abstraction
 */
class Window {
public:
    using EventCallback = std::function<void(const WindowEvent&)>;

    /**
     * @brief Create a new window with the given configuration
     */
    explicit Window(const WindowConfig& config = WindowConfig{});
    
    /**
     * @brief Destructor
     */
    ~Window();

    // Non-copyable but movable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) noexcept;
    Window& operator=(Window&&) noexcept;

    /**
     * @brief Check if the window should close
     */
    bool ShouldClose() const;

    /**
     * @brief Set whether the window should close
     */
    void SetShouldClose(bool should_close);

    /**
     * @brief Poll window events
     */
    void PollEvents();

    /**
     * @brief Swap the front and back buffers
     */
    void SwapBuffers();

    /**
     * @brief Get window size
     */
    PyNovaGE::Vector2i GetSize() const;

    /**
     * @brief Set window size
     */
    void SetSize(int width, int height);

    /**
     * @brief Get framebuffer size (may differ from window size on high-DPI displays)
     */
    PyNovaGE::Vector2i GetFramebufferSize() const;

    /**
     * @brief Get window position
     */
    PyNovaGE::Vector2i GetPosition() const;

    /**
     * @brief Set window position
     */
    void SetPosition(int x, int y);

    /**
     * @brief Get window title
     */
    const std::string& GetTitle() const;

    /**
     * @brief Set window title
     */
    void SetTitle(const std::string& title);

    /**
     * @brief Check if window is fullscreen
     */
    bool IsFullscreen() const;

    /**
     * @brief Set fullscreen mode
     */
    void SetFullscreen(bool fullscreen);

    /**
     * @brief Check if window is minimized
     */
    bool IsMinimized() const;

    /**
     * @brief Check if window is maximized
     */
    bool IsMaximized() const;

    /**
     * @brief Check if window is focused
     */
    bool IsFocused() const;

    /**
     * @brief Minimize the window
     */
    void Minimize();

    /**
     * @brief Maximize the window
     */
    void Maximize();

    /**
     * @brief Restore the window
     */
    void Restore();

    /**
     * @brief Show the window
     */
    void Show();

    /**
     * @brief Hide the window
     */
    void Hide();

    /**
     * @brief Check if VSync is enabled
     */
    bool IsVSyncEnabled() const;

    /**
     * @brief Enable/disable VSync
     */
    void SetVSync(bool enabled);

    /**
     * @brief Set event callback
     */
    void SetEventCallback(EventCallback callback);

    /**
     * @brief Get the native window handle (GLFW window)
     */
    GLFWwindow* GetNativeWindow() const { return window_; }

    /**
     * @brief Get time since window creation in seconds
     */
    double GetTime() const;

    /**
     * @brief Make this window's context current for OpenGL
     */
    void MakeContextCurrent();

private:
    GLFWwindow* window_;
    WindowConfig config_;
    EventCallback event_callback_;
    
    // Static callbacks for GLFW
    static void WindowCloseCallback(GLFWwindow* window);
    static void WindowSizeCallback(GLFWwindow* window, int width, int height);
    static void WindowFocusCallback(GLFWwindow* window, int focused);
    static void WindowIconifyCallback(GLFWwindow* window, int iconified);
    static void WindowMaximizeCallback(GLFWwindow* window, int maximized);
    
    void SetupCallbacks();
    void TriggerEvent(const WindowEvent& event);
};

/**
 * @brief Initialize the window system (must be called before creating windows)
 */
bool InitializeWindowSystem();

/**
 * @brief Shutdown the window system
 */
void ShutdownWindowSystem();

/**
 * @brief RAII wrapper for window system initialization
 */
class WindowSystemGuard {
public:
    WindowSystemGuard();
    ~WindowSystemGuard();
    
    bool IsInitialized() const { return initialized_; }

private:
    bool initialized_;
};

} // namespace Window
} // namespace PyNovaGE