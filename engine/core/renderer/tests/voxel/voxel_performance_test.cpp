#include <gtest/gtest.h>
#include <chrono>
#include "renderer/voxel/voxel_types.hpp"
#include "renderer/voxel/chunk.hpp"
#include "renderer/voxel/meshing.hpp"
#include "renderer/voxel/frustum_culler.hpp"
#include "renderer/voxel/voxel_renderer.hpp"

using namespace PyNovaGE::Renderer::Voxel;

class VoxelPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test chunk with realistic data
        test_chunk_ = std::make_unique<Chunk>();
        GenerateRealisticChunkData(*test_chunk_);
    }

    void TearDown() override {
        test_chunk_.reset();
    }

    void GenerateRealisticChunkData(Chunk& chunk) {
        // Generate a chunk that looks like real terrain
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                // Simple height map
                int height = 4 + (x + z) / 4; // Vary height from 4 to 8
                height = std::min(height, CHUNK_SIZE - 1);
                
                for (int y = 0; y <= height; ++y) {
                    VoxelType voxel_type = VoxelType::STONE;
                    
                    if (y == height) {
                        voxel_type = VoxelType::GRASS;
                    } else if (y >= height - 2) {
                        voxel_type = VoxelType::DIRT;
                    }
                    
                    chunk.SetVoxel(x, y, z, voxel_type);
                }
            }
        }
        
        // Add some structures
        for (int i = 0; i < 5; ++i) {
            int x = (i * 3) % CHUNK_SIZE;
            int z = (i * 5) % CHUNK_SIZE;
            int height = 9 + i;
            
            if (height < CHUNK_SIZE) {
                chunk.SetVoxel(x, height, z, VoxelType::WOOD);
            }
        }
    }

protected:
    std::unique_ptr<Chunk> test_chunk_;
};

// Test meshing performance with realistic data
TEST_F(VoxelPerformanceTest, MeshingPerformance) {
    GreedyMesher mesher;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Generate mesh multiple times to get average
    constexpr int iterations = 100;
    size_t total_vertices = 0;
    size_t total_quads = 0;
    
    for (int i = 0; i < iterations; ++i) {
        auto mesh_data = mesher.GenerateMesh(*test_chunk_);
        total_vertices += mesh_data.vertices.size();
        total_quads += mesh_data.quad_count;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double avg_time_ms = duration.count() / 1000.0 / iterations;
    double avg_vertices = static_cast<double>(total_vertices) / iterations;
    double avg_quads = static_cast<double>(total_quads) / iterations;
    
    // Performance expectations
    EXPECT_LT(avg_time_ms, 10.0); // Should take less than 10ms on average
    EXPECT_GT(avg_vertices, 0);   // Should generate some vertices
    EXPECT_GT(avg_quads, 0);      // Should generate some quads
    
    // Output performance info
    std::cout << "Average meshing time: " << avg_time_ms << " ms" << std::endl;
    std::cout << "Average vertices: " << avg_vertices << std::endl;
    std::cout << "Average quads: " << avg_quads << std::endl;
}

// Test culling performance with many chunks
TEST_F(VoxelPerformanceTest, CullingPerformance) {
    FrustumCuller culler;
    Camera camera;
    
    camera.SetPerspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    camera.SetPosition(Vector3f(0.0f, 0.0f, 0.0f));
    culler.UpdateCamera(camera);
    
    // Create many chunks in a grid
    std::vector<ChunkCullInfo> chunks;
    constexpr int grid_size = 20; // 20x20 = 400 chunks
    
    for (int x = -grid_size/2; x < grid_size/2; ++x) {
        for (int z = -grid_size/2; z < grid_size/2; ++z) {
            Vector3f chunk_pos(static_cast<float>(x * CHUNK_SIZE), 0.0f, static_cast<float>(z * CHUNK_SIZE));
            chunks.emplace_back(test_chunk_.get(), chunk_pos);
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Perform culling multiple times
    constexpr int iterations = 100;
    size_t total_visible = 0;
    
    for (int i = 0; i < iterations; ++i) {
        auto cull_result = culler.CullChunks(chunks);
        total_visible += cull_result.visible_chunks;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double avg_time_ms = duration.count() / 1000.0 / iterations;
    double avg_visible = static_cast<double>(total_visible) / iterations;
    double culling_ratio = 1.0 - (avg_visible / chunks.size());
    
    // Performance expectations
    EXPECT_LT(avg_time_ms, 5.0); // Should take less than 5ms for 400 chunks
    EXPECT_GT(culling_ratio, 0.5); // Should cull at least 50% of chunks
    
    std::cout << "Average culling time: " << avg_time_ms << " ms for " << chunks.size() << " chunks" << std::endl;
    std::cout << "Average visible chunks: " << avg_visible << " (" << (100.0 * avg_visible / chunks.size()) << "%)" << std::endl;
    std::cout << "Culling efficiency: " << (100.0 * culling_ratio) << "%" << std::endl;
}

// Test meshing quality - check for proper quad merging
TEST_F(VoxelPerformanceTest, MeshingQuality) {
    GreedyMesher mesher;
    
    // Create a chunk with large flat surfaces for optimal quad merging
    Chunk flat_chunk;
    
    // Fill bottom half with stone (should create large quads)
    for (int y = 0; y < CHUNK_SIZE/2; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                flat_chunk.SetVoxel(x, y, z, VoxelType::STONE);
            }
        }
    }
    
    auto mesh_data = mesher.GenerateMesh(flat_chunk);
    
    // Calculate theoretical minimum quads vs actual quads
    size_t solid_voxels = (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE) / 2; // Half filled
    size_t exposed_faces = (CHUNK_SIZE * CHUNK_SIZE) + // Top face
                          (CHUNK_SIZE * CHUNK_SIZE * 4); // Side faces (approximate)
    
    // With perfect greedy meshing, we should have much fewer quads than faces
    double compression_ratio = static_cast<double>(mesh_data.quad_count) / exposed_faces;
    
    EXPECT_LT(compression_ratio, 0.5); // At least 50% reduction
    EXPECT_GT(mesh_data.quad_count, 0);
    
    std::cout << "Solid voxels: " << solid_voxels << std::endl;
    std::cout << "Generated quads: " << mesh_data.quad_count << std::endl;
    std::cout << "Compression ratio: " << (100.0 * compression_ratio) << "%" << std::endl;
}

// Test memory efficiency
TEST_F(VoxelPerformanceTest, MemoryEfficiency) {
    // Test chunk memory usage
    size_t chunk_size = sizeof(Chunk);
    size_t voxel_data_size = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(VoxelType);
    
    // Chunk should not be significantly larger than its voxel data
    EXPECT_LT(chunk_size, voxel_data_size * 2); // Less than 2x the voxel data size
    
    // Test mesh data memory usage
    GreedyMesher mesher;
    auto mesh_data = mesher.GenerateMesh(*test_chunk_);
    
    size_t vertex_memory = mesh_data.vertices.size() * sizeof(Vertex);
    size_t index_memory = mesh_data.indices.size() * sizeof(uint32_t);
    size_t total_mesh_memory = vertex_memory + index_memory;
    
    // Mesh should be reasonable size (not more than a few MB per chunk)
    EXPECT_LT(total_mesh_memory, 5 * 1024 * 1024); // Less than 5MB
    
    std::cout << "Chunk size: " << chunk_size << " bytes" << std::endl;
    std::cout << "Voxel data size: " << voxel_data_size << " bytes" << std::endl;
    std::cout << "Mesh memory: " << total_mesh_memory << " bytes" << std::endl;
    std::cout << "Vertices: " << mesh_data.vertices.size() << " (" << vertex_memory << " bytes)" << std::endl;
    std::cout << "Indices: " << mesh_data.indices.size() << " (" << index_memory << " bytes)" << std::endl;
}

// Test different meshing configurations
TEST_F(VoxelPerformanceTest, MeshingConfigurations) {
    // Test with different mesher configurations
    struct ConfigTest {
        std::string name;
        GreedyMesher::Config config;
    };
    
    std::vector<ConfigTest> configs = {
        {"Default", GreedyMesher::Config{}},
        {"No Face Culling", [](){
            GreedyMesher::Config config;
            config.enable_face_culling = false;
            return config;
        }()},
        {"No AO", [](){
            GreedyMesher::Config config;
            config.enable_ambient_occlusion = false;
            return config;
        }()},
        {"Small Quads", [](){
            GreedyMesher::Config config;
            config.max_quad_size = 4;
            return config;
        }()},
    };
    
    for (const auto& config_test : configs) {
        GreedyMesher mesher(config_test.config);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto mesh_data = mesher.GenerateMesh(*test_chunk_);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double time_ms = duration.count() / 1000.0;
        
        // All configurations should complete successfully
        EXPECT_GT(mesh_data.vertices.size(), 0);
        EXPECT_GT(mesh_data.quad_count, 0);
        EXPECT_LT(time_ms, 50.0); // Should complete within 50ms
        
        std::cout << config_test.name << ": " << time_ms << " ms, "
                  << mesh_data.vertices.size() << " vertices, "
                  << mesh_data.quad_count << " quads" << std::endl;
    }
}

// Test integration - full render pipeline simulation
TEST_F(VoxelPerformanceTest, FullPipelineSimulation) {
    // Simulate a complete frame without actual OpenGL calls
    VoxelRenderer renderer;
    ASSERT_TRUE(renderer.Initialize());
    
    SimpleVoxelWorld world(4); // 4x4 world = 16 chunks
    renderer.SetWorld(&world);
    
    Camera camera;
    camera.SetPerspective(45.0f, 16.0f/9.0f, 0.1f, 500.0f);
    camera.SetPosition(Vector3f(32.0f, 10.0f, 32.0f)); // Center of world
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Simulate multiple frames
    constexpr int frames = 10;
    for (int frame = 0; frame < frames; ++frame) {
        // Move camera slightly each frame
        camera.MoveForward(1.0f);
        camera.Rotate(1.0f, 0.0f);
        
        // Update renderer
        renderer.Update(0.016f, camera); // 60fps = 16ms per frame
        
        // Simulate render call (won't actually render without OpenGL context)
        // renderer.Render(camera); // Skip actual rendering
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double avg_frame_time = (duration.count() / 1000.0) / frames;
    
    // Check performance expectations
    EXPECT_LT(avg_frame_time, 16.0); // Should maintain 60fps (16ms per frame)
    
    auto stats = renderer.GetStats();
    EXPECT_GT(stats.total_chunks, 0);
    
    std::cout << "Average frame time: " << avg_frame_time << " ms" << std::endl;
    std::cout << "Total chunks: " << stats.total_chunks << std::endl;
    std::cout << "Simulated FPS: " << (1000.0 / avg_frame_time) << std::endl;
}

// Stress test with maximum chunk size
TEST_F(VoxelPerformanceTest, MaxChunkStressTest) {
    // Create worst-case scenario: checkerboard pattern (no quad merging possible)
    Chunk checkerboard_chunk;
    
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                VoxelType voxel = ((x + y + z) % 2 == 0) ? VoxelType::STONE : VoxelType::AIR;
                checkerboard_chunk.SetVoxel(x, y, z, voxel);
            }
        }
    }
    
    GreedyMesher mesher;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto mesh_data = mesher.GenerateMesh(checkerboard_chunk);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Even worst case should complete within reasonable time
    EXPECT_LT(duration.count(), 100); // Less than 100ms
    EXPECT_GT(mesh_data.vertices.size(), 0);
    
    std::cout << "Checkerboard pattern meshing time: " << duration.count() << " ms" << std::endl;
    std::cout << "Generated vertices: " << mesh_data.vertices.size() << std::endl;
    std::cout << "Generated quads: " << mesh_data.quad_count << std::endl;
}