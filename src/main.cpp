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

using namespace std;

extern entt::registry registry;

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

// General variables
int width = 1024;
int height = 1024;
float deltaTime = 0;
int seed = 0;
unordered_map<int, bool> keys;
GLfloat cursorPos[2] = {0, 0};
GLfloat playerPosition[2] = {0, 0};
GLfloat gridSpacingValue = 2048.0f;
GLfloat offsetValue[2] = {0.0f, 0.0f};
GLfloat toplefttile[2] = {0.0f, 0.0f};
int tileMinMax[4] = {0, 0, 0, 0};
float waterMax;
float sandMax;
float dirtMax;
float grassMax;
float stoneMax;
float snowMax;
float frequency = -1.9243269;
float amplitude = 1.0;
float persistence = 0.5;
float lacunarity = 2.0;
float scale = 1;
int octaves = 12;
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
unordered_map<int, vector<float>> entities;
unordered_map<int, int> entity_validated;
unordered_map<int, int[3]> entity_pos_rgb;
unordered_map<int, float[2]> entity_pos_screen;
unordered_map<int, bool> entity_visible;
float defaultGSV = 16.0f;
SDL_Surface *image = nullptr;
bool first_start = false;
bool one_time = false;
float _moveSpeed = 0;

struct context
{
    SDL_Event event;
    int iteration;
    SDL_Window *window;
};

struct Position {
    float x;
    float y;
};

context ctx;

void mainloop(void *arg);
void EventHandler(int, SDL_Event *);
void animations(context *ctx);
int simple_tile_color(float);
float getTerrainAtPoint(float, float, float, float);

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
    
    centerX = width / 2;
    centerY = height / 2;

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

    numEntities = 10000;
    for (int i = numEntities; i > 0; i--)
    {
        float x = static_cast<float>(rand() % 300) + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float y = static_cast<float>(rand() % 300) + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

        entities[i] = {x, y};
        entity_validated[i] = false;
        entity_pos_rgb[i][0] = -1;
        entity_pos_rgb[i][1] = -1;
        entity_pos_rgb[i][2] = -1;
        
    }

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

    // Update player position
    _moveSpeed = moveSpeed * deltaTime;

    deltaX = (keys[SDLK_d] - keys[SDLK_a]);
    deltaY = (keys[SDLK_s] - keys[SDLK_w]);
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

int simple_tile_color(float n) {
    if (n < waterMax) {
        // Water
        return 0;
    } else if (n < sandMax) {
        // Sand 
        return 1;
    } else if (n < dirtMax) {
        // Dirt
        return 2;
    } else if (n < grassMax) {
        // Grass
        return 3;
    } else if (n < stoneMax) {
        // Stone
        return 4;
    } else {
        // Snow
        return 5;
    }
    return -1;
}

float getTerrainAtPoint(float px, float py, float frequency, float amplitude) {
    return simple_tile_color(smootherNoise({px * frequency, py * frequency}) * amplitude);
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
        textureID = loadGLTexture(shaderProgramMap["texture"]);
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render the crosshair (third shader)
    updateUniforms2(shaderProgramMap["ui_layer"], width, height, gridSpacingValue);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


    for(auto& entity : entities) {
        if(entity_validated[entity.first] == 2) {
            // refresh entity position
            // entity_validated[entity.first] = 0;
            // entity_pos_rgb[entity.first][0] = -1;
            // entity_pos_rgb[entity.first][1] = -1;
            // entity_pos_rgb[entity.first][2] = -1;

            // entities[entity.first] = {
            //     static_cast<float>(rand() % 300) + static_cast <float> (rand()) / static_cast <float> (RAND_MAX), 
            //     static_cast<float>(rand() % 300) + static_cast <float> (rand()) / static_cast <float> (RAND_MAX)
            // };

            continue;
        }
        
        float entity_player_diffX = (playerPosition[0] - entity.second[0]);
        float entity_player_diffY = (playerPosition[1] - entity.second[1]);


        float distanceX = abs(entity_player_diffX * gridSpacingValue / defaultGSV);
        float distanceY = abs(entity_player_diffY * gridSpacingValue / defaultGSV);

        if (distanceX < width / 2 && distanceY < height / 2) {

            float posX = (playerPosition[0] - entity.second[0]) * gridSpacingValue + centerX;
            float posY = (playerPosition[1] - entity.second[1]) * gridSpacingValue + centerY;

            entity_pos_screen[entity.first][0] = (2 * posX / width - 1)/defaultGSV;
            entity_pos_screen[entity.first][1] = (2 * posY / height - 1)/defaultGSV;

            entity_pos_screen[entity.first][0] -= _moveSpeed/width;
            entity_pos_screen[entity.first][1] += _moveSpeed/height;

            // check if entity has been validated
            if (entity_validated[entity.first] == 0) {
                // check if the entity is on the grass, if not remove it and skip
                unsigned char pixel[4];
                glReadPixels(entity_pos_screen[entity.first][0] * width / 2 + width / 2, entity_pos_screen[entity.first][1] * height / 2 + height / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

                if (pixel[0] == 48 && pixel[1] == 71 && pixel[2] == 40) {
                    entity_validated[entity.first] = 1;
                } 
                else {
                    entity_validated[entity.first] = 2;
                }
            }

            // // check if entity is visible
            if (entity_validated[entity.first] == 1) {
                entity_visible[entity.first] = true;
            }
        }
        else if(entity_visible[entity.first]) {
            entity_visible[entity.first] = false;
        }
    }

    // Render visible test entities
    for(auto& entity : entities) {
        if (entity_validated[entity.first] == 1 && entity_visible[entity.first]) {
            updateUniformsTexture(shaderProgramMap["texture"], 
                textureID, entity_pos_screen[entity.first][0], entity_pos_screen[entity.first][1], 0.1f * gridSpacingValue/1000);
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

    float modifiedPosX = ((2 * playerPosition[0] / width - 1) / defaultGSV) - (_moveSpeed / width);
    float modifiedPosY = ((2 * playerPosition[1] / height - 1) / defaultGSV) + (_moveSpeed / height);
    
    _js__kvdata("player-terrain-noise", getTerrainAtPoint(modifiedPosX, modifiedPosY, frequency, amplitude));

    ctx->iteration++;
}
