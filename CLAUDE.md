# Component Configuration Update Routine

## When updating a component that can be configured from parsed configs

When adding or modifying parameters for components that can be configured via text configs (like econfigs), you need to update multiple files to ensure consistency across the system:

### Required File Updates:

1. **include/WebUtils.hpp** - Update the component parser in the `load_json` function
   - Add new parameter parsing logic in the appropriate `safe_emplace` block
   - Example: `if (camera.contains("priority") && camera["priority"].is_number()) { cameraComponent.priority = camera["priority"]; }`

2. **web/scripts/ebuilder.js** - Update the EntityBuilder class
   - Update parameter count in `componentParameterCounts` object
   - Update parser function in `componentParsers` object
   - Example: `camera: 4` (increase count) and add parameter to parser function

3. **web/econfigs/default.txt** - Update existing component instances
   - Add the new parameter with default values to existing component definitions
   - Example: `camera 1024.0 16.0 0` (added priority parameter with default 0)

4. **web/tests/scenebuilderv2.html** - Update the scene builder interface
   - Add component schema to `COMPONENT_SCHEMAS` array if not present
   - Add export logic in `exportEntities` function for the new parameter
   - Example: Add Camera schema with priority parameter and export logic

### Parameter Defaults:
- Always provide sensible default values for new parameters
- Use 0 as default for priority-type parameters
- Maintain backward compatibility by making new parameters optional

### Testing:
After updates, verify:
- Config files parse correctly
- Scene builder can create/edit components with new parameters  
- Export functionality includes new parameters
- Default values work when parameters are omitted

## Camera Priority System

The game supports multiple cameras with priority-based selection:

### Camera Selection Logic:
1. **Priority-based**: Highest priority camera is selected as the main camera
2. **Inside filtering**: When a Player exists, only cameras in the same Inside context are considered:
   - If Player is inside an interior, only cameras inside the same interior are considered
   - If Player is outside, only cameras that are also outside are considered
3. **Fallback**: If no suitable camera is found, the first available camera is used

### Camera Parameters:
- **priority** (int, default: 0): Higher values take precedence over lower values
- **radius** (float, default: 0.0): Distance-based selection radius when player is active
- **important** (bool, default: false): When true, always renders first regardless of other factors

### Camera Selection Behavior:
1. **Important cameras**: Always selected first if they match the current Inside context
2. **Priority + Distance**: When Player is active and in player camera mode, cameras within radius get distance bonus
3. **Player Camera Mode Toggle**: Press C to switch between player-based camera selection and global priority camera
4. **Multiple cameras**: Can exist in the same interior or outside context
5. **Dynamic switching**: Camera switches automatically when Player moves between inside/outside contexts or when toggling modes

### Camera Modes:
- **Player Camera Mode** (default): Cameras are filtered by Player's Inside context and use distance-based scoring
- **Priority Camera Mode**: Uses highest priority camera globally, ignoring Player position and context

### Implementation:
- **selectMainCamera()** function in ViewSystems.hpp handles all camera selection logic
- **gameState.playerCameraMode** tracks current camera mode (true = player camera, false = priority camera)
- All camera usage throughout the codebase uses this centralized selection function
- Zoom, rendering, and positioning systems all respect the priority-based camera selection
- **C key**: Toggles between camera modes when Player exists and game is active