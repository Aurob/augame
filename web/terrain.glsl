#define time_divamt 30000.0
precision mediump float;
uniform float grid_spacing;
uniform vec2 offset;
uniform vec2 resolution;
varying vec3 color;
uniform vec2 playerPos;
uniform vec2 toplefttile;
uniform float waterMax;
uniform float sandMax;
uniform float dirtMax;
uniform float grassMax;
uniform float stoneMax;
uniform float snowMax;
uniform float time;
uniform vec2 cursorPos;
uniform float frequency;
uniform float amplitude;
uniform float persistence;
uniform float lacunarity;
uniform int octaves;
uniform float scale; // Add scale as a uniform to control zoom level
uniform float seed; // Add seed as a uniform to influence noise generation

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
    return smootherNoise(p * frequency) * amplitude;
}

void main() {
    vec2 coord1 = gl_FragCoord.xy;
    vec2 coord = (((coord1 / grid_spacing) - toplefttile) * grid_spacing) - offset;
    vec2 _coord = coord / grid_spacing;
    _coord -= resolution * 1.0 / grid_spacing;

    // Calculate distance from the center of the screen (player position)
    vec2 screenCenter = resolution / 2.0;
    float playerDistance = distance(screenCenter, gl_FragCoord.xy);

    vec3 finalColor;
    float scaledRadius = 0.5 * grid_spacing; // Scale the radius with grid spacing
    // Create noise-based terrain
    float n = configurableNoise(_coord, frequency, amplitude);

    // Calculate the gradient factor for the atmosphere based on the distance from the player
    float atmosphereFactor = smoothstep(scaledRadius, scaledRadius * 2.0, playerDistance);

    // Calculate terrain color
    vec3 terrainColor = simple_tile_color(_coord, n);

    // Calculate atmosphere color (light blue gradient)
    // vec3 atmosphereColor = mix(vec3(0.0), vec3(0.5, 0.7, 1.0), atmosphereFactor);

    // Calculate space color (black with points of white and colored light)
    // vec3 spaceColor = vec3(0.0);
    // float starValue = sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233)) * 43758.5453);
    // if (fract(starValue) > 0.995) {
    //     spaceColor = vec3(1.0); // White points
    // }
    // Mix terrain, atmosphere, and space colors based on the gradient factor
    // finalColor = mix(spaceColor, mix(terrainColor, atmosphereColor, atmosphereFactor), 1.0 - atmosphereFactor);
    finalColor = terrainColor;
    gl_FragColor = vec4(finalColor, 1.0);
}