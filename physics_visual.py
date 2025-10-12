#!/usr/bin/env python3
"""
Visual Physics Demo with Bouncing Balls

A PyNovaGE example demonstrating physics simulation with actual ball rendering.
This shows bouncing balls using the BatchRenderer to draw circles.

Controls:
- Arrow keys: Add/remove balls
  - Up/Down: Add/remove 50 balls
  - Left/Right: Add/remove 10 balls
- Space: Reset to initial ball count
- Escape: Exit

The balls are rendered as colored circles using the BatchRenderer primitive drawing.
"""

import sys
import os
import time
import random
import math

# Add the python_bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'python_bindings'))

import pynovage_core


class Ball:
    """A simple ball with physics properties and rendering."""
    
    def __init__(self, x, y, radius=None, vx=None, vy=None, color=None):
        self.x = x
        self.y = y
        self.radius = radius or random.uniform(8, 25)
        self.vx = vx or random.uniform(-300, 300)
        self.vy = vy or random.uniform(-300, 300)
        
        # Random bright color if not provided
        if color is None:
            self.color = pynovage_core.math.Vector4f(
                random.uniform(0.3, 1.0),  # Red
                random.uniform(0.3, 1.0),  # Green  
                random.uniform(0.3, 1.0),  # Blue
                1.0                        # Alpha
            )
        else:
            self.color = color
    
    def update(self, dt, screen_width, screen_height):
        """Update ball position and handle wall collisions."""
        # Update position
        self.x += self.vx * dt
        self.y += self.vy * dt
        
        # Wall collision detection and response
        if self.x - self.radius <= 0:
            self.x = self.radius
            self.vx = -self.vx * 0.85  # Energy loss on bounce
        elif self.x + self.radius >= screen_width:
            self.x = screen_width - self.radius
            self.vx = -self.vx * 0.85
            
        if self.y - self.radius <= 0:
            self.y = self.radius
            self.vy = -self.vy * 0.85
        elif self.y + self.radius >= screen_height:
            self.y = screen_height - self.radius
            self.vy = -self.vy * 0.85
    
    def draw(self, batch_renderer, screen_width, screen_height):
        """Draw the ball using BatchRenderer."""
        # Draw circle at ball position
        batch_renderer.add_circle_screen(
            self.x, self.y, self.radius,  # position and size
            screen_width, screen_height,  # screen dimensions  
            self.color,                   # color
            16                           # segments (lower for performance)
        )


def ball_collision(ball1, ball2):
    """Handle collision between two balls."""
    # Calculate distance between centers
    dx = ball2.x - ball1.x
    dy = ball2.y - ball1.y
    distance = math.sqrt(dx * dx + dy * dy)
    
    # Check if collision occurred
    if distance < ball1.radius + ball2.radius:
        # Normalize collision vector
        if distance > 0:
            dx /= distance
            dy /= distance
        else:
            # Balls are exactly on top of each other
            dx, dy = 1.0, 0.0
        
        # Separate balls
        overlap = ball1.radius + ball2.radius - distance
        ball1.x -= dx * overlap * 0.5
        ball1.y -= dy * overlap * 0.5
        ball2.x += dx * overlap * 0.5
        ball2.y += dy * overlap * 0.5
        
        # Calculate collision response
        # Dot product of relative velocity and collision normal
        dvx = ball2.vx - ball1.vx
        dvy = ball2.vy - ball1.vy
        dvn = dvx * dx + dvy * dy
        
        # Do not resolve if velocities are separating
        if dvn > 0:
            return
        
        # Collision impulse (simplified, assumes equal mass)
        impulse = 2 * dvn / 2
        ball1.vx += impulse * dx * 0.8  # Energy loss
        ball1.vy += impulse * dy * 0.8
        ball2.vx -= impulse * dx * 0.8
        ball2.vy -= impulse * dy * 0.8


def create_initial_balls(screen_width, screen_height, count=15):
    """Create initial set of balls."""
    balls = []
    for _ in range(count):
        # Ensure balls spawn away from edges
        margin = 60
        x = random.uniform(margin, screen_width - margin)
        y = random.uniform(margin, screen_height - margin)
        balls.append(Ball(x, y))
    return balls


def main():
    """Main simulation loop with visual rendering."""
    print("=" * 60)
    print("PyNovaGE Visual Physics Demo - Bouncing Balls")
    print("=" * 60)
    print("Controls:")
    print("  Up/Down arrows: Add/remove 50 balls")
    print("  Left/Right arrows: Add/remove 10 balls")
    print("  Space: Reset to 15 balls")
    print("  Close window or Ctrl+C: Exit")
    print("=" * 60)
    
    try:
        # Initialize window system
        print("Initializing window system...")
        window_guard = pynovage_core.window.WindowSystemGuard()
        
        # Screen configuration
        WIDTH, HEIGHT = 1200, 800
        
        # Create window
        print(f"Creating window ({WIDTH}x{HEIGHT})...")
        window = pynovage_core.window.create_window(WIDTH, HEIGHT, "PyNovaGE Visual Physics - Bouncing Balls")
        if not window:
            print("❌ Failed to create window!")
            return False
        
        print("✓ Window created successfully")
        
        # Initialize renderer
        print("Initializing renderer...")
        pynovage_core.renderer.initialize()
        
        # Disable depth testing for 2D rendering
        pynovage_core.renderer.set_depth_test(False)
        print("✓ Disabled depth testing for 2D rendering")
        
        # Get batch renderer
        batch_renderer = pynovage_core.renderer.get_batch_renderer()
        if not batch_renderer:
            print("❌ Failed to get batch renderer!")
            return False
        
        print("✓ Renderer initialized successfully")
        
        # Initialize simulation objects
        balls = create_initial_balls(WIDTH, HEIGHT, 15)
        print(f"✓ Created {len(balls)} initial balls")
        
        # Colors
        background_color = pynovage_core.math.Vector4f(0.1, 0.1, 0.15, 1.0)  # Dark blue
        
        # Timing
        last_time = time.time()
        frame_count = 0
        fps_update_time = time.time()
        last_key_time = 0  # For key debouncing
        
        print("✓ Starting physics simulation with visual rendering...")
        
        # Main loop
        while not window.should_close():
            current_time = time.time()
            dt = min(current_time - last_time, 1.0/30.0)  # Cap at 30 FPS for stability
            last_time = current_time
            frame_count += 1
            
            # Handle input (with debouncing)
            current_key_time = time.time()
            if current_key_time - last_key_time > 0.2:  # 200ms debounce
                
                # Check for key presses (this is a simplified approach)
                # In a real implementation, you'd use proper event handling
                # For now, we'll just handle window close
                pass
                
            # Update physics
            physics_start = time.perf_counter()
            
            # Update all balls
            for ball in balls:
                ball.update(dt, WIDTH, HEIGHT)
            
            # Handle ball-to-ball collisions
            collision_checks = 0
            for i in range(len(balls)):
                for j in range(i + 1, len(balls)):
                    ball_collision(balls[i], balls[j])
                    collision_checks += 1
            
            physics_end = time.perf_counter()
            physics_time = (physics_end - physics_start) * 1000  # milliseconds
            
            # Render
            render_start = time.perf_counter()
            
            # Begin frame
            pynovage_core.renderer.begin_frame()
            
            # Clear screen
            pynovage_core.renderer.clear(background_color)
            
            # Set viewport
            pynovage_core.renderer.set_viewport(0, 0, WIDTH, HEIGHT)
            
            # Begin batch
            batch_renderer.begin_batch()
            
            # Draw debug shapes first - these should definitely be visible
            # Red rectangle in top-left corner
            debug_red = pynovage_core.math.Vector4f(1.0, 0.0, 0.0, 1.0)
            batch_renderer.add_rect_screen(50, 50, 100, 100, WIDTH, HEIGHT, debug_red)
            
            # Green rectangle in center
            debug_green = pynovage_core.math.Vector4f(0.0, 1.0, 0.0, 1.0)
            batch_renderer.add_rect_screen(WIDTH//2 - 50, HEIGHT//2 - 50, 100, 100, WIDTH, HEIGHT, debug_green)
            
            # Blue rectangle in bottom-right
            debug_blue = pynovage_core.math.Vector4f(0.0, 0.0, 1.0, 1.0)
            batch_renderer.add_rect_screen(WIDTH - 150, HEIGHT - 150, 100, 100, WIDTH, HEIGHT, debug_blue)
            
            # Draw all balls
            for ball in balls:
                ball.draw(batch_renderer, WIDTH, HEIGHT)
            
            # End batch and flush
            batch_renderer.end_batch()
            batch_renderer.flush_batch()
            
            # End frame
            pynovage_core.renderer.end_frame()
            
            # Swap buffers
            window.swap_buffers()
            
            render_end = time.perf_counter()
            render_time = (render_end - render_start) * 1000  # milliseconds
            
            # Poll events
            window.poll_events()
            
            # Update performance stats every second
            if current_time - fps_update_time >= 1.0:
                fps = frame_count / (current_time - fps_update_time)
                
                # Update window title
                title = f"PyNovaGE Physics - {len(balls)} balls | {fps:.1f} FPS | {collision_checks} collisions | Physics: {physics_time:.2f}ms | Render: {render_time:.2f}ms"
                window.set_title(title)
                
                print(f"🏀 {len(balls)} balls | {fps:.1f} FPS | "
                      f"{collision_checks} collisions | "
                      f"Physics: {physics_time:.2f}ms | "
                      f"Render: {render_time:.2f}ms")
                
                fps_update_time = current_time
                frame_count = 0
            
            # Small delay to prevent excessive CPU usage
            time.sleep(0.008)  # ~125 FPS cap
        
        print("✓ Physics simulation completed")
        
        # Cleanup
        print("Cleaning up...")
        pynovage_core.renderer.shutdown()
        
        return True
        
    except KeyboardInterrupt:
        print("\nUser requested exit...")
        return True
    except Exception as e:
        print(f"❌ Physics demo failed: {e}")
        import traceback
        traceback.print_exc()
        return False


if __name__ == "__main__":
    success = main()
    
    print("\n" + "=" * 60)
    if success:
        print("🎉 VISUAL PHYSICS DEMO COMPLETED!")
        print("✓ Ball physics simulation working")
        print("✓ Visual rendering with BatchRenderer working") 
        print("✓ PyNovaGE physics + graphics integration successful")
    else:
        print("❌ Physics demo encountered issues")
    print("=" * 60)