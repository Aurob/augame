
<!-- Add your logo here -->
![Tilesh Logo](path/to/logo.png)

# Tilesh Game Engine

> A lightweight, web-based 2D game engine powered by C++ and WebAssembly

<!-- Add a demo image or gif here -->
![Tilesh Demo](path/to/demo.gif)

Tilesh is a flexible framework for creating 2D games that run directly in web browsers. It combines the performance of C++ with the accessibility of web technologies.

## Features

- 🚀 C++ core for high performance
- 🌐 WebAssembly compilation for browser deployment
- 🎮 SDL2 for cross-platform compatibility
- 🖼️ OpenGL ES 2.0 for efficient rendering
- 🧩 Entity-Component-System (ECS) architecture using EnTT
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
   git clone https://github.com/yourusername/tilesh-engine.git
   cd tilesh-engine
   ```
3. Compile the project:
   ```
   ./compile.sh main
   ```
4. Serve the resulting files using a local web server and open in your browser.

## Dependencies

- SDL2
- SDL2_image
- Emscripten
- EnTT (header-only, included in the project)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
