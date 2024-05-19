#pragma once

#include <SDL2/SDL.h>
#include <SDL_opengles2.h>


// Test Shaders:
const GLchar *vertexSourceTest = R"glsl(
    #version 100 // GLSL ES version for OpenGL ES 2.0
    attribute vec3 position;

    void main() {
        gl_Position = vec4(position, 1.0);
    }
)glsl";

const GLchar *fragmentSourceTest = R"glsl(
    #version 100
    precision mediump float;
    
    void main() {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
)glsl";

// Test Shaders that will be set when json config is loaded
//  using the shaders passed from the json config
const GLchar *vertexSource = nullptr;

const GLchar *fragmentSource = nullptr;


const GLchar *vertexSource2 = nullptr;
const GLchar *fragmentSource2 = nullptr;
