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
float defaultGSV = 8.0f;
GLfloat generationSize[2] = {defaultGSV*2, defaultGSV*2};
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
    makePlayer(registry);

    // Trigger JS functions
    _js__fetch_configs();   
    _js__ready();

    srand(time(NULL));
    // seed = rand() % 100000;
    seed = 85582;
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
    updateFlags(registry);
    updateTeleporters(registry);
    updateShapes(registry);
    updateLinkedEntities(registry);
    updateOther(registry);
    updatePaths(registry);
    updateInteractions(registry);
    updatePositions(registry);
    updateMovement(registry);
    updateCollisions(registry);
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
            int width{0}, height{0};
            textureIDMap[name] = loadGLTexture(shaderProgramMap["texture"], src.c_str(), width, height);
            
            // Set the shape of the texture
            textureShapeMap[name] = {width, height};

            printf("Texture: %s, ID: %d, Width: %d, Height: %d\n", name.c_str(), textureIDMap[name], width, height);
        }
        // set defaultMoveSpeed
        Movement &playerMovement = registry.get<Movement>(_player);
        defaultMoveSpeed = playerMovement.speed;

        runFactories(registry);
        
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
    
    // Update player's TextureAlts based on direction and movement
    if (registry.all_of<TextureAlts>(_player)) {
        auto& textureAlts = registry.get<TextureAlts>(_player);
        bool isMoving = keys[SDLK_w] || keys[SDLK_s] || keys[SDLK_a] || keys[SDLK_d];
        std::string action = isMoving ? "Run" : "Idle";
        static std::string lastDirection = "Down"; // Static variable to remember last direction

        if (keys[SDLK_w]) {
            lastDirection = "Up";
        } else if (keys[SDLK_s]) {
            lastDirection = "Down";
        } else if (keys[SDLK_a]) {
            lastDirection = "Left";
        } else if (keys[SDLK_d]) {
            lastDirection = "Right";
        }

        textureAlts.current = action + "_" + lastDirection;
    }

    // // If T is pressed, increment the player's TextureAlts or Textures .current
    // if (keys[SDLK_t]) {
    //     if (registry.all_of<TextureAlts>(_player)) {
    //         auto& textureAlts = registry.get<TextureAlts>(_player);
    //         auto it = textureAlts.alts.find(textureAlts.current);
    //         if (it != textureAlts.alts.end()) {
    //             it->second.current = (it->second.current + 1) % it->second.textures.size();
    //         }
    //     } else if (registry.all_of<Textures>(_player)) {
    //         auto& textures = registry.get<Textures>(_player);
    //         textures.current = (textures.current + 1) % textures.textures.size();
    //     }
    //     keys[SDLK_t] = false;
    // }

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
        if (registry.all_of<RenderPriority>(lhs) && registry.all_of<RenderPriority>(rhs)) {
            const auto& priority1 = registry.get<RenderPriority>(lhs);
            const auto& priority2 = registry.get<RenderPriority>(rhs);
            if (priority1.priority != priority2.priority) {
                return priority1.priority < priority2.priority;
            }
        }

        return lhs < rhs;
    });


    // Render visible entities
    auto visible_entities = registry.view<Position, Shape, Visible>();
    for(auto& entity : visible_entities) {

        auto &position = visible_entities.get<Position>(entity);
        auto &shape = visible_entities.get<Shape>(entity);
        
        bool isDebug = registry.all_of<Debug>(entity);
        bool isTeleport = registry.all_of<Teleport>(entity);

        // if(entity == _player) {

        //     // Render player at the center of the screen
        //     updateUniformsTexture(shaderProgramMap["texture"], 
        //         textureIDMap["1_Template_Idle_Down-Sheet"],
        //         playerPos.sx + playerShape.scaled_size.x, (playerPos.sy) + playerShape.scaled_size.y,
        //         playerShape.scaled_size.x, playerShape.scaled_size.y);
        //     glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            
        // }

        if (registry.all_of<Texture>(entity)) {
            const auto& texture = registry.get<Texture>(entity);

            // Check if the entity has RenderDebug
            if (registry.all_of<RenderDebug>(entity)) {
                // Render a small transparent square to indicate bounding box
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    1.0f, 1.0f, 1.0f, 0.2f, // White color with 20% opacity
                    position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y,
                    shape.scaled_size.x, shape.scaled_size.y, 0.0f);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap[texture.name],
                position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y,
                shape.scaled_size.x * texture.scalex, shape.scaled_size.y * texture.scaley,
                texture.x, texture.y, texture.w, texture.h);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        } else if (registry.all_of<Textures>(entity)) {
            const auto& textures = registry.get<Textures>(entity);
            const auto& current_texture = textures.textures[textures.current];

            // Check if the entity has RenderDebug
            if (registry.all_of<RenderDebug>(entity)) {
                // Render a small transparent square to indicate bounding box
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    1.0f, 1.0f, 1.0f, 0.2f, // White color with 20% opacity
                    position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y,
                    shape.scaled_size.x, shape.scaled_size.y, 0.0f);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap[current_texture.name],
                position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y,
                shape.scaled_size.x * current_texture.scalex, shape.scaled_size.y * current_texture.scaley,
                current_texture.x, current_texture.y, current_texture.w, current_texture.h);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        } else if (registry.all_of<TextureAlts>(entity)) {
            const auto& textureAlts = registry.get<TextureAlts>(entity);
            const auto& currentTextures = textureAlts.alts.at(textureAlts.current);
            const auto& current_texture = currentTextures.textures[currentTextures.current];

            // Check if the entity has RenderDebug
            if (registry.all_of<RenderDebug>(entity)) {
                // Render a small transparent square to indicate bounding box
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    1.0f, 1.0f, 1.0f, 0.2f, // White color with 20% opacity
                    position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y,
                    shape.scaled_size.x, shape.scaled_size.y, 0.0f);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap[current_texture.name],
                position.sx + playerShape.scaled_size.x,
                position.sy + playerShape.scaled_size.y,
                shape.scaled_size.x * current_texture.scalex, 
                shape.scaled_size.y * current_texture.scaley,
                current_texture.x, current_texture.y, 
                current_texture.w, current_texture.h);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        else if(registry.all_of<Color>(entity)) {
            auto color = registry.get<Color>(entity);

            float angle = 0.0f;
            if(registry.all_of<Rotation>(entity)) {
                angle = registry.get<Rotation>(entity).angle;
            }

            updateUniformsDebug(shaderProgramMap["debug_entity"],
                color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a,
                position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y + shape.scaled_size.z,
                shape.scaled_size.x, shape.scaled_size.y + shape.scaled_size.z, angle);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        else if(isTeleport) {
            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap["door"],
                position.sx + playerShape.scaled_size.x, 
                position.sy + playerShape.scaled_size.y,
                shape.scaled_size.x, shape.scaled_size.y);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        }
    }

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
