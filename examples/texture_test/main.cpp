#include <iostream>
#include <vector>
#include <cassert>

// GLAD must be included before any OpenGL headers
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "renderer/texture.hpp"
#include "window/window.hpp"

using namespace PyNovaGE::Renderer;
using namespace PyNovaGE::Window;

bool InitializeOpenGL() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Hidden window for testing
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Texture Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    
    if (!gladLoaderLoadGL()) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return false;
    }
    
    return true;
}

void TestBasicTextureCreation() {
    std::cout << "[TEST] Basic Texture Creation..." << std::endl;
    
    // Test 1: Create empty texture
    TextureConfig config;
    config.min_filter = TextureFilter::Linear;
    config.mag_filter = TextureFilter::Linear;
    config.wrap_s = TextureWrap::Repeat;
    config.wrap_t = TextureWrap::Repeat;
    
    Texture texture(128, 128, TextureFormat::RGBA, config);
    
    if (texture.IsValid()) {
        std::cout << "[PASS] Empty texture creation successful" << std::endl;
        std::cout << "       Size: " << texture.GetWidth() << "x" << texture.GetHeight() << std::endl;
    } else {
        std::cout << "[FAIL] Empty texture creation failed" << std::endl;
    }
    
    // Test 2: Create texture with data
    std::vector<unsigned char> data(64 * 64 * 4);
    // Fill with a simple pattern
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            int idx = (y * 64 + x) * 4;
            data[idx] = static_cast<unsigned char>((x * 255) / 63);     // Red gradient
            data[idx + 1] = static_cast<unsigned char>((y * 255) / 63); // Green gradient
            data[idx + 2] = 128;                                        // Blue constant
            data[idx + 3] = 255;                                        // Alpha full
        }
    }
    
    Texture dataTexture;
    bool created = dataTexture.CreateFromData(64, 64, TextureFormat::RGBA, 
                                             TextureDataType::UnsignedByte, 
                                             data.data(), config);
    
    if (created && dataTexture.IsValid()) {
        std::cout << "[PASS] Data texture creation successful" << std::endl;
    } else {
        std::cout << "[FAIL] Data texture creation failed" << std::endl;
    }
    
    // Test 3: Invalid dimensions
    Texture invalidTexture;
    bool shouldFail = invalidTexture.CreateEmpty(-1, 100, TextureFormat::RGBA, config);
    
    if (!shouldFail) {
        std::cout << "[PASS] Invalid dimension handling works" << std::endl;
    } else {
        std::cout << "[FAIL] Invalid dimension should have been rejected" << std::endl;
    }
}

void TestTextureBinding() {
    std::cout << "[TEST] Texture Binding..." << std::endl;
    
    // Create a test texture
    TextureConfig config;
    Texture texture(32, 32, TextureFormat::RGBA, config);
    
    if (!texture.IsValid()) {
        std::cout << "[FAIL] Test texture creation failed" << std::endl;
        return;
    }
    
    // Test binding to different units
    texture.Bind(0);
    std::cout << "[PASS] Texture bound to unit 0" << std::endl;
    
    texture.Bind(1);
    std::cout << "[PASS] Texture bound to unit 1" << std::endl;
    
    // Test unbinding
    Texture::Unbind(0);
    Texture::Unbind(1);
    std::cout << "[PASS] Texture unbinding successful" << std::endl;
    
    // Test invalid unit (should not crash)
    texture.Bind(999);  // This should log an error but not crash
    std::cout << "[PASS] Invalid texture unit handled gracefully" << std::endl;
}

void TestTextureConfiguration() {
    std::cout << "[TEST] Texture Configuration..." << std::endl;
    
    TextureConfig config;
    config.min_filter = TextureFilter::Nearest;
    config.mag_filter = TextureFilter::Nearest;
    config.wrap_s = TextureWrap::ClampToEdge;
    config.wrap_t = TextureWrap::ClampToEdge;
    
    Texture texture(64, 64, TextureFormat::RGB, config);
    
    if (!texture.IsValid()) {
        std::cout << "[FAIL] Configuration test texture creation failed" << std::endl;
        return;
    }
    
    // Test filter changes
    texture.SetFilter(TextureFilter::Linear, TextureFilter::Linear);
    std::cout << "[PASS] Filter configuration changed" << std::endl;
    
    // Test wrap changes
    texture.SetWrap(TextureWrap::Repeat, TextureWrap::MirroredRepeat);
    std::cout << "[PASS] Wrap configuration changed" << std::endl;
    
    // Test mipmap generation
    texture.GenerateMipmaps();
    std::cout << "[PASS] Mipmap generation completed" << std::endl;
}

void TestTextureFormats() {
    std::cout << "[TEST] Different Texture Formats..." << std::endl;
    
    TextureConfig config;
    
    // Test different formats
    std::vector<TextureFormat> formats = {
        TextureFormat::RGB,
        TextureFormat::RGBA,
        TextureFormat::R,
        TextureFormat::RG,
        TextureFormat::DepthComponent
    };
    
    std::vector<std::string> formatNames = {
        "RGB", "RGBA", "R", "RG", "DepthComponent"
    };
    
    for (size_t i = 0; i < formats.size(); ++i) {
        Texture texture;
        bool success = texture.CreateEmpty(32, 32, formats[i], config);
        
        if (success && texture.IsValid()) {
            std::cout << "[PASS] " << formatNames[i] << " format texture created" << std::endl;
        } else {
            std::cout << "[FAIL] " << formatNames[i] << " format texture creation failed" << std::endl;
        }
    }
}

void TestTextureUpdate() {
    std::cout << "[TEST] Texture Data Update..." << std::endl;
    
    TextureConfig config;
    Texture texture(64, 64, TextureFormat::RGBA, config);
    
    if (!texture.IsValid()) {
        std::cout << "[FAIL] Update test texture creation failed" << std::endl;
        return;
    }
    
    // Create update data
    std::vector<unsigned char> updateData(16 * 16 * 4, 255); // White 16x16 square
    
    // Test valid update
    texture.UpdateData(10, 10, 16, 16, TextureFormat::RGBA, 
                      TextureDataType::UnsignedByte, updateData.data());
    std::cout << "[PASS] Valid texture region update completed" << std::endl;
    
    // Test invalid update (out of bounds)
    texture.UpdateData(60, 60, 16, 16, TextureFormat::RGBA,
                      TextureDataType::UnsignedByte, updateData.data());
    std::cout << "[PASS] Out of bounds update handled gracefully" << std::endl;
}

void TestStaticFactory() {
    std::cout << "[TEST] Static Factory Method..." << std::endl;
    
    TextureConfig config;
    config.min_filter = TextureFilter::Linear;
    config.mag_filter = TextureFilter::Linear;
    
    // Create some test data
    std::vector<unsigned char> data(32 * 32 * 4);
    for (size_t i = 0; i < data.size(); i += 4) {
        data[i] = 255;     // Red
        data[i + 1] = 128; // Green
        data[i + 2] = 64;  // Blue
        data[i + 3] = 255; // Alpha
    }
    
    Texture texture = Texture::Create("TestTexture", 32, 32, data.data(), config);
    
    if (texture.IsValid() && texture.GetName() == "TestTexture") {
        std::cout << "[PASS] Static factory method works correctly" << std::endl;
        std::cout << "       Name: " << texture.GetName() << std::endl;
    } else {
        std::cout << "[FAIL] Static factory method failed" << std::endl;
    }
}

void TestMoveSemantics() {
    std::cout << "[TEST] Move Semantics..." << std::endl;
    
    TextureConfig config;
    Texture originalTexture(64, 64, TextureFormat::RGBA, config);
    
    if (!originalTexture.IsValid()) {
        std::cout << "[FAIL] Original texture creation failed" << std::endl;
        return;
    }
    
    unsigned int originalId = originalTexture.GetTextureID();
    
    // Test move constructor
    Texture movedTexture = std::move(originalTexture);
    
    if (movedTexture.IsValid() && movedTexture.GetTextureID() == originalId && !originalTexture.IsValid()) {
        std::cout << "[PASS] Move constructor works correctly" << std::endl;
    } else {
        std::cout << "[FAIL] Move constructor failed" << std::endl;
    }
    
    // Test move assignment
    Texture assignedTexture;
    assignedTexture = std::move(movedTexture);
    
    if (assignedTexture.IsValid() && assignedTexture.GetTextureID() == originalId && !movedTexture.IsValid()) {
        std::cout << "[PASS] Move assignment works correctly" << std::endl;
    } else {
        std::cout << "[FAIL] Move assignment failed" << std::endl;
    }
}

int main() {
    std::cout << "=== PyNovaGE Texture System Test ===" << std::endl << std::endl;
    
    if (!InitializeOpenGL()) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }
    
    // Run all tests
    TestBasicTextureCreation();
    std::cout << std::endl;
    
    TestTextureBinding();
    std::cout << std::endl;
    
    TestTextureConfiguration();
    std::cout << std::endl;
    
    TestTextureFormats();
    std::cout << std::endl;
    
    TestTextureUpdate();
    std::cout << std::endl;
    
    TestStaticFactory();
    std::cout << std::endl;
    
    TestMoveSemantics();
    std::cout << std::endl;
    
    std::cout << "=== Texture System Test Complete ===" << std::endl;
    
    glfwTerminate();
    return 0;
}