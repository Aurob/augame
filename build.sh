#!/bin/bash
echo "compile commenced at:" $(date)
start_time=$(date +%s)
  em++ -std=c++1z src/$1.cpp \
  -s WASM=1 -s USE_SDL=2 -s USE_WEBGL2=1\
  -lSDL\
  -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']"\
  -s EXPORTED_FUNCTIONS="['_main', '_load_json', _malloc, UTF8ToString, stringToUTF8]"\
  --use-preload-plugins\
  -s ALLOW_MEMORY_GROWTH=1 \
  -s NO_DISABLE_EXCEPTION_CATCHING\
  -o build/$1.js\

end_time=$(date +%s)
echo "compile finished at:" $(date)
echo "compile time:" $(($end_time - $start_time)) "seconds"