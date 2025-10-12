#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <renderer/renderer.hpp>
#include <renderer/sprite_renderer.hpp>
#include <renderer/batch_renderer.hpp>
#include <renderer/texture.hpp>
#include <renderer/shader.hpp>

namespace py = pybind11;

void bind_renderer(py::module& m) {
    auto renderer_module = m.def_submodule("renderer", "Rendering system");
    
    // RendererConfig structure
    py::class_<PyNovaGE::Renderer::RendererConfig>(renderer_module, "RendererConfig")
        .def(py::init<>())
        .def_readwrite("enable_vsync", &PyNovaGE::Renderer::RendererConfig::enable_vsync)
        .def_readwrite("enable_depth_test", &PyNovaGE::Renderer::RendererConfig::enable_depth_test)
        .def_readwrite("enable_blend", &PyNovaGE::Renderer::RendererConfig::enable_blend)
        .def_readwrite("enable_culling", &PyNovaGE::Renderer::RendererConfig::enable_culling)
        .def_readwrite("max_sprites_per_batch", &PyNovaGE::Renderer::RendererConfig::max_sprites_per_batch)
        .def_readwrite("max_textures_per_batch", &PyNovaGE::Renderer::RendererConfig::max_textures_per_batch)
        .def("__str__", [](const PyNovaGE::Renderer::RendererConfig& config) {
            return "RendererConfig(vsync=" + std::to_string(config.enable_vsync) + 
                   ", depth_test=" + std::to_string(config.enable_depth_test) + ")";
        });
    
    // RenderStats structure
    py::class_<PyNovaGE::Renderer::RenderStats>(renderer_module, "RenderStats")
        .def_readonly("draw_calls", &PyNovaGE::Renderer::RenderStats::draw_calls)
        .def_readonly("sprites_rendered", &PyNovaGE::Renderer::RenderStats::sprites_rendered)
        .def_readonly("vertices_rendered", &PyNovaGE::Renderer::RenderStats::vertices_rendered)
        .def_readonly("triangles_rendered", &PyNovaGE::Renderer::RenderStats::triangles_rendered)
        .def_readonly("texture_binds", &PyNovaGE::Renderer::RenderStats::texture_binds)
        .def_readonly("shader_binds", &PyNovaGE::Renderer::RenderStats::shader_binds)
        .def_readonly("frame_time_ms", &PyNovaGE::Renderer::RenderStats::frame_time_ms)
        .def("reset", &PyNovaGE::Renderer::RenderStats::Reset)
        .def("__str__", [](const PyNovaGE::Renderer::RenderStats& stats) {
            return "RenderStats(draw_calls=" + std::to_string(stats.draw_calls) + 
                   ", sprites=" + std::to_string(stats.sprites_rendered) + 
                   ", frame_time=" + std::to_string(stats.frame_time_ms) + "ms)";
        });
    
    // RenderAPI enum
    py::enum_<PyNovaGE::Renderer::RenderAPI>(renderer_module, "RenderAPI")
        .value("OPENGL", PyNovaGE::Renderer::RenderAPI::OpenGL)
        .export_values();
    
    // RendererGuard class
    py::class_<PyNovaGE::Renderer::RendererGuard>(renderer_module, "RendererGuard")
        .def(py::init<const PyNovaGE::Renderer::RendererConfig&>(),
             py::arg("config") = PyNovaGE::Renderer::RendererConfig{})
        .def("is_initialized", &PyNovaGE::Renderer::RendererGuard::IsInitialized)
        .def("__bool__", &PyNovaGE::Renderer::RendererGuard::IsInitialized);
    
    // Main Renderer class (static methods only - using module level functions)
    // Note: Renderer is a static-only class with deleted constructor/destructor
    renderer_module.def("initialize", &PyNovaGE::Renderer::Renderer::Initialize,
                       py::arg("config") = PyNovaGE::Renderer::RendererConfig{});
    renderer_module.def("shutdown", &PyNovaGE::Renderer::Renderer::Shutdown);
    renderer_module.def("is_initialized", &PyNovaGE::Renderer::Renderer::IsInitialized);
    renderer_module.def("get_api", &PyNovaGE::Renderer::Renderer::GetAPI);
    // Frame control
    renderer_module.def("begin_frame", &PyNovaGE::Renderer::Renderer::BeginFrame);
    renderer_module.def("end_frame", &PyNovaGE::Renderer::Renderer::EndFrame);
    // Viewport and clearing
    renderer_module.def("set_viewport", &PyNovaGE::Renderer::Renderer::SetViewport);
    renderer_module.def("clear", &PyNovaGE::Renderer::Renderer::Clear,
                       py::arg("color") = PyNovaGE::Vector4f{0.0f, 0.0f, 0.0f, 1.0f});
    renderer_module.def("set_clear_color", &PyNovaGE::Renderer::Renderer::SetClearColor);
    // Render state
    renderer_module.def("set_depth_test", &PyNovaGE::Renderer::Renderer::SetDepthTest);
    renderer_module.def("set_blending", &PyNovaGE::Renderer::Renderer::SetBlending);
    renderer_module.def("set_culling", &PyNovaGE::Renderer::Renderer::SetCulling);
    renderer_module.def("set_wireframe", &PyNovaGE::Renderer::Renderer::SetWireframe);
    // Statistics and info
    renderer_module.def("get_stats", &PyNovaGE::Renderer::Renderer::GetStats, 
                       py::return_value_policy::reference_internal);
    renderer_module.def("get_renderer_info", &PyNovaGE::Renderer::Renderer::GetRendererInfo);
    renderer_module.def("check_gl_error", &PyNovaGE::Renderer::Renderer::CheckGLError,
                       py::arg("operation") = "");
    // Renderer instances
    renderer_module.def("get_sprite_renderer", &PyNovaGE::Renderer::Renderer::GetSpriteRenderer,
                       py::return_value_policy::reference_internal);
    renderer_module.def("get_batch_renderer", &PyNovaGE::Renderer::Renderer::GetBatchRenderer,
                       py::return_value_policy::reference_internal);
    // Texture class (basic bindings - simplified for now)
    py::class_<PyNovaGE::Renderer::Texture>(renderer_module, "Texture")
        .def(py::init<>())
        .def("get_width", &PyNovaGE::Renderer::Texture::GetWidth)
        .def("get_height", &PyNovaGE::Renderer::Texture::GetHeight)
        .def("get_texture_id", &PyNovaGE::Renderer::Texture::GetTextureID)
        .def("is_valid", &PyNovaGE::Renderer::Texture::IsValid)
        .def("__str__", [](const PyNovaGE::Renderer::Texture& texture) {
            return "Texture(id=" + std::to_string(texture.GetTextureID()) + ", size=" +
                   std::to_string(texture.GetWidth()) + "x" + std::to_string(texture.GetHeight()) + ")";
        });
    
    // SpriteRenderer class (basic binding - actual methods to be added based on real API)
    py::class_<PyNovaGE::Renderer::SpriteRenderer>(renderer_module, "SpriteRenderer");
    // Note: Actual SpriteRenderer methods need to be examined and added
    
    // Helper functions will be added later when texture loading is properly implemented
    
    // Color constants
    renderer_module.attr("BLACK") = PyNovaGE::Vector4f{0.0f, 0.0f, 0.0f, 1.0f};
    renderer_module.attr("WHITE") = PyNovaGE::Vector4f{1.0f, 1.0f, 1.0f, 1.0f};
    renderer_module.attr("RED") = PyNovaGE::Vector4f{1.0f, 0.0f, 0.0f, 1.0f};
    renderer_module.attr("GREEN") = PyNovaGE::Vector4f{0.0f, 1.0f, 0.0f, 1.0f};
    renderer_module.attr("BLUE") = PyNovaGE::Vector4f{0.0f, 0.0f, 1.0f, 1.0f};
    renderer_module.attr("YELLOW") = PyNovaGE::Vector4f{1.0f, 1.0f, 0.0f, 1.0f};
    renderer_module.attr("MAGENTA") = PyNovaGE::Vector4f{1.0f, 0.0f, 1.0f, 1.0f};
    renderer_module.attr("CYAN") = PyNovaGE::Vector4f{0.0f, 1.0f, 1.0f, 1.0f};
    renderer_module.attr("CLEAR") = PyNovaGE::Vector4f{0.0f, 0.0f, 0.0f, 0.0f};
}