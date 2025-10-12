#!/usr/bin/env python3
"""
Basic Window Example for PyNovaGE

This example demonstrates how to create a basic window using the pygame-like interface.
Similar to a basic pygame program but using PyNovaGE's high-performance C++ backend.
"""

import pynovage as pg
import sys

def main():
    # Initialize PyNovaGE
    pg.init()
    
    # Set up the display
    screen = pg.display_set_mode((800, 600))
    clock = pg.Clock()
    
    # Colors
    BLACK = pg.Color(0, 0, 0)
    WHITE = pg.Color(255, 255, 255)
    RED = pg.Color(255, 0, 0)
    GREEN = pg.Color(0, 255, 0)
    BLUE = pg.Color(0, 0, 255)
    
    colors = [RED, GREEN, BLUE, WHITE]
    color_index = 0
    
    print("PyNovaGE Basic Window Demo")
    print("==========================")
    print("The window will cycle through different background colors.")
    print("Press ESCAPE to quit or close the window.")
    print(f"Using PyNovaGE version: {pg.__version__}")
    
    running = True
    frame_count = 0
    
    while running:
        # Handle events
        for event in pg.event_get():
            if event.type == pg.QUIT:
                running = False
        
        # Check for escape key
        keys = pg.key_get_pressed()
        if keys[pg.K_ESCAPE]:
            running = False
        
        # Change color every 60 frames (1 second at 60fps)
        if frame_count % 60 == 0:
            color_index = (color_index + 1) % len(colors)
            print(f"Frame {frame_count}: Switching to color {colors[color_index]}")
        
        # Fill screen with current color
        current_color = colors[color_index]
        pg.fill_screen(current_color)
        
        # Update display
        pg.display_flip()
        
        # Control frame rate
        clock.tick(60)
        frame_count += 1
        
        # Auto-quit after 5 seconds for demo purposes
        if frame_count >= 300:
            print("Demo completed after 5 seconds!")
            running = False
    
    # Clean up
    pg.quit()
    print("PyNovaGE Basic Window Demo finished successfully!")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)