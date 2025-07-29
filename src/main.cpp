
#include "../include/events.hpp"
#include "../include/GLUtils.hpp"
#include "../include/JSUtils.hpp"
#include "../include/WebUtils.hpp"
#include "../include/lib/entt.hpp"
#include "../include/GameUtils.hpp"
#include "../include/EntityFactory.hpp"
#include "../include/Systems.hpp"
#include "../include/lib/physics.hpp"

using namespace std;
// External variables
extern entt::registry registry;
extern entt::registry registry2;
extern entt::registry registry_temp;
extern float deltaTime;
extern bool windowResized;
extern bool ready;
extern entt::entity _player;
extern float seed;
extern p2d::Physics physics;
extern GameState gameState;

// General variables

p2d::Physics physics;
float deltaTime = 0;
bool windowResized = false;
bool ready = false;
int lastTime = 0;
GLfloat generationSize[2] = {16.0f*2, 16.0f*2};
bool first_start = false;
float seed = 0.0f;
GameState gameState;

entt::entity _player = entt::null;
entt::registry registry;
entt::registry registry2;
entt::registry registry_temp;

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

    // Trigger JS functions
    _js__fetch_configs();   
    _js__ready();
    srand(time(NULL));
    // seed = rand() % 100000;
    seed = 85582;
    printf("Seed: %f\n", seed);

    physics.setGravity(p2d::Vec2f{0, 0}); // No gravity for top-down game
    physics.setDrag({4.9f, 4.9f}); // Adjust drag for realistic movement

    // Set the main loop
    ctx.window = mpWindow;
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
        
        loadTextures();
        loadFont();

        makePlayer(registry);
        runFactories(registry);

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

    // Check if the window size has been updated
    if (windowResized)
    {
        glViewport(0, 0, gameState.width, gameState.height);
        SDL_SetWindowSize(ctx->window, gameState.width, gameState.height);
        windowResized = false;
    }
    
    // Update frame
    updateFrame();

    // Render
    renderAll();
    // Swap buffers
    SDL_GL_SwapWindow(ctx->window);
}
