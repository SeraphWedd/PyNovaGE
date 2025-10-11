#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

namespace PyNovaGE {
namespace Audio {

// Forward declarations
class AudioSource;
class AudioClip;

/**
 * @brief Main audio system managing OpenAL context and global audio state
 */
class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();

    // Disable copy constructor and assignment
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    /**
     * @brief Initialize the audio system
     * @return true if successful
     */
    bool Initialize();

    /**
     * @brief Shutdown the audio system
     */
    void Shutdown();

    /**
     * @brief Check if audio system is initialized
     */
    bool IsInitialized() const { return initialized_; }

    /**
     * @brief Update audio system (call once per frame)
     */
    void Update(float delta_time);

    /**
     * @brief Create an audio source for playing sounds
     * @return Unique pointer to audio source
     */
    std::unique_ptr<AudioSource> CreateSource();

    /**
     * @brief Set master volume (0.0 to 1.0)
     */
    void SetMasterVolume(float volume);

    /**
     * @brief Get master volume
     */
    float GetMasterVolume() const { return master_volume_; }

    /**
     * @brief Set listener position for 3D audio
     */
    void SetListenerPosition(float x, float y, float z = 0.0f);

    /**
     * @brief Set listener orientation for 3D audio
     */
    void SetListenerOrientation(float forward_x, float forward_y, float forward_z = -1.0f,
                               float up_x = 0.0f, float up_y = 1.0f, float up_z = 0.0f);

    /**
     * @brief Set listener velocity for doppler effect
     */
    void SetListenerVelocity(float x, float y, float z = 0.0f);

    /**
     * @brief Pause all audio sources
     */
    void PauseAll();

    /**
     * @brief Resume all audio sources
     */
    void ResumeAll();

    /**
     * @brief Stop all audio sources
     */
    void StopAll();

    /**
     * @brief Get OpenAL error string
     */
    static std::string GetALErrorString(ALenum error);

    /**
     * @brief Check for OpenAL errors
     */
    static bool CheckALError(const std::string& operation = "");

private:
    bool initialized_ = false;
    ALCdevice* device_ = nullptr;
    ALCcontext* context_ = nullptr;
    float master_volume_ = 1.0f;
    
    // Keep track of created sources for bulk operations
    std::vector<AudioSource*> active_sources_;
    
    friend class AudioSource; // Allow AudioSource to register/unregister itself
    void RegisterSource(AudioSource* source);
    void UnregisterSource(AudioSource* source);

    bool InitializeDevice();
    bool CreateContext();
    void SetupListener();
};

/**
 * @brief Global audio system instance
 */
extern std::unique_ptr<AudioSystem> g_audio_system;

/**
 * @brief Initialize global audio system
 */
bool InitializeAudio();

/**
 * @brief Shutdown global audio system
 */
void ShutdownAudio();

/**
 * @brief Get global audio system instance
 */
AudioSystem* GetAudioSystem();

} // namespace Audio
} // namespace PyNovaGE