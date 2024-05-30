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


using namespace std;

// External variables
extern int width, height;
extern float deltaTime;
extern int seed;
extern unordered_map<int, bool> keys;
extern GLfloat cursorPos[2];
// Shader uniform variables
extern float waterMax;
extern float sandMax;
extern float dirtMax;
extern float grassMax;
extern float stoneMax;
extern float snowMax;
extern float frequency;
extern float amplitude;
extern float persistence;
extern float lacunarity;
extern float scale;
extern int octaves;
extern int tileMinMax[4];
extern bool windowResized;
extern GLfloat playerPosition[2];
extern GLfloat gridSpacingValue;
extern bool ready;

float deltaTime = 0;
int seed = 0;
unordered_map<int, bool> keys;
GLfloat cursorPos[2] = {0, 0};

GLuint *shaderProgram;
GLuint *shaderProgram2;
GLuint *shaderProgramTexture;

GLfloat positions[1][2]; // Adjust the size as needed
int numPositions = 0;

GLfloat gridSpacingValue = 8.0f;
GLfloat offsetValue[2] = {0.0f, 0.0f};
int width = 1024;
int height = 1024;
GLfloat playerPosition[2] = {0, 0};
GLfloat toplefttile[2] = {0.0f, 0.0f};
int tileMinMax[4] = {0, 0, 0, 0};
float waterMax;
float sandMax;
float dirtMax;
float grassMax;
float stoneMax;
float snowMax;
int lastTime = 0;
float frequency = -0.07243269;
float amplitude = 1.0;
float persistence = 0.5;
float lacunarity = 2.0;
float scale = 1;
int octaves = 12;
bool windowResized = false;
bool ready = false;

GLubyte pixelData[4];

float defaultGSV = gridSpacingValue;
GLfloat tempPlayerPosition[2] = {0, 0};

SDL_Surface *image = nullptr;

GLuint textureID;
GLint centerX;
GLint centerY;
float moveSpeed = 1;
float defaultMoveSpeed = moveSpeed;
bool zooming = false;
// General variables
int frameTime = 0;
// Set structs
struct context
{
    SDL_Event event;
    int iteration;
    SDL_Window *window;
};

context ctx;

void mainloop(void *arg);
void EventHandler(int, SDL_Event *);
void animations(context *ctx);

int main(int argc, char *argv[])
{

    // seed = time(0);
    // srand(seed);
    // seed = rand() % 100000;
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

    SDL_Window *mpWindow = SDL_CreateWindow("Shader Terrain",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    if (!mpWindow) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;

    }

    ctx.window = mpWindow;

    // Link vertex and fragment shader into shader program and use it
    loadGl(mpWindow);
    
    // --------------------------------
    centerX = width / 2;
    centerY = height / 2;

    // Set random position
    // playerPosition[0] = static_cast<float>(playerPosition[0]);
    // playerPosition[1] = static_cast<float>(playerPosition[1]);
    playerPosition[0] = rand() % 1000;
    playerPosition[1] = rand() % 1000;

    positions[0][0] = playerPosition[0];
    positions[0][1] = playerPosition[1];
    numPositions = 1;

    // Assume total weight = 1.0; adjust weights as desired
    const float totalWeight = 1.0;
    float weights[] = {0.09f, 0.03f, 0.05f, 0.25f, 0.7f, 0.09f}; // Example weights for water, sand, dirt, grass, stone, snow

    // Calculate cumulative weights
    for(int i = 1; i < sizeof(weights)/sizeof(weights[0]); ++i) {
        weights[i] += weights[i-1];
    }

    // Normalize and set terrain max values based on weighted randomness
    waterMax = weights[0] * totalWeight;
    sandMax = weights[1] * totalWeight;
    dirtMax = weights[2] * totalWeight;
    grassMax = weights[3] * totalWeight;
    stoneMax = weights[4] * totalWeight;
    snowMax = weights[5] * totalWeight; // This should naturally be totalWeight (1.0) if weights sum correctly

    // Send variable defaults to JS with _js__kvdata(key, value)
    _js__kvdata("waterMax", waterMax);
    _js__kvdata("sandMax", sandMax);
    _js__kvdata("dirtMax", dirtMax);
    _js__kvdata("grassMax", grassMax);
    _js__kvdata("stoneMax", stoneMax);
    _js__kvdata("snowMax", snowMax);
    _js__kvdata("frequency", frequency);
    _js__kvdata("amplitude", amplitude);
    _js__kvdata("persistence", persistence);
    _js__kvdata("lacunarity", lacunarity);
    _js__kvdata("scale", scale);
    _js__kvdata("octaves", octaves);

    _js__ready();
    
    // Set the main loop
    emscripten_set_main_loop_arg(mainloop, &ctx, 0, 1);
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);

    // Quit
    SDL_DestroyWindow(mpWindow);
    SDL_Quit();
    IMG_Quit();

    return EXIT_SUCCESS;
}

void updateFrame()
{
    if (keys[ZOOM_IN])
    {
        gridSpacingValue *= 2.0f;
        keys[ZOOM_IN] = false;
    }
    else if (keys[ZOOM_OUT])
    {
        gridSpacingValue /= 2.0f;
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

    // Update player position
    float _moveSpeed = moveSpeed * deltaTime;
    tempPlayerPosition[0] = playerPosition[0];
    tempPlayerPosition[1] = playerPosition[1];

    float deltaX = (keys[SDLK_d] - keys[SDLK_a]);
    float deltaY = (keys[SDLK_s] - keys[SDLK_w]);
    float normFactor = sqrt(deltaX * deltaX + deltaY * deltaY);
    if (normFactor != 0) {
        deltaX /= normFactor;
        deltaY /= normFactor;
    }

    playerPosition[0] -= deltaX * _moveSpeed;
    playerPosition[1] += deltaY * _moveSpeed;

    
    // set view offset
    offsetValue[0] = (fmod(playerPosition[0], defaultGSV) * gridSpacingValue) / defaultGSV;
    offsetValue[1] = (fmod(playerPosition[1], defaultGSV) * gridSpacingValue) / defaultGSV;

    // set bounds
    toplefttile[0] = static_cast<int>(playerPosition[0] / defaultGSV) - (width / gridSpacingValue / 2);
    toplefttile[1] = static_cast<int>(playerPosition[1] / defaultGSV) - (height / gridSpacingValue / 2);
}

bool first_start = false;
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
        loadGL1(*shaderProgram);
        loadGL2(*shaderProgram2);
        loadGLTexture(*shaderProgramTexture);
    }

    while (SDL_PollEvent(&ctx->event))
    {
        EventHandler(0, &ctx->event);
    }

    updateFrame();

    glClear(GL_COLOR_BUFFER_BIT);

    // Render terrain (first shader)
    updateUniforms(
        *shaderProgram,
        gridSpacingValue, 
        offsetValue, 
        width, height, 
        playerPosition, 
        toplefttile, 
        scale,
        waterMax,
        sandMax,
        dirtMax,
        grassMax,
        stoneMax,
        snowMax,
        lastTime,
        frequency, amplitude, persistence, lacunarity, octaves);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // // Render the test texture (second shader)
    // updateUniformsTexture(*shaderProgramTexture, textureID, 100, 100);

    // Enable blending for the next shaders
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render the crosshair (third shader)
    updateUniforms2(*shaderProgram2, width, height, gridSpacingValue);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


    // Swap buffers

    SDL_GL_SwapWindow(ctx->window);

    // Update Info on front end

    // Player position x:y
    _js__kvdata("x", playerPosition[0]);
    _js__kvdata("y", playerPosition[1]);

    // Current tile x:y
    _js__kvdata("tilex", static_cast<int>(playerPosition[0] / defaultGSV));
    _js__kvdata("tiley", static_cast<int>(playerPosition[1] / defaultGSV));

    // Current scale from gridSpacingValue
    _js__kvdata("scale", gridSpacingValue);



    ctx->iteration++;
    
}
