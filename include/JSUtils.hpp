#pragma once
#include <emscripten.h>
#include "shaders.hpp"
#include "../include/entt.hpp"
#include "../include/structs.hpp"

using namespace std;

extern entt::entity _player;
extern entt::registry registry;

// key, float value
void _js__kvdata(string k, float v)
{
    // Send a float to JS
    EM_ASM_({ Module.setkv(UTF8ToString($0), $1); }, k.c_str(), v);
}

void _js__log(string str)
{
    // Send a log to JS
    EM_ASM_({ console.log(UTF8ToString($0)); }, str.c_str());
}

void _js__ready()
{
    // Send a ready signal to JS
    EM_ASM({
        Module.ready();
    });
}

void _js__refresh()
{
    // Refresh the UI
    EM_ASM({
        Module.refresh();
    });
}

void _js__fetch_configs()
{
    // Fetch the configs from JS
    EM_ASM({
        Module.fetch_configs();
    });
}

void _js__play_tone(string note, string duration, float volume = 0.5)
{
    // Play a tone
    EM_ASM_({
        // Create a volume control and oscillator, then connect them to the main output (your speakers)
        const vol = new Tone.Volume($2).toDestination();
        const osc = new Tone.Oscillator().connect(vol).start();
        
        // Set the oscillator frequency to the specified note and duration
        osc.frequency.value = UTF8ToString($0);
        osc.stop("+" + UTF8ToString($1));
    }, note.c_str(), duration.c_str(), volume);
}

void _js__update_client() {
    Position &playerPos = registry.get<Position>(_player);
    Shape &playerShape = registry.get<Shape>(_player);

    // Update info on front end
    auto& playerCursorPos = registry.get<CursorPosition>(_player);
    _js__kvdata("x", playerPos.x);
    _js__kvdata("y", playerPos.y);
    _js__kvdata("z", playerPos.z);
    _js__kvdata("sx", playerPos.sx);
    _js__kvdata("sy", playerPos.sy);
    _js__kvdata("gridSpacingValue", gridSpacingValue);
    _js__kvdata("cursorX", playerCursorPos.x);
    _js__kvdata("cursorY", playerCursorPos.y);
}

