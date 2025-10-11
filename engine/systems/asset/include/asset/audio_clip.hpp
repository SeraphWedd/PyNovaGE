#pragma once

#include "asset_manager.hpp"
#include <memory>
#include <vector>

namespace PyNovaGE {
namespace Asset {

/**
 * @brief Audio format information
 */
struct AudioFormat {
    int sample_rate = 44100;    // Samples per second
    int channels = 2;           // Number of channels (1=mono, 2=stereo)
    int bits_per_sample = 16;   // Bits per sample (8, 16, 24, 32)
    int byte_rate = 0;          // Bytes per second
    int block_align = 0;        // Block alignment
};

/**
 * @brief Audio clip asset for sound data
 */
class AudioClip : public Asset {
public:
    AudioClip(const std::string& path);
    ~AudioClip() = default;

    /**
     * @brief Load audio from file
     * @param path Path to audio file (.wav)
     * @return true if successful
     */
    bool LoadFromFile(const std::string& path);

    /**
     * @brief Get raw audio data
     * @return Pointer to audio samples
     */
    const void* GetData() const { return data_.data(); }

    /**
     * @brief Get data size in bytes
     */
    size_t GetDataSize() const { return data_.size(); }

    /**
     * @brief Get audio format
     */
    const AudioFormat& GetFormat() const { return format_; }

    /**
     * @brief Get duration in seconds
     */
    float GetDuration() const;

    /**
     * @brief Get sample count
     */
    size_t GetSampleCount() const;

    /**
     * @brief Check if audio is loaded
     */
    bool IsLoaded() const { return loaded_; }

private:
    bool LoadWAV(const std::string& path);

    bool loaded_ = false;
    AudioFormat format_;
    std::vector<unsigned char> data_;
};

} // namespace Asset
} // namespace PyNovaGE