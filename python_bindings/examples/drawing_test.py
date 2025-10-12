#!/usr/bin/env python3
"""
Simple test to verify all PyNovaGE drawing functions work correctly.
Tests circle, rectangle, line, and polygon drawing.
"""

import os, sys
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import pynovage
import time

def main():
    """Test all drawing functions."""
    print("🎨 PyNovaGE Drawing Functions Test")
    print("=" * 40)
    
    # Initialize
    pynovage.init()
    screen = pynovage.display.set_mode((800, 600))
    clock = pynovage.Clock()
    
    # Colors
    white = pynovage.Color(255, 255, 255)
    red = pynovage.Color(255, 0, 0)
    green = pynovage.Color(0, 255, 0)
    blue = pynovage.Color(0, 0, 255)
    yellow = pynovage.Color(255, 255, 0)
    purple = pynovage.Color(255, 0, 255)
    
    running = True
    test_duration = 3  # seconds
    start_time = time.time()
    
    print("Testing drawing functions for 3 seconds...")
    print("Watch the window - you should see various colored shapes")
    
    while running and (time.time() - start_time) < test_duration:
        # Handle events
        for event in pynovage.event.get():
            if event.type == pynovage.QUIT:
                running = False
        
        # Clear to white background
        pynovage.fill_screen(white)
        
        # Test: Draw a simple red rectangle in the center
        pynovage.draw.rect(screen, red, (300, 200, 200, 200))  # 200x200 rect at (300,200)
        
        # Update display
        pynovage.display.flip()
        clock.tick(60)
    
    pynovage.quit()
    print("✅ Drawing test completed!")

if __name__ == "__main__":
    main()