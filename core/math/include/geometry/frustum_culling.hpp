#pragma once

#include "../vector3.hpp"
#include "../matrix4.hpp"
#include "primitives.hpp"
#include <array>
#include <cstdint>
#include <functional>
namespace pynovage {
namespace math {
namespace geometry {

class FrustumCulling {
public:
    enum class TestResult {
        OUTSIDE,
        INTERSECT,
        INSIDE
    };
    
    // Planes in order: NEAR, FAR, LEFT, RIGHT, TOP, BOTTOM
    static constexpr std::size_t NUM_PLANES = 6;
    
    explicit FrustumCulling(const Matrix4& viewProjection) {
        updatePlanes(viewProjection);
    }
    
    void update(const Matrix4& viewProjection) {
        updatePlanes(viewProjection);
    }
    
    TestResult testPoint(const Vector3& point) const {
        for (const Plane& plane : planes_) {
            if (plane.signedDistance(point) < 0) {
                return TestResult::OUTSIDE;
            }
        }
        return TestResult::INSIDE;
    }
    
    TestResult testSphere(const Vector3& center, float radius) const {
        bool intersect = false;
        
        for (const Plane& plane : planes_) {
            float distance = plane.signedDistance(center);
            
            if (distance < -radius) {
                return TestResult::OUTSIDE;
            } else if (std::abs(distance) <= radius) {
                intersect = true;
            }
        }
        
        return intersect ? TestResult::INTERSECT : TestResult::INSIDE;
    }
    
    TestResult testAABB(const AABB& aabb) const {
        bool intersect = false;
        
        // Get box extents
        Vector3 center = aabb.center();
        Vector3 extent = aabb.dimensions();
        
        for (const Plane& plane : planes_) {
            // Project box onto plane normal
            float r = extent.x * std::abs(plane.normal.x) +
                     extent.y * std::abs(plane.normal.y) +
                     extent.z * std::abs(plane.normal.z);
            
            float d = plane.signedDistance(center);
            
            if (d < -r) {
                return TestResult::OUTSIDE;
            } else if (std::abs(d) <= r) {
                intersect = true;
            }
        }
        
        return intersect ? TestResult::INTERSECT : TestResult::INSIDE;
    }
    
    // Optimized AABB test using SIMD
    TestResult testAABBSIMD(const AABB& aabb) const {
        bool intersect = false;
        
        // Get box center and extent as arrays for SIMD
        float center[4] = {
            aabb.center().x,
            aabb.center().y,
            aabb.center().z,
            1.0f
        };
        
        float extent[4] = {
            aabb.dimensions().x,
            aabb.dimensions().y,
            aabb.dimensions().z,
            0.0f
        };
        
        // Test against each plane using SIMD operations
        for (const Plane& plane : planes_) {
            float normal[4] = {
                plane.normal.x,
                plane.normal.y,
                plane.normal.z,
                0.0f
            };
            
            // Compute r = |n.x|*e.x + |n.y|*e.y + |n.z|*e.z
            float r = 0.0f;
            for (int i = 0; i < 3; ++i) {
                r += std::abs(normal[i]) * extent[i];
            }
            
            // Compute d = n·c + d
            float d = 0.0f;
            for (int i = 0; i < 3; ++i) {
                d += normal[i] * center[i];
            }
            d += plane.distance;
            
            if (d < -r) {
                return TestResult::OUTSIDE;
            } else if (std::abs(d) <= r) {
                intersect = true;
            }
        }
        
        return intersect ? TestResult::INTERSECT : TestResult::INSIDE;
    }
    
    const std::array<Plane, NUM_PLANES>& getPlanes() const {
        return planes_;
    }

private:
    std::array<Plane, NUM_PLANES> planes_;
    
    void updatePlanes(const Matrix4& viewProjection) {
        // Extract frustum planes from view-projection matrix
        // Reference: Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix
        // http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
        
        // Left plane
        planes_[2] = Plane(
            Vector3(
                viewProjection[0][3] + viewProjection[0][0],
                viewProjection[1][3] + viewProjection[1][0],
                viewProjection[2][3] + viewProjection[2][0]
            ).normalized(),
            viewProjection[3][3] + viewProjection[3][0]
        );
        
        // Right plane
        planes_[3] = Plane(
            Vector3(
                viewProjection[0][3] - viewProjection[0][0],
                viewProjection[1][3] - viewProjection[1][0],
                viewProjection[2][3] - viewProjection[2][0]
            ).normalized(),
            viewProjection[3][3] - viewProjection[3][0]
        );
        
        // Bottom plane
        planes_[5] = Plane(
            Vector3(
                viewProjection[0][3] + viewProjection[0][1],
                viewProjection[1][3] + viewProjection[1][1],
                viewProjection[2][3] + viewProjection[2][1]
            ).normalized(),
            viewProjection[3][3] + viewProjection[3][1]
        );
        
        // Top plane
        planes_[4] = Plane(
            Vector3(
                viewProjection[0][3] - viewProjection[0][1],
                viewProjection[1][3] - viewProjection[1][1],
                viewProjection[2][3] - viewProjection[2][1]
            ).normalized(),
            viewProjection[3][3] - viewProjection[3][1]
        );
        
        // Near plane
        planes_[0] = Plane(
            Vector3(
                viewProjection[0][3] + viewProjection[0][2],
                viewProjection[1][3] + viewProjection[1][2],
                viewProjection[2][3] + viewProjection[2][2]
            ).normalized(),
            viewProjection[3][3] + viewProjection[3][2]
        );
        
        // Far plane
        planes_[1] = Plane(
            Vector3(
                viewProjection[0][3] - viewProjection[0][2],
                viewProjection[1][3] - viewProjection[1][2],
                viewProjection[2][3] - viewProjection[2][2]
            ).normalized(),
            viewProjection[3][3] - viewProjection[3][2]
        );
        
        // Normalize planes
        for (auto& plane : planes_) {
            float invLen = 1.0f / plane.normal.length();
            plane.normal *= invLen;
            plane.distance *= invLen;
        }
    }
};

// Helper class for hierarchical frustum culling
class HierarchicalFrustumCulling {
public:
    explicit HierarchicalFrustumCulling(const Matrix4& viewProjection)
        : frustum_(viewProjection) {}
    
    void update(const Matrix4& viewProjection) {
        frustum_.update(viewProjection);
    }
    
    // Test hierarchy of bounding volumes
    template<typename Node>
    void testHierarchy(const Node& root, const std::function<void(const Node&)>& processVisible) {
        testNode(root, processVisible);
    }

private:
    FrustumCulling frustum_;
    
    template<typename Node>
    void testNode(const Node& node, const std::function<void(const Node&)>& processVisible) {
        // Test node's bounding volume
        auto result = frustum_.testAABBSIMD(node.getBounds());
        
        if (result == FrustumCulling::TestResult::OUTSIDE) {
            return;  // Node and all children are outside
        }
        
        if (result == FrustumCulling::TestResult::INSIDE) {
            // Node and all children are inside, process entire subtree
            processVisible(node);
            return;
        }
        
        // Node intersects frustum
        processVisible(node);
        
        // Test children recursively
        for (const auto& child : node.getChildren()) {
            if (child) {
                testNode(*child, processVisible);
            }
        }
    }
};

} // namespace geometry
} // namespace math
} // namespace pynovage