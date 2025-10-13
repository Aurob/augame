#define time_divamt 30000.0
precision mediump float;
uniform float grid_spacing;
uniform vec2 offset;
uniform vec2 resolution;
varying vec3 color;
uniform vec2 playerPos;
uniform vec2 toplefttile;
uniform vec2 cursorPos;
uniform vec2 generationSize;
uniform float scale;
uniform float seed;
uniform float time;
uniform sampler2D uTerrainTexture;
uniform vec4 terrain_bounds; // minX, minY, maxX, maxY (0,0,0,0 = no bounds)

const float frequency = 9.5;
const float amplitude = 0.70;
const float tileScale = 8.0; // Higher = smaller tiles

// Hardcoded drawgrid variable
const bool drawgrid = false;

// Forward declarations
float calculate_n(vec2 _coord);

// Get UV coordinates for tile in 4x2 tileset
vec2 getTileUV(int tileIndex, vec2 localCoord) {
    // Tileset layout: 4 tiles wide, 2 tiles tall
    // Top row (y=0): 0=grass, 1=water, 2=light_grass, 3=dark_water
    // Bottom row (y=1): 4=sand, 5=stone, 6=dirt, 7=snow

    vec2 tileOffset;

    // Manual division and modulo for GLSL ES 1.0
    int row = tileIndex / 4;
    int col = tileIndex - (row * 4);

    tileOffset.x = float(col) * 0.25;
    tileOffset.y = float(row) * 0.5;

    // Wrap local coordinates to [0,1] range for tiling
    vec2 wrappedCoord = fract(localCoord);

    // Scale to tile size and add offset
    return tileOffset + vec2(wrappedCoord.x * 0.25, wrappedCoord.y * 0.5);
}

// Get biome type from noise value
int getBiome(float n, vec2 coord) {
    if (n < 0.1) {
        // Water: use light (1) near sand, dark (3) deeper
        float waterDepth = n / 0.1; // 0=deep, 1=shallow
        return waterDepth > 0.5 ? 1 : 3;
    }
    else if (n < 0.3) return 4; // Sand
    else if (n < 0.6) {
        // Grass: randomly choose between types 0 and 2
        float grassRandom = fract(sin(dot(floor(coord * tileScale), vec2(12.9898, 78.233)) + seed) * 43758.5453);
        return grassRandom > 0.5 ? 0 : 2;
    }
    else if (n < 0.8) return 6; // Dirt
    else if (n < 0.95) return 5; // Stone
    else return 7;              // Snow
}

vec3 simple_tile_color(vec2 _coord, float n) {
    int centerBiome = getBiome(n, _coord);

    // Default: render base tile
    vec2 uv = getTileUV(centerBiome, _coord * tileScale);
    return texture2D(uTerrainTexture, uv).rgb;
}

vec3 hash(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)), 
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 101.5)));
    return -1.0 + 2.0 * fract(sin(p + seed) * 43758.5453);
}

float smoothNoise(vec2 p) {
    const float xFreq1 = 0.1;
    const float yFreq1 = 0.1;
    const float xFreq2 = 0.05;
    const float yFreq2 = 0.07;

    // Add seed-based phase shifts to make noise vary with different seeds
    float phaseShift = seed * 0.1;

    float noise1 = 0.5 * sin(p.x * xFreq1 + phaseShift) + 0.5 * cos(p.y * yFreq1 + phaseShift * 1.3);
    float noise2 = 0.3 * sin(p.x * xFreq2 + p.y * 0.08 + phaseShift * 0.7) + 0.3 * cos(p.y * yFreq2 - p.x * 0.06 + phaseShift * 1.7);
    float noise3 = 0.2 * sin((p.x + p.y) * 0.12 + phaseShift * 2.1) * cos((p.x - p.y) * 0.09 + phaseShift * 1.1);

    float combinedNoise = noise1 + noise2 * (1.0 + 0.2 * sin(p.x * 0.3 + phaseShift * 0.5)) + noise3;

    return (combinedNoise + 1.5) * 0.33;
}

float calculate_n(vec2 _coord) {
    float n = 0.0;
    float layerFrequency = frequency;
    float layerAmplitude = amplitude;
    const int numLayers = 10;

    float rotationAngle = 0.15;

    for (int i = 0; i < numLayers; i++) {
        vec2 rotatedCoord = _coord * layerFrequency;
        if (i > 0) {
            float rotAmount = float(i) * rotationAngle;
            float s = sin(rotAmount);
            float c = cos(rotAmount);
            rotatedCoord = vec2(
                rotatedCoord.x * c - rotatedCoord.y * s,
                rotatedCoord.x * s + rotatedCoord.y * c
            );
        }
        
        float noiseVal = smoothNoise(rotatedCoord);
        
        if (i > 3) {
            // Add seed to domain warping for variation
            vec2 warp = vec2(
                sin(rotatedCoord.y * 0.5 + float(i) * 0.1 + seed * 0.05),
                cos(rotatedCoord.x * 0.5 + float(i) * 0.2 + seed * 0.07)
            ) * 0.15;
            noiseVal = smoothNoise(rotatedCoord + warp);
        }
        n += layerAmplitude * noiseVal;
        
        layerFrequency *= 1.5;
        layerAmplitude *= 0.55;
    }

    n = abs(n * 2.0 - 1.0);
    n = 1.0 - n;
    n = n * n;
    n = clamp(n, 0.0, 1.0);
    
    return n;
}

void main() {
    vec2 coord = gl_FragCoord.xy;
    coord.y = resolution.y - coord.y;

    // Simple planar world coords
    vec2 generationOffset = vec2(generationSize.x / 2.0, generationSize.y / 2.0);
    vec2 sampleCoord = (coord / grid_spacing) + toplefttile + (offset / grid_spacing) + generationOffset;

    // Check if we have bounds and if we're outside them
    bool hasBounds = (terrain_bounds.x != 0.0 || terrain_bounds.y != 0.0 || terrain_bounds.z != 0.0 || terrain_bounds.w != 0.0);
    bool outOfBounds = hasBounds && (
        sampleCoord.x < terrain_bounds.x ||
        sampleCoord.x > terrain_bounds.z ||
        sampleCoord.y < terrain_bounds.y ||
        sampleCoord.y > terrain_bounds.w
    );

    vec3 terrainColor;
    if (outOfBounds) {
        // Outside bounds - render black/void
        terrainColor = vec3(0.0, 0.0, 0.0);
    } else {
        // Inside bounds or no bounds - render terrain
        float n = calculate_n(sampleCoord);
        terrainColor = simple_tile_color(sampleCoord, n);
    }

    vec3 finalColor = terrainColor;

    // Optional grid
    if (drawgrid) {
        vec2 worldGridPos = fract(sampleCoord);
        float worldGridThickness = 1.0 / grid_spacing;
        float lineX = step(worldGridPos.x, worldGridThickness) + step(1.0 - worldGridPos.x, worldGridThickness);
        float lineY = step(worldGridPos.y, worldGridThickness) + step(1.0 - worldGridPos.y, worldGridThickness);
        float gridLine = clamp(lineX + lineY, 0.0, 1.0);
        vec3 gridColor = vec3(0.85);
        finalColor = mix(finalColor, gridColor, gridLine * 0.7);
    }

    gl_FragColor = vec4(finalColor, 1.0);
}