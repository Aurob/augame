#pragma once

#include <SDL2/SDL.h>

using namespace std;

extern int width, height;
extern unordered_map<int, bool> keys;
extern GLfloat cursorPos[2];
extern float moveSpeed;
extern float defaultMoveSpeed;
extern float gridSpacingValue;


void EventHandler(int type, SDL_Event *event)
{

    // wasd for offset
    if (event->type == SDL_KEYDOWN)
    {
        keys[event->key.keysym.sym] = true;
    }
    else if (event->type == SDL_KEYUP)
    {
        keys[event->key.keysym.sym] = false;
    }


    // Mouse position
    if (event->type == SDL_MOUSEMOTION)
    {
        cursorPos[0] = event->motion.x;
        cursorPos[1] = event->motion.y;
    }

    // Mouse Interactions
    if (event->type == SDL_MOUSEBUTTONDOWN)
    {
        keys[SDL_BUTTON_LEFT] = true;
    }
    else if (event->type == SDL_MOUSEBUTTONUP)
    {
        keys[SDL_BUTTON_LEFT] = false;
    }

    // Zoom in and out
    if (event->type == SDL_MOUSEWHEEL)
    {
        if (event->wheel.y > 0)
        {
            gridSpacingValue *= 1.08f;
        }
        else if (event->wheel.y < 0)
        {
            gridSpacingValue /= 1.08f;
        }
    }

}
