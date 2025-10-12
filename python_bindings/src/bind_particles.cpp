#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>

#include <particles/particle_system.hpp>
#include <particles/particle.hpp>
#include <particles/particle_emitter.hpp>

namespace py = pybind11;

void bind_particles(py::module_ &m) {
    auto particles_module = m.def_submodule("particles", "Particle system module");
    
    // Particle struct
    py::class_<PyNovaGE::Particles::Particle>(particles_module, "Particle")
        .def(py::init<>())
        .def_readwrite("position", &PyNovaGE::Particles::Particle::position)
        .def_readwrite("velocity", &PyNovaGE::Particles::Particle::velocity)
        .def_readwrite("rotation", &PyNovaGE::Particles::Particle::rotation)
        .def_readwrite("angular_velocity", &PyNovaGE::Particles::Particle::angular_velocity)
        .def_readwrite("size", &PyNovaGE::Particles::Particle::size)
        .def_readwrite("color", &PyNovaGE::Particles::Particle::color)
        .def_readwrite("lifetime", &PyNovaGE::Particles::Particle::lifetime)
        .def_readwrite("age", &PyNovaGE::Particles::Particle::age)
        .def_readwrite("acceleration", &PyNovaGE::Particles::Particle::acceleration)
        .def_readwrite("mass", &PyNovaGE::Particles::Particle::mass)
        .def_readwrite("drag", &PyNovaGE::Particles::Particle::drag)
        .def_readwrite("size_over_time", &PyNovaGE::Particles::Particle::size_over_time)
        .def_readwrite("color_over_time", &PyNovaGE::Particles::Particle::color_over_time)
        .def_readwrite("uv_rect", &PyNovaGE::Particles::Particle::uv_rect)
        .def("is_alive", &PyNovaGE::Particles::Particle::IsAlive)
        .def("get_normalized_age", &PyNovaGE::Particles::Particle::GetNormalizedAge)
        .def("update", &PyNovaGE::Particles::Particle::Update)
        .def("reset", &PyNovaGE::Particles::Particle::Reset)
        .def("apply_force", &PyNovaGE::Particles::Particle::ApplyForce);
    
    // ParticleInitData struct
    py::class_<PyNovaGE::Particles::ParticleInitData>(particles_module, "ParticleInitData")
        .def(py::init<>())
        .def_readwrite("position", &PyNovaGE::Particles::ParticleInitData::position)
        .def_readwrite("velocity", &PyNovaGE::Particles::ParticleInitData::velocity)
        .def_readwrite("rotation", &PyNovaGE::Particles::ParticleInitData::rotation)
        .def_readwrite("angular_velocity", &PyNovaGE::Particles::ParticleInitData::angular_velocity)
        .def_readwrite("size", &PyNovaGE::Particles::ParticleInitData::size)
        .def_readwrite("color", &PyNovaGE::Particles::ParticleInitData::color)
        .def_readwrite("lifetime", &PyNovaGE::Particles::ParticleInitData::lifetime)
        .def_readwrite("acceleration", &PyNovaGE::Particles::ParticleInitData::acceleration)
        .def_readwrite("mass", &PyNovaGE::Particles::ParticleInitData::mass)
        .def_readwrite("drag", &PyNovaGE::Particles::ParticleInitData::drag)
        .def_readwrite("size_over_time", &PyNovaGE::Particles::ParticleInitData::size_over_time)
        .def_readwrite("color_over_time", &PyNovaGE::Particles::ParticleInitData::color_over_time)
        .def_readwrite("uv_rect", &PyNovaGE::Particles::ParticleInitData::uv_rect);
    
    // EmissionShape enum
    py::enum_<PyNovaGE::Particles::EmissionShape>(particles_module, "EmissionShape")
        .value("Point", PyNovaGE::Particles::EmissionShape::Point)
        .value("Circle", PyNovaGE::Particles::EmissionShape::Circle)
        .value("Box", PyNovaGE::Particles::EmissionShape::Box)
        .value("Line", PyNovaGE::Particles::EmissionShape::Line)
        .export_values();
    
    // EmissionBurst struct
    py::class_<PyNovaGE::Particles::EmissionBurst>(particles_module, "EmissionBurst")
        .def(py::init<>())
        .def(py::init<float, int, float>())
        .def_readwrite("time", &PyNovaGE::Particles::EmissionBurst::time)
        .def_readwrite("count", &PyNovaGE::Particles::EmissionBurst::count)
        .def_readwrite("probability", &PyNovaGE::Particles::EmissionBurst::probability);
    
    // EmitterConfig struct  
    py::class_<PyNovaGE::Particles::EmitterConfig>(particles_module, "EmitterConfig")
        .def(py::init<>())
        .def_readwrite("position", &PyNovaGE::Particles::EmitterConfig::position)
        .def_readwrite("emission_rate", &PyNovaGE::Particles::EmitterConfig::emission_rate)
        .def_readwrite("auto_emit", &PyNovaGE::Particles::EmitterConfig::auto_emit)
        .def_readwrite("looping", &PyNovaGE::Particles::EmitterConfig::looping)
        .def_readwrite("duration", &PyNovaGE::Particles::EmitterConfig::duration)
        .def_readwrite("shape", &PyNovaGE::Particles::EmitterConfig::shape)
        .def_readwrite("shape_data", &PyNovaGE::Particles::EmitterConfig::shape_data)
        .def_readwrite("bursts", &PyNovaGE::Particles::EmitterConfig::bursts)
        .def_readwrite("initial", &PyNovaGE::Particles::EmitterConfig::initial)
        .def_readwrite("animation", &PyNovaGE::Particles::EmitterConfig::animation)
        .def_readwrite("gravity", &PyNovaGE::Particles::EmitterConfig::gravity)
        .def_readwrite("uv_rect", &PyNovaGE::Particles::EmitterConfig::uv_rect);
    
    // ParticleEmitter class
    py::class_<PyNovaGE::Particles::ParticleEmitter, std::shared_ptr<PyNovaGE::Particles::ParticleEmitter>>(particles_module, "ParticleEmitter")
        .def(py::init<const PyNovaGE::Particles::EmitterConfig&>())
        .def("set_config", &PyNovaGE::Particles::ParticleEmitter::SetConfig)
        .def("get_config", &PyNovaGE::Particles::ParticleEmitter::GetConfig, py::return_value_policy::reference_internal)
        .def("set_emit_callback", &PyNovaGE::Particles::ParticleEmitter::SetEmitCallback)
        .def("start", &PyNovaGE::Particles::ParticleEmitter::Start)
        .def("stop", &PyNovaGE::Particles::ParticleEmitter::Stop)
        .def("set_paused", &PyNovaGE::Particles::ParticleEmitter::SetPaused)
        .def("is_active", &PyNovaGE::Particles::ParticleEmitter::IsActive)
        .def("is_looping", &PyNovaGE::Particles::ParticleEmitter::IsLooping)
        .def("update", &PyNovaGE::Particles::ParticleEmitter::Update)
        .def("emit_particle", &PyNovaGE::Particles::ParticleEmitter::EmitParticle)
        .def("emit_burst", &PyNovaGE::Particles::ParticleEmitter::EmitBurst)
        .def("set_position", &PyNovaGE::Particles::ParticleEmitter::SetPosition)
        .def("get_position", &PyNovaGE::Particles::ParticleEmitter::GetPosition, py::return_value_policy::reference_internal)
        .def("reset", &PyNovaGE::Particles::ParticleEmitter::Reset)
        .def("get_emission_time", &PyNovaGE::Particles::ParticleEmitter::GetEmissionTime);
    
    // ParticleSystemStats struct
    py::class_<PyNovaGE::Particles::ParticleSystemStats>(particles_module, "ParticleSystemStats")
        .def(py::init<>())
        .def_readwrite("active_particles", &PyNovaGE::Particles::ParticleSystemStats::active_particles)
        .def_readwrite("total_particles_spawned", &PyNovaGE::Particles::ParticleSystemStats::total_particles_spawned)
        .def_readwrite("peak_active_particles", &PyNovaGE::Particles::ParticleSystemStats::peak_active_particles)
        .def_readwrite("active_emitters", &PyNovaGE::Particles::ParticleSystemStats::active_emitters)
        .def_readwrite("pool_size", &PyNovaGE::Particles::ParticleSystemStats::pool_size)
        .def_readwrite("pool_free", &PyNovaGE::Particles::ParticleSystemStats::pool_free)
        .def_readwrite("update_time_ms", &PyNovaGE::Particles::ParticleSystemStats::update_time_ms)
        .def_readwrite("render_time_ms", &PyNovaGE::Particles::ParticleSystemStats::render_time_ms)
        .def("reset", &PyNovaGE::Particles::ParticleSystemStats::Reset);
    
    // ParticleSystemConfig struct
    py::class_<PyNovaGE::Particles::ParticleSystemConfig>(particles_module, "ParticleSystemConfig")
        .def(py::init<>())
        .def_readwrite("max_particles", &PyNovaGE::Particles::ParticleSystemConfig::max_particles)
        .def_readwrite("enable_sorting", &PyNovaGE::Particles::ParticleSystemConfig::enable_sorting)
        .def_readwrite("enable_culling", &PyNovaGE::Particles::ParticleSystemConfig::enable_culling)
        .def_readwrite("culling_bounds", &PyNovaGE::Particles::ParticleSystemConfig::culling_bounds);
    
    // ParticleSystem class
    py::class_<PyNovaGE::Particles::ParticleSystem>(particles_module, "ParticleSystem")
        .def(py::init<const PyNovaGE::Particles::ParticleSystemConfig&>())
        .def("initialize", &PyNovaGE::Particles::ParticleSystem::Initialize)
        .def("shutdown", &PyNovaGE::Particles::ParticleSystem::Shutdown)
        .def("is_initialized", &PyNovaGE::Particles::ParticleSystem::IsInitialized)
        .def("update", &PyNovaGE::Particles::ParticleSystem::Update)
        .def("render", &PyNovaGE::Particles::ParticleSystem::Render)
        .def("create_emitter", &PyNovaGE::Particles::ParticleSystem::CreateEmitter)
        .def("remove_emitter", &PyNovaGE::Particles::ParticleSystem::RemoveEmitter)
        .def("clear_emitters", &PyNovaGE::Particles::ParticleSystem::ClearEmitters)
        .def("spawn_particle", &PyNovaGE::Particles::ParticleSystem::SpawnParticle, py::return_value_policy::reference_internal)
        .def("destroy_particle", &PyNovaGE::Particles::ParticleSystem::DestroyParticle)
        .def("clear_particles", &PyNovaGE::Particles::ParticleSystem::ClearParticles)
        .def("apply_global_force", &PyNovaGE::Particles::ParticleSystem::ApplyGlobalForce)
        .def("apply_radial_force", &PyNovaGE::Particles::ParticleSystem::ApplyRadialForce)
        .def("set_config", &PyNovaGE::Particles::ParticleSystem::SetConfig)
        .def("get_config", &PyNovaGE::Particles::ParticleSystem::GetConfig, py::return_value_policy::reference_internal)
        .def("get_stats", &PyNovaGE::Particles::ParticleSystem::GetStats, py::return_value_policy::reference_internal)
        .def("reset_stats", &PyNovaGE::Particles::ParticleSystem::ResetStats)
        .def("get_active_particle_count", &PyNovaGE::Particles::ParticleSystem::GetActiveParticleCount)
        .def("get_active_emitter_count", &PyNovaGE::Particles::ParticleSystem::GetActiveEmitterCount)
        .def("get_max_particles", &PyNovaGE::Particles::ParticleSystem::GetMaxParticles)
        .def("is_pool_full", &PyNovaGE::Particles::ParticleSystem::IsPoolFull);
}