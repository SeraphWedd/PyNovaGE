#include <gtest/gtest.h>
#include "asset/asset_manager.hpp"
#include "asset/font.hpp"
#include "asset/audio_clip.hpp"

#include <filesystem>
#include <fstream>

using namespace PyNovaGE::Asset;

class AssetManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test asset directory
        test_dir_ = "test_assets/";
        std::filesystem::create_directories(test_dir_);
        
        // Initialize asset manager
        AssetManager::Instance().Initialize(test_dir_);
    }
    
    void TearDown() override {
        AssetManager::Instance().Shutdown();
        
        // Clean up test directory
        if (std::filesystem::exists(test_dir_)) {
            std::filesystem::remove_all(test_dir_);
        }
    }

    void CreateDummyPNG(const std::string& path) {
        // Create a minimal valid PNG file (1x1 black pixel)
        unsigned char png_data[] = {
            0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,  // PNG signature
            0x00, 0x00, 0x00, 0x0D,                          // IHDR chunk length
            0x49, 0x48, 0x44, 0x52,                          // "IHDR"
            0x00, 0x00, 0x00, 0x01,                          // Width: 1
            0x00, 0x00, 0x00, 0x01,                          // Height: 1
            0x08, 0x06, 0x00, 0x00, 0x00,                    // Bit depth: 8, Color type: RGBA, etc.
            0x1F, 0x15, 0xC4, 0x89,                          // CRC
            0x00, 0x00, 0x00, 0x0B,                          // IDAT chunk length
            0x49, 0x44, 0x41, 0x54,                          // "IDAT"
            0x78, 0x9C, 0x62, 0x00, 0x02, 0x00, 0x00, 0x05, 0x00, 0x01, 0x0D, // Compressed data
            0x0A, 0x2D, 0xB4,                                // CRC
            0x00, 0x00, 0x00, 0x00,                          // IEND chunk length
            0x49, 0x45, 0x4E, 0x44,                          // "IEND"
            0xAE, 0x42, 0x60, 0x82                           // CRC
        };
        
        std::ofstream file(test_dir_ + path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(png_data), sizeof(png_data));
    }

    void CreateDummyWAV(const std::string& path) {
        // Create a minimal valid WAV file (440Hz sine wave for 1 second)
        struct WAVHeader {
            char riff_header[4] = {'R', 'I', 'F', 'F'};
            uint32_t wav_size = 44;
            char wave_header[4] = {'W', 'A', 'V', 'E'};
            char fmt_header[4] = {'f', 'm', 't', ' '};
            uint32_t fmt_chunk_size = 16;
            uint16_t audio_format = 1;     // PCM
            uint16_t num_channels = 1;     // Mono
            uint32_t sample_rate = 44100;
            uint32_t byte_rate = 44100 * 2;
            uint16_t sample_alignment = 2;
            uint16_t bit_depth = 16;
            char data_header[4] = {'d', 'a', 't', 'a'};
            uint32_t data_bytes = 4;       // 2 samples * 2 bytes
        };
        
        WAVHeader header;
        uint16_t samples[] = {0x7FFF, 0x0000}; // Two samples
        
        std::ofstream file(test_dir_ + path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));
        file.write(reinterpret_cast<const char*>(samples), sizeof(samples));
    }

    std::string test_dir_;
};

TEST_F(AssetManagerTest, Initialization) {
    EXPECT_TRUE(AssetManager::Instance().IsInitialized());
}

// NOTE: Texture loading requires OpenGL context initialization
// This functionality is tested in the examples instead

TEST_F(AssetManagerTest, AudioLoading) {
    // Create test WAV file
    CreateDummyWAV("test.wav");
    
    // Load audio
    auto result = AssetManager::Instance().LoadAudio("test.wav");
    EXPECT_TRUE(result.success);
    EXPECT_NE(result.asset, nullptr);
    EXPECT_TRUE(result.asset->IsLoaded());
    EXPECT_EQ(result.asset->GetFormat().sample_rate, 44100);
    EXPECT_EQ(result.asset->GetFormat().channels, 1);
    EXPECT_EQ(result.asset->GetFormat().bits_per_sample, 16);
}

TEST_F(AssetManagerTest, AssetUnloading) {
    CreateDummyWAV("test.wav");
    
    // Load and verify audio asset
    auto result = AssetManager::Instance().LoadAudio("test.wav");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(AssetManager::Instance().GetLoadedAssetCount(), 1);
    
    // Unload specific asset
    AssetManager::Instance().UnloadAsset("test.wav");
    EXPECT_EQ(AssetManager::Instance().GetLoadedAssetCount(), 0);
    
    // Load and unload all
    AssetManager::Instance().LoadAudio("test.wav");
    EXPECT_EQ(AssetManager::Instance().GetLoadedAssetCount(), 1);
    
    AssetManager::Instance().UnloadAllAssets();
    EXPECT_EQ(AssetManager::Instance().GetLoadedAssetCount(), 0);
}

TEST_F(AssetManagerTest, ImageSaving) {
    // Test PNG saving
    unsigned char test_data[] = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255}; // RGB pixels
    bool result = AssetManager::Instance().SaveImagePNG("output/test_output.png", 2, 1, 3, test_data);
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(test_dir_ + "output/test_output.png"));
    
    // Test JPG saving
    result = AssetManager::Instance().SaveImageJPG("output/test_output.jpg", 2, 1, 3, test_data, 80);
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(test_dir_ + "output/test_output.jpg"));
}

TEST_F(AssetManagerTest, ErrorHandling) {
    // Try to load non-existent audio file
    auto result = AssetManager::Instance().LoadAudio("nonexistent.wav");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
    
    // Try to load unsupported audio format
    std::ofstream bad_file(test_dir_ + "bad.xyz");
    bad_file << "not a valid file";
    bad_file.close();
    
    auto audio_result = AssetManager::Instance().LoadAudio("bad.xyz");
    EXPECT_FALSE(audio_result.success);
}

TEST_F(AssetManagerTest, HotReloadingInterface) {
    // Test hot reloading enable/disable
    AssetManager::Instance().EnableHotReloading(true);
    
    bool callback_triggered = false;
    AssetManager::Instance().RegisterChangeCallback([&](const std::string&, AssetType) {
        callback_triggered = true;
    });
    
    // Update should not crash
    AssetManager::Instance().Update();
    
    AssetManager::Instance().EnableHotReloading(false);
    EXPECT_FALSE(callback_triggered); // No files actually changed
}