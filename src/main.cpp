
#include "../include/events.hpp"
#include "../include/GLUtils.hpp"
#include "../include/JSUtils.hpp"
#include "../include/WebUtils.hpp"
#include "../include/lib/entt.hpp"
#include "../include/GameUtils.hpp"
#include "../include/EntityFactory.hpp"
#include "../include/Systems.hpp"
#include "../include/lib/physics.hpp"
#include "../include/SceneManager.hpp"

using namespace std;
// External variables
extern float deltaTime;
extern bool ready;
extern entt::entity _player;
extern p2d::Physics physics;
extern GameState gameState;

// General variables

p2d::Physics physics;
float deltaTime = 0;
bool ready = false;
int lastTime = 0;
GLfloat generationSize[2] = {1.0f*2, 1.0f*2};
bool first_start = false;
GameState gameState;
MetaData metaData;

entt::entity _player = entt::null;
SceneManager sceneManager;

context ctx;
// 
// Function declarations
void mainloop(void *arg);
void EventHandler(int, SDL_Event *);

int main(int argc, char *argv[])
{
    // Initialize SDL and SDL_Image
    SDL_Window *mpWindow = loadSDL();
    SDL_GLContext gl_context = loadGl(mpWindow);

    // Hide the cursor in Emscripten (and SDL in general)
    // SDL_ShowCursor(SDL_DISABLE);
    _js__fetch_configs(); 
    // Trigger JS functions
    // Helper function to read JSON files from embedded filesystem
    // auto readJsonFile = [](const std::string& filePath) -> std::string {
    //     FILE* file = fopen(filePath.c_str(), "r");
    //     if (!file) {
    //         printf("Failed to open JSON file: %s\n", filePath.c_str());
    //         return "";
    //     }

    //     fseek(file, 0, SEEK_END);
    //     long fileSize = ftell(file);
    //     fseek(file, 0, SEEK_SET);

    //     std::string content(fileSize, '\0');
    //     fread(&content[0], 1, fileSize, file);
    //     fclose(file);

    //     return content;
    // };

    // std::string jsonContent = readJsonFile("resources/main.json");
    // if (!jsonContent.empty()) {
    //     // Create a non-const buffer for the function that requires char*
    //     std::vector<char> buffer(jsonContent.begin(), jsonContent.end());
    //     buffer.push_back('\0');
    //     load_json_to_registry(buffer.data(), sceneManager.getCurrentRegistry(), sceneManager.getCurrentMetadata());

    //     isready();
    // }

    physics.setGravity(p2d::Vec2f{0, 0}); // No gravity for top-down game
    physics.setDrag({4.9f, 4.9f}); // Adjust drag for realistic movement

    // Set the main loop
    ctx.window = mpWindow;
    SDL_SetWindowSize(mpWindow, gameState.width, gameState.height);

    emscripten_set_main_loop_arg(mainloop, &ctx, 0, 1);
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);

    // Quit
    SDL_GL_DeleteContext(gl_context);
    SDL_Quit();
    IMG_Quit();

    return EXIT_SUCCESS;
}

bool js_loaded() {
    if(!ready) return false;
    if(!first_start) {
        first_start = true;
        
        metaData = sceneManager.getCurrentMetadata();
        srand(metaData.seed);
        gameState.seed = rand() % 10000;

        loadTextures();
        loadFont();

        makePlayer(sceneManager.getCurrentRegistry());
        runFactories(sceneManager.getCurrentRegistry());

        gameState.gameState = -1;        
        
    }
    return true; 
}
void mainloop(void *arg)
{
    if(!js_loaded()) return;

    deltaTime = (SDL_GetTicks() - lastTime) / 5000.0f;
    lastTime = SDL_GetTicks();
        
    context *ctx = (context *)arg;
    
    // Handle events
    processEvents();

    // Sync global metadata with current scene
    metaData = sceneManager.getCurrentMetadata();
    
    // Update frame
    updateFrame();

    // Render
    renderAll();
    // Swap buffers
    SDL_GL_SwapWindow(ctx->window);
}
