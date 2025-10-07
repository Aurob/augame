# AuGame Editor - Viewport Controls

## Navigation Controls

### Panning
- **Alt + Left Mouse Drag**: Pan the viewport around the world
- The viewport position is shown in the top-left corner

### Zooming
- **Mouse Wheel Up**: Zoom in (up to 400%)
- **Mouse Wheel Down**: Zoom out (down to 25%)
- Zoom level is shown in the viewport info panel

### Multi-Select
- **Shift + Drag**: Draw selection box to select multiple entities
- **Ctrl + Shift + Drag**: Add to current selection

## Features

### Grid Display
- Grid automatically adjusts to zoom level
- Grid disappears when zoomed out too far (for performance)
- Origin axes (0,0) shown with darker lines

### Viewport Info Panel
- Shows current viewport position (top-left corner of view)
- Shows current zoom level as percentage
- Shows center point of current view

### Entity Placement
- New entities are created where you draw them
- Blueprints are loaded at the current view center
- Imported entities maintain their world positions

### World Coordinates
- All entity positions are in world coordinates
- Negative coordinates are fully supported
- You can pan to any position in the world

## Tips
- Use Alt+drag to navigate to entities that are off-screen
- Zoom out to get an overview of your scene
- The grid snapping works at any zoom level
- Entity labels and borders scale with zoom