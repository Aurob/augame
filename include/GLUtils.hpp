
#include <SDL_opengles2.h>
#include "../include/shaders.hpp"

extern int seed;
extern GLfloat cursorPos[2];


void updateUniforms(GLuint shaderProgram, 
float gridSpacingValue, 
float offsetValue[2], 
float _width, float _height, 
float playerPosition[2], 
float toplefttile[2], 
float scale,
float waterMax, float sandMax, float dirtMax, float grassMax, float stoneMax, float snowMax, 
float lastTime, 
float frequency, float amplitude, float persistence, float lacunarity, int octaves) {
    
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

void loadGl(SDL_Window *mpWindow) {
    
    // Create OpenGLES 2 context on SDL window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GLContext glc = SDL_GL_CreateContext(mpWindow);

    // Set clear color to black
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

}

GLuint loadGL2(GLuint &shaderProgram) {

    // Create and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSourceTest, NULL);
    glCompileShader(vertexShader);

    // Create and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSourceTest, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();    

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    GLfloat vertices[] = {
        // Positions        // Colors (R, G, B)
        0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // Top vertex (Red)
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // Bottom left vertex (Green)
        0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // Bottom right vertex (Blue)
        0.0f, -0.5f, 0.0f,  1.0f, 1.0f, 0.0f   // Additional vertex to satisfy vertex fetch requirement
    };
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //Bind VBO before setting vertex attributes for safety
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);



    return shaderProgram;
}

GLuint loadGL1(GLuint &shaderProgram) {
    
    // Create and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Create and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1000, 1000, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // Attach texture to framebuffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

    // Check framebuffer is complete
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Framebuffer not complete!\n");

    // Render to texture (rtt)
    glViewport(0, 0, 1000, 1000); // Match texture size
    // Add your render code here: this will render to texture instead of screen
    // Remember to clear the framebuffer using glClear if necessary

    // Bind the default framebuffer to render to screen again
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1000, 1000); // Match window size

    // In your render loop, use the generated texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    return shaderProgram;
}