#pragma once

#include "rigid_body.hpp"
#include "collision_shapes.hpp"
#include "simd/geometry_ops.hpp"
#include <vector>
#include <unordered_set>
#include <memory>

namespace PyNovaGE {
namespace Physics {

/**
 * @brief Configuration for physics simulation
 */
struct PhysicsConfig {
    Vector2<float> gravity{0.0f, -9.81f};  // Default gravity pointing down
    float time_scale = 1.0f;               // Time scale multiplier
    int velocity_iterations = 8;           // Constraint solver iterations for velocity
    int position_iterations = 3;           // Constraint solver iterations for position
    float sleep_threshold = 0.5f;          // Time before bodies go to sleep
    bool enable_sleeping = true;           // Whether to use sleeping optimization
    float broad_phase_margin = 0.1f;       // Extra margin for broad-phase collision detection
};

/**
 * @brief Contact constraint for collision resolution
 */
struct Contact {
    RigidBody* body1 = nullptr;
    RigidBody* body2 = nullptr;
    CollisionDetection::CollisionManifold manifold;
    
    // Constraint solving data
    float normal_impulse = 0.0f;
    float tangent_impulse = 0.0f;
    float normal_mass = 0.0f;
    float tangent_mass = 0.0f;
    float bias = 0.0f;
    
    bool isValid() const { return body1 && body2 && manifold.hasCollision; }
};

/**
 * @brief 2D Physics World
 * 
 * Manages all rigid bodies and simulates physics using your existing SIMD collision detection.
 * Uses efficient broad-phase collision detection with SIMD AABB tests.
 */
class PhysicsWorld {
public:
    PhysicsWorld(const PhysicsConfig& config = PhysicsConfig{});
    ~PhysicsWorld() = default;

    // World configuration
    void setConfig(const PhysicsConfig& config) { config_ = config; }
    const PhysicsConfig& getConfig() const { return config_; }
    
    void setGravity(const Vector2<float>& gravity) { config_.gravity = gravity; }
    const Vector2<float>& getGravity() const { return config_.gravity; }

    // Body management
    void addBody(std::shared_ptr<RigidBody> body);
    void removeBody(std::shared_ptr<RigidBody> body);
    void removeBody(RigidBody* body);
    void clear();
    
    size_t getBodyCount() const { return bodies_.size(); }
    const std::vector<std::shared_ptr<RigidBody>>& getBodies() const { return bodies_; }

    // Physics simulation
    void step(float deltaTime);
    void setTimeScale(float scale) { config_.time_scale = scale; }
    float getTimeScale() const { return config_.time_scale; }

    // Collision queries (leveraging SIMD broad-phase)
    std::vector<RigidBody*> queryAABB(const AABB<float>& bounds) const;
    std::vector<RigidBody*> queryPoint(const Vector2<float>& point) const;
    std::vector<RigidBody*> queryShape(const CollisionShape& shape, const Vector2<float>& position) const;
    
    // Ray casting
    struct RaycastHit {
        RigidBody* body = nullptr;
        Vector2<float> point;
        Vector2<float> normal;
        float distance = 0.0f;
        bool hasHit = false;
    };
    
    RaycastHit raycast(const Vector2<float>& start, const Vector2<float>& end) const;
    std::vector<RaycastHit> raycastAll(const Vector2<float>& start, const Vector2<float>& end) const;

    // Debug and statistics
    struct PhysicsStats {
        size_t active_bodies = 0;
        size_t sleeping_bodies = 0;
        size_t contacts = 0;
        size_t broad_phase_pairs = 0;
        float step_time = 0.0f;
        float broad_phase_time = 0.0f;
        float narrow_phase_time = 0.0f;
        float solve_time = 0.0f;
    };
    
    const PhysicsStats& getStats() const { return stats_; }
    void resetStats() { stats_ = PhysicsStats{}; }

private:
    PhysicsConfig config_;
    std::vector<std::shared_ptr<RigidBody>> bodies_;
    std::vector<Contact> contacts_;
    PhysicsStats stats_;

    // Simulation steps
    void integrate(float deltaTime);
    void broadPhaseCollision();
    void narrowPhaseCollision();
    void solveConstraints(float deltaTime);
    void updateSleepingBodies(float deltaTime);
    
    // Collision detection phases
    struct BroadPhasePair {
        size_t index1;
        size_t index2;
        
        bool operator==(const BroadPhasePair& other) const {
            return (index1 == other.index1 && index2 == other.index2) ||
                   (index1 == other.index2 && index2 == other.index1);
        }
    };
    
    std::vector<BroadPhasePair> broad_phase_pairs_;
    
    // Broad-phase collision using SIMD AABB intersection
    void performBroadPhase();
    
    // Narrow-phase collision using shape-specific tests
    void performNarrowPhase();
    
    // Contact constraint solving
    void warmStartContacts();
    void solveVelocityConstraints();
    void solvePositionConstraints();
    
    // Utility methods
    void clearContacts() { contacts_.clear(); }
    bool isValidPair(const RigidBody& body1, const RigidBody& body2) const;
    
    // Performance optimization
    void updateActiveBodyList();
    std::vector<size_t> active_body_indices_;
    
    // Time accumulation for fixed time step (optional)
    float time_accumulator_ = 0.0f;
    static constexpr float FIXED_TIME_STEP = 1.0f / 60.0f; // 60 FPS physics
};

/**
 * @brief Physics World Builder for easy setup
 */
class PhysicsWorldBuilder {
public:
    PhysicsWorldBuilder& setGravity(const Vector2<float>& gravity) {
        config_.gravity = gravity;
        return *this;
    }
    
    PhysicsWorldBuilder& setIterations(int velocity, int position) {
        config_.velocity_iterations = velocity;
        config_.position_iterations = position;
        return *this;
    }
    
    PhysicsWorldBuilder& enableSleeping(bool enable) {
        config_.enable_sleeping = enable;
        return *this;
    }
    
    PhysicsWorldBuilder& setBroadPhaseMargin(float margin) {
        config_.broad_phase_margin = margin;
        return *this;
    }
    
    std::unique_ptr<PhysicsWorld> build() {
        return std::make_unique<PhysicsWorld>(config_);
    }

private:
    PhysicsConfig config_;
};

} // namespace Physics
} // namespace PyNovaGE