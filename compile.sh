#!/bin/bash
echo "compile commenced at:" $(date)
start_time=$(date +%s)

mkdir -p build

em++ -std=c++17 src/main.cpp \
  -s WASM=1 -s USE_SDL=2 -s USE_WEBGL2=1\
  -s USE_SDL_IMAGE=2\
  -s STB_IMAGE=1\
  -s USE_SDL_TTF=2\
  --embed-file resources/audio\
  --embed-file resources/fonts\
  --embed-file resources/shaders\
  --embed-file resources/cshaders\
  --embed-file resources/text\
  --embed-file resources/textures\
  -sASSERTIONS\
  -sALLOW_MEMORY_GROWTH\
  -sNO_DISABLE_EXCEPTION_CATCHING\
  -lSDL\
  -s SDL2_IMAGE_FORMATS='["png", "jpg"]'\
  -s EXPORTED_FUNCTIONS="[\
    '_main',\
    '_load_json',\
    '_isready',\
    '_reload',\
    '_createSceneFromJson',\
    '_switchToNextScene',\
    '_switchToPrevScene',\
    '_addEntityToScene',\
    '_set_inactive',\
    '_set_active',\
    _malloc,\
    _free,\
    UTF8ToString,\
    stringToUTF8\
  ]"\
  -o build/main.js\
  -O0
  
end_time=$(date +%s)
echo "compile finished at:" $(date)
echo "compile time:" $(($end_time - $start_time)) "seconds"

  # -s NO_DISABLE_EXCEPTION_CATCHING\
  # -s ALLOW_MEMORY_GROWTH=1 \
  # --use-preload-plugins\

