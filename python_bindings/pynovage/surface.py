"""Surface class for PyNovaGE rendering."""

import pynovage_core
from .color import Color
from .rect import Rect

class Surface:
    """
    A surface represents a rectangular collection of pixels that can be drawn to.
    Similar to pygame.Surface but backed by PyNovaGE rendering system.
    """
    
    def __init__(self, size, flags=0):
        """
        Initialize a Surface.
        
        Args:
            size: Tuple of (width, height) for the surface
            flags: Surface creation flags (currently unused)
        """
        self.width, self.height = size
        # For now, we'll use a texture as the backing store
        # In a full implementation, this would be more sophisticated
        self._texture = None
        
    def get_size(self):
        """Get the size of the surface as (width, height)."""
        return (self.width, self.height)
    
    def get_width(self):
        """Get the width of the surface."""
        return self.width
    
    def get_height(self):
        """Get the height of the surface."""
        return self.height
    
    def get_rect(self):
        """Get a Rect representing the surface area."""
        return Rect(0, 0, self.width, self.height)
    
    def copy(self):
        """Create a copy of this surface."""
        new_surface = Surface((self.width, self.height))
        # In a full implementation, this would copy the pixel data
        return new_surface
    
    def fill(self, color, rect=None):
        """
        Fill the surface with a solid color.
        
        Args:
            color: Color to fill with (Color object or tuple)
            rect: Optional Rect to limit filling area
        """
        if not isinstance(color, Color):
            color = Color(color)
        
        # This is a simplified implementation
        # In reality, this would involve actual pixel manipulation or rendering
        pass
    
    def blit(self, source, dest, area=None, special_flags=0):
        """
        Draw one surface onto another.
        
        Args:
            source: Source Surface to draw from
            dest: Destination position (x, y) or Rect
            area: Optional source Rect to copy from
            special_flags: Special blending flags (currently unused)
        """
        if isinstance(dest, (tuple, list)):
            dest_rect = Rect(dest[0], dest[1], source.width, source.height)
        elif isinstance(dest, Rect):
            dest_rect = dest
        else:
            raise ValueError("dest must be a position tuple or Rect")
        
        # This is a simplified implementation
        # In reality, this would involve actual pixel copying or rendering
        pass
    
    def convert(self):
        """
        Convert surface to display pixel format for faster blitting.
        Currently returns self as a placeholder.
        """
        return self
    
    def convert_alpha(self):
        """
        Convert surface to display pixel format with alpha for faster blitting.
        Currently returns self as a placeholder.
        """
        return self
    
    @staticmethod
    def load_image(filename):
        """
        Load an image from file and create a Surface.
        
        Args:
            filename: Path to image file
            
        Returns:
            Surface containing the loaded image
        """
        # Load texture using PyNovaGE renderer
        texture = pynovage_core.renderer.load_texture(filename)
        if texture and texture.is_valid():
            surface = Surface((texture.get_width(), texture.get_height()))
            surface._texture = texture
            return surface
        else:
            raise ValueError(f"Could not load image: {filename}")
    
    def get_texture(self):
        """Get the underlying PyNovaGE texture (if any)."""
        return self._texture
    
    def __str__(self):
        return f"Surface({self.width}x{self.height})"
    
    def __repr__(self):
        return self.__str__()