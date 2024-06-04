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

float positions[100000][2]; // Adjust the size as needed
// vector<float[2]> entities;

int numPositions = 0;
int numEntities = 0;
unordered_map<int, vector<float>> entities;
unordered_map<int, int> entity_validated;
unordered_map<int, int[3]> entity_pos_rgb;
unordered_map<int, float[2]> entity_pos_screen;
unordered_map<int, bool> entity_visible;

float defaultGSV = 16.0f;
GLfloat gridSpacingValue = 2048.0f;
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
float frequency = -1.9243269;
float amplitude = 1.0;
float persistence = 0.5;
float lacunarity = 2.0;
float scale = 1;
int octaves = 12;
bool windowResized = false;
bool ready = false;

// test entity for frustrum cull testing
float _testEntity[2] = {0.0f, 20.0f};

GLubyte pixelData[4];

GLfloat tempPlayerPosition[2] = {0, 0};

SDL_Surface *image = nullptr;

// deltax
float deltaX = 0;
float deltaY = 0;

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

    // SDL_ShowCursor(SDL_DISABLE);

    ctx.window = mpWindow;

    // Link vertex and fragment shader into shader program and use it
    loadGl(mpWindow);
    
    // --------------------------------
    centerX = width / 2;
    centerY = height / 2;

    // Set random position
    // playerPosition[0] = static_cast<float>(playerPosition[0]);
    // playerPosition[1] = static_cast<float>(playerPosition[1]);
    // playerPosition[0] = rand() % 1000;
    // playerPosition[1] = rand() % 1000;

    // numPositions = 1000;
    // for (int i = 0; i < numPositions; i++)
    // {
    //     positions[i][0] = static_cast<float>(rand() % 100) + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    //     positions[i][1] = static_cast<float>(rand() % 100) + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    // }

    numEntities = 10000;
    for (int i = 0; i < numEntities; i++)
    {
        entities[i] = {
            static_cast<float>(rand() % 300) + static_cast <float> (rand()) / static_cast <float> (RAND_MAX), 
            static_cast<float>(rand() % 300) + static_cast <float> (rand()) / static_cast <float> (RAND_MAX)
        };
        entity_validated[i] = false;
        entity_pos_rgb[i][0] = -1;
        entity_pos_rgb[i][1] = -1;
        entity_pos_rgb[i][2] = -1;
    }

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

    // random walk each position
    // for (int i = 0; i < numPositions; i++)
    // {
    //     float time = SDL_GetTicks() / 1000.0f;
    //     positions[i][0] += sin(time + i) * 0.1f;
    //     positions[i][1] += cos(time + i) * 0.1f;
    // }
}


// Water RGB: (20.0 / 255.0, 24.0 / 255.0, 34.0 / 255.0)
// Sand RGB: (0.95, 0.87, 0.70)
// Dirt RGB: (164.0 / 255.0, 158.0 / 255.0, 130.0 / 255.0)
// Grass RGB: (48.0 / 255.0, 71.0 / 255.0, 40.0 / 255.0)
// Stone RGB: (144.0 / 255.0, 144.0 / 255.0, 144.0 / 255.0)
// Snow RGB: (1.0, 1.0, 1.0)

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


bool first_start = false;
bool one_time = false;
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
        // *shaderProgram,
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

    // // Render the crosshair (third shader)
    updateUniforms2(shaderProgramMap["ui_layer"], width, height, gridSpacingValue);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Render visible test entities
    // render each valid entity
    for(auto& entity : entities) {
        if (entity_validated[entity.first]==1 && entity_visible[entity.first]) {
            updateUniformsTexture(shaderProgramMap["texture"], 
                textureID, entity_pos_screen[entity.first][0], entity_pos_screen[entity.first][1], 0.1f * gridSpacingValue/1000);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }

    // Swap buffers
    SDL_GL_SwapWindow(ctx->window);

    
    for(auto& entity : entities) {

        if(entity_validated[entity.first] == 2) {
            // refresh entity position
            entity_validated[entity.first] = 0;
            entity_pos_rgb[entity.first][0] = -1;
            entity_pos_rgb[entity.first][1] = -1;
            entity_pos_rgb[entity.first][2] = -1;

            entities[entity.first] = {
                static_cast<float>(rand() % 300) + static_cast <float> (rand()) / static_cast <float> (RAND_MAX), 
                static_cast<float>(rand() % 300) + static_cast <float> (rand()) / static_cast <float> (RAND_MAX)
            };
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
            
            // check if entity has been validated
            if (entity_validated[entity.first] == 0) {
                // check if the entity is on the grass, if not remove it and skip
                unsigned char pixel[4];
                glReadPixels(entity_pos_screen[entity.first][0] * width / 2 + width / 2, entity_pos_screen[entity.first][1] * height / 2 + height / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

                // Water RGB: (20.0 / 255.0, 24.0 / 255.0, 34.0 / 255.0)
                // Sand RGB: (0.95, 0.87, 0.70)
                // Dirt RGB: (164.0 / 255.0, 158.0 / 255.0, 130.0 / 255.0)
                // Grass RGB: (48.0 / 255.0, 71.0 / 255.0, 40.0 / 255.0)
                // Stone RGB: (144.0 / 255.0, 144.0 / 255.0, 144.0 / 255.0)
                // Snow RGB: (1.0, 1.0, 1.0)

                if (pixel[0] == 48 && pixel[1] == 71 && pixel[2] == 40) {
                    entity_validated[entity.first] = 1;
                } 
                else {
                    entity_validated[entity.first] = 2;
                }
            }

            // check if entity is visible
            if (entity_validated[entity.first] == 1) {
                entity_visible[entity.first] = true;
            }
        }
        else if(entity_visible[entity.first]) {
            entity_visible[entity.first] = false;
        }
    }

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
