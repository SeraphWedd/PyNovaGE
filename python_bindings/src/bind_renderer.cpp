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
    
    // Sprite data structure
    py::class_<PyNovaGE::Renderer::Sprite>(renderer_module, "Sprite")
        .def(py::init<>())
        .def(py::init<const PyNovaGE::Vector2f&, std::shared_ptr<PyNovaGE::Renderer::Texture>>())
        .def_readwrite("position", &PyNovaGE::Renderer::Sprite::position)
        .def_readwrite("rotation", &PyNovaGE::Renderer::Sprite::rotation)
        .def_readwrite("scale", &PyNovaGE::Renderer::Sprite::scale)
        .def_readwrite("origin", &PyNovaGE::Renderer::Sprite::origin)
        .def_readwrite("color", &PyNovaGE::Renderer::Sprite::color)
        .def_readwrite("texture", &PyNovaGE::Renderer::Sprite::texture)
        .def_readwrite("size", &PyNovaGE::Renderer::Sprite::size)
        .def("set_texture_region", &PyNovaGE::Renderer::Sprite::SetTextureRegion)
        .def("set_texture_region_normalized", &PyNovaGE::Renderer::Sprite::SetTextureRegionNormalized)
        .def("__str__", [](const PyNovaGE::Renderer::Sprite& sprite) {
            return "Sprite(pos=(" + std::to_string(sprite.position.x) + "," + 
                   std::to_string(sprite.position.y) + "), size=(" + 
                   std::to_string(sprite.size.x) + "," + std::to_string(sprite.size.y) + "))";
        });
    
    // SpriteRenderer class with proper method bindings
    py::class_<PyNovaGE::Renderer::SpriteRenderer>(renderer_module, "SpriteRenderer")
        .def(py::init<>())
        .def("initialize", &PyNovaGE::Renderer::SpriteRenderer::Initialize)
        .def("shutdown", &PyNovaGE::Renderer::SpriteRenderer::Shutdown)
        .def("render_sprite", &PyNovaGE::Renderer::SpriteRenderer::RenderSprite)
        .def("is_initialized", &PyNovaGE::Renderer::SpriteRenderer::IsInitialized)
        .def("__str__", [](const PyNovaGE::Renderer::SpriteRenderer& renderer) {
            return "SpriteRenderer(initialized=" + std::to_string(renderer.IsInitialized()) + ")";
        });
    
    // BatchVertex structure
    py::class_<PyNovaGE::Renderer::BatchVertex>(renderer_module, "BatchVertex")
        .def(py::init<>())
        .def(py::init<const PyNovaGE::Vector3f&, const PyNovaGE::Vector2f&, const PyNovaGE::Vector4f&, float>())
        .def_readwrite("position", &PyNovaGE::Renderer::BatchVertex::position)
        .def_readwrite("tex_coords", &PyNovaGE::Renderer::BatchVertex::texCoords)
        .def_readwrite("color", &PyNovaGE::Renderer::BatchVertex::color)
        .def_readwrite("texture_index", &PyNovaGE::Renderer::BatchVertex::textureIndex);
    
    // BatchStats structure
    py::class_<PyNovaGE::Renderer::BatchStats>(renderer_module, "BatchStats")
        .def_readonly("draw_calls", &PyNovaGE::Renderer::BatchStats::draw_calls)
        .def_readonly("sprites_batched", &PyNovaGE::Renderer::BatchStats::sprites_batched)
        .def_readonly("batches_flushed", &PyNovaGE::Renderer::BatchStats::batches_flushed)
        .def_readonly("texture_binds", &PyNovaGE::Renderer::BatchStats::texture_binds)
        .def_readonly("avg_sprites_per_batch", &PyNovaGE::Renderer::BatchStats::avg_sprites_per_batch)
        .def("reset", &PyNovaGE::Renderer::BatchStats::Reset)
        .def("update_average", &PyNovaGE::Renderer::BatchStats::UpdateAverage);
    
    // BatchRenderer class with drawing primitives
    py::class_<PyNovaGE::Renderer::BatchRenderer>(renderer_module, "BatchRenderer")
        .def(py::init<int, int>(), py::arg("max_sprites") = 1000, py::arg("max_textures") = 16)
        .def("initialize", &PyNovaGE::Renderer::BatchRenderer::Initialize)
        .def("shutdown", &PyNovaGE::Renderer::BatchRenderer::Shutdown)
        .def("is_initialized", &PyNovaGE::Renderer::BatchRenderer::IsInitialized)
        // Batch control
        .def("begin_batch", &PyNovaGE::Renderer::BatchRenderer::BeginBatch)
        .def("add_sprite", &PyNovaGE::Renderer::BatchRenderer::AddSprite)
        .def("flush_batch", &PyNovaGE::Renderer::BatchRenderer::FlushBatch)
        .def("end_batch", &PyNovaGE::Renderer::BatchRenderer::EndBatch)
        // Sprite rendering
        .def("render_sprites", py::overload_cast<const std::vector<PyNovaGE::Renderer::Sprite>&>(&PyNovaGE::Renderer::BatchRenderer::RenderSprites))
        // Primitive drawing functions
        .def("add_rect_screen", &PyNovaGE::Renderer::BatchRenderer::AddRectScreen)
        .def("add_line_screen", &PyNovaGE::Renderer::BatchRenderer::AddLineScreen)
        .def("add_textured_quad_screen", &PyNovaGE::Renderer::BatchRenderer::AddTexturedQuadScreen,
             py::arg("x"), py::arg("y"), py::arg("w"), py::arg("h"), 
             py::arg("screen_w"), py::arg("screen_h"), py::arg("texture"),
             py::arg("color") = PyNovaGE::Vector4f{1.0f, 1.0f, 1.0f, 1.0f})
        .def("add_circle_screen", &PyNovaGE::Renderer::BatchRenderer::AddCircleScreen,
             py::arg("x"), py::arg("y"), py::arg("radius"), 
             py::arg("screen_w"), py::arg("screen_h"), py::arg("color"),
             py::arg("segments") = 32)
        // Statistics
        .def("get_stats", &PyNovaGE::Renderer::BatchRenderer::GetStats, py::return_value_policy::reference_internal)
        .def("reset_stats", &PyNovaGE::Renderer::BatchRenderer::ResetStats)
        .def("get_max_sprites", &PyNovaGE::Renderer::BatchRenderer::GetMaxSprites)
        .def("get_max_textures", &PyNovaGE::Renderer::BatchRenderer::GetMaxTextures)
        .def("__str__", [](const PyNovaGE::Renderer::BatchRenderer& renderer) {
            return "BatchRenderer(max_sprites=" + std::to_string(renderer.GetMaxSprites()) + 
                   ", max_textures=" + std::to_string(renderer.GetMaxTextures()) + ")";
        });
    
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