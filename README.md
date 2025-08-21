# AuGame

**2D game engine** - C++ with WebAssembly, SDL2, OpenGL  
**ECS Architecture** - EnTT-based entity component system  
**Web-Native** - Runs directly in browsers via WebAssembly

## Features

- Custom shader pipeline with dynamic loading
- Multi-scene management system
- Interactive entity system (hover, click, drag)
- Text rendering with font support
- Physics simulation
- Configurable world generation

## Setup

1. **Install Emscripten**:
   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   source ./emsdk_env.sh
   ```

2. **Build & Run**:
   ```bash
   git clone https://github.com/Aurob/augame.git
   cd augame
   ./compile.sh
   python -m http.server 8000
   ```
   
   Open `http://localhost:8000`

## Architecture

- **Core**: `src/main.cpp` - Main game loop
- **Rendering**: `include/GLUtils.hpp` - OpenGL pipeline
- **Systems**: `include/Systems/` - ECS system implementations  
- **Physics**: `include/lib/physics.hpp` - 2D physics engine (based on [Physics2D](https://github.com/SifuF/physics-2d))
- **Config**: `web/econfigs/` - Scene definitions
- **Assets**: `resources/` - Shaders, textures, fonts

See `CLAUDE.md` for detailed development documentation.

## Controls

- **WASD**: Player movement
- **Mouse**: Entity interaction (hover, click, drag)
- **ESC**: Pause/unpause
- **C**: Toggle camera mode
- **Shift+</>**: Scene navigation

## License

MIT License