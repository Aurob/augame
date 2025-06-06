#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL2/SDL_ttf.h>

#include "../include/shaders.hpp"
#include "events.hpp"
#include "JSUtils.hpp"

extern float seed;
extern float gridSpacingValue;
extern float offsetValue[2];
extern float toplefttile[2];
extern float generationSize[2];
extern entt::registry registry;

// Character structure
struct Character {
    GLuint textureID;
    int minx, miny, maxx, maxy;
    int width;
    int height;
    int advance;
};

std::map<char, Character> characters;
GLuint textShaderProgram;

SDL_Window* loadSDL() {
    // Initialize SDL and SDL_Image
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return nullptr;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", SDL_GetError());
        return nullptr;
    }
    
    TTF_Init();

    SDL_Window *mpWindow = SDL_CreateWindow(
        "Untitled",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );

    if (!mpWindow) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return nullptr;
    }


    return mpWindow;
}
bool _log_uniform = true;

inline float fract(float x) {
    return x - std::floor(x);
}

// Helper function to hash a 3D point
// Renamed to hashPoint to avoid ambiguity with std::hash
float* hashPoint(float p[3]) {
    static float result[3];
    float dot1 = p[0] * 127.1f + p[1] * 311.7f + p[2] * 74.7f;
    float dot2 = p[0] * 269.5f + p[1] * 183.3f + p[2] * 246.1f;
    float dot3 = p[0] * 113.5f + p[1] * 271.9f + p[2] * 101.5f;
    
    result[0] = -1.0f + 2.0f * fract(sin(dot1 + seed) * 43758.5453f);
    result[1] = -1.0f + 2.0f * fract(sin(dot2 + seed) * 43758.5453f);
    result[2] = -1.0f + 2.0f * fract(sin(dot3 + seed) * 43758.5453f);
    
    return result;
}

// Helper function for smooth noise, matching the shader implementation
float smoothNoise(float x, float y) {
    // More complex noise using multiple trigonometric functions with phase shifts
    const float xFreq1 = 0.1f;  // Primary frequency for x component
    const float yFreq1 = 0.1f;  // Primary frequency for y component
    const float xFreq2 = 0.05f; // Secondary frequency for x component
    const float yFreq2 = 0.07f; // Secondary frequency for y component
    
    // Add phase shifts based on seed for more variation
    float phaseX = sin(seed * 0.1f) * 3.14f;
    float phaseY = cos(seed * 0.1f) * 3.14f;
    
    // Combine multiple sine and cosine waves with different frequencies and phases
    float noise1 = 0.5f * sin(x * xFreq1 + phaseX) + 0.5f * cos(y * yFreq1 + phaseY);
    float noise2 = 0.3f * sin(x * xFreq2 + y * 0.08f) + 0.3f * cos(y * yFreq2 - x * 0.06f);
    float noise3 = 0.2f * sin((x + y) * 0.12f) * cos((x - y) * 0.09f);
    
    // Combine the noise components with some non-linear operations
    float combinedNoise = noise1 + noise2 * (1.0f + 0.2f * sin(x * 0.3f)) + noise3;
    
    // Normalize to 0.0-1.0 range
    return (combinedNoise + 1.5f) * 0.33f;
}

float frequency = 9.5f;
float amplitude = 0.70f;

// Helper function to calculate n value for terrain, matching the shader implementation
float calculate_n(float x, float y) {
    float n = 0.0f;
    float layerFrequency = frequency;
    float layerAmplitude = amplitude;
    const int numLayers = 10; // Adjust as needed for desired complexity

    // Add a slight rotation to each octave for more natural patterns
    float rotationAngle = 0.15f;
    float sinRot = sin(rotationAngle);
    float cosRot = cos(rotationAngle);

    for (int i = 0; i < numLayers; i++) {
        // Apply slight rotation to coordinates for each layer
        float rotatedCoord_x = x * layerFrequency;
        float rotatedCoord_y = y * layerFrequency;
        
        if (i > 0) {
            float rotAmount = float(i) * rotationAngle;
            float s = sin(rotAmount);
            float c = cos(rotAmount);
            float temp_x = rotatedCoord_x;
            rotatedCoord_x = rotatedCoord_x * c - rotatedCoord_y * s;
            rotatedCoord_y = temp_x * s + rotatedCoord_y * c;
        }
        
        // Get noise value and apply domain warping for more complex patterns
        float noiseVal = smoothNoise(rotatedCoord_x, rotatedCoord_y);
        
        // Apply domain warping for higher octaves
        if (i > 3) {
            float warp_x = sin(rotatedCoord_y * 0.5f + seed * 0.1f) * 0.15f;
            float warp_y = cos(rotatedCoord_x * 0.5f + seed * 0.2f) * 0.15f;
            noiseVal = smoothNoise(rotatedCoord_x + warp_x, rotatedCoord_y + warp_y);
        }
        
        n += layerAmplitude * noiseVal;
        
        // Adjust frequency and amplitude for next layer
        layerFrequency *= 1.5f;
        layerAmplitude *= 0.55f;
    }

    // Apply a subtle ridge effect to create more interesting terrain features
    n = abs(n * 2.0f - 1.0f);
    n = 1.0f - n;
    n = n * n;
    
    // Ensure n stays within reasonable bounds (0.0 to 1.0)
    n = (n < 0.0f) ? 0.0f : ((n > 1.0f) ? 1.0f : n);
    
    return n;
}

// Log uniform values and update color in JS
void logUniformValues(float _width, float _height, float gridSpacingValue, 
                     float offsetValue[2], float toplefttile[2], float generationSize[2]) {
    // Calculate the same values as in the shader
    // Example coordinates at the center of the screen
    float coord_x = _width / 2.0f;
    float coord_y = _height / 2.0f;
    
    // Invert y-coordinate as in the shader
    coord_y = _height - coord_y;
    
    // Calculate generationOffset
    float generationOffset_x = generationSize[0] / 2.0f;
    float generationOffset_y = generationSize[1] / 2.0f;
    
    // Adjust coordinates with grid spacing, toplefttile, offset, and generationOffset
    float adjustedCoord_x = (coord_x / gridSpacingValue) + toplefttile[0] + (offsetValue[0] / gridSpacingValue) + generationOffset_x;
    float adjustedCoord_y = (coord_y / gridSpacingValue) + toplefttile[1] + (offsetValue[1] / gridSpacingValue) + generationOffset_y;
    
    // Calculate n using helper function
    float n = calculate_n(adjustedCoord_x, adjustedCoord_y);
    // Determine color based on n value, similar to simple_tile_color in shader
    float r, g, b;
    if (n < 0.1f) {
        // Water - slightly lighter ocean
        float depth = 0.1f - n;  // Deeper water is darker
        float depthFactor = depth / 0.1f;  // Normalize to 0-1 range
        r = 0.12f * (1.0f - depthFactor) + 0.08f * depthFactor;
        g = 0.16f * (1.0f - depthFactor) + 0.12f * depthFactor;
        b = 0.24f * (1.0f - depthFactor) + 0.20f * depthFactor;
    } else if (n < 0.3f) {
        // Sand - more pale with variation
        float sandFactor = (n - 0.1f) / 0.2f;  // Normalize to 0-1 range
        r = 0.92f * (1.0f - sandFactor) + 0.85f * sandFactor;
        g = 0.88f * (1.0f - sandFactor) + 0.82f * sandFactor;
        b = 0.78f * (1.0f - sandFactor) + 0.70f * sandFactor;
    } else if (n < 0.6f) {
        // Grass with variation
        float grassFactor = (n - 0.3f) / 0.3f;  // Normalize to 0-1 range
        r = 0.2f * (1.0f - grassFactor) + 0.15f * grassFactor;
        g = 0.6f * (1.0f - grassFactor) + 0.5f * grassFactor;
        b = 0.3f * (1.0f - grassFactor) + 0.2f * grassFactor;
    } else if (n < 0.8f) {
        // Greyish brownish green transition zone
        float transitionFactor = (n - 0.6f) / 0.2f;  // Normalize to 0-1 range
        r = 0.4f * (1.0f - transitionFactor) + 0.35f * transitionFactor;
        g = 0.4f * (1.0f - transitionFactor) + 0.35f * transitionFactor;
        b = 0.3f * (1.0f - transitionFactor) + 0.25f * transitionFactor;
    } else {
        // Stone/mountain
        float stoneFactor = (n - 0.8f) / 0.2f;  // Normalize to 0-1 range
        r = 0.5f * (1.0f - stoneFactor) + 0.4f * stoneFactor;
        g = 0.5f * (1.0f - stoneFactor) + 0.4f * stoneFactor;
        b = 0.5f * (1.0f - stoneFactor) + 0.45f * stoneFactor;
    }
    // Update color in JS
    _js__update_color(r*256, g*256, b*256);
}

void updateUniforms(GLuint &shaderProgram,
                    float gridSpacingValue,
                    float offsetValue[2],
                    float _width, float _height,
                    float toplefttile[2],
                    float generationSize[2])   
{

    glUseProgram(shaderProgram);
    // grid_spacing uniform
    GLint gridSpacingLocation = glGetUniformLocation(shaderProgram, "grid_spacing");
    glUniform1f(gridSpacingLocation, gridSpacingValue);

    // offset uniform
    GLint offsetLocation = glGetUniformLocation(shaderProgram, "offset");
    glUniform2fv(offsetLocation, 1, offsetValue);

    // resolution uniform
    GLint resolutionLocation = glGetUniformLocation(shaderProgram, "resolution");
    glUniform2f(resolutionLocation, _width, _height);

    // bounds uniform
    GLint boundsLocation = glGetUniformLocation(shaderProgram, "toplefttile");
    glUniform2fv(boundsLocation, 1, toplefttile);

    // cursorPos uniform
    // auto &cursor = registry.get<Cursor>(_player);
    // GLint cursorPosLocation = glGetUniformLocation(shaderProgram, "cursorPos");
    // glUniform2f(cursorPosLocation, cursor.position.x, cursor.position.y);

    // time
    float timeValue = SDL_GetTicks() / 1000000.0f;
    GLint timeLocation = glGetUniformLocation(shaderProgram, "time");
    glUniform1f(timeLocation, timeValue);

    // generationSize
    GLint generationSizeLocation = glGetUniformLocation(shaderProgram, "generationSize");
    glUniform2fv(generationSizeLocation, 1, generationSize);

    // seed
    GLint seedLocation = glGetUniformLocation(shaderProgram, "seed");
    glUniform1f(seedLocation, seed);
    
    if (_log_uniform) {
        logUniformValues(_width, _height, gridSpacingValue, offsetValue, toplefttile, generationSize);
    }
}

void updateUIShader(GLuint &shaderProgram, float _width, float _height, float gridSpacingValue, float toplefttile[2])
{
    glUseProgram(shaderProgram);

    // resolution uniform
    GLint resolutionLocation = glGetUniformLocation(shaderProgram, "resolution");
    glUniform2f(resolutionLocation, _width, _height);

    // grid_spacing uniform
    GLint gridSpacingLocation = glGetUniformLocation(shaderProgram, "grid_spacing");
    glUniform1f(gridSpacingLocation, gridSpacingValue);
    // cursorPos uniform
    auto &cursor = registry.get<Cursor>(_player);
    GLint cursorPosLocation = glGetUniformLocation(shaderProgram, "cursorPos");
    glUniform2f(cursorPosLocation, cursor.position.x, cursor.position.y);

    // toplefttile uniform
    GLint toplefttileLocation = glGetUniformLocation(shaderProgram, "toplefttile");
    glUniform2fv(toplefttileLocation, 1, toplefttile);
}

void updateUniformsTexture(GLuint &shaderProgram, GLuint textureID, float x, float y, 
                            float scalex, float scaley, 
                            float startX = 0.0f, float startY = 0.0f, 
                            float sizeX = 1.0f, float sizeY = 1.0f, float angle = 0.0f) {
    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLint instancePositionLocation = glGetUniformLocation(shaderProgram, "instancePosition");
    glUniform2f(instancePositionLocation, x, y);

    GLint instanceScaleLocation = glGetUniformLocation(shaderProgram, "instanceScale");
    glUniform2f(instanceScaleLocation, scalex, scaley);

    GLint cropStartLocation = glGetUniformLocation(shaderProgram, "cropStart");
    glUniform2f(cropStartLocation, startX, startY);

    GLint cropSizeLocation = glGetUniformLocation(shaderProgram, "cropSize");
    glUniform2f(cropSizeLocation, sizeX, sizeY);
    
    GLint angleLocation = glGetUniformLocation(shaderProgram, "angle");
    glUniform1f(angleLocation, angle);
}

void updateUniformsDebug(GLuint &shaderProgram, 
float r, float g, float b, float a, float x, float y, 
float scalex, float scaley, float angle) {
    glUseProgram(shaderProgram);

    GLint colorLocation = glGetUniformLocation(shaderProgram, "uColor");
    glUniform4f(colorLocation, r, g, b, a);

    GLint instancePositionLocation = glGetUniformLocation(shaderProgram, "instancePosition");
    glUniform2f(instancePositionLocation, x, y);

    GLint debugScaleLocation = glGetUniformLocation(shaderProgram, "entityScale");
    glUniform2f(debugScaleLocation, scalex, scaley);

    GLint angleLocation = glGetUniformLocation(shaderProgram, "angle");
    glUniform1f(angleLocation, angle);
}


void updateUniformFont(GLuint &shaderProgram, 
float r, float g, float b, float a, float x, float y, 
float scalex, float scaley) {
    glUseProgram(shaderProgram);

    GLint colorLocation = glGetUniformLocation(shaderProgram, "uTextColor");
    glUniform4f(colorLocation, r, g, b, a);

    GLint instancePositionLocation = glGetUniformLocation(shaderProgram, "charPosition");
    glUniform2f(instancePositionLocation, x, y);

    GLint debugScaleLocation = glGetUniformLocation(shaderProgram, "charScale");
    glUniform2f(debugScaleLocation, scalex, scaley);
}


void createShader(GLuint &shaderProgram, std::string program_name) {

    // Create and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &shaderGLSLMap[program_name][0], NULL);
    glCompileShader(vertexShader);
    
    // Check for vertex shader compilation errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {   
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // Create and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &shaderGLSLMap[program_name][1], NULL);
    glCompileShader(fragmentShader);

    // Check for fragment shader compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    shaderProgram = glCreateProgram();

    // printf("Attaching shaders...\n");
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // printf("Linking shaders...\n");

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GLfloat vertices[] =
        {
            -1.0f, 1.0f, 0.0f,  // Top Left
            1.0f, 1.0f, 0.0f,   // Top Right
            -1.0f, -1.0f, 0.0f, // Bottom Left
            1.0f, -1.0f, 0.0f   // Bottom Right
        };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glUseProgram(shaderProgram);

    // Specify the layout of the shader vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Don't forget to bind the VAO before you draw
    GLuint ebo;
    GLuint indices[] = {
        0, 1, 2, // First Triangle
        2, 1, 3  // Second Triangle
    };

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // -------------------------------------------------------------
    // Create texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Allocate texture storage (but don't upload data yet)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // Attach texture to framebuffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

    // Check framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Framebuffer not complete!\n");

    // Render to texture (rtt)
    glViewport(0, 0, width, height); // Match texture size
    // Add your render code here: this will render to texture instead of screen
    // Remember to clear the framebuffer using glClear if necessary

    // Bind the default framebuffer to render to screen again
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height); // Match window size

    // In your render loop, use the generated texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // return shaderProgram;
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    shaderProgramMap[program_name] = shaderProgram;
}

GLuint compileShader(const char* shaderSource, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    return shader;
}

GLuint createProgram(const char* vertexShaderSrc, const char* fragmentShaderSrc) {
    GLuint vertexShader = compileShader(vertexShaderSrc, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentShaderSrc, GL_FRAGMENT_SHADER);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    return program;
}

void loadImageAndCreateTexture(const char* imagePath, GLuint &textureID) {
    SDL_Surface* image = IMG_Load(imagePath);
    if (!image) {
        printf("IMG_Load: %s\n", IMG_GetError());
        return;
    }
    // printf("Image loaded successfully\n");

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Determine the format
    GLenum format;
    if (image->format->BytesPerPixel == 3) { // RGB 24bit
        format = GL_RGB;
    } else if (image->format->BytesPerPixel == 4) { // RGBA 32bit
        format = GL_RGBA;
    } else if (image->format->BytesPerPixel == 1) { // Grayscale 8bit
        format = GL_LUMINANCE;
    } else if (image->format->BytesPerPixel == 2) { // Grayscale 16bit
        format = GL_LUMINANCE_ALPHA;
    } else {
        printf("Unknown image format\n");
        SDL_FreeSurface(image);
        return;
    }
    // Load the texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels);

    SDL_FreeSurface(image);
}

GLuint loadGLTexture(GLuint &shaderProgram, std::string textureSrc, int &width, int &height) {
    // Load shaders and create a program
    shaderProgram = createProgram(shaderGLSLMap["texture"][0], shaderGLSLMap["texture"][1]);
    glUseProgram(shaderProgram);

    // Load an image and create a texture from it
    GLuint textureID;
    SDL_Surface* image = IMG_Load(textureSrc.c_str());
    if (image) {
        width = image->w;
        height = image->h;
        SDL_FreeSurface(image);
    }
    loadImageAndCreateTexture(textureSrc.c_str(), textureID);

    // Define vertices for the texture
    // Each vertex has a position (x, y) and texture coordinates (s, t)
    GLfloat vertices[] = {
        // Positions       // TexCoords
        -1.0f, -1.0f,     0.0f, 1.0f,
         1.0f, -1.0f,     1.0f, 1.0f,
         1.0f,  1.0f,     1.0f, 0.0f,
        -1.0f,  1.0f,     0.0f, 0.0f
    };

    // Define indices for the vertices
    GLuint indices[] = { 0, 1, 2, 2, 3, 0 };

    // Generate buffers for vertices and elements
    GLuint VBO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind the vertex buffer and load the vertices into it
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Bind the element buffer and load the indices into it
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Get the location of the 'position' attribute in the shader program
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(posAttrib);
    // Get the location of the 'texCoord' attribute in the shader program
    GLint texAttrib = glGetAttribLocation(shaderProgram, "texCoord");
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(texAttrib);

    return textureID;
}

void loadTextures() {
    createShader(shaderProgramMap["terrain"], "terrain");
    createShader(shaderProgramMap["ui_layer"], "ui_layer");
    createShader(shaderProgramMap["debug_entity"], "debug_entity");
    createShader(shaderProgramMap["texture"], "texture");
    // createShader(shaderProgramMap["font"], "font");

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
}

void renderText(const std::string& text, float x, float y, float scale, float r, float g, float b, float a) {
    // Use the font shader program
    glUseProgram(shaderProgramMap["font"]);

    // // Render the entire text as one texture instead of individual glyphs
    SDL_Color color = {static_cast<Uint8>(r * 255), static_cast<Uint8>(g * 255), static_cast<Uint8>(b * 255), static_cast<Uint8>(a * 255)};

    // Load font for rendering
    TTF_Font* font = TTF_OpenFont("resources/fonts/42dotSans-Regular.ttf", 18);
    if (!font) {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        return;
    }

    // // Render text to surface
    // We'll use a temporary width for wrapping, as before
    int wrapWidth = width / 1.3;
    SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), color, wrapWidth);
    if (!surface) {
        printf("Failed to render text: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        return;
    }
    
    // Create texture from surface
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Prepare the surface for OpenGL
    SDL_Surface* rgba_surface = SDL_CreateRGBSurface(
        0, surface->w, surface->h, 32,
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
    );
    
    SDL_BlitSurface(surface, NULL, rgba_surface, NULL);
    
    // Upload to texture
    glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_RGBA, 
        rgba_surface->w, 
        rgba_surface->h, 
        0, 
        GL_RGBA, 
        GL_UNSIGNED_BYTE, 
        rgba_surface->pixels
    );
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Calculate dimensions for rendering
    float width = rgba_surface->w * scale;
    float height = rgba_surface->h * scale;

    // Offset the text x position by width/1.3
    float x_offset = x - width / 1.3f;

    // Update uniforms for rendering
    updateUniformFont(shaderProgramMap["font"], r, g, b, a, x_offset, y, width, height);
    
    // Bind texture and draw
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLint texLoc = glGetUniformLocation(shaderProgramMap["font"], "uTexture");
    glUniform1i(texLoc, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    SDL_FreeSurface(rgba_surface);
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);

}

void loadFont() {
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        // Handle error
    }
    
    // Create and store the font shader program
    GLuint textShaderProgram = createProgram(shaderGLSLMap["font"][0], shaderGLSLMap["font"][1]);
    shaderProgramMap["font"] = textShaderProgram;
}

void renderAll() {
    // Get player entity from Player view
    auto playerView = registry.view<Player>();
    auto _player = playerView.front();

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
        bool lhsHasPrio = registry.all_of<RenderPriority>(lhs);
        bool rhsHasPrio = registry.all_of<RenderPriority>(rhs);
        
        // If one has priority and the other doesn't, prioritize the one with priority
        if (lhsHasPrio && !rhsHasPrio)
            return false; // lhs has higher priority
        if (!lhsHasPrio && rhsHasPrio)
            return true;  // rhs has higher priority
            
        // If both have priority, compare their priority values
        if (lhsHasPrio && rhsHasPrio) {
            const auto& lhsPrio = registry.get<RenderPriority>(lhs);
            const auto& rhsPrio = registry.get<RenderPriority>(rhs);
            
            if (lhsPrio.priority != rhsPrio.priority)
                return lhsPrio.priority < rhsPrio.priority;
        }
        
        // If neither has priority or they have the same priority, sort by position
        const auto& lhsPos = registry.get<Position>(lhs);
        const auto& rhsPos = registry.get<Position>(rhs);
        const auto& lhsShape = registry.get<Shape>(lhs);
        const auto& rhsShape = registry.get<Shape>(rhs);

        // Compare y + z + shape.z
        float lhsYZS = lhsPos.y;// + lhsPos.z + lhsShape.size.z;
        float rhsYZS = rhsPos.y;// + rhsPos.z + rhsShape.size.z;
        return lhsYZS < rhsYZS;
    });

    // Render visible entities
    auto visible_entities = registry.view<Visible, InView>();
    for(auto& entity : visible_entities) {
        auto position = registry.get<Position>(entity);
        auto shape = registry.get<Shape>(entity);
        
        bool isDebug = registry.all_of<Debug>(entity);
        bool is = registry.all_of<Teleport>(entity);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if(registry.all_of<Terrain>(entity)) {
            // Update uniforms for terrain shader
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
        else if (registry.all_of<Texture>(entity)) {
            const auto& texture = registry.get<Texture>(entity);

            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap[texture.name],
                position.sx + playerShape.scaled_size.x,
                position.sy + playerShape.scaled_size.y, // + shape.scaled_size.z,
                shape.scaled_size.x, 
                shape.scaled_size.y, // + shape.scaled_size.z,
                texture.x, texture.y, texture.w, texture.h
            );
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        else if (registry.all_of<TextureGroupPart>(entity)) {
            const auto& textureGroupPart = registry.get<TextureGroupPart>(entity);
            auto groupName = textureGroupPart.groupName;
            auto partName = textureGroupPart.partName;

            auto rootTexture = textureIDMap[groupName];
            const auto& texture = textureGroupMap[groupName].at(partName);

            auto IdName = registry.get<Id>(entity).name;
            if (textureGroupPart.tilex > 0 && textureGroupPart.tiley > 0) {
                int divisorX = textureGroupPart.tilex;
                int divisorY = textureGroupPart.tiley;
                auto ssizex = shape.scaled_size.x / divisorX;
                auto ssizey = shape.scaled_size.y / divisorY;
                auto posX = position.sx + playerShape.scaled_size.x;
                auto posY = position.sy + playerShape.scaled_size.y; // + position.sz + playerShape.scaled_size.z;

                // Increase size by 1%
                auto increasedSsizex = ssizex;// * 1.01f;
                auto increasedSsizey = ssizey;// * 1.01f;

                for (int i = 0; i < divisorX; ++i) {
                    for (int j = 0; j < divisorY; ++j) {
                        updateUniformsTexture(shaderProgramMap["texture"], 
                            rootTexture,
                            (posX + i * ssizex*2) - shape.scaled_size.x + ssizex - (increasedSsizex - ssizex) / 2,
                            (posY + j * ssizey*2),
                            increasedSsizex, increasedSsizey,
                            texture.x, texture.y, texture.w, texture.h
                        );
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    }
                }
            } 
            else {
                updateUniformsTexture(shaderProgramMap["texture"], 
                    rootTexture,
                    position.sx + playerShape.scaled_size.x,
                    position.sy + playerShape.scaled_size.y, // + position.sz + playerShape.scaled_size.z,
                    shape.scaled_size.x,
                    shape.scaled_size.y,
                    texture.x, texture.y, texture.w, texture.h
                );
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        } 
        else if (registry.all_of<Textures>(entity)) {
            const auto& textures = registry.get<Textures>(entity);
            const auto& current_texture = textures.textures[textures.current];


            if (registry.all_of<RenderDebug>(entity)) {
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    1.0f, 1.0f, 1.0f, 0.2f,
                    position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y,
                    shape.scaled_size.x, shape.scaled_size.y, 0.0f);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
            float angle = 0.0f;
            if(registry.all_of<Rotation>(entity)) {
                angle = registry.get<Rotation>(entity).angle;
            }

            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap[current_texture.name],
                position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y,
                shape.scaled_size.x * current_texture.scalex, shape.scaled_size.y * current_texture.scaley,
                current_texture.x, current_texture.y, current_texture.w, current_texture.h, angle);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        } else if (registry.all_of<TextureAlts, Player>(entity)) {
            const auto& textureAlts = registry.get<TextureAlts>(entity);
            const auto& currentTextures = textureAlts.alts.at(textureAlts.current);
            const auto& current_texture = currentTextures.textures[currentTextures.current];

            // Check if the entity has RenderDebug
            if (registry.all_of<Color>(entity)) {
                // Render a small transparent square to indicate bounding box
                auto color = registry.get<Color>(entity);
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    color.r, color.g, color.b, color.a, // Set color to Color
                    position.sx + playerShape.scaled_size.x, 
                    position.sy + playerShape.scaled_size.y,
                    shape.scaled_size.x, shape.scaled_size.y, 0.0f);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap[current_texture.name],
                position.sx + playerShape.scaled_size.x,
                position.sy + playerShape.scaled_size.y, // + position.sz + playerShape.scaled_size.z*2,
                shape.scaled_size.x * current_texture.scalex, 
                shape.scaled_size.y * current_texture.scaley,
                current_texture.x, current_texture.y, 
                current_texture.w, current_texture.h);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        }
        else if(registry.all_of<Color>(entity) && !registry.all_of<Text>(entity)) {
            auto color = registry.get<Color>(entity);

            float angle = 0.0f;
            if(registry.all_of<Rotation>(entity)) {
                angle = registry.get<Rotation>(entity).angle;
            }

            float r = color.r;
            float g = color.g; 
            float b = color.b;

            if(registry.all_of<Hovered>(entity)) {
                r = 0.0f;
                g = 0.0f;
                b = 1.0f;
            }
            if(registry.all_of<Interacted>(entity)) {
                r = 0.0f;
                g = 1.0f;
                b = 0.0f;
            }

            updateUniformsDebug(shaderProgramMap["debug_entity"],
                r, g, b, color.a,
                position.sx + playerShape.scaled_size.x, 
                position.sy + playerShape.scaled_size.y,
                shape.scaled_size.x, shape.scaled_size.y, 
                angle);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }

    auto textView = registry.view<Text, Position, Shape, Visible>();
    for (auto e : textView) {
        Text text = textView.get<Text>(e);
        Position pos = textView.get<Position>(e);
        Shape shape = textView.get<Shape>(e);
        // Determine text visibility based on hide flag
        
        float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
        if (registry.all_of<Color>(e)) {
            auto color = registry.get<Color>(e);
            r = color.r;
            g = color.g;
            b = color.b;
            a = color.a;
        }
        a = text.hide ? 0.0f : a;
        renderText(text.text, 
            pos.sx + playerShape.scaled_size.x + shape.scaled_size.x*2,
            pos.sy + playerShape.scaled_size.y, 
            (shape.scaled_size.x+shape.scaled_size.y)/200, 
        r, g, b, a);
    }
}

SDL_GLContext loadGl(SDL_Window *mpWindow)
{
    // Create OpenGLES 2 context on SDL window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GLContext glc = SDL_GL_CreateContext(mpWindow);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    return glc;
}
