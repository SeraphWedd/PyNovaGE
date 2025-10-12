#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <window/input.hpp>

namespace py = pybind11;

void bind_input(py::module& m) {
    auto input_module = m.def_submodule("input", "Input handling system");
    
    // Key enum
    py::enum_<PyNovaGE::Window::Key>(input_module, "Key")
        .value("UNKNOWN", PyNovaGE::Window::Key::Unknown)
        // Printable keys
        .value("SPACE", PyNovaGE::Window::Key::Space)
        .value("APOSTROPHE", PyNovaGE::Window::Key::Apostrophe)
        .value("COMMA", PyNovaGE::Window::Key::Comma)
        .value("MINUS", PyNovaGE::Window::Key::Minus)
        .value("PERIOD", PyNovaGE::Window::Key::Period)
        .value("SLASH", PyNovaGE::Window::Key::Slash)
        // Numbers
        .value("NUM0", PyNovaGE::Window::Key::Num0)
        .value("NUM1", PyNovaGE::Window::Key::Num1)
        .value("NUM2", PyNovaGE::Window::Key::Num2)
        .value("NUM3", PyNovaGE::Window::Key::Num3)
        .value("NUM4", PyNovaGE::Window::Key::Num4)
        .value("NUM5", PyNovaGE::Window::Key::Num5)
        .value("NUM6", PyNovaGE::Window::Key::Num6)
        .value("NUM7", PyNovaGE::Window::Key::Num7)
        .value("NUM8", PyNovaGE::Window::Key::Num8)
        .value("NUM9", PyNovaGE::Window::Key::Num9)
        .value("SEMICOLON", PyNovaGE::Window::Key::Semicolon)
        .value("EQUAL", PyNovaGE::Window::Key::Equal)
        // Letters
        .value("A", PyNovaGE::Window::Key::A)
        .value("B", PyNovaGE::Window::Key::B)
        .value("C", PyNovaGE::Window::Key::C)
        .value("D", PyNovaGE::Window::Key::D)
        .value("E", PyNovaGE::Window::Key::E)
        .value("F", PyNovaGE::Window::Key::F)
        .value("G", PyNovaGE::Window::Key::G)
        .value("H", PyNovaGE::Window::Key::H)
        .value("I", PyNovaGE::Window::Key::I)
        .value("J", PyNovaGE::Window::Key::J)
        .value("K", PyNovaGE::Window::Key::K)
        .value("L", PyNovaGE::Window::Key::L)
        .value("M", PyNovaGE::Window::Key::M)
        .value("N", PyNovaGE::Window::Key::N)
        .value("O", PyNovaGE::Window::Key::O)
        .value("P", PyNovaGE::Window::Key::P)
        .value("Q", PyNovaGE::Window::Key::Q)
        .value("R", PyNovaGE::Window::Key::R)
        .value("S", PyNovaGE::Window::Key::S)
        .value("T", PyNovaGE::Window::Key::T)
        .value("U", PyNovaGE::Window::Key::U)
        .value("V", PyNovaGE::Window::Key::V)
        .value("W", PyNovaGE::Window::Key::W)
        .value("X", PyNovaGE::Window::Key::X)
        .value("Y", PyNovaGE::Window::Key::Y)
        .value("Z", PyNovaGE::Window::Key::Z)
        .value("LEFT_BRACKET", PyNovaGE::Window::Key::LeftBracket)
        .value("BACKSLASH", PyNovaGE::Window::Key::Backslash)
        .value("RIGHT_BRACKET", PyNovaGE::Window::Key::RightBracket)
        .value("GRAVE_ACCENT", PyNovaGE::Window::Key::GraveAccent)
        // Function keys
        .value("ESCAPE", PyNovaGE::Window::Key::Escape)
        .value("ENTER", PyNovaGE::Window::Key::Enter)
        .value("TAB", PyNovaGE::Window::Key::Tab)
        .value("BACKSPACE", PyNovaGE::Window::Key::Backspace)
        .value("INSERT", PyNovaGE::Window::Key::Insert)
        .value("DELETE", PyNovaGE::Window::Key::Delete)
        .value("RIGHT", PyNovaGE::Window::Key::Right)
        .value("LEFT", PyNovaGE::Window::Key::Left)
        .value("DOWN", PyNovaGE::Window::Key::Down)
        .value("UP", PyNovaGE::Window::Key::Up)
        .value("PAGE_UP", PyNovaGE::Window::Key::PageUp)
        .value("PAGE_DOWN", PyNovaGE::Window::Key::PageDown)
        .value("HOME", PyNovaGE::Window::Key::Home)
        .value("END", PyNovaGE::Window::Key::End)
        .value("CAPS_LOCK", PyNovaGE::Window::Key::CapsLock)
        .value("SCROLL_LOCK", PyNovaGE::Window::Key::ScrollLock)
        .value("NUM_LOCK", PyNovaGE::Window::Key::NumLock)
        .value("PRINT_SCREEN", PyNovaGE::Window::Key::PrintScreen)
        .value("PAUSE", PyNovaGE::Window::Key::Pause)
        .value("F1", PyNovaGE::Window::Key::F1)
        .value("F2", PyNovaGE::Window::Key::F2)
        .value("F3", PyNovaGE::Window::Key::F3)
        .value("F4", PyNovaGE::Window::Key::F4)
        .value("F5", PyNovaGE::Window::Key::F5)
        .value("F6", PyNovaGE::Window::Key::F6)
        .value("F7", PyNovaGE::Window::Key::F7)
        .value("F8", PyNovaGE::Window::Key::F8)
        .value("F9", PyNovaGE::Window::Key::F9)
        .value("F10", PyNovaGE::Window::Key::F10)
        .value("F11", PyNovaGE::Window::Key::F11)
        .value("F12", PyNovaGE::Window::Key::F12)
        // Keypad
        .value("KEYPAD_0", PyNovaGE::Window::Key::Keypad0)
        .value("KEYPAD_1", PyNovaGE::Window::Key::Keypad1)
        .value("KEYPAD_2", PyNovaGE::Window::Key::Keypad2)
        .value("KEYPAD_3", PyNovaGE::Window::Key::Keypad3)
        .value("KEYPAD_4", PyNovaGE::Window::Key::Keypad4)
        .value("KEYPAD_5", PyNovaGE::Window::Key::Keypad5)
        .value("KEYPAD_6", PyNovaGE::Window::Key::Keypad6)
        .value("KEYPAD_7", PyNovaGE::Window::Key::Keypad7)
        .value("KEYPAD_8", PyNovaGE::Window::Key::Keypad8)
        .value("KEYPAD_9", PyNovaGE::Window::Key::Keypad9)
        .value("KEYPAD_DECIMAL", PyNovaGE::Window::Key::KeypadDecimal)
        .value("KEYPAD_DIVIDE", PyNovaGE::Window::Key::KeypadDivide)
        .value("KEYPAD_MULTIPLY", PyNovaGE::Window::Key::KeypadMultiply)
        .value("KEYPAD_SUBTRACT", PyNovaGE::Window::Key::KeypadSubtract)
        .value("KEYPAD_ADD", PyNovaGE::Window::Key::KeypadAdd)
        .value("KEYPAD_ENTER", PyNovaGE::Window::Key::KeypadEnter)
        .value("KEYPAD_EQUAL", PyNovaGE::Window::Key::KeypadEqual)
        // Modifiers
        .value("LEFT_SHIFT", PyNovaGE::Window::Key::LeftShift)
        .value("LEFT_CONTROL", PyNovaGE::Window::Key::LeftControl)
        .value("LEFT_ALT", PyNovaGE::Window::Key::LeftAlt)
        .value("LEFT_SUPER", PyNovaGE::Window::Key::LeftSuper)
        .value("RIGHT_SHIFT", PyNovaGE::Window::Key::RightShift)
        .value("RIGHT_CONTROL", PyNovaGE::Window::Key::RightControl)
        .value("RIGHT_ALT", PyNovaGE::Window::Key::RightAlt)
        .value("RIGHT_SUPER", PyNovaGE::Window::Key::RightSuper)
        .value("MENU", PyNovaGE::Window::Key::Menu)
        .export_values();
    
    // MouseButton enum
    py::enum_<PyNovaGE::Window::MouseButton>(input_module, "MouseButton")
        .value("LEFT", PyNovaGE::Window::MouseButton::Left)
        .value("RIGHT", PyNovaGE::Window::MouseButton::Right)
        .value("MIDDLE", PyNovaGE::Window::MouseButton::Middle)
        .value("BUTTON_4", PyNovaGE::Window::MouseButton::Button4)
        .value("BUTTON_5", PyNovaGE::Window::MouseButton::Button5)
        .value("BUTTON_6", PyNovaGE::Window::MouseButton::Button6)
        .value("BUTTON_7", PyNovaGE::Window::MouseButton::Button7)
        .value("BUTTON_8", PyNovaGE::Window::MouseButton::Button8)
        .export_values();
    
    // GamepadButton enum
    py::enum_<PyNovaGE::Window::GamepadButton>(input_module, "GamepadButton")
        .value("A", PyNovaGE::Window::GamepadButton::A)
        .value("B", PyNovaGE::Window::GamepadButton::B)
        .value("X", PyNovaGE::Window::GamepadButton::X)
        .value("Y", PyNovaGE::Window::GamepadButton::Y)
        .value("LEFT_BUMPER", PyNovaGE::Window::GamepadButton::LeftBumper)
        .value("RIGHT_BUMPER", PyNovaGE::Window::GamepadButton::RightBumper)
        .value("BACK", PyNovaGE::Window::GamepadButton::Back)
        .value("START", PyNovaGE::Window::GamepadButton::Start)
        .value("GUIDE", PyNovaGE::Window::GamepadButton::Guide)
        .value("LEFT_THUMB", PyNovaGE::Window::GamepadButton::LeftThumb)
        .value("RIGHT_THUMB", PyNovaGE::Window::GamepadButton::RightThumb)
        .value("DPAD_UP", PyNovaGE::Window::GamepadButton::DpadUp)
        .value("DPAD_RIGHT", PyNovaGE::Window::GamepadButton::DpadRight)
        .value("DPAD_DOWN", PyNovaGE::Window::GamepadButton::DpadDown)
        .value("DPAD_LEFT", PyNovaGE::Window::GamepadButton::DpadLeft)
        .export_values();
    
    // GamepadAxis enum
    py::enum_<PyNovaGE::Window::GamepadAxis>(input_module, "GamepadAxis")
        .value("LEFT_X", PyNovaGE::Window::GamepadAxis::LeftX)
        .value("LEFT_Y", PyNovaGE::Window::GamepadAxis::LeftY)
        .value("RIGHT_X", PyNovaGE::Window::GamepadAxis::RightX)
        .value("RIGHT_Y", PyNovaGE::Window::GamepadAxis::RightY)
        .value("LEFT_TRIGGER", PyNovaGE::Window::GamepadAxis::LeftTrigger)
        .value("RIGHT_TRIGGER", PyNovaGE::Window::GamepadAxis::RightTrigger)
        .export_values();
    
    // InputState enum
    py::enum_<PyNovaGE::Window::InputState>(input_module, "InputState")
        .value("RELEASED", PyNovaGE::Window::InputState::Released)
        .value("PRESSED", PyNovaGE::Window::InputState::Pressed)
        .value("HELD", PyNovaGE::Window::InputState::Held)
        .export_values();
    
    // InputEventType enum
    py::enum_<PyNovaGE::Window::InputEventType>(input_module, "InputEventType")
        .value("KEY_PRESS", PyNovaGE::Window::InputEventType::KeyPress)
        .value("KEY_RELEASE", PyNovaGE::Window::InputEventType::KeyRelease)
        .value("MOUSE_PRESS", PyNovaGE::Window::InputEventType::MousePress)
        .value("MOUSE_RELEASE", PyNovaGE::Window::InputEventType::MouseRelease)
        .value("MOUSE_MOVE", PyNovaGE::Window::InputEventType::MouseMove)
        .value("MOUSE_SCROLL", PyNovaGE::Window::InputEventType::MouseScroll)
        .value("GAMEPAD_CONNECT", PyNovaGE::Window::InputEventType::GamepadConnect)
        .value("GAMEPAD_DISCONNECT", PyNovaGE::Window::InputEventType::GamepadDisconnect)
        .value("GAMEPAD_BUTTON_PRESS", PyNovaGE::Window::InputEventType::GamepadButtonPress)
        .value("GAMEPAD_BUTTON_RELEASE", PyNovaGE::Window::InputEventType::GamepadButtonRelease)
        .export_values();
    
    // InputEvent structure
    py::class_<PyNovaGE::Window::InputEvent>(input_module, "InputEvent")
        .def_readonly("type", &PyNovaGE::Window::InputEvent::type)
        .def_readonly("key", &PyNovaGE::Window::InputEvent::key)
        .def_readonly("mouse_button", &PyNovaGE::Window::InputEvent::mouse_button)
        .def_readonly("mouse_position", &PyNovaGE::Window::InputEvent::mouse_position)
        .def_readonly("scroll_offset", &PyNovaGE::Window::InputEvent::scroll_offset)
        .def_readonly("gamepad_id", &PyNovaGE::Window::InputEvent::gamepad_id)
        .def_readonly("gamepad_button", &PyNovaGE::Window::InputEvent::gamepad_button)
        .def_readonly("shift", &PyNovaGE::Window::InputEvent::shift)
        .def_readonly("control", &PyNovaGE::Window::InputEvent::control)
        .def_readonly("alt", &PyNovaGE::Window::InputEvent::alt)
        .def_readonly("super", &PyNovaGE::Window::InputEvent::super);
    
    // GamepadState structure
    py::class_<PyNovaGE::Window::GamepadState>(input_module, "GamepadState")
        .def_readonly("connected", &PyNovaGE::Window::GamepadState::connected)
        .def_readonly("name", &PyNovaGE::Window::GamepadState::name)
        .def("get_button", [](const PyNovaGE::Window::GamepadState& state, int button_id) {
            if (button_id < 0 || button_id >= 15) return false;
            return state.buttons[button_id];
        })
        .def("get_axis", [](const PyNovaGE::Window::GamepadState& state, int axis_id) {
            if (axis_id < 0 || axis_id >= 6) return 0.0f;
            return state.axes[axis_id];
        });
    
    // InputManager class
    py::class_<PyNovaGE::Window::InputManager>(input_module, "InputManager")
        // Note: Constructor with GLFWwindow* not exposed due to forward declaration
        
        // Core update
        .def("update", &PyNovaGE::Window::InputManager::Update)
        
        // Keyboard input
        .def("is_key_pressed", &PyNovaGE::Window::InputManager::IsKeyPressed)
        .def("is_key_just_pressed", &PyNovaGE::Window::InputManager::IsKeyJustPressed)
        .def("is_key_just_released", &PyNovaGE::Window::InputManager::IsKeyJustReleased)
        // Note: GetKeyState method doesn't exist, using IsKeyPressed
        
        // Mouse input
        .def("is_mouse_button_pressed", &PyNovaGE::Window::InputManager::IsMouseButtonPressed)
        .def("is_mouse_button_just_pressed", &PyNovaGE::Window::InputManager::IsMouseButtonJustPressed)
        .def("is_mouse_button_just_released", &PyNovaGE::Window::InputManager::IsMouseButtonJustReleased)
        // Note: GetMouseButtonState method doesn't exist, using IsMouseButtonPressed
        .def("get_mouse_position", &PyNovaGE::Window::InputManager::GetMousePosition)
        .def("get_mouse_delta", &PyNovaGE::Window::InputManager::GetMouseDelta)
        .def("get_scroll_delta", &PyNovaGE::Window::InputManager::GetScrollDelta)
        
        // Mouse cursor
        .def("set_mouse_cursor_visible", &PyNovaGE::Window::InputManager::SetMouseCursorVisible)
        .def("is_mouse_cursor_visible", &PyNovaGE::Window::InputManager::IsMouseCursorVisible)
        // Note: SetMousePosition method doesn't exist in the API
        
        // Gamepad input
        .def("is_gamepad_connected", &PyNovaGE::Window::InputManager::IsGamepadConnected)
        .def("get_gamepad_state", &PyNovaGE::Window::InputManager::GetGamepadState, py::return_value_policy::reference_internal)
        .def("is_gamepad_button_pressed", &PyNovaGE::Window::InputManager::IsGamepadButtonPressed)
        .def("is_gamepad_button_just_pressed", &PyNovaGE::Window::InputManager::IsGamepadButtonJustPressed)
        .def("is_gamepad_button_just_released", &PyNovaGE::Window::InputManager::IsGamepadButtonJustReleased)
        // Note: GetGamepadButtonState method doesn't exist, using IsGamepadButtonPressed
        .def("get_gamepad_axis", &PyNovaGE::Window::InputManager::GetGamepadAxis)
        
        // Callbacks
        .def("set_input_callback", &PyNovaGE::Window::InputManager::SetInputCallback);
}