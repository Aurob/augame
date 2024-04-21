#pragma once
#include <vector>
#include <SDL2/SDL.h>

using namespace std;

struct context
{
    SDL_Event event;
    int iteration;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_PixelFormat *format;
};

struct temp_point
{
    int x;
    int y;
    int age;
    int max_age;
};

struct player
{
    int id;
    SDL_Point position;
    SDL_Color color;
    int width;
    int height;
    // vector<SDL_Point> cursor_trail;
    vector<temp_point> cursor_trail;
};
