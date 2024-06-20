#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_opengles2.h>
#include <unordered_map>
#include <cmath>
#include <time.h>

#include "../include/events.hpp"
#include "../include/GLUtils.hpp"
#include "../include/JSUtils.hpp"
#include "../include/entt.hpp"
#include "../include/structs.hpp"
#include "../include/Utils.hpp"
#include "../include/EFactory.hpp"
#include "../include/Systems.hpp"

using namespace std;

extern entt::registry registry;

// External variables
extern int width, height;
extern float deltaTime;
extern int seed;
extern unordered_map<int, bool> keys;
extern GLfloat cursorPos[2];
extern int tileMinMax[4];
extern bool windowResized;
extern GLfloat playerPosition[2];
extern GLfloat gridSpacingValue;
extern bool ready;

entt::registry registry;

// General variables
int width = 1024;
int height = 1024;
float deltaTime = 0;
int seed = 0;
unordered_map<int, bool> keys;
GLfloat cursorPos[2] = {0, 0};
GLfloat globalCursorPos[2] = {0, 0};
GLfloat playerPosition[2] = {0, 0};
Shape playerShape;
GLfloat tempPlayerPosition[2] = {0, 0};
GLfloat gridSpacingValue = 2048.0f;
GLfloat offsetValue[2] = {0.0f, 0.0f};
GLfloat toplefttile[2] = {0.0f, 0.0f};
float scale = 1;
int tileMinMax[4] = {0, 0, 0, 0};
bool windowResized = false;
bool ready = false;
float moveSpeed = 1;
float defaultMoveSpeed = moveSpeed;
float deltaX = 0;
float deltaY = 0;
int frameTime = 0;
int lastTime = 0;
int numEntities = 0;
GLuint textureID;
GLint centerX;
GLint centerY;
GLuint *shaderProgram;
GLuint *shaderProgram2;
GLuint *shaderProgramTexture;
float defaultGSV = 16.0f;
SDL_Surface *image = nullptr;
bool first_start = false;
bool one_time = false;
float _moveSpeed = 0;

context ctx;

void mainloop(void *arg);
void EventHandler(int, SDL_Event *);
void animations(context *ctx);

int main(int argc, char *argv[])
{
    seed = 0;
    shaderProgram = new GLuint;
    shaderProgram2 = new GLuint;
    shaderProgramTexture = new GLuint;
    printf("Seed: %d\n", seed);

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

    ctx.window = mpWindow;

    // Link vertex and fragment shader into shader program and use it
        // Create OpenGLES 2 context on SDL window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GLContext glc = SDL_GL_CreateContext(mpWindow);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    
    centerX = width / 2;
    centerY = height / 2;

    for(int i = 0; i < 100; i++) {
        auto entity = createDebugEntity(registry);
    }
    // create player entity
    auto player = createPlayerEntity(registry);

    createDebugBuilding(registry);
    createDebugTeleporter(registry);

    _js__kvdata("test", 1234);

    _js__fetch_configs();   
    _js__ready();
    
    // Set the main loop
    emscripten_set_main_loop_arg(mainloop, &ctx, 0, 1);
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);

    // Quit
    glDeleteTextures(1, &textureID);
    SDL_Quit();
    IMG_Quit();

    return EXIT_SUCCESS;
}

void updateFrame()
{
    if (keys[ZOOM_IN])
    {
        // gridSpacingValue *= 2.0f;
        gridSpacingValue *= 1.1f;
        keys[ZOOM_IN] = false;
    }
    else if (keys[ZOOM_OUT])
    {
        // gridSpacingValue /= 2.0f;
        gridSpacingValue /= 1.1f;
        keys[ZOOM_OUT] = false;
    }

    if (keys[SPEED_MULTI])
    {
        moveSpeed = defaultMoveSpeed * 20.0f;
        keys[SPEED_MULTI] = false;
    }
    else if (keys[SPEED_DIV])
    {
        moveSpeed = defaultMoveSpeed;
        keys[SPEED_DIV] = false;
    }

    // update entity movement
    updateMovement(registry);
    updateCollisions(registry);

    // tempPlayerPosition[0] = playerPosition[0];
    // tempPlayerPosition[1] = playerPosition[1];

    // Update player position
    // _moveSpeed = moveSpeed * deltaTime;

    // deltaX = (keys[SDLK_d] - keys[SDLK_a]);
    // deltaY = (keys[SDLK_s] - keys[SDLK_w]);
    // float normFactor = sqrt(deltaX * deltaX + deltaY * deltaY);
    // if (normFactor != 0) {
    //     deltaX /= normFactor;
    //     deltaY /= normFactor;
    // }

    // playerPosition[0] -= deltaX * _moveSpeed;
    // playerPosition[1] += deltaY * _moveSpeed;

    // update shapes
    auto entities = registry.view<Shape>();
    for(auto& entity : entities) {
        auto &shape = entities.get<Shape>(entity);
        shape.scaled_size.x = (shape.size.x / defaultGSV) * gridSpacingValue / width;
        shape.scaled_size.y = (shape.size.y / defaultGSV) * gridSpacingValue / height;

        // (0.1f * gridSpacingValue / 1000.0f) * 
    }

    // Check if mouse is colliding with any Debug entities. use the screen position
    auto debug_entities = registry.view<Position, Shape, Debug, Hoverable, Interactable, Color>();
    for(auto& entity : debug_entities) {
        auto &position = debug_entities.get<Position>(entity);
        auto &debug = debug_entities.get<Debug>(entity);

        // Convert cursor position to normalized screen coordinates
        float normalizedCursorX = (2.0f * cursorPos[0] / width) - 1.0f;
        float normalizedCursorY = 1.0f - (2.0f * cursorPos[1] / height);

        // Convert entity position to screen coordinates
        float screenEntityX = (position.sx);
        float screenEntityY = (position.sy);


        // Check for collision
        bool is_hovered = registry.all_of<Hovered>(entity);
        
        auto &color = debug_entities.get<Color>(entity);

        auto &shape = debug_entities.get<Shape>(entity);
        float radiusX = shape.scaled_size.x;
        float radiusY = shape.scaled_size.y;
        if (fabs(normalizedCursorX - screenEntityX) < radiusX && fabs(normalizedCursorY - screenEntityY) < radiusY) {
            // Handle collision with debug entity
            if (!is_hovered) {
                registry.emplace<Hovered>(entity);
            }

            if (keys[SDL_BUTTON_LEFT]) {
                if (!registry.all_of<Interacted>(entity)) {
                    registry.emplace<Interacted>(entity);
                }
                // Change color to blue when interacted and hovered
                color.r = 0;
                color.g = 0;
                color.b = 255;

                // update entity position based on globalCursorPos
                position.x = globalCursorPos[0];
                position.y = globalCursorPos[1];
                
            } else {
                // Change color to red when only hovered
                color.r = 255;
                color.g = 0;
                color.b = 0;
            }
        } else if (is_hovered) {
            registry.remove<Hovered>(entity);
            if (registry.all_of<Interacted>(entity)) {
                registry.remove<Interacted>(entity);
            }

            // Reset color to default
            color.r = debug.defaultColor.r;
            color.g = debug.defaultColor.g;
            color.b = debug.defaultColor.b;
        }
    }

    auto player = registry.view<Position, Shape, Player>();
    for(auto& entity : player) {
        auto &position = player.get<Position>(entity);
        auto &shape = player.get<Shape>(entity);
        playerPosition[0] = position.x;
        playerPosition[1] = position.y;
        playerShape = shape;

    }

    // set view offset
    offsetValue[0] = (fmod(playerPosition[0], defaultGSV) * gridSpacingValue) / defaultGSV;
    offsetValue[1] = (fmod(playerPosition[1], defaultGSV) * gridSpacingValue) / defaultGSV;

    // set bounds
    toplefttile[0] = static_cast<int>(playerPosition[0] / defaultGSV) - (width / gridSpacingValue / 2);
    toplefttile[1] = static_cast<int>(playerPosition[1] / defaultGSV) - (height / gridSpacingValue / 2);

    // Calculate cursor global position based on calculated player position, tile, and offset
    globalCursorPos[0] = toplefttile[0]*defaultGSV + (cursorPos[0]/gridSpacingValue) + playerPosition[0];
    globalCursorPos[1] = toplefttile[1]*defaultGSV + (cursorPos[1]/gridSpacingValue) + playerPosition[1];
    
    // do collision checks for teleport entities
    auto teleport_entities = registry.view<Position, Shape, Teleport, Validation>();
    for(auto& entity : teleport_entities) {
        auto &position = teleport_entities.get<Position>(entity);
        auto &shape = teleport_entities.get<Shape>(entity);
        auto &teleport = teleport_entities.get<Teleport>(entity);
        auto &validation = teleport_entities.get<Validation>(entity);


        if(positionsCollide(playerPosition[0], playerPosition[1], playerShape.size.x, playerShape.size.y, position.x, position.y, shape.size.x, shape.size.y)) {

            for(auto& entity : player) {
                auto &position = player.get<Position>(entity);
                position.x = teleport.destination.x;
                position.y = teleport.destination.y;
                playerPosition[0] = teleport.destination.x;
                playerPosition[1] = teleport.destination.y;
            }
        }
    }



}


void mainloop(void *arg)
{
    deltaTime = (SDL_GetTicks() - lastTime) / 1000.0f;
    lastTime = SDL_GetTicks();
    context *ctx = static_cast<context *>(arg);

    // Check if the window size has been updated
    if (windowResized)
    {
        glViewport(0, 0, width, height);
        SDL_SetWindowSize(ctx->window, width, height);

        windowResized = false;
    }

    if(!ready) return;
    else if(!first_start) {
        first_start = true;
        loadGL1(shaderProgramMap["terrain"], "terrain");
        loadGL1(shaderProgramMap["ui_layer"], "ui_layer");
        loadGL1(shaderProgramMap["debug_entity"], "debug_entity");
        loadGL1(shaderProgramMap["texture"], "texture");
        // load textures from textureMap
        for(auto& [name, src] : textureMap) {
            printf("Loading texture: %s\n", src.c_str());
            textureIDMap[name] = loadGLTexture(shaderProgramMap["texture"], src.c_str());
        }
    }

    while (SDL_PollEvent(&ctx->event))
    {
        EventHandler(0, &ctx->event);
    }


    updateFrame();
    
    glClear(GL_COLOR_BUFFER_BIT);

    // Render terrain (first shader)
    updateUniforms(
        shaderProgramMap["terrain"],
        gridSpacingValue, 
        offsetValue, 
        width, height, 
        playerPosition, 
        toplefttile, 
        scale);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render the crosshair (third shader)
    // updateUniforms2(shaderProgramMap["ui_layer"], width, height, gridSpacingValue);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Render player texture: 'smile'
    auto player = registry.view<Position, Shape, Player>();
    for(auto& entity : player) {
        auto &position = player.get<Position>(entity);
        auto &shape = player.get<Shape>(entity);
        updateUniformsTexture(shaderProgramMap["texture"], 
            textureIDMap["smile"],
            position.sx, position.sy, 
            shape.scaled_size.x, shape.scaled_size.y);
    }
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Render entities shape
    auto entities = registry.view<Position, Shape, Validation>();

    for(auto& entity : entities) {

        auto &position = entities.get<Position>(entity);
        auto &shape = entities.get<Shape>(entity);
        auto &validation = entities.get<Validation>(entity);
        if(validation.state == 2) {
            continue;
        }
        bool has_visible = registry.all_of<Visible>(entity);
        
        float entity_player_diffX = (playerPosition[0] - position.x);
        float entity_player_diffY = (playerPosition[1] - position.y);


        float distanceX = abs(entity_player_diffX * gridSpacingValue / defaultGSV);
        float distanceY = abs(entity_player_diffY * gridSpacingValue / defaultGSV);

        if (distanceX < (width - shape.size.x) / 2 && distanceY < (height - shape.size.y) / 2) {

            float posX = (playerPosition[0] - position.x) * gridSpacingValue + centerX;
            float posY = (playerPosition[1] - position.y) * gridSpacingValue + centerY;

            position.sx = (2 * posX / width - 1)/defaultGSV;
            position.sy = (2 * posY / height - 1)/defaultGSV;

            position.sx -= shape.scaled_size.x * (.999);
            position.sy -= shape.scaled_size.y * (.999);

            // position.sx -= _moveSpeed/width;
            // position.sy += _moveSpeed/height;
            
            bool isDebug = registry.all_of<Debug>(entity);
            bool isTeleport = registry.all_of<Teleport>(entity);
            // if(isDebug || isTeleport) {
            //     validation.state = 1;
            // }
            // // check if entity has been validated
            // else if (validation.state == 0) {
            //     // check if the entity is on the grass, if not remove it and skip
            //     unsigned char pixel[4];
            //     glReadPixels(position.sx * width / 2 + width / 2, 
            //         position.sy * height / 2 + height / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

            //     if (pixel[0] == 48 && pixel[1] == 71 && pixel[2] == 40) {
            //         validation.state = 1;
            //     } 
            //     else {
            //         validation.state = 2;
            //     }
            // }

            validation.state = 1;

            // // check if entity is visible
            if (validation.state == 1 && !has_visible) {
                registry.emplace<Visible>(entity);
            }
        }
        else if(has_visible) {
            registry.remove<Visible>(entity);
        }
    }

    // Render visible test entities
    auto visible_entities = registry.view<Position, Shape, Validation, Visible>();

    for(auto& entity : visible_entities) {
        auto &position = visible_entities.get<Position>(entity);
        auto &shape = visible_entities.get<Shape>(entity);
        auto &validation = visible_entities.get<Validation>(entity);
        
        if(validation.state == 1) {

            bool isDebug = registry.all_of<Debug>(entity);
            bool isTeleport = registry.all_of<Teleport>(entity);
            if(isDebug && registry.all_of<Color>(entity)) {
                auto &color = registry.get<Color>(entity);
                auto &shape = registry.get<Shape>(entity);
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a,
                    position.sx, position.sy, 
                    // (0.1f * gridSpacingValue / 1000.0f) * shape.w/2, (0.1f * gridSpacingValue / 1000.0f) * shape.h/2);
                    shape.scaled_size.x, shape.scaled_size.y);
            }
            else if(isTeleport) {
                auto &teleport = registry.get<Teleport>(entity);
                updateUniformsTexture(shaderProgramMap["texture"], 
                    textureIDMap["door"],
                    position.sx, position.sy,
                    // (0.1f * gridSpacingValue / 1000.0f) * shape.w/2, (0.1f * gridSpacingValue / 1000.0f) * shape.h/2);
                    shape.scaled_size.x, shape.scaled_size.y);

            }
            else {
                // tree 
                updateUniformsTexture(shaderProgramMap["texture"], 
                    textureIDMap["tree"],
                    position.sx,position.sy,
                    // (0.1f * gridSpacingValue / 1000.0f) * shape.w/2, (0.1f * gridSpacingValue / 1000.0f) * shape.h/2);
                    shape.scaled_size.x, shape.scaled_size.y);
            }

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }

    // Swap buffers
    SDL_GL_SwapWindow(ctx->window);


    // Update Info on front end
    _js__kvdata("x", playerPosition[0]);
    _js__kvdata("y", playerPosition[1]);
    _js__kvdata("tilex", static_cast<int>(playerPosition[0] / defaultGSV));
    _js__kvdata("tiley", static_cast<int>(playerPosition[1] / defaultGSV));
    _js__kvdata("scale", gridSpacingValue);
    ctx->iteration++;
}
