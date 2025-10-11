#include "audio/audio_source.hpp"
#include "audio/audio_system.hpp"
#include "asset/audio_clip.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace PyNovaGE {
namespace Audio {

AudioSource::AudioSource(AudioSystem* audio_system) 
    : audio_system_(audio_system) {
    if (audio_system_) {
        CreateSource();
        audio_system_->RegisterSource(this);
    }
}

AudioSource::~AudioSource() {
    if (audio_system_) {
        audio_system_->UnregisterSource(this);
    }
    
    DestroyBuffer();
    DestroySource();
}

bool AudioSource::LoadClip(const std::shared_ptr<Asset::AudioClip>& clip) {
    if (!clip || !clip->IsLoaded()) {
        std::cerr << "Invalid or unloaded audio clip" << std::endl;
        return false;
    }

    if (!audio_system_ || !audio_system_->IsInitialized()) {
        std::cerr << "Audio system not initialized" << std::endl;
        return false;
    }

    // Stop current playback if any
    Stop();

    // Clean up previous buffer
    DestroyBuffer();

    // Store the clip
    clip_ = clip;

    // Create new OpenAL buffer
    if (!CreateBuffer()) {
        clip_.reset();
        return false;
    }

    return true;
}

void AudioSource::Play(bool loop) {
    if (source_id_ == AL_NONE || buffer_id_ == AL_NONE) {
        std::cerr << "No audio clip loaded" << std::endl;
        return;
    }

    // Set looping
    SetLooping(loop);

    // Start playback
    alSourcePlay(source_id_);
    AudioSystem::CheckALError("AudioSource::Play");
}

void AudioSource::Pause() {
    if (source_id_ != AL_NONE) {
        alSourcePause(source_id_);
        AudioSystem::CheckALError("AudioSource::Pause");
    }
}

void AudioSource::Stop() {
    if (source_id_ != AL_NONE) {
        alSourceStop(source_id_);
        AudioSystem::CheckALError("AudioSource::Stop");
    }
}

void AudioSource::Resume() {
    if (source_id_ != AL_NONE && IsPaused()) {
        alSourcePlay(source_id_);
        AudioSystem::CheckALError("AudioSource::Resume");
    }
}

AudioState AudioSource::GetState() const {
    if (source_id_ == AL_NONE) {
        return AudioState::Stopped;
    }

    ALint state;
    alGetSourcei(source_id_, AL_SOURCE_STATE, &state);

    switch (state) {
        case AL_PLAYING:
            return AudioState::Playing;
        case AL_PAUSED:
            return AudioState::Paused;
        case AL_STOPPED:
        case AL_INITIAL:
        default:
            return AudioState::Stopped;
    }
}

void AudioSource::SetVolume(float volume) {
    if (source_id_ != AL_NONE) {
        volume = std::clamp(volume, 0.0f, 1.0f);
        alSourcef(source_id_, AL_GAIN, volume);
        AudioSystem::CheckALError("AudioSource::SetVolume");
    }
}

float AudioSource::GetVolume() const {
    if (source_id_ == AL_NONE) {
        return 0.0f;
    }

    ALfloat volume;
    alGetSourcef(source_id_, AL_GAIN, &volume);
    return volume;
}

void AudioSource::SetPitch(float pitch) {
    if (source_id_ != AL_NONE) {
        pitch = std::clamp(pitch, 0.5f, 2.0f);
        alSourcef(source_id_, AL_PITCH, pitch);
        AudioSystem::CheckALError("AudioSource::SetPitch");
    }
}

float AudioSource::GetPitch() const {
    if (source_id_ == AL_NONE) {
        return 1.0f;
    }

    ALfloat pitch;
    alGetSourcef(source_id_, AL_PITCH, &pitch);
    return pitch;
}

void AudioSource::SetLooping(bool loop) {
    if (source_id_ != AL_NONE) {
        alSourcei(source_id_, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        AudioSystem::CheckALError("AudioSource::SetLooping");
    }
}

bool AudioSource::IsLooping() const {
    if (source_id_ == AL_NONE) {
        return false;
    }

    ALint looping;
    alGetSourcei(source_id_, AL_LOOPING, &looping);
    return looping == AL_TRUE;
}

void AudioSource::SetPosition(float x, float y, float z) {
    if (source_id_ != AL_NONE) {
        alSource3f(source_id_, AL_POSITION, x, y, z);
        AudioSystem::CheckALError("AudioSource::SetPosition");
    }
}

void AudioSource::SetVelocity(float x, float y, float z) {
    if (source_id_ != AL_NONE) {
        alSource3f(source_id_, AL_VELOCITY, x, y, z);
        AudioSystem::CheckALError("AudioSource::SetVelocity");
    }
}

void AudioSource::SetMinDistance(float distance) {
    if (source_id_ != AL_NONE) {
        distance = std::max(distance, 0.0f);
        alSourcef(source_id_, AL_REFERENCE_DISTANCE, distance);
        AudioSystem::CheckALError("AudioSource::SetMinDistance");
    }
}

void AudioSource::SetMaxDistance(float distance) {
    if (source_id_ != AL_NONE) {
        distance = std::max(distance, 0.0f);
        alSourcef(source_id_, AL_MAX_DISTANCE, distance);
        AudioSystem::CheckALError("AudioSource::SetMaxDistance");
    }
}

void AudioSource::SetRolloffFactor(float factor) {
    if (source_id_ != AL_NONE) {
        factor = std::max(factor, 0.0f);
        alSourcef(source_id_, AL_ROLLOFF_FACTOR, factor);
        AudioSystem::CheckALError("AudioSource::SetRolloffFactor");
    }
}

float AudioSource::GetPlaybackPosition() const {
    if (source_id_ == AL_NONE) {
        return 0.0f;
    }

    ALfloat position;
    alGetSourcef(source_id_, AL_SEC_OFFSET, &position);
    return position;
}

void AudioSource::SetPlaybackPosition(float seconds) {
    if (source_id_ != AL_NONE) {
        seconds = std::max(seconds, 0.0f);
        alSourcef(source_id_, AL_SEC_OFFSET, seconds);
        AudioSystem::CheckALError("AudioSource::SetPlaybackPosition");
    }
}

float AudioSource::GetDuration() const {
    if (clip_) {
        return clip_->GetDuration();
    }
    return 0.0f;
}

bool AudioSource::CreateSource() {
    if (source_id_ != AL_NONE) {
        return true; // Already created
    }

    alGenSources(1, &source_id_);
    if (!AudioSystem::CheckALError("AudioSource::CreateSource - alGenSources")) {
        source_id_ = AL_NONE;
        return false;
    }

    if (source_id_ == AL_NONE) {
        std::cerr << "Failed to generate OpenAL source" << std::endl;
        return false;
    }

    // Set default properties
    alSourcef(source_id_, AL_PITCH, 1.0f);
    alSourcef(source_id_, AL_GAIN, 1.0f);
    alSource3f(source_id_, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source_id_, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source_id_, AL_LOOPING, AL_FALSE);

    // Set 3D audio properties
    alSourcef(source_id_, AL_REFERENCE_DISTANCE, 1.0f);
    alSourcef(source_id_, AL_MAX_DISTANCE, 1000.0f);
    alSourcef(source_id_, AL_ROLLOFF_FACTOR, 1.0f);

    return AudioSystem::CheckALError("AudioSource::CreateSource - setup");
}

void AudioSource::DestroySource() {
    if (source_id_ != AL_NONE) {
        alDeleteSources(1, &source_id_);
        AudioSystem::CheckALError("AudioSource::DestroySource");
        source_id_ = AL_NONE;
    }
}

bool AudioSource::CreateBuffer() {
    if (!clip_ || !clip_->IsLoaded()) {
        return false;
    }

    if (buffer_id_ != AL_NONE) {
        DestroyBuffer();
    }

    // Generate OpenAL buffer
    alGenBuffers(1, &buffer_id_);
    if (!AudioSystem::CheckALError("AudioSource::CreateBuffer - alGenBuffers")) {
        buffer_id_ = AL_NONE;
        return false;
    }

    if (buffer_id_ == AL_NONE) {
        std::cerr << "Failed to generate OpenAL buffer" << std::endl;
        return false;
    }

    // Get audio format
    const auto& format = clip_->GetFormat();
    ALenum al_format = GetALFormat(format.channels, format.bits_per_sample);
    if (al_format == AL_NONE) {
        std::cerr << "Unsupported audio format: " << format.channels 
                  << " channels, " << format.bits_per_sample << " bits" << std::endl;
        DestroyBuffer();
        return false;
    }

    // Upload audio data to buffer
    alBufferData(buffer_id_, al_format, clip_->GetData(), 
                 static_cast<ALsizei>(clip_->GetDataSize()), 
                 format.sample_rate);

    if (!AudioSystem::CheckALError("AudioSource::CreateBuffer - alBufferData")) {
        DestroyBuffer();
        return false;
    }

    // Attach buffer to source
    alSourcei(source_id_, AL_BUFFER, buffer_id_);
    if (!AudioSystem::CheckALError("AudioSource::CreateBuffer - attach buffer")) {
        DestroyBuffer();
        return false;
    }

    return true;
}

void AudioSource::DestroyBuffer() {
    if (buffer_id_ != AL_NONE) {
        // Detach buffer from source first
        if (source_id_ != AL_NONE) {
            alSourcei(source_id_, AL_BUFFER, AL_NONE);
            AudioSystem::CheckALError("AudioSource::DestroyBuffer - detach");
        }

        alDeleteBuffers(1, &buffer_id_);
        AudioSystem::CheckALError("AudioSource::DestroyBuffer");
        buffer_id_ = AL_NONE;
    }
}

ALenum AudioSource::GetALFormat(int channels, int bits_per_sample) const {
    if (channels == 1) {
        if (bits_per_sample == 8) {
            return AL_FORMAT_MONO8;
        } else if (bits_per_sample == 16) {
            return AL_FORMAT_MONO16;
        }
    } else if (channels == 2) {
        if (bits_per_sample == 8) {
            return AL_FORMAT_STEREO8;
        } else if (bits_per_sample == 16) {
            return AL_FORMAT_STEREO16;
        }
    }

    return AL_NONE; // Unsupported format
}

void AudioSource::UpdateState() {
    // This could be used for automatic state management
    // Currently not needed as we query state on demand
}

} // namespace Audio
} // namespace PyNovaGE