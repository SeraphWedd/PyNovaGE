"""
PyNovaGE - Python bindings for the NovaGE game engine

This module provides a pygame-like interface to the PyNovaGE game engine,
making it easy to create 2D games with high performance C++ backend.
"""

import pynovage_core
from .pygame_like import *
from .color import Color
from .surface import Surface
from .rect import Rect

# Re-export core modules for advanced usage
math = pynovage_core.math
window = pynovage_core.window  
input = pynovage_core.input
renderer = pynovage_core.renderer
physics = pynovage_core.physics
scene = pynovage_core.scene
asset = pynovage_core.asset
audio = pynovage_core.audio
particles = pynovage_core.particles

# Version info
__version__ = "0.1.0"
__author__ = "PyNovaGE Team"

# Initialize flag
_initialized = False