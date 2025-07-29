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
const float amplitude = 0.70;

vec3 simple_tile_color(vec2 _coord, float n) {
    vec3 color;
    if (n < 0.1) {
        // Water - slightly lighter ocean
        float depth = 0.1 - n;  // Deeper water is darker
        float depthFactor = depth / 0.1;  // Normalize to 0-1 range
        color = vec3(
            mix(0.12, 0.08, depthFactor),
            mix(0.16, 0.12, depthFactor),
            mix(0.24, 0.20, depthFactor)
        );
    } else if (n < 0.3) {
        // Sand - more pale with exaggerated gradient
        float sandFactor = (n - 0.1) / 0.2;  // Normalize to 0-1 range
        // Apply a power function to exaggerate the gradient
        sandFactor = pow(sandFactor, 2.5);
        color = vec3(
            mix(0.96, 0.82, sandFactor),
            mix(0.94, 0.76, sandFactor),
            mix(0.82, 0.62, sandFactor)
        );
    } else if (n < 0.6) {
        // Grass with variation
        float grassFactor = (n - 0.3) / 0.3;  // Normalize to 0-1 range
        color = vec3(
            mix(0.2, 0.15, grassFactor),
            mix(0.6, 0.5, grassFactor),
            mix(0.3, 0.2, grassFactor)
        );
    } else if (n < 0.8) {
        // Greyish brownish green transition zone
        float transitionFactor = (n - 0.6) / 0.2;  // Normalize to 0-1 range
        color = vec3(
            mix(0.4, 0.35, transitionFactor),
            mix(0.4, 0.35, transitionFactor),
            mix(0.3, 0.25, transitionFactor)
        );
    } else {
        // Stone/mountain
        float stoneFactor = (n - 0.8) / 0.2;  // Normalize to 0-1 range
        color = vec3(
            mix(0.5, 0.4, stoneFactor),
            mix(0.5, 0.4, stoneFactor),
            mix(0.5, 0.45, stoneFactor)
        );
    }
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
    
    // Add phase shifts based on seed for more variation
    float phaseX = sin(seed * 0.1) * 3.14;
    float phaseY = cos(seed * 0.1) * 3.14;
    
    // Combine multiple sine and cosine waves with different frequencies and phases
    float noise1 = 0.5 * sin(p.x * xFreq1 + phaseX) + 0.5 * cos(p.y * yFreq1 + phaseY);
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
                sin(rotatedCoord.y * 0.5 + seed * 0.1),
                cos(rotatedCoord.x * 0.5 + seed * 0.2)
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

    vec2 _coord = adjustedCoord;

    // Calculate n using enhanced trigonometric functions
    float n = calculate_n(_coord);
    
    // Calculate terrain color
    vec3 terrainColor = simple_tile_color(_coord, n);
    vec3 finalColor = terrainColor;

    gl_FragColor = vec4(finalColor, 1.0);
}
