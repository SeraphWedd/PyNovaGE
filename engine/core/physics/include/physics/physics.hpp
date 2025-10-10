#pragma once

/**
 * @file physics.hpp
 * @brief Main PyNovaGE Physics System Header
 * 
 * This header provides access to the complete 2D physics system built on top of 
 * PyNovaGE's SIMD-optimized math foundation. The physics system includes:
 * 
 * - Collision shapes (Rectangle, Circle)
 * - Rigid body dynamics
 * - Physics world simulation
 * - SIMD-accelerated broad-phase collision detection
 * - Constraint-based collision resolution
 * 
 * Example usage:
 * @code
 * using namespace PyNovaGE::Physics;
 * 
 * // Create physics world
 * auto world = PhysicsWorldBuilder()
 *     .setGravity({0.0f, -9.81f})
 *     .setIterations(8, 3)
 *     .build();
 * 
 * // Create a dynamic box
 * auto boxShape = std::make_shared<RectangleShape>(Vector2<float>(2.0f, 2.0f));
 * auto boxBody = std::make_shared<RigidBody>(boxShape, BodyType::Dynamic);
 * boxBody->setPosition({0.0f, 10.0f});
 * world->addBody(boxBody);
 * 
 * // Create a static ground
 * auto groundShape = std::make_shared<RectangleShape>(Vector2<float>(20.0f, 1.0f));
 * auto groundBody = std::make_shared<RigidBody>(groundShape, BodyType::Static);
 * groundBody->setPosition({0.0f, -5.0f});
 * world->addBody(groundBody);
 * 
 * // Simulate physics
 * float deltaTime = 1.0f / 60.0f;
 * world->step(deltaTime);
 * @endcode
 */

// Core physics components
#include "collision_shapes.hpp"
#include "rigid_body.hpp"
#include "physics_world.hpp"

// Convenience namespace alias for common usage
namespace PyNovaGE {
    /**
     * @brief Convenience namespace for physics system
     * 
     * This allows users to write `using namespace PyNovaGE::Physics;` 
     * to access all physics functionality without fully qualifying names.
     */
    namespace Physics {
        // Re-export commonly used types for convenience
        using Vec2f = Vector2<float>;
        using AABBf = SIMD::AABB<float>;
        
        // Common material presets
        namespace Materials {
            const Material METAL{7.8f, 0.1f, 0.3f, 0.01f};      // Dense, low bounce, medium friction
            const Material RUBBER{1.2f, 0.9f, 0.8f, 0.05f};     // Medium density, high bounce, high friction  
            const Material ICE{0.9f, 0.1f, 0.02f, 0.001f};      // Light, low bounce, very low friction
            const Material WOOD{0.6f, 0.3f, 0.5f, 0.02f};       // Light, medium bounce, medium friction
            const Material STONE{2.7f, 0.2f, 0.7f, 0.01f};      // Heavy, low bounce, high friction
        }
        
        // Common gravity presets
        namespace Gravity {
            const Vec2f EARTH{0.0f, -9.81f};      // Earth gravity
            const Vec2f MOON{0.0f, -1.62f};       // Moon gravity
            const Vec2f MARS{0.0f, -3.71f};       // Mars gravity  
            const Vec2f ZERO{0.0f, 0.0f};         // No gravity (space)
        }
        
        /**
         * @brief Helper function to create common shapes
         */
        namespace Shapes {
            inline std::shared_ptr<RectangleShape> box(float width, float height) {
                return std::make_shared<RectangleShape>(Vec2f(width, height));
            }
            
            inline std::shared_ptr<RectangleShape> square(float size) {
                return std::make_shared<RectangleShape>(Vec2f(size, size));
            }
            
            inline std::shared_ptr<CircleShape> circle(float radius) {
                return std::make_shared<CircleShape>(radius);
            }
        }
        
        /**
         * @brief Helper function to create common bodies
         */
        namespace Bodies {
            inline std::shared_ptr<RigidBody> dynamicBox(float width, float height, const Material& material = Material{}) {
                auto shape = Shapes::box(width, height);
                auto body = std::make_shared<RigidBody>(shape, BodyType::Dynamic);
                body->setMaterial(material);
                return body;
            }
            
            inline std::shared_ptr<RigidBody> staticBox(float width, float height) {
                auto shape = Shapes::box(width, height);
                return std::make_shared<RigidBody>(shape, BodyType::Static);
            }
            
            inline std::shared_ptr<RigidBody> dynamicCircle(float radius, const Material& material = Material{}) {
                auto shape = Shapes::circle(radius);
                auto body = std::make_shared<RigidBody>(shape, BodyType::Dynamic);
                body->setMaterial(material);
                return body;
            }
            
            inline std::shared_ptr<RigidBody> staticCircle(float radius) {
                auto shape = Shapes::circle(radius);
                return std::make_shared<RigidBody>(shape, BodyType::Static);
            }
        }
    }
}

/**
 * @brief Performance notes
 * 
 * The physics system is designed to leverage PyNovaGE's SIMD optimizations:
 * 
 * - Broad-phase collision detection uses SIMD AABB intersection tests
 * - Point-in-shape queries use SIMD containment tests  
 * - Vector operations leverage existing SIMD vector math
 * - Memory layout is optimized for cache-friendly access patterns
 * 
 * For best performance:
 * - Use appropriate body sleeping to reduce unnecessary computations
 * - Keep the number of dynamic bodies reasonable (< 1000 for typical games)
 * - Use static bodies for level geometry that never moves
 * - Consider kinematic bodies for platforms moved by scripts/animation
 */