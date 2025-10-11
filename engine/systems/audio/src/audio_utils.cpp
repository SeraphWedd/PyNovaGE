#include "audio/audio.hpp"
#include "audio/audio_system.hpp"
#include "audio/audio_source.hpp"

namespace PyNovaGE {
namespace Audio {
namespace Utils {

std::unique_ptr<AudioSource> PlaySound(const std::shared_ptr<Asset::AudioClip>& clip, 
                                      float volume, float pitch) {
    AudioSystem* audio_system = GetAudioSystem();
    if (!audio_system) {
        return nullptr;
    }

    auto source = audio_system->CreateSource();
    if (!source) {
        return nullptr;
    }

    if (!source->LoadClip(clip)) {
        return nullptr;
    }

    source->SetVolume(volume);
    source->SetPitch(pitch);
    source->Play(false); // Don't loop

    return source;
}

std::unique_ptr<AudioSource> PlayLoopingSound(const std::shared_ptr<Asset::AudioClip>& clip, 
                                             float volume) {
    AudioSystem* audio_system = GetAudioSystem();
    if (!audio_system) {
        return nullptr;
    }

    auto source = audio_system->CreateSource();
    if (!source) {
        return nullptr;
    }

    if (!source->LoadClip(clip)) {
        return nullptr;
    }

    source->SetVolume(volume);
    source->Play(true); // Loop

    return source;
}

std::unique_ptr<AudioSource> PlaySound2D(const std::shared_ptr<Asset::AudioClip>& clip,
                                         float x, float y, float volume) {
    AudioSystem* audio_system = GetAudioSystem();
    if (!audio_system) {
        return nullptr;
    }

    auto source = audio_system->CreateSource();
    if (!source) {
        return nullptr;
    }

    if (!source->LoadClip(clip)) {
        return nullptr;
    }

    source->SetVolume(volume);
    source->SetPosition(x, y, 0.0f); // 2D positioning with Z=0
    
    // Set reasonable 2D audio properties
    source->SetMinDistance(50.0f);   // Start attenuating at 50 units
    source->SetMaxDistance(500.0f);  // Stop attenuating at 500 units
    source->SetRolloffFactor(1.0f);  // Linear rolloff
    
    source->Play(false); // Don't loop

    return source;
}

} // namespace Utils
} // namespace Audio
} // namespace PyNovaGE