#pragma once

/**
 * @brief Main PyNovaGE Audio System
 * 
 * Complete C++ audio system with OpenAL backend providing:
 * - High-performance audio playback
 * - 2D/3D spatial audio
 * - Volume, pitch, and effects control
 * - Integration with asset system
 * - Zero Python overhead
 */

#include "audio_system.hpp"
#include "audio_source.hpp"

// Re-export Asset::AudioClip for convenience
#include "../../../asset/include/asset/audio_clip.hpp"

namespace PyNovaGE {
namespace Audio {

/**
 * @brief Audio System Utilities
 */
namespace Utils {

/**
 * @brief Create and play a simple sound effect
 * @param clip Audio clip to play
 * @param volume Volume (0.0 to 1.0)
 * @param pitch Pitch (0.5 to 2.0, 1.0 = normal)
 * @return Unique pointer to the audio source
 */
std::unique_ptr<AudioSource> PlaySound(const std::shared_ptr<Asset::AudioClip>& clip, 
                                      float volume = 1.0f, 
                                      float pitch = 1.0f);

/**
 * @brief Create and play a looping sound
 * @param clip Audio clip to play
 * @param volume Volume (0.0 to 1.0)
 * @return Unique pointer to the audio source
 */
std::unique_ptr<AudioSource> PlayLoopingSound(const std::shared_ptr<Asset::AudioClip>& clip, 
                                             float volume = 1.0f);

/**
 * @brief Create and play a 2D positioned sound
 * @param clip Audio clip to play
 * @param x X position
 * @param y Y position
 * @param volume Volume (0.0 to 1.0)
 * @return Unique pointer to the audio source
 */
std::unique_ptr<AudioSource> PlaySound2D(const std::shared_ptr<Asset::AudioClip>& clip,
                                         float x, float y,
                                         float volume = 1.0f);

} // namespace Utils

} // namespace Audio
} // namespace PyNovaGE