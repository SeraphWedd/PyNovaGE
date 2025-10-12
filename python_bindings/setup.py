#!/usr/bin/env python3
"""
Setup script for PyNovaGE Python bindings.

This script builds the Python bindings for the PyNovaGE game engine using pybind11.
"""

from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup, Extension
import pybind11
import os
import sys
import platform

# Define the version
VERSION = "0.1.0"

# Get the project root directory (parent of python_bindings)
project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Define include directories
include_dirs = [
    # pybind11 includes
    pybind11.get_cmake_dir() + "/../include",
    
    # Engine include directories (relative to project root)
    os.path.join(project_root, "engine/core/window/include"),
    os.path.join(project_root, "engine/core/renderer/include"), 
    os.path.join(project_root, "engine/core/physics/include"),
    os.path.join(project_root, "engine/core/scene/include"),
    os.path.join(project_root, "engine/systems/asset/include"),
    os.path.join(project_root, "engine/systems/audio/include"),
    os.path.join(project_root, "engine/foundation/math/include"),
    os.path.join(project_root, "engine/foundation/core/include"),
    os.path.join(project_root, "engine/foundation/memory/include"),
]

# Filter to existing directories only
include_dirs = [path for path in include_dirs if os.path.exists(path)]

# Define library directories and libraries
library_dirs = []
libraries = []

# Platform-specific settings
if platform.system() == "Windows":
    # Add built engine libraries
    build_dir = os.path.join(project_root, "build")
    library_dirs.extend([
        os.path.join(build_dir, "engine/core/window/Release"),
        os.path.join(build_dir, "engine/core/renderer/Release"),
        os.path.join(build_dir, "engine/core/physics/Release"),
        os.path.join(build_dir, "engine/core/scene/Release"),
        os.path.join(build_dir, "engine/systems/asset/Release"),
        os.path.join(build_dir, "engine/foundation/memory/Release"),
        os.path.join(build_dir, "_deps/glfw-build/src/Release"),
    ])
    libraries.extend([
        "window", "renderer", "physics", "scene", "asset", "stb_libs", "memory", "glfw3", "opengl32",
        # Windows system libraries required by GLFW
        "user32", "gdi32", "shell32", "ole32", "oleaut32", "imm32", "winmm", "version"
    ])
elif platform.system() == "Linux":
    libraries.extend(["GL", "glfw"])
elif platform.system() == "Darwin":  # macOS
    libraries.extend(["OpenGL", "glfw"])

# Define source files
source_files = [
    "src/main.cpp",
    "src/bind_math.cpp",
    "src/bind_window.cpp",
    "src/bind_input.cpp", 
    "src/bind_renderer.cpp",
    "src/bind_physics.cpp",
    "src/bind_scene.cpp",
    "src/bind_asset.cpp",
    "src/bind_audio.cpp",
]

# Define the extension module
ext_modules = [
    Pybind11Extension(
        "pynovage_core",
        source_files,
        include_dirs=include_dirs,
        libraries=libraries,
        library_dirs=library_dirs,
        cxx_std=17,
        define_macros=[("VERSION_INFO", f'"{VERSION}"')],
    )
]

# Read the README file for long description
def read_readme():
    readme_path = os.path.join(os.path.dirname(__file__), "README.md")
    if os.path.exists(readme_path):
        with open(readme_path, "r", encoding="utf-8") as f:
            return f.read()
    return "PyNovaGE - Python bindings for the NovaGE game engine"

# Setup configuration
setup(
    name="pynovage",
    version=VERSION,
    author="PyNovaGE Team",
    author_email="",
    description="Python bindings for the PyNovaGE game engine",
    long_description=read_readme(),
    long_description_content_type="text/markdown",
    url="https://github.com/your-repo/pynovage",
    
    # Python package
    packages=["pynovage"],
    package_dir={"": "."},
    
    # C++ extension module
    ext_modules=ext_modules,
    
    # Build requirements
    setup_requires=["pybind11>=2.6.0"],
    install_requires=[],
    
    # Python version requirement
    python_requires=">=3.7",
    
    # Classifiers
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Topic :: Games/Entertainment",
        "Topic :: Multimedia :: Graphics",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8", 
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: C++",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX :: Linux",
        "Operating System :: MacOS",
    ],
    
    # Keywords
    keywords="gamedev game-engine graphics rendering physics 2d 3d pygame",
    
    # Entry points (if needed)
    entry_points={
        "console_scripts": [
            # "pynovage-tool=pynovage.tools:main",
        ],
    },
    
    # Additional data files
    package_data={
        "pynovage": ["*.py"],
    },
    
    # Include additional files
    include_package_data=True,
    
    # Zip safe
    zip_safe=False,
    
    # Custom build command
    cmdclass={"build_ext": build_ext},
)

# Build instructions for users
if __name__ == "__main__":
    print("PyNovaGE Python Bindings Setup")
    print("==============================")
    print(f"Version: {VERSION}")
    print(f"Python: {sys.version}")
    print(f"Platform: {platform.system()} {platform.machine()}")
    print()
    print("To build and install:")
    print("  python setup.py build_ext --inplace  # Build in place")
    print("  python setup.py install              # Install system-wide")
    print("  pip install -e .                     # Install in development mode")
    print()
    print("Requirements:")
    print("  - pybind11 >= 2.6.0")
    print("  - CMake >= 3.16 (for building the C++ engine)")
    print("  - Visual Studio 2019+ (Windows) or GCC 8+ (Linux)")
    print("  - GLFW3 and OpenGL development libraries")
    print()