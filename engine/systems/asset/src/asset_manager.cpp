#include "asset/asset_manager.hpp"
#include "asset/font.hpp"
#include "asset/audio_clip.hpp"
#include "renderer/texture.hpp"

#include <filesystem>
#include <iostream>
#include <ctime>
#include <algorithm>

// For stb_image_write
#include "../../../third_party/stb/stb_image_write.h"

namespace PyNovaGE {
namespace Asset {

AssetManager& AssetManager::Instance() {
    static AssetManager instance;
    return instance;
}

AssetManager::~AssetManager() {
    Shutdown();
}

bool AssetManager::Initialize(const std::string& asset_root_path) {
    if (initialized_) {
        return true;
    }

    asset_root_ = asset_root_path;
    
    // Ensure the asset root path exists
    if (!std::filesystem::exists(asset_root_)) {
        std::filesystem::create_directories(asset_root_);
    }

    initialized_ = true;
    std::cout << "AssetManager initialized with root: " << asset_root_ << std::endl;
    return true;
}

void AssetManager::Shutdown() {
    if (!initialized_) {
        return;
    }

    UnloadAllAssets();
    change_callbacks_.clear();
    file_timestamps_.clear();
    
    initialized_ = false;
    std::cout << "AssetManager shut down" << std::endl;
}

AssetResult<Renderer::Texture> AssetManager::LoadTexture(const std::string& relative_path) {
    AssetResult<Renderer::Texture> result;
    
    if (!initialized_) {
        result.error_message = "AssetManager not initialized";
        return result;
    }

    std::string full_path = GetFullPath(relative_path);
    
    // Check if already loaded
    auto existing = GetTexture(relative_path);
    if (existing) {
        result.asset = existing;
        result.success = true;
        return result;
    }

    // Create new texture and load
    auto texture = std::make_shared<Renderer::Texture>();
    if (texture->LoadFromFile(full_path)) {
        textures_[relative_path] = texture;
        result.asset = texture;
        result.success = true;
        
        // Track for hot reloading
        if (hot_reloading_enabled_) {
            try {
                auto write_time = std::filesystem::last_write_time(full_path);
                auto time_t = std::chrono::system_clock::to_time_t(
                    std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        write_time - std::filesystem::file_time_type::clock::now() + 
                        std::chrono::system_clock::now()
                    )
                );
                file_timestamps_[relative_path] = time_t;
            } catch (...) {
                // Ignore timestamp errors
            }
        }
    } else {
        result.error_message = "Failed to load texture from: " + full_path;
    }

    return result;
}

std::shared_ptr<Renderer::Texture> AssetManager::GetTexture(const std::string& relative_path) {
    auto it = textures_.find(relative_path);
    if (it != textures_.end()) {
        return it->second;
    }
    return nullptr;
}

AssetResult<Font> AssetManager::LoadFont(const std::string& relative_path, float size) {
    AssetResult<Font> result;
    
    if (!initialized_) {
        result.error_message = "AssetManager not initialized";
        return result;
    }

    std::string cache_key = relative_path + "_" + std::to_string(size);
    std::string full_path = GetFullPath(relative_path);
    
    // Check if already loaded
    auto existing = GetFont(relative_path, size);
    if (existing) {
        result.asset = existing;
        result.success = true;
        return result;
    }

    // Create new font and load
    auto font = std::make_shared<Font>(relative_path, size);
    if (font->LoadFromFile(full_path)) {
        fonts_[cache_key] = font;
        result.asset = font;
        result.success = true;
        
        // Track for hot reloading
        if (hot_reloading_enabled_) {
            try {
                auto write_time = std::filesystem::last_write_time(full_path);
                auto time_t = std::chrono::system_clock::to_time_t(
                    std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        write_time - std::filesystem::file_time_type::clock::now() + 
                        std::chrono::system_clock::now()
                    )
                );
                file_timestamps_[cache_key] = time_t;
            } catch (...) {
                // Ignore timestamp errors
            }
        }
    } else {
        result.error_message = "Failed to load font from: " + full_path;
    }

    return result;
}

std::shared_ptr<Font> AssetManager::GetFont(const std::string& relative_path, float size) {
    std::string cache_key = relative_path + "_" + std::to_string(size);
    auto it = fonts_.find(cache_key);
    if (it != fonts_.end()) {
        return it->second;
    }
    return nullptr;
}

AssetResult<AudioClip> AssetManager::LoadAudio(const std::string& relative_path) {
    AssetResult<AudioClip> result;
    
    if (!initialized_) {
        result.error_message = "AssetManager not initialized";
        return result;
    }

    std::string full_path = GetFullPath(relative_path);
    
    // Check if already loaded
    auto existing = GetAudio(relative_path);
    if (existing) {
        result.asset = existing;
        result.success = true;
        return result;
    }

    // Create new audio clip and load
    auto audio = std::make_shared<AudioClip>(relative_path);
    if (audio->LoadFromFile(full_path)) {
        audio_clips_[relative_path] = audio;
        result.asset = audio;
        result.success = true;
        
        // Track for hot reloading
        if (hot_reloading_enabled_) {
            try {
                auto write_time = std::filesystem::last_write_time(full_path);
                auto time_t = std::chrono::system_clock::to_time_t(
                    std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        write_time - std::filesystem::file_time_type::clock::now() + 
                        std::chrono::system_clock::now()
                    )
                );
                file_timestamps_[relative_path] = time_t;
            } catch (...) {
                // Ignore timestamp errors
            }
        }
    } else {
        result.error_message = "Failed to load audio from: " + full_path;
    }

    return result;
}

std::shared_ptr<AudioClip> AssetManager::GetAudio(const std::string& relative_path) {
    auto it = audio_clips_.find(relative_path);
    if (it != audio_clips_.end()) {
        return it->second;
    }
    return nullptr;
}

bool AssetManager::SaveImagePNG(const std::string& path, int width, int height, int channels, const void* data) {
    std::string full_path = GetFullPath(path);
    
    // Ensure directory exists
    auto dir = std::filesystem::path(full_path).parent_path();
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
    
    int result = stbi_write_png(full_path.c_str(), width, height, channels, data, width * channels);
    return result != 0;
}

bool AssetManager::SaveImageJPG(const std::string& path, int width, int height, int channels, const void* data, int quality) {
    std::string full_path = GetFullPath(path);
    
    // Ensure directory exists
    auto dir = std::filesystem::path(full_path).parent_path();
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
    
    int result = stbi_write_jpg(full_path.c_str(), width, height, channels, data, quality);
    return result != 0;
}

void AssetManager::UnloadAsset(const std::string& relative_path) {
    // Try to find and erase from each map
    textures_.erase(relative_path);
    audio_clips_.erase(relative_path);
    
    // For fonts, we need to check all possible size variations
    auto it = fonts_.begin();
    while (it != fonts_.end()) {
        if (it->first.substr(0, relative_path.length()) == relative_path) {
            it = fonts_.erase(it);
        } else {
            ++it;
        }
    }
    
    file_timestamps_.erase(relative_path);
}

void AssetManager::UnloadAllAssets() {
    textures_.clear();
    fonts_.clear();
    audio_clips_.clear();
    file_timestamps_.clear();
}

void AssetManager::ReloadAsset(const std::string& relative_path) {
    // Detect asset type by extension and reload
    AssetType type = DetectAssetType(relative_path);
    UnloadAsset(relative_path);
    
    switch (type) {
        case AssetType::Texture:
            LoadTexture(relative_path);
            break;
        case AssetType::Audio:
            LoadAudio(relative_path);
            break;
        case AssetType::Font:
            // Font reloading is more complex due to size parameter
            // For now, we'll skip automatic font reloading
            break;
        default:
            break;
    }
}

size_t AssetManager::GetLoadedAssetCount() const {
    return textures_.size() + fonts_.size() + audio_clips_.size();
}

void AssetManager::EnableHotReloading(bool enable) {
    hot_reloading_enabled_ = enable;
    if (!enable) {
        file_timestamps_.clear();
    }
}

void AssetManager::RegisterChangeCallback(AssetChangeCallback callback) {
    change_callbacks_.push_back(callback);
}

void AssetManager::Update() {
    if (!hot_reloading_enabled_ || !initialized_) {
        return;
    }
    
    CheckForChanges();
}

std::string AssetManager::GetFullPath(const std::string& relative_path) const {
    return asset_root_ + relative_path;
}

AssetType AssetManager::DetectAssetType(const std::string& path) const {
    std::string ext = std::filesystem::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp") {
        return AssetType::Texture;
    }
    if (ext == ".ttf" || ext == ".otf") {
        return AssetType::Font;
    }
    if (ext == ".wav" || ext == ".ogg") {
        return AssetType::Audio;
    }
    
    return AssetType::Unknown;
}

void AssetManager::CheckForChanges() {
    for (auto& [path, timestamp] : file_timestamps_) {
        try {
            std::string full_path = GetFullPath(path);
            if (!std::filesystem::exists(full_path)) {
                continue;
            }
            
            auto write_time = std::filesystem::last_write_time(full_path);
            auto new_time_t = std::chrono::system_clock::to_time_t(
                std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    write_time - std::filesystem::file_time_type::clock::now() + 
                    std::chrono::system_clock::now()
                )
            );
            
            if (new_time_t > timestamp) {
                // File changed, reload it
                AssetType type = DetectAssetType(path);
                ReloadAsset(path);
                file_timestamps_[path] = new_time_t;
                
                // Notify callbacks
                for (auto& callback : change_callbacks_) {
                    callback(path, type);
                }
            }
        } catch (...) {
            // Ignore errors during file checking
        }
    }
}

} // namespace Asset
} // namespace PyNovaGE