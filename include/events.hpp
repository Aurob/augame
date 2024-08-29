#pragma once

#include <SDL2/SDL.h>
#include "entt.hpp"
#include "structs.hpp"

using namespace std;

extern int width, height;
extern GLfloat cursorPos[2];
extern float gridSpacingValue;
extern entt::entity _player;
extern entt::registry registry;

void EventHandler(int type, SDL_Event *event)
{
    // wasd for offset
    auto &playerKeys = registry.get<Keys>(_player).keys;
    if (event->type == SDL_KEYDOWN)
    {
        playerKeys[event->key.keysym.sym] = true;
    }
    else if (event->type == SDL_KEYUP)
    {
        playerKeys[event->key.keysym.sym] = false;
    }

    // Mouse/Touch position
    if (event->type == SDL_MOUSEMOTION || event->type == SDL_FINGERMOTION)
    {
        if (event->type == SDL_MOUSEMOTION)
        {
            cursorPos[0] = event->motion.x;
            cursorPos[1] = event->motion.y;
        }
        else
        {
            cursorPos[0] = event->tfinger.x * width;
            cursorPos[1] = event->tfinger.y * height;
        }
    }

    // Mouse/Touch Interactions
    if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_FINGERDOWN)
    {
        playerKeys[SDL_BUTTON_LEFT] = true;
    }
    else if (event->type == SDL_MOUSEBUTTONUP || event->type == SDL_FINGERUP)
    {
        playerKeys[SDL_BUTTON_LEFT] = false;
    }

    // Zoom in and out (Mouse wheel and pinch)
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
    else if (event->type == SDL_MULTIGESTURE)
    {
        if (event->mgesture.numFingers == 2)
        {
            if (event->mgesture.dDist > 0)
            {
                gridSpacingValue *= (1.0f + event->mgesture.dDist);
            }
            else if (event->mgesture.dDist < 0)
            {
                gridSpacingValue /= (1.0f - event->mgesture.dDist);
            }
        }
    }
}
