# Augame Development Framework

**Game Engine**: C++ with Emscripten, SDL2, OpenGL, EnTT ECS
**Build**: `./compile.sh` - Outputs to `build/main.js` and `build/main.wasm`
**Web Interface**: `web/` directory with HTML/JS frontend

## Architecture Overview

### Core Systems
- **ECS Framework**: EnTT registry-based entity component system
- **Rendering**: OpenGL with custom shader pipeline (`include/GLUtils.hpp`)
- **Physics**: Custom 2D physics system (`include/lib/physics.hpp`)
- **Scene Management**: Multi-registry system for scene switching (`include/SceneManager.hpp`)
- **Input Handling**: SDL2 events (`include/events.hpp`)

### Key Directories
```
include/                    # C++ headers
├── Systems/               # ECS system implementations
├── lib/                   # Third-party libraries (EnTT, physics)
├── structs.hpp           # Component definitions
├── GLUtils.hpp           # Rendering pipeline
├── WebUtils.hpp          # JSON parsing, web interface
└── SceneManager.hpp      # Multi-scene management

src/main.cpp              # Main application entry
web/                      # Frontend interface
├── scripts/module.js     # Main JS logic
├── scripts/ebuilder.js   # Entity configuration builder
└── econfigs/            # Scene configuration files

resources/                # Assets
├── shaders/             # GLSL shader files
├── textures/            # Image assets
└── fonts/               # TTF font files
```

## Component System

### Core Components (`include/structs.hpp`)
- **Position**: `{x, y, z, sx, sy, sz}` - World and screen coordinates
- **Shape**: `{size, scaled_size}` - Entity dimensions  
- **Color**: `{r, g, b, a}` - Rendering color with defaults
- **Text**: `{text, scale, hide, offsetX, offsetY}` - Text rendering
- **Movement**: Speed, velocity, acceleration, physics properties
- **Player**: Marks player entity
- **Camera**: View control with grid spacing and priority
- **Interactable/Hoverable**: User interaction components

### Component Configuration Updates
When adding new components:
1. **include/structs.hpp** - Define component struct
2. **include/WebUtils.hpp** - Add parsing in `load_json` function `safe_emplace` block  
3. **web/scripts/ebuilder.js** - Update `componentParameterCounts` and `componentParsers`
4. **web/econfigs/default.txt** - Add defaults to existing entities

## Scene & Config System

### Config File Format
```
-- Comments use double dash
id <id> <name> position <x> <y> <z> shape <w> <h> <d> color <r> <g> <b> <a> [components...]

meta <tag> <value>         # World-level metadata
```

### Meta Tags
- `world "shader_name"` / `"#hex"` / `"r,g,b"` - Outside area rendering (terrain/tiles/color)
- `void "#hex"` / `"r,g,b"` - Inside area clear color  
- `font "filename.ttf"` - Font file path
- `start_menu "text"` / `"@file.txt"` - Start screen text
- `pause_menu "text"` / `"@file.txt"` - Pause screen text
- `seed "value"` - World generation seed

### Multi-Scene System
```cpp
// Scene switching: Shift+< (prev), Shift+> (next)
meta scene "@web/econfigs/scene1.txt"  // File reference
meta scene "inline_scene_name"         // Inline definition
```

**Exported Functions**: `_createSceneFromJson`, `_switchToNextScene`, `_switchToPrevScene`, `_addEntityToScene`

### Runtime Entity Addition
Add, update, or recreate entities at runtime using JavaScript:

**Create New Entity:**
```javascript
Module.add_entity(0, {
    "Entities": [{
        "New": true, // Creates new entity (removes existing with same ID)
        "Components": {
            "Id": {"id": 123, "name": "RuntimeEntity"},
            "Position": {"x": 2, "y": 3, "z": 0},
            "Shape": {"size": [1, 1, 1]},
            "Color": {"r": 1, "g": 0, "b": 0, "a": 1}
        }
    }]
});
```

**Update Existing Entity:**
```javascript
Module.add_entity(0, {
    "Entities": [{
        "New": false, // Updates existing entity by ID
        "Components": {
            "Id": {"id": 123, "name": "RuntimeEntity"}, // Required for updates
            "Position": {"x": 5, "y": 5, "z": 0}, // Only specified components updated
            "Color": {"r": 0, "g": 1, "b": 0, "a": 1}
        }
    }]
});
```

- Uses same JSON format as econfig files
- Scene indices are zero-based (0, 1, 2, etc.)
- `New: true` removes existing entity with same ID, then creates new
- `New: false` updates existing entity components by ID
- All standard components supported

## Shader System

### Static Shaders (Embedded in C++)
- `font` - Text rendering with color uniforms
- `terrain` - Procedural RGB terrain generation (used via `meta world terrain`)
- `debug_entity` - Colored rectangles
- `ui_layer` - UI texture rendering
- `texture` - General texture rendering

### Dynamic Custom Shaders
1. Create `resources/cshaders/<name>_v.glsl` and `<name>_f.glsl`
2. Add to `web/scripts/config.js`
3. Use with `cshader <name>` in entity configs

**Requirements**: Vertex shaders use `attribute vec2 position`, `uniform vec2 instancePosition`, `uniform vec2 entityScale`

### Terrain Shader Seeds
The `seed` uniform is passed to terrain shaders (`terrain_gradient.glsl`, `terrain_simple.glsl`) to vary noise patterns.

**IMPORTANT**: The seed must be incorporated into noise functions with phase shifts:
```glsl
float phaseShift = seed * 0.1;
float noise = sin(coord.x * freq + phaseShift) + cos(coord.y * freq + phaseShift * 1.3);
```

**Why**: Using `seed` directly in sin/cos (e.g., `sin(coord + seed)`) doesn't create sufficient variation because the small coordinate changes dominate. Phase shifts multiply the seed effect, creating distinct patterns.

**Don't**: Just add seed to coordinates: `sin(p.x + seed)` - minimal visible change
**Do**: Use seed as phase shift multiplier: `sin(p.x * freq + seed * 0.1)` - distinct patterns

## Interaction System

### Mouse/Touch Handling (`include/Systems/ActionSystems.hpp`)
- **Hoverable**: Blue highlight on mouse over
- **Interactable**: Green on click, drag when `cursor.downtime > 2`
- **Coordinate System**: Normalized device coordinates with camera-aware positioning

### Input Events (`include/events.hpp`)
- Mouse/keyboard state in `Keys` component
- Cursor position tracking in `Cursor` component
- Game state transitions (start menu, pause, gameplay)

## Rendering Pipeline (`include/GLUtils.hpp`)

### Render Passes
1. **Background**: Terrain shader or solid color based on player context
2. **Entities**: Components rendered by priority (textures, colors, custom shaders)
3. **Text**: Font rendering with proper color handling
4. **Walls**: Interior wall rendering (context-dependent)

### Text Rendering
- Uses SDL_ttf with OpenGL texture generation
- Font shader handles coloring via `uTextColor` uniform
- SDL_ttf renders white, shader applies final color

### Coordinate Systems (CRITICAL)

**Two Different Coordinate Spaces:**

1. **Entity/Physics Space** (World Units)
   - Used by physics engine (`include/Systems/PhysicsSystems.hpp`)
   - Entity positions are in raw world units (e.g., `x = -10`)
   - Collision bounds use these directly
   - Terrain bounds in meta tags use this space

2. **Shader/Terrain Space** (Tile Units)
   - Used by terrain shaders (`web/resources/shaders/terrain_*.glsl`)
   - Calculated as: `sampleCoord = (coord / grid_spacing) + toplefttile + (offset / grid_spacing) + generationOffset`
   - Where `toplefttile` is in tile units: `cameraPos / defaultGSV`
   - Each tile = `defaultGSV` world units (default: 16)

**Converting Terrain Bounds for Shaders:**
When passing `terrain_bounds` to shaders, convert from world units to tile units:
```cpp
// Entity bounds are in world units (e.g., -10, -10, 10, 10)
// Convert to tile space for shader comparison
float tileMinX = (worldMinX / camera.defaultGSV) + generationOffset.x;
float tileMaxX = (worldMaxX / camera.defaultGSV) + generationOffset.x;
// Then pass to shader uniform
```

**Why This Matters:**
- Physics checks: `if (entity.x < terrain_bounds.minX)` uses world units
- Shader checks: `if (sampleCoord.x < terrain_bounds.x)` uses tile units
- Must convert bounds when passing to shaders (`GLUtils.hpp:271-306`)
- `generationOffset` is added in shader, so add it to converted bounds too

**Example:**
- Entity bounds: `-10, -10, 10, 10` (world units)
- defaultGSV: `16`
- generationOffset: `[1.0, 1.0]`
- Shader bounds: `(-10/16 + 1.0, -10/16 + 1.0, 10/16 + 1.0, 10/16 + 1.0)` = `(0.375, 0.375, 1.625, 1.625)` (tile units)

## Build & Development

### Compilation
```bash
./compile.sh               # Full build to WebAssembly
```

### Testing
- **Scene Builder**: `web/tests/scenebuilderv2.html`
- **Entity Configs**: Test configurations in `web/econfigs/`

### Git Workflow
- Always create unique branch for each session
- Don't compile or run Python server manually

## Quick Reference

### Finding Components
- **Component definitions**: `include/structs.hpp`
- **Rendering logic**: `include/GLUtils.hpp`
- **Interaction system**: `include/Systems/ActionSystems.hpp`
- **Input handling**: `include/events.hpp`

### Adding Features
- **New components**: Follow Component Configuration Updates process
- **New shaders**: Use Dynamic Custom Shaders workflow
- **New scenes**: Create econfig file with meta tags
- **New interactions**: Modify ActionSystems.hpp and add component types

### Common Patterns
- **Entity creation**: Use `safe_emplace` in WebUtils.hpp
- **Shader uniforms**: Follow existing patterns in GLUtils.hpp
- **Event handling**: Maintain mouse button state consistency
- **Text rendering**: Always render white in SDL_ttf, apply color in shader