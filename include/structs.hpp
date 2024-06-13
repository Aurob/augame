#pragma once

#include <vector>

using namespace std;

struct context
{
    SDL_Event event;
    int iteration;
    SDL_Window *window;
};

struct Position {
    float x;
    float y;
    float sx;
    float sy;
};

struct Shape {
    float w;
    float h;
};

struct Color {
    float r;
    float g;
    float b;
    float a;
};

struct Validation {
    int state{0};
};

struct Visible {};

struct Debug {
    Color defaultColor;
};

struct Teleport {
    Position origin;
    Position destination;
};

struct Hoverable {};
struct Hovered {};

struct Interactable {};
struct Interacted {};

struct Collisions {};