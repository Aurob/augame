
<!-- Add your logo here -->
<!-- ![AuGame Logo](path/to/logo.png) -->

# AuGame Game Engine

> A lightweight, web-based 2D game engine powered by C++ and WebAssembly

<!-- Add a demo image or gif here -->
<!-- ![AuGame Demo](path/to/demo.gif) -->

AuGame is an early prototype for a future unnamed game. It's a 2D game engine that runs directly in web browsers, combining the performance of C++ with the accessibility of web technologies.
## Features

- 🚀 C++ core for high performance
- 🌐 [WebAssembly](https://webassembly.org/) compilation for browser deployment
- 🎮 [SDL2](https://www.libsdl.org/) for cross-platform compatibility
- 🖼️ [WebGL](https://www.khronos.org/webgl/) for efficient rendering
- 🧩 Entity-Component-System (ECS) architecture using [EnTT](https://github.com/skypjack/entt)
- 🎨 Custom shader support
- 🖼️ Texture loading and rendering
- 🏃‍♂️ Basic physics and collision detection
- 🕹️ Player movement and interactions
- 🌍 Tile-based world generation
- 🎭 Dynamic entity creation and management

## Key Components

- `main.cpp`: Core game loop and initialization
- `GLUtils.hpp`: OpenGL utility functions
- `JSUtils.hpp`: JavaScript interop utilities
- `Systems.hpp`: Game systems (movement, collisions, etc.)
- `EFactory.hpp`: Entity factory for creating game objects
- `events.hpp`: Event handling
- Various GLSL shaders for rendering

## Building and Running

1. Set up Emscripten by following the [official installation guide](https://emscripten.org/docs/getting_started/downloads.html).
2. Clone this repository:
   ```
   git clone https://github.com/Aurob/augame.git
   cd augame
   ```
3. Compile the project:
   ```
   ./compile.sh main
   ```
4. Serve the resulting files using a local web server and open in your browser. You can use Python's built-in HTTP server:
   ```
   python -m http.server 8000
   ```
   Then open your web browser and navigate to `http://localhost:8000`.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
