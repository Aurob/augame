# Development Update Guide

## Component Configuration Updates

When adding/modifying configurable components, update these files:

1. **include/WebUtils.hpp** - Add parsing in `load_json` function `safe_emplace` block
2. **web/scripts/ebuilder.js** - Update `componentParameterCounts` and `componentParsers`
3. **web/econfigs/default.txt** - Add parameter with defaults to existing entities
4. **web/tests/scenebuilderv2.html** - Update component schemas and export logic

## Custom Shader Workflow

To add a new custom shader (like `cshader grid` or `cshader colorquads`):

### Required Steps:
1. **Create shader files** in `resources/shaders/`:
   - `<name>_v.glsl` (vertex shader)
   - `<name>_f.glsl` (fragment shader)

2. **Add to config** in `web/scripts/config.js`:
   ```js
   {
       "name": "shadername",
       "vertex": "shadername_v.glsl", 
       "fragment": "shadername_f.glsl"
   }
   ```

3. **Enable compilation** in `include/GLUtils.hpp` `loadTextures()`:
   ```cpp
   createShader(shaderProgramMap["shadername"], "shadername");
   ```

4. **Use in entities** with `cshader <name>` in econfig files

### Shader Requirements:
- Vertex: Use `attribute vec2 position`, `uniform vec2 instancePosition`, `uniform vec2 entityScale`
- Fragment: Use `varying vec2 vPosition` for position-based effects
- Follow existing shader patterns (see `test_rgb_*.glsl`)

## Camera System

**Priority-based selection**: Higher priority cameras are selected first
**Context filtering**: Player Inside context determines available cameras
**Toggle modes**: Press C to switch between player/priority camera modes

### Key Parameters:
- `priority` (int): Selection priority
- `radius` (float): Distance-based bonus
- `important` (bool): Always renders first

**Implementation**: `selectMainCamera()` in ViewSystems.hpp handles all logic

## More Information
- update this guide as needed as the user prompts reveal new requirements
- refer to existing code and documentation for examples