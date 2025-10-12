# PyNovaGE Python Examples

This directory contains working examples demonstrating the PyNovaGE game engine with Python bindings.

## Examples

### physics_benchmark_simple.py
**Pure Physics Performance Test**

A computational physics benchmark that focuses on testing the engine's physics simulation performance without graphics rendering. This example demonstrates:

- Real-time physics simulation with multiple balls
- Ball-to-ball collision detection and response
- Wall collision handling with energy loss
- Dynamic object addition/removal via keyboard controls
- Performance monitoring (FPS, collision timing, collision count)

**Controls:**
- `Up/Down arrows`: Add/remove 50 balls
- `Left/Right arrows`: Add/remove 10 balls 
- `Space`: Reset to 25 balls
- `Escape`: Exit

**Features:**
- Handles 500+ balls at 60+ FPS
- Physics computation typically takes 0.3-1.4ms per frame
- Console output shows detailed performance metrics
- Window title displays real-time statistics

### visual_balls_demo.py
**Visual Physics Demonstration**

A visual physics demonstration that uses background color changes to show collision activity and ball movement patterns. This example demonstrates:

- Real-time physics simulation with visual feedback
- Dynamic background colors based on collision activity
- Color variations representing ball density and movement
- Interactive ball management

**Controls:**
- `Up/Down arrows`: Add/remove 50 balls
- `Left/Right arrows`: Add/remove 10 balls
- `Space`: Reset to 10 balls
- `Escape`: Exit

**Visual Feedback:**
- Background color intensity increases with collision activity
- Red/orange tints indicate high activity levels
- Color variations show ball center-of-mass changes
- Smooth color transitions provide fluid visual experience

## Running the Examples

Make sure you have built the PyNovaGE Python bindings first:

```bash
cd python_bindings
python setup.py build_ext --inplace
```

Then run any example:

```bash
python examples/physics_benchmark_simple.py
python examples/visual_balls_demo.py
```

## Performance Expectations

Both examples are designed to demonstrate excellent performance:

- **60+ FPS** with dozens of balls
- **300+ FPS** with minimal balls
- Sub-millisecond physics computation times
- Smooth, responsive interactive controls
- Stable memory usage even with 500+ objects

## Technical Notes

- Both examples use the pygame-like PyNovaGE interface for familiar API
- Physics simulation uses simple ball-ball collision detection (O(n²))
- Wall collisions include energy loss simulation (damping factor 0.8)
- Examples demonstrate proper resource management and cleanup
- Compatible with Windows, built using Visual Studio 2022

## Future Enhancements

- Shape drawing (circles, rectangles) once BatchRenderer binding issues are resolved
- Texture-based sprite rendering for balls
- Spatial partitioning for O(n log n) collision detection
- Additional physics properties (mass, friction, restitution)
- Sound effects and visual particle effects