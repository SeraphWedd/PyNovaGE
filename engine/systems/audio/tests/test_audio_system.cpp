#include <gtest/gtest.h>
#include "audio/audio.hpp"
#include "asset/audio_clip.hpp"
#include "asset/asset_manager.hpp"
#include <memory>
#include <fstream>
#include <thread>
#include <chrono>
#include <filesystem>

namespace PyNovaGE {
namespace Audio {

class AudioSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize asset manager for audio clip loading
        asset_manager_ = &Asset::AssetManager::Instance();
        asset_manager_->Initialize("test_assets/");
        
        // Create test WAV data
        CreateTestWAVFile();
        
        // Initialize audio system
        initialized_ = InitializeAudio();
        if (initialized_) {
            audio_system_ = GetAudioSystem();
        }
    }
    
    void TearDown() override {
        if (initialized_) {
            ShutdownAudio();
        }
        
        if (asset_manager_) {
            asset_manager_->Shutdown();
        }
        
        // Clean up test file
        std::remove("test_assets/test_audio.wav");
    }
    
    void CreateTestWAVFile() {
        // Create test_assets directory
        std::filesystem::create_directories("test_assets");
        
        // Simple WAV file structure for testing
        struct WAVHeader {
            char riff[4] = {'R', 'I', 'F', 'F'};
            uint32_t chunk_size = 36 + 1000; // Header + data size
            char wave[4] = {'W', 'A', 'V', 'E'};
            char fmt[4] = {'f', 'm', 't', ' '};
            uint32_t fmt_size = 16;
            uint16_t audio_format = 1; // PCM
            uint16_t num_channels = 1; // Mono
            uint32_t sample_rate = 44100;
            uint32_t byte_rate = 44100 * 1 * 16 / 8;
            uint16_t block_align = 1 * 16 / 8;
            uint16_t bits_per_sample = 16;
            char data[4] = {'d', 'a', 't', 'a'};
            uint32_t data_size = 1000;
        } header;
        
        std::ofstream file("test_assets/test_audio.wav", std::ios::binary);
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));
        
        // Write simple sine wave data
        for (int i = 0; i < 500; ++i) { // 500 samples = 1000 bytes at 16-bit
            int16_t sample = static_cast<int16_t>(16000 * std::sin(2 * 3.14159 * 440 * i / 44100.0));
            file.write(reinterpret_cast<const char*>(&sample), sizeof(sample));
        }
        
        file.close();
    }
    
    bool initialized_ = false;
    AudioSystem* audio_system_ = nullptr;
    Asset::AssetManager* asset_manager_ = nullptr;
    const float EPSILON = 1e-6f;
};

// Audio System Initialization Tests
TEST_F(AudioSystemTest, Initialization) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    ASSERT_TRUE(audio_system_ != nullptr);
    EXPECT_TRUE(audio_system_->IsInitialized());
}

TEST_F(AudioSystemTest, GlobalFunctions) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    EXPECT_EQ(GetAudioSystem(), audio_system_);
    
    // Test shutdown and reinitialize
    ShutdownAudio();
    EXPECT_EQ(GetAudioSystem(), nullptr);
    
    EXPECT_TRUE(InitializeAudio());
    EXPECT_NE(GetAudioSystem(), nullptr);
    
    audio_system_ = GetAudioSystem(); // Update pointer for teardown
}

// Audio System Properties Tests
TEST_F(AudioSystemTest, MasterVolumeControl) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    // Test default volume
    EXPECT_FLOAT_EQ(audio_system_->GetMasterVolume(), 1.0f);
    
    // Test volume setting
    audio_system_->SetMasterVolume(0.5f);
    EXPECT_FLOAT_EQ(audio_system_->GetMasterVolume(), 0.5f);
    
    // Test clamping
    audio_system_->SetMasterVolume(2.0f);
    EXPECT_FLOAT_EQ(audio_system_->GetMasterVolume(), 1.0f);
    
    audio_system_->SetMasterVolume(-0.5f);
    EXPECT_FLOAT_EQ(audio_system_->GetMasterVolume(), 0.0f);
}

TEST_F(AudioSystemTest, ListenerControl) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    // These functions should not crash
    audio_system_->SetListenerPosition(10.0f, 20.0f, 30.0f);
    audio_system_->SetListenerOrientation(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    audio_system_->SetListenerVelocity(5.0f, 0.0f, 0.0f);
}

// Audio Source Creation Tests
TEST_F(AudioSystemTest, SourceCreation) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    auto source = audio_system_->CreateSource();
    ASSERT_NE(source, nullptr);
    
    EXPECT_FALSE(source->HasClip());
    EXPECT_TRUE(source->IsStopped());
    EXPECT_FALSE(source->IsPlaying());
    EXPECT_FALSE(source->IsPaused());
}

TEST_F(AudioSystemTest, MultipleSourceCreation) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    std::vector<std::unique_ptr<AudioSource>> sources;
    for (int i = 0; i < 10; ++i) {
        auto source = audio_system_->CreateSource();
        ASSERT_NE(source, nullptr) << "Failed to create source " << i;
        sources.push_back(std::move(source));
    }
    
    EXPECT_EQ(sources.size(), 10);
}

// Audio Source Property Tests
TEST_F(AudioSystemTest, SourceProperties) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    auto source = audio_system_->CreateSource();
    ASSERT_NE(source, nullptr);
    
    // Test volume
    source->SetVolume(0.5f);
    EXPECT_NEAR(source->GetVolume(), 0.5f, EPSILON);
    
    // Test pitch
    source->SetPitch(1.5f);
    EXPECT_NEAR(source->GetPitch(), 1.5f, EPSILON);
    
    // Test looping
    source->SetLooping(true);
    EXPECT_TRUE(source->IsLooping());
    
    source->SetLooping(false);
    EXPECT_FALSE(source->IsLooping());
    
    // Test 3D properties
    source->SetPosition(10.0f, 20.0f, 30.0f);
    source->SetVelocity(1.0f, 2.0f, 3.0f);
    source->SetMinDistance(5.0f);
    source->SetMaxDistance(100.0f);
    source->SetRolloffFactor(2.0f);
}

// Audio Loading Tests  
TEST_F(AudioSystemTest, AudioClipLoading) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    auto clip = std::make_shared<Asset::AudioClip>("test_assets/test_audio.wav");
    ASSERT_TRUE(clip->LoadFromFile("test_assets/test_audio.wav"));
    
    auto source = audio_system_->CreateSource();
    ASSERT_NE(source, nullptr);
    
    EXPECT_TRUE(source->LoadClip(clip));
    EXPECT_TRUE(source->HasClip());
    EXPECT_GT(source->GetDuration(), 0.0f);
}

TEST_F(AudioSystemTest, InvalidAudioClipLoading) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    auto source = audio_system_->CreateSource();
    ASSERT_NE(source, nullptr);
    
    // Test with null clip
    EXPECT_FALSE(source->LoadClip(nullptr));
    EXPECT_FALSE(source->HasClip());
    
    // Test with invalid clip
    auto invalid_clip = std::make_shared<Asset::AudioClip>("nonexistent.wav");
    EXPECT_FALSE(source->LoadClip(invalid_clip));
    EXPECT_FALSE(source->HasClip());
}

// Audio Playback Tests
TEST_F(AudioSystemTest, BasicPlayback) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    auto clip = std::make_shared<Asset::AudioClip>("test_assets/test_audio.wav");
    ASSERT_TRUE(clip->LoadFromFile("test_assets/test_audio.wav"));
    
    auto source = audio_system_->CreateSource();
    ASSERT_NE(source, nullptr);
    ASSERT_TRUE(source->LoadClip(clip));
    
    // Test play
    source->Play();
    
    // Give it a moment to start playing
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // State should be playing (might be stopped if very short clip)
    AudioState state = source->GetState();
    EXPECT_TRUE(state == AudioState::Playing || state == AudioState::Stopped);
    
    // Test pause
    source->Pause();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Test resume
    source->Resume();
    
    // Test stop
    source->Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(source->IsStopped());
}

TEST_F(AudioSystemTest, PlaybackWithoutClip) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    auto source = audio_system_->CreateSource();
    ASSERT_NE(source, nullptr);
    
    // These should not crash but should not change state
    source->Play();
    EXPECT_TRUE(source->IsStopped());
    
    source->Pause();
    EXPECT_TRUE(source->IsStopped());
    
    source->Resume();
    EXPECT_TRUE(source->IsStopped());
    
    source->Stop();
    EXPECT_TRUE(source->IsStopped());
}

// Bulk Operations Tests
TEST_F(AudioSystemTest, BulkOperations) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    auto clip = std::make_shared<Asset::AudioClip>("test_assets/test_audio.wav");
    ASSERT_TRUE(clip->LoadFromFile("test_assets/test_audio.wav"));
    
    std::vector<std::unique_ptr<AudioSource>> sources;
    for (int i = 0; i < 3; ++i) {
        auto source = audio_system_->CreateSource();
        ASSERT_NE(source, nullptr);
        ASSERT_TRUE(source->LoadClip(clip));
        source->Play();
        sources.push_back(std::move(source));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Test pause all
    audio_system_->PauseAll();
    
    // Test resume all
    audio_system_->ResumeAll();
    
    // Test stop all
    audio_system_->StopAll();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    for (const auto& source : sources) {
        EXPECT_TRUE(source->IsStopped());
    }
}

// Utility Functions Tests
TEST_F(AudioSystemTest, UtilityFunctions) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    auto clip = std::make_shared<Asset::AudioClip>("test_assets/test_audio.wav");
    ASSERT_TRUE(clip->LoadFromFile("test_assets/test_audio.wav"));
    
    // Test PlaySound
    auto source1 = Utils::PlaySound(clip, 0.8f, 1.2f);
    ASSERT_NE(source1, nullptr);
    EXPECT_TRUE(source1->HasClip());
    EXPECT_NEAR(source1->GetVolume(), 0.8f, EPSILON);
    EXPECT_NEAR(source1->GetPitch(), 1.2f, EPSILON);
    EXPECT_FALSE(source1->IsLooping());
    
    // Test PlayLoopingSound
    auto source2 = Utils::PlayLoopingSound(clip, 0.6f);
    ASSERT_NE(source2, nullptr);
    EXPECT_TRUE(source2->HasClip());
    EXPECT_NEAR(source2->GetVolume(), 0.6f, EPSILON);
    EXPECT_TRUE(source2->IsLooping());
    
    // Test PlaySound2D
    auto source3 = Utils::PlaySound2D(clip, 100.0f, 200.0f, 0.7f);
    ASSERT_NE(source3, nullptr);
    EXPECT_TRUE(source3->HasClip());
    EXPECT_NEAR(source3->GetVolume(), 0.7f, EPSILON);
    EXPECT_FALSE(source3->IsLooping());
}

// Error Handling Tests
TEST_F(AudioSystemTest, ErrorHandling) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    // Test error string function
    std::string error_str = AudioSystem::GetALErrorString(AL_NO_ERROR);
    EXPECT_EQ(error_str, "No error");
    
    error_str = AudioSystem::GetALErrorString(AL_INVALID_NAME);
    EXPECT_EQ(error_str, "Invalid name parameter");
    
    // Test error checking function (should always return true for no error)
    EXPECT_TRUE(AudioSystem::CheckALError("test operation"));
}

// System Update Tests
TEST_F(AudioSystemTest, SystemUpdate) {
    if (!initialized_) {
        GTEST_SKIP() << "OpenAL not available on this system";
    }
    
    // Update should not crash
    audio_system_->Update(0.016f); // 60 FPS delta time
    audio_system_->Update(0.033f); // 30 FPS delta time
}

} // namespace Audio
} // namespace PyNovaGE