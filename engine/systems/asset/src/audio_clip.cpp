#include "asset/audio_clip.hpp"

#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>

namespace PyNovaGE {
namespace Asset {

// WAV file header structure
#pragma pack(push, 1)
struct WAVHeader {
    char riff_header[4];        // "RIFF"
    uint32_t wav_size;          // File size - 8
    char wave_header[4];        // "WAVE"
    char fmt_header[4];         // "fmt "
    uint32_t fmt_chunk_size;    // Format chunk size
    uint16_t audio_format;      // Audio format (1 = PCM)
    uint16_t num_channels;      // Number of channels
    uint32_t sample_rate;       // Sample rate
    uint32_t byte_rate;         // Bytes per second
    uint16_t sample_alignment;  // Block alignment
    uint16_t bit_depth;         // Bits per sample
    char data_header[4];        // "data"
    uint32_t data_bytes;        // Data size
};
#pragma pack(pop)

AudioClip::AudioClip(const std::string& path) 
    : Asset(path, AssetType::Audio) {
}

bool AudioClip::LoadFromFile(const std::string& path) {
    if (loaded_) {
        return true;
    }

    // Determine file type by extension
    std::string ext = path.substr(path.find_last_of('.'));
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".wav") {
        return LoadWAV(path);
    }
    // Could add OGG support here with libvorbis
    
    std::cerr << "Unsupported audio format: " << ext << std::endl;
    return false;
}

float AudioClip::GetDuration() const {
    if (!loaded_ || format_.byte_rate == 0) {
        return 0.0f;
    }
    
    return static_cast<float>(data_.size()) / static_cast<float>(format_.byte_rate);
}

size_t AudioClip::GetSampleCount() const {
    if (!loaded_) {
        return 0;
    }
    
    int bytes_per_sample = format_.bits_per_sample / 8;
    return data_.size() / (bytes_per_sample * format_.channels);
}

bool AudioClip::LoadWAV(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open WAV file: " << path << std::endl;
        return false;
    }

    // Read header
    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    
    if (file.gcount() != sizeof(header)) {
        std::cerr << "Failed to read WAV header: " << path << std::endl;
        return false;
    }

    // Validate header
    if (std::strncmp(header.riff_header, "RIFF", 4) != 0 ||
        std::strncmp(header.wave_header, "WAVE", 4) != 0 ||
        std::strncmp(header.fmt_header, "fmt ", 4) != 0 ||
        std::strncmp(header.data_header, "data", 4) != 0) {
        std::cerr << "Invalid WAV file format: " << path << std::endl;
        return false;
    }

    // Only support PCM format
    if (header.audio_format != 1) {
        std::cerr << "Unsupported WAV audio format (only PCM supported): " << path << std::endl;
        return false;
    }

    // Fill format structure
    format_.sample_rate = header.sample_rate;
    format_.channels = header.num_channels;
    format_.bits_per_sample = header.bit_depth;
    format_.byte_rate = header.byte_rate;
    format_.block_align = header.sample_alignment;

    // Read audio data
    data_.resize(header.data_bytes);
    file.read(reinterpret_cast<char*>(data_.data()), header.data_bytes);
    
    if (file.gcount() != header.data_bytes) {
        std::cerr << "Failed to read audio data: " << path << std::endl;
        data_.clear();
        return false;
    }

    file.close();
    loaded_ = true;
    
    std::cout << "Audio loaded: " << path 
              << " (" << format_.sample_rate << "Hz, " 
              << format_.channels << " channels, " 
              << format_.bits_per_sample << "-bit)" << std::endl;
    
    return true;
}

} // namespace Asset
} // namespace PyNovaGE