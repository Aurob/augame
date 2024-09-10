#include <SDL_opengles2.h>
#include <cmath>
#include <array>
#include "../include/shaders.hpp"
#include "../include/json.hpp"
#include <SDL_opengles2.h>
#include <vector>
#include <unordered_map>
#include <string>
#include "../include/UIUtils.hpp"

extern GLfloat cursorPos[2];
extern float seed;
extern float gridSpacingValue;
extern float offsetValue[2];
extern float toplefttile[2];
extern float generationSize[2];
extern entt::registry registry;

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
    GLint cursorPosLocation = glGetUniformLocation(shaderProgram, "cursorPos");
    glUniform2f(cursorPosLocation, cursorPos[0], cursorPos[1]);

    // time
    GLint timeLocation = glGetUniformLocation(shaderProgram, "time");
    glUniform1f(timeLocation, SDL_GetTicks() / 1000000.0f);

    // generationSize
    GLint generationSizeLocation = glGetUniformLocation(shaderProgram, "generationSize");
    glUniform2fv(generationSizeLocation, 1, generationSize);

    // seed
    GLint seedLocation = glGetUniformLocation(shaderProgram, "seed");
    glUniform1f(seedLocation, seed);
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
    GLint cursorPosLocation = glGetUniformLocation(shaderProgram, "cursorPos");
    glUniform2f(cursorPosLocation, cursorPos[0], cursorPos[1]);

    // toplefttile uniform
    GLint toplefttileLocation = glGetUniformLocation(shaderProgram, "toplefttile");
    glUniform2fv(toplefttileLocation, 1, toplefttile);
}
void updateUniformsTexture(GLuint &shaderProgram, GLuint textureID, float x, float y, float scalex, float scaley, float startX = 0.0f, float startY = 0.0f, float sizeX = 1.0f, float sizeY = 1.0f) {
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


void createShader(GLuint &shaderProgram, std::string program_name)
{

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
        // printf("IMG_Load: %s\n", IMG_GetError());
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

void renderAll(const context *ctx) {

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
        const auto& lhsPos = registry.get<Position>(lhs);
        const auto& rhsPos = registry.get<Position>(rhs);
        const auto& lhsShape = registry.get<Shape>(lhs);
        const auto& rhsShape = registry.get<Shape>(rhs);
        const auto& lhsPrio = registry.get<RenderPriority>(lhs);
        const auto& rhsPrio = registry.get<RenderPriority>(rhs);
        

        // Compare y + z + shape.z
        float lhsYZS = lhsPos.y + lhsPos.z + lhsShape.size.z;
        float rhsYZS = rhsPos.y + rhsPos.z + rhsShape.size.z;
        if (lhsYZS != rhsYZS)
            return lhsYZS < rhsYZS;

        return lhsPrio.priority < rhsPrio.priority;
    });

    // Render visible entities
    auto visible_entities = registry.view<Visible>();
    for(auto& entity : visible_entities) {
        auto position = registry.get<Position>(entity);
        auto shape = registry.get<Shape>(entity);
        
        bool isDebug = registry.all_of<Debug>(entity);
        bool isTeleport = registry.all_of<Teleport>(entity);

 
        if (registry.all_of<Texture>(entity)) {

            const auto& texture = registry.get<Texture>(entity);


            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap[texture.name],
                position.sx + playerShape.scaled_size.x,
                position.sy + playerShape.scaled_size.y + shape.scaled_size.z,
                shape.scaled_size.x, 
                shape.scaled_size.y + shape.scaled_size.z,
                texture.x, texture.y, texture.w, texture.h
            );
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        }
        else if (registry.all_of<TextureGroupPart>(entity)) {
            const auto& textureGroupPart = registry.get<TextureGroupPart>(entity);
            auto groupName = textureGroupPart.groupName;
            auto partName = textureGroupPart.partName;


            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            auto rootTexture = textureIDMap[groupName];
            const auto& texture = textureGroupMap[groupName].at(partName);
            updateUniformsTexture(shaderProgramMap["texture"], 
                rootTexture,
                position.sx + playerShape.scaled_size.x,
                position.sy + playerShape.scaled_size.y + position.sz + playerShape.scaled_size.z,
                shape.scaled_size.x,
                shape.scaled_size.y,
                texture.x, texture.y, texture.w, texture.h
            );
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        } 
        else if (registry.all_of<Textures>(entity)) {
            const auto& textures = registry.get<Textures>(entity);
            const auto& current_texture = textures.textures[textures.current];


            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            if (registry.all_of<RenderDebug>(entity)) {
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    1.0f, 1.0f, 1.0f, 0.2f,
                    position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y,
                    shape.scaled_size.x, shape.scaled_size.y, 0.0f);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap[current_texture.name],
                position.sx + playerShape.scaled_size.x, position.sy + playerShape.scaled_size.y,
                shape.scaled_size.x * current_texture.scalex, shape.scaled_size.y * current_texture.scaley,
                current_texture.x, current_texture.y, current_texture.w, current_texture.h);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        } else if (registry.all_of<TextureAlts, Player>(entity)) {
            const auto& textureAlts = registry.get<TextureAlts>(entity);
            const auto& currentTextures = textureAlts.alts.at(textureAlts.current);
            const auto& current_texture = currentTextures.textures[currentTextures.current];
            // Check if the entity has RenderDebug
            // if (registry.all_of<RenderDebug, Player>(entity)) {
                // Render a small transparent square to indicate bounding box
                updateUniformsDebug(shaderProgramMap["debug_entity"],
                    1.0f, 1.0f, 1.0f, 0.2f, // White color with 20% opacity
                    position.sx + playerShape.scaled_size.x, 
                    position.sy + playerShape.scaled_size.y,
                    shape.scaled_size.x, shape.scaled_size.y, 0.0f);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            // }

            updateUniformsTexture(shaderProgramMap["texture"], 
                textureIDMap[current_texture.name],
                position.sx + playerShape.scaled_size.x,
                position.sy + playerShape.scaled_size.y + position.sz + playerShape.scaled_size.z*2,
                shape.scaled_size.x * current_texture.scalex, 
                shape.scaled_size.y * current_texture.scaley,
                current_texture.x, current_texture.y, 
                current_texture.w, current_texture.h);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        }
        else if(registry.all_of<Color>(entity)) {
            auto color = registry.get<Color>(entity);

            float angle = 0.0f;
            if(registry.all_of<Rotation>(entity)) {
                angle = registry.get<Rotation>(entity).angle;
            }

            updateUniformsDebug(shaderProgramMap["debug_entity"],
                color.r, color.g, color.b, color.a,
                position.sx + playerShape.scaled_size.x, 
                position.sy + playerShape.scaled_size.y,
                shape.scaled_size.x, shape.scaled_size.y, 
                angle);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

    }

    updateUIShader(shaderProgramMap["ui_layer"], 
        width, height, gridSpacingValue,
        toplefttile);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    // Handle UIElement Components
    auto ui_elements = registry.view<UIElement>();
    for(auto& entity : ui_elements) {
        auto& uiElement = registry.get<UIElement>(entity);
        if(uiElement.visible) {
            auto position = registry.get<Position>(entity);
            auto shape = registry.get<Shape>(entity);
            test_imgui(uiElement.content, 
                ( (1 - ((position.sx + playerShape.scaled_size.x*1.25 + shape.scaled_size.x*0.1) + 1) / 2) * width),
                ( (1 - ((position.sy + playerShape.scaled_size.y*1.25 + shape.scaled_size.y*0.1) + 1) / 2) * height),
                (shape.scaled_size.x + shape.scaled_size.x*.1) * width,
                (shape.scaled_size.y + shape.scaled_size.y*.1) * height
            );
        }
    }

    // Swap buffers
    SDL_GL_SwapWindow(ctx->window);

}