#pragma once

#include "simd/geometry_ops.hpp"
#include "vectors/vectors.hpp"
#include <memory>
#include <variant>

namespace PyNovaGE {
namespace Physics {

using namespace PyNovaGE;
using namespace PyNovaGE::SIMD;

// Forward declarations
class RigidBody;

/**
 * @brief Shape types for 2D collision detection
 */
enum class ShapeType {
    Rectangle,  // AABB (Axis-Aligned Bounding Box)
    Circle,     // Sphere in 2D
    Capsule     // Rounded rectangle (for future expansion)
};

/**
 * @brief Base class for collision shapes
 */
class CollisionShape {
public:
    CollisionShape(ShapeType type) : type_(type) {}
    virtual ~CollisionShape() = default;

    ShapeType getType() const { return type_; }
    
    // Pure virtual methods for collision detection
    virtual bool intersects(const CollisionShape& other, const Vector2<float>& thisPos, const Vector2<float>& otherPos) const = 0;
    virtual AABB<float> getBounds(const Vector2<float>& position) const = 0;
    virtual Vector2<float> getClosestPoint(const Vector2<float>& point, const Vector2<float>& position) const = 0;
    
    // Mass properties
    virtual float getArea() const = 0;
    virtual float getInertia(float mass) const = 0;

protected:
    ShapeType type_;
};

/**
 * @brief Rectangle collision shape (AABB)
 * Uses the existing SIMD AABB implementation from geometry_ops.hpp
 */
class RectangleShape : public CollisionShape {
public:
    RectangleShape(const Vector2<float>& size) 
        : CollisionShape(ShapeType::Rectangle), half_size_(size * 0.5f) {}
    
    const Vector2<float>& getHalfSize() const { return half_size_; }
    Vector2<float> getSize() const { return half_size_ * 2.0f; }
    
    bool intersects(const CollisionShape& other, const Vector2<float>& thisPos, const Vector2<float>& otherPos) const override;
    AABB<float> getBounds(const Vector2<float>& position) const override;
    Vector2<float> getClosestPoint(const Vector2<float>& point, const Vector2<float>& position) const override;
    
    float getArea() const override { return (half_size_.x * 2.0f) * (half_size_.y * 2.0f); }
    float getInertia(float mass) const override {
        Vector2<float> size = getSize();
        return mass * (size.x * size.x + size.y * size.y) / 12.0f;
    }

private:
    Vector2<float> half_size_;
};

/**
 * @brief Circle collision shape
 * Uses the existing SIMD Sphere implementation adapted for 2D
 */
class CircleShape : public CollisionShape {
public:
    CircleShape(float radius) 
        : CollisionShape(ShapeType::Circle), radius_(radius) {}
    
    float getRadius() const { return radius_; }
    
    bool intersects(const CollisionShape& other, const Vector2<float>& thisPos, const Vector2<float>& otherPos) const override;
    AABB<float> getBounds(const Vector2<float>& position) const override;
    Vector2<float> getClosestPoint(const Vector2<float>& point, const Vector2<float>& position) const override;
    
    float getArea() const override { return 3.14159265359f * radius_ * radius_; }
    float getInertia(float mass) const override { return 0.5f * mass * radius_ * radius_; }

private:
    float radius_;
};

/**
 * @brief Collision detection utilities
 */
namespace CollisionDetection {
    
    // AABB vs AABB collision (leveraging existing SIMD implementation)
    bool intersects(const RectangleShape& rect1, const Vector2<float>& pos1,
                   const RectangleShape& rect2, const Vector2<float>& pos2);
    
    // Circle vs Circle collision (leveraging existing SIMD Sphere implementation)  
    bool intersects(const CircleShape& circle1, const Vector2<float>& pos1,
                   const CircleShape& circle2, const Vector2<float>& pos2);
    
    // AABB vs Circle collision
    bool intersects(const RectangleShape& rect, const Vector2<float>& rectPos,
                   const CircleShape& circle, const Vector2<float>& circlePos);
    
    // Point-in-shape tests (leveraging existing SIMD containment)
    bool contains(const RectangleShape& rect, const Vector2<float>& rectPos, const Vector2<float>& point);
    bool contains(const CircleShape& circle, const Vector2<float>& circlePos, const Vector2<float>& point);
    
    // Collision manifold generation for physics response
    struct CollisionManifold {
        bool hasCollision = false;
        Vector2<float> normal;      // Collision normal (from body1 to body2)
        float penetration = 0.0f;   // Penetration depth
        Vector2<float> contactPoint; // Contact point in world space
    };
    
    CollisionManifold generateManifold(const CollisionShape& shape1, const Vector2<float>& pos1,
                                     const CollisionShape& shape2, const Vector2<float>& pos2);

} // namespace CollisionDetection

} // namespace Physics
} // namespace PyNovaGE