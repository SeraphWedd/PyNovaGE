#!/usr/bin/env python3
"""
Visual Balls Demo - Simple Version

A PyNovaGE example demonstrating physics simulation with visual feedback.
This version uses color variations to show ball activity since shape drawing
is not yet fully implemented.

Controls:
- Arrow keys: Add/remove balls
  - Up/Down: Add/remove 50 balls
  - Left/Right: Add/remove 10 balls
- Space: Reset to initial ball count
- Escape: Exit

The background color changes based on collision activity.
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
        bounced = False
        if self.x - self.radius <= 0:
            self.x = self.radius
            self.vx = -self.vx * 0.8  # Energy loss on bounce
            bounced = True
        elif self.x + self.radius >= screen_width:
            self.x = screen_width - self.radius
            self.vx = -self.vx * 0.8
            bounced = True
            
        if self.y - self.radius <= 0:
            self.y = self.radius
            self.vy = -self.vy * 0.8
            bounced = True
        elif self.y + self.radius >= screen_height:
            self.y = screen_height - self.radius
            self.vy = -self.vy * 0.8
            bounced = True
        
        return bounced


def ball_collision(ball1, ball2):
    """Handle collision between two balls. Returns True if collision occurred."""
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
            return False
        
        # Collision impulse (simplified, assumes equal mass)
        impulse = 2 * dvn / 2
        ball1.vx += impulse * dx * 0.8  # Energy loss
        ball1.vy += impulse * dy * 0.8
        ball2.vx -= impulse * dx * 0.8
        ball2.vy -= impulse * dy * 0.8
        
        return True
    
    return False


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


def get_activity_color(collisions, max_collisions):
    """Get background color based on collision activity."""
    if max_collisions == 0:
        intensity = 0
    else:
        intensity = min(collisions / max_collisions, 1.0)
    
    # Base dark blue color
    base_r, base_g, base_b = 20, 20, 30
    
    # Add red/orange tint based on activity
    activity_r = int(base_r + intensity * 100)  # More red with activity
    activity_g = int(base_g + intensity * 50)   # Slight green increase
    activity_b = base_b                         # Keep blue constant
    
    return pynovage.Color(
        min(activity_r, 255),
        min(activity_g, 255), 
        activity_b
    )


def draw_ball_indicators(screen, balls, width, height):
    """Draw simple indicators for ball positions using screen fills."""
    # This is a workaround since shape drawing isn't implemented yet
    # We'll divide the screen into regions and color them based on ball density
    
    regions_x, regions_y = 20, 15  # Grid of regions
    region_width = width // regions_x
    region_height = height // regions_y
    
    # Count balls in each region
    region_counts = {}
    for ball in balls:
        region_x = int(ball.x // region_width)
        region_y = int(ball.y // region_height)
        region_x = max(0, min(region_x, regions_x - 1))
        region_y = max(0, min(region_y, regions_y - 1))
        
        key = (region_x, region_y)
        region_counts[key] = region_counts.get(key, 0) + 1
    
    # For visualization, we'll just return the maximum density found
    max_density = max(region_counts.values()) if region_counts else 0
    return max_density


def main():
    """Main game loop."""
    # Initialize PyNovaGE
    pynovage.init()
    
    # Screen configuration
    WIDTH, HEIGHT = 1200, 800
    screen = pynovage.display.set_mode((WIDTH, HEIGHT))
    
    # Initialize game objects
    balls = create_initial_balls(WIDTH, HEIGHT, 10)  # Start with 10 balls
    
    # Timing and stats
    clock = pynovage.Clock()
    running = True
    frame_count = 0
    fps_update_time = time.time()
    
    # Activity tracking
    max_possible_collisions = 1000  # For color scaling
    collision_history = []
    
    print(f"Starting visual balls demo with {len(balls)} balls")
    print("Controls:")
    print("  Up/Down arrows: Add/remove 50 balls")  
    print("  Left/Right arrows: Add/remove 10 balls")
    print("  Space: Reset to 10 balls")
    print("  Escape: Exit")
    print("Note: Background color changes with collision activity!")
    print("      Ball positions affect color intensity in different screen regions")
    
    while running:
        dt = clock.tick(60) / 1000.0  # Convert to seconds, target 60 FPS
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
            balls = create_initial_balls(WIDTH, HEIGHT, 10)
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
        wall_bounces = 0
        for ball in balls:
            if ball.update(dt, WIDTH, HEIGHT):
                wall_bounces += 1
        
        # Collision detection
        ball_collisions = 0
        for i in range(len(balls)):
            for j in range(i + 1, len(balls)):
                if ball_collision(balls[i], balls[j]):
                    ball_collisions += 1
        
        # Track collision activity
        total_collisions = wall_bounces + ball_collisions
        collision_history.append(total_collisions)
        if len(collision_history) > 30:  # Keep last 30 frames
            collision_history.pop(0)
        
        # Calculate average activity
        avg_collisions = sum(collision_history) / len(collision_history) if collision_history else 0
        
        # Get ball density indicator
        ball_density = draw_ball_indicators(screen, balls, WIDTH, HEIGHT)
        
        # Calculate background color based on activity
        # Combine collision activity and ball density
        combined_activity = avg_collisions + ball_density * 2
        background_color = get_activity_color(combined_activity, max_possible_collisions / 10)
        
        # Add some variation based on ball positions
        if balls:
            # Add slight color variation based on center of mass
            center_x = sum(ball.x for ball in balls) / len(balls)
            center_y = sum(ball.y for ball in balls) / len(balls)
            
            # Normalize to 0-1
            center_x_norm = center_x / WIDTH
            center_y_norm = center_y / HEIGHT
            
            # Modify color slightly
            color_mod_r = int(center_x_norm * 30)
            color_mod_g = int(center_y_norm * 30)
            
            final_color = pynovage.Color(
                min(background_color.r + color_mod_r, 255),
                min(background_color.g + color_mod_g, 255),
                background_color.b
            )
        else:
            final_color = background_color
        
        # Clear screen with activity-based color
        pynovage.fill_screen(final_color)
        
        # Update display
        pynovage.display.flip()
        
        # Update window title every second
        current_time = time.time()
        if current_time - fps_update_time >= 1.0:
            fps = clock.get_fps()
            collision_checks = len(balls) * (len(balls) - 1) // 2
            title = (f"PyNovaGE Visual Demo - {len(balls)} balls | {fps:.1f} FPS | "
                    f"Collisions: {total_collisions} | Avg Activity: {avg_collisions:.1f}")
            pynovage.set_window_title(title)
            fps_update_time = current_time
    
    # Cleanup
    print(f"\nDemo finished after {frame_count} frames with {len(balls)} balls")
    pynovage.quit()


if __name__ == "__main__":
    main()