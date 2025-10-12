"""
Pygame-like interface for PyNovaGE.

This module provides a familiar pygame-style API for easy game development.
"""

import pynovage_core
import time
from .color import Color
from .surface import Surface
from .rect import Rect

# Global state
_window = None
_window_system_guard = None
_renderer_guard = None
_input_manager = None
_clock_last_time = 0
_screen_surface = None

# Constants similar to pygame
QUIT = "QUIT"
KEYDOWN = "KEYDOWN" 
KEYUP = "KEYUP"
MOUSEBUTTONDOWN = "MOUSEBUTTONDOWN"
MOUSEBUTTONUP = "MOUSEBUTTONUP"
MOUSEMOTION = "MOUSEMOTION"

# Key constants (expose from core)
K_SPACE = pynovage_core.input.Key.SPACE
K_ESCAPE = pynovage_core.input.Key.ESCAPE
K_RETURN = pynovage_core.input.Key.ENTER
K_LEFT = pynovage_core.input.Key.LEFT
K_RIGHT = pynovage_core.input.Key.RIGHT
K_UP = pynovage_core.input.Key.UP
K_DOWN = pynovage_core.input.Key.DOWN
K_a = pynovage_core.input.Key.A
K_b = pynovage_core.input.Key.B
K_c = pynovage_core.input.Key.C
K_d = pynovage_core.input.Key.D
K_e = pynovage_core.input.Key.E
K_f = pynovage_core.input.Key.F
K_g = pynovage_core.input.Key.G
K_h = pynovage_core.input.Key.H
K_i = pynovage_core.input.Key.I
K_j = pynovage_core.input.Key.J
K_k = pynovage_core.input.Key.K
K_l = pynovage_core.input.Key.L
K_m = pynovage_core.input.Key.M
K_n = pynovage_core.input.Key.N
K_o = pynovage_core.input.Key.O
K_p = pynovage_core.input.Key.P
K_q = pynovage_core.input.Key.Q
K_r = pynovage_core.input.Key.R
K_s = pynovage_core.input.Key.S
K_t = pynovage_core.input.Key.T
K_u = pynovage_core.input.Key.U
K_v = pynovage_core.input.Key.V
K_w = pynovage_core.input.Key.W
K_x = pynovage_core.input.Key.X
K_y = pynovage_core.input.Key.Y
K_z = pynovage_core.input.Key.Z

class Event:
    """Simple event class similar to pygame.event.Event."""
    def __init__(self, type, **kwargs):
        self.type = type
        self.__dict__.update(kwargs)
    
    def __str__(self):
        return f"Event(type={self.type})"

def init():
    """Initialize PyNovaGE - must be called before using other functions."""
    global _window_system_guard, _renderer_guard
    
    if _window_system_guard is None:
        _window_system_guard = pynovage_core.window.WindowSystemGuard()
        if not _window_system_guard.is_initialized():
            raise RuntimeError("Failed to initialize window system")
    
    # Renderer will be initialized when display is set
    return True

def quit():
    """Quit PyNovaGE and cleanup resources."""
    global _window, _window_system_guard, _renderer_guard, _input_manager, _screen_surface
    
    _window = None
    _input_manager = None
    _screen_surface = None
    _renderer_guard = None
    _window_system_guard = None

def display_set_mode(size, flags=0, depth=32, display=0):
    """
    Create a display window or surface.
    
    Args:
        size: Tuple of (width, height) for the display
        flags: Display flags (currently unused)
        depth: Color depth (currently unused)
        display: Display index (currently unused)
        
    Returns:
        Surface representing the display
    """
    global _window, _renderer_guard, _input_manager, _screen_surface
    
    if not _window_system_guard or not _window_system_guard.is_initialized():
        raise RuntimeError("PyNovaGE not initialized. Call init() first.")
    
    # Create window
    config = pynovage_core.window.WindowConfig()
    config.width = size[0]
    config.height = size[1]
    config.title = "PyNovaGE Game"
    config.resizable = True
    config.vsync = True
    
    _window = pynovage_core.window.Window(config)
    
    # Initialize renderer
    renderer_config = pynovage_core.renderer.RendererConfig()
    _renderer_guard = pynovage_core.renderer.RendererGuard(renderer_config)
    
    if not _renderer_guard.is_initialized():
        raise RuntimeError("Failed to initialize renderer")
    
    # Set viewport to match window
    window_size = _window.get_framebuffer_size()
    pynovage_core.renderer.set_viewport(0, 0, window_size.x, window_size.y)
    
    # Initialize input manager (disabled for now until get_native_window is bound)
    # _input_manager = pynovage_core.input.InputManager(_window.get_native_window())
    _input_manager = None
    
    # Create screen surface
    _screen_surface = Surface(size)
    
    return _screen_surface

def display_flip():
    """Update the display - swap front and back buffers."""
    if _window:
        _window.swap_buffers()

def display_update():
    """Update portions of the display - currently same as flip()."""
    display_flip()

def display_get_surface():
    """Get the main display surface."""
    return _screen_surface

def event_get():
    """Get all pending events."""
    events = []
    
    if not _window or not _input_manager:
        return events
    
    # Poll window events
    _window.poll_events()
    _input_manager.update()
    
    # Check for window close
    if _window.should_close():
        events.append(Event(QUIT))
    
    # Convert input events (this is simplified)
    # In a full implementation, we'd set up callbacks to capture all events
    
    return events

def event_pump():
    """Process events without returning them."""
    if _window:
        _window.poll_events()
    if _input_manager:
        _input_manager.update()

def key_get_pressed():
    """Get the state of all keyboard buttons."""
    # Return a simple key state object
    class KeyState:
        def __init__(self, input_manager):
            self.input_manager = input_manager
        
        def __getitem__(self, key):
            if self.input_manager:
                return self.input_manager.is_key_pressed(key)
            return False
    
    return KeyState(_input_manager)

def mouse_get_pos():
    """Get the mouse cursor position."""
    if _input_manager:
        pos = _input_manager.get_mouse_position()
        return (int(pos.x), int(pos.y))
    return (0, 0)

def mouse_get_pressed():
    """Get the state of mouse buttons."""
    if _input_manager:
        return (
            _input_manager.is_mouse_button_pressed(pynovage_core.input.MouseButton.LEFT),
            _input_manager.is_mouse_button_pressed(pynovage_core.input.MouseButton.MIDDLE), 
            _input_manager.is_mouse_button_pressed(pynovage_core.input.MouseButton.RIGHT)
        )
    return (False, False, False)

def draw_rect(surface, color, rect, width=0):
    """
    Draw a rectangle on a surface.
    
    Args:
        surface: Surface to draw on
        color: Color to draw with
        rect: Rectangle to draw (Rect object or tuple)
        width: Line width (0 for filled rectangle)
    """
    if not isinstance(color, Color):
        color = Color(color)
    
    if not isinstance(rect, Rect):
        rect = Rect(rect)
    
    # For now, this is a placeholder implementation
    # In a full implementation, this would use the sprite renderer or draw calls
    pass

def draw_circle(surface, color, center, radius, width=0):
    """
    Draw a circle on a surface.
    
    Args:
        surface: Surface to draw on
        color: Color to draw with  
        center: Center point (x, y)
        radius: Circle radius
        width: Line width (0 for filled circle)
    """
    if not isinstance(color, Color):
        color = Color(color)
    
    # Placeholder implementation
    pass

def draw_line(surface, color, start_pos, end_pos, width=1):
    """
    Draw a line on a surface.
    
    Args:
        surface: Surface to draw on
        color: Color to draw with
        start_pos: Starting point (x, y)
        end_pos: Ending point (x, y)
        width: Line width
    """
    if not isinstance(color, Color):
        color = Color(color)
    
    # Placeholder implementation
    pass

def draw_polygon(surface, color, points, width=0):
    """
    Draw a polygon on a surface.
    
    Args:
        surface: Surface to draw on
        color: Color to draw with
        points: List of (x, y) points
        width: Line width (0 for filled polygon)
    """
    if not isinstance(color, Color):
        color = Color(color)
    
    # Placeholder implementation
    pass

def fill_screen(color):
    """Fill the screen with a solid color."""
    if not isinstance(color, Color):
        color = Color(color)
    
    # Use renderer to clear with color
    pynovage_core.renderer.clear(color.to_vector4f())

def blit_surface(dest, source, dest_pos, source_area=None):
    """
    Blit (copy) one surface to another.
    
    Args:
        dest: Destination surface  
        source: Source surface
        dest_pos: Position to blit to
        source_area: Optional source rectangle
    """
    if dest == _screen_surface and source._texture:
        # Direct rendering to screen using sprite renderer
        sprite_renderer = pynovage_core.renderer.get_sprite_renderer()
        if sprite_renderer:
            pos = pynovage_core.math.Vector2f(dest_pos[0], dest_pos[1])
            size = pynovage_core.math.Vector2f(source.width, source.height)
            sprite_renderer.draw_textured_sprite(pos, size, source._texture)
    else:
        # Use Surface.blit method
        dest.blit(source, dest_pos, source_area)

class Clock:
    """A clock class for timing and frame rate control."""
    
    def __init__(self):
        self.last_time = time.time()
        self.fps = 60
        
    def tick(self, framerate=60):
        """
        Control frame rate and return time since last call.
        
        Args:
            framerate: Target frame rate
            
        Returns:
            Milliseconds since last call
        """
        current_time = time.time()
        dt = current_time - self.last_time
        
        # Calculate target frame time
        target_frame_time = 1.0 / framerate if framerate > 0 else 0
        
        # Sleep if we have time left
        if dt < target_frame_time:
            time.sleep(target_frame_time - dt)
            current_time = time.time()
            dt = current_time - self.last_time
        
        self.last_time = current_time
        self.fps = 1.0 / dt if dt > 0 else 0
        
        return int(dt * 1000)  # Return milliseconds
    
    def get_fps(self):
        """Get the current frame rate."""
        return self.fps
    
    def get_time(self):
        """Get time since last tick in milliseconds."""
        return int((time.time() - self.last_time) * 1000)

# Additional utility functions
def load_image(filename):
    """Load an image file and return a Surface."""
    return Surface.load_image(filename)

def make_surface(size, flags=0):
    """Create a new Surface."""
    return Surface(size, flags)