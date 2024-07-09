#define time_divamt 30000.0
precision mediump float;
uniform float grid_spacing;
uniform vec2 offset;
uniform vec2 resolution;
varying vec3 color;
uniform vec2 playerPos;
uniform vec2 toplefttile;
uniform vec2 cursorPos; // Add cursorPos uniform
// Add a lot of grass with patches of dirt and a sandy shore around small seas, sparse stone and rare snow
const float waterMax = 0.01;  // Less water
const float sandMax = 0.10;   // Sandy shore around seas
const float dirtMax = 0.15;   // Less dirt
const float grassMax = 0.80;  // More grass
const float stoneMax = 0.90;  // More stone
const float snowMax = 1.0;    // Rare snow
const float frequency = 0.25;
const float amplitude = 1.0;
const float persistence = 0.5;
const float lacunarity = 2.0;
const int octaves = 4;
uniform float scale; // Add scale as a uniform to control zoom level
uniform float seed; // Add seed as a uniform to influence noise generation
uniform float time;

vec4 permute(in vec4 x) {return mod(((x * 34.0) + 1.0) * x, 289.0);}
vec4 taylorInvSqrt(in vec4 r) {return 1.79284291400159 - 0.85373472095314 * r;}

vec3 simple_tile_color(vec2 _coord, float n) {
    vec3 color;
    if (n < waterMax) {
        // Water
        color = vec3(20.0 / 255.0, 24.0 / 255.0, 34.0 / 255.0); // Darker deep water color
    } else if (n < sandMax) {
        // Sand 
        color = vec3(0.95, 0.87, 0.70); // Lighter sand color
    } else if (n < dirtMax) {
        // Dirt
        color = vec3(164.0 / 255.0, 158.0 / 255.0, 130.0 / 255.0); // Lightest dirt color
    } else if (n < grassMax) {
        // Grass
        color = vec3(48.0 / 255.0, 71.0 / 255.0, 40.0 / 255.0); // Darkest grass color
    } else if (n < stoneMax) {
        // Stone
        color = vec3(144.0 / 255.0, 144.0 / 255.0, 144.0 / 255.0); // Lightest stone color
    } else {
        // Snow
        color = vec3(1.0, 1.0, 1.0); // Lighter snow color
    }
    return color;
}

vec3 hash(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)), 
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 101.5)));
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453);
}
float smootherNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);

    vec3 g00 = hash(vec3(i.x, i.y, 0.0));
    vec3 g10 = hash(vec3(i.x + 1.0, i.y, 0.0));
    vec3 g01 = hash(vec3(i.x, i.y + 1.0, 0.0));
    vec3 g11 = hash(vec3(i.x + 1.0, i.y + 1.0, 0.0));

    float n00 = dot(g00.xy, f - vec2(0.0, 0.0));
    float n10 = dot(g10.xy, f - vec2(1.0, 0.0));
    float n01 = dot(g01.xy, f - vec2(0.0, 1.0));
    float n11 = dot(g11.xy, f - vec2(1.0, 1.0));

    float nX0 = mix(n00, n10, u.x);
    float nX1 = mix(n01, n11, u.x);
    return mix(nX0, nX1, u.y);
}
float configurableNoise(vec2 p, float frequency, float amplitude) {
    return amplitude * smootherNoise(p * frequency);
}
void main() {
    vec2 coord = gl_FragCoord.xy;

    // Invert the y-coordinate
    coord.y = resolution.y - coord.y;

    // Adjust the coordinates with grid spacing, toplefttile, and offset
    vec2 adjustedCoord = (coord / grid_spacing) + toplefttile + (offset / grid_spacing);

    vec2 _coord = adjustedCoord;

    float n = 0.0;
    float layerFrequency = frequency;
    float layerAmplitude = amplitude;
    const int numLayers = 9; // Adjust as needed for desired complexity

    for (int i = 0; i < numLayers; i++) {
        vec2 z = _coord * layerFrequency;
        n += configurableNoise(z, layerFrequency, layerAmplitude);
        layerFrequency *= 1.25;
        layerAmplitude *= 0.55;
    }
    
    // Calculate terrain color
    vec3 terrainColor = simple_tile_color(_coord, n);
    vec3 finalColor = terrainColor;
    if (grid_spacing <= 128.0) {
        if(length(_coord) > resolution.y/2.0) {
            finalColor = vec3(0.0, 0.0, 0.0);
        }
    }

    gl_FragColor = vec4(finalColor, 1.0);
}

