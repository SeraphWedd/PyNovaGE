#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <physics/physics.hpp>
#include <physics/physics_world.hpp>
#include <physics/rigid_body.hpp>
#include <physics/collision_shapes.hpp>
#include <vectors/vectors.hpp>

namespace py = pybind11;

using namespace PyNovaGE;
using namespace PyNovaGE::Physics;

void bind_physics(py::module& m) {
    auto physics = m.def_submodule("physics", "Physics simulation system");

    // BodyType enum
    py::enum_<BodyType>(physics, "BodyType")
        .value("Static", BodyType::Static)
        .value("Kinematic", BodyType::Kinematic)
        .value("Dynamic", BodyType::Dynamic)
        .export_values();

    // Material struct
    py::class_<Material>(physics, "Material")
        .def(py::init<>())
        .def_readwrite("density", &Material::density)
        .def_readwrite("restitution", &Material::restitution)
        .def_readwrite("friction", &Material::friction)
        .def_readwrite("drag", &Material::drag);

    // CollisionShape base (polymorphic)
    py::class_<CollisionShape, std::shared_ptr<CollisionShape>>(physics, "CollisionShape")
        .def("get_type", &CollisionShape::getType);

    // RectangleShape
    py::class_<RectangleShape, CollisionShape, std::shared_ptr<RectangleShape>>(physics, "RectangleShape")
        .def(py::init<const Vector2<float>&>(), py::arg("size"))
        .def("get_half_size", &RectangleShape::getHalfSize, py::return_value_policy::reference_internal)
        .def("get_size", &RectangleShape::getSize);

    // CircleShape
    py::class_<CircleShape, CollisionShape, std::shared_ptr<CircleShape>>(physics, "CircleShape")
        .def(py::init<float>(), py::arg("radius"))
        .def("get_radius", &CircleShape::getRadius);

    // RigidBody
    py::class_<RigidBody, std::shared_ptr<RigidBody>>(physics, "RigidBody")
        .def(py::init<std::shared_ptr<CollisionShape>, BodyType>(), py::arg("shape"), py::arg("type") = BodyType::Dynamic)
        // Transform
        .def("set_position", &RigidBody::setPosition)
        .def("get_position", &RigidBody::getPosition, py::return_value_policy::reference_internal)
        .def("set_rotation", &RigidBody::setRotation)
        .def("get_rotation", &RigidBody::getRotation)
        // Motion
        .def("set_linear_velocity", &RigidBody::setLinearVelocity)
        .def("get_linear_velocity", &RigidBody::getLinearVelocity, py::return_value_policy::reference_internal)
        .def("set_angular_velocity", &RigidBody::setAngularVelocity)
        .def("get_angular_velocity", &RigidBody::getAngularVelocity)
        // Mass/inertia
        .def("set_mass", &RigidBody::setMass)
        .def("get_mass", &RigidBody::getMass)
        .def("get_inverse_mass", &RigidBody::getInverseMass)
        .def("set_inertia", &RigidBody::setInertia)
        .def("get_inertia", &RigidBody::getInertia)
        .def("get_inverse_inertia", &RigidBody::getInverseInertia)
        // Forces/impulses
        .def("apply_force", &RigidBody::applyForce)
        .def("apply_force_at_point", &RigidBody::applyForceAtPoint)
        .def("apply_impulse", &RigidBody::applyImpulse)
        .def("apply_angular_impulse", &RigidBody::applyAngularImpulse)
        .def("clear_forces", &RigidBody::clearForces)
        // Material/shape
        .def("set_material", &RigidBody::setMaterial)
        .def("get_material", &RigidBody::getMaterial, py::return_value_policy::reference_internal)
        .def("get_collision_shape", &RigidBody::getCollisionShapePtr)
        // State
        .def("set_active", &RigidBody::setActive)
        .def("is_active", &RigidBody::isActive)
        .def("set_awake", &RigidBody::setAwake)
        .def("is_awake", &RigidBody::isAwake)
        // Utility
        .def("is_static", &RigidBody::isStatic)
        .def("is_kinematic", &RigidBody::isKinematic)
        .def("is_dynamic", &RigidBody::isDynamic);

    // PhysicsConfig
    py::class_<PhysicsConfig>(physics, "PhysicsConfig")
        .def(py::init<>())
        .def_readwrite("gravity", &PhysicsConfig::gravity)
        .def_readwrite("time_scale", &PhysicsConfig::time_scale)
        .def_readwrite("velocity_iterations", &PhysicsConfig::velocity_iterations)
        .def_readwrite("position_iterations", &PhysicsConfig::position_iterations)
        .def_readwrite("sleep_threshold", &PhysicsConfig::sleep_threshold)
        .def_readwrite("enable_sleeping", &PhysicsConfig::enable_sleeping)
        .def_readwrite("broad_phase_margin", &PhysicsConfig::broad_phase_margin);

    // PhysicsWorld nested types
    py::class_<PhysicsWorld::RaycastHit>(physics, "RaycastHit")
        .def(py::init<>())
        .def_readwrite("body", &PhysicsWorld::RaycastHit::body)
        .def_readwrite("point", &PhysicsWorld::RaycastHit::point)
        .def_readwrite("normal", &PhysicsWorld::RaycastHit::normal)
        .def_readwrite("distance", &PhysicsWorld::RaycastHit::distance)
        .def_readwrite("hasHit", &PhysicsWorld::RaycastHit::hasHit);

    py::class_<PhysicsWorld::PhysicsStats>(physics, "PhysicsStats")
        .def(py::init<>())
        .def_readonly("active_bodies", &PhysicsWorld::PhysicsStats::active_bodies)
        .def_readonly("sleeping_bodies", &PhysicsWorld::PhysicsStats::sleeping_bodies)
        .def_readonly("contacts", &PhysicsWorld::PhysicsStats::contacts)
        .def_readonly("broad_phase_pairs", &PhysicsWorld::PhysicsStats::broad_phase_pairs)
        .def_readonly("step_time", &PhysicsWorld::PhysicsStats::step_time)
        .def_readonly("broad_phase_time", &PhysicsWorld::PhysicsStats::broad_phase_time)
        .def_readonly("narrow_phase_time", &PhysicsWorld::PhysicsStats::narrow_phase_time)
        .def_readonly("solve_time", &PhysicsWorld::PhysicsStats::solve_time);

    // PhysicsWorld
    py::class_<PhysicsWorld>(physics, "PhysicsWorld")
        .def(py::init<const PhysicsConfig&>(), py::arg("config") = PhysicsConfig{})
        // Config and gravity
        .def("set_config", &PhysicsWorld::setConfig)
        .def("get_config", &PhysicsWorld::getConfig, py::return_value_policy::reference_internal)
        .def("set_gravity", &PhysicsWorld::setGravity)
        .def("get_gravity", &PhysicsWorld::getGravity, py::return_value_policy::reference_internal)
        // Bodies
        .def("add_body", &PhysicsWorld::addBody)
        .def("remove_body", py::overload_cast<std::shared_ptr<RigidBody>>(&PhysicsWorld::removeBody))
        .def("clear", &PhysicsWorld::clear)
        .def("get_body_count", &PhysicsWorld::getBodyCount)
        .def("get_bodies", [](PhysicsWorld& w) {
            // Return a Python list of shared_ptr<RigidBody>
            py::list lst;
            for (auto& b : w.getBodies()) lst.append(b);
            return lst;
        })
        // Simulation
        .def("step", &PhysicsWorld::step, py::arg("delta_time"))
        .def("set_time_scale", &PhysicsWorld::setTimeScale)
        .def("get_time_scale", &PhysicsWorld::getTimeScale)
        // Ray casting (AABB/shape queries can be added later)
        .def("raycast", &PhysicsWorld::raycast)
        .def("raycast_all", &PhysicsWorld::raycastAll)
        // Stats
        .def("get_stats", &PhysicsWorld::getStats, py::return_value_policy::reference_internal)
        .def("reset_stats", &PhysicsWorld::resetStats);

    // PhysicsWorldBuilder
    py::class_<PhysicsWorldBuilder>(physics, "PhysicsWorldBuilder")
        .def(py::init<>())
        .def("set_gravity", &PhysicsWorldBuilder::setGravity, py::return_value_policy::reference_internal)
        .def("set_iterations", &PhysicsWorldBuilder::setIterations, py::return_value_policy::reference_internal)
        .def("enable_sleeping", &PhysicsWorldBuilder::enableSleeping, py::return_value_policy::reference_internal)
        .def("set_broad_phase_margin", &PhysicsWorldBuilder::setBroadPhaseMargin, py::return_value_policy::reference_internal)
        .def("build", &PhysicsWorldBuilder::build);

    // Convenience factory functions
    physics.def("box_shape", [](float w, float h) { return std::make_shared<RectangleShape>(Vector2<float>(w, h)); },
                py::arg("width"), py::arg("height"));
    physics.def("circle_shape", [](float r) { return std::make_shared<CircleShape>(r); }, py::arg("radius"));

    physics.def("dynamic_box", [](float w, float h, const Material& m) {
        auto shape = std::make_shared<RectangleShape>(Vector2<float>(w, h));
        auto body = std::make_shared<RigidBody>(shape, BodyType::Dynamic);
        body->setMaterial(m);
        return body;
    }, py::arg("width"), py::arg("height"), py::arg("material") = Material{});

    physics.def("static_box", [](float w, float h) {
        auto shape = std::make_shared<RectangleShape>(Vector2<float>(w, h));
        return std::make_shared<RigidBody>(shape, BodyType::Static);
    }, py::arg("width"), py::arg("height"));

    physics.def("dynamic_circle", [](float r, const Material& m) {
        auto shape = std::make_shared<CircleShape>(r);
        auto body = std::make_shared<RigidBody>(shape, BodyType::Dynamic);
        body->setMaterial(m);
        return body;
    }, py::arg("radius"), py::arg("material") = Material{});

    physics.def("static_circle", [](float r) {
        auto shape = std::make_shared<CircleShape>(r);
        return std::make_shared<RigidBody>(shape, BodyType::Static);
    }, py::arg("radius"));

    // Preset materials and gravity
    auto materials = physics.def_submodule("materials", "Common material presets");
    materials.attr("METAL") = Materials::METAL;
    materials.attr("RUBBER") = Materials::RUBBER;
    materials.attr("ICE") = Materials::ICE;
    materials.attr("WOOD") = Materials::WOOD;
    materials.attr("STONE") = Materials::STONE;

    auto gravity = physics.def_submodule("gravity", "Common gravity presets");
    gravity.attr("EARTH") = Gravity::EARTH;
    gravity.attr("MOON") = Gravity::MOON;
    gravity.attr("MARS") = Gravity::MARS;
    gravity.attr("ZERO") = Gravity::ZERO;
}
