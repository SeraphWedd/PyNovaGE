#include "physics/physics_world.hpp"
#include <chrono>
#include <algorithm>
#include <unordered_set>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4723)  // potential divide by 0
#endif

namespace PyNovaGE {
namespace Physics {

// Safe division function to avoid compiler warnings about divide-by-zero
static Vector2<float> safeDivide(const Vector2<float>& vector, float divisor) {
    if (divisor > 1e-6f || divisor < -1e-6f) {
        return Vector2<float>(vector.x / divisor, vector.y / divisor);
    }
    return vector;
}

PhysicsWorld::PhysicsWorld(const PhysicsConfig& config) 
    : config_(config) {
}

void PhysicsWorld::addBody(std::shared_ptr<RigidBody> body) {
    if (body && std::find(bodies_.begin(), bodies_.end(), body) == bodies_.end()) {
        bodies_.push_back(body);
        updateActiveBodyList();
    }
}

void PhysicsWorld::removeBody(std::shared_ptr<RigidBody> body) {
    auto it = std::find(bodies_.begin(), bodies_.end(), body);
    if (it != bodies_.end()) {
        bodies_.erase(it);
        updateActiveBodyList();
    }
}

void PhysicsWorld::removeBody(RigidBody* body) {
    auto it = std::find_if(bodies_.begin(), bodies_.end(),
        [body](const std::shared_ptr<RigidBody>& ptr) {
            return ptr && ptr.get() == body;
        });
    if (it != bodies_.end()) {
        bodies_.erase(it);
        updateActiveBodyList();
    }
}

void PhysicsWorld::clear() {
    bodies_.clear();
    contacts_.clear();
    active_body_indices_.clear();
    broad_phase_pairs_.clear();
}

void PhysicsWorld::step(float deltaTime) {
    if (deltaTime <= 0.0f) return;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Scale time
    deltaTime *= config_.time_scale;
    
    // Fixed timestep accumulation
    time_accumulator_ += deltaTime;
    
    // Process fixed timesteps
    while (time_accumulator_ >= FIXED_TIME_STEP) {
        integrate(FIXED_TIME_STEP);
        broadPhaseCollision();
        narrowPhaseCollision();
        solveConstraints(FIXED_TIME_STEP);
        updateSleepingBodies(FIXED_TIME_STEP);
        
        time_accumulator_ -= FIXED_TIME_STEP;
    }
    
    // Update statistics
    auto end = std::chrono::high_resolution_clock::now();
    stats_.step_time = std::chrono::duration<float>(end - start).count();
    stats_.active_bodies = 0;
    stats_.sleeping_bodies = 0;
    
    for (const auto& body : bodies_) {
        if (body->isAwake()) {
            stats_.active_bodies++;
        } else {
            stats_.sleeping_bodies++;
        }
    }
}

// Physics simulation implementation methods
void PhysicsWorld::integrate(float deltaTime) {
    // Apply gravity to dynamic bodies first
    for (auto& body : bodies_) {
        if (body->getBodyType() == BodyType::Dynamic && body->isAwake()) {
            Vector2<float> gravityForce = config_.gravity * body->getMass();
            body->applyForce(gravityForce);
        }
    }
    
    // Integrate all dynamic bodies
    for (auto& body : bodies_) {
        if (body->getBodyType() == BodyType::Dynamic && body->isAwake()) {
            body->integrate(deltaTime);
        }
    }
}

void PhysicsWorld::broadPhaseCollision() {
    auto start = std::chrono::high_resolution_clock::now();
    
    broad_phase_pairs_.clear();
    
    // Simple O(n²) broad phase using SIMD AABB tests
    for (size_t i = 0; i < bodies_.size(); ++i) {
        for (size_t j = i + 1; j < bodies_.size(); ++j) {
            auto& bodyA = bodies_[i];
            auto& bodyB = bodies_[j];
            
            if (!isValidPair(*bodyA, *bodyB)) {
                continue;
            }
            
            // Check AABB overlap using existing SIMD implementation
            auto boundsA = bodyA->getWorldBounds();
            auto boundsB = bodyB->getWorldBounds();
            
            if (boundsA.intersects(boundsB)) {
                broad_phase_pairs_.push_back({i, j});
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    stats_.broad_phase_time = std::chrono::duration<float>(end - start).count();
    stats_.broad_phase_pairs = broad_phase_pairs_.size();
}

void PhysicsWorld::narrowPhaseCollision() {
    auto start = std::chrono::high_resolution_clock::now();
    
    clearContacts();
    
    // Generate collision manifolds for each broad-phase pair
    for (const auto& pair : broad_phase_pairs_) {
        auto& bodyA = bodies_[pair.index1];
        auto& bodyB = bodies_[pair.index2];
        
        // Generate collision manifold
        auto manifold = CollisionDetection::generateManifold(
            bodyA->getCollisionShape(), bodyA->getPosition(),
            bodyB->getCollisionShape(), bodyB->getPosition()
        );
        
        if (manifold.hasCollision) {
            Contact contact;
            contact.body1 = bodyA.get();
            contact.body2 = bodyB.get();
            contact.manifold = manifold;
            
            // Initialize contact constraint data
            float totalInverseMass = bodyA->getInverseMass() + bodyB->getInverseMass();
            contact.normal_mass = (totalInverseMass > 0.0f) ? 1.0f / totalInverseMass : 0.0f;
            
            // Calculate effective friction mass (simplified - no rotation for now)
            contact.tangent_mass = contact.normal_mass;
            
            // Calculate bias for position correction (Baumgarte stabilization)
            const float BIAS_FACTOR = 0.2f;
            const float BIAS_THRESHOLD = 0.01f;
            contact.bias = std::max(0.0f, BIAS_FACTOR * (manifold.penetration - BIAS_THRESHOLD) / FIXED_TIME_STEP);
            
            // Reset accumulated impulses (will be warmed started if contact persists)
            contact.normal_impulse = 0.0f;
            contact.tangent_impulse = 0.0f;
            
            contacts_.push_back(contact);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    stats_.narrow_phase_time = std::chrono::duration<float>(end - start).count();
    stats_.contacts = contacts_.size();
}

void PhysicsWorld::solveConstraints(float) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Warm start contacts
    warmStartContacts();
    
    // Solve velocity constraints
    for (int i = 0; i < config_.velocity_iterations; ++i) {
        solveVelocityConstraints();
    }
    
    // Solve position constraints
    for (int i = 0; i < config_.position_iterations; ++i) {
        solvePositionConstraints();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    stats_.solve_time = std::chrono::duration<float>(end - start).count();
}

void PhysicsWorld::updateSleepingBodies(float) {
    // Sleep state updates are handled by each RigidBody during integration
    // This method ensures that bodies wake up when they should
    for (auto& body : bodies_) {
        if (body->getBodyType() == BodyType::Dynamic) {
            // Bodies are woken up during collision resolution
            // Additional sleep logic could go here if needed
        }
    }
}

// Query methods
std::vector<RigidBody*> PhysicsWorld::queryAABB(const AABB<float>& bounds) const {
    std::vector<RigidBody*> results;
    
    for (const auto& body : bodies_) {
        if (body->getWorldBounds().intersects(bounds)) {
            results.push_back(body.get());
        }
    }
    
    return results;
}

std::vector<RigidBody*> PhysicsWorld::queryPoint(const Vector2<float>& point) const {
    std::vector<RigidBody*> results;
    
    for (const auto& body : bodies_) {
        auto bounds = body->getWorldBounds();
        SIMD::Vector<float, 3> point3d(point.x, point.y, 0.0f);
        
        if (bounds.contains(point3d)) {
            // Additional precise point-in-shape test could go here
            results.push_back(body.get());
        }
    }
    
    return results;
}

std::vector<RigidBody*> PhysicsWorld::queryShape(const CollisionShape& shape, const Vector2<float>& position) const {
    std::vector<RigidBody*> results;
    
    for (const auto& body : bodies_) {
        if (body->getCollisionShape().intersects(shape, body->getPosition(), position)) {
            results.push_back(body.get());
        }
    }
    
    return results;
}

PhysicsWorld::RaycastHit PhysicsWorld::raycast(const Vector2<float>& start, const Vector2<float>& end) const {
    RaycastHit result;
    result.hasHit = false;
    
    Vector2<float> direction = end - start;
    float maxDistance = direction.length();
    
    if (maxDistance < 0.0001f) {
        return result; // Invalid ray
    }
    
    direction = safeDivide(direction, maxDistance);
    result.distance = maxDistance;
    
    // Simple raycast against all body AABBs
    for (const auto& body : bodies_) {
        auto bounds = body->getWorldBounds();
        
        // Create 3D ray for SIMD operations
        SIMD::Ray<float> ray(
            SIMD::Vector<float, 3>(start.x, start.y, 0.0f),
            SIMD::Vector<float, 3>(direction.x, direction.y, 0.0f)
        );
        
        float t;
        if (ray.intersects(bounds, t) && t < result.distance && t >= 0.0f) {
            result.hasHit = true;
            result.distance = t;
            result.point = start + direction * t;
            result.body = body.get();
            
            // Simple normal calculation (can be improved)
            Vector2<float> center = body->getPosition();
            Vector2<float> toHit = result.point - center;
            float len = toHit.length();
            result.normal = safeDivide(toHit, len);
            if (result.normal.lengthSquared() < 1e-12f) {
                result.normal = Vector2<float>(1.0f, 0.0f);
            }
        }
    }
    
    return result;
}

std::vector<PhysicsWorld::RaycastHit> PhysicsWorld::raycastAll(const Vector2<float>& start, const Vector2<float>& end) const {
    std::vector<RaycastHit> results;
    
    Vector2<float> direction = end - start;
    float maxDistance = direction.length();
    
    if (maxDistance < 0.0001f) {
        return results; // Invalid ray
    }
    
    direction = safeDivide(direction, maxDistance);
    
    // Raycast against all bodies
    for (const auto& body : bodies_) {
        auto bounds = body->getWorldBounds();
        
        // Create 3D ray for SIMD operations
        SIMD::Ray<float> ray(
            SIMD::Vector<float, 3>(start.x, start.y, 0.0f),
            SIMD::Vector<float, 3>(direction.x, direction.y, 0.0f)
        );
        
        float t;
        if (ray.intersects(bounds, t) && t <= maxDistance && t >= 0.0f) {
            RaycastHit hit;
            hit.hasHit = true;
            hit.distance = t;
            hit.point = start + direction * t;
            hit.body = body.get();
            
            // Simple normal calculation
            Vector2<float> center = body->getPosition();
            Vector2<float> toHit = hit.point - center;
            float len = toHit.length();
            hit.normal = safeDivide(toHit, len);
            if (hit.normal.lengthSquared() < 1e-12f) {
                hit.normal = Vector2<float>(1.0f, 0.0f);
            }
            
            results.push_back(hit);
        }
    }
    
    // Sort by distance
    std::sort(results.begin(), results.end(),
        [](const RaycastHit& a, const RaycastHit& b) {
            return a.distance < b.distance;
        });
    
    return results;
}

// Private implementation methods
void PhysicsWorld::performBroadPhase() {
    broadPhaseCollision();
}

void PhysicsWorld::performNarrowPhase() {
    narrowPhaseCollision();
}

void PhysicsWorld::warmStartContacts() {
    // Apply cached impulses from previous frame for better stability
    for (auto& contact : contacts_) {
        if (!contact.isValid()) continue;
        
        RigidBody* bodyA = contact.body1;
        RigidBody* bodyB = contact.body2;
        
        // Apply normal impulse
        Vector2<float> normalImpulse = contact.manifold.normal * contact.normal_impulse;
        if (bodyA->isDynamic()) {
            bodyA->setLinearVelocity(bodyA->getLinearVelocity() - normalImpulse * bodyA->getInverseMass());
        }
        if (bodyB->isDynamic()) {
            bodyB->setLinearVelocity(bodyB->getLinearVelocity() + normalImpulse * bodyB->getInverseMass());
        }
        
        // Apply tangent impulse (friction)
        Vector2<float> tangent = Vector2<float>(-contact.manifold.normal.y, contact.manifold.normal.x);
        Vector2<float> tangentImpulse = tangent * contact.tangent_impulse;
        if (bodyA->isDynamic()) {
            bodyA->setLinearVelocity(bodyA->getLinearVelocity() - tangentImpulse * bodyA->getInverseMass());
        }
        if (bodyB->isDynamic()) {
            bodyB->setLinearVelocity(bodyB->getLinearVelocity() + tangentImpulse * bodyB->getInverseMass());
        }
    }
}

void PhysicsWorld::solveVelocityConstraints() {
    // Iterative impulse-based constraint solving
    for (auto& contact : contacts_) {
        if (!contact.isValid()) continue;
        
        RigidBody* bodyA = contact.body1;
        RigidBody* bodyB = contact.body2;
        
        // Calculate relative velocity
        Vector2<float> relativeVelocity = bodyB->getLinearVelocity() - bodyA->getLinearVelocity();
        float contactVelocity = relativeVelocity.dot(contact.manifold.normal);
        
        // Calculate desired velocity change
        float restitution = std::min(bodyA->getMaterial().restitution, bodyB->getMaterial().restitution);
        float desiredDeltaVelocity = -contactVelocity * (1.0f + restitution) + contact.bias;
        
        // Calculate impulse magnitude
        float deltaImpulse = desiredDeltaVelocity * contact.normal_mass;
        
        // Clamp accumulated impulse (non-penetration constraint)
        float oldNormalImpulse = contact.normal_impulse;
        contact.normal_impulse = std::max(0.0f, contact.normal_impulse + deltaImpulse);
        deltaImpulse = contact.normal_impulse - oldNormalImpulse;
        
        // Apply normal impulse
        Vector2<float> impulse = contact.manifold.normal * deltaImpulse;
        if (bodyA->isDynamic()) {
            bodyA->setLinearVelocity(bodyA->getLinearVelocity() - impulse * bodyA->getInverseMass());
            bodyA->setAwake(true);
        }
        if (bodyB->isDynamic()) {
            bodyB->setLinearVelocity(bodyB->getLinearVelocity() + impulse * bodyB->getInverseMass());
            bodyB->setAwake(true);
        }
        
        // Friction constraint
        Vector2<float> tangent = Vector2<float>(-contact.manifold.normal.y, contact.manifold.normal.x);
        relativeVelocity = bodyB->getLinearVelocity() - bodyA->getLinearVelocity();
        float tangentVelocity = relativeVelocity.dot(tangent);
        
        float friction = std::sqrt(bodyA->getMaterial().friction * bodyB->getMaterial().friction);
        float maxFriction = friction * contact.normal_impulse;
        
        float tangentImpulseDelta = -tangentVelocity * contact.tangent_mass;
        float oldTangentImpulse = contact.tangent_impulse;
        contact.tangent_impulse = std::max(-maxFriction, std::min(maxFriction, contact.tangent_impulse + tangentImpulseDelta));
        tangentImpulseDelta = contact.tangent_impulse - oldTangentImpulse;
        
        // Apply friction impulse
        Vector2<float> frictionImpulse = tangent * tangentImpulseDelta;
        if (bodyA->isDynamic()) {
            bodyA->setLinearVelocity(bodyA->getLinearVelocity() - frictionImpulse * bodyA->getInverseMass());
        }
        if (bodyB->isDynamic()) {
            bodyB->setLinearVelocity(bodyB->getLinearVelocity() + frictionImpulse * bodyB->getInverseMass());
        }
    }
}

void PhysicsWorld::solvePositionConstraints() {
    // Position-based constraint solving to prevent sinking
    const float POSITION_CORRECTION_PERCENT = 0.4f;
    const float POSITION_CORRECTION_THRESHOLD = 0.01f;
    
    for (auto& contact : contacts_) {
        if (!contact.isValid()) continue;
        
        // Only correct if penetration is significant
        if (contact.manifold.penetration <= POSITION_CORRECTION_THRESHOLD) {
            continue;
        }
        
        RigidBody* bodyA = contact.body1;
        RigidBody* bodyB = contact.body2;
        
        // Calculate mass-weighted correction
        float totalInverseMass = bodyA->getInverseMass() + bodyB->getInverseMass();
        if (totalInverseMass <= 0.0001f) continue; // Both bodies are static or nearly infinite mass
        
        float correctionMagnitude = totalInverseMass > 0.0001f ? (contact.manifold.penetration * POSITION_CORRECTION_PERCENT) / totalInverseMass : 0.0f;
        Vector2<float> correction = contact.manifold.normal * correctionMagnitude;
        
        // Apply position correction
        if (bodyA->isDynamic()) {
            Vector2<float> bodyACorrection = correction * (-bodyA->getInverseMass());
            bodyA->setPosition(bodyA->getPosition() + bodyACorrection);
        }
        if (bodyB->isDynamic()) {
            Vector2<float> bodyBCorrection = correction * bodyB->getInverseMass();
            bodyB->setPosition(bodyB->getPosition() + bodyBCorrection);
        }
    }
}

bool PhysicsWorld::isValidPair(const RigidBody& body1, const RigidBody& body2) const {
    // Skip if both bodies are static
    if (body1.isStatic() && body2.isStatic()) {
        return false;
    }
    
    // Skip if both bodies are sleeping
    if (!body1.isAwake() && !body2.isAwake()) {
        return false;
    }
    
    // Skip if either body is not active
    if (!body1.isActive() || !body2.isActive()) {
        return false;
    }
    
    return true;
}

void PhysicsWorld::updateActiveBodyList() {
    active_body_indices_.clear();
    
    for (size_t i = 0; i < bodies_.size(); ++i) {
        if (bodies_[i]->isAwake() && bodies_[i]->isActive()) {
            active_body_indices_.push_back(i);
        }
    }
}

} // namespace Physics
} // namespace PyNovaGE

#ifdef _MSC_VER
#pragma warning(pop)
#endif
