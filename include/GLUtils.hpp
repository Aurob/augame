#include <SDL_opengles2.h>
#include <functional>
#include <cmath>
#include <array>
#include "../include/shaders.hpp"

extern int seed;
extern GLfloat cursorPos[2];

void loadGl(SDL_Window *mpWindow)
{

    // Create OpenGLES 2 context on SDL window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GLContext glc = SDL_GL_CreateContext(mpWindow);

    // Set clear color to black
    // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    // testing red
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
}

void updateUniforms(GLuint &shaderProgram,
                    float gridSpacingValue,
                    float offsetValue[2],
                    float _width, float _height,
                    float playerPosition[2],
                    float toplefttile[2],
                    float scale,
                    float waterMax, float sandMax, float dirtMax, float grassMax, float stoneMax, float snowMax,
                    float lastTime,
                    float frequency, float amplitude, float persistence, float lacunarity, int octaves)
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

    // playerPos uniform
    GLint playerPosLocation = glGetUniformLocation(shaderProgram, "playerPos");
    glUniform2f(playerPosLocation, playerPosition[0], playerPosition[1]);

    // bounds uniform
    GLint boundsLocation = glGetUniformLocation(shaderProgram, "toplefttile");
    glUniform2fv(boundsLocation, 1, toplefttile);

    // scale uniform
    GLint scaleLocation = glGetUniformLocation(shaderProgram, "scale");
    glUniform1f(scaleLocation, scale);

    // waterMax uniform
    GLint waterMaxLocation = glGetUniformLocation(shaderProgram, "waterMax");
    glUniform1f(waterMaxLocation, waterMax);

    // sandMax uniform
    GLint sandMaxLocation = glGetUniformLocation(shaderProgram, "sandMax");
    glUniform1f(sandMaxLocation, sandMax);

    // dirtMax uniform
    GLint dirtMaxLocation = glGetUniformLocation(shaderProgram, "dirtMax");
    glUniform1f(dirtMaxLocation, dirtMax);

    // grassMax uniform
    GLint grassMaxLocation = glGetUniformLocation(shaderProgram, "grassMax");
    glUniform1f(grassMaxLocation, grassMax);

    // stoneMax uniform
    GLint stoneMaxLocation = glGetUniformLocation(shaderProgram, "stoneMax");
    glUniform1f(stoneMaxLocation, stoneMax);

    // snowMax uniform
    GLint snowMaxLocation = glGetUniformLocation(shaderProgram, "snowMax");
    glUniform1f(snowMaxLocation, snowMax);

    // time uniform
    GLint timeLocation = glGetUniformLocation(shaderProgram, "time");
    glUniform1f(timeLocation, lastTime);

    // cursorPos uniform
    GLint cursorPosLocation = glGetUniformLocation(shaderProgram, "cursorPos");
    glUniform2f(cursorPosLocation, cursorPos[0], cursorPos[1]);

    // frequency uniform
    GLint frequencyLocation = glGetUniformLocation(shaderProgram, "frequency");
    glUniform1f(frequencyLocation, frequency);

    // amplitude uniform
    GLint amplitudeLocation = glGetUniformLocation(shaderProgram, "amplitude");
    glUniform1f(amplitudeLocation, amplitude);

    // persistence uniform
    GLint persistenceLocation = glGetUniformLocation(shaderProgram, "persistence");
    glUniform1f(persistenceLocation, persistence);

    // lacunarity uniform
    GLint lacunarityLocation = glGetUniformLocation(shaderProgram, "lacunarity");
    glUniform1f(lacunarityLocation, lacunarity);

    // octaves uniform
    GLint octavesLocation = glGetUniformLocation(shaderProgram, "octaves");
    glUniform1i(octavesLocation, octaves);
}

void updateUniforms2(GLuint &shaderProgram, float _width, float _height, float gridSpacingValue)
{
    glUseProgram(shaderProgram);

    // resolution uniform
    GLint resolutionLocation = glGetUniformLocation(shaderProgram, "resolution");
    glUniform2f(resolutionLocation, _width, _height);

    // grid_spacing uniform
    GLint gridSpacingLocation = glGetUniformLocation(shaderProgram, "grid_spacing");
    glUniform1f(gridSpacingLocation, gridSpacingValue);
}



void updateUniformsTree(GLuint &shaderProgram, float x, float y, float scale, float _width, float _height, float gridSpacingValue) {
    glUseProgram(shaderProgram);

    GLint instancePositionLocation = glGetUniformLocation(shaderProgram, "u_position");
    glUniform2f(instancePositionLocation, x, y);

    GLint instanceScaleLocation = glGetUniformLocation(shaderProgram, "u_scale");
    glUniform1f(instanceScaleLocation, scale);

    GLint resolutionLocation = glGetUniformLocation(shaderProgram, "u_resolution");
    glUniform2f(resolutionLocation, _width, _height);

    GLint gridSpacingLocation = glGetUniformLocation(shaderProgram, "u_grid_spacing");
    glUniform1f(gridSpacingLocation, gridSpacingValue);
}

void updateUniformsTexture(GLuint &shaderProgram, GLuint textureID, float x, float y, float scale) {
    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLint instancePositionLocation = glGetUniformLocation(shaderProgram, "instancePosition");
    glUniform2f(instancePositionLocation, x, y);

    GLint instanceScaleLocation = glGetUniformLocation(shaderProgram, "instanceScale");
    glUniform1f(instanceScaleLocation, scale);
}


void loadGL1(GLuint &shaderProgram, std::string program_name)
{

    // Create and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &shaderGLSLMap[program_name][0], NULL);
    glCompileShader(vertexShader);
    
    // Check for vertex shader compilation errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    // printf("Vertex: %s %s\n", program_name.c_str(), shaderGLSLMap[program_name][0]);
    if (!success)
    {   
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // Create and compile fragment shader
    // printf("Fragment: %s %s\n", program_name.c_str(), shaderGLSLMap[program_name][1]);
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

    printf("Attaching shaders...\n");
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    printf("Linking shaders...\n");

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

    // seed uniform
    GLint seedLocation = glGetUniformLocation(shaderProgram, "seed");
    glUniform1f(seedLocation, static_cast<float>(seed));

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

// // Texture Stuff

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
    printf("Image loaded successfully\n");

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

GLuint loadGLTexture(GLuint &shaderProgram) {
    // Load shaders and create a program
    shaderProgram = createProgram(shaderGLSLMap["texture"][0], shaderGLSLMap["texture"][1]);
    glUseProgram(shaderProgram);

    // Load an image and create a texture from it
    GLuint textureID;
    loadImageAndCreateTexture(shaderGLSLMap["texture"][2], textureID);

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

// Utility functions

float dot(const std::array<float, 3>& v1, const std::array<float, 3>& v2) {
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

std::array<float, 3> fract(const std::array<float, 3>& v) {
    return {v[0] - std::floor(v[0]), v[1] - std::floor(v[1]), v[2] - std::floor(v[2])};
}

// Hash function
std::array<float, 3> customHash(const std::array<float, 3>& p);
std::array<float, 3> customHash(const std::array<float, 3>& p) {
    auto p1 = std::array<float, 3>{dot(p, {127.1f, 311.7f, 74.7f}), 
                                   dot(p, {269.5f, 183.3f, 246.1f}),
                                   dot(p, {113.5f, 271.9f, 101.5f})};
    auto s = [](float x) { return std::sin(x) * 43758.5453f; };
    auto f = fract({s(p1[0]), s(p1[1]), s(p1[2])});
    return {-1.0f + 2.0f * f[0], -1.0f + 2.0f * f[1], -1.0f + 2.0f * f[2]};
}

// Smoother noise function

float smootherNoise(const std::array<float, 2>& p) {
    auto i = std::array<float, 2>{std::floor(p[0]), std::floor(p[1])};
    auto f = std::array<float, 2>{p[0] - i[0], p[1] - i[1]};
    auto u = std::array<float, 2>{f[0] * f[0] * (3.0f - 2.0f * f[0]), 
                                  f[1] * f[1] * (3.0f - 2.0f * f[1])};

    auto g = [i](float x, float y){
        return customHash({i[0] + x, i[1] + y, 0.0f});
    };

    auto g00 = g(0.0f, 0.0f);
    auto g10 = g(1.0f, 0.0f);
    auto g01 = g(0.0f, 1.0f);
    auto g11 = g(1.0f, 1.0f);

    auto n = [f](const std::array<float, 3>& g, float dx, float dy) {
        return g[0] * (f[0] - dx) + g[1] * (f[1] - dy);
    };

    float n00 = n(g00, 0.0f, 0.0f);
    float n10 = n(g10, 1.0f, 0.0f);
    float n01 = n(g01, 0.0f, 1.0f);
    float n11 = n(g11, 1.0f, 1.0f);

    float nX0 = (1 - u[0]) * n00 + u[0] * n10;
    float nX1 = (1 - u[0]) * n01 + u[0] * n11;
    return (1 - u[1]) * nX0 + u[1] * nX1;
}
