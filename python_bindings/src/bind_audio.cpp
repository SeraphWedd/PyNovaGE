#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <audio/audio.hpp>
#include <audio/audio_system.hpp>
#include <audio/audio_source.hpp>

namespace py = pybind11;
namespace PNA = PyNovaGE::Audio;

void bind_audio(py::module& m) {
    auto audio_module = m.def_submodule("audio", "Audio system");

    // AudioState enum
    py::enum_<PNA::AudioState>(audio_module, "AudioState")
        .value("Stopped", PNA::AudioState::Stopped)
        .value("Playing", PNA::AudioState::Playing)
        .value("Paused", PNA::AudioState::Paused);

    // AudioSource class
    py::class_<PNA::AudioSource>(audio_module, "AudioSource")
        .def("play", &PNA::AudioSource::Play)
        .def("pause", &PNA::AudioSource::Pause)
        .def("stop", &PNA::AudioSource::Stop)
        .def("resume", &PNA::AudioSource::Resume)
        .def("set_volume", &PNA::AudioSource::SetVolume)
        .def("get_volume", &PNA::AudioSource::GetVolume)
        .def("set_pitch", &PNA::AudioSource::SetPitch)
        .def("get_pitch", &PNA::AudioSource::GetPitch)
        .def("set_looping", &PNA::AudioSource::SetLooping)
        .def("is_looping", &PNA::AudioSource::IsLooping)
        .def("set_position", &PNA::AudioSource::SetPosition)
        .def("get_state", &PNA::AudioSource::GetState)
        .def("is_playing", &PNA::AudioSource::IsPlaying)
        .def("is_paused", &PNA::AudioSource::IsPaused)
        .def("is_stopped", &PNA::AudioSource::IsStopped);

    // AudioSystem class
    py::class_<PNA::AudioSystem>(audio_module, "AudioSystem")
        .def("initialize", &PNA::AudioSystem::Initialize)
        .def("shutdown", &PNA::AudioSystem::Shutdown)
        .def("is_initialized", &PNA::AudioSystem::IsInitialized)
        .def("update", &PNA::AudioSystem::Update)
        .def("create_source", &PNA::AudioSystem::CreateSource, py::return_value_policy::take_ownership)
        .def("set_master_volume", &PNA::AudioSystem::SetMasterVolume)
        .def("get_master_volume", &PNA::AudioSystem::GetMasterVolume)
        .def("set_listener_position", &PNA::AudioSystem::SetListenerPosition)
        .def("set_listener_orientation", &PNA::AudioSystem::SetListenerOrientation)
        .def("set_listener_velocity", &PNA::AudioSystem::SetListenerVelocity)
        .def("pause_all", &PNA::AudioSystem::PauseAll)
        .def("resume_all", &PNA::AudioSystem::ResumeAll)
        .def("stop_all", &PNA::AudioSystem::StopAll);

    // Global audio system functions
    audio_module.def("initialize_audio", &PNA::InitializeAudio);
    audio_module.def("shutdown_audio", &PNA::ShutdownAudio);
    audio_module.def("get_audio_system", &PNA::GetAudioSystem, py::return_value_policy::reference);
    
    // Utility function
    audio_module.def("is_supported", [](){ return true; },
                     "Return True to indicate audio bindings are available with OpenAL");
}
