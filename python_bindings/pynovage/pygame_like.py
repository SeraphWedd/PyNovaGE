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
_screen_size = (0, 0)
_batch_started = False

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
    global _window, _renderer_guard, _input_manager, _screen_surface, _screen_size, _batch_started
    
    if not _window_system_guard or not _window_system_guard.is_initialized():
        raise RuntimeError("PyNovaGE not initialized. Call init() first.")
    
    # Create window
    config = pynovage_core.window.WindowConfig()
    config.width = size[0]
    config.height = size[1]
    config.title = "PyNovaGE Game"
    config.resizable = True
    config.vsync = True
    config.visible = True  # Ensure window is visible
    
    _window = pynovage_core.window.Window(config)
    
    # Explicitly ensure window is shown
    _window.show()
    
    # Initialize renderer
    renderer_config = pynovage_core.renderer.RendererConfig()
    _renderer_guard = pynovage_core.renderer.RendererGuard(renderer_config)
    
    if not _renderer_guard.is_initialized():
        raise RuntimeError("Failed to initialize renderer")
    
    # Set viewport to match window
    window_size = _window.get_framebuffer_size()
    _screen_size = (int(window_size.x), int(window_size.y))
    print(f"Setting viewport: 0, 0, {window_size.x}, {window_size.y}")
    print(f"Window is visible: {not _window.is_minimized()}")
    print(f"Window is focused: {_window.is_focused()}")
    print(f"Window size: {_window.get_size()}")
    print(f"Framebuffer size: {window_size}")
    pynovage_core.renderer.set_viewport(0, 0, window_size.x, window_size.y)
    
    # Initialize input manager
    _input_manager = pynovage_core.input.InputManager(_window)
    
    # Create screen surface
    _screen_surface = Surface(size)
    
    # Begin first frame and start a batch
    pynovage_core.renderer.begin_frame()
    import pynovage
    br = pynovage.renderer.get_batch_renderer()
    if br:
        br.begin_batch()
        _batch_started = True
    
    return _screen_surface

def display_flip():
    """Update the display - swap front and back buffers."""
    if _window:
        # End the current batch before finishing the frame
        global _batch_started
        import pynovage
        br = pynovage.renderer.get_batch_renderer()
        if br and _batch_started:
            br.end_batch()
            br.flush_batch()
            _batch_started = False
        
        # End the frame and swap buffers
        pynovage_core.renderer.end_frame()
        _window.swap_buffers()
        
        # Begin new frame for next render cycle and start a fresh batch
        pynovage_core.renderer.begin_frame()
        if br:
            br.begin_batch()
            _batch_started = True

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
    
    # If drawing to the screen, use the batch renderer primitives
    if surface == _screen_surface:
        import pynovage
        br = pynovage.renderer.get_batch_renderer()
        if not br:
            return
        screen_w, screen_h = _screen_size
        # Convert from pygame-like top-left origin to bottom-left origin
        x = rect.x
        y = screen_h - rect.y - rect.height
        if width <= 0:
            br.add_rect_screen(float(x), float(y), float(rect.width), float(rect.height), int(screen_w), int(screen_h), color.to_vector4f())
        else:
            # Draw rectangle border using 4 thick lines
            t = float(width)
            c = color.to_vector4f()
            # Top
            br.add_line_screen(float(x), float(y + rect.height), float(x + rect.width), float(y + rect.height), t, int(screen_w), int(screen_h), c)
            # Bottom
            br.add_line_screen(float(x), float(y), float(x + rect.width), float(y), t, int(screen_w), int(screen_h), c)
            # Left
            br.add_line_screen(float(x), float(y), float(x), float(y + rect.height), t, int(screen_w), int(screen_h), c)
            # Right
            br.add_line_screen(float(x + rect.width), float(y), float(x + rect.width), float(y + rect.height), t, int(screen_w), int(screen_h), c)
    else:
        # For other surfaces, use the fill method if available
        if hasattr(surface, 'fill_rect'):
            surface.fill_rect(rect, color)


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
    
    if surface == _screen_surface:
        import pynovage
        br = pynovage.renderer.get_batch_renderer()
        if not br:
            return
        screen_w, screen_h = _screen_size
        x0, y0 = float(start_pos[0]), float(start_pos[1])
        x1, y1 = float(end_pos[0]), float(end_pos[1])
        # Convert Y from top-left to bottom-left origin
        y0 = float(screen_h) - y0
        y1 = float(screen_h) - y1
        br.add_line_screen(x0, y0, x1, y1, float(max(1, width)), int(screen_w), int(screen_h), color.to_vector4f())
    else:
        # No-op for offscreen surfaces for now
        return

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
    
    if surface == _screen_surface and points:
        import pynovage
        br = pynovage.renderer.get_batch_renderer()
        if not br:
            return
        screen_w, screen_h = _screen_size
        c = color.to_vector4f()
        # Draw as outline by default using lines between successive points
        pts = list(points)
        if width <= 0:
            w = 1.0
        else:
            w = float(width)
        for i in range(len(pts)):
            x0, y0 = float(pts[i][0]), float(pts[i][1])
            x1, y1 = float(pts[(i + 1) % len(pts)][0]), float(pts[(i + 1) % len(pts)][1])
            y0 = float(screen_h) - y0
            y1 = float(screen_h) - y1
            br.add_line_screen(x0, y0, x1, y1, w, int(screen_w), int(screen_h), c)
    else:
        return

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
    
    if surface == _screen_surface:
        import pynovage
        br = pynovage.renderer.get_batch_renderer()
        if not br:
            return
        screen_w, screen_h = _screen_size
        x = float(center[0])
        y = float(screen_h) - float(center[1])  # convert to bottom-left origin
        if width <= 0:
            br.add_circle_screen(x, y, float(radius), int(screen_w), int(screen_h), color.to_vector4f(), 32)
        else:
            # Approximate an outline circle by connecting segments with lines
            import math
            segments = 32
            angle_step = 2.0 * math.pi / segments
            pts = []
            for i in range(segments + 1):
                angle = i * angle_step
                vx = x + math.cos(angle) * radius
                vy = y + math.sin(angle) * radius
                pts.append((vx, vy))
            c = color.to_vector4f()
            t = float(max(1, width))
            for i in range(segments):
                x0, y0 = pts[i]
                x1, y1 = pts[i + 1]
                br.add_line_screen(float(x0), float(y0), float(x1), float(y1), t, int(screen_w), int(screen_h), c)
    else:
        # Offscreen surfaces: no-op for now
        return
    
def set_window_title(title):
    """Set the window title."""
    global _window
    if _window:
        _window.set_title(title)

def fill_screen(color):
    """Fill the screen with a solid color."""
    if not isinstance(color, Color):
        color = Color(color)
    
    # Try alternative approach: Use batch renderer to draw a full-screen rectangle
    import pynovage
    br = pynovage.renderer.get_batch_renderer()
    if br:
        screen_w, screen_h = _screen_size
        # Draw a full-screen rectangle
        br.add_rect_screen(0.0, 0.0, float(screen_w), float(screen_h), 
                          int(screen_w), int(screen_h), color.to_vector4f())
    
    # Also try the original clear command in case it works
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
    if dest == _screen_surface and getattr(source, "_texture", None):
        # Direct rendering to screen using sprite renderer
        sprite_renderer = pynovage_core.renderer.get_sprite_renderer()
        if sprite_renderer:
            # Build a Sprite and render it
            pos = pynovage_core.math.Vector2f(float(dest_pos[0]), float(dest_pos[1]))
            # Convert to bottom-left origin in SpriteRenderer space (matches current simple NDC mapping);
            # here we keep top-left and accept mismatch until matrices are implemented.
            sprite = pynovage_core.renderer.Sprite()
            sprite.position = pos
            sprite.size = pynovage_core.math.Vector2f(float(source.width), float(source.height))
            sprite.texture = source._texture
            sprite_renderer.render_sprite(sprite)
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

# Create display module to match pygame structure
class DisplayModule:
    """Display module - similar to pygame.display"""
    
    @staticmethod
    def set_mode(size, flags=0, depth=32, display=0):
        """Create a display window."""
        return display_set_mode(size, flags, depth, display)
    
    @staticmethod
    def flip():
        """Update the display."""
        return display_flip()
    
    @staticmethod
    def update():
        """Update portions of the display."""
        return display_update()
    
    @staticmethod
    def get_surface():
        """Get the main display surface."""
        return display_get_surface()

# Create event module to match pygame structure
class EventModule:
    """Event module - similar to pygame.event"""
    
    @staticmethod
    def get():
        """Get all pending events."""
        return event_get()
    
    @staticmethod
    def pump():
        """Process events without returning them."""
        return event_pump()

# Create key module to match pygame structure
class KeyModule:
    """Key module - similar to pygame.key"""
    
    @staticmethod
    def get_pressed():
        """Get the state of all keyboard buttons."""
        return key_get_pressed()

# Create mouse module to match pygame structure
class MouseModule:
    """Mouse module - similar to pygame.mouse"""
    
    @staticmethod
    def get_pos():
        """Get the mouse cursor position."""
        return mouse_get_pos()
    
    @staticmethod
    def get_pressed():
        """Get the state of mouse buttons."""
        return mouse_get_pressed()

# Create draw module to match pygame structure
class DrawModule:
    """Draw module - similar to pygame.draw"""
    
    @staticmethod
    def rect(surface, color, rect, width=0):
        """Draw a rectangle."""
        return draw_rect(surface, color, rect, width)
    
    @staticmethod
    def circle(surface, color, center, radius, width=0):
        """Draw a circle."""
        return draw_circle(surface, color, center, radius, width)
    
    @staticmethod
    def line(surface, color, start_pos, end_pos, width=1):
        """Draw a line."""
        return draw_line(surface, color, start_pos, end_pos, width)
    
    @staticmethod
    def polygon(surface, color, points, width=0):
        """Draw a polygon."""
        return draw_polygon(surface, color, points, width)

# Create module instances
display = DisplayModule()
event = EventModule()
key = KeyModule()
mouse = MouseModule()
draw = DrawModule()
