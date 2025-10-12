#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <window/window.hpp>

namespace py = pybind11;

void bind_window(py::module& m) {
    auto window_module = m.def_submodule("window", "Window management system");
    
    // WindowConfig structure
    py::class_<PyNovaGE::Window::WindowConfig>(window_module, "WindowConfig")
        .def(py::init<>())
        .def_readwrite("title", &PyNovaGE::Window::WindowConfig::title)
        .def_readwrite("width", &PyNovaGE::Window::WindowConfig::width)
        .def_readwrite("height", &PyNovaGE::Window::WindowConfig::height)
        .def_readwrite("fullscreen", &PyNovaGE::Window::WindowConfig::fullscreen)
        .def_readwrite("resizable", &PyNovaGE::Window::WindowConfig::resizable)
        .def_readwrite("vsync", &PyNovaGE::Window::WindowConfig::vsync)
        .def_readwrite("samples", &PyNovaGE::Window::WindowConfig::samples)
        .def_readwrite("visible", &PyNovaGE::Window::WindowConfig::visible)
        .def("__str__", [](const PyNovaGE::Window::WindowConfig& config) {
            return "WindowConfig(title='" + config.title + "', size=" + 
                   std::to_string(config.width) + "x" + std::to_string(config.height) + ")";
        });
    
    // WindowEventType enum
    py::enum_<PyNovaGE::Window::WindowEventType>(window_module, "WindowEventType")
        .value("CLOSE", PyNovaGE::Window::WindowEventType::Close)
        .value("RESIZE", PyNovaGE::Window::WindowEventType::Resize)
        .value("FOCUS", PyNovaGE::Window::WindowEventType::Focus)
        .value("UNFOCUS", PyNovaGE::Window::WindowEventType::Unfocus)
        .value("MINIMIZE", PyNovaGE::Window::WindowEventType::Minimize)
        .value("MAXIMIZE", PyNovaGE::Window::WindowEventType::Maximize)
        .value("RESTORE", PyNovaGE::Window::WindowEventType::Restore)
        .export_values();
    
    // WindowEvent structure
    py::class_<PyNovaGE::Window::WindowEvent>(window_module, "WindowEvent")
        .def_readonly("type", &PyNovaGE::Window::WindowEvent::type)
        .def_readonly("width", &PyNovaGE::Window::WindowEvent::width)
        .def_readonly("height", &PyNovaGE::Window::WindowEvent::height)
        .def_readonly("focused", &PyNovaGE::Window::WindowEvent::focused)
        .def("__str__", [](const PyNovaGE::Window::WindowEvent& event) {
            std::string type_str;
            switch(event.type) {
                case PyNovaGE::Window::WindowEventType::Close: type_str = "CLOSE"; break;
                case PyNovaGE::Window::WindowEventType::Resize: type_str = "RESIZE"; break;
                case PyNovaGE::Window::WindowEventType::Focus: type_str = "FOCUS"; break;
                case PyNovaGE::Window::WindowEventType::Unfocus: type_str = "UNFOCUS"; break;
                case PyNovaGE::Window::WindowEventType::Minimize: type_str = "MINIMIZE"; break;
                case PyNovaGE::Window::WindowEventType::Maximize: type_str = "MAXIMIZE"; break;
                case PyNovaGE::Window::WindowEventType::Restore: type_str = "RESTORE"; break;
            }
            return "WindowEvent(type=" + type_str + ", width=" + std::to_string(event.width) + 
                   ", height=" + std::to_string(event.height) + ")";
        });
    
    // WindowSystemGuard - RAII window system initialization
    py::class_<PyNovaGE::Window::WindowSystemGuard>(window_module, "WindowSystemGuard")
        .def(py::init<>())
        .def("is_initialized", &PyNovaGE::Window::WindowSystemGuard::IsInitialized)
        .def("__bool__", &PyNovaGE::Window::WindowSystemGuard::IsInitialized);
    
    // Window class
    py::class_<PyNovaGE::Window::Window>(window_module, "Window")
        .def(py::init<const PyNovaGE::Window::WindowConfig&>(),
             py::arg("config") = PyNovaGE::Window::WindowConfig{})
        
        // Window lifecycle
        .def("should_close", &PyNovaGE::Window::Window::ShouldClose)
        .def("set_should_close", &PyNovaGE::Window::Window::SetShouldClose)
        .def("poll_events", &PyNovaGE::Window::Window::PollEvents)
        .def("swap_buffers", &PyNovaGE::Window::Window::SwapBuffers)
        
        // Window size and position
        .def("get_size", &PyNovaGE::Window::Window::GetSize)
        .def("set_size", &PyNovaGE::Window::Window::SetSize)
        .def("get_framebuffer_size", &PyNovaGE::Window::Window::GetFramebufferSize)
        .def("get_position", &PyNovaGE::Window::Window::GetPosition)
        .def("set_position", &PyNovaGE::Window::Window::SetPosition)
        
        // Window properties
        .def("get_title", &PyNovaGE::Window::Window::GetTitle, py::return_value_policy::reference_internal)
        .def("set_title", &PyNovaGE::Window::Window::SetTitle)
        
        // Window state
        .def("is_fullscreen", &PyNovaGE::Window::Window::IsFullscreen)
        .def("set_fullscreen", &PyNovaGE::Window::Window::SetFullscreen)
        .def("is_minimized", &PyNovaGE::Window::Window::IsMinimized)
        .def("is_maximized", &PyNovaGE::Window::Window::IsMaximized)
        .def("is_focused", &PyNovaGE::Window::Window::IsFocused)
        
        // Window controls
        .def("minimize", &PyNovaGE::Window::Window::Minimize)
        .def("maximize", &PyNovaGE::Window::Window::Maximize)
        .def("restore", &PyNovaGE::Window::Window::Restore)
        .def("show", &PyNovaGE::Window::Window::Show)
        .def("hide", &PyNovaGE::Window::Window::Hide)
        
        // VSync
        .def("is_vsync_enabled", &PyNovaGE::Window::Window::IsVSyncEnabled)
        .def("set_vsync", &PyNovaGE::Window::Window::SetVSync)
        
        // Time
        .def("get_time", &PyNovaGE::Window::Window::GetTime)
        
        // Event callback
        .def("set_event_callback", &PyNovaGE::Window::Window::SetEventCallback)
        
        // String representation
        .def("__str__", [](const PyNovaGE::Window::Window& window) {
            auto size = window.GetSize();
            return "Window(title='" + window.GetTitle() + "', size=" + 
                   std::to_string(size.x) + "x" + std::to_string(size.y) + ")";
        });
    
    // Python helper functions
    window_module.def("create_window", [](int width = 800, int height = 600, const std::string& title = "PyNovaGE Window") {
        PyNovaGE::Window::WindowConfig config;
        config.width = width;
        config.height = height;
        config.title = title;
        return std::make_unique<PyNovaGE::Window::Window>(config);
    }, py::arg("width") = 800, py::arg("height") = 600, py::arg("title") = "PyNovaGE Window",
       "Create a window with simple parameters");
}