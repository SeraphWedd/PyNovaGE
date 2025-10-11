#include "renderer/voxel/chunk.hpp"
#include <glad/gl.h>
#include <iostream>

#ifndef PVG_VOXEL_DEBUG_LOGS
#define PVG_VOXEL_DEBUG_LOGS 0
#endif

namespace PyNovaGE {
namespace Renderer {
namespace Voxel {

Chunk::Chunk() : coordinates_(0, 0) {
    Clear();
}

Chunk::Chunk(const ChunkCoord2D& coord) : coordinates_(coord) {
    Clear();
}

Chunk::~Chunk() = default;

VoxelType Chunk::GetVoxel(int x, int y, int z) const {
    if (!IsValidCoordinate(x, y, z)) {
        return VoxelType::AIR;
    }
    return voxels_[GetIndex(x, y, z)];
}

void Chunk::SetVoxel(int x, int y, int z, VoxelType voxel_type) {
    if (!IsValidCoordinate(x, y, z)) {
        return;
    }
    voxels_[GetIndex(x, y, z)] = voxel_type;
    MarkDirty();
    UpdateEmptyStatus();
}

void Chunk::GenerateTestData() {
    // Generate simple test pattern
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                VoxelType voxel = VoxelType::AIR;
                
                // Simple ground layer
                if (y < 3) {
                    voxel = VoxelType::STONE;
                } else if (y == 3) {
                    voxel = VoxelType::DIRT;
                }
                
                SetVoxel(x, y, z, voxel);
            }
        }
    }
}

void Chunk::Clear() {
    voxels_.fill(VoxelType::AIR);
    empty_ = true;
    MarkDirty();
}

void Chunk::SetMesh(std::unique_ptr<VoxelMesh> mesh) {
    mesh_ = std::move(mesh);
}

Chunk::VoxelStats Chunk::GetStats() const {
    VoxelStats stats;
    stats.solid_voxels = 0;
    stats.air_voxels = 0;
    stats.total_voxels = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    
    for (const auto& voxel : voxels_) {
        if (voxel == VoxelType::AIR) {
            stats.air_voxels++;
        } else {
            stats.solid_voxels++;
        }
    }
    
    return stats;
}

void Chunk::UpdateEmptyStatus() {
    bool is_empty = true;
    for (const auto& voxel : voxels_) {
        if (voxel != VoxelType::AIR) {
            is_empty = false;
            break;
        }
    }
    empty_ = is_empty;
}

// VoxelMesh implementation
VoxelMesh::~VoxelMesh() {
    // Cleanup OpenGL resources
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }
    if (ebo_ != 0) {
        glDeleteBuffers(1, &ebo_);
        ebo_ = 0;
    }
}

VoxelMesh::VoxelMesh(VoxelMesh&& other) noexcept 
    : vao_(other.vao_), vbo_(other.vbo_), ebo_(other.ebo_), 
      vertex_count_(other.vertex_count_), index_count_(other.index_count_) {
    // Take ownership of OpenGL resources
    other.vao_ = 0;
    other.vbo_ = 0;
    other.ebo_ = 0;
    other.vertex_count_ = 0;
    other.index_count_ = 0;
}

VoxelMesh& VoxelMesh::operator=(VoxelMesh&& other) noexcept {
    if (this != &other) {
        // Clean up existing resources
        if (vao_ != 0) glDeleteVertexArrays(1, &vao_);
        if (vbo_ != 0) glDeleteBuffers(1, &vbo_);
        if (ebo_ != 0) glDeleteBuffers(1, &ebo_);
        
        // Take ownership of other's resources
        vao_ = other.vao_;
        vbo_ = other.vbo_;
        ebo_ = other.ebo_;
        vertex_count_ = other.vertex_count_;
        index_count_ = other.index_count_;
        
        // Reset other's resources
        other.vao_ = 0;
        other.vbo_ = 0;
        other.ebo_ = 0;
        other.vertex_count_ = 0;
        other.index_count_ = 0;
    }
    return *this;
}

void VoxelMesh::UploadData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    if (vertices.empty() || indices.empty()) {
        std::cerr << "Warning: Trying to upload empty mesh data" << std::endl;
        vertex_count_ = 0;
        index_count_ = 0;
        return;
    }
    
    // Store counts
    vertex_count_ = vertices.size();
    index_count_ = indices.size();
    
    // Generate OpenGL objects if needed
    if (vao_ == 0) {
        glGenVertexArrays(1, &vao_);
    }
    if (vbo_ == 0) {
        glGenBuffers(1, &vbo_);
    }
    if (ebo_ == 0) {
        glGenBuffers(1, &ebo_);
    }
    
    // Bind VAO
    glBindVertexArray(vao_);
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    
    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
    
    // Set up vertex attributes
    // Position (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    
    // Normal (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    
    // Texture coordinates (location 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);
    
    // Texture ID (light level) (location 3) 
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture_id));
    glEnableVertexAttribArray(3);
    
    // Ambient occlusion (location 4)
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, ambient_occlusion));
    glEnableVertexAttribArray(4);
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
#if PVG_VOXEL_DEBUG_LOGS
    // Debug: Print first few vertices and indices to verify data
    static int upload_count = 0;
    if (++upload_count <= 3 && vertices.size() >= 3 && indices.size() >= 3) {
        std::cout << "DEBUG: First 3 vertices:" << std::endl;
        for (int i = 0; i < 3; ++i) {
            const auto& v = vertices[i];
            std::cout << "  [" << i << "] pos=(" << v.position.x << ", " << v.position.y << ", " << v.position.z 
                      << "), normal=(" << v.normal.x << ", " << v.normal.y << ", " << v.normal.z << ")" << std::endl;
        }
        std::cout << "First 3 indices: [" << indices[0] << ", " << indices[1] << ", " << indices[2] << "]" << std::endl;
    }
    
    std::cout << "Mesh uploaded: " << vertex_count_ << " vertices, " << index_count_ << " indices" << std::endl;
#endif
}

void VoxelMesh::Bind() const {
    if (vao_ != 0) {
        glBindVertexArray(vao_);
    }
}

void VoxelMesh::Unbind() const {
    glBindVertexArray(0);
}

void VoxelMesh::Draw() const {
    if (vao_ == 0) {
        std::cout << "ERROR: VoxelMesh::Draw() called with invalid VAO!" << std::endl;
        return;
    }
    if (index_count_ == 0) {
        std::cout << "ERROR: VoxelMesh::Draw() called with zero indices!" << std::endl;
        return;
    }
    
#if PVG_VOXEL_DEBUG_LOGS
    // Debug: Print draw call details once every 60 frames
    static int draw_call_count = 0;
    if (++draw_call_count % 60 == 1) {
        std::cout << "VoxelMesh::Draw() - VAO: " << vao_ << ", VBO: " << vbo_ << ", EBO: " << ebo_ << std::endl;
        std::cout << "  Vertex count: " << vertex_count_ << ", Index count: " << index_count_ << std::endl;
        
        // Check if VAO is still bound
        GLint bound_vao;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &bound_vao);
        std::cout << "  Currently bound VAO: " << bound_vao << std::endl;
        
        // Check if we have a valid shader program bound
        GLint program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &program);
        std::cout << "  Currently bound shader program: " << program << std::endl;
        
        if (program == 0) {
            std::cout << "  ERROR: No shader program bound!" << std::endl;
        }
        
        // Detailed OpenGL state debugging
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        std::cout << "  Viewport: (" << viewport[0] << ", " << viewport[1] 
                  << ", " << viewport[2] << ", " << viewport[3] << ")" << std::endl;
        
        GLboolean depth_test_enabled = glIsEnabled(GL_DEPTH_TEST);
        std::cout << "  Depth test: " << (depth_test_enabled ? "ENABLED" : "DISABLED") << std::endl;
        
        GLboolean cull_face_enabled = glIsEnabled(GL_CULL_FACE);
        std::cout << "  Face culling: " << (cull_face_enabled ? "ENABLED" : "DISABLED") << std::endl;
        
        if (cull_face_enabled) {
            GLint cull_face_mode;
            glGetIntegerv(GL_CULL_FACE_MODE, &cull_face_mode);
            std::cout << "  Cull face mode: " << (cull_face_mode == GL_BACK ? "GL_BACK" : 
                      cull_face_mode == GL_FRONT ? "GL_FRONT" : "GL_FRONT_AND_BACK") << std::endl;
            
            GLint front_face;
            glGetIntegerv(GL_FRONT_FACE, &front_face);
            std::cout << "  Front face: " << (front_face == GL_CCW ? "GL_CCW" : "GL_CW") << std::endl;
        }
        
        GLint polygon_mode[2];
        glGetIntegerv(GL_POLYGON_MODE, polygon_mode);
        std::cout << "  Polygon mode: " << (polygon_mode[0] == GL_FILL ? "GL_FILL" : 
                  polygon_mode[0] == GL_LINE ? "GL_LINE" : "GL_POINT") << std::endl;
        
        // Check framebuffer binding
        GLint framebuffer;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);
        std::cout << "  Bound framebuffer: " << framebuffer << " (0 = default)" << std::endl;
        
        // Check if there are pending OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "  OPENGL ERROR before draw: " << error << std::endl;
        }
        
        std::cout << "  Mesh draw complete" << std::endl;
    }
#endif
    
    // Perform the draw call
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_count_), GL_UNSIGNED_INT, nullptr);
    
    // Check for OpenGL errors after draw
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "OpenGL error in VoxelMesh::Draw(): " << error << std::endl;
    }
}

} // namespace Voxel
} // namespace Renderer
} // namespace PyNovaGE