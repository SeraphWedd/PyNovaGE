#include "physics_world.hpp"
#include <algorithm>
#include <unordered_set>

namespace PyNovaGE {
namespace Physics {

PhysicsWorld::PhysicsWorld(const PhysicsConfig& config) 
    : config_(config) {
}

void PhysicsWorld::addRigidBody(std::shared_ptr<RigidBody> body) {
    if (body && std::find(rigid_bodies_.begin(), rigid_bodies_.end(), body) == rigid_bodies_.end()) {
        rigid_bodies_.push_back(body);
    }
}

void PhysicsWorld::removeRigidBody(std::shared_ptr<RigidBody> body) {
    auto it = std::find(rigid_bodies_.begin(), rigid_bodies_.end(), body);
    if (it != rigid_bodies_.end()) {
        rigid_bodies_.erase(it);
    }
}

void PhysicsWorld::step(float deltaTime) {
    if (deltaTime <= 0.0f) return;
    
    // Clamp deltaTime to prevent instability
    deltaTime = std::min(deltaTime, config_.maxTimeStep);
    
    // Accumulate time for fixed timestep
    accumulated_time_ += deltaTime;
    
    while (accumulated_time_ >= config_.timeStep) {
        stepSimulation(config_.timeStep);
        accumulated_time_ -= config_.timeStep;
    }
}

void PhysicsWorld::stepSimulation(float dt) {
    // Apply gravity to dynamic bodies
    applyGravity(dt);
    
    // Integrate forces to velocities
    integrateForces(dt);
    
    // Broad phase collision detection
    auto collisionPairs = broadPhaseCollision();
    
    // Narrow phase collision detection and manifold generation
    std::vector<CollisionManifold> manifolds;
    for (const auto& pair : collisionPairs) {
        if (pair.first->isAwake() || pair.second->isAwake()) {
            auto manifold = narrowPhaseCollision(*pair.first, *pair.second);
            if (manifold.hasCollision) {
                manifolds.push_back(manifold);
            }
        }
    }
    
    // Solve constraints (collision resolution)
    for (int i = 0; i < config_.velocityIterations; ++i) {
        for (auto& manifold : manifolds) {
            solveVelocityConstraints(manifold);
        }
    }
    
    // Integrate velocities to positions
    integrateVelocities(dt);
    
    // Position correction
    for (int i = 0; i < config_.positionIterations; ++i) {
        for (const auto& manifold : manifolds) {
            solvePositionConstraints(manifold);
        }
    }
    
    // Update sleep states
    updateSleepStates(dt);
}

void PhysicsWorld::applyGravity(float dt) {
    for (auto& body : rigid_bodies_) {
        if (body->getType() == BodyType::Dynamic && body->isAwake()) {
            Vector2<float> gravityForce = config_.gravity * body->getMass();
            body->applyForce(gravityForce);
        }
    }
}

void PhysicsWorld::integrateForces(float dt) {
    for (auto& body : rigid_bodies_) {
        if (body->getType() == BodyType::Dynamic && body->isAwake()) {
            // Integration is handled by RigidBody::integrate
            // This is where we could add additional force integration if needed
        }
    }
}

void PhysicsWorld::integrateVelocities(float dt) {
    for (auto& body : rigid_bodies_) {
        if (body->getType() == BodyType::Dynamic && body->isAwake()) {
            body->integrate(dt);
        }
    }
}

void PhysicsWorld::updateSleepStates(float dt) {
    // Sleep state updates are handled by each RigidBody during integration
    // This is where we could add additional sleep logic if needed
}

std::vector<std::pair<std::shared_ptr<RigidBody>, std::shared_ptr<RigidBody>>>
PhysicsWorld::broadPhaseCollision() {
    std::vector<std::pair<std::shared_ptr<RigidBody>, std::shared_ptr<RigidBody>>> pairs;
    
    // Simple O(n²) broad phase - can be optimized with spatial partitioning later
    for (size_t i = 0; i < rigid_bodies_.size(); ++i) {
        for (size_t j = i + 1; j < rigid_bodies_.size(); ++j) {
            auto& bodyA = rigid_bodies_[i];
            auto& bodyB = rigid_bodies_[j];
            
            // Skip if both bodies are static
            if (bodyA->getType() == BodyType::Static && bodyB->getType() == BodyType::Static) {
                continue;
            }
            
            // Skip if both bodies are sleeping
            if (!bodyA->isAwake() && !bodyB->isAwake()) {
                continue;
            }
            
            // Check AABB overlap
            auto boundsA = bodyA->getWorldBounds();
            auto boundsB = bodyB->getWorldBounds();
            
            if (boundsA.intersects(boundsB)) {
                pairs.emplace_back(bodyA, bodyB);
            }
        }
    }
    
    return pairs;
}

CollisionDetection::CollisionManifold 
PhysicsWorld::narrowPhaseCollision(const RigidBody& bodyA, const RigidBody& bodyB) {
    auto shapeA = bodyA.getCollisionShape();
    auto shapeB = bodyB.getCollisionShape();
    
    if (!shapeA || !shapeB) {
        return CollisionDetection::CollisionManifold(); // No collision
    }
    
    return CollisionDetection::generateManifold(*shapeA, bodyA.getPosition(), 
                                               *shapeB, bodyB.getPosition());
}

void PhysicsWorld::solveVelocityConstraints(CollisionDetection::CollisionManifold& manifold) {
    // This is a simplified constraint solver
    // A more robust implementation would use iterative methods
    
    // For now, this method doesn't do additional work since 
    // collision resolution is handled in RigidBody::resolveCollision
}

void PhysicsWorld::solvePositionConstraints(const CollisionDetection::CollisionManifold& manifold) {
    // Position correction is also handled in RigidBody::resolveCollision
    // This method could implement additional position-based corrections
}

// Query methods
std::vector<std::shared_ptr<RigidBody>> 
PhysicsWorld::queryRegion(const AABB<float>& region) const {
    std::vector<std::shared_ptr<RigidBody>> results;
    
    for (const auto& body : rigid_bodies_) {
        if (body->getWorldBounds().intersects(region)) {
            results.push_back(body);
        }
    }
    
    return results;
}

std::shared_ptr<RigidBody> 
PhysicsWorld::queryPoint(const Vector2<float>& point) const {
    for (const auto& body : rigid_bodies_) {
        auto shape = body->getCollisionShape();
        if (shape && shape->contains(point, body->getPosition())) {
            return body;
        }
    }
    
    return nullptr;
}

PhysicsWorld::RaycastResult 
PhysicsWorld::raycast(const Vector2<float>& origin, const Vector2<float>& direction, float maxDistance) const {
    RaycastResult result;
    result.hit = false;
    result.distance = maxDistance;
    
    Vector2<float> normalizedDir = direction;
    float dirLength = direction.length();
    if (dirLength > 0.0001f) {
        normalizedDir = direction / dirLength;
    } else {
        return result; // Invalid direction
    }
    
    for (const auto& body : rigid_bodies_) {
        // Simple raycast implementation - can be optimized
        // For now, we'll check against AABB and then do detailed shape testing
        auto bounds = body->getWorldBounds();
        
        // Create a ray in 3D for SIMD operations
        SIMD::Ray<float> ray(SIMD::Vector<float, 3>(origin.x, origin.y, 0.0f), 
                           SIMD::Vector<float, 3>(normalizedDir.x, normalizedDir.y, 0.0f));
        
        float t;
        if (ray.intersects(bounds, t) && t < result.distance) {
            // More detailed intersection test with actual shape could go here
            result.hit = true;
            result.distance = t;
            result.point = origin + normalizedDir * t;
            result.body = body;
            
            // Simple normal calculation (could be improved)
            Vector2<float> center = body->getPosition();
            Vector2<float> toHit = result.point - center;
            if (toHit.length() > 0.0001f) {
                result.normal = toHit.normalized();
            } else {
                result.normal = Vector2<float>(1.0f, 0.0f);
            }
        }
    }
    
    return result;
}

void PhysicsWorld::setGravity(const Vector2<float>& gravity) {
    config_.gravity = gravity;
    
    // Wake up all dynamic bodies when gravity changes
    for (auto& body : rigid_bodies_) {
        if (body->getType() == BodyType::Dynamic) {
            body->setAwake(true);
        }
    }
}

size_t PhysicsWorld::getBodyCount() const {
    return rigid_bodies_.size();
}

size_t PhysicsWorld::getActiveBodyCount() const {
    size_t count = 0;
    for (const auto& body : rigid_bodies_) {
        if (body->isAwake()) {
            ++count;
        }
    }
    return count;
}

void PhysicsWorld::setTimeStep(float timeStep) {
    if (timeStep > 0.0f) {
        config_.timeStep = timeStep;
    }
}

void PhysicsWorld::setVelocityIterations(int iterations) {
    config_.velocityIterations = std::max(1, iterations);
}

void PhysicsWorld::setPositionIterations(int iterations) {
    config_.positionIterations = std::max(0, iterations);
}

void PhysicsWorld::clearForces() {
    for (auto& body : rigid_bodies_) {
        body->clearForces();
    }
}

} // namespace Physics
} // namespace PyNovaGE