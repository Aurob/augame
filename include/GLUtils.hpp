#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL2/SDL_ttf.h>

#include "../include/shaders.hpp"
#include "events.hpp"
#include "JSUtils.hpp"

extern float generationSize[2];
extern entt::registry registry;
extern GameState gameState;
extern MetaData metaData;

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
        gameState.width, gameState.height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );

    if (!mpWindow) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return nullptr;
    }


    return mpWindow;
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
    
    result[0] = -1.0f + 2.0f * fract(sin(dot1 + gameState.seed) * 43758.5453f);
    result[1] = -1.0f + 2.0f * fract(sin(dot2 + gameState.seed) * 43758.5453f);
    result[2] = -1.0f + 2.0f * fract(sin(dot3 + gameState.seed) * 43758.5453f);
    
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
    float phaseX = sin(gameState.seed * 0.1f) * 3.14f;
    float phaseY = cos(gameState.seed * 0.1f) * 3.14f;
    
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
            float warp_x = sin(rotatedCoord_y * 0.5f + gameState.seed * 0.1f) * 0.15f;
            float warp_y = cos(rotatedCoord_x * 0.5f + gameState.seed * 0.2f) * 0.15f;
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
                    float generationSize[2],
                    float rgb[3] = nullptr)   // Optional rgb parameter, default nullptr
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
    auto &cursor = registry.get<Cursor>(_player);
    GLint cursorPosLocation = glGetUniformLocation(shaderProgram, "cursorPos");
    glUniform2f(cursorPosLocation, cursor.position.x, cursor.position.y);

    // time
    float timeValue = SDL_GetTicks() / 1000000.0f;
    GLint timeLocation = glGetUniformLocation(shaderProgram, "time");
    glUniform1f(timeLocation, timeValue);

    // generationSize
    GLint generationSizeLocation = glGetUniformLocation(shaderProgram, "generationSize");
    glUniform2fv(generationSizeLocation, 1, generationSize);

    // seed
    GLint seedLocation = glGetUniformLocation(shaderProgram, "seed");
    glUniform1f(seedLocation, gameState.seed);

    // rgb uniform (optional)
    if (rgb != nullptr) {
        GLint rgbLocation = glGetUniformLocation(shaderProgram, "rgb");
        glUniform3fv(rgbLocation, 1, rgb);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gameState.width, gameState.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // Attach texture to framebuffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

    // Check framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Framebuffer not complete!\n");

    // Render to texture (rtt)
    glViewport(0, 0, gameState.width, gameState.height); // Match texture size
    // Add your render code here: this will render to texture instead of screen
    // Remember to clear the framebuffer using glClear if necessary

    // Bind the default framebuffer to render to screen again
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, gameState.width, gameState.height); // Match window size

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

// Storage for persistent shader strings
std::unordered_map<std::string, std::string> shaderSourceStorage;

// Helper function to read shader files from embedded filesystem
const char* readShaderFile(const std::string& filePath) {
    FILE* file = fopen(filePath.c_str(), "r");
    if (!file) {
        printf("Failed to open shader file: %s\n", filePath.c_str());
        return "";
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    std::string content(fileSize, '\0');
    fread(&content[0], 1, fileSize, file);
    fclose(file);
    
    // Store in persistent storage and return pointer
    shaderSourceStorage[filePath] = content;
    return shaderSourceStorage[filePath].c_str();
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
    // Load static shaders from embedded files
    
    // Font shader
    shaderGLSLMap["font"] = {
        readShaderFile("resources/shaders/font_v.glsl"),
        readShaderFile("resources/shaders/font_f.glsl")
    };
    
    // Terrain shader
    shaderGLSLMap["terrain"] = {
        readShaderFile("resources/shaders/terrain_v.glsl"),
        readShaderFile("resources/shaders/terrain_simple.glsl")
    };
    
    // Debug entity shader (test_rgb)
    shaderGLSLMap["debug_entity"] = {
        readShaderFile("resources/shaders/test_rgb_v.glsl"),
        readShaderFile("resources/shaders/test_rgb_f.glsl")
    };
    
    // UI Layer shader
    shaderGLSLMap["ui_layer"] = {
        readShaderFile("resources/shaders/ui_layer_v.glsl"),
        readShaderFile("resources/shaders/ui_layer_f.glsl")
    };
    
    // Texture shader (vert_tex + frag_tex)
    shaderGLSLMap["texture"] = {
        readShaderFile("resources/shaders/vert_tex.glsl"),
        readShaderFile("resources/shaders/frag_tex.glsl")
    };
    
    // Create static shader programs
    createShader(shaderProgramMap["terrain"], "terrain");
    createShader(shaderProgramMap["ui_layer"], "ui_layer");
    createShader(shaderProgramMap["texture"], "texture");
    createShader(shaderProgramMap["debug_entity"], "debug_entity");
    createShader(shaderProgramMap["font"], "font");

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

bool isValidFont(const std::string& fontPath) {
    TTF_Font* testFont = TTF_OpenFont(fontPath.c_str(), 12);
    if (testFont) {
        TTF_CloseFont(testFont);
        return true;
    }
    return false;
}

void renderText(const std::string& text, float x, float y, float scale, float r, float g, float b, float a) {
    glUseProgram(shaderProgramMap["font"]);

    SDL_Color color = {static_cast<Uint8>(r * 255), static_cast<Uint8>(g * 255), static_cast<Uint8>(b * 255), static_cast<Uint8>(a * 255)};

    // Build the full font path and validate
    std::string fontPath = "resources/fonts/" + metaData.font;
    
    // Validate the font exists, otherwise use default
    if (!isValidFont(fontPath)) {
        fontPath = "resources/fonts/HomeVideo-Regular.ttf";
    }

    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), 64);
    if (!font) {
        printf("TTF_OpenFont failed for %s: %s\n", fontPath.c_str(), TTF_GetError());
        return;
    }

    // Reduce wrap width to avoid double spacing
    int wrapWidth = gameState.width/1.5;
    SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), color, wrapWidth);
    if (!surface) {
        printf("Failed to render text: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        return;
    }

    // Fix: Remove extra line spacing by setting font style to TTF_STYLE_NORMAL and adjusting line skip
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    TTF_SetFontHinting(font, TTF_HINTING_MONO);

    // Try to set the line skip to the font height (removes extra spacing)
    int fontHeight = TTF_FontHeight(font);
    surface->h = fontHeight * ((surface->h + fontHeight - 1) / fontHeight);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    SDL_Surface* rgba_surface = SDL_CreateRGBSurface(
        0, surface->w, surface->h, 32,
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
    );
    SDL_BlitSurface(surface, NULL, rgba_surface, NULL);

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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float width = rgba_surface->w * scale;
    float height = rgba_surface->h * scale;

    updateUniformFont(shaderProgramMap["font"], r, g, b, a, x, y, width, height);

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
    // Only proceed if there is at least one Player entity
    auto playerView = registry.view<Player>();
    Position playerPos = {0, 0, 0}; // Default to zeroed Position
    Shape playerShape = {0, 0};  // Default to zeroed Shape
    float playerScreenX = 0.0f, playerScreenY = 0.0f; // Player's actual screen position

    bool hasPlayer = !playerView.empty();
    entt::entity _player;
    if (hasPlayer) {
        _player = playerView.front();
        playerPos = registry.get<Position>(_player);
        playerShape = registry.get<Shape>(_player);
        // Calculate player's actual screen position for interactions
        playerScreenX = playerPos.sx;
        playerScreenY = playerPos.sy;
    }

    // Set clear color based on player context and meta tags
    float clearColor[3];
    bool playerIsInside = false;
    Inside playerInside{};
    
    if (hasPlayer) {
        playerIsInside = registry.all_of<Inside>(_player);
        playerInside = (playerIsInside) ? registry.get<Inside>(_player) : Inside{};
    }
    
    if (playerIsInside && static_cast<int>(playerInside.interior) != -1) {
        // Player is inside - use void color
        if (metaData.void_bg == "color") {
            clearColor[0] = metaData.void_color[0];
            clearColor[1] = metaData.void_color[1];
            clearColor[2] = metaData.void_color[2];
        } else {
            // Default void color is black
            clearColor[0] = 0.0f;
            clearColor[1] = 0.0f;
            clearColor[2] = 0.0f;
        }
    } else {
        // Player is outside - use terrain color if terrain is color
        if (metaData.terrain == "color") {
            clearColor[0] = metaData.terrain_color[0];
            clearColor[1] = metaData.terrain_color[1];
            clearColor[2] = metaData.terrain_color[2];
        } else {
            // Default clear color for terrain shaders
            clearColor[0] = 0.0f;
            clearColor[1] = 0.0f;
            clearColor[2] = 0.0f;
        }
    }
    
    glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(gameState.gameState > 0) {
        // Get camera data using priority-based selection
        entt::entity cameraEntity = selectMainCamera(registry);
        if(cameraEntity == entt::null) return; // No camera, can't render
        
        auto& camera = registry.get<Camera>(cameraEntity);
        auto& cameraShape = registry.get<Shape>(cameraEntity); // Get camera shape for centering offset

        // Render terrain shader if player is outside and terrain is a shader
        if (!playerIsInside && metaData.terrain == "terrain") {
            // Determine which shader to use
            std::string shaderName = metaData.terrain;
            
            float rgb[3] = {0.2f, 0.5f, 0.2f}; // lightish dark green for terrain
            float offsetArray[2] = {camera.offset.x, camera.offset.y};
            float topleftArray[2] = {camera.topLeftTile.x, camera.topLeftTile.y};
            
            // Use the appropriate shader program
            if (shaderProgramMap.find(shaderName) != shaderProgramMap.end()) {
                updateUniforms(
                    shaderProgramMap[shaderName],
                    camera.gridSpacing, 
                    offsetArray, 
                    gameState.width, gameState.height, 
                    topleftArray,
                    generationSize,
                    rgb
                );
            }
        }
            
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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

        // Collect Interior entities for wall rendering
        std::vector<entt::entity> interiorEntities;
        auto visible_entities = registry.view<Visible, InView>();
        for(auto& entity : visible_entities) {
            if(registry.all_of<Interior>(entity)) {
                interiorEntities.push_back(entity);
            }
        }

        // Wall rendering constants
        float wallR = 0.3f, wallG = 0.25f, wallB = 0.2f, wallA = 1.0f;
        
        // Calculate scaled wall height using same logic as ViewSystems.hpp
        float yScale = camera.gridSpacing / (camera.defaultGSV * gameState.height);
        float standardWallHeight = 2 * yScale; // Base height of 5 units, scaled consistently
        
        // PASS 1: Render first wall layer (depends on player context)
        if (playerIsInside) {
            // When inside: render back walls first (behind everything) - opaque
            for(auto& entity : interiorEntities) {
                auto position = registry.get<Position>(entity);
                auto shape = registry.get<Shape>(entity);
                
                // Back wall (top of interior shape) - rendered first when inside
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    wallR, wallG, wallB, wallA, // Opaque when inside
                    position.sx + cameraShape.scaled_size.x,
                    position.sy + cameraShape.scaled_size.y + shape.scaled_size.y + standardWallHeight/2,
                    shape.scaled_size.x, standardWallHeight + standardWallHeight*1.5,
                    0.0f);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        } else {
            // When outside: render front walls first (so player can walk behind them) - opaque
            for(auto& entity : interiorEntities) {
                auto position = registry.get<Position>(entity);
                auto shape = registry.get<Shape>(entity);
                
                // Front wall (bottom of interior shape) - rendered first when outside, opaque
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    wallR, wallG, wallB, wallA, // Opaque when outside
                    position.sx + cameraShape.scaled_size.x,
                    position.sy + cameraShape.scaled_size.y - shape.scaled_size.y + standardWallHeight,
                    shape.scaled_size.x, standardWallHeight + standardWallHeight/2,
                    0.0f);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        // PASS 2: Render all main entities (including Interiors)
        for(auto& entity : visible_entities) {
            auto position = registry.get<Position>(entity);
            auto shape = registry.get<Shape>(entity);
            
            bool isDebug = registry.all_of<Debug>(entity);
            bool is = registry.all_of<Teleport>(entity);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            if(!registry.all_of<Text>(entity)) {
                if (
                    registry.all_of<Texture>(entity) ||
                    registry.all_of<TextureGroupPart>(entity) ||
                    registry.all_of<Textures>(entity) ||
                    (registry.all_of<TextureAlts>(entity) && registry.all_of<Player>(entity))
                ) {
                    // --- Texture (single) ---
                    if (registry.all_of<Texture>(entity)) {
                        const auto& texture = registry.get<Texture>(entity);

                        // Use entity's own position, not offset by player
                        updateUniformsTexture(shaderProgramMap["texture"], 
                            textureIDMap[texture.name],
                            position.sx + cameraShape.scaled_size.x,
                            position.sy + cameraShape.scaled_size.y,
                            shape.scaled_size.x, 
                            shape.scaled_size.y,
                            texture.x, texture.y, texture.w, texture.h
                        );
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    }
                    // --- TextureGroupPart ---
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
                            auto posX = position.sx + cameraShape.scaled_size.x;
                            auto posY = position.sy + cameraShape.scaled_size.y;

                            // Increase size by 1%
                            auto increasedSsizex = ssizex;
                            auto increasedSsizey = ssizey;

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
                                position.sx + cameraShape.scaled_size.x,
                                position.sy + cameraShape.scaled_size.y,
                                shape.scaled_size.x,
                                shape.scaled_size.y,
                                texture.x, texture.y, texture.w, texture.h
                            );
                            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                        }
                    }
                    // --- Textures (animated or multi-frame) ---
                    else if (registry.all_of<Textures>(entity)) {
                        const auto& textures = registry.get<Textures>(entity);
                        const auto& current_texture = textures.textures[textures.current];

                        float angle = 0.0f;
                        if(registry.all_of<Rotation>(entity)) {
                            angle = registry.get<Rotation>(entity).angle;
                        }

                        updateUniformsTexture(shaderProgramMap["texture"], 
                            textureIDMap[current_texture.name],
                            position.sx + cameraShape.scaled_size.x, position.sy + cameraShape.scaled_size.y,
                            shape.scaled_size.x * current_texture.scalex, shape.scaled_size.y * current_texture.scaley,
                            current_texture.x, current_texture.y, current_texture.w, current_texture.h, angle);
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    }
                    // --- TextureAlts (player) ---
                    else if (registry.all_of<TextureAlts>(entity) && registry.all_of<Player>(entity)) {
                        const auto& textureAlts = registry.get<TextureAlts>(entity);
                        const auto& currentTextures = textureAlts.alts.at(textureAlts.current);
                        const auto& current_texture = currentTextures.textures[currentTextures.current];

                        updateUniformsTexture(shaderProgramMap["texture"], 
                            textureIDMap[current_texture.name],
                            position.sx + cameraShape.scaled_size.x,
                            position.sy + cameraShape.scaled_size.y,
                            shape.scaled_size.x * current_texture.scalex, 
                            shape.scaled_size.y * current_texture.scaley,
                            current_texture.x, current_texture.y, 
                            current_texture.w, current_texture.h);
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    }
                }
                else if(registry.all_of<CustomShader>(entity)) {
                    const auto& customShader = registry.get<CustomShader>(entity);
                    
                    float angle = 0.0f;
                    if(registry.all_of<Rotation>(entity)) {
                        angle = registry.get<Rotation>(entity).angle;
                    }
                    
                    // Use the same update pattern as debug/color entities
                    updateUniformsDebug(shaderProgramMap[customShader.shaderName],
                        1.0f, 1.0f, 1.0f, 1.0f, // Default white color
                        position.sx + cameraShape.scaled_size.x, 
                        position.sy + cameraShape.scaled_size.y,
                        shape.scaled_size.x, shape.scaled_size.y, 
                        angle);
                    
                    // Add custom uniforms for all cshaders
                    if (customShader.uniforms.size() >= 3) {
                        // All cshaders now support seed
                        GLint seedLocation = glGetUniformLocation(shaderProgramMap[customShader.shaderName], "uSeed");
                        glUniform1f(seedLocation, customShader.uniforms[2]);
                        
                        // Terrain shader also needs center position
                        if (customShader.shaderName == "terrainmap") {
                            GLint centerPosLocation = glGetUniformLocation(shaderProgramMap[customShader.shaderName], "uCenterPos");
                            glUniform2f(centerPosLocation, customShader.uniforms[0], customShader.uniforms[1]);
                        }
                        
                    }
                    
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                }
                else if(registry.all_of<Color>(entity)) {
                    auto color = registry.get<Color>(entity);

                    float angle = 0.0f;
                    if(registry.all_of<Rotation>(entity)) {
                        angle = registry.get<Rotation>(entity).angle;
                    }

                    float r = color.r;
                    float g = color.g; 
                    float b = color.b;

                    // InteriorPortal color override
                    if (registry.all_of<InteriorPortal>(entity)) {
                        const auto& portal = registry.get<InteriorPortal>(entity);
                        // Offset the original color toward green or red, preserving darkness/brightness
                        float maxComponent = std::max({r, g, b, 0.0001f});
                        float scale = (maxComponent > 0.0f) ? (1.0f / maxComponent) : 1.0f;
                        // Normalize to [0,1] range for offsetting
                        float orig_r = r * scale;
                        float orig_g = g * scale;
                        float orig_b = b * scale;

                        if (!portal.locked) {
                            // Green: keep original color, but set green to max, red and blue to original
                            r = orig_r * 0.3f; // darken red
                            g = std::max(0.7f, orig_g); // boost green
                            b = orig_b * 0.3f; // darken blue
                        } else {
                            // Red: keep original color, but set red to max, green and blue to original
                            r = std::max(0.7f, orig_r); // boost red
                            g = orig_g * 0.3f; // darken green
                            b = orig_b * 0.3f; // darken blue
                        }
                        // Rescale to original intensity
                        float intensity = std::max({color.r, color.g, color.b, 0.0001f});
                        r *= intensity;
                        g *= intensity;
                        b *= intensity;
                    }

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
                        position.sx + cameraShape.scaled_size.x,
                        position.sy + cameraShape.scaled_size.y,
                        shape.scaled_size.x, shape.scaled_size.y, 
                        angle);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                }
            }
            
            // Render text entities within the main rendering order
            if(registry.all_of<Text>(entity)) {
                Text text = registry.get<Text>(entity);
                
                float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
                if (registry.all_of<Color>(entity)) {
                    auto color = registry.get<Color>(entity);
                    r = color.r;
                    g = color.g;
                    b = color.b;
                    a = color.a;
                }
                a = text.hide ? 0.0f : a;

                std::string textStr = text.text;
                float xVal = position.sx + cameraShape.scaled_size.x;
                float yVal = position.sy + cameraShape.scaled_size.y;
                float scaleVal = (shape.scaled_size.x+shape.scaled_size.y)/200;

                renderText(textStr, xVal, yVal, scaleVal, r, g, b, a);
            }
        }

        // PASS 3: Render second wall layer (depends on player context)
        if (!playerIsInside) {
            // When outside: render back walls last (so player walks under them) - translucent for visibility
            for(auto& entity : interiorEntities) {
                auto position = registry.get<Position>(entity);
                auto shape = registry.get<Shape>(entity);
                
                // Back wall (top of interior shape) - rendered last when outside, translucent
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    wallR, wallG, wallB, wallA, // More translucent when outside for player visibility
                    position.sx + cameraShape.scaled_size.x,
                    position.sy + cameraShape.scaled_size.y + shape.scaled_size.y + standardWallHeight/2,
                    shape.scaled_size.x, standardWallHeight + standardWallHeight*1.5,
                    0.0f);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

    }

    // Render menu text from meta tags based on game state
    if (gameState.gameState <= 0) {
        // Get camera for UI scaling using priority-based selection
        entt::entity cameraEntity = selectMainCamera(registry);
        if(cameraEntity != entt::null) {
            auto& camera = registry.get<Camera>(cameraEntity);
            
            std::string menuText = "";
            if (gameState.gameState == 0 && !metaData.pause_menu.empty()) {
                // Pause menu
                menuText = metaData.pause_menu;
            } else if (gameState.gameState == -1 && !metaData.start_menu.empty()) {
                // Start menu
                menuText = metaData.start_menu;
            }
            
            if (!menuText.empty()) {
                // Center of the screen
                float xScale = camera.gridSpacing / (camera.defaultGSV * gameState.width);
                float yScale = camera.gridSpacing / (camera.defaultGSV * gameState.height);
                float offsetArray[2] = {camera.offset.x, camera.offset.y};
                float topleftArray[2] = {camera.topLeftTile.x, camera.topLeftTile.y};
                float x = xScale;
                float y = yScale;
                float scale = 0.001f;
                
                float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
                
                float rgb[3] = {0.0f, 0.0f, 0.0f};
                updateUniforms(
                    shaderProgramMap["terrain"],
                    camera.gridSpacing, 
                    offsetArray, 
                    gameState.width, gameState.height, 
                    topleftArray,
                    generationSize,
                    rgb
                );
                renderText(menuText, x, y, scale, r, g, b, a);
            }
        }
    }

    
}
