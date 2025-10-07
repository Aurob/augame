# AuGame Technical Documentation

## Executive Summary
AuGame is a browser-native 2D game engine that compiles C++ to WebAssembly, featuring an Entity-Component-System (ECS) architecture, custom physics simulation, dynamic shader loading, and a unique human-readable scene configuration language. The engine emphasizes spatial interaction through its novel interior/portal system and supports multi-scene management with seamless transitions.

## Architecture Deep Dive

### Technology Stack
- **Core Engine**: C++17 with Emscripten (WebAssembly target)
- **ECS Framework**: EnTT v3.x for entity management
- **Graphics**: OpenGL ES 2.0 with custom shader pipeline
- **Physics**: Custom 2D rectangle-based physics engine
- **UI/Input**: SDL2 for windowing/events, SDL_ttf for text
- **Frontend**: Vanilla JavaScript with ES6 modules
- **Build System**: Shell script wrapper around emcc
- **Distribution**: Redbean APE for self-contained executable

### System Architecture

#### Entity-Component-System (ECS)
The engine uses EnTT to implement a pure ECS architecture where:
- **Entities**: Opaque identifiers (entt::entity)
- **Components**: POD structs containing only data (no methods)
- **Systems**: Functions that process entities with specific component combinations
- **Registry**: Container managing entity-component relationships

#### Component Taxonomy
**Core Components**:
- `Position {x, y, z, sx, sy, sz}`: World and screen coordinates
- `Shape {size[3], scaled_size[3]}`: 3D bounding box
- `Color {r, g, b, a}`: RGBA values [0-1]
- `Movement {speed, mass, restitution, friction}`: Physics properties

**Rendering Components**:
- `Texture {name, x, y, w, h, scalex, scaley}`: Texture atlas coordinates
- `TextureAlts {current, textures[]}`: Multi-state textures
- `CustomShader {shaderName, uniformCount, uniforms[]}`: Per-entity shaders
- `RenderPriority {priority}`: Z-ordering

**Interaction Components**:
- `Hoverable`: Mouse-over detection
- `Interactable {radius, toggleState}`: Click/tap interaction
- `Draggable {radius}`: Mouse drag support
- `Keys {keys[]}`: Input state storage

**Spatial Components**:
- `Interior {showInside}`: Defines contained space
- `Inside {interiorEntity, showOutside}`: Marks containment
- `InteriorPortal {A, B, locked, key}`: Space transitions

**Special Components**:
- `Player`: Singleton player marker
- `Camera {gridSpacing, priority, radius}`: View control
- `Text {text, scale, hide, offsetX, offsetY}`: Text rendering

#### System Pipeline
Systems execute in strict order each frame:
1. **Input Systems**: Process SDL events → update Keys components
2. **Physics Systems**: Update positions, detect collisions, resolve constraints
3. **View Systems**: Calculate screen positions from world coordinates
4. **Action Systems**: Process interactions (hover, click, drag)
5. **Render Systems**: Multi-pass rendering to framebuffer

#### Rendering Pipeline
**Pass Structure**:
1. **Clear Pass**: Clear color based on context (void color)
2. **Background Pass**: Terrain shader or solid color
3. **Entity Pass**: Sort by RenderPriority, render each entity
4. **Text Pass**: SDL_ttf generated textures with shader coloring
5. **Wall Pass**: Interior walls with context-based visibility
6. **UI Pass**: Screen-space overlays

**Shader Management**:
- **Static Shaders**: Compiled into binary (font, terrain, texture, debug_entity, ui_layer)
- **Dynamic Shaders**: Loaded from `resources/cshaders/` at runtime
- **Uniform System**: Standard uniforms (uTime, uResolution, uSeed, uColor, uCenterPos)
- **Per-Entity Shaders**: CustomShader component assigns shader per entity

#### Physics Engine
**Custom 2D Physics** (`lib/physics.hpp`):
- **Shapes**: Rectangle-only collision detection
- **Integration**: Euler integration with fixed timestep
- **Collision Response**: Impulse-based with restitution
- **Constraints**: Interior boundaries, portal transitions
- **Optimization**: No spatial partitioning (brute force)

#### Scene Management
**Multi-Scene System**:
- **Scene Registry**: Each scene owns an entt::registry
- **Metadata**: Per-scene configuration (terrain, void, font)
- **Transitions**: Shift+< / Shift+> for prev/next
- **Player Persistence**: Player entity recreated per scene
- **Scene Loading**: Dynamic from config files or inline

### Configuration System

#### Configuration Language Specification
**Entity Syntax**:
```
id <int> <string> position <x> <y> <z> shape <w> <h> <d> [components...]
```

**Component Syntax**:
```
color <r> <g> <b> [a]
texture <name> [scalex] [scaley] [x] [y] [w] [h]
movement <speed> <mass> <restitution> <friction>
interactable [radius] [toggleState]
interiorPortal <entityA> <entityB> <locked> <keyEntity>
cshader <name> [uniform1] [uniform2] [seed|~]
text "string with spaces" <scale> <hide> [offsetX] [offsetY]
```

**Meta Tags**:
```
meta terrain <"shader"|"#hex"|"r,g,b">
meta void <"#hex"|"r,g,b">
meta scene <"@file.txt"|"name">
meta font <"filename.ttf">
meta start_menu <"text"|"@file.txt">
meta pause_menu <"text"|"@file.txt">
meta slides <"scene1","scene2","@file.txt">
meta seed <"string">
```

#### Entity Builder Pipeline
1. **Parsing** (`ebuilder.js`):
   - Tokenization (space-delimited, quote-aware)
   - Component identification
   - Parameter extraction
   - JSON generation

2. **Transmission**:
   - JavaScript fetches configs
   - Parses to JSON
   - Calls C++ via `ccall`

3. **Entity Creation** (`WebUtils.hpp`):
   - JSON deserialization
   - Component instantiation via `safe_emplace`
   - Registry population
   - Relationship resolution (portals, interiors)

### JavaScript-WebAssembly Bridge

#### Exported C++ Functions
```cpp
EMSCRIPTEN_KEEPALIVE functions:
- isready(): Signal engine initialization complete
- reload(): Reset engine state
- load_json(char* str): Load entity configuration
- createSceneFromJson(char* str): Add new scene
- switchToNextScene(): Scene navigation
- switchToPrevScene(): Scene navigation
```

#### JavaScript Modules
**module.js**:
- WebAssembly loading and initialization
- Config file fetching with cache busting
- Event listener setup
- Mobile/desktop input normalization

**ebuilder.js**:
- EntityBuilder class
- Component parsers (32 types)
- Meta tag processing
- JSON structure generation

**config.js**:
- Shader definitions
- Texture mappings
- Sprite sheet configuration

### Interior/Portal System

#### Spatial Containment Model
The interior system creates bounded spaces with controlled visibility:

**Interior Entity**: Defines a rectangular bounded space
- Component: `Interior {showInside}`
- Creates physics boundaries
- Controls rendering context

**Contained Entities**: Exist within interior spaces
- Component: `Inside {interiorEntity, showOutside}`
- Position relative to interior
- Visibility controlled by context

**Portal System**: Transitions between spaces
- Component: `InteriorPortal {A, B, locked, key}`
- Bidirectional links
- Lock/key mechanism
- Automatic player transfer

#### Rendering Context
Player location determines rendering:
- **Outside**: Terrain shader, all exterior entities visible
- **Inside**: Void color, only same-interior entities visible
- **Walls**: Rendered based on player's interior context

### Performance Characteristics

#### Memory Usage
- **Entity Overhead**: ~64 bytes per entity (EnTT)
- **Component Storage**: Packed arrays per component type
- **Texture Memory**: Shared texture atlas
- **Physics Bodies**: ~128 bytes per collidable entity

#### Processing Complexity
- **ECS Iteration**: O(n) where n = entities with component
- **Physics**: O(n²) collision detection (no spatial partitioning)
- **Rendering**: O(n log n) due to priority sorting
- **Scene Switch**: O(n) entity creation

#### Optimization Opportunities
1. **Spatial Partitioning**: Quadtree for physics/rendering
2. **Component Pools**: Pre-allocate common components
3. **Texture Batching**: Reduce draw calls
4. **Culling**: Frustum and occlusion culling
5. **LOD System**: Distance-based detail reduction

## File Structure Analysis

### Core Engine Files
```
src/main.cpp (522 lines)          - Game loop, initialization
include/structs.hpp (406 lines)   - All component definitions
include/GLUtils.hpp (1749 lines)  - Complete rendering pipeline
include/WebUtils.hpp (736 lines)  - Config parsing, entity creation
include/events.hpp (355 lines)    - Input handling
include/EntityFactory.hpp (158 lines) - Entity templates
include/SceneManager.hpp (125 lines) - Multi-scene management
```

### System Implementations
```
include/Systems/ViewSystems.hpp (110 lines)    - Position calculations
include/Systems/ActionSystems.hpp (325 lines)  - Interaction processing
include/Systems/PhysicsSystems.hpp (408 lines) - Physics simulation
include/Systems/MoveSystems.hpp (191 lines)    - Movement logic
include/Systems/AnimSystems.hpp (275 lines)    - Animation updates
```

### Web Interface
```
web/index.html              - Main entry point
web/scripts/module.js       - WebAssembly loader
web/scripts/ebuilder.js     - Config parser
web/scripts/config.js       - Asset definitions
```

### Configuration Files
```
web/econfigs/default.txt    - Main game config
web/econfigs/menu.txt       - Menu scenes
web/econfigs/levels/*.txt   - Level definitions
web/econfigs/tests/*.txt    - Test configurations
```

## Development Workflow

### Build Process
```bash
./compile.sh
# Invokes: emcc with flags:
# -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_WEBGL2=1
# -s EXPORTED_FUNCTIONS=['_main','_isready',...]
# -s ALLOW_MEMORY_GROWTH=1
# Output: web/build/main.js + main.wasm
```

### Adding New Components
1. Define struct in `include/structs.hpp`
2. Add parser in `web/scripts/ebuilder.js`:
   - `componentParsers` object
   - `componentParameterCounts` entry
3. Add C++ parsing in `WebUtils.hpp`:
   - `safe_emplace` block in `load_json_to_registry`
4. Create system in `include/Systems/`
5. Update config files in `web/econfigs/`

### Testing Workflow
1. Modify config files for rapid iteration
2. Use browser console for shader errors
3. Check `web/tests/` for component tests
4. Scene builder tool: `web/tests/scenebuilderv2.html`

## Advanced Features

### Dynamic Shader System
**Creation Process**:
1. Write GLSL files: `<name>_v.glsl`, `<name>_f.glsl`
2. Add to `web/scripts/config.js`
3. Use in entity: `cshader <name> [uniforms]`

**Shader Requirements**:
- Vertex: `attribute vec2 position`, `uniform vec2 instancePosition`
- Fragment: `uniform vec3 uColor`, custom uniforms
- Time-based: `uniform float uTime`

### Animation System
**TextureAlts Component**:
- Stores multiple texture states
- Frame-based switching
- Directional animations (idle/run × up/down/left/right)

**Animation Pipeline**:
1. Load sprite sheets via config
2. Parse into TextureAlts
3. AnimationSystem updates current texture
4. RenderSystem draws current frame

### Text Rendering with Embedded Images
**Syntax**: `!@image:texture_name,scaleX,scaleY@!`
**Processing**:
1. Parse text for image markers
2. Calculate positions
3. Render text segments
4. Insert images at markers

## Known Limitations & Quirks

### Technical Limitations
- **Physics**: Rectangle-only collisions
- **Shaders**: Limited uniform support
- **Mobile**: Basic touch mapping, no optimization
- **Audio**: No sound system implemented
- **Networking**: No multiplayer support
- **Save System**: No persistence mechanism

### Behavioral Quirks
- Camera `gridSpacing` controls zoom (counterintuitive)
- Physics bodies auto-created for Position+Shape entities
- Interior walls render differently based on player context
- Text with embedded images has approximate positioning
- Shader errors only visible in browser console
- Scene transitions reset player state

### Performance Considerations
- All entities processed every frame (no culling)
- Physics runs O(n²) collision checks
- Text rendering recreates textures frequently
- No texture atlasing optimization
- Scene switching loads all entities at once

## Future Architecture Considerations

### Potential Improvements
1. **ECS Enhancements**:
   - Archetype storage for cache efficiency
   - System dependencies and parallelization
   - Component versioning for change detection

2. **Rendering Upgrades**:
   - WebGPU migration
   - Instanced rendering
   - Texture atlasing
   - Particle system

3. **Physics Evolution**:
   - Spatial partitioning (quadtree/grid)
   - Circle and polygon shapes
   - Continuous collision detection
   - Constraint solver

4. **Content Pipeline**:
   - Visual scene editor
   - Hot reload without recompilation
   - Asset preprocessing
   - Localization support

5. **Platform Extensions**:
   - Native builds (remove Emscripten)
   - Mobile-specific optimizations
   - Controller support
   - Accessibility features

This engine represents approximately 5 years of iterative development, with a focus on browser-native experiences and ease of content creation through its configuration language. The architecture prioritizes flexibility and simplicity over raw performance, making it suitable for 2D games with moderate entity counts and emphasis on spatial interaction.