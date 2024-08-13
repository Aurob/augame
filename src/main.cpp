#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_opengles2.h>
#include <unordered_map>
#include <cmath>

#include "../include/events.hpp"
#include "../include/GLUtils.hpp"
#include "../include/JSUtils.hpp"
#include "../include/entt.hpp"
#include "../include/structs.hpp"
#include "../include/Utils.hpp"
#include "../include/EFactory.hpp"
#include "../include/Systems.hpp"

using namespace std;

// External variables
extern entt::registry registry;
extern int width, height;
extern float deltaTime;
extern unordered_map<int, bool> keys;
extern GLfloat cursorPos[2];
extern GLfloat globalCursorPos[2];
extern bool windowResized;
extern GLfloat gridSpacingValue;
extern bool ready;
extern float defaultGSV;
extern GLfloat toplefttile[2];
extern entt::entity _player;
extern float seed;

// General variables
int width = 1024;
int height = 1024;
float deltaTime = 0;
unordered_map<int, bool> keys;
GLfloat cursorPos[2] = {0, 0};
GLfloat globalCursorPos[2] = {0, 0};
GLfloat offsetValue[2] = {0.0f, 0.0f};
GLfloat toplefttile[2] = {0.0f, 0.0f};
bool windowResized = false;
bool ready = false;
int lastTime = 0;
float defaultGSV = 16.0f;
GLfloat generationSize[2] = {defaultGSV/3, defaultGSV/3};
GLfloat gridSpacingValue = 1024.0f;
bool first_start = false;
float defaultMoveSpeed = 0.0f;
float seed = 0.0f;

entt::entity _player;
entt::registry registry;

context ctx;
// 
// Function declarations
void mainloop(void *arg);
void EventHandler(int, SDL_Event *);

int main(int argc, char *argv[])
{
    // Initialize SDL and SDL_Image
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return -1;
    }

    SDL_Window *mpWindow = SDL_CreateWindow("Untitled",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    if (!mpWindow) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    loadGl(mpWindow);
    runFactories(registry);

    // Trigger JS functions
    _js__fetch_configs();   
    _js__ready();
    
    srand(time(NULL));
    seed = 82938; //rand() % 100000;
    printf("Seed: %f\n", seed);

    // Set the main loop
    ctx.window = mpWindow;
    emscripten_set_main_loop_arg(mainloop, &ctx, 0, 1);
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);

    // Quit
    SDL_Quit();
    IMG_Quit();

    return EXIT_SUCCESS;
}

void updateFrame(context *ctx)
{
    // Check if the window size has been updated
    if (windowResized)
    {
        glViewport(0, 0, width, height);
        SDL_SetWindowSize(ctx->window, width, height);
        windowResized = false;
    }

    // Update player-based calculations
    Position &playerPos = registry.get<Position>(_player);
    Shape &playerShape = registry.get<Shape>(_player);

    // Set view offset
    offsetValue[0] = ((fmod(playerPos.x, defaultGSV) * gridSpacingValue) / defaultGSV) - (playerShape.size.x / 2);
    offsetValue[1] = ((fmod(playerPos.y, defaultGSV) * gridSpacingValue) / defaultGSV) - (playerShape.size.y / 2);

    // Set bounds
    toplefttile[0] = static_cast<int>(playerPos.x / defaultGSV) - (width / gridSpacingValue / 2);
    toplefttile[1] = static_cast<int>(playerPos.y / defaultGSV) - (height / gridSpacingValue / 2);

    // Calculate cursor global position
    globalCursorPos[0] = toplefttile[0] * defaultGSV + (cursorPos[0] / gridSpacingValue) + playerPos.x;
    globalCursorPos[1] = toplefttile[1] * defaultGSV + (cursorPos[1] / gridSpacingValue) + playerPos.y;

    // Update entity movement and interactions
    updateTeleporters(registry);
    updateInteractions(registry);
    updateShapes(registry);
    updateLinkedEntities(registry);
    updateMovement(registry);
    updateCollisions(registry);
    updateOther(registry);
    updatePaths(registry);
    updatePositions(registry);

}

bool js_loaded() {
    if(!ready) return false;
    if(!first_start) {
        first_start = true;
        loadGL1(shaderProgramMap["terrain"], "terrain");
        loadGL1(shaderProgramMap["ui_layer"], "ui_layer");
        loadGL1(shaderProgramMap["debug_entity"], "debug_entity");
        loadGL1(shaderProgramMap["texture"], "texture");
        // Load textures from textureMap
        for(auto& [name, src] : textureMap) {
            printf("Loading texture: %s\n", src.c_str());
            textureIDMap[name] = loadGLTexture(shaderProgramMap["texture"], src.c_str());
        }

        // set defaultMoveSpeed
        Movement &playerMovement = registry.get<Movement>(_player);
        defaultMoveSpeed = playerMovement.speed;
    }
    return true;
}

bool onetime = false;
void mainloop(void *arg)
{
    if(!js_loaded()) return;

    deltaTime = (SDL_GetTicks() - lastTime) / 1000.0f;
    lastTime = SDL_GetTicks();
    context *ctx = static_cast<context *>(arg);

    while (SDL_PollEvent(&ctx->event))
    {
        EventHandler(0, &ctx->event);
    }

    // If Shift is pressed, increase speed
    auto& playerMovement = registry.get<Movement>(_player);
    if(keys[SDLK_LSHIFT]) {
        playerMovement.speed = defaultMoveSpeed * 20;
    }
    else {
        playerMovement.speed = defaultMoveSpeed;
    }

    // Update frame
    updateFrame(ctx);

    Position &playerPos = registry.get<Position>(_player);
    Shape &playerShape = registry.get<Shape>(_player);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bool playerIsInside = registry.all_of<Inside>(_player);
    Inside playerInside = (playerIsInside) ? registry.get<Inside>(_player) : Inside{};

    if(!playerIsInside) {
        // Render terrain
        updateUniforms(
            shaderProgramMap["terrain"],
            gridSpacingValue, 
            offsetValue, 
            width, height, 
            toplefttile,
            generationSize
        );
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    
    registry.sort<Visible>([&](const entt::entity lhs, const entt::entity rhs) {
        // Check if either entity has RenderPriority component
        // bool lhs_has_priority = registry.all_of<RenderPriority>(lhs);
        // bool rhs_has_priority = registry.all_of<RenderPriority>(rhs);

        // // If both have RenderPriority, compare their priority values
        // if (lhs_has_priority && rhs_has_priority) {
        //     return registry.get<RenderPriority>(lhs).priority > registry.get<RenderPriority>(rhs).priority;
        // }

        // // If only one has RenderPriority, it should be rendered first
        // if (lhs_has_priority) {
        //     return true;
        // }
        // if (rhs_has_priority) {
        //     return false;
        // }
        // Default to x-order, then y-order
        const auto& lhsPos = registry.get<Position>(lhs);
        const auto& rhsPos = registry.get<Position>(rhs);
        if (lhsPos.x == rhsPos.x) {
            return lhsPos.y < rhsPos.y;
        }
        return lhsPos.x < rhsPos.x;
    });


    // Render visible entities
    auto visible_entities = registry.view<Position, Shape, Visible, RenderPriority>();
    for(auto& entity : visible_entities) {

        if(registry.all_of<RenderPriority>(entity) && keys[SDLK_p]) {
            auto &priority = registry.get<RenderPriority>(entity);
            printf("Priority: %d\n", priority.priority);
        }

        auto &position = visible_entities.get<Position>(entity);
        auto &shape = visible_entities.get<Shape>(entity);
        
        bool isDebug = registry.all_of<Debug>(entity);
        bool isTeleport = registry.all_of<Teleport>(entity);

        if(entity == _player) {

            // Render player at the center of the screen
            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap["smile"],
                playerPos.sx + playerShape.scaled_size.x, (playerPos.sy) + playerShape.scaled_size.y,
                playerShape.scaled_size.x, playerShape.scaled_size.y);
            
        }
        else if(isDebug && registry.all_of<Color>(entity)) {
            auto color = registry.get<Color>(entity);

            float angle = 0.0f;
            if(registry.all_of<Rotation>(entity)) {
                angle = registry.get<Rotation>(entity).angle;
            }

            updateUniformsDebug(shaderProgramMap["debug_entity"],
                color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a,
                position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y,
                shape.scaled_size.x, shape.scaled_size.y, angle);

        }
        else if(isTeleport) {
            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap["door"],
                position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y,
                shape.scaled_size.x, shape.scaled_size.y);
        }
        else {
            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap["tree"],
                position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y,
                shape.scaled_size.x, shape.scaled_size.y);
        }
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    // Render player at the center of the screen
    updateUniforms2(shaderProgramMap["ui_layer"], 
        width, height, gridSpacingValue,
        toplefttile);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    // Swap buffers
    SDL_GL_SwapWindow(ctx->window);

    // Update info on front end
    _js__kvdata("x", playerPos.x);
    _js__kvdata("y", playerPos.y);
    _js__kvdata("z", playerPos.z);
    _js__kvdata("gridSpacingValue", gridSpacingValue);
    ctx->iteration++;
}
