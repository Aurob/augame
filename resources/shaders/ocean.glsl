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

const float frequency = 9.5;
const float amplitude = 0.09;

// Hardcoded drawgrid variable
const bool drawgrid = false;

// --- Begin: Time-based ocean movement helpers (inspired by water1_f.glsl) ---

// Fractal Brownian Motion for subtle turbulence
float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    for (int i = 0; i < 3; i++) {
        value += amplitude * (0.5 + 0.5 * sin(dot(p, vec2(1.7, 2.3)) * frequency + float(i) * 1.7));
        p = p * 1.9 + 0.13;
        amplitude *= 0.5;
        frequency *= 1.7;
    }
    return value;
}

// Gerstner-like wave for large-scale swells
vec2 gerstnerWave(vec2 pos, vec2 direction, float wavelength, float amplitude, float speed, float t, float steepness) {
    float k = 2.0 * 3.14159 / wavelength;
    float phase = speed * k;
    float qi = steepness / (k * amplitude + 0.0001);
    float f = k * dot(direction, pos) + phase * t;
    float sinF = sin(f);
    float cosF = cos(f);
    return vec2(
        qi * amplitude * direction.x * cosF,
        qi * amplitude * direction.y * cosF
    );
}

// Main ocean displacement function
vec2 oceanDisplacement(vec2 pos, float t) {
    vec2 displacement = vec2(0.0);

    // Primary wave direction
    vec2 primaryDir = normalize(vec2(cos(1.2), sin(1.2)));
    vec2 dir1 = normalize(primaryDir + vec2(0.1, 0.0));
    vec2 dir2 = normalize(primaryDir + vec2(-0.05, 0.1));
    vec2 dir3 = normalize(primaryDir + vec2(0.0, -0.08));

    // Large, slow swells
    displacement += gerstnerWave(pos, dir1, 8.0, 0.10, 0.5, t, 0.25);
    displacement += gerstnerWave(pos, dir2, 6.5, 0.07, 0.4, t * 1.1, 0.18);
    displacement += gerstnerWave(pos, dir3, 5.0, 0.05, 0.6, t * 0.9, 0.13);

    // Subtle secondary waves
    displacement += gerstnerWave(pos, primaryDir, 3.0, 0.025, 0.8, t * 1.3, 0.09);

    // Minimal turbulence for calm water
    displacement += vec2(fbm(pos * 1.5 + t * 0.1)) * 0.012;

    return displacement;
}

// --- End: Time-based ocean movement helpers ---

vec3 simple_tile_color(vec2 _coord, float n) {
    vec3 color;

    // Deep ocean - darkest blue
    float depth = 0.1 - n;
    float depthFactor = depth / 0.1;
    color = vec3(
        mix(0.04, 0.01, depthFactor), // R
        mix(0.13, 0.07, depthFactor), // G
        mix(0.28, 0.18, depthFactor)  // B
    );
    return color;
}

vec3 hash(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)), 
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 101.5)));
    return -1.0 + 2.0 * fract(sin(p + seed) * 43758.5453);
}

float smoothNoise(vec2 p) {
    // More complex noise using multiple trigonometric functions with phase shifts
    const float xFreq1 = 0.1;  // Primary frequency for x component
    const float yFreq1 = 0.1;  // Primary frequency for y component
    const float xFreq2 = 0.05; // Secondary frequency for x component
    const float yFreq2 = 0.07; // Secondary frequency for y component
    
    // Combine multiple sine and cosine waves with different frequencies (no phase shifts)
    float noise1 = 0.5 * sin(p.x * xFreq1) + 0.5 * cos(p.y * yFreq1);
    float noise2 = 0.3 * sin(p.x * xFreq2 + p.y * 0.08) + 0.3 * cos(p.y * yFreq2 - p.x * 0.06);
    float noise3 = 0.2 * sin((p.x + p.y) * 0.12) * cos((p.x - p.y) * 0.09);
    
    // Combine the noise components with some non-linear operations
    float combinedNoise = noise1 + noise2 * (1.0 + 0.2 * sin(p.x * 0.3)) + noise3;
    
    // Normalize to 0.0-1.0 range
    return (combinedNoise + 1.5) * 0.33;
}

float calculate_n(vec2 _coord) {
    float n = 0.0;
    float layerFrequency = frequency;
    float layerAmplitude = amplitude;
    const int numLayers = 10; // Adjust as needed for desired complexity

    // Add a slight rotation to each octave for more natural patterns
    float rotationAngle = 0.15;
    float sinRot = sin(rotationAngle);
    float cosRot = cos(rotationAngle);

    for (int i = 0; i < numLayers; i++) {
        // Apply slight rotation to coordinates for each layer
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
        
        // Get noise value and apply domain warping for more complex patterns
        float noiseVal = smoothNoise(rotatedCoord);
        
        // Apply domain warping for higher octaves
        if (i > 3) {
            vec2 warp = vec2(
                sin(rotatedCoord.y * 0.5 + float(i) * 0.1),
                cos(rotatedCoord.x * 0.5 + float(i) * 0.2)
            ) * 0.15;
            noiseVal = smoothNoise(rotatedCoord + warp);
        }
        n += layerAmplitude * noiseVal;
        
        // Adjust frequency and amplitude for next layer
        layerFrequency *= 1.5;
        layerAmplitude *= 0.55;
    }

    // Apply a subtle ridge effect to create more interesting terrain features
    n = abs(n * 2.0 - 1.0);
    n = 1.0 - n;
    n = n * n;
    
    // Ensure n stays within reasonable bounds (0.0 to 1.0)
    n = clamp(n, 0.0, 1.0);
    
    return n;
}

void main() {
    vec2 coord = gl_FragCoord.xy;

    // Invert the y-coordinate
    coord.y = resolution.y - coord.y;

    // Calculate generationOffset based on generationSize
    vec2 generationOffset = vec2(generationSize.x / 2.0, generationSize.y / 2.0);

    // Adjust the coordinates with grid spacing, toplefttile, offset, and generationOffset
    vec2 adjustedCoord = (coord / grid_spacing) + toplefttile + (offset / grid_spacing) + generationOffset;

    // --- Begin: Time-based animated ocean movement ---
    // Use time to animate the ocean surface, like in water1_f.glsl
    // Make sure time is scaled so that movement is visible
    float t = time * 50.0 + seed * 0.13;

    // Use a slightly scaled version of adjustedCoord for more natural movement
    vec2 oceanPos = adjustedCoord * 1.0;

    // Get animated displacement for ocean swells and waves
    vec2 displacement = oceanDisplacement(oceanPos, t);

    // Apply the displacement to the coordinates used for noise
    vec2 _coord = adjustedCoord + displacement;
    // --- End: Time-based animated ocean movement ---

    // Calculate n using enhanced trigonometric functions
    float n = calculate_n(_coord);
    
    // Calculate terrain color
    vec3 terrainColor = simple_tile_color(_coord, n);
    vec3 finalColor = terrainColor;

    // Draw grid overlay if drawgrid is true
    if (drawgrid) {
        // Use world coordinates for grid calculation instead of screen coordinates
        // This ensures grid cells maintain consistent size regardless of zoom level
        vec2 worldGridPos = mod(_coord, 1.0);

        // Grid line thickness in world coordinates (scaled by zoom level)
        float worldGridThickness = 1.0 / grid_spacing;

        // If close to a grid line in x or y, draw the grid color
        float lineX = step(worldGridPos.x, worldGridThickness) + step(1.0 - worldGridPos.x, worldGridThickness);
        float lineY = step(worldGridPos.y, worldGridThickness) + step(1.0 - worldGridPos.y, worldGridThickness);

        float gridLine = clamp(lineX + lineY, 0.0, 1.0);

        // Grid color (light gray)
        vec3 gridColor = vec3(0.85, 0.85, 0.85);

        // Blend grid color over terrain color
        finalColor = mix(finalColor, gridColor, gridLine * 0.7);
    }

    gl_FragColor = vec4(finalColor, 1.0);
}
