#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>

#include <vectors/vectors.hpp>
#include <matrices/matrices.hpp>
#include <scene/transform2d.hpp>

namespace py = pybind11;

void bind_math(py::module& m) {
    auto math_module = m.def_submodule("math", "Math utilities");
    
    // Vector2f binding
    py::class_<PyNovaGE::Vector2f>(math_module, "Vector2f")
        .def(py::init<>())
        .def(py::init<float, float>())
        .def(py::init<float>())
        .def_readwrite("x", &PyNovaGE::Vector2f::x)
        .def_readwrite("y", &PyNovaGE::Vector2f::y)
        .def_readwrite("u", &PyNovaGE::Vector2f::u) 
        .def_readwrite("v", &PyNovaGE::Vector2f::v)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * py::self)
        .def(py::self / py::self)
        .def(py::self * float())
        .def(py::self / float())
        .def(-py::self)
        .def("dot", &PyNovaGE::Vector2f::dot)
        .def("length_squared", &PyNovaGE::Vector2f::lengthSquared)
        .def("length", &PyNovaGE::Vector2f::length)
        .def("normalized", &PyNovaGE::Vector2f::normalized)
        .def("normalize", &PyNovaGE::Vector2f::normalize)
        .def("__getitem__", [](const PyNovaGE::Vector2f& v, size_t i) {
            if (i >= 2) throw py::index_error();
            return v[i];
        })
        .def("__setitem__", [](PyNovaGE::Vector2f& v, size_t i, float value) {
            if (i >= 2) throw py::index_error();
            v[i] = value;
        })
        .def("__len__", [](const PyNovaGE::Vector2f&) { return 2; })
        .def("__str__", [](const PyNovaGE::Vector2f& v) {
            return "Vector2f(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
        });
    
    // Vector2d binding
    py::class_<PyNovaGE::Vector2d>(math_module, "Vector2d")
        .def(py::init<>())
        .def(py::init<double, double>())
        .def(py::init<double>())
        .def_readwrite("x", &PyNovaGE::Vector2d::x)
        .def_readwrite("y", &PyNovaGE::Vector2d::y)
        .def_readwrite("u", &PyNovaGE::Vector2d::u) 
        .def_readwrite("v", &PyNovaGE::Vector2d::v)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * py::self)
        .def(py::self / py::self)
        .def(py::self * double())
        .def(py::self / double())
        .def(-py::self)
        .def("dot", &PyNovaGE::Vector2d::dot)
        .def("length_squared", &PyNovaGE::Vector2d::lengthSquared)
        .def("length", &PyNovaGE::Vector2d::length)
        .def("normalized", &PyNovaGE::Vector2d::normalized)
        .def("normalize", &PyNovaGE::Vector2d::normalize)
        .def("__getitem__", [](const PyNovaGE::Vector2d& v, size_t i) {
            if (i >= 2) throw py::index_error();
            return v[i];
        })
        .def("__setitem__", [](PyNovaGE::Vector2d& v, size_t i, double value) {
            if (i >= 2) throw py::index_error();
            v[i] = value;
        })
        .def("__len__", [](const PyNovaGE::Vector2d&) { return 2; })
        .def("__str__", [](const PyNovaGE::Vector2d& v) {
            return "Vector2d(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
        });
    
    // Vector2i binding
    py::class_<PyNovaGE::Vector2i>(math_module, "Vector2i")
        .def(py::init<>())
        .def(py::init<int, int>())
        .def(py::init<int>())
        .def_readwrite("x", &PyNovaGE::Vector2i::x)
        .def_readwrite("y", &PyNovaGE::Vector2i::y)
        .def_readwrite("u", &PyNovaGE::Vector2i::u) 
        .def_readwrite("v", &PyNovaGE::Vector2i::v)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * py::self)
        .def(py::self / py::self)
        .def(py::self * int())
        .def(py::self / int())
        .def(-py::self)
        .def("dot", &PyNovaGE::Vector2i::dot)
        .def("length_squared", &PyNovaGE::Vector2i::lengthSquared)
        .def("__getitem__", [](const PyNovaGE::Vector2i& v, size_t i) {
            if (i >= 2) throw py::index_error();
            return v[i];
        })
        .def("__setitem__", [](PyNovaGE::Vector2i& v, size_t i, int value) {
            if (i >= 2) throw py::index_error();
            v[i] = value;
        })
        .def("__len__", [](const PyNovaGE::Vector2i&) { return 2; })
        .def("__str__", [](const PyNovaGE::Vector2i& v) {
            return "Vector2i(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
        });
    
    // Add Vector2f as the default Vector2 
    math_module.attr("Vector2") = math_module.attr("Vector2f");
    
    // Vector3 binding (similar pattern but I'll make it more compact)
    py::class_<PyNovaGE::Vector3f>(math_module, "Vector3f")
        .def(py::init<>())
        .def(py::init<float, float, float>())
        .def(py::init<float>())
        .def_readwrite("x", &PyNovaGE::Vector3f::x)
        .def_readwrite("y", &PyNovaGE::Vector3f::y)
        .def_readwrite("z", &PyNovaGE::Vector3f::z)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * py::self)
        .def(py::self / py::self)
        .def(py::self * float())
        .def(py::self / float())
        .def(-py::self)
        .def("dot", &PyNovaGE::Vector3f::dot)
        .def("cross", &PyNovaGE::Vector3f::cross)
        .def("length_squared", &PyNovaGE::Vector3f::lengthSquared)
        .def("length", &PyNovaGE::Vector3f::length)
        .def("normalized", &PyNovaGE::Vector3f::normalized)
        .def("normalize", &PyNovaGE::Vector3f::normalize)
        .def("__getitem__", [](const PyNovaGE::Vector3f& v, size_t i) {
            if (i >= 3) throw py::index_error();
            return v[i];
        })
        .def("__setitem__", [](PyNovaGE::Vector3f& v, size_t i, float value) {
            if (i >= 3) throw py::index_error();
            v[i] = value;
        })
        .def("__len__", [](const PyNovaGE::Vector3f&) { return 3; })
        .def("__str__", [](const PyNovaGE::Vector3f& v) {
            return "Vector3f(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
        });
    
    math_module.attr("Vector3") = math_module.attr("Vector3f");
    
    // Vector4 binding  
    py::class_<PyNovaGE::Vector4f>(math_module, "Vector4f")
        .def(py::init<>())
        .def(py::init<float, float, float, float>())
        .def(py::init<float>())
        .def_readwrite("x", &PyNovaGE::Vector4f::x)
        .def_readwrite("y", &PyNovaGE::Vector4f::y)
        .def_readwrite("z", &PyNovaGE::Vector4f::z)
        .def_readwrite("w", &PyNovaGE::Vector4f::w)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * py::self)
        .def(py::self / py::self)
        .def(py::self * float())
        .def(py::self / float())
        .def(-py::self)
        .def("dot", &PyNovaGE::Vector4f::dot)
        .def("length_squared", &PyNovaGE::Vector4f::lengthSquared)
        .def("length", &PyNovaGE::Vector4f::length)
        .def("normalized", &PyNovaGE::Vector4f::normalized)
        .def("normalize", &PyNovaGE::Vector4f::normalize)
        .def("__getitem__", [](const PyNovaGE::Vector4f& v, size_t i) {
            if (i >= 4) throw py::index_error();
            return v[i];
        })
        .def("__setitem__", [](PyNovaGE::Vector4f& v, size_t i, float value) {
            if (i >= 4) throw py::index_error();
            v[i] = value;
        })
        .def("__len__", [](const PyNovaGE::Vector4f&) { return 4; })
        .def("__str__", [](const PyNovaGE::Vector4f& v) {
            return "Vector4f(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ", " + std::to_string(v.w) + ")";
        });
    
    math_module.attr("Vector4") = math_module.attr("Vector4f");
    
    // Transform2D binding
    py::class_<PyNovaGE::Scene::Transform2D>(math_module, "Transform2D")
        .def(py::init<>())
        .def(py::init<const PyNovaGE::Vector2f&, float, const PyNovaGE::Vector2f&>(),
             py::arg("position"), py::arg("rotation") = 0.0f, py::arg("scale") = PyNovaGE::Vector2f(1.0f, 1.0f))
        // Local transform properties
        .def("set_position", &PyNovaGE::Scene::Transform2D::SetPosition)
        .def("get_position", &PyNovaGE::Scene::Transform2D::GetPosition, py::return_value_policy::reference_internal)
        .def("set_rotation", &PyNovaGE::Scene::Transform2D::SetRotation)
        .def("get_rotation", &PyNovaGE::Scene::Transform2D::GetRotation)
        .def("set_scale", &PyNovaGE::Scene::Transform2D::SetScale)
        .def("get_scale", &PyNovaGE::Scene::Transform2D::GetScale, py::return_value_policy::reference_internal)
        // Transform operations
        .def("translate", &PyNovaGE::Scene::Transform2D::Translate)
        .def("rotate", &PyNovaGE::Scene::Transform2D::Rotate)
        .def("scale", py::overload_cast<const PyNovaGE::Vector2f&>(&PyNovaGE::Scene::Transform2D::Scale))
        .def("scale", py::overload_cast<float>(&PyNovaGE::Scene::Transform2D::Scale))
        // World transform properties  
        .def("get_world_position", &PyNovaGE::Scene::Transform2D::GetWorldPosition)
        .def("get_world_rotation", &PyNovaGE::Scene::Transform2D::GetWorldRotation)
        .def("get_world_scale", &PyNovaGE::Scene::Transform2D::GetWorldScale)
        // Point transformation
        .def("transform_point", &PyNovaGE::Scene::Transform2D::TransformPoint)
        .def("transform_direction", &PyNovaGE::Scene::Transform2D::TransformDirection)
        .def("inverse_transform_point", &PyNovaGE::Scene::Transform2D::InverseTransformPoint)
        .def("inverse_transform_direction", &PyNovaGE::Scene::Transform2D::InverseTransformDirection)
        // Reset and comparison
        .def("reset", &PyNovaGE::Scene::Transform2D::Reset)
        .def(py::self == py::self)
        .def(py::self != py::self);
    
    // Python properties for more Pythonic access
    math_module.attr("Transform2D").attr("position") = py::cpp_function([](PyNovaGE::Scene::Transform2D& t) -> PyNovaGE::Vector2f& {
        return const_cast<PyNovaGE::Vector2f&>(t.GetPosition());
    }, py::return_value_policy::reference_internal);
}