#ifndef PYNOVAGE_MATH_GEOMETRY_PRIMITIVES_HPP
#define PYNOVAGE_MATH_GEOMETRY_PRIMITIVES_HPP

#include "../vector3.hpp"
#include "../matrix3.hpp"
#include "../simd_utils.hpp"
#include "../math_constants.hpp"
#include <algorithm>

namespace pynovage {
namespace math {
namespace geometry {

/**
 * @brief Represents an infinite line in 3D space
 * 
 * A line is defined by a point and a direction vector.
 * The direction vector is always normalized.
 */
class Line3D {
public:
    Vector3 origin;    // Point on the line
    Vector3 direction; // Normalized direction vector

Line3D() : origin(0.0f, 0.0f, 0.0f), direction(1.0f, 0.0f, 0.0f) {}
    
    Line3D(const Vector3& origin_, const Vector3& direction_)
        : origin(origin_), direction(direction_.normalized()) {}
    
    /**
     * @brief Returns the closest point on the line to a given point
     * @param point Point to find closest position to
     * @return Closest point on the line
     */
    Vector3 closestPoint(const Vector3& point) const {
        Vector3 toPoint = point - origin;
        float t = toPoint.dot(direction);
        return origin + direction * t;
    }
};

/**
 * @brief Represents a ray in 3D space (half-line from origin in direction)
 * 
 * A ray is similar to a line but extends only in the positive direction
 * from its origin.
 */
class Ray3D {
public:
    Vector3 origin;    // Starting point of the ray
    Vector3 direction; // Normalized direction vector

Ray3D() : origin(0.0f, 0.0f, 0.0f), direction(1.0f, 0.0f, 0.0f) {}
    
    Ray3D(const Vector3& origin_, const Vector3& direction_)
        : origin(origin_), direction(direction_.normalized()) {}
    
    /**
     * @brief Returns the point at distance t along the ray
     * @param t Distance along the ray (must be >= 0)
     * @return Point at distance t
     */
    Vector3 getPoint(float t) const {
#if PYNOVAGE_MATH_HAS_FMA
        alignas(16) float scalarArr[4] = {t, t, t, 0.0f};
        alignas(16) float temp[4];
        SimdUtils::MultiplyAndAdd4f(direction.xyzw, scalarArr, origin.xyzw, temp);
        return Vector3(temp[0], temp[1], temp[2]);
#elif PYNOVAGE_MATH_HAS_SSE
        __m128 dir = _mm_load_ps(direction.xyzw);
        __m128 orig = _mm_load_ps(origin.xyzw);
        __m128 scalar = _mm_set1_ps(t);
        __m128 result = _mm_add_ps(orig, _mm_mul_ps(dir, scalar));
        alignas(16) float temp[4];
        _mm_store_ps(temp, result);
        return Vector3(temp[0], temp[1], temp[2]);
#else
        return origin + direction * t;
#endif
    }

    /**
     * @brief Returns the closest point on the ray to a given point
     * @param point Point to find closest position to
     * @return Closest point on the ray
     */
    Vector3 closestPoint(const Vector3& point) const {
        Vector3 toPoint = point - origin;
        float t = toPoint.dot(direction);
        t = t < 0.0f ? 0.0f : t;  // Clamp to origin if behind ray
        return origin + direction * t;
    }
};

/**
 * @brief Represents a plane in 3D space
 * 
 * A plane is defined by a normal vector and a distance from the origin
 * along that normal. The normal is always normalized.
 */
class Plane {
public:
    Vector3 normal; // Normalized normal vector
    float distance; // Distance from origin along normal

Plane() : normal(0.0f, 1.0f, 0.0f), distance(0.0f) {}
    
    Plane(const Vector3& normal_, float distance_)
        : normal(normal_), distance(distance_) {
        // Ensure plane is in normalized form: |normal| = 1 and distance scaled accordingly
        float len = normal.length();
        if (len > 0.0f && std::abs(len - 1.0f) > 1e-6f) {
            float invLen = 1.0f / len;
            normal *= invLen;
            distance *= invLen;
        } else if (len == 0.0f) {
            // Degenerate normal; default to Y-up plane at origin
            normal = Vector3(0.0f, 1.0f, 0.0f);
            distance = 0.0f;
        }
    }
    
    /**
     * @brief Constructs a plane from a point and normal
     * @param point Point on the plane
     * @param normal Normal vector of the plane
     */
    static Plane fromPointAndNormal(const Vector3& point, const Vector3& normal) {
        Vector3 normalizedNormal = normal.normalized();
        return Plane(normalizedNormal, point.dot(normalizedNormal));
    }

    /**
     * @brief Returns the closest point on the plane to a given point
     * @param point Point to find closest position to
     * @return Closest point on the plane
     */
    Vector3 closestPoint(const Vector3& point) const {
        float d = signedDistance(point);
        return point - normal * d;
    }

    /**
     * @brief Returns the signed distance from a point to the plane
     * @param point Point to calculate distance to
     * @return Signed distance (negative if point is behind the plane)
     */
    float signedDistance(const Vector3& point) const {
        return point.dot(normal) - distance;
    }
};

/**
 * @brief Represents an axis-aligned bounding box in 3D space
 * 
 * An AABB is defined by its minimum and maximum points along each axis.
 */
class AABB {
public:
    Vector3 min; // Minimum point
    Vector3 max; // Maximum point

AABB() : min(0.0f, 0.0f, 0.0f), max(0.0f, 0.0f, 0.0f) {}
    
    AABB(const Vector3& min_, const Vector3& max_) : min(min_), max(max_) {}

    /**
     * @brief Returns the center point of the AABB
     */
    Vector3 center() const {
#if PYNOVAGE_MATH_HAS_SSE
        __m128 vmin = _mm_load_ps(min.xyzw);
        __m128 vmax = _mm_load_ps(max.xyzw);
        __m128 sum = _mm_add_ps(vmin, vmax);
        __m128 result = _mm_mul_ps(sum, _mm_set1_ps(0.5f));
        alignas(16) float temp[4];
        _mm_store_ps(temp, result);
        return Vector3(temp[0], temp[1], temp[2]);
#else
        return (min + max) * 0.5f;
#endif
    }

    /**
     * @brief Returns the dimensions (width, height, depth) of the AABB
     */
    Vector3 dimensions() const {
#if PYNOVAGE_MATH_HAS_SSE
        __m128 vmin = _mm_load_ps(min.xyzw);
        __m128 vmax = _mm_load_ps(max.xyzw);
        __m128 result = _mm_sub_ps(vmax, vmin);
        alignas(16) float temp[4];
        _mm_store_ps(temp, result);
        return Vector3(temp[0], temp[1], temp[2]);
#else
        return max - min;
#endif
    }

    /**
     * @brief Returns the half-extents (half width, height, depth) of the AABB
     */
    Vector3 halfExtents() const {
#if PYNOVAGE_MATH_HAS_SSE
        __m128 vmin = _mm_load_ps(min.xyzw);
        __m128 vmax = _mm_load_ps(max.xyzw);
        __m128 diff = _mm_sub_ps(vmax, vmin);
        __m128 result = _mm_mul_ps(diff, _mm_set1_ps(0.5f));
        alignas(16) float temp[4];
        _mm_store_ps(temp, result);
        return Vector3(temp[0], temp[1], temp[2]);
#else
        return (max - min) * 0.5f;
#endif
    }

    /**
     * @brief Returns whether the AABB contains a point
     */
    bool contains(const Vector3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    /**
     * @brief Returns the closest point on/in the AABB to a given point
     */
    Vector3 closestPoint(const Vector3& point) const {
        return Vector3(
            std::clamp(point.x, min.x, max.x),
            std::clamp(point.y, min.y, max.y),
            std::clamp(point.z, min.z, max.z)
        );
    }
};

/**
 * @brief Represents a sphere in 3D space
 * 
 * A sphere is defined by its center point and radius.
 */
class Sphere {
public:
    Vector3 center; // Center point
    float radius;   // Radius

Sphere() : center(0.0f, 0.0f, 0.0f), radius(1.0f) {}
    
    Sphere(const Vector3& center_, float radius_)
        : center(center_), radius(radius_) {}

    /**
     * @brief Returns whether the sphere contains a point
     */
    bool contains(const Vector3& point) const {
        return (point - center).lengthSquared() <= radius * radius;
    }

    /**
     * @brief Returns the closest point on/in the sphere to a given point
     */
    Vector3 closestPoint(const Vector3& point) const {
        Vector3 toPoint = point - center;
        float distSq = toPoint.lengthSquared();
        
        if (distSq > radius * radius) {
            toPoint *= radius / std::sqrt(distSq);
        }
        
        return center + toPoint;
    }
};

} // namespace geometry
} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_GEOMETRY_PRIMITIVES_HPP