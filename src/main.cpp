#include <emscripten.h>
#include <SDL2/SDL.h>
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

float deltaTime = 0;
int seed = 0;
unordered_map<int, bool> keys;
GLfloat cursorPos[2] = {0, 0};

GLuint shaderProgram;
GLuint shaderProgram2;
GLfloat gridSpacingValue = 256.0f;
GLfloat offsetValue[2] = {0.0f, 0.0f};
int width = 1024;
int height = 1024;
GLfloat playerPosition[2] = {0, 0};
GLfloat toplefttile[2] = {0.0f, 0.0f};
float waterMax;
float sandMax;
float dirtMax;
float grassMax;
float stoneMax;
float snowMax;
int lastTime = 0;
float frequency = 1.0;
float amplitude = 1.0;
float persistence = 0.5;
float lacunarity = 2.0;
float scale = 1;
int octaves = 6;

GLubyte pixelData[4];

float defaultGSV = gridSpacingValue;
GLfloat tempPlayerPosition[2] = {0, 0};

GLint centerX;
GLint centerY;
float moveSpeed = 5;
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

    seed = time(0);
    srand(seed);
    seed = rand() % 100000;

    printf("Seed: %d\n", seed);

    SDL_Window *mpWindow =
        SDL_CreateWindow("Shader Terrain",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            width, height,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    Uint32 mWindowID = SDL_GetWindowID(mpWindow);

    ctx.window = mpWindow;

    // Link vertex and fragment shader into shader program and use it
    loadGl(mpWindow);
    shaderProgram = loadGL1(shaderProgram);
    // shaderProgram2 = loadGL2(shaderProgram2);
    
    // --------------------------------
    centerX = width / 2;
    centerY = height / 2;

    // Set random position
    playerPosition[0] = static_cast<float>(playerPosition[0]);
    playerPosition[1] = static_cast<float>(playerPosition[1]);

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
    SDL_Quit();
    return EXIT_SUCCESS;
}

void animations(context *ctx)
{
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);

    // glUseProgram(shaderProgram);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // Read the pixel color at the center of the screen
    // glReadPixels(centerX, centerY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

    
    // glUseProgram(shaderProgram2);
    // GLfloat vertices[] = {
    //     // Positions        // Colors (R, G, B)
    //     0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // Top vertex (Red)
    //     -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // Bottom left vertex (Green)
    //     0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // Bottom right vertex (Blue)
    //     0.0f, -0.5f, 0.0f,  1.0f, 1.0f, 0.0f   // Additional vertex to satisfy vertex fetch requirement
    // };
    // GLuint VBO;
    // glGenBuffers(1, &VBO);
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // //Bind VBO before setting vertex attributes for safety
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // // Position attribute
    // glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    // glEnableVertexAttribArray(0);
    // // Color attribute
    // glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    // glEnableVertexAttribArray(1);

    // // glDrawArrays(GL_TRIANGLES, 0, 3); // Draw the triangle
    // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Draw the triangle

    // // Optionally disable vertex attrib array if not used elsewhere
    // glDisableVertexAttribArray(0);
    // glDisableVertexAttribArray(1);

    // Swap buffers
    SDL_GL_SwapWindow(ctx->window);
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
        moveSpeed = defaultMoveSpeed * 200.0f;
        keys[SPEED_MULTI] = false;
    }
    else if (keys[SPEED_DIV])
    {
        moveSpeed = defaultMoveSpeed;
        keys[SPEED_DIV] = false;
    }

    // if (gridSpacingValue > (width / 2)/2/2)
    // {
    //     gridSpacingValue = (width / 2)/2/2;
    // }
    // else if (gridSpacingValue < .0625)
    // {
    //     gridSpacingValue = .0625;
    // }

    // Update player position
    float _moveSpeed = moveSpeed * deltaTime;

    // // if pixeldata is blue, revert movement
    // if (pixelData[2] > pixelData[0] && pixelData[2] > pixelData[1])
    // {
    //     // playerPosition = tempPlayerPosition;
    //     playerPosition[0] = tempPlayerPosition[0];
    //     playerPosition[1] = tempPlayerPosition[1];
    // }
    // else
    // {
    //     // tempPlayerPosition = playerPosition;
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
    // }
    
    // set view offset
    offsetValue[0] = (fmod(playerPosition[0], defaultGSV) * gridSpacingValue) / defaultGSV;
    offsetValue[1] = (fmod(playerPosition[1], defaultGSV) * gridSpacingValue) / defaultGSV;

    // set bounds
    toplefttile[0] = static_cast<int>(playerPosition[0] / defaultGSV) - (width / gridSpacingValue / 2);
    toplefttile[1] = static_cast<int>(playerPosition[1] / defaultGSV) - (height / gridSpacingValue / 2);
}


void mainloop(void *arg)
{

    deltaTime = (SDL_GetTicks() - lastTime) / 1000.0f;
    lastTime = SDL_GetTicks();
    context *ctx = static_cast<context *>(arg);

    while (SDL_PollEvent(&ctx->event))
    {
        EventHandler(0, &ctx->event);
    }

    updateFrame();
    updateUniforms(
        shaderProgram, 
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



    // Draw
    animations(ctx);

    ctx->iteration++;
}
