#pragma once

#include <AL/al.h>
#include <memory>
#include <string>

namespace PyNovaGE {
namespace Asset {
    class AudioClip;
}

namespace Audio {

class AudioSystem;

/**
 * @brief Audio playback states
 */
enum class AudioState {
    Stopped,
    Playing,
    Paused
};

/**
 * @brief Individual audio source for playing sounds
 */
class AudioSource {
public:
    AudioSource(AudioSystem* audio_system);
    ~AudioSource();

    // Disable copy constructor and assignment
    AudioSource(const AudioSource&) = delete;
    AudioSource& operator=(const AudioSource&) = delete;

    /**
     * @brief Load audio clip to be played by this source
     * @param clip Audio clip to play
     * @return true if successful
     */
    bool LoadClip(const std::shared_ptr<Asset::AudioClip>& clip);

    /**
     * @brief Play the loaded audio clip
     * @param loop Whether to loop the audio
     */
    void Play(bool loop = false);

    /**
     * @brief Pause playback
     */
    void Pause();

    /**
     * @brief Stop playback
     */
    void Stop();

    /**
     * @brief Resume playback if paused
     */
    void Resume();

    /**
     * @brief Get current playback state
     */
    AudioState GetState() const;

    /**
     * @brief Check if currently playing
     */
    bool IsPlaying() const { return GetState() == AudioState::Playing; }

    /**
     * @brief Check if paused
     */
    bool IsPaused() const { return GetState() == AudioState::Paused; }

    /**
     * @brief Check if stopped
     */
    bool IsStopped() const { return GetState() == AudioState::Stopped; }

    /**
     * @brief Set volume (0.0 to 1.0)
     */
    void SetVolume(float volume);

    /**
     * @brief Get volume
     */
    float GetVolume() const;

    /**
     * @brief Set pitch (0.5 to 2.0, 1.0 = normal)
     */
    void SetPitch(float pitch);

    /**
     * @brief Get pitch
     */
    float GetPitch() const;

    /**
     * @brief Set whether the audio should loop
     */
    void SetLooping(bool loop);

    /**
     * @brief Check if audio is set to loop
     */
    bool IsLooping() const;

    /**
     * @brief Set 3D position for spatial audio
     */
    void SetPosition(float x, float y, float z = 0.0f);

    /**
     * @brief Set velocity for doppler effect
     */
    void SetVelocity(float x, float y, float z = 0.0f);

    /**
     * @brief Set minimum distance for 3D audio attenuation
     */
    void SetMinDistance(float distance);

    /**
     * @brief Set maximum distance for 3D audio attenuation
     */
    void SetMaxDistance(float distance);

    /**
     * @brief Set rolloff factor for distance attenuation
     */
    void SetRolloffFactor(float factor);

    /**
     * @brief Get playback position in seconds
     */
    float GetPlaybackPosition() const;

    /**
     * @brief Set playback position in seconds
     */
    void SetPlaybackPosition(float seconds);

    /**
     * @brief Get duration of loaded clip in seconds
     */
    float GetDuration() const;

    /**
     * @brief Check if a clip is loaded
     */
    bool HasClip() const { return clip_ != nullptr && buffer_id_ != AL_NONE; }

private:
    AudioSystem* audio_system_;
    ALuint source_id_ = AL_NONE;
    ALuint buffer_id_ = AL_NONE;
    std::shared_ptr<Asset::AudioClip> clip_;
    
    bool CreateSource();
    void DestroySource();
    bool CreateBuffer();
    void DestroyBuffer();
    ALenum GetALFormat(int channels, int bits_per_sample) const;
    void UpdateState();
};

} // namespace Audio
} // namespace PyNovaGE