#include "../include/events.hpp"
#include "../include/GLUtils.hpp"
#include "../include/JSUtils.hpp"
#include "../include/EntityConfig.hpp"
#include "../include/UIUtils.hpp"
#include "../include/entt.hpp"
#include "../include/Utils.hpp"
#include "../include/EFactory.hpp"
#include "../include/Systems.hpp"

using namespace std;

// External variables
extern entt::registry registry;
extern int width, height;
extern float deltaTime;
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
GLfloat offsetValue[2] = {0.0f, 0.0f};
GLfloat toplefttile[2] = {0.0f, 0.0f};
bool windowResized = false;
bool ready = false;
int lastTime = 0;
float defaultGSV = 16.0f;
GLfloat generationSize[2] = {defaultGSV*2, defaultGSV*2};
GLfloat gridSpacingValue = 1024.0f;
bool first_start = false;
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
    SDL_Window *mpWindow = loadSDL();
    SDL_GLContext gl_context = loadGl(mpWindow);

    loadUI(mpWindow, gl_context);
     
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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

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
        makePlayer(registry);
        runFactories(registry);
    }
    return true; 
}

void mainloop(void *arg)
{
    if(!js_loaded()) return;

    deltaTime = (SDL_GetTicks() - lastTime) / 1000.0f;
    lastTime = SDL_GetTicks();
    // Handle events
    processEvents();
    
    context *ctx = (context *)arg;

    // Check if the window size has been updated
    if (windowResized)
    {
        glViewport(0, 0, width, height);
        SDL_SetWindowSize(ctx->window, width, height);
        windowResized = false;
    }

    // Update frame
    updateFrame();
    // Render
    renderAll();

    // Swap buffers
    SDL_GL_SwapWindow(ctx->window);
    // Update JS Client
    _js__update_client();

    ctx->iteration++;
}
