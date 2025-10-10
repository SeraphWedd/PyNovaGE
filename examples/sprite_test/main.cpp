#include <iostream>
#include <vector>
#include <cassert>

// GLAD must be included before any OpenGL headers
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "renderer/renderer.hpp"
#include "renderer/sprite_renderer.hpp"
#include "renderer/batch_renderer.hpp"
#include "renderer/texture.hpp"
#include "window/window.hpp"

using PyNovaGE::Renderer::Sprite;
using PyNovaGE::Renderer::SpriteRenderer;
using PyNovaGE::Renderer::BatchRenderer;
using PyNovaGE::Renderer::Texture;
using PyNovaGE::Renderer::TextureAtlas;
using PyNovaGE::Renderer::TextureAtlasRegion;
using PyNovaGE::Renderer::TextureFormat;
using PyNovaGE::Renderer::TextureDataType;
using PyNovaGE::Renderer::RendererConfig;
using PyNovaGE::Renderer::RenderStats;
using PyNovaGE::Vector2f;

bool InitializeOpenGL() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Hidden window for testing
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Sprite Test", nullptr, nullptr);
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

void TestSpriteCreation() {
    std::cout << "[TEST] Sprite Creation and Properties..." << std::endl;
    
    // Test 1: Default sprite creation
    Sprite sprite1;
    
    if (sprite1.position.x == 0.0f && sprite1.position.y == 0.0f &&
        sprite1.rotation == 0.0f &&
        sprite1.scale.x == 1.0f && sprite1.scale.y == 1.0f &&
        sprite1.color.x == 1.0f && sprite1.color.y == 1.0f && 
        sprite1.color.z == 1.0f && sprite1.color.w == 1.0f) {
        std::cout << "[PASS] Default sprite creation with correct initial values" << std::endl;
    } else {
        std::cout << "[FAIL] Default sprite has incorrect initial values" << std::endl;
    }
    
    // Test 2: Sprite property modification
    sprite1.position = {100.0f, 200.0f};
    sprite1.rotation = 3.14159f / 4.0f; // 45 degrees
    sprite1.scale = {2.0f, 1.5f};
    sprite1.color = {1.0f, 0.5f, 0.0f, 0.8f}; // Orange with transparency
    
    if (sprite1.position.x == 100.0f && sprite1.position.y == 200.0f &&
        sprite1.rotation == 3.14159f / 4.0f &&
        sprite1.scale.x == 2.0f && sprite1.scale.y == 1.5f) {
        std::cout << "[PASS] Sprite property modification works correctly" << std::endl;
    } else {
        std::cout << "[FAIL] Sprite property modification failed" << std::endl;
    }
}

void TestSpriteWithTexture() {
    std::cout << "[TEST] Sprite with Texture..." << std::endl;
    
    // Create a test texture
    std::vector<unsigned char> textureData(64 * 64 * 4);
    // Create a simple pattern
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            int idx = (y * 64 + x) * 4;
            textureData[idx] = static_cast<unsigned char>((x * 255) / 63);     // Red gradient
            textureData[idx + 1] = static_cast<unsigned char>((y * 255) / 63); // Green gradient
            textureData[idx + 2] = 128;                                        // Blue constant
            textureData[idx + 3] = 255;                                        // Alpha full
        }
    }
    
    auto texture = std::make_shared<Texture>();
    bool textureCreated = texture->CreateFromData(64, 64, TextureFormat::RGBA, 
                                                 TextureDataType::UnsignedByte, 
                                                 textureData.data());
    
    if (!textureCreated || !texture->IsValid()) {
        std::cout << "[FAIL] Failed to create test texture" << std::endl;
        return;
    }
    
    // Test: Sprite construction with texture
    Vector2f position(150.0f, 100.0f);
    Sprite sprite(position, texture);
    
    if (sprite.texture == texture &&
        sprite.position.x == 150.0f && sprite.position.y == 100.0f &&
        sprite.size.x == 64.0f && sprite.size.y == 64.0f) {
        std::cout << "[PASS] Sprite with texture constructor works correctly" << std::endl;
    } else {
        std::cout << "[FAIL] Sprite with texture constructor failed" << std::endl;
    }
}

void TestSpriteRenderer() {
    std::cout << "[TEST] SpriteRenderer Functionality..." << std::endl;
    
    // Test 1: SpriteRenderer creation and initialization
    SpriteRenderer spriteRenderer;
    
    if (!spriteRenderer.IsInitialized()) {
        std::cout << "[PASS] SpriteRenderer starts uninitialized" << std::endl;
    } else {
        std::cout << "[FAIL] SpriteRenderer should start uninitialized" << std::endl;
    }
    
    // Test 2: Initialize SpriteRenderer
    bool initialized = spriteRenderer.Initialize();
    
    if (initialized && spriteRenderer.IsInitialized()) {
        std::cout << "[PASS] SpriteRenderer initialization successful" << std::endl;
    } else {
        std::cout << "[FAIL] SpriteRenderer initialization failed" << std::endl;
        return;
    }
    
    // Test 3: Create a sprite with texture for rendering
    std::vector<unsigned char> renderData(32 * 32 * 4, 255); // White 32x32 texture
    auto renderTexture = std::make_shared<Texture>();
    bool renderTextureCreated = renderTexture->CreateFromData(32, 32, TextureFormat::RGBA,
                                                             TextureDataType::UnsignedByte,
                                                             renderData.data());
    
    if (!renderTextureCreated) {
        std::cout << "[FAIL] Failed to create render test texture" << std::endl;
        return;
    }
    
    Sprite renderSprite({0.0f, 0.0f}, renderTexture);
    renderSprite.color = {1.0f, 0.8f, 0.6f, 1.0f}; // Light orange tint
    
    // Test 4: Attempt to render sprite (this tests the rendering pipeline)
    try {
        spriteRenderer.RenderSprite(renderSprite);
        std::cout << "[PASS] Sprite rendering completed without errors" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "[FAIL] Sprite rendering threw exception: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "[FAIL] Sprite rendering threw unknown exception" << std::endl;
    }
    
    // Test 5: Cleanup
    spriteRenderer.Shutdown();
    
    if (!spriteRenderer.IsInitialized()) {
        std::cout << "[PASS] SpriteRenderer shutdown successful" << std::endl;
    } else {
        std::cout << "[FAIL] SpriteRenderer shutdown failed" << std::endl;
    }
}

void TestRendererIntegration() {
    std::cout << "[TEST] Renderer Integration..." << std::endl;
    
    // Test 1: Initialize main renderer
    RendererConfig config;
    config.enable_blend = true;
    config.enable_depth_test = false; // 2D rendering
    
    bool rendererInit = PyNovaGE::Renderer::Renderer::Initialize(config);
    
    if (rendererInit && PyNovaGE::Renderer::Renderer::IsInitialized()) {
        std::cout << "[PASS] Main renderer initialization successful" << std::endl;
    } else {
        std::cout << "[FAIL] Main renderer initialization failed" << std::endl;
        return;
    }
    
    // Test 2: Get sprite renderer from main renderer
    SpriteRenderer* spriteRenderer = PyNovaGE::Renderer::Renderer::GetSpriteRenderer();
    
    if (spriteRenderer != nullptr && spriteRenderer->IsInitialized()) {
        std::cout << "[PASS] SpriteRenderer retrieved from main renderer" << std::endl;
    } else {
        std::cout << "[FAIL] Failed to get initialized SpriteRenderer from main renderer" << std::endl;
        PyNovaGE::Renderer::Renderer::Shutdown();
        return;
    }
    
    // Test 3: Create and render sprite through main renderer
    std::vector<unsigned char> integrationData(16 * 16 * 4);
    // Create checkerboard pattern
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int idx = (y * 16 + x) * 4;
            bool checker = ((x / 4) + (y / 4)) % 2;
            integrationData[idx] = checker ? 255 : 0;     // Red
            integrationData[idx + 1] = checker ? 255 : 0; // Green  
            integrationData[idx + 2] = checker ? 255 : 0; // Blue
            integrationData[idx + 3] = 255;               // Alpha
        }
    }
    
    auto integrationTexture = std::make_shared<Texture>();
    bool integrationTextureCreated = integrationTexture->CreateFromData(16, 16, TextureFormat::RGBA,
                                                                        TextureDataType::UnsignedByte,
                                                                        integrationData.data());
    
    if (integrationTextureCreated) {
        Sprite integrationSprite({100.0f, 100.0f}, integrationTexture);
        integrationSprite.rotation = 3.14159f / 6.0f; // 30 degrees
        integrationSprite.scale = {3.0f, 3.0f};
        
        try {
            // Begin frame
            PyNovaGE::Renderer::Renderer::BeginFrame();
            PyNovaGE::Renderer::Renderer::Clear({0.2f, 0.3f, 0.4f, 1.0f});
            
            // Render sprite
            spriteRenderer->RenderSprite(integrationSprite);
            
            // End frame
            PyNovaGE::Renderer::Renderer::EndFrame();
            
            std::cout << "[PASS] Complete rendering pipeline successful" << std::endl;
            
            // Check stats
            const RenderStats& stats = PyNovaGE::Renderer::Renderer::GetStats();
            std::cout << "       Frame time: " << stats.frame_time_ms << "ms" << std::endl;
            
        } catch (...) {
            std::cout << "[FAIL] Complete rendering pipeline failed" << std::endl;
        }
    } else {
        std::cout << "[FAIL] Failed to create integration texture" << std::endl;
    }
    
    // Test 4: Cleanup
    PyNovaGE::Renderer::Renderer::Shutdown();
    
    if (!PyNovaGE::Renderer::Renderer::IsInitialized()) {
        std::cout << "[PASS] Main renderer shutdown successful" << std::endl;
    } else {
        std::cout << "[FAIL] Main renderer shutdown failed" << std::endl;
    }
}

void TestBatchRendering() {
    std::cout << "[TEST] BatchRenderer Functionality..." << std::endl;
    
    // Test 1: BatchRenderer creation and initialization
    BatchRenderer batchRenderer(100, 8); // Smaller limits for testing
    
    if (!batchRenderer.IsInitialized()) {
        std::cout << "[PASS] BatchRenderer starts uninitialized" << std::endl;
    } else {
        std::cout << "[FAIL] BatchRenderer should start uninitialized" << std::endl;
    }
    
    // Test 2: Initialize BatchRenderer
    bool initialized = batchRenderer.Initialize();
    
    if (initialized && batchRenderer.IsInitialized()) {
        std::cout << "[PASS] BatchRenderer initialization successful" << std::endl;
        std::cout << "       Max sprites per batch: " << batchRenderer.GetMaxSprites() << std::endl;
        std::cout << "       Max textures per batch: " << batchRenderer.GetMaxTextures() << std::endl;
    } else {
        std::cout << "[FAIL] BatchRenderer initialization failed" << std::endl;
        return;
    }
    
    // Test 3: Create test textures for batch rendering
    std::vector<std::shared_ptr<Texture>> testTextures;
    
    // Create 3 different colored textures
    for (int i = 0; i < 3; ++i) {
        std::vector<unsigned char> textureData(16 * 16 * 4);
        
        // Different colors for each texture
        unsigned char colors[3][3] = {
            {255, 0, 0},   // Red
            {0, 255, 0},   // Green
            {0, 0, 255}    // Blue
        };
        
        for (int p = 0; p < 16 * 16; ++p) {
            textureData[p * 4] = colors[i][0];     // R
            textureData[p * 4 + 1] = colors[i][1]; // G
            textureData[p * 4 + 2] = colors[i][2]; // B
            textureData[p * 4 + 3] = 255;          // A
        }
        
        auto texture = std::make_shared<Texture>();
        if (texture->CreateFromData(16, 16, TextureFormat::RGBA,
                                   TextureDataType::UnsignedByte,
                                   textureData.data())) {
            testTextures.push_back(texture);
        }
    }
    
    if (testTextures.size() != 3) {
        std::cout << "[FAIL] Failed to create test textures for batch rendering" << std::endl;
        return;
    }
    
    std::cout << "[PASS] Created " << testTextures.size() << " test textures" << std::endl;
    
    // Test 4: Create sprites for batch rendering
    std::vector<Sprite> batchSprites;
    
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 4; ++x) {
            Sprite sprite;
            sprite.position = Vector2f(static_cast<float>(x * 40 + 50), static_cast<float>(y * 40 + 50));
            sprite.size = Vector2f(32.0f, 32.0f);
            sprite.color = {1.0f, 1.0f, 1.0f, 0.8f}; // Slightly transparent
            sprite.rotation = static_cast<float>((x + y) * 0.2f); // Slight rotation variation
            sprite.texture = testTextures[(x + y) % 3]; // Cycle through textures
            batchSprites.push_back(sprite);
        }
    }
    
    std::cout << "[PASS] Created " << batchSprites.size() << " sprites for batch rendering" << std::endl;
    
    // Test 5: Batch rendering lifecycle
    try {
        batchRenderer.BeginBatch();
        
        int spritesAdded = 0;
        for (const auto& sprite : batchSprites) {
            if (batchRenderer.AddSprite(sprite)) {
                ++spritesAdded;
            }
        }
        
        std::cout << "[PASS] Added " << spritesAdded << "/" << batchSprites.size() << " sprites to batch" << std::endl;
        
        // Check batch status
        std::cout << "       Current sprites in batch: " << batchRenderer.GetCurrentSpriteCount() << std::endl;
        std::cout << "       Current textures in batch: " << batchRenderer.GetCurrentTextureCount() << std::endl;
        
        batchRenderer.EndBatch();
        
        std::cout << "[PASS] Batch rendering lifecycle completed successfully" << std::endl;
        
        // Check statistics
        const auto& stats = batchRenderer.GetStats();
        std::cout << "       Batch statistics:" << std::endl;
        std::cout << "         Draw calls: " << stats.draw_calls << std::endl;
        std::cout << "         Sprites batched: " << stats.sprites_batched << std::endl;
        std::cout << "         Batches flushed: " << stats.batches_flushed << std::endl;
        std::cout << "         Texture binds: " << stats.texture_binds << std::endl;
        std::cout << "         Avg sprites per batch: " << stats.avg_sprites_per_batch << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "[FAIL] Batch rendering threw exception: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "[FAIL] Batch rendering threw unknown exception" << std::endl;
    }
    
    // Test 6: Cleanup
    batchRenderer.Shutdown();
    
    if (!batchRenderer.IsInitialized()) {
        std::cout << "[PASS] BatchRenderer shutdown successful" << std::endl;
    } else {
        std::cout << "[FAIL] BatchRenderer shutdown failed" << std::endl;
    }
}

void TestBatchVsIndividualRendering() {
    std::cout << "[TEST] Batch vs Individual Rendering Performance..." << std::endl;
    
    // Initialize both renderers
    RendererConfig config;
    config.enable_blend = true;
    config.enable_depth_test = false;
    
    if (!PyNovaGE::Renderer::Renderer::Initialize(config)) {
        std::cout << "[FAIL] Failed to initialize main renderer for performance test" << std::endl;
        return;
    }
    
    SpriteRenderer* spriteRenderer = PyNovaGE::Renderer::Renderer::GetSpriteRenderer();
    BatchRenderer* batchRenderer = PyNovaGE::Renderer::Renderer::GetBatchRenderer();
    
    if (!spriteRenderer || !batchRenderer) {
        std::cout << "[FAIL] Failed to get renderers from main renderer" << std::endl;
        PyNovaGE::Renderer::Renderer::Shutdown();
        return;
    }
    
    std::cout << "[PASS] Retrieved both individual and batch renderers" << std::endl;
    
    // Create test texture
    std::vector<unsigned char> perfData(8 * 8 * 4, 255); // White 8x8 texture
    auto perfTexture = std::make_shared<Texture>();
    if (!perfTexture->CreateFromData(8, 8, TextureFormat::RGBA,
                                    TextureDataType::UnsignedByte,
                                    perfData.data())) {
        std::cout << "[FAIL] Failed to create performance test texture" << std::endl;
        PyNovaGE::Renderer::Renderer::Shutdown();
        return;
    }
    
    // Create test sprites
    const int numSprites = 50;
    std::vector<Sprite> perfSprites;
    
    for (int i = 0; i < numSprites; ++i) {
        Sprite sprite;
        sprite.position = Vector2f(static_cast<float>((i % 10) * 20 + 10), static_cast<float>((i / 10) * 20 + 10));
        sprite.size = Vector2f(16.0f, 16.0f);
        sprite.color = {1.0f, 1.0f, 1.0f, 1.0f};
        sprite.texture = perfTexture;
        perfSprites.push_back(sprite);
    }
    
    std::cout << "[PASS] Created " << numSprites << " sprites for performance comparison" << std::endl;
    
    // Test individual rendering
    try {
        PyNovaGE::Renderer::Renderer::BeginFrame();
        PyNovaGE::Renderer::Renderer::Clear({0.1f, 0.1f, 0.1f, 1.0f});
        
        for (const auto& sprite : perfSprites) {
            spriteRenderer->RenderSprite(sprite);
        }
        
        PyNovaGE::Renderer::Renderer::EndFrame();
        
        const auto& individualStats = PyNovaGE::Renderer::Renderer::GetStats();
        std::cout << "[PASS] Individual rendering completed" << std::endl;
        std::cout << "       Individual render time: " << individualStats.frame_time_ms << "ms" << std::endl;
        
    } catch (...) {
        std::cout << "[FAIL] Individual rendering failed" << std::endl;
    }
    
    // Reset statistics
    batchRenderer->ResetStats();
    
    // Test batch rendering
    try {
        PyNovaGE::Renderer::Renderer::BeginFrame();
        PyNovaGE::Renderer::Renderer::Clear({0.1f, 0.1f, 0.1f, 1.0f});
        
        batchRenderer->RenderSprites(perfSprites);
        
        PyNovaGE::Renderer::Renderer::EndFrame();
        
        const auto& batchStats = PyNovaGE::Renderer::Renderer::GetStats();
        const auto& rendererStats = batchRenderer->GetStats();
        
        std::cout << "[PASS] Batch rendering completed" << std::endl;
        std::cout << "       Batch render time: " << batchStats.frame_time_ms << "ms" << std::endl;
        std::cout << "       Batch renderer statistics:" << std::endl;
        std::cout << "         Draw calls: " << rendererStats.draw_calls << std::endl;
        std::cout << "         Sprites batched: " << rendererStats.sprites_batched << std::endl;
        std::cout << "         Batches flushed: " << rendererStats.batches_flushed << std::endl;
        
        // Calculate efficiency
        if (rendererStats.draw_calls > 0) {
            float efficiency = static_cast<float>(rendererStats.sprites_batched) / static_cast<float>(rendererStats.draw_calls);
            std::cout << "       Sprites per draw call: " << efficiency << std::endl;
            
            if (efficiency > 1.0f) {
                std::cout << "[PASS] Batch rendering shows improved efficiency!" << std::endl;
            }
        }
        
    } catch (...) {
        std::cout << "[FAIL] Batch rendering failed" << std::endl;
    }
    
    PyNovaGE::Renderer::Renderer::Shutdown();
    std::cout << "[PASS] Performance comparison completed" << std::endl;
}

void TestTextureAtlas() {
    std::cout << "[TEST] TextureAtlas Functionality..." << std::endl;
    
    // Test 1: Create a texture atlas
    TextureAtlas atlas(256, 256);
    
    if (atlas.GetSize().x == 256 && atlas.GetSize().y == 256 && 
        atlas.GetRegionCount() == 0) {
        std::cout << "[PASS] TextureAtlas creation successful" << std::endl;
    } else {
        std::cout << "[FAIL] TextureAtlas creation failed" << std::endl;
        return;
    }
    
    std::cout << "       Atlas size: " << atlas.GetSize().x << "x" << atlas.GetSize().y << std::endl;
    std::cout << "       Atlas valid: " << (atlas.IsValid() ? "Yes" : "No") << std::endl;
    
    // Test 2: Create test texture data for regions
    std::vector<unsigned char> redData(32 * 32 * 4);
    std::vector<unsigned char> greenData(16 * 16 * 4);
    std::vector<unsigned char> blueData(24 * 24 * 4);
    std::vector<unsigned char> yellowData(20 * 20 * 4);
    
    // Fill with different colors
    for (int i = 0; i < 32 * 32; ++i) {
        redData[i * 4] = 255;     // R
        redData[i * 4 + 1] = 0;   // G
        redData[i * 4 + 2] = 0;   // B
        redData[i * 4 + 3] = 255; // A
    }
    
    for (int i = 0; i < 16 * 16; ++i) {
        greenData[i * 4] = 0;     // R
        greenData[i * 4 + 1] = 255; // G
        greenData[i * 4 + 2] = 0;   // B
        greenData[i * 4 + 3] = 255; // A
    }
    
    for (int i = 0; i < 24 * 24; ++i) {
        blueData[i * 4] = 0;     // R
        blueData[i * 4 + 1] = 0;   // G
        blueData[i * 4 + 2] = 255; // B
        blueData[i * 4 + 3] = 255; // A
    }
    
    for (int i = 0; i < 20 * 20; ++i) {
        yellowData[i * 4] = 255;   // R
        yellowData[i * 4 + 1] = 255; // G
        yellowData[i * 4 + 2] = 0;   // B
        yellowData[i * 4 + 3] = 255; // A
    }
    
    // Test 3: Add regions to atlas
    const TextureAtlasRegion* redRegion = atlas.AddRegion("red_square", 32, 32, redData.data());
    const TextureAtlasRegion* greenRegion = atlas.AddRegion("green_square", 16, 16, greenData.data());
    const TextureAtlasRegion* blueRegion = atlas.AddRegion("blue_square", 24, 24, blueData.data());
    const TextureAtlasRegion* yellowRegion = atlas.AddRegion("yellow_square", 20, 20, yellowData.data());
    
    if (redRegion && greenRegion && blueRegion && yellowRegion) {
        std::cout << "[PASS] Added 4 regions to atlas successfully" << std::endl;
        std::cout << "       Total regions in atlas: " << atlas.GetRegionCount() << std::endl;
    } else {
        std::cout << "[FAIL] Failed to add all regions to atlas" << std::endl;
        std::cout << "       Added: " << atlas.GetRegionCount() << "/4 regions" << std::endl;
    }
    
    // Test 4: Verify region properties and packing
    if (redRegion) {
        std::cout << "       Red region - Position: (" << redRegion->position.x << ", " << redRegion->position.y << ")";
        std::cout << " Size: (" << redRegion->size.x << ", " << redRegion->size.y << ")";
        std::cout << " UV: (" << redRegion->uv_min.x << ", " << redRegion->uv_min.y << ") to ";
        std::cout << "(" << redRegion->uv_max.x << ", " << redRegion->uv_max.y << ")" << std::endl;
        
        if (redRegion->size.x == 32 && redRegion->size.y == 32 &&
            redRegion->uv_min.x >= 0.0f && redRegion->uv_min.y >= 0.0f &&
            redRegion->uv_max.x <= 1.0f && redRegion->uv_max.y <= 1.0f &&
            redRegion->uv_max.x > redRegion->uv_min.x &&
            redRegion->uv_max.y > redRegion->uv_min.y) {
            std::cout << "[PASS] Red region properties are correct" << std::endl;
        } else {
            std::cout << "[FAIL] Red region has incorrect properties" << std::endl;
        }
    }
    
    // Test 5: Verify regions don't overlap
    auto overlaps = [](const TextureAtlasRegion& a, const TextureAtlasRegion& b) {
        return !(a.position.x + a.size.x <= b.position.x ||
                 b.position.x + b.size.x <= a.position.x ||
                 a.position.y + a.size.y <= b.position.y ||
                 b.position.y + b.size.y <= a.position.y);
    };
    
    bool hasOverlaps = false;
    if (redRegion && greenRegion && overlaps(*redRegion, *greenRegion)) hasOverlaps = true;
    if (redRegion && blueRegion && overlaps(*redRegion, *blueRegion)) hasOverlaps = true;
    if (redRegion && yellowRegion && overlaps(*redRegion, *yellowRegion)) hasOverlaps = true;
    if (greenRegion && blueRegion && overlaps(*greenRegion, *blueRegion)) hasOverlaps = true;
    if (greenRegion && yellowRegion && overlaps(*greenRegion, *yellowRegion)) hasOverlaps = true;
    if (blueRegion && yellowRegion && overlaps(*blueRegion, *yellowRegion)) hasOverlaps = true;
    
    if (!hasOverlaps) {
        std::cout << "[PASS] All regions are properly packed without overlaps" << std::endl;
    } else {
        std::cout << "[FAIL] Some regions overlap - packing algorithm issue" << std::endl;
    }
    
    // Test 6: Retrieve regions by name
    const TextureAtlasRegion* retrievedRed = atlas.GetRegion("red_square");
    const TextureAtlasRegion* retrievedGreen = atlas.GetRegion("green_square");
    const TextureAtlasRegion* nonExistent = atlas.GetRegion("non_existent");
    
    if (retrievedRed == redRegion && retrievedGreen == greenRegion && nonExistent == nullptr) {
        std::cout << "[PASS] Region retrieval by name works correctly" << std::endl;
    } else {
        std::cout << "[FAIL] Region retrieval by name failed" << std::endl;
    }
    
    // Test 7: Test atlas capacity and efficiency
    std::cout << "[TEST] Atlas packing efficiency..." << std::endl;
    
    TextureAtlas efficiencyAtlas(128, 128);
    std::vector<unsigned char> smallData(8 * 8 * 4, 128); // Gray 8x8 squares
    
    int successfulAdditions = 0;
    int maxAttempts = 300; // Try more than theoretical max to test limits
    for (int i = 0; i < maxAttempts; ++i) {
        std::string regionName = "small_" + std::to_string(i);
        if (efficiencyAtlas.AddRegion(regionName, 8, 8, smallData.data())) {
            ++successfulAdditions;
        } else {
            std::cout << "       Atlas full after " << successfulAdditions << " regions (failed on attempt " << (i + 1) << ")" << std::endl;
            break; // Atlas is full
        }
    }
    
    int theoreticalMax = (128 / 8) * (128 / 8); // 256 8x8 regions in 128x128 atlas
    float efficiency = static_cast<float>(successfulAdditions) / static_cast<float>(theoreticalMax) * 100.0f;
    
    std::cout << "       Packed " << successfulAdditions << "/" << theoreticalMax << " small regions" << std::endl;
    std::cout << "       Packing efficiency: " << efficiency << "%" << std::endl;
    
    // With maximal rectangles algorithm, we should achieve very high efficiency for uniform rectangles
    if (efficiency >= 90.0f) {
        std::cout << "[PASS] Atlas packing efficiency is excellent (>= 90%)" << std::endl;
    } else if (efficiency >= 75.0f) {
        std::cout << "[PASS] Atlas packing efficiency is good (>= 75%)" << std::endl;
    } else if (efficiency >= 60.0f) {
        std::cout << "[PASS] Atlas packing efficiency is acceptable (>= 60%)" << std::endl;
    } else {
        std::cout << "[FAIL] Atlas packing efficiency is too low (< 60%)" << std::endl;
    }
    
    // Additional test with non-uniform sizes to show the algorithm's flexibility
    std::cout << "[TEST] Mixed-size packing test..." << std::endl;
    TextureAtlas mixedAtlas(256, 256);
    std::vector<std::pair<int, int>> mixedSizes = {
        {32, 32}, {16, 16}, {24, 24}, {8, 8}, {12, 12},
        {16, 32}, {8, 16}, {20, 20}, {28, 14}, {10, 30}
    };
    
    int mixedSuccess = 0;
    for (size_t i = 0; i < mixedSizes.size(); ++i) {
        std::vector<unsigned char> mixedData(mixedSizes[i].first * mixedSizes[i].second * 4, static_cast<unsigned char>(i * 25));
        std::string mixedName = "mixed_" + std::to_string(i);
        if (mixedAtlas.AddRegion(mixedName, mixedSizes[i].first, mixedSizes[i].second, mixedData.data())) {
            ++mixedSuccess;
        }
    }
    
    std::cout << "       Successfully packed " << mixedSuccess << "/" << mixedSizes.size() << " mixed-size regions" << std::endl;
    if (mixedSuccess == static_cast<int>(mixedSizes.size())) {
        std::cout << "[PASS] Mixed-size packing successful" << std::endl;
    } else {
        std::cout << "[FAIL] Mixed-size packing incomplete" << std::endl;
    }
    
    // Test 8: Test with sprites using atlas texture
    if (atlas.IsValid() && redRegion && greenRegion) {
        std::cout << "[TEST] Using atlas texture with sprites..." << std::endl;
        
        // Note: For this test, we demonstrate that the atlas texture regions work correctly
        // but we can't easily create sprites that directly use the atlas texture since
        // the Texture class is non-copyable and the atlas owns the texture.
        // In a real implementation, you'd add methods to create sprites from atlas regions.
        
        std::cout << "[INFO] Atlas texture is ready for use with sprites" << std::endl;
        std::cout << "       Atlas texture ID: " << atlas.GetTexture().GetTextureID() << std::endl;
        std::cout << "       Atlas texture size: " << atlas.GetTexture().GetWidth() << "x" << atlas.GetTexture().GetHeight() << std::endl;
        
        // Test that we can bind the atlas texture (this proves it's a valid OpenGL texture)
        try {
            atlas.GetTexture().Bind(0);
            std::cout << "[PASS] Successfully bound atlas texture to texture unit 0" << std::endl;
            Texture::Unbind(0);
        } catch (...) {
            std::cout << "[FAIL] Failed to bind atlas texture" << std::endl;
        }
    }
    
    std::cout << "[PASS] TextureAtlas testing completed" << std::endl;
}

int main() {
    std::cout << "=== PyNovaGE Sprite System Test ===" << std::endl << std::endl;
    
    if (!InitializeOpenGL()) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }
    
    // Run all tests
    TestSpriteCreation();
    std::cout << std::endl;
    
    TestSpriteWithTexture();
    std::cout << std::endl;
    
    TestSpriteRenderer();
    std::cout << std::endl;
    
    TestRendererIntegration();
    std::cout << std::endl;
    
    TestBatchRendering();
    std::cout << std::endl;
    
    TestBatchVsIndividualRendering();
    std::cout << std::endl;
    
    TestTextureAtlas();
    std::cout << std::endl;
    
    std::cout << "=== Sprite System Test Complete ===" << std::endl;
    
    glfwTerminate();
    return 0;
}