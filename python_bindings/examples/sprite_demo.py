#!/usr/bin/env python3
"""
Sprite Demo for PyNovaGE

This example demonstrates how to load and render sprites/textures.
Shows the power of PyNovaGE's C++ rendering backend.
"""

import pynovage as pg
import sys
import os
import math

def main():
    # Initialize PyNovaGE
    pg.init()
    
    # Set up the display
    screen = pg.display_set_mode((1024, 768))
    clock = pg.Clock()
    
    print("PyNovaGE Sprite Demo")
    print("====================")
    print("This demo would show sprite rendering with textures.")
    print("Currently demonstrates the pygame-like interface structure.")
    print("Press ESCAPE to quit.")
    print()
    
    # Colors
    BLACK = pg.Color.BLACK
    WHITE = pg.Color.WHITE
    RED = pg.Color.RED
    BLUE = pg.Color.BLUE
    
    # Game state
    background_color = BLACK
    sprite_positions = []
    
    # Create some virtual sprites (positions that would represent sprites)
    for i in range(10):
        x = 100 + (i % 5) * 150
        y = 100 + (i // 5) * 200
        sprite_positions.append([x, y])
    
    running = True
    frame_count = 0
    time_accumulator = 0
    
    while running:
        dt = clock.tick(60)
        time_accumulator += dt / 1000.0  # Convert to seconds
        
        # Handle events
        for event in pg.event_get():
            if event.type == pg.QUIT:
                running = False
        
        # Check for escape key
        keys = pg.key_get_pressed()
        if keys[pg.K_ESCAPE]:
            running = False
        
        # Animate sprite positions
        for i, pos in enumerate(sprite_positions):
            # Simple sine wave animation
            offset_x = math.sin(time_accumulator + i * 0.5) * 50
            offset_y = math.cos(time_accumulator + i * 0.3) * 30
            pos[0] = 100 + (i % 5) * 150 + offset_x
            pos[1] = 100 + (i // 5) * 200 + offset_y
        
        # Clear screen
        pg.fill_screen(background_color)
        
        # In a full implementation, this would render actual sprites:
        # for pos in sprite_positions:
        #     pg.blit_surface(screen, sprite_surface, pos)
        
        # For now, we just change background color to show animation
        intensity = int(128 + 127 * math.sin(time_accumulator))
        background_color = pg.Color(intensity // 4, intensity // 6, intensity // 3)
        
        # Update display
        pg.display_flip()
        frame_count += 1
        
        # Print FPS every 2 seconds
        if frame_count % 120 == 0:
            fps = clock.get_fps()
            sprite_count = len(sprite_positions)
            print(f"Frame {frame_count}: FPS={fps:.1f}, Sprites={sprite_count}, Time={time_accumulator:.1f}s")
        
        # Auto-quit after 10 seconds for demo
        if time_accumulator > 10:
            print("Demo completed after 10 seconds!")
            running = False
    
    # Clean up
    pg.quit()
    print("Sprite Demo finished!")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)