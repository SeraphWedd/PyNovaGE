#include "physics/collision_shapes.hpp"
#include <algorithm>
#include <cmath>

namespace PyNovaGE {
namespace Physics {

//------------------------------------------------------------------------------
// RectangleShape Implementation
//------------------------------------------------------------------------------

bool RectangleShape::intersects(const CollisionShape& other, const Vector2<float>& thisPos, const Vector2<float>& otherPos) const {
    switch (other.getType()) {
        case ShapeType::Rectangle:
            return CollisionDetection::intersects(*this, thisPos, static_cast<const RectangleShape&>(other), otherPos);
        case ShapeType::Circle:
            return CollisionDetection::intersects(*this, thisPos, static_cast<const CircleShape&>(other), otherPos);
        default:
            return false;
    }
}

AABB<float> RectangleShape::getBounds(const Vector2<float>& position) const {
    Vector2<float> min = position - half_size_;
    Vector2<float> max = position + half_size_;
    return AABB<float>(SIMD::Vector<float, 3>(min.x, min.y, 0.0f), SIMD::Vector<float, 3>(max.x, max.y, 0.0f));
}

Vector2<float> RectangleShape::getClosestPoint(const Vector2<float>& point, const Vector2<float>& position) const {
    Vector2<float> relative = point - position;
    Vector2<float> closest;
    closest.x = std::max(-half_size_.x, std::min(half_size_.x, relative.x));
    closest.y = std::max(-half_size_.y, std::min(half_size_.y, relative.y));
    return position + closest;
}

//------------------------------------------------------------------------------
// CircleShape Implementation  
//------------------------------------------------------------------------------

bool CircleShape::intersects(const CollisionShape& other, const Vector2<float>& thisPos, const Vector2<float>& otherPos) const {
    switch (other.getType()) {
        case ShapeType::Circle:
            return CollisionDetection::intersects(*this, thisPos, static_cast<const CircleShape&>(other), otherPos);
        case ShapeType::Rectangle:
            return CollisionDetection::intersects(static_cast<const RectangleShape&>(other), otherPos, *this, thisPos);
        default:
            return false;
    }
}

AABB<float> CircleShape::getBounds(const Vector2<float>& position) const {
    Vector2<float> extent(radius_, radius_);
    Vector2<float> min = position - extent;
    Vector2<float> max = position + extent;
    return AABB<float>(SIMD::Vector<float, 3>(min.x, min.y, 0.0f), SIMD::Vector<float, 3>(max.x, max.y, 0.0f));
}

Vector2<float> CircleShape::getClosestPoint(const Vector2<float>& point, const Vector2<float>& position) const {
    Vector2<float> direction = point - position;
    float distance = direction.length();
    
    if (distance <= radius_) {
        return point; // Point is inside circle
    }
    
    return position + (direction / distance) * radius_;
}

//------------------------------------------------------------------------------
// CollisionDetection Implementation
//------------------------------------------------------------------------------

bool CollisionDetection::intersects(const RectangleShape& rect1, const Vector2<float>& pos1,
                                   const RectangleShape& rect2, const Vector2<float>& pos2) {
    // Convert to SIMD AABB format and use existing optimized intersection
    auto bounds1 = rect1.getBounds(pos1);
    auto bounds2 = rect2.getBounds(pos2);
    return bounds1.intersects(bounds2);
}

bool CollisionDetection::intersects(const CircleShape& circle1, const Vector2<float>& pos1,
                                   const CircleShape& circle2, const Vector2<float>& pos2) {
    // Convert to SIMD Sphere format and use existing optimized intersection  
    Sphere<float> sphere1(SIMD::Vector<float, 3>(pos1.x, pos1.y, 0.0f), circle1.getRadius());
    Sphere<float> sphere2(SIMD::Vector<float, 3>(pos2.x, pos2.y, 0.0f), circle2.getRadius());
    return sphere1.intersects(sphere2);
}

bool CollisionDetection::intersects(const RectangleShape& rect, const Vector2<float>& rectPos,
                                   const CircleShape& circle, const Vector2<float>& circlePos) {
    // Convert to SIMD format and use existing optimized AABB-Sphere intersection
    auto rectBounds = rect.getBounds(rectPos);
    Sphere<float> sphere(SIMD::Vector<float, 3>(circlePos.x, circlePos.y, 0.0f), circle.getRadius());
    return sphere.intersects(rectBounds);
}

bool CollisionDetection::contains(const RectangleShape& rect, const Vector2<float>& rectPos, const Vector2<float>& point) {
    // Use existing SIMD AABB containment test
    auto bounds = rect.getBounds(rectPos);
    return bounds.contains(SIMD::Vector<float, 3>(point.x, point.y, 0.0f));
}

bool CollisionDetection::contains(const CircleShape& circle, const Vector2<float>& circlePos, const Vector2<float>& point) {
    // Use existing SIMD Sphere containment test
    Sphere<float> sphere(SIMD::Vector<float, 3>(circlePos.x, circlePos.y, 0.0f), circle.getRadius());
    return sphere.contains(SIMD::Vector<float, 3>(point.x, point.y, 0.0f));
}

CollisionDetection::CollisionManifold CollisionDetection::generateManifold(const CollisionShape& shape1, const Vector2<float>& pos1,
                                                                          const CollisionShape& shape2, const Vector2<float>& pos2) {
    CollisionManifold manifold;
    
    // Check if shapes intersect
    if (!shape1.intersects(shape2, pos1, pos2)) {
        return manifold; // No collision
    }
    
    manifold.hasCollision = true;
    
    // Generate manifold based on shape types
    if (shape1.getType() == ShapeType::Rectangle && shape2.getType() == ShapeType::Rectangle) {
        // Rectangle vs Rectangle
        const auto& rect1 = static_cast<const RectangleShape&>(shape1);
        const auto& rect2 = static_cast<const RectangleShape&>(shape2);
        
        Vector2<float> separation = pos2 - pos1;
        Vector2<float> overlap;
        overlap.x = rect1.getHalfSize().x + rect2.getHalfSize().x - std::abs(separation.x);
        overlap.y = rect1.getHalfSize().y + rect2.getHalfSize().y - std::abs(separation.y);
        
        if (overlap.x < overlap.y) {
            manifold.normal = separation.x > 0 ? Vector2<float>(1.0f, 0.0f) : Vector2<float>(-1.0f, 0.0f);
            manifold.penetration = overlap.x;
            manifold.contactPoint = pos1 + Vector2<float>(rect1.getHalfSize().x * (separation.x > 0 ? 1.0f : -1.0f), 0.0f);
        } else {
            manifold.normal = separation.y > 0 ? Vector2<float>(0.0f, 1.0f) : Vector2<float>(0.0f, -1.0f);
            manifold.penetration = overlap.y;
            manifold.contactPoint = pos1 + Vector2<float>(0.0f, rect1.getHalfSize().y * (separation.y > 0 ? 1.0f : -1.0f));
        }
    }
    else if (shape1.getType() == ShapeType::Circle && shape2.getType() == ShapeType::Circle) {
        // Circle vs Circle
        const auto& circle1 = static_cast<const CircleShape&>(shape1);
        const auto& circle2 = static_cast<const CircleShape&>(shape2);
        
        Vector2<float> separation = pos2 - pos1;
        float distance = separation.length();
        float radiusSum = circle1.getRadius() + circle2.getRadius();
        
        if (distance > 0.0001f) { // Avoid division by zero
            manifold.normal = separation / distance;
            manifold.penetration = radiusSum - distance;
            manifold.contactPoint = pos1 + manifold.normal * circle1.getRadius();
        } else {
            // Circles are at same position, use arbitrary normal
            manifold.normal = Vector2<float>(1.0f, 0.0f);
            manifold.penetration = radiusSum;
            manifold.contactPoint = pos1 + Vector2<float>(circle1.getRadius(), 0.0f);
        }
    }
    else if ((shape1.getType() == ShapeType::Rectangle && shape2.getType() == ShapeType::Circle) ||
             (shape1.getType() == ShapeType::Circle && shape2.getType() == ShapeType::Rectangle)) {
        // Rectangle vs Circle (or Circle vs Rectangle)
        const RectangleShape* rect;
        const CircleShape* circle;
        Vector2<float> rectPos, circlePos;
        bool flipped = false;
        
        if (shape1.getType() == ShapeType::Rectangle) {
            rect = &static_cast<const RectangleShape&>(shape1);
            circle = &static_cast<const CircleShape&>(shape2);
            rectPos = pos1;
            circlePos = pos2;
        } else {
            rect = &static_cast<const RectangleShape&>(shape2);
            circle = &static_cast<const CircleShape&>(shape1);
            rectPos = pos2;
            circlePos = pos1;
            flipped = true;
        }
        
        // Find closest point on rectangle to circle center
        Vector2<float> closestPoint = rect->getClosestPoint(circlePos, rectPos);
        Vector2<float> separation = circlePos - closestPoint;
        float distance = separation.length();
        
        if (distance < circle->getRadius()) {
            if (distance > 0.0001f) {
                manifold.normal = separation / distance;
                manifold.penetration = circle->getRadius() - distance;
            } else {
                // Circle center is exactly on rectangle, use direction from rect center
                Vector2<float> direction = circlePos - rectPos;
                float dirLength = direction.length();
                if (dirLength > 0.0001f) {
                    manifold.normal = direction / dirLength;
                } else {
                    manifold.normal = Vector2<float>(1.0f, 0.0f);
                }
                manifold.penetration = circle->getRadius();
            }
            
            manifold.contactPoint = closestPoint;
            
            if (flipped) {
                manifold.normal = -manifold.normal;
            }
        }
    }
    
    return manifold;
}

} // namespace Physics
} // namespace PyNovaGE