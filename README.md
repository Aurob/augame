# AuGame

[DEMO](https://aurob.github.io/augame/)

**2D game engine** - C++ with WebAssembly, SDL2, OpenGL  
**ECS Architecture** - EnTT-based entity component system  
**Web-Native** - Runs directly in browsers via WebAssembly

---

## Features

- Custom shader pipeline with dynamic loading
- Multi-scene management system
- Interactive entity system (hover, click, drag)
- Text rendering with font support
- Physics simulation
- Configurable world generation

## Architecture

- **Core**: `src/main.cpp` - Main game loop
- **Rendering**: `include/GLUtils.hpp` - OpenGL pipeline
- **Systems**: `include/Systems/` - ECS system implementations  
- **Physics**: `include/lib/physics.hpp` - 2D physics engine (based on [Physics2D](https://github.com/SifuF/physics-2d))
- **Config**: `web/econfigs/` - Scene definitions
- **Assets**: `resources/` - Shaders, textures, fonts

## Controls

- **WASD** - Player movement
- **Mouse** - Entity interaction (hover, click, drag)
- **ESC** - Pause/unpause
- **C** - Toggle camera mode
- **Shift+</>** - Scene navigation

## Notes

This project has evolved over the past ~5 years as a casual hobby project. I've done many rewrites using different languages (JS, C, C++) and frameworks (Vanilla HTML/JS Canvas, SFML, Raylib, SDL2), before settling on the current setup.

In the past 1-2 years I've slowly begun integrating coding models and tools into my development process, to great success in my opinion. My favorite of these, that has been genuinely helpful/useful, is [Claude Code](https://claude.com/product/claude-code). I've used it extensively for some of the more complex aspects of this engine.

See `CLAUDE.md` for detailed development documentation from a number of sessions I've had using it.


## Build

### Install Emscripten

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### Build & Run

```bash
git clone https://github.com/Aurob/augame.git
cd augame
chmod +x compile.sh
./compile.sh
```

This creates `web/build/main.js` and `web/build/main.wasm`. The `web/index.html` file will attempt to load these, but the wasm binary won't load properly without a web server hosting these files.

**Quick web server setup:**

```bash
cd augame
python3 -m http.server 8080
```

- Home page: http://localhost:8080
- Engine: http://localhost:8080/web

## Self-Contained Executable (Beta)

[View the latest release](https://github.com/Aurob/augame/releases/)

Single file executable that runs without any setup on Linux, Windows, and MacOS using [Redbean](https://redbean.dev) web server (an [actually portable executable](https://justine.lol/ape.html)).

### Download

[Download augame.com](https://github.com/Aurob/augame/releases/latest/download/augame.com)

> **Note:** By convention, redbean .ape binaries use the `.com` extension. This works as both a colloquial similarity to domain .com and as a valid executable extension on Windows (can be double-clicked like any .exe). The file can be also be renamed to `.exe` and still work.

### Build It Yourself

Building this repo's .com file is actually pretty simple. The redbean server is already a fully functionaing .ape executable, it also behaves like a .zip file meaning files and directories can be zipped into it using `zip`. 

So to create augame.com, clone this repo, download the latest redbean.com file, zip the contents into it, and run `./augame.com`

```bash
# Clone this repo
git clone https://github.com/aurob/augame
cd augame

# Download the latest version of redbean
wget -O augame.com "http://cdn.dump.garden/redbean-3.0.0.com"
# original source: https://redbean.dev/redbean-3.0.0.com

# Run the compile script
chmod +x compile.sh
./compile.sh

# Zip only the necessary files
# Note: resources don't need to be included since they're compiled with the .wasm binary
zip -r augame.com \
   .init.lua \
   web/index.html \
   web/econfigs \
   web/tests \
   web/scripts \
   README.md \
   LICENSE

# Zip these with no compression (-0) to avoid issues with fetching from the client
zip -0 augame.com web/resources web/build/main.wasm web/build/main.js

# Run the augame redbean server
chmod +x augame.com
./augame.com
```

## License

ISC License
