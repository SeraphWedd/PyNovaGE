#ifndef PYNOVAGE_MATH_PRIMITIVES_HPP
#define PYNOVAGE_MATH_PRIMITIVES_HPP

#include "math/vector3.hpp"
#include "math/matrix3.hpp"
#include <limits>

namespace pynovage {
namespace math {

/**
 * @brief Represents a ray in 3D space
 * 
 * A ray is defined by an origin point and a direction vector.
 * The direction vector is always normalized.
 */
class Ray {
public:
    Ray() : origin(), direction(0.0f, 0.0f, 1.0f) {}
    Ray(const Vector3& origin_, const Vector3& direction_) 
        : origin(origin_), direction(direction_.normalized()) {}

    Vector3 getPoint(float t) const { return origin + direction * t; }

    Vector3 origin;
    Vector3 direction;
};

/**
 * @brief Represents a sphere in 3D space
 * 
 * A sphere is defined by a center point and a radius.
 */
class Sphere {
public:
    Sphere() : center(), radius(1.0f) {}
    Sphere(const Vector3& center_, float radius_) 
        : center(center_), radius(radius_) {}

    Vector3 center;
    float radius;
};

/**
 * @brief Represents an Axis-Aligned Bounding Box in 3D space
 * 
 * An AABB is defined by its minimum and maximum points.
 * The box's edges are always aligned with the world coordinate axes.
 */
class AABB {
public:
    AABB() : min(), max() {}
    AABB(const Vector3& min_, const Vector3& max_) 
        : min(min_), max(max_) {}

    Vector3 getCenter() const { return (min + max) * 0.5f; }
    Vector3 getExtents() const { return (max - min) * 0.5f; }
    Vector3 getSize() const { return max - min; }

    void expand(float amount) {
        Vector3 expansion(amount, amount, amount);
        min -= expansion;
        max += expansion;
    }

    void expand(const Vector3& amount) {
        min -= amount;
        max += amount;
    }

    Vector3 min;
    Vector3 max;
};

/**
 * @brief Represents a plane in 3D space
 * 
 * A plane is defined by its normal vector and distance from origin.
 * The normal vector is always normalized.
 * The plane equation is: normal·X + d = 0, where X is any point on the plane.
 */
class Plane {
public:
    Plane() : normal(0.0f, 1.0f, 0.0f), d(0.0f) {}
    Plane(const Vector3& normal_, float d_) 
        : normal(normal_.normalized()), d(d_) {}
    Plane(const Vector3& normal_, const Vector3& point) 
        : normal(normal_.normalized()) {
        d = -normal.dot(point);
    }

    float getSignedDistance(const Vector3& point) const {
        return normal.dot(point) + d;
    }

    Vector3 normal;
    float d;
};

/**
 * @brief Represents a triangle in 3D space
 * 
 * A triangle is defined by three vertices in counter-clockwise order.
 */
class Triangle {
public:
    Triangle() : v0(), v1(), v2() {}
    Triangle(const Vector3& v0_, const Vector3& v1_, const Vector3& v2_)
        : v0(v0_), v1(v1_), v2(v2_) {}

    Vector3 getNormal() const {
        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        return edge1.cross(edge2).normalized();
    }

    float getArea() const {
        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        return edge1.cross(edge2).length() * 0.5f;
    }

    Vector3 getCenter() const {
        return (v0 + v1 + v2) * (1.0f / 3.0f);
    }

    struct Properties {
        Vector3 normal;
        float area;
        Vector3 center;
    };

    // Compute normal, area, and center in one pass
    Properties computePropertiesFast() const {
        Properties p{};
        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        Vector3 c = edge1.cross(edge2);
        float len2 = c.lengthSquared();
        if (len2 > 1e-12f) {
            float invLen = 1.0f / std::sqrt(len2);
            p.normal = c * invLen;
            p.area = 0.5f * (len2 * invLen); // 0.5 * |c|, where |c| = len2 * invLen
        } else {
            p.normal = Vector3(0.0f, 0.0f, 1.0f);
            p.area = 0.0f;
        }
        p.center = (v0 + v1 + v2) * (1.0f / 3.0f);
        return p;
    }

    Vector3 v0, v1, v2;
};

/**
 * @brief Represents an oriented bounding box in 3D space
 * 
 * An OBB is defined by its center, half-extents along its local axes,
 * and orientation (stored as a 3x3 rotation matrix).
 */
class OBB {
public:
    OBB() : center(), halfExtents(1.0f, 1.0f, 1.0f), orientation() {}
    OBB(const Vector3& center_, const Vector3& halfExtents_, const Matrix3x3& orientation_)
        : center(center_), halfExtents(halfExtents_), orientation(orientation_) {}

    Vector3 center;
    Vector3 halfExtents;
    Matrix3x3 orientation;
};

/**
 * @brief Represents a capsule in 3D space
 * 
 * A capsule is defined by two points (the centers of the spherical caps)
 * and a radius. It can be thought of as a cylinder with spherical caps.
 */
class Capsule {
public:
    Capsule() : point1(), point2(), radius(1.0f) {}
    Capsule(const Vector3& point1_, const Vector3& point2_, float radius_)
        : point1(point1_), point2(point2_), radius(radius_) {}

    float getHeight() const {
        return (point2 - point1).length();
    }

    Vector3 getDirection() const {
        return (point2 - point1).normalized();
    }

    Vector3 point1, point2;
    float radius;
};

} // namespace math
} // namespace pynovage

#endif // PYNOVAGE_MATH_PRIMITIVES_HPP