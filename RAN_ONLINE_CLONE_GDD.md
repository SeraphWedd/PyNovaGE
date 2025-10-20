# 🎓 Ran Online Clone - Game Design Document & Technical Implementation Guide

## Table of Contents
1. [Game Overview](#game-overview)
2. [Technical Architecture](#technical-architecture)
3. [Core Systems](#core-systems)
4. [Implementation Roadmap](#implementation-roadmap)
5. [Technical Specifications](#technical-specifications)

---

## 1. Game Overview 🎮

### 1.1 Genre & Theme
- **Genre**: 3D MMORPG
- **Setting**: Modern school life mixed with martial arts, magic, and gang warfare
- **Theme**: "College students with superpowers"
- **Target Era**: DirectX 8/9-era rendering (efficient, old-school MMO style)

### 1.2 Core Concept
Players are students belonging to one of four schools, engaging in:
- PvE combat against monsters
- PvP battles between rival schools
- Social gameplay through parties and guilds
- Character progression through grinding and skill development

### 1.3 Schools (Factions)
1. **Sacred Gate**
2. **Mystic Peak** 
3. **Phoenix**
4. **Leonair**

Each school has unique characteristics, equipment, and storylines.

---

## 2. Technical Architecture 🏗️

### 2.1 Engine Foundation
- **Base Engine**: PyNovaGE (already optimized with 11x performance improvements)
- **Rendering API**: OpenGL (DirectX 8/9-style fixed-function pipeline)
- **Platform**: Windows primary, cross-platform capable
- **Language**: C++17 with Python bindings
- **Architecture**: Client-server with authoritative server

### 2.2 Performance Targets
- **Players Visible**: 500-1000+ at 60 FPS
- **Concurrent NPCs**: 1000+ with AI updates
- **Combat Responsiveness**: Real-time with minimal lag
- **World Size**: Large campus environments with seamless zones

---

## 3. Core Systems 🔧

### 3.1 Camera System 📹

#### 3.1.1 Technical Requirements
```cpp
class RanOnlineCameraController {
    // Pivot-based camera attached to invisible anchor behind player
    Vector3f pivot_offset;           // Distance behind player
    float yaw_angle;                // Horizontal rotation
    float pitch_angle;              // Vertical rotation (clamped)
    float zoom_distance;            // Camera distance from pivot
    float smooth_factor;            // Interpolation smoothness
};
```

#### 3.1.2 Control Scheme
- **Right Mouse Button**: Hold and drag to rotate camera
- **Scroll Wheel**: Zoom in/out with limits
- **Recenter Key**: (Insert/Home) Reset camera behind character
- **Independent Movement**: Character and camera facing are separate

#### 3.1.3 Implementation Formula
```
camera_pos = player_pos + rotate(offset_vector, yaw, pitch) * zoom_factor
```

### 3.2 Movement System 🚶‍♂️

#### 3.2.1 Point-and-Click Movement
- **Primary Input**: Left-click to move
- **Pathfinding**: A* algorithm for obstacle avoidance
- **Character Rotation**: Smooth auto-facing toward movement direction
- **Animation Blending**: Walk/run/idle state transitions

#### 3.2.2 Alternative: WASD Mode
- **Modern Clients**: Optional keyboard movement
- **Camera Independence**: Character moves relative to camera view
- **Smooth Transitions**: Blend between control modes

#### 3.2.3 Technical Implementation
```cpp
class MovementSystem {
    PathfindingGrid world_grid;
    std::queue<Vector3f> movement_path;
    float movement_speed;
    Vector3f target_position;
    float rotation_speed;
    AnimationState current_anim;
};
```

### 3.3 Combat System ⚔️

#### 3.3.1 Combat Flow
1. **Target Selection**: Left-click enemy or tab-cycle
2. **Auto-Face**: Character smoothly rotates toward target
3. **Attack Request**: Client → Server validation
4. **Server Checks**:
   - Range validation
   - Cooldown ready
   - Hit chance calculation
5. **Result**: Server → Client damage/miss result
6. **Animation**: Client plays attack animation + effects

#### 3.3.2 Attack Calculations
```cpp
// Hit Chance Formula
hit_chance = base_accuracy + (DEX_bonus) - (target_dodge + DEX_defense)
bool hit = random(1-100) <= hit_chance

// Damage Formula  
base_damage = weapon_damage + (STR_scaling * STR_stat)
final_damage = max(1, base_damage - target_defense)
critical_damage = final_damage * crit_multiplier (if crit)
```

#### 3.3.3 Combat States
- **Idle**: Ready to attack
- **Attacking**: Animation playing, cooldown active
- **Casting**: Skill activation (interruptible)
- **Stunned**: Cannot act
- **Moving**: Can cancel into attack

### 3.4 Skill System 🔮

#### 3.4.1 Skill Types
1. **Single Target**: Direct damage/effect to one enemy
2. **Area of Effect (AoE)**: Affects multiple targets in radius
3. **Buff**: Positive effect on self/allies
4. **Debuff**: Negative effect on enemies
5. **Passive**: Permanent character enhancement

#### 3.4.2 Skill Properties
```cpp
struct Skill {
    uint32_t skill_id;
    std::string name;
    SkillType type;
    
    // Casting
    float cast_time;              // Animation duration
    float cooldown;              // Time before reuse
    int mp_cost;                 // MP consumption
    int sp_cost;                 // SP (stamina) consumption
    
    // Effects
    int base_damage;
    float stat_scaling;          // STR/INT modifier
    float range;                 // Effective distance
    float aoe_radius;            // For AoE skills
    float duration;              // For buffs/debuffs
    
    // Visual
    std::string animation_name;
    std::string effect_name;
    bool sync_with_server;       // Timing synchronization
};
```

#### 3.4.3 Skill Execution Flow
1. **Input**: Player presses skill hotkey
2. **Validation**: Check MP/SP, cooldown, target
3. **Cast Start**: Begin animation, show casting bar
4. **Server Request**: Send skill packet
5. **Server Processing**: Validate and calculate effects
6. **Effect Application**: Server sends results
7. **Visual Effects**: Client shows skill effects
8. **Cooldown**: Start cooldown timer

### 3.5 Character Statistics 📊

#### 3.5.1 Primary Stats
| Stat | Full Name | Primary Effects |
|------|-----------|----------------|
| **STR** | Strength | Melee damage, weapon requirements, carrying capacity |
| **DEX** | Dexterity | Hit rate, evasion, critical rate, ranged damage |
| **INT** | Intelligence | Skill damage, MP pool, magic defense |
| **VIT** | Vitality | Max HP, physical defense, HP regeneration |
| **CHA** | Charm | Pet bonuses, item drop rates, some skill effects |

#### 3.5.2 Derived Stats
```cpp
struct DerivedStats {
    // Health & Resources
    int max_hp = base_hp + (VIT * hp_per_vit);
    int max_mp = base_mp + (INT * mp_per_int);
    int max_sp = base_sp + (VIT * sp_per_vit);
    
    // Combat Stats
    int physical_attack = weapon_damage + (STR * str_scaling);
    int magical_attack = skill_power + (INT * int_scaling);
    int accuracy = base_accuracy + (DEX * accuracy_per_dex);
    int evasion = base_evasion + (DEX * evasion_per_dex);
    int critical_rate = base_crit + (DEX * crit_per_dex);
    
    // Defense
    int physical_defense = armor_value + (VIT * def_per_vit);
    int magical_defense = base_mdef + (INT * mdef_per_int);
};
```

#### 3.5.3 Character Classes & Stat Focus
1. **Swordsman**: STR primary, VIT secondary (Tank/DPS)
2. **Brawler**: STR/DEX hybrid (Fast melee DPS)
3. **Archer**: DEX primary, STR secondary (Ranged physical)
4. **Gunner**: DEX primary, INT secondary (Ranged hybrid)
5. **Shaman**: INT primary, VIT secondary (Magic DPS/Support)

### 3.6 Rendering Pipeline 🎨

#### 3.6.1 Fixed-Function Rendering (DirectX 8/9 Style)
- **No Advanced Shaders**: Per-vertex lighting only
- **Simple Materials**: Diffuse textures with basic alpha blending
- **Efficient Sorting**: Opaque → Alpha-blended → UI layers
- **Low-Poly Models**: Optimized for older hardware

#### 3.6.2 Rendering Components
```cpp
class RanOnlineRenderer {
    // World Rendering
    void RenderTerrain();          // Static environment
    void RenderBuildings();        // Campus structures
    void RenderCharacters();       // Players and NPCs
    void RenderEffects();          // Skill effects, particles
    
    // Effect Systems
    ParticleSystem skill_effects;  // Energy trails, explosions
    BillboardSystem projectiles;   // Flying effects
    AlphaBlending aura_effects;    // Character buffs/debuffs
};
```

#### 3.6.3 Visual Effects
- **Skill Effects**: Particle systems for magic/combat skills
- **Damage Numbers**: 2D text overlay with animation
- **Status Indicators**: Icons above character heads
- **Environmental**: Simple lighting, no dynamic shadows initially

### 3.7 World Systems 🌍

#### 3.7.1 Map Structure
- **Tile-Based**: Grid-based world with pre-baked collision
- **Zone System**: Campus areas, dungeons, PvP zones
- **Seamless Transitions**: No loading screens between adjacent areas
- **Instanced Content**: Private dungeons for parties

#### 3.7.2 Campus Environments
1. **School Buildings**: Classrooms, libraries, cafeterias
2. **Outdoor Areas**: Courtyards, sports fields, gardens  
3. **Special Zones**: PvP arenas, boss areas, event locations
4. **Dungeons**: Instanced challenging content

### 3.8 Networking Architecture 🌐

#### 3.8.1 Client-Server Model
- **Authoritative Server**: All game state validation server-side
- **Client Prediction**: Immediate visual feedback with server confirmation
- **Lag Compensation**: Interpolation and extrapolation for smooth gameplay
- **Anti-Cheat**: Server-side validation of all actions

#### 3.8.2 Network Protocols
```cpp
// Core Network Messages
enum NetworkMessageType {
    // Movement
    MSG_MOVE_REQUEST,
    MSG_MOVE_UPDATE,
    
    // Combat
    MSG_ATTACK_REQUEST,
    MSG_ATTACK_RESULT,
    MSG_SKILL_CAST,
    MSG_SKILL_EFFECT,
    
    // World
    MSG_PLAYER_ENTER,
    MSG_PLAYER_LEAVE,
    MSG_WORLD_UPDATE,
    
    // Chat & Social
    MSG_CHAT,
    MSG_PARTY_INVITE,
    MSG_GUILD_ACTION
};
```

#### 3.8.3 Synchronization
- **Position Updates**: 20Hz for smooth movement
- **Combat Events**: Immediate with confirmation
- **World State**: Periodic full synchronization
- **Chat/UI**: Best-effort delivery

---

## 4. Implementation Roadmap 🛣️

### Phase 1: Core Foundation (Weeks 1-2)
1. **Camera System**: 3rd person pivot-based camera with mouse controls
2. **Movement System**: Point-and-click pathfinding with character rotation
3. **Basic Rendering**: Fixed-function pipeline with simple models
4. **Input Handling**: Mouse/keyboard integration

### Phase 2: Combat Basics (Weeks 3-4)
1. **Combat System**: Server-authoritative attack validation
2. **Stat System**: STR/DEX/INT/VIT/CHA with calculations
3. **Animation System**: Basic attack/walk/idle animations
4. **UI Foundation**: Health bars, hotkeys, basic interface

### Phase 3: Skills & Effects (Weeks 5-6)
1. **Skill System**: Cast times, cooldowns, MP/SP costs
2. **Effect System**: Particle effects for skills
3. **Damage Display**: Floating damage numbers
4. **Status Effects**: Buffs, debuffs, duration tracking

### Phase 4: World & Content (Weeks 7-8)
1. **Map System**: Campus environments with collision
2. **NPC System**: Basic AI and interaction
3. **Quest System**: Simple kill/fetch quests
4. **Zone Management**: Area transitions and loading

### Phase 5: Multiplayer & Polish (Weeks 9-10)
1. **Networking**: Complete client-server implementation  
2. **Party System**: Group mechanics and shared experience
3. **PvP System**: School-based faction combat
4. **Performance Optimization**: Final tuning and testing

---

## 5. Technical Specifications 📋

### 5.1 File Structure
```
engine/
├── ran_online/
│   ├── camera/              # 3rd person camera system
│   ├── movement/            # Point-click pathfinding
│   ├── combat/              # Attack/skill systems
│   ├── character/           # Stats, classes, progression
│   ├── world/               # Maps, NPCs, environment
│   ├── effects/             # Particles, visual effects
│   ├── ui/                  # Interface systems
│   └── network/             # Client-server networking
└── examples/
    └── ran_online_demo/     # Playable demo
```

### 5.2 Key Classes
```cpp
// Core Game Classes
class RanOnlineGame;           // Main game controller
class Player;                  // Player character representation
class NPC;                     // Non-player character
class CombatSystem;            // Attack/skill management  
class WorldManager;            // Map and zone handling
class NetworkManager;          // Client-server communication

// Specialized Systems
class CameraController;        // 3rd person camera
class PathfindingSystem;       // Movement and navigation
class SkillManager;           // Skill casting and effects
class StatSystem;             // Character statistics
class EffectRenderer;         // Visual effects pipeline
```

### 5.3 Performance Targets
- **Minimum FPS**: 60 FPS with 100+ players visible
- **Network Latency**: <100ms response time for combat
- **Memory Usage**: <2GB for full client
- **Loading Times**: <3 seconds for zone transitions

### 5.4 Art Requirements
- **Character Models**: 1000-3000 triangles per character
- **Texture Resolution**: 256x256 for characters, 512x512 for environments
- **Animation**: 15-30 FPS keyframe animation
- **Effects**: Simple particle systems with alpha blending

---

## 6. Success Criteria ✅

### 6.1 Core Functionality
- [ ] Smooth 3rd person camera with mouse controls
- [ ] Reliable point-and-click movement with pathfinding  
- [ ] Real-time combat with server validation
- [ ] Complete skill system with visual effects
- [ ] Character progression with stat distribution
- [ ] Multiplayer support with 50+ concurrent players

### 6.2 Performance Benchmarks
- [ ] 60+ FPS with recommended settings
- [ ] <100ms combat response time
- [ ] Stable networking with minimal lag
- [ ] Efficient memory usage (<2GB)

### 6.3 Content Milestones
- [ ] One complete campus area
- [ ] All 5 character classes playable
- [ ] Basic skill trees for each class
- [ ] Party system with shared experience
- [ ] PvP combat between schools
- [ ] Simple quest and progression system

---

This document serves as our north star for implementation. Each system should be built according to these specifications to ensure we create an authentic Ran Online experience! 🎓⚔️