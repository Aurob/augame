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
#include "../include/imgui/imgui.h"
#include "../include/imgui/imgui_impl_sdl.h"
#include "../include/imgui/imgui_impl_opengl3.h"

using namespace std;

// External variables
extern entt::registry registry;
extern int width, height;
extern float deltaTime;
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
GLfloat cursorPos[2] = {0, 0};
GLfloat globalCursorPos[2] = {0, 0};
GLfloat offsetValue[2] = {0.0f, 0.0f};
GLfloat toplefttile[2] = {0.0f, 0.0f};
bool windowResized = false;
bool ready = false;
int lastTime = 0;
float defaultGSV = 16.0f;
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
        printf("SDL_image could not initialize! SDL_image Error: %s\n", SDL_GetError());
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

    SDL_GLContext gl_context = loadGl(mpWindow);

    // Trigger JS functions
    _js__fetch_configs();   
    _js__ready();
    srand(time(NULL));
    // seed = rand() % 100000;
    seed = 85582;
    printf("Seed: %f\n", seed);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(mpWindow, gl_context);
    ImGui_ImplOpenGL3_Init("#version 100");


    // Set the main loop
    ctx.window = mpWindow;
    emscripten_set_main_loop_arg(mainloop, &ctx, 0, 1);
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Quit
    SDL_GL_DeleteContext(gl_context);
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
    CursorPosition &playerCursorPos = registry.get<CursorPosition>(_player);

    // Set view offset
    offsetValue[0] = ((fmod(playerPos.x, defaultGSV) * gridSpacingValue) / defaultGSV) - (playerShape.size.x / 2);
    offsetValue[1] = ((fmod(playerPos.y, defaultGSV) * gridSpacingValue) / defaultGSV) - (playerShape.size.y / 2);

    // Set bounds
    toplefttile[0] = static_cast<int>(playerPos.x / defaultGSV) - (width / gridSpacingValue / 2);
    toplefttile[1] = static_cast<int>(playerPos.y / defaultGSV) - (height / gridSpacingValue / 2);

    // Determine cursor position using toplefttile and offset, factoring in defaultGSV
    playerCursorPos.x = playerPos.x + ((cursorPos[0] - width / 2) * defaultGSV / gridSpacingValue);
    playerCursorPos.y = playerPos.y + ((cursorPos[1] - height / 2) * defaultGSV / gridSpacingValue);

    // printf("Cursor: %f, %f\n", playerCursorPos.x, playerCursorPos.y);
    
    // Update entity movement and interactions
    updateFlags(registry);
    updateCollisions(registry);
    updateMovement(registry);
    updateShapes(registry);
    updatePositions(registry);
    updateOther(registry);
    updateInteractions(registry);
}


void test_imgui(const std::string& displayText, float screenX, float screenY, float scaleX, float scaleY)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(screenX, screenY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(scaleX, scaleY), ImGuiCond_Always);
    ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    
    // Center the text
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 textSize = ImGui::CalcTextSize(displayText.c_str());
    ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
    ImGui::SetCursorPosY((windowSize.y - textSize.y) * 0.5f);
    
    ImGui::Text("%s", displayText.c_str());
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

            // printf("Loading texture: %s\n", name.c_str());
            int width{0}, height{0};
            textureIDMap[name] = loadGLTexture(shaderProgramMap["texture"], src.c_str(), width, height);
            
            // Set the shape of the texture
            textureShapeMap[name] = {width, height};

            // Normalize TextureGroupPart if necessary
            if (textureGroupMap.find(name) != textureGroupMap.end()) {
                for (auto& [partName, part] : textureGroupMap[name]) {
                    if (part.x > 1) part.x /= width;
                    if (part.y > 1) part.y /= height;
                    if (part.w > 1) part.w /= width;
                    if (part.h > 1) part.h /= height;
                }
            }
        }

    
        makePlayer(registry);
        runFactories(registry);

        
        auto key = registry.create();
        registry.emplace<Position>(key, Position{15, 10, 0});
        registry.emplace<Shape>(key, Shape{.25, .25, .1});
        registry.emplace<Color>(key, Color{250, 255, 0, 1.0f});
        registry.emplace<RenderPriority>(key, RenderPriority{0});
        registry.emplace<Debug>(key, Debug{Color{0, 1.0, 0, 1.0f}});
        registry.emplace<Test>(key, Test{"key entity"});
        registry.emplace<Interactable>(key, Interactable{.radius = 1.0f});
        registry.emplace<Collidable>(key, Collidable{.ignorePlayer = true});
        registry.emplace<Hoverable>(key);
        registry.emplace<UIElement>(key, UIElement{"Key", false});
        registry.emplace_or_replace<InteractionAction>(key, InteractionAction{[](entt::registry &registry, entt::entity entity, std::optional<entt::entity> opt_entity) {
            auto& uiElement = registry.get<UIElement>(entity);
            uiElement.visible = !uiElement.visible;
        }, true});

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
        ImGui_ImplSDL2_ProcessEvent(&ctx->event);
        EventHandler(0, &ctx->event);
    }

    auto keys = registry.get<Keys>(_player).keys;

    // If Shift is pressed, increase speed
    auto& playerMovement = registry.get<Movement>(_player);
    if(keys[SDLK_LSHIFT]) {
        playerMovement.speed = defaultMoveSpeed * 20;
    }
    else {
        playerMovement.speed = defaultMoveSpeed;
    }
    
    // If B increase player z
    if(keys[SDLK_b]) {
        auto& playerPos = registry.get<Position>(_player);
        playerPos.z += 1;
        keys[SDLK_b] = false;
    }
    // If N decrease player z
    if(keys[SDLK_n]) {
        auto& playerPos = registry.get<Position>(_player);
        playerPos.z -= 1;
        keys[SDLK_n] = false;
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
        const auto& lhsPos = registry.get<Position>(lhs);
        const auto& rhsPos = registry.get<Position>(rhs);
        const auto& lhsShape = registry.get<Shape>(lhs);
        const auto& rhsShape = registry.get<Shape>(rhs);
        const auto& lhsPrio = registry.get<RenderPriority>(lhs);
        const auto& rhsPrio = registry.get<RenderPriority>(rhs);
        

        // Compare y + z + shape.z
        float lhsYZS = lhsPos.y + lhsPos.z + lhsShape.size.z;
        float rhsYZS = rhsPos.y + rhsPos.z + rhsShape.size.z;
        if (lhsYZS != rhsYZS)
            return lhsYZS < rhsYZS;

        return lhsPrio.priority < rhsPrio.priority;
    });

    // Render visible entities
    auto visible_entities = registry.view<Visible>();
    for(auto& entity : visible_entities) {
        auto position = registry.get<Position>(entity);
        auto shape = registry.get<Shape>(entity);
        
        bool isDebug = registry.all_of<Debug>(entity);
        bool isTeleport = registry.all_of<Teleport>(entity);

 
        if (registry.all_of<Texture>(entity)) {

            const auto& texture = registry.get<Texture>(entity);


            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap[texture.name],
                position.sx + playerShape.scaled_size.x,
                position.sy + playerShape.scaled_size.y + position.sz + playerShape.scaled_size.z,
                shape.scaled_size.x,
                shape.scaled_size.y + shape.scaled_size.z,
                texture.x, texture.y, texture.w, texture.h
            );
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        }
        else if (registry.all_of<TextureGroupPart>(entity)) {
            const auto& textureGroupPart = registry.get<TextureGroupPart>(entity);
            auto groupName = textureGroupPart.groupName;
            auto partName = textureGroupPart.partName;


            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            auto rootTexture = textureIDMap[groupName];
            const auto& texture = textureGroupMap[groupName].at(partName);
            updateUniformsTexture(shaderProgramMap["texture"], 
                rootTexture,
                position.sx + playerShape.scaled_size.x,
                position.sy + playerShape.scaled_size.y + position.sz + playerShape.scaled_size.z,
                shape.scaled_size.x,
                shape.scaled_size.y,
                texture.x, texture.y, texture.w, texture.h
            );
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        } 
        else if (registry.all_of<Textures>(entity)) {
            const auto& textures = registry.get<Textures>(entity);
            const auto& current_texture = textures.textures[textures.current];


            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            if (registry.all_of<RenderDebug>(entity)) {
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    1.0f, 1.0f, 1.0f, 0.2f,
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
        } else if (registry.all_of<TextureAlts, Player>(entity)) {
            const auto& textureAlts = registry.get<TextureAlts>(entity);
            const auto& currentTextures = textureAlts.alts.at(textureAlts.current);
            const auto& current_texture = currentTextures.textures[currentTextures.current];
            // Check if the entity has RenderDebug
            // if (registry.all_of<RenderDebug, Player>(entity)) {
                // Render a small transparent square to indicate bounding box
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    1.0f, 1.0f, 1.0f, 0.2f, // White color with 20% opacity
                    position.sx + playerShape.scaled_size.x, 
                    position.sy + playerShape.scaled_size.y,
                    shape.scaled_size.x, shape.scaled_size.y, 0.0f);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            // }

            // updateUniformsTexture(shaderProgramMap["texture"], 
            //     textureIDMap[current_texture.name],
            //     position.sx + playerShape.scaled_size.x,
            //     position.sy + playerShape.scaled_size.y + position.sz + playerShape.scaled_size.z/2,
            //     shape.scaled_size.x * current_texture.scalex, 
            //     shape.scaled_size.y * current_texture.scaley,
            //     current_texture.x, current_texture.y, 
            //     current_texture.w, current_texture.h);
            // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        }
        else if(registry.all_of<Color>(entity)) {
            auto color = registry.get<Color>(entity);

            float angle = 0.0f;
            if(registry.all_of<Rotation>(entity)) {
                angle = registry.get<Rotation>(entity).angle;
            }

            updateUniformsDebug(shaderProgramMap["debug_entity"],
                color.r, color.g, color.b, color.a,
                position.sx + playerShape.scaled_size.x, 
                position.sy + playerShape.scaled_size.y,
                shape.scaled_size.x, shape.scaled_size.y, 
                angle);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

    }

    updateUniforms2(shaderProgramMap["ui_layer"], 
        width, height, gridSpacingValue,
        toplefttile);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    // Handle UIElement Components
    auto ui_elements = registry.view<UIElement>();
    for(auto& entity : ui_elements) {
        auto& uiElement = registry.get<UIElement>(entity);
        if(uiElement.visible) {
            auto position = registry.get<Position>(entity);
            auto shape = registry.get<Shape>(entity);
            test_imgui(uiElement.content, 
                ( (1 - ((position.sx + playerShape.scaled_size.x*1.25 + shape.scaled_size.x*0.1) + 1) / 2) * width),
                ( (1 - ((position.sy + playerShape.scaled_size.y*1.25 + shape.scaled_size.y*0.1) + 1) / 2) * height),
                (shape.scaled_size.x + shape.scaled_size.x*.1) * width,
                (shape.scaled_size.y + shape.scaled_size.y*.1) * height
            );
        }
    }

    // Swap buffers
    SDL_GL_SwapWindow(ctx->window);

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
    ctx->iteration++;
}
