# PyNovaGE (Python Nova Game Engine)

A high-performance game engine combining C++ performance with Python's ease of use, optimized for 2D to 2.5D game development with support for basic voxel-style 3D games.

## Scope & Features

### Primary Focus
- High-performance 2D game development
- 2.5D game capabilities (isometric, side-scrolling 3D)
- Basic voxel-based 3D game support (Minecraft/Roblox style)
- SIMD-optimized math operations
- Efficient memory management
- Python-friendly API

### Target Use Cases
- 2D Games: Platformers, top-down games, sprite-based games
- 2.5D Games: Isometric games, side-scrolling 3D
- Simple 3D: Voxel-based games, block-style worlds
- Educational: Game development learning and prototyping

## Structure

- engine/
  - foundation/ (Track A)
    - memory/
    - math/
    - containers/
  - core/ (Track B)
    - window/
    - render/
    - physics/
  - systems/ (Track C)
    - asset/
    - scene/
    - input/
    - audio/
- python/
  - bindings/
  - api/
- tools/
- tests/
- docs/
- cmake/

## Performance Targets

### Key Goals
- Optimized memory management with minimal fragmentation
- Fast and efficient math operations using SIMD
- High cache utilization and reduced CPU overhead
- Minimal Python binding overhead

### Runtime Targets
- Smooth performance for complex 2D scenes
- Stable framerate for typical voxel worlds
- Quick response time for standard 2D/2.5D operations
- Efficient Python-C++ interop
