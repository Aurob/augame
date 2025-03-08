#pragma once
#include <emscripten.h>
#include "shaders.hpp"
#include "lib/entt.hpp"
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

void _js__fetch_configs()
{
    // Fetch the configs from JS
    EM_ASM({
        Module.fetch_configs();
    });
}
enum LogLevel {
    CONSOLE = EM_LOG_CONSOLE,
    WARN = EM_LOG_WARN, 
    ERROR = EM_LOG_ERROR,
    DEBUG = EM_LOG_DEBUG,
    INFO = EM_LOG_INFO
};

void emlog(const char* msg, LogLevel level = LogLevel::CONSOLE) {
    // Supports log levels:
    // LogLevel::CONSOLE - Standard output (default)
    // LogLevel::WARN - Warnings 
    // LogLevel::ERROR - Errors
    // LogLevel::DEBUG - Debug
    // LogLevel::INFO - Info
    // Can combine with | for multiple flags
    emscripten_log(static_cast<int>(level), "%s", msg);
}