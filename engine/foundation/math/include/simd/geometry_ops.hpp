#pragma once

// Disable unreachable code warning in optimized builds  
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4702) // unreachable code
#endif

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Undefine Windows min/max macros if already defined
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "types.hpp"
#include "vector_ops.hpp"
#include "matrix_ops.hpp"
#include <limits>

namespace PyNovaGE {
namespace SIMD {

// AABB (Axis-Aligned Bounding Box)
template<typename T>
class AABB {
    static_assert(std::is_floating_point_v<T>, "AABB only supports floating-point types");
public:
    using value_type = T;
    Vector<T, 3> min;
    Vector<T, 3> max;

    constexpr AABB() : min(T(0)), max(T(0)) {}
    constexpr AABB(const Vector<T, 3>& min_, const Vector<T, 3>& max_) : min(min_), max(max_) {}

    // Check if point is inside AABB
    bool contains(const Vector<T, 3>& point) const {
        #if defined(NOVA_AVX2_AVAILABLE)
        if constexpr (std::is_same_v<T, float>) {
            // Use AVX2 for optimized point containment - still process as 3D with Z=0 for 2D physics
            __m256 p_wide = _mm256_set_ps(0, 0, 0, 0, 0, point[2], point[1], point[0]);
            __m256 mn_wide = _mm256_set_ps(0, 0, 0, 0, 0, min[2], min[1], min[0]);
            __m256 mx_wide = _mm256_set_ps(0, 0, 0, 0, 0, max[2], max[1], max[0]);
            
            __m256 greater = _mm256_cmp_ps(p_wide, mn_wide, _CMP_GE_OQ);
            __m256 lesser = _mm256_cmp_ps(p_wide, mx_wide, _CMP_LE_OQ);
            __m256 result = _mm256_and_ps(greater, lesser);
            
            // Get bitmask for all 8 lanes, but only check the lower 3 bits (x, y, z)
            // The physics system uses this for 2D (Z always 0) so effectively checks x, y
            int mask = _mm256_movemask_ps(result);
            return (mask & 0x07) == 0x07;
        }
        #elif defined(NOVA_SSE2_AVAILABLE)
        if constexpr (std::is_same_v<T, float>) {
            // Use SSE2 for 3D point containment (x, y, z components)
            __m128 p = _mm_set_ps(0, point[2], point[1], point[0]);
            __m128 mn = _mm_set_ps(0, min[2], min[1], min[0]);
            __m128 mx = _mm_set_ps(0, max[2], max[1], max[0]);
            
            __m128 greater = _mm_cmpge_ps(p, mn);
            __m128 lesser = _mm_cmple_ps(p, mx);
            __m128 result = _mm_and_ps(greater, lesser);
            
            // Check the lower 3 bits (x, y, z) are all set
            return (_mm_movemask_ps(result) & 0x07) == 0x07;
        }
        #elif defined(NOVA_NEON_AVAILABLE)
        if constexpr (std::is_same_v<T, float>) {
            float32x4_t p = vld1q_f32(point.data());
            float32x4_t mn = vld1q_f32(min.data());
            float32x4_t mx = vld1q_f32(max.data());
            
            uint32x4_t greater = vcgeq_f32(p, mn);
            uint32x4_t lesser = vcleq_f32(p, mx);
            uint32x4_t result = vandq_u32(greater, lesser);
            
            return (vgetq_lane_u32(result, 0) & vgetq_lane_u32(result, 1) & 
                   vgetq_lane_u32(result, 2)) != 0;
        }
        #endif

        // Fallback implementation
        return point[0] >= min[0] && point[0] <= max[0] &&
               point[1] >= min[1] && point[1] <= max[1] &&
               point[2] >= min[2] && point[2] <= max[2];
    }

    // Check if this AABB intersects with another
    bool intersects(const AABB& other) const {
        #if defined(NOVA_AVX2_AVAILABLE)
        if constexpr (std::is_same_v<T, float>) {
            // Use AVX2 for optimized AABB intersection - process as 3D with Z=0 for 2D physics
            __m256 this_min_wide = _mm256_set_ps(0, 0, 0, 0, 0, min[2], min[1], min[0]);
            __m256 this_max_wide = _mm256_set_ps(0, 0, 0, 0, 0, max[2], max[1], max[0]);
            __m256 other_min_wide = _mm256_set_ps(0, 0, 0, 0, 0, other.min[2], other.min[1], other.min[0]);
            __m256 other_max_wide = _mm256_set_ps(0, 0, 0, 0, 0, other.max[2], other.max[1], other.max[0]);
            
            // For intersection: this.max >= other.min && this.min <= other.max
            __m256 left = _mm256_cmp_ps(this_max_wide, other_min_wide, _CMP_GE_OQ);
            __m256 right = _mm256_cmp_ps(this_min_wide, other_max_wide, _CMP_LE_OQ);
            __m256 result = _mm256_and_ps(left, right);
            
            // Get bitmask for all 8 lanes, but only check the lower 3 bits (x, y, z)
            // The physics system uses this for 2D (Z always 0) so effectively checks x, y
            int mask = _mm256_movemask_ps(result);
            return (mask & 0x07) == 0x07;
        }
        #elif defined(NOVA_SSE2_AVAILABLE)
        if constexpr (std::is_same_v<T, float>) {
            // Use SSE2 for 3D AABB intersection (x, y, z components)
            __m128 this_min = _mm_set_ps(0, min[2], min[1], min[0]);
            __m128 this_max = _mm_set_ps(0, max[2], max[1], max[0]);
            __m128 other_min = _mm_set_ps(0, other.min[2], other.min[1], other.min[0]);
            __m128 other_max = _mm_set_ps(0, other.max[2], other.max[1], other.max[0]);
            
            // For intersection: this.max >= other.min && this.min <= other.max
            __m128 left = _mm_cmpge_ps(this_max, other_min);
            __m128 right = _mm_cmple_ps(this_min, other_max);
            __m128 result = _mm_and_ps(left, right);
            
            // Check the lower 3 bits (x, y, z) are all set
            return (_mm_movemask_ps(result) & 0x07) == 0x07;
        }
        #endif

        // Fallback implementation
        return min[0] <= other.max[0] && max[0] >= other.min[0] &&
               min[1] <= other.max[1] && max[1] >= other.min[1] &&
               min[2] <= other.max[2] && max[2] >= other.min[2];
    }

    // Get AABB center
    Vector<T, 3> center() const {
        return (min + max) * T(0.5);
    }

    // Get AABB extent
    Vector<T, 3> extent() const {
        return (max - min) * T(0.5);
    }
};

// Sphere
template<typename T>
class Sphere {
    static_assert(std::is_floating_point_v<T>, "Sphere only supports floating-point types");
public:
    using value_type = T;
    Vector<T, 3> center;
    T radius;

    constexpr Sphere() : center(T(0)), radius(T(0)) {}
    constexpr Sphere(const Vector<T, 3>& center_, T radius_) : center(center_), radius(radius_) {}

    // Check if point is inside sphere
    bool contains(const Vector<T, 3>& point) const {
        Vector<T, 3> diff = point - center;
        return length_squared(diff) <= radius * radius;
    }

    // Check if this sphere intersects with another
    bool intersects(const Sphere& other) const {
        Vector<T, 3> diff = other.center - center;
        T radii = radius + other.radius;
        return length_squared(diff) <= radii * radii;
    }

    // Check if this sphere intersects with an AABB
    bool intersects(const AABB<T>& box) const {
        // Find the closest point on the AABB to the sphere center
        Vector<T, 3> closest;
        for (int i = 0; i < 3; ++i) {
            closest[i] = std::max(box.min[i], std::min(center[i], box.max[i]));
        }

        // Check if the closest point is within the sphere
        Vector<T, 3> diff = closest - center;
        return length_squared(diff) <= radius * radius;
    }
};

// Ray
template<typename T>
class Ray {
    static_assert(std::is_floating_point_v<T>, "Ray only supports floating-point types");
public:
    using value_type = T;
    Vector<T, 3> origin;
    Vector<T, 3> direction;

    constexpr Ray() : origin(T(0)), direction(T(0), T(0), T(1)) {}
    constexpr Ray(const Vector<T, 3>& origin_, const Vector<T, 3>& direction_)
        : origin(origin_), direction(normalize(direction_)) {}

    // Intersect ray with sphere
    bool intersects(const Sphere<T>& sphere, T& t) const {
        Vector<T, 3> m = origin - sphere.center;
        T b = dot(m, direction);
        T c = dot(m, m) - sphere.radius * sphere.radius;

        // Exit if ray origin outside sphere (c > 0) and ray pointing away from sphere (b > 0)
        if (c > T(0) && b > T(0)) return false;

        T discr = b * b - c;
        if (discr < T(0)) return false;

        t = -b - std::sqrt(discr);
        if (t < T(0)) t = T(0);
        return true;
    }

    // Intersect ray with AABB
    bool intersects(const AABB<T>& box, T& t) const {
        T tmin = T(0);
        T tmax = std::numeric_limits<T>::max();
        
        for (int i = 0; i < 3; ++i) {
            if (std::abs(direction[i]) < T(1e-8)) {
                // Ray is parallel to slab - check if origin is within slab bounds
                if (origin[i] < box.min[i] || origin[i] > box.max[i]) {
                    return false;
                }
            } else {
                // Compute intersection t values
                T ood = T(1) / direction[i];
                T t1 = (box.min[i] - origin[i]) * ood;
                T t2 = (box.max[i] - origin[i]) * ood;
                
                if (t1 > t2) std::swap(t1, t2);
                
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);
                
                if (tmin > tmax) return false;
            }
        }
        
        // Ray intersection occurs when tmax >= tmin and tmax >= 0
        if (tmax < T(0)) return false;
        
        t = tmin < T(0) ? tmax : tmin;
        return true;
    }

    // Get point along ray at distance t
    Vector<T, 3> at(T t) const {
        return origin + direction * t;
    }
};

// Plane
template<typename T>
class Plane {
    static_assert(std::is_floating_point_v<T>, "Plane only supports floating-point types");
public:
    using value_type = T;
    Vector<T, 3> normal;
    T distance;

    constexpr Plane() : normal(T(0), T(1), T(0)), distance(T(0)) {}
    constexpr Plane(const Vector<T, 3>& normal_, T distance_)
        : normal(normalize(normal_)), distance(distance_) {}
    constexpr Plane(const Vector<T, 3>& normal_, const Vector<T, 3>& point)
        : normal(normalize(normal_)), distance(-dot(normalize(normal_), point)) {}

    // Get signed distance from point to plane
    T signedDistance(const Vector<T, 3>& point) const {
        return dot(normal, point) + distance;
    }

    // Check which side of the plane a point is on
    int classifyPoint(const Vector<T, 3>& point) const {
        T d = signedDistance(point);
        if (d > T(1e-6)) return 1;      // Front
        if (d < T(-1e-6)) return -1;     // Back
        return 0;                         // On plane
    }

    // Intersect plane with ray
    bool intersects(const Ray<T>& ray, T& t) const {
        T denom = dot(normal, ray.direction);
        if (std::abs(denom) < T(1e-6)) return false;

        t = -(dot(normal, ray.origin) + distance) / denom;
        return t >= T(0);
    }
};

} // namespace SIMD
} // namespace PyNovaGE

#ifdef _MSC_VER
#pragma warning(pop)
#endif
