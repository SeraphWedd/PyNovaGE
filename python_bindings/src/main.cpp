#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

// Forward declarations for binding modules
void bind_math(pybind11::module&);
void bind_window(pybind11::module&);
void bind_input(pybind11::module&);
void bind_renderer(pybind11::module&);
void bind_physics(pybind11::module&);
void bind_scene(pybind11::module&);
void bind_asset(pybind11::module&);
void bind_audio(pybind11::module&);

namespace py = pybind11;

PYBIND11_MODULE(pynovage_core, m) {
    m.doc() = "PyNovaGE - Python bindings for the NovaGE game engine";
    
    // Bind subsystems in dependency order
    bind_math(m);        // Foundation - vectors, matrices, transforms
    bind_window(m);      // Window management and events
    bind_input(m);       // Input handling
    bind_renderer(m);    // Rendering system
    bind_physics(m);     // Physics simulation
    bind_scene(m);       // Scene graph and entities
    bind_asset(m);       // Asset management
    bind_audio(m);       // Audio system
    
    // Module version info
    m.attr("__version__") = "0.1.0";
}