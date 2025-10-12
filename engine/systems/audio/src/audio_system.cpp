#include "audio/audio_system.hpp"
#include "audio/audio_source.hpp"
#include <iostream>
#include <algorithm>
#include <cstring>

namespace PyNovaGE {
namespace Audio {

// Global audio system instance
std::unique_ptr<AudioSystem> g_audio_system = nullptr;

AudioSystem::AudioSystem() {
}

AudioSystem::~AudioSystem() {
    if (initialized_) {
        Shutdown();
    }
}

bool AudioSystem::Initialize() {
    if (initialized_) {
        return true;
    }

    std::cout << "Initializing Audio System..." << std::endl;

    if (!InitializeDevice()) {
        std::cerr << "Failed to initialize audio device" << std::endl;
        return false;
    }

    if (!CreateContext()) {
        std::cerr << "Failed to create audio context" << std::endl;
        Shutdown();
        return false;
    }

    SetupListener();

    initialized_ = true;
    std::cout << "Audio System initialized successfully" << std::endl;
    return true;
}

void AudioSystem::Shutdown() {
    if (!initialized_) {
        return;
    }

    std::cout << "Shutting down Audio System..." << std::endl;

    // Stop all sources before cleanup
    StopAll();
    active_sources_.clear();

    // Clean up OpenAL context
    if (context_) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context_);
        context_ = nullptr;
    }

    // Clean up OpenAL device
    if (device_) {
        alcCloseDevice(device_);
        device_ = nullptr;
    }

    initialized_ = false;
    std::cout << "Audio System shut down" << std::endl;
}

void AudioSystem::Update(float delta_time) {
    (void)delta_time; // Mark as used to avoid warning
    
    if (!initialized_) {
        return;
    }

    // Update all active sources (remove completed ones)
    active_sources_.erase(
        std::remove_if(active_sources_.begin(), active_sources_.end(),
            [](AudioSource* source) {
                if (source && source->IsStopped() && !source->IsLooping()) {
                    return false; // Keep stopped non-looping sources for now
                }
                return false; // Keep all sources for manual management
            }
        ), active_sources_.end()
    );

    // Check for OpenAL errors
    CheckALError("AudioSystem::Update");
}

std::unique_ptr<AudioSource> AudioSystem::CreateSource() {
    if (!initialized_) {
        std::cerr << "Audio system not initialized!" << std::endl;
        return nullptr;
    }

    return std::make_unique<AudioSource>(this);
}

void AudioSystem::SetMasterVolume(float volume) {
    master_volume_ = std::clamp(volume, 0.0f, 1.0f);
    
    if (initialized_) {
        alListenerf(AL_GAIN, master_volume_);
        CheckALError("SetMasterVolume");
    }
}

void AudioSystem::SetListenerPosition(float x, float y, float z) {
    if (initialized_) {
        alListener3f(AL_POSITION, x, y, z);
        CheckALError("SetListenerPosition");
    }
}

void AudioSystem::SetListenerOrientation(float forward_x, float forward_y, float forward_z,
                                        float up_x, float up_y, float up_z) {
    if (initialized_) {
        ALfloat orientation[] = { forward_x, forward_y, forward_z, up_x, up_y, up_z };
        alListenerfv(AL_ORIENTATION, orientation);
        CheckALError("SetListenerOrientation");
    }
}

void AudioSystem::SetListenerVelocity(float x, float y, float z) {
    if (initialized_) {
        alListener3f(AL_VELOCITY, x, y, z);
        CheckALError("SetListenerVelocity");
    }
}

void AudioSystem::PauseAll() {
    for (AudioSource* source : active_sources_) {
        if (source && source->IsPlaying()) {
            source->Pause();
        }
    }
}

void AudioSystem::ResumeAll() {
    for (AudioSource* source : active_sources_) {
        if (source && source->IsPaused()) {
            source->Resume();
        }
    }
}

void AudioSystem::StopAll() {
    for (AudioSource* source : active_sources_) {
        if (source) {
            source->Stop();
        }
    }
}

std::string AudioSystem::GetALErrorString(ALenum error) {
    switch (error) {
        case AL_NO_ERROR:
            return "No error";
        case AL_INVALID_NAME:
            return "Invalid name parameter";
        case AL_INVALID_ENUM:
            return "Invalid enum parameter value";
        case AL_INVALID_VALUE:
            return "Invalid value parameter value";
        case AL_INVALID_OPERATION:
            return "Invalid operation";
        case AL_OUT_OF_MEMORY:
            return "Out of memory";
        default:
            return "Unknown error";
    }
}

bool AudioSystem::CheckALError(const std::string& operation) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "OpenAL Error";
        if (!operation.empty()) {
            std::cerr << " in " << operation;
        }
        std::cerr << ": " << GetALErrorString(error) << " (" << error << ")" << std::endl;
        return false;
    }
    return true;
}

void AudioSystem::RegisterSource(AudioSource* source) {
    if (source && std::find(active_sources_.begin(), active_sources_.end(), source) == active_sources_.end()) {
        active_sources_.push_back(source);
    }
}

void AudioSystem::UnregisterSource(AudioSource* source) {
    auto it = std::find(active_sources_.begin(), active_sources_.end(), source);
    if (it != active_sources_.end()) {
        active_sources_.erase(it);
    }
}

bool AudioSystem::InitializeDevice() {
    // Open the default audio device
    device_ = alcOpenDevice(nullptr);
    if (!device_) {
        std::cerr << "Failed to open OpenAL device" << std::endl;
        return false;
    }

    // Check for OpenAL errors
    ALCenum error = alcGetError(device_);
    if (error != ALC_NO_ERROR) {
        std::cerr << "OpenAL device error: " << error << std::endl;
        alcCloseDevice(device_);
        device_ = nullptr;
        return false;
    }

    std::cout << "OpenAL device opened successfully" << std::endl;
    return true;
}

bool AudioSystem::CreateContext() {
    // Create OpenAL context
    context_ = alcCreateContext(device_, nullptr);
    if (!context_) {
        std::cerr << "Failed to create OpenAL context" << std::endl;
        return false;
    }

    // Make the context current
    if (!alcMakeContextCurrent(context_)) {
        std::cerr << "Failed to make OpenAL context current" << std::endl;
        alcDestroyContext(context_);
        context_ = nullptr;
        return false;
    }

    // Check for context errors
    ALCenum error = alcGetError(device_);
    if (error != ALC_NO_ERROR) {
        std::cerr << "OpenAL context error: " << error << std::endl;
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context_);
        context_ = nullptr;
        return false;
    }

    std::cout << "OpenAL context created successfully" << std::endl;
    return true;
}

void AudioSystem::SetupListener() {
    // Set up default listener properties
    SetMasterVolume(master_volume_);
    SetListenerPosition(0.0f, 0.0f, 0.0f);
    SetListenerOrientation(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f); // Looking down -Z, up is +Y
    SetListenerVelocity(0.0f, 0.0f, 0.0f);

    std::cout << "Audio listener configured" << std::endl;
}

// Global functions
bool InitializeAudio() {
    if (g_audio_system) {
        return true; // Already initialized
    }

    g_audio_system = std::make_unique<AudioSystem>();
    return g_audio_system->Initialize();
}

void ShutdownAudio() {
    if (g_audio_system) {
        g_audio_system->Shutdown();
        g_audio_system.reset();
    }
}

AudioSystem* GetAudioSystem() {
    return g_audio_system.get();
}

} // namespace Audio
} // namespace PyNovaGE