# AuGame Scene Editor - AI Context

## Purpose
Visual scene editor for the AuGame engine. Creates and edits entity configurations that are consumed by the engine's text-based config format.

## Architecture

### Module Structure
```
web/editor/
├── index.html      - UI and canvas
├── app.js          - Main application controller
├── components.js   - Component schemas (imports from ebuilder.js)
├── entities.js     - Entity management
├── canvas.js       - Canvas rendering and interaction
└── exporter.js     - Config format export/import
```

### Key Design Decisions

1. **Single Source of Truth**: Component definitions come from `web/scripts/ebuilder.js` - the same parser the engine uses. This ensures parity.

2. **Visual-First**: Draw rectangles on canvas, components are secondary. Most entities start as Position + Shape + Color.

3. **Config Output**: Exports to AuGame's space-delimited format, not JSON. This is what the engine expects.

4. **Minimal State**: No complex state management. Entity array + selected index is all we track.

## Component System

### Data Flow
```
ebuilder.js (ComponentSchemas)
    ↓
editor/components.js (transforms for UI)
    ↓
Editor UI (creates/modifies entities)
    ↓
exporter.js (converts to config format)
    ↓
"id 1 door position 10 20 0 shape 2 3 0 ..."
```

### Special Cases

**Color Component**:
- Config uses 0-1 range: `color 0.5 0.9 1.0 1.0`
- Editor uses 0-255 range for UI: `{r: 127, g: 230, b: 255, a: 1.0}`
- Conversion happens in exporter.js

**Teleporter Component**:
- Config format: `teleporter x y z interiorId`
- Editor format: `{destX, destY, destZ, interiorEntity}`

**Text Component**:
- Schema says `hide` but editor uses `hidden` for clarity
- Conversion handled in components.js transform

## Canvas Rendering

### Coordinate System
- Canvas pixels = game units * 16
- Grid snapping at 0.5 unit intervals
- Position (0,0) is top-left

### Interaction Modes
- **Draw**: Click-drag to create entities
- **Move**: Click to select, drag to move
- **Edit**: Select + resize handles + component panel

### Visual Hierarchy
1. Entity rectangles (sorted by RenderPriority.z)
2. Selection border (blue = selected, gray = normal)
3. Entity label: "id:name"
4. Z-order indicator

## Export Format

### Entity Line Format
```
id <num> <name> [components...]
```

### Component Formats
```
position x y z
shape w h d
color r g b a        (0-1 range)
renderPriority z
movement speed mass restitution friction
text "string" scale hidden offsetX offsetY
```

### Example Output
```
id 1 player position 0 0 0 shape 2 2 0 color 1 0 0 1 player moveable
id 2 wall position 5 0 0 shape 1 10 0 collidable
```

## Storage

### LocalStorage Keys
- `augame_editor_autosave` - Current state auto-saved every 10s
- `augame_scene_*` - Saved scenes (user-named)

### State Structure
```javascript
{
    entities: [...],
    selectedIdx: null|number,
    nextId: number
}
```

## Common Tasks

### Adding a New Component Type

1. It should already exist in `ebuilder.js` ComponentSchemas
2. If special UI handling needed, add transform in `components.js`
3. Add export case in `exporter.js` exportComponent()
4. Add import case in `exporter.js` parseComponent()

### Fixing Component Mapping Issues

The editor type names don't always match config keywords:
- Editor: `CustomShader` → Config: `cshader`
- Editor: `RenderPriority` → Config: `renderPriority`

Mappings are handled via `configKey` property in component schemas.

### Debugging Export Issues

1. Check browser console for component warnings
2. Verify parameter counts in ebuilder.js match actual export
3. Test round-trip: Export → Copy → Import → Export should be identical

## Quirks & Gotchas

1. **Component Order Matters**: In config files, `id` must come first, then position/shape usually follow.

2. **Color Values**: Always check if using 0-1 or 0-255 range. Config is 0-1, editor UI is 0-255.

3. **Entity IDs**: Must be unique integers. Editor auto-increments but user can override.

4. **Portal Components**: Reference other entity IDs. -1 means "no entity".

5. **Snap Grid**: 0.5 units in game space = 8 pixels on canvas.

## Future Improvements Needed

- [ ] Undo/redo system
- [ ] Copy/paste entities
- [ ] Multi-select and group operations
- [ ] Import validation with error messages
- [ ] Meta tag editing (scene-level config)
- [ ] Visual texture preview
- [ ] Interior/portal visualization
- [ ] Direct ebuilder.js parser integration for import

## Quick Reference

### Keyboard Shortcuts
- `Delete` - Delete selected entity
- `Ctrl+D` - Duplicate selected entity
- `Ctrl+C` - Copy export to clipboard
- `Ctrl+S` - Save scene

### Canvas Scale
- 1 game unit = 16 pixels
- Snap grid = 0.5 game units = 8 pixels

### Component Priorities
- Default components: Position, Shape, Color, RenderPriority
- Player always needs: Player, Moveable components
- Walls need: Collidable component
- Doors need: InteriorPortal, Interactable components