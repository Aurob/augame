#!/bin/bash
echo "compile commenced at:" $(date)
start_time=$(date +%s)
  em++ -std=c++1z src/$1.cpp \
  src/imgui/imgui.cpp src/imgui/imgui_draw.cpp \
  src/imgui/imgui_tables.cpp src/imgui/imgui_impl_sdl.cpp src/imgui/imgui_widgets.cpp src/imgui/imgui_impl_opengl3.cpp \
  -s WASM=1 -s USE_SDL=2 -s USE_WEBGL2=1\
  --embed-file resources\
  -s USE_SDL_IMAGE=2\
  -s STB_IMAGE=1\
  -s SDL2_IMAGE_FORMATS='["png", "jpg"]'\
  -lSDL\
  -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']"\
  -s EXPORTED_FUNCTIONS="['_main', '_load_json', '_isready', _malloc, UTF8ToString, stringToUTF8, _free]"\
  --use-preload-plugins\
  -s ALLOW_MEMORY_GROWTH=1 \
  -s NO_DISABLE_EXCEPTION_CATCHING\
  -o build/$1.js\


end_time=$(date +%s)
echo "compile finished at:" $(date)
echo "compile time:" $(($end_time - $start_time)) "seconds"