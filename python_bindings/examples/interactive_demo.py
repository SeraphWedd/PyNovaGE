#!/usr/bin/env python3
"""
Interactive Demo for PyNovaGE

This example demonstrates input handling, colors, and basic drawing operations.
Shows how to use keyboard and mouse input with the pygame-like interface.
"""

import pynovage as pg
import sys
import math

def main():
    # Initialize PyNovaGE
    pg.init()
    
    # Set up the display
    screen = pg.display_set_mode((800, 600))
    clock = pg.Clock()
    
    # Colors
    BLACK = pg.Color.BLACK
    WHITE = pg.Color.WHITE
    RED = pg.Color.RED
    GREEN = pg.Color.GREEN
    BLUE = pg.Color.BLUE
    YELLOW = pg.Color.YELLOW
    
    print("PyNovaGE Interactive Demo")
    print("=========================")
    print("Controls:")
    print("  WASD or Arrow Keys - Change background color")
    print("  Mouse - Move cursor around")
    print("  Left Click - Change to red background")
    print("  Right Click - Change to blue background") 
    print("  SPACE - Reset to black background")
    print("  ESCAPE - Quit")
    print()
    
    # Game state
    background_color = BLACK
    mouse_pos = (400, 300)
    mouse_trail = []
    color_change_timer = 0
    
    running = True
    frame_count = 0
    
    while running:
        dt = clock.tick(60)
        
        # Handle events
        for event in pg.event_get():
            if event.type == pg.QUIT:
                running = False
        
        # Get input states
        keys = pg.key_get_pressed()
        mouse_buttons = pg.mouse_get_pressed()
        mouse_pos = pg.mouse_get_pos()
        
        # Handle keyboard input
        if keys[pg.K_ESCAPE]:
            running = False
        
        if keys[pg.K_SPACE]:
            background_color = BLACK
            color_change_timer = 30
        
        # WASD and arrow key controls
        color_speed = 2
        if keys[pg.K_w] or keys[pg.K_UP]:
            background_color = pg.Color(
                min(255, background_color.r + color_speed),
                background_color.g,
                background_color.b
            )
        if keys[pg.K_s] or keys[pg.K_DOWN]:
            background_color = pg.Color(
                max(0, background_color.r - color_speed),
                background_color.g,
                background_color.b
            )
        if keys[pg.K_a] or keys[pg.K_LEFT]:
            background_color = pg.Color(
                background_color.r,
                min(255, background_color.g + color_speed),
                background_color.b
            )
        if keys[pg.K_d] or keys[pg.K_RIGHT]:
            background_color = pg.Color(
                background_color.r,
                background_color.g,
                max(0, background_color.b - color_speed)
            )
        
        # Handle mouse input
        if mouse_buttons[0]:  # Left click
            background_color = RED
            color_change_timer = 30
        elif mouse_buttons[2]:  # Right click
            background_color = BLUE
            color_change_timer = 30
        
        # Add to mouse trail
        mouse_trail.append(mouse_pos)
        if len(mouse_trail) > 20:
            mouse_trail.pop(0)
        
        # Update color change timer
        if color_change_timer > 0:
            color_change_timer -= 1
        
        # Clear screen
        pg.fill_screen(background_color)
        
        # Draw mouse trail (placeholder - actual drawing would be implemented)
        # In a full implementation, this would draw circles at each trail position
        
        # Update display
        pg.display_flip()
        frame_count += 1
        
        # Print status every 2 seconds
        if frame_count % 120 == 0:
            fps = clock.get_fps()
            print(f"Frame {frame_count}: FPS={fps:.1f}, Mouse=({mouse_pos[0]}, {mouse_pos[1]}), Color={background_color}")
    
    # Clean up
    pg.quit()
    print("Interactive Demo finished!")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
