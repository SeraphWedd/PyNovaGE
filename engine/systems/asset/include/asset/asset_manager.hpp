#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>

namespace PyNovaGE {
namespace Asset {

// Forward declarations
class Font;
class AudioClip;

} // namespace Asset
} // namespace PyNovaGE

// Forward declarations from other namespaces
namespace PyNovaGE {
namespace Renderer {
    class Texture;
}
}

namespace PyNovaGE {
namespace Asset {

/**
 * @brief Asset types supported by the asset system
 */
enum class AssetType {
    Texture,
    Font,
    Audio,
    Unknown
};

/**
 * @brief Base asset class - all assets inherit from this
 */
class Asset {
public:
    Asset(const std::string& path, AssetType type) : path_(path), type_(type) {}
    virtual ~Asset() = default;

    const std::string& GetPath() const { return path_; }
    AssetType GetType() const { return type_; }
    
    // Non-copyable but movable
    Asset(const Asset&) = delete;
    Asset& operator=(const Asset&) = delete;
    Asset(Asset&&) = default;
    Asset& operator=(Asset&&) = default;

private:
    std::string path_;
    AssetType type_;
};

/**
 * @brief Asset loading result
 */
template<typename T>
struct AssetResult {
    std::shared_ptr<T> asset;
    bool success = false;
    std::string error_message;
};

/**
 * @brief Asset change callback for hot reloading
 */
using AssetChangeCallback = std::function<void(const std::string& path, AssetType type)>;

/**
 * @brief Central asset management system
 * 
 * Handles loading, caching, and hot reloading of all asset types.
 */
class AssetManager {
public:
    /**
     * @brief Get the singleton instance
     */
    static AssetManager& Instance();

    /**
     * @brief Initialize the asset manager
     * @param asset_root_path Root directory for assets
     * @return true if successful
     */
    bool Initialize(const std::string& asset_root_path = "assets/");

    /**
     * @brief Shutdown and cleanup
     */
    void Shutdown();

    /**
     * @brief Check if manager is initialized
     */
    bool IsInitialized() const { return initialized_; }

    // Texture loading
    AssetResult<Renderer::Texture> LoadTexture(const std::string& relative_path);
    std::shared_ptr<Renderer::Texture> GetTexture(const std::string& relative_path);

    // Font loading  
    AssetResult<Font> LoadFont(const std::string& relative_path, float size = 16.0f);
    std::shared_ptr<Font> GetFont(const std::string& relative_path, float size = 16.0f);

    // Audio loading
    AssetResult<AudioClip> LoadAudio(const std::string& relative_path);
    std::shared_ptr<AudioClip> GetAudio(const std::string& relative_path);

    // Image I/O
    bool SaveImagePNG(const std::string& path, int width, int height, int channels, const void* data);
    bool SaveImageJPG(const std::string& path, int width, int height, int channels, const void* data, int quality = 90);

    // Asset management
    void UnloadAsset(const std::string& relative_path);
    void UnloadAllAssets();
    void ReloadAsset(const std::string& relative_path);
    size_t GetLoadedAssetCount() const;

    // Hot reloading
    void EnableHotReloading(bool enable = true);
    void RegisterChangeCallback(AssetChangeCallback callback);
    void Update(); // Call this regularly to check for file changes

private:
    AssetManager() = default;
    ~AssetManager();

    // Non-copyable and non-movable
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    std::string GetFullPath(const std::string& relative_path) const;
    AssetType DetectAssetType(const std::string& path) const;
    void CheckForChanges();

    bool initialized_ = false;
    std::string asset_root_;
    
    // Asset caches - separate maps for each type
    std::unordered_map<std::string, std::shared_ptr<Renderer::Texture>> textures_;
    std::unordered_map<std::string, std::shared_ptr<Font>> fonts_;
    std::unordered_map<std::string, std::shared_ptr<AudioClip>> audio_clips_;
    
    // Hot reloading
    bool hot_reloading_enabled_ = false;
    std::vector<AssetChangeCallback> change_callbacks_;
    std::unordered_map<std::string, std::time_t> file_timestamps_;
};

} // namespace Asset  
} // namespace PyNovaGE