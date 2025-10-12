"""Rectangle utility class for PyNovaGE."""

import pynovage_core

class Rect:
    """
    A rectangle class that provides pygame-like rectangle functionality.
    
    Represents a rectangular area defined by position (x, y) and size (width, height).
    """
    
    def __init__(self, x, y=None, width=None, height=None):
        """
        Initialize a Rect object.
        
        Args:
            x: X coordinate or a tuple/list of (x, y, width, height)
            y: Y coordinate or None if x is a tuple  
            width: Width or None if x is a tuple
            height: Height or None if x is a tuple
        """
        if isinstance(x, (tuple, list)):
            if len(x) == 4:
                self.x, self.y, self.width, self.height = x
            else:
                raise ValueError("Rect tuple must have 4 components (x, y, width, height)")
        elif y is not None and width is not None and height is not None:
            self.x, self.y, self.width, self.height = x, y, width, height
        else:
            raise ValueError("Invalid rect specification")
        
        # Ensure all values are numeric
        self.x = float(self.x)
        self.y = float(self.y)
        self.width = float(self.width)
        self.height = float(self.height)
    
    # Properties for convenience (similar to pygame.Rect)
    @property
    def left(self):
        return self.x
    
    @left.setter
    def left(self, value):
        self.x = float(value)
    
    @property
    def right(self):
        return self.x + self.width
    
    @right.setter
    def right(self, value):
        self.x = float(value) - self.width
    
    @property
    def top(self):
        return self.y
    
    @top.setter
    def top(self, value):
        self.y = float(value)
    
    @property
    def bottom(self):
        return self.y + self.height
    
    @bottom.setter
    def bottom(self, value):
        self.y = float(value) - self.height
    
    @property
    def center(self):
        return (self.x + self.width / 2, self.y + self.height / 2)
    
    @center.setter
    def center(self, value):
        cx, cy = value
        self.x = cx - self.width / 2
        self.y = cy - self.height / 2
    
    @property
    def centerx(self):
        return self.x + self.width / 2
    
    @centerx.setter
    def centerx(self, value):
        self.x = float(value) - self.width / 2
    
    @property
    def centery(self):
        return self.y + self.height / 2
    
    @centery.setter
    def centery(self, value):
        self.y = float(value) - self.height / 2
    
    @property
    def size(self):
        return (self.width, self.height)
    
    @size.setter
    def size(self, value):
        self.width, self.height = value
    
    @property
    def topleft(self):
        return (self.x, self.y)
    
    @topleft.setter
    def topleft(self, value):
        self.x, self.y = value
    
    @property
    def topright(self):
        return (self.right, self.y)
    
    @topright.setter
    def topright(self, value):
        self.right, self.y = value
    
    @property
    def bottomleft(self):
        return (self.x, self.bottom)
    
    @bottomleft.setter
    def bottomleft(self, value):
        self.x, self.bottom = value
    
    @property
    def bottomright(self):
        return (self.right, self.bottom)
    
    @bottomright.setter
    def bottomright(self, value):
        self.right, self.bottom = value
    
    def to_vector2f(self):
        """Convert position to PyNovaGE Vector2f."""
        return pynovage_core.math.Vector2f(self.x, self.y)
    
    def size_to_vector2f(self):
        """Convert size to PyNovaGE Vector2f."""
        return pynovage_core.math.Vector2f(self.width, self.height)
    
    def copy(self):
        """Create a copy of this rectangle."""
        return Rect(self.x, self.y, self.width, self.height)
    
    def move(self, dx, dy):
        """Move the rectangle by the given offset and return a new Rect."""
        return Rect(self.x + dx, self.y + dy, self.width, self.height)
    
    def move_ip(self, dx, dy):
        """Move the rectangle by the given offset in place."""
        self.x += dx
        self.y += dy
    
    def inflate(self, dx, dy):
        """Inflate the rectangle by the given amount and return a new Rect."""
        return Rect(self.x - dx/2, self.y - dy/2, self.width + dx, self.height + dy)
    
    def inflate_ip(self, dx, dy):
        """Inflate the rectangle by the given amount in place."""
        self.x -= dx / 2
        self.y -= dy / 2
        self.width += dx
        self.height += dy
    
    def contains(self, other):
        """Check if this rectangle completely contains another rectangle or point."""
        if isinstance(other, Rect):
            return (self.x <= other.x and 
                   self.y <= other.y and
                   self.right >= other.right and
                   self.bottom >= other.bottom)
        elif isinstance(other, (tuple, list)) and len(other) == 2:
            x, y = other
            return (self.x <= x < self.right and 
                   self.y <= y < self.bottom)
        return False
    
    def colliderect(self, other):
        """Check if this rectangle collides with another rectangle."""
        if not isinstance(other, Rect):
            return False
        return not (self.right <= other.x or 
                   self.x >= other.right or
                   self.bottom <= other.y or
                   self.y >= other.bottom)
    
    def collidepoint(self, point):
        """Check if this rectangle contains a point."""
        x, y = point
        return (self.x <= x < self.right and 
               self.y <= y < self.bottom)
    
    def union(self, other):
        """Return a new rectangle that covers both rectangles."""
        if not isinstance(other, Rect):
            return self.copy()
        
        left = min(self.x, other.x)
        top = min(self.y, other.y)
        right = max(self.right, other.right)
        bottom = max(self.bottom, other.bottom)
        
        return Rect(left, top, right - left, bottom - top)
    
    def union_ip(self, other):
        """Update this rectangle to cover both rectangles."""
        if isinstance(other, Rect):
            left = min(self.x, other.x)
            top = min(self.y, other.y)
            right = max(self.right, other.right)
            bottom = max(self.bottom, other.bottom)
            
            self.x = left
            self.y = top
            self.width = right - left
            self.height = bottom - top
    
    def clip(self, other):
        """Return the intersection of this rectangle with another."""
        if not isinstance(other, Rect) or not self.colliderect(other):
            return Rect(0, 0, 0, 0)
        
        left = max(self.x, other.x)
        top = max(self.y, other.y)
        right = min(self.right, other.right)
        bottom = min(self.bottom, other.bottom)
        
        return Rect(left, top, right - left, bottom - top)
    
    def __str__(self):
        return f"Rect({self.x}, {self.y}, {self.width}, {self.height})"
    
    def __repr__(self):
        return self.__str__()
    
    def __eq__(self, other):
        if isinstance(other, Rect):
            return (self.x, self.y, self.width, self.height) == (other.x, other.y, other.width, other.height)
        return False
    
    def __bool__(self):
        return self.width > 0 and self.height > 0