"""Color utility class for PyNovaGE."""

import pynovage_core

class Color:
    """
    A color class that provides pygame-like color functionality.
    
    Colors can be created from RGB, RGBA values, or from named colors.
    """
    
    # Named color constants (similar to pygame)
    BLACK = (0, 0, 0)
    WHITE = (255, 255, 255)
    RED = (255, 0, 0)
    GREEN = (0, 255, 0)
    BLUE = (0, 0, 255)
    YELLOW = (255, 255, 0)
    MAGENTA = (255, 0, 255)
    CYAN = (0, 255, 255)
    ORANGE = (255, 165, 0)
    PURPLE = (128, 0, 128)
    BROWN = (165, 42, 42)
    PINK = (255, 192, 203)
    GRAY = (128, 128, 128)
    GREY = GRAY  # Alias
    LIGHTGRAY = (211, 211, 211)
    DARKGRAY = (169, 169, 169)
    LIGHTBLUE = (173, 216, 230)
    DARKBLUE = (0, 0, 139)
    LIGHTGREEN = (144, 238, 144)
    DARKGREEN = (0, 100, 0)
    LIGHTRED = (255, 182, 193)
    DARKRED = (139, 0, 0)
    
    def __init__(self, r, g=None, b=None, a=255):
        """
        Initialize a Color object.
        
        Args:
            r: Red component (0-255) or a tuple/list of (r,g,b) or (r,g,b,a)
            g: Green component (0-255) or None if r is a tuple
            b: Blue component (0-255) or None if r is a tuple
            a: Alpha component (0-255), defaults to 255 (opaque)
        """
        if isinstance(r, (tuple, list)):
            if len(r) == 3:
                self.r, self.g, self.b = r
                self.a = a
            elif len(r) == 4:
                self.r, self.g, self.b, self.a = r
            else:
                raise ValueError("Color tuple must have 3 or 4 components")
        elif g is not None and b is not None:
            self.r, self.g, self.b, self.a = r, g, b, a
        else:
            raise ValueError("Invalid color specification")
        
        # Clamp values to valid range
        self.r = max(0, min(255, int(self.r)))
        self.g = max(0, min(255, int(self.g)))
        self.b = max(0, min(255, int(self.b)))
        self.a = max(0, min(255, int(self.a)))
    
    def to_vector4f(self):
        """Convert to PyNovaGE Vector4f (normalized 0.0-1.0 range)."""
        return pynovage_core.math.Vector4f(
            self.r / 255.0,
            self.g / 255.0, 
            self.b / 255.0,
            self.a / 255.0
        )
    
    def to_tuple(self, include_alpha=True):
        """Convert to tuple (r, g, b) or (r, g, b, a)."""
        if include_alpha:
            return (self.r, self.g, self.b, self.a)
        else:
            return (self.r, self.g, self.b)
    
    def copy(self):
        """Create a copy of this color."""
        return Color(self.r, self.g, self.b, self.a)
    
    def __str__(self):
        return f"Color({self.r}, {self.g}, {self.b}, {self.a})"
    
    def __repr__(self):
        return self.__str__()
    
    def __eq__(self, other):
        if isinstance(other, Color):
            return (self.r, self.g, self.b, self.a) == (other.r, other.g, other.b, other.a)
        elif isinstance(other, (tuple, list)):
            if len(other) == 3:
                return (self.r, self.g, self.b) == tuple(other)
            elif len(other) == 4:
                return (self.r, self.g, self.b, self.a) == tuple(other)
        return False
    
    def __getitem__(self, index):
        """Allow indexing like a tuple."""
        if index == 0: return self.r
        elif index == 1: return self.g
        elif index == 2: return self.b
        elif index == 3: return self.a
        else: raise IndexError("Color index out of range")
    
    def __setitem__(self, index, value):
        """Allow setting components by index."""
        value = max(0, min(255, int(value)))
        if index == 0: self.r = value
        elif index == 1: self.g = value
        elif index == 2: self.b = value
        elif index == 3: self.a = value
        else: raise IndexError("Color index out of range")
    
    def __len__(self):
        return 4
    
    @classmethod
    def from_name(cls, name):
        """Create a color from a name string."""
        name = name.upper()
        if hasattr(cls, name):
            color_tuple = getattr(cls, name)
            return cls(color_tuple)
        else:
            raise ValueError(f"Unknown color name: {name}")
    
    @classmethod
    def from_hex(cls, hex_string):
        """Create a color from a hex string like '#FF0000' or 'FF0000'."""
        hex_string = hex_string.lstrip('#')
        if len(hex_string) == 6:
            return cls(
                int(hex_string[0:2], 16),
                int(hex_string[2:4], 16),
                int(hex_string[4:6], 16)
            )
        elif len(hex_string) == 8:
            return cls(
                int(hex_string[0:2], 16),
                int(hex_string[2:4], 16),
                int(hex_string[4:6], 16),
                int(hex_string[6:8], 16)
            )
        else:
            raise ValueError("Hex color string must be 6 or 8 characters")