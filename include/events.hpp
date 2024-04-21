#pragma once
#include <emscripten.h>
#include <SDL2/SDL.h>

using namespace std;

extern int width, height;
extern unordered_map<int, bool> keys;
extern GLfloat cursorPos[2];
enum CustomKeyInts {
    ZOOM_IN = -10000,
    ZOOM_OUT = -10001,
    SPEED_MULTI = -10002,
    SPEED_DIV = -10003,
};


void EventHandler(int type, SDL_Event *event)
{
    // mousescroll updated gsvIncrement
    if (event->type == SDL_MOUSEWHEEL)
    {

        if (event->wheel.y > 0)
        {
            // gridSpacingValue *= 2.0f;
            keys[ZOOM_IN] = true;
        }
        else if (event->wheel.y < 0)
        {
            // gridSpacingValue /= 2.0f;
            keys[ZOOM_OUT] = true;
        }
    }

    // wasd for offset
    if (event->type == SDL_KEYDOWN)
    {
        keys[event->key.keysym.sym] = true;
    }
    else if (event->type == SDL_KEYUP)
    {
        keys[event->key.keysym.sym] = false;
    }

    // Shift for speed boost
    if (event->type == SDL_KEYDOWN)
    {
        if (event->key.keysym.sym == SDLK_LSHIFT)
        {
            // moveSpeed *= 200.0f;
            keys[SPEED_MULTI] = true;
        }
    }
    else if (event->type == SDL_KEYUP)
    {
        if (event->key.keysym.sym == SDLK_LSHIFT)
        {
            // moveSpeed /= 200.0f;
            keys[SPEED_DIV] = true;
        }
    }


    // Mobile touch controls
    if (event->type == SDL_FINGERDOWN || event->type == SDL_FINGERMOTION)
    {
        // Handle single finger movement
        if (event->tfinger.touchId == 0)
        {
            float touchX = event->tfinger.x * width;  // Normalize touch position to screen coordinates
            float touchY = event->tfinger.y * height; // Normalize touch position to screen coordinates
            keys[SDLK_w] = touchY < height / 2;       // If touch is in the upper half of the screen, move up
            keys[SDLK_s] = touchY >= height / 2;      // If touch is in the lower half of the screen, move down
            keys[SDLK_a] = touchX < width / 2;        // If touch is in the left half of the screen, move left
            keys[SDLK_d] = touchX >= width / 2;       // If touch is in the right half of the screen, move right
        }
    }
    else if (event->type == SDL_FINGERUP)
    {
        // Reset movement keys
        keys[SDLK_w] = false;
        keys[SDLK_s] = false;
        keys[SDLK_a] = false;
        keys[SDLK_d] = false;
    }

    // Mouse position
    if (event->type == SDL_MOUSEMOTION)
    {
        cursorPos[0] = event->motion.x;
        cursorPos[1] = event->motion.y;
    }
}
