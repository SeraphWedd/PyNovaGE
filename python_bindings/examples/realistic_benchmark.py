#!/usr/bin/env python3
"""
Realistic Physics + Rendering Benchmark

Uses the pygame-like API but actually draws all the balls to get realistic performance.
This tests both physics simulation AND rendering in a real game scenario.

Controls:
- Arrow keys: Add/remove balls
  - Up/Down: Add/remove 50 balls
  - Left/Right: Add/remove 10 balls
- Space: Reset to initial ball count
- Escape: Exit
"""

import os, sys
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import pynovage
import random
import math
import time


class VisualBall:
    """A ball with both physics and visual rendering."""
    
    def __init__(self, x, y, radius=None, vx=None, vy=None):
        self.x = x
        self.y = y
        self.radius = radius or random.uniform(6, 20)
        self.vx = vx or random.uniform(-250, 250)
        self.vy = vy or random.uniform(-250, 250)
        
        # Visual properties
        self.color = pynovage.Color(
            random.randint(100, 255),
            random.randint(100, 255), 
            random.randint(100, 255)
        )
    
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
    
    def draw(self, screen):
        """Draw the ball to the screen using rectangles."""
        # Draw filled rectangle for the ball (approximation of circle)
        rect_size = int(self.radius * 2)
        rect_x = int(self.x - self.radius)
        rect_y = int(self.y - self.radius)
        
        # Draw filled rectangle
        pynovage.draw.rect(screen, self.color, 
                          (rect_x, rect_y, rect_size, rect_size))
        
        # Draw a small border for better visibility
        border_color = pynovage.Color(
            max(0, self.color.r - 50),
            max(0, self.color.g - 50),
            max(0, self.color.b - 50)
        )
        pynovage.draw.rect(screen, border_color, 
                          (rect_x - 1, rect_y - 1, rect_size + 2, rect_size + 2), 2)


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
            dx, dy = 1.0, 0.0
        
        # Separate balls
        overlap = ball1.radius + ball2.radius - distance
        ball1.x -= dx * overlap * 0.5
        ball1.y -= dy * overlap * 0.5
        ball2.x += dx * overlap * 0.5
        ball2.y += dy * overlap * 0.5
        
        # Calculate collision response
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


def create_initial_balls(screen_width, screen_height, count=100):
    """Create initial set of visual balls."""
    balls = []
    for _ in range(count):
        # Ensure balls spawn away from edges
        margin = 50
        x = random.uniform(margin, screen_width - margin)
        y = random.uniform(margin, screen_height - margin)
        balls.append(VisualBall(x, y))
    return balls


def main():
    """Main simulation loop with full rendering."""
    print("🚀 PyNovaGE Realistic Physics + Rendering Benchmark")
    print("=" * 60)
    
    # Initialize PyNovaGE
    pynovage.init()
    
    # Screen configuration
    WIDTH, HEIGHT = 1400, 900
    screen = pynovage.display.set_mode((WIDTH, HEIGHT))
    # Note: set_caption not available in this API
    
    # Initialize simulation objects - start with fewer balls for realistic test
    balls = create_initial_balls(WIDTH, HEIGHT, 100)  # Start with 100 balls
    
    # Timing and performance tracking
    clock = pynovage.Clock()
    running = True
    frame_count = 0
    fps_update_time = time.time()
    total_collision_checks = 0
    
    # Performance tracking
    physics_times = []
    render_times = []
    
    # Colors
    background_color = pynovage.Color(255, 0, 255)  # Bright magenta background to test
    text_color = pynovage.Color(255, 255, 255)
    
    print(f"Starting realistic benchmark with {len(balls)} balls")
    print("This test includes BOTH physics simulation AND visual rendering")
    print("Controls:")
    print("  Up/Down arrows: Add/remove 50 balls")  
    print("  Left/Right arrows: Add/remove 10 balls")
    print("  Space: Reset to 100 balls")
    print("  Escape: Exit")
    print()
    
    while running:
        dt = clock.tick(120) / 1000.0  # Convert to seconds
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
            balls = create_initial_balls(WIDTH, HEIGHT, 100)
            print(f"Reset to {len(balls)} balls")
        elif keys[pynovage.K_UP]:
            for _ in range(50):
                margin = 50
                x = random.uniform(margin, WIDTH - margin)
                y = random.uniform(margin, HEIGHT - margin)
                balls.append(VisualBall(x, y))
            print(f"Added 50 balls, total: {len(balls)}")
        elif keys[pynovage.K_DOWN]:
            old_count = len(balls)
            balls = balls[:-50] if len(balls) > 50 else []
            print(f"Removed {old_count - len(balls)} balls, total: {len(balls)}")
        elif keys[pynovage.K_RIGHT]:
            for _ in range(10):
                margin = 50
                x = random.uniform(margin, WIDTH - margin)
                y = random.uniform(margin, HEIGHT - margin)
                balls.append(VisualBall(x, y))
            print(f"Added 10 balls, total: {len(balls)}")
        elif keys[pynovage.K_LEFT]:
            old_count = len(balls)
            balls = balls[:-10] if len(balls) > 10 else []
            print(f"Removed {old_count - len(balls)} balls, total: {len(balls)}")
        
        # === PHYSICS UPDATE ===
        physics_start = time.perf_counter()
        
        # Update ball positions
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
        physics_times.append(physics_time)
        
        # === RENDERING ===
        render_start = time.perf_counter()
        
        # Clear screen using the working fill_screen function
        pynovage.fill_screen(background_color)
        
        # Draw all balls using the ball's draw method
        for ball in balls:
            ball.draw(screen)
        
        # Draw performance info on screen
        if frame_count % 10 == 0:  # Update text every 10 frames for performance
            fps = clock.get_fps()
            avg_physics = sum(physics_times[-60:]) / min(len(physics_times), 60) if physics_times else 0
            avg_render = sum(render_times[-60:]) / min(len(render_times), 60) if render_times else 0
            
            # Note: Window title updates not available in pygame-like API
            # title = f"PyNovaGE Realistic Benchmark | {len(balls)} balls | {fps:.1f} FPS | P:{avg_physics:.1f}ms R:{avg_render:.1f}ms"
        
        # Update display
        pynovage.display.flip()
        
        render_end = time.perf_counter()
        render_time = (render_end - render_start) * 1000  # milliseconds
        render_times.append(render_time)
        
        # Console performance updates every second
        current_time = time.time()
        if current_time - fps_update_time >= 1.0:
            fps = clock.get_fps()
            avg_physics_time = sum(physics_times[-60:]) / min(len(physics_times), 60) if physics_times else 0
            avg_render_time = sum(render_times[-60:]) / min(len(render_times), 60) if render_times else 0
            total_frame_time = avg_physics_time + avg_render_time
            
            print(f"🎯 {len(balls)} balls | {fps:.1f} FPS | "
                  f"Physics: {avg_physics_time:.1f}ms | Render: {avg_render_time:.1f}ms | "
                  f"Collisions: {collision_checks_this_frame} | Total: {total_frame_time:.1f}ms")
            
            fps_update_time = current_time
    
    # Final performance summary
    if physics_times and render_times:
        avg_physics = sum(physics_times) / len(physics_times)
        avg_render = sum(render_times) / len(render_times)
        total_avg = avg_physics + avg_render
        
        print(f"\n🏁 REALISTIC BENCHMARK RESULTS:")
        print(f"   Final ball count: {len(balls)}")
        print(f"   Average physics time: {avg_physics:.2f}ms")
        print(f"   Average render time: {avg_render:.2f}ms")
        print(f"   Average TOTAL frame time: {total_avg:.2f}ms")
        print(f"   Equivalent FPS: {1000/total_avg:.1f} FPS")
        print(f"   Physics vs Render ratio: {avg_physics/avg_render:.1f}:1")
        print(f"   Total collision checks: {total_collision_checks:,}")
    
    # Cleanup
    pynovage.quit()
    print("✅ Realistic benchmark completed successfully!")


if __name__ == "__main__":
    main()