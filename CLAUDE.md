# Development Update Guide

If the current dir claude is running in has a .git
 - always make sure there is a unique branch for the current session

## Component Configuration Updates

When adding/modifying configurable components, update these files:

1. **include/WebUtils.hpp** - Add parsing in `load_json` function `safe_emplace` block
2. **web/scripts/ebuilder.js** - Update `componentParameterCounts` and `componentParsers`
3. **web/econfigs/default.txt** - Add parameter with defaults to existing entities
4. **web/tests/scenebuilderv2.html** - Update component schemas and export logic

## Shader System

### Static Shaders (Compiled in C++)
Core shaders are defined directly in C++ code in `include/GLUtils.hpp`:
- `font` - Text rendering
- `terrain` - Procedural terrain generation  
- `debug_entity` - Colored rectangles (test_rgb)
- `ui_layer` - UI texture rendering
- `texture` - General texture rendering (vert_tex + frag_tex)

These are embedded in the binary and cannot be modified at runtime.

### Dynamic Custom Shaders (cshaders)
Runtime-loadable shaders for live editing without recompilation:

#### Required Steps:
1. **Create shader files** in `resources/cshaders/`:
   - `<name>_v.glsl` (vertex shader)
   - `<name>_f.glsl` (fragment shader)

2. **Add to config** in `web/scripts/config.js`:
   ```js
   {
       "name": "shadername",
       "vertex": "resources/cshaders/shadername_v.glsl", 
       "fragment": "resources/cshaders/shadername_f.glsl"
   }
   ```

3. **Use in entities** with `cshader <name>` in econfig files

#### Shader Requirements:
- Vertex: Use `attribute vec2 position`, `uniform vec2 instancePosition`, `uniform vec2 entityScale`
- Fragment: Use `varying vec2 vPosition` for position-based effects
- Dynamic shaders are loaded from `/cshaders` and fetched at runtime
- Static shaders are compiled inline in C++ and embedded in binary

## Meta Tags System

Meta tags provide world-level configuration and metadata in econfig files.

### Adding New Meta Tags

To add a new meta tag like those in `web/econfigs/default.txt`:

1. **Update MetaData struct** in `include/structs.hpp`:
   ```cpp
   struct MetaData {
       // existing fields...
       std::string newField = "default_value";
   };
   ```

2. **Add parsing logic** in `include/WebUtils.hpp` in the `load_json` function:
   ```cpp
   if (meta.contains("newField") && meta["newField"].is_string()) {
       metaData.newField = meta["newField"];
   }
   ```

3. **Update ebuilder.js** in `web/scripts/ebuilder.js`:
   - Add to `componentParameterCounts`: `meta: 3` (parameter count includes tag name + value)
   - Meta parser already handles arbitrary tags, no changes needed

4. **Use in econfig files**:
   ```
   meta newField "value"
   ```

### Existing Meta Tags:
- `world` - World identifier
- `title` - Display title 
- `description` - World description
- `author` - Author name
- `font` - Font file path
- `terrain` - Outside area rendering (when player is not Inside):
  - `"terrain"` - Default terrain shader (default)
  - `"water1"` - Water shader
  - `"#rrggbb"` - Hex color format (e.g., `"#ff0000"` for red)
  - `"r,g,b"` - RGB format (e.g., `"255,0,0"` for red)
- `void` - Inside area clear color (when player is Inside):
  - `"#rrggbb"` - Hex color format (e.g., `"#000000"` for black)
  - `"r,g,b"` - RGB format (e.g., `"0,0,0"` for black)
  - Default: black if not specified
- `start_menu` - Text displayed on start screen (gameState -1):
  - `"text content"` - Direct text content
  - `"@path/to/file.txt"` - Load text from file (like text component)
- `pause_menu` - Text displayed on pause screen (gameState 0):
  - `"text content"` - Direct text content
  - `"@path/to/file.txt"` - Load text from file (like text component)
- `seed` - World generation seed

Meta tags are parsed at world load time and stored in the global `metaData` object.

### Meta Tag Examples:
```
meta terrain "terrain"           // Default terrain shader when outside
meta terrain "water1"            // Water shader when outside  
meta terrain "#336699"           // Blue color when outside
meta terrain "40,121,22"         // RGB dark green when outside

meta void "#000000"              // Black void when inside
meta void "80,40,10"             // Brown void when inside

meta start_menu "Press any key to start"              // Direct text for start screen
meta start_menu "@resources/text/start.txt"           // Load text from file
meta pause_menu "Game Paused - Press ESC to resume"   // Direct text for pause screen
meta pause_menu "@resources/text/pause.txt"           // Load text from file
```

**Meta Tag Behavior:**
- `terrain` controls rendering when the player is outside Interior entities
- `void` controls the clear color when the player is Inside Interior entities
- `start_menu` displays text when gameState is -1 (start screen)
- `pause_menu` displays text when gameState is 0 (pause screen)
- Color parsing is handled in JavaScript and passed to C++ as normalized [r,g,b] arrays
- Shader names (like "terrain", "water1") render the corresponding shader programs
- File references with `@` prefix are loaded asynchronously in JavaScript (same as text component)
- Invalid color formats fall back to black
