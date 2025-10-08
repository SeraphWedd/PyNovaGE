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
    enum class TestResult : uint32_t {
        OUTSIDE = 0,
        INTERSECT = 1,
        INSIDE = 2
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
        bool allPositive = true;
        for (const Plane& plane : planes_) {
            float d = plane.signedDistance(point);
            if (d < 0) {
                return TestResult::OUTSIDE;
            }
            allPositive = allPositive && (d > 0);
        }
        return allPositive ? TestResult::INSIDE : TestResult::OUTSIDE;
    }
    
    TestResult testSphere(const Vector3& center, float radius) const {
        constexpr float eps = 1e-6f;
        bool allInside = true;
        bool anyIntersect = false;
        
        for (const Plane& plane : planes_) {
            float distance = plane.signedDistance(center);
            
            if (distance < -radius - eps) {
                return TestResult::OUTSIDE;
            }
            
            // Sphere is inside the plane only if the center is farther from the plane than radius
            if (distance < radius + eps) {
                if (distance > -radius - eps) {
                    anyIntersect = true;
                }
                if (distance < radius - eps) {
                    allInside = false;
                }
            }
        }
        
        return allInside ? TestResult::INSIDE : (anyIntersect ? TestResult::INTERSECT : TestResult::OUTSIDE);
    }
    
    TestResult testAABB(const AABB& aabb) const {
        constexpr float eps = 1e-6f;
        bool allInside = true;
        bool anyIntersect = false;
        
        // Get box extents
        Vector3 center = aabb.center();
        Vector3 extent = aabb.halfExtents();  // Use half extents for r calculation
        
        for (const Plane& plane : planes_) {
            // Project box onto plane normal using half extents
            float r = extent.x * std::abs(plane.normal.x) +
                     extent.y * std::abs(plane.normal.y) +
                     extent.z * std::abs(plane.normal.z);
            
            float d = plane.signedDistance(center);
            
            if (d < -r - eps) {
                return TestResult::OUTSIDE;
            }
            
            if (d < r - eps) {
                if (d > -r + eps) {
                    anyIntersect = true;
                }
                allInside = false;
            }
        }
        
        if (allInside) {
            return TestResult::INSIDE;
        }
        return anyIntersect ? TestResult::INTERSECT : TestResult::OUTSIDE;
    }
    
    // Optimized AABB test using SIMD
    TestResult testAABBSIMD(const AABB& aabb) const {
        constexpr float eps = 1e-6f;
        bool anyIntersect = false;
        bool allInside = true;
        
        // Get box center and half extents as arrays for SIMD
        float center[4] = {
            aabb.center().x,
            aabb.center().y,
            aabb.center().z,
            1.0f
        };
        
        Vector3 halfExt = aabb.halfExtents();
        float extent[4] = {
            halfExt.x,
            halfExt.y,
            halfExt.z,
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
            
            // Compute d = n·c - distance
            float d = 0.0f;
            for (int i = 0; i < 3; ++i) {
                d += normal[i] * center[i];
            }
            d -= plane.distance;
            
            if (d < -r - eps) {
                return TestResult::OUTSIDE;
            }
            if (d < r - eps) {
                if (d > -r + eps) {
                    anyIntersect = true;
                }
                allInside = false;
            }
        }
        
        if (allInside) {
            return TestResult::INSIDE;
        }
        return anyIntersect ? TestResult::INTERSECT : TestResult::OUTSIDE;
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
        
// Extract planes from view-projection matrix using correct plane equations
        
        // Extract with row-major, row-vector convention: plane = row3 ± rowN
        auto makePlane = [](float a, float b, float c, float d) {
            // Convert ax+by+cz+d=0 into Plane(normal, distance) with signedDistance = dot(p,n) - distance
            // so set normal=(a,b,c), distance = -d
            Plane p(Vector3(a, b, c), -d);
            // Ensure origin (0,0,0) is inside or on plane for typical view frustums around origin
            if (p.distance > 0.0f) {
                p.normal *= -1.0f;
                p.distance *= -1.0f;
            }
            return p;
        };

        // Left: row3 + row0
        planes_[2] = makePlane(
            viewProjection[0][3] + viewProjection[0][0],
            viewProjection[1][3] + viewProjection[1][0],
            viewProjection[2][3] + viewProjection[2][0],
            viewProjection[3][3] + viewProjection[3][0]
        );

        // Right: row3 - row0
        planes_[3] = makePlane(
            viewProjection[0][3] - viewProjection[0][0],
            viewProjection[1][3] - viewProjection[1][0],
            viewProjection[2][3] - viewProjection[2][0],
            viewProjection[3][3] - viewProjection[3][0]
        );

        // Bottom: row3 + row1
        planes_[5] = makePlane(
            viewProjection[0][3] + viewProjection[0][1],
            viewProjection[1][3] + viewProjection[1][1],
            viewProjection[2][3] + viewProjection[2][1],
            viewProjection[3][3] + viewProjection[3][1]
        );

        // Top: row3 - row1
        planes_[4] = makePlane(
            viewProjection[0][3] - viewProjection[0][1],
            viewProjection[1][3] - viewProjection[1][1],
            viewProjection[2][3] - viewProjection[2][1],
            viewProjection[3][3] - viewProjection[3][1]
        );

        // Near: row3 + row2
        planes_[0] = makePlane(
            viewProjection[0][3] + viewProjection[0][2],
            viewProjection[1][3] + viewProjection[1][2],
            viewProjection[2][3] + viewProjection[2][2],
            viewProjection[3][3] + viewProjection[3][2]
        );

        // Far: row3 - row2
        planes_[1] = makePlane(
            viewProjection[0][3] - viewProjection[0][2],
            viewProjection[1][3] - viewProjection[1][2],
            viewProjection[2][3] - viewProjection[2][2],
            viewProjection[3][3] - viewProjection[3][2]
        );

        // Ensure normals are unit length (Plane ctor normalizes); no extra normalization needed
        for (auto& plane : planes_) {
            // nothing else; Plane constructor already normalized
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