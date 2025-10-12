#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <asset/asset_manager.hpp>
#include <asset/font.hpp>
#include <asset/audio_clip.hpp>
#include <renderer/texture.hpp>

namespace py = pybind11;

namespace PNA = PyNovaGE::Asset;

void bind_asset(py::module& m) {
    auto asset_module = m.def_submodule("asset", "Asset management system");

    // AssetType enum
    py::enum_<PNA::AssetType>(asset_module, "AssetType")
        .value("Texture", PNA::AssetType::Texture)
        .value("Font", PNA::AssetType::Font)
        .value("Audio", PNA::AssetType::Audio)
        .value("Unknown", PNA::AssetType::Unknown);

    // Font class (light bindings)
    py::class_<PNA::Font, std::shared_ptr<PNA::Font>>(asset_module, "Font")
        .def("get_size", &PNA::Font::GetSize)
        .def("get_ascent", &PNA::Font::GetAscent)
        .def("get_descent", &PNA::Font::GetDescent)
        .def("get_line_gap", &PNA::Font::GetLineGap)
        .def("is_loaded", &PNA::Font::IsLoaded);

    // AudioClip class (light bindings)
    py::class_<PNA::AudioClip, std::shared_ptr<PNA::AudioClip>>(asset_module, "AudioClip")
        .def("get_duration", &PNA::AudioClip::GetDuration)
        .def("get_data_size", &PNA::AudioClip::GetDataSize)
        .def("is_loaded", &PNA::AudioClip::IsLoaded);

    // AssetManager functions (module-level wrappers around singleton)
    asset_module.def("initialize", [](const std::string& root){
        return PNA::AssetManager::Instance().Initialize(root);
    }, py::arg("root") = "assets/");

    asset_module.def("shutdown", [](){ PNA::AssetManager::Instance().Shutdown(); });
    asset_module.def("is_initialized", [](){ return PNA::AssetManager::Instance().IsInitialized(); });

    // Textures
    asset_module.def("load_texture", [](const std::string& path){
        auto res = PNA::AssetManager::Instance().LoadTexture(path);
        return res.asset; // shared_ptr<Texture>
    }, py::arg("path"));

    asset_module.def("get_texture", [](const std::string& path){
        return PNA::AssetManager::Instance().GetTexture(path);
    }, py::arg("path"));

    // Fonts
    asset_module.def("load_font", [](const std::string& path, float size){
        auto res = PNA::AssetManager::Instance().LoadFont(path, size);
        return res.asset; // shared_ptr<Font>
    }, py::arg("path"), py::arg("size") = 16.0f);

    asset_module.def("get_font", [](const std::string& path, float size){
        return PNA::AssetManager::Instance().GetFont(path, size);
    }, py::arg("path"), py::arg("size") = 16.0f);

    // Audio
    asset_module.def("load_audio", [](const std::string& path){
        auto res = PNA::AssetManager::Instance().LoadAudio(path);
        return res.asset; // shared_ptr<AudioClip>
    }, py::arg("path"));

    asset_module.def("get_audio", [](const std::string& path){
        return PNA::AssetManager::Instance().GetAudio(path);
    }, py::arg("path"));

    // Image I/O
    asset_module.def("save_png", [](const std::string& path, int w, int h, int channels, py::buffer data){
        py::buffer_info info = data.request();
        return PNA::AssetManager::Instance().SaveImagePNG(path, w, h, channels, info.ptr);
    });

    asset_module.def("save_jpg", [](const std::string& path, int w, int h, int channels, py::buffer data, int quality){
        py::buffer_info info = data.request();
        return PNA::AssetManager::Instance().SaveImageJPG(path, w, h, channels, info.ptr, quality);
    }, py::arg("path"), py::arg("w"), py::arg("h"), py::arg("channels"), py::arg("data"), py::arg("quality") = 90);

    // Hot reloading (basic)
    asset_module.def("enable_hot_reloading", [](bool enable){
        PNA::AssetManager::Instance().EnableHotReloading(enable);
    }, py::arg("enable") = true);

    asset_module.def("update", [](){ PNA::AssetManager::Instance().Update(); });
}
