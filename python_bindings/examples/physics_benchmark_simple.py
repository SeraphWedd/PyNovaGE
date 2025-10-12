#!/usr/bin/env python3
"""
Physics Benchmark: Computational Only

A PyNovaGE example demonstrating physics simulation performance.
This benchmark focuses on computation and timing without graphics.

Controls:
- Arrow keys: Add/remove balls
  - Up/Down: Add/remove 50 balls
  - Left/Right: Add/remove 10 balls
- Space: Reset to initial ball count
- Escape: Exit

Performance metrics are displayed in the window title and console.
"""

import pynovage
import random
import math
import time


class Ball:
    """A simple ball with physics properties."""
    
    def __init__(self, x, y, radius=None, vx=None, vy=None, color=None):
        self.x = x
        self.y = y
        self.radius = radius or random.uniform(5, 20)
        self.vx = vx or random.uniform(-200, 200)
        self.vy = vy or random.uniform(-200, 200)
        
        # Random color if not provided
        if color is None:
            self.color = pynovage.Color(
                random.randint(50, 255),
                random.randint(50, 255), 
                random.randint(50, 255)
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
            self.vx = -self.vx * 0.8  # Energy loss on bounce
        elif self.x + self.radius >= screen_width:
            self.x = screen_width - self.radius
            self.vx = -self.vx * 0.8
            
        if self.y - self.radius <= 0:
            self.y = self.radius
            self.vy = -self.vy * 0.8
        elif self.y + self.radius >= screen_height:
            self.y = screen_height - self.radius
            self.vy = -self.vy * 0.8


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


def create_initial_balls(screen_width, screen_height, count=10):
    """Create initial set of balls."""
    balls = []
    for _ in range(count):
        # Ensure balls spawn away from edges
        margin = 50
        x = random.uniform(margin, screen_width - margin)
        y = random.uniform(margin, screen_height - margin)
        balls.append(Ball(x, y))
    return balls


def main():
    """Main simulation loop."""
    # Initialize PyNovaGE
    pynovage.init()
    
    # Screen configuration
    WIDTH, HEIGHT = 1200, 800
    screen = pynovage.display.set_mode((WIDTH, HEIGHT))
    
    # Initialize simulation objects
    balls = create_initial_balls(WIDTH, HEIGHT, 25)  # Start with 25 balls
    
    # Timing
    clock = pynovage.Clock()
    running = True
    frame_count = 0
    fps_update_time = time.time()
    total_collision_checks = 0
    
    # Colors
    background_color = pynovage.Color(20, 20, 30)  # Dark blue background
    
    print(f"Starting physics benchmark with {len(balls)} balls")
    print("Controls:")
    print("  Up/Down arrows: Add/remove 50 balls")  
    print("  Left/Right arrows: Add/remove 10 balls")
    print("  Space: Reset to 25 balls")
    print("  Escape: Exit")
    print("Note: Graphics drawing disabled for pure physics performance test")
    
    while running:
        dt = clock.tick(120) / 1000.0  # Convert to seconds, target 120 FPS
        frame_count += 1
        
        # Handle events
        for event in pynovage.event.get():
            if event.type == pynovage.QUIT:
                running = False
        
        # Handle keyboard input
        keys = pynovage.key.get_pressed()
        
        if keys[pynovage.K_ESCAPE]:
            running = False
        elif keys[pynovage.K_SPACE]:
            # Reset to initial ball count
            balls = create_initial_balls(WIDTH, HEIGHT, 25)
            print(f"Reset to {len(balls)} balls")
        elif keys[pynovage.K_UP]:
            # Add 50 balls
            for _ in range(50):
                margin = 50
                x = random.uniform(margin, WIDTH - margin)
                y = random.uniform(margin, HEIGHT - margin)
                balls.append(Ball(x, y))
            print(f"Added 50 balls, total: {len(balls)}")
        elif keys[pynovage.K_DOWN]:
            # Remove 50 balls
            old_count = len(balls)
            balls = balls[:-50] if len(balls) > 50 else []
            print(f"Removed {old_count - len(balls)} balls, total: {len(balls)}")
        elif keys[pynovage.K_RIGHT]:
            # Add 10 balls
            for _ in range(10):
                margin = 50
                x = random.uniform(margin, WIDTH - margin)
                y = random.uniform(margin, HEIGHT - margin)
                balls.append(Ball(x, y))
            print(f"Added 10 balls, total: {len(balls)}")
        elif keys[pynovage.K_LEFT]:
            # Remove 10 balls
            old_count = len(balls)
            balls = balls[:-10] if len(balls) > 10 else []
            print(f"Removed {old_count - len(balls)} balls, total: {len(balls)}")
        
        # Update physics
        physics_start = time.perf_counter()
        for ball in balls:
            ball.update(dt, WIDTH, HEIGHT)
        
        # Collision detection (O(n²))
        collision_checks_this_frame = 0
        for i in range(len(balls)):
            for j in range(i + 1, len(balls)):
                ball_collision(balls[i], balls[j])
                collision_checks_this_frame += 1
        
        total_collision_checks += collision_checks_this_frame
        physics_end = time.perf_counter()
        physics_time = (physics_end - physics_start) * 1000  # milliseconds
        
        # Clear screen with background color
        pynovage.fill_screen(background_color)
        
        # Update display (but don't draw balls to focus on physics performance)
        pynovage.display.flip()
        
        # Update performance stats every second
        current_time = time.time()
        if current_time - fps_update_time >= 1.0:
            fps = clock.get_fps()
            avg_physics_time = physics_time
            title = (f"PyNovaGE Physics Benchmark - {len(balls)} balls | "
                    f"{fps:.1f} FPS | {collision_checks_this_frame} collision checks/frame | "
                    f"{avg_physics_time:.2f}ms physics/frame")
            pynovage.set_window_title(title)
            
            # Console output
            print(f"Frame {frame_count}: {len(balls)} balls, {fps:.1f} FPS, "
                  f"{collision_checks_this_frame} collisions, {avg_physics_time:.2f}ms physics")
            
            fps_update_time = current_time
    
    # Final stats
    avg_collision_checks = total_collision_checks / frame_count if frame_count > 0 else 0
    print(f"\nBenchmark finished!")
    print(f"Total frames: {frame_count}")
    print(f"Total collision checks: {total_collision_checks}")
    print(f"Average collision checks per frame: {avg_collision_checks:.1f}")
    
    # Cleanup
    pynovage.quit()


if __name__ == "__main__":
    main()