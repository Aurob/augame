#pragma once
#include <vector>
#include <SDL2/SDL.h>
#include <pair>

using namespace std;

struct Position {
    pair<float, float> global;
    pair<int, int> tile;
    pair<float, float> screen;
};