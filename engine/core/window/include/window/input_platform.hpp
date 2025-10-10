#pragma once

#include <vectors/vector2.hpp>
#include <array>
#include <functional>
#include <string>
#include <memory>

namespace PyNovaGE {
namespace Window {

// Forward declarations
enum class Key;
enum class MouseButton;
enum class GamepadButton;
enum class GamepadAxis;
enum class InputEventType;
struct InputEvent;
struct GamepadState;
enum class InputState;

/**
 * @brief Touch input data (for mobile platforms)
 */
struct TouchData {
    int id = 0;                          // Touch finger ID
    PyNovaGE::Vector2f position{0.0f, 0.0f};  // Touch position
    float pressure = 1.0f;              // Pressure (0.0 to 1.0)
    bool active = false;                 // Whether touch is currently active
};

/**
 * @brief Touch event types
 */
enum class TouchEventType {
    TouchDown,      // Finger touched screen
    TouchUp,        // Finger lifted from screen
    TouchMove,      // Finger moved on screen
    TouchCancel     // Touch was cancelled (e.g., system interrupt)
};

/**
 * @brief Touch event data
 */
struct TouchEvent {
    TouchEventType type;
    TouchData touch;
    PyNovaGE::Vector2f delta{0.0f, 0.0f};  // Movement delta for TouchMove
};

/**
 * @brief Platform-specific input backend interface
 */
class IInputPlatform {
public:
    using InputCallback = std::function<void(const InputEvent&)>;
    using TouchCallback = std::function<void(const TouchEvent&)>;
    
    virtual ~IInputPlatform() = default;
    
    // Core lifecycle
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Update() = 0;
    
    // Callback registration
    virtual void SetInputCallback(InputCallback callback) = 0;
    virtual void SetTouchCallback(TouchCallback callback) = 0;
    
    // Keyboard support
    virtual bool IsKeyPressed(Key key) const = 0;
    virtual bool IsKeyJustPressed(Key key) const = 0;
    virtual bool IsKeyJustReleased(Key key) const = 0;
    
    // Mouse support (for desktop platforms)
    virtual bool IsMouseButtonPressed(MouseButton button) const = 0;
    virtual bool IsMouseButtonJustPressed(MouseButton button) const = 0;
    virtual bool IsMouseButtonJustReleased(MouseButton button) const = 0;
    virtual PyNovaGE::Vector2f GetMousePosition() const = 0;
    virtual PyNovaGE::Vector2f GetMouseDelta() const = 0;
    virtual PyNovaGE::Vector2f GetScrollDelta() const = 0;
    virtual void SetMouseCursorVisible(bool visible) = 0;
    virtual bool IsMouseCursorVisible() const = 0;
    virtual void SetMouseCursorMode(int mode) = 0;
    
    // Touch support (for mobile platforms)
    virtual bool SupportsTouchInput() const = 0;
    virtual int GetActiveTouchCount() const = 0;
    virtual TouchData GetTouch(int touch_id) const = 0;
    virtual std::vector<TouchData> GetActiveTouches() const = 0;
    
    // Gamepad support
    virtual bool IsGamepadConnected(int gamepad_id) const = 0;
    virtual const GamepadState& GetGamepadState(int gamepad_id) const = 0;
    virtual bool IsGamepadButtonPressed(int gamepad_id, GamepadButton button) const = 0;
    virtual bool IsGamepadButtonJustPressed(int gamepad_id, GamepadButton button) const = 0;
    virtual bool IsGamepadButtonJustReleased(int gamepad_id, GamepadButton button) const = 0;
    virtual float GetGamepadAxis(int gamepad_id, GamepadAxis axis) const = 0;
    
    // Platform capabilities
    virtual bool SupportsKeyboard() const = 0;
    virtual bool SupportsMouse() const = 0;
    virtual bool SupportsGamepad() const = 0;
    virtual std::string GetPlatformName() const = 0;
};

/**
 * @brief Desktop input platform (GLFW-based)
 */
class DesktopInputPlatform : public IInputPlatform {
public:
    explicit DesktopInputPlatform(void* native_window); // GLFWwindow*
    ~DesktopInputPlatform() override;
    
    // IInputPlatform implementation
    bool Initialize() override;
    void Shutdown() override;
    void Update() override;
    
    void SetInputCallback(InputCallback callback) override;
    void SetTouchCallback(TouchCallback callback) override;
    
    bool IsKeyPressed(Key key) const override;
    bool IsKeyJustPressed(Key key) const override;
    bool IsKeyJustReleased(Key key) const override;
    
    bool IsMouseButtonPressed(MouseButton button) const override;
    bool IsMouseButtonJustPressed(MouseButton button) const override;
    bool IsMouseButtonJustReleased(MouseButton button) const override;
    PyNovaGE::Vector2f GetMousePosition() const override;
    PyNovaGE::Vector2f GetMouseDelta() const override;
    PyNovaGE::Vector2f GetScrollDelta() const override;
    void SetMouseCursorVisible(bool visible) override;
    bool IsMouseCursorVisible() const override;
    void SetMouseCursorMode(int mode) override;
    
    bool SupportsTouchInput() const override { return false; }
    int GetActiveTouchCount() const override { return 0; }
    TouchData GetTouch(int touch_id) const override;
    std::vector<TouchData> GetActiveTouches() const override { return {}; }
    
    bool IsGamepadConnected(int gamepad_id) const override;
    const GamepadState& GetGamepadState(int gamepad_id) const override;
    bool IsGamepadButtonPressed(int gamepad_id, GamepadButton button) const override;
    bool IsGamepadButtonJustPressed(int gamepad_id, GamepadButton button) const override;
    bool IsGamepadButtonJustReleased(int gamepad_id, GamepadButton button) const override;
    float GetGamepadAxis(int gamepad_id, GamepadAxis axis) const override;
    
    bool SupportsKeyboard() const override { return true; }
    bool SupportsMouse() const override { return true; }
    bool SupportsGamepad() const override { return true; }
    std::string GetPlatformName() const override { return "Desktop (GLFW)"; }

private:
    class DesktopInputImpl;
    std::unique_ptr<DesktopInputImpl> impl_;
};

#ifdef PYNOVAGE_PLATFORM_ANDROID
/**
 * @brief Android input platform (NDK-based)
 */
class AndroidInputPlatform : public IInputPlatform {
public:
    explicit AndroidInputPlatform(void* native_activity); // ANativeActivity*
    ~AndroidInputPlatform() override;
    
    // IInputPlatform implementation
    bool Initialize() override;
    void Shutdown() override;
    void Update() override;
    
    void SetInputCallback(InputCallback callback) override;
    void SetTouchCallback(TouchCallback callback) override;
    
    bool IsKeyPressed(Key key) const override;
    bool IsKeyJustPressed(Key key) const override;
    bool IsKeyJustReleased(Key key) const override;
    
    // Mouse not supported on Android
    bool IsMouseButtonPressed(MouseButton button) const override { return false; }
    bool IsMouseButtonJustPressed(MouseButton button) const override { return false; }
    bool IsMouseButtonJustReleased(MouseButton button) const override { return false; }
    PyNovaGE::Vector2f GetMousePosition() const override { return {0.0f, 0.0f}; }
    PyNovaGE::Vector2f GetMouseDelta() const override { return {0.0f, 0.0f}; }
    PyNovaGE::Vector2f GetScrollDelta() const override { return {0.0f, 0.0f}; }
    void SetMouseCursorVisible(bool visible) override { (void)visible; }
    bool IsMouseCursorVisible() const override { return false; }
    void SetMouseCursorMode(int mode) override { (void)mode; }
    
    // Touch support
    bool SupportsTouchInput() const override { return true; }
    int GetActiveTouchCount() const override;
    TouchData GetTouch(int touch_id) const override;
    std::vector<TouchData> GetActiveTouches() const override;
    
    // Android gamepad support
    bool IsGamepadConnected(int gamepad_id) const override;
    const GamepadState& GetGamepadState(int gamepad_id) const override;
    bool IsGamepadButtonPressed(int gamepad_id, GamepadButton button) const override;
    bool IsGamepadButtonJustPressed(int gamepad_id, GamepadButton button) const override;
    bool IsGamepadButtonJustReleased(int gamepad_id, GamepadButton button) const override;
    float GetGamepadAxis(int gamepad_id, GamepadAxis axis) const override;
    
    bool SupportsKeyboard() const override { return true; } // Software/hardware keyboard
    bool SupportsMouse() const override { return false; }
    bool SupportsGamepad() const override { return true; }
    std::string GetPlatformName() const override { return "Android"; }

private:
    class AndroidInputImpl;
    std::unique_ptr<AndroidInputImpl> impl_;
};
#endif

/**
 * @brief Factory function to create platform-specific input backend
 */
std::unique_ptr<IInputPlatform> CreateInputPlatform(void* native_handle);

} // namespace Window  
} // namespace PyNovaGE