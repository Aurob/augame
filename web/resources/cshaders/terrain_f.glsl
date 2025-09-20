precision mediump float;

uniform float uSeed;
varying vec2 vPosition;
varying vec2 vTerrainCoord;

const float frequency = 9.5;
const float amplitude = 0.70;

// Terrain color matching terrain_simple.glsl
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

// Noise function matching terrain_simple.glsl
float smoothNoise(vec2 p) {
    const float xFreq1 = 0.1;  // Primary frequency for x component
    const float yFreq1 = 0.1;  // Primary frequency for y component
    const float xFreq2 = 0.05; // Secondary frequency for x component
    const float yFreq2 = 0.07; // Secondary frequency for y component
    
    // Add phase shifts based on seed for more variation
    float phaseX = sin(uSeed * 0.1) * 3.14;
    float phaseY = cos(uSeed * 0.1) * 3.14;
    
    // Combine multiple sine and cosine waves with different frequencies and phases
    float noise1 = 0.5 * sin(p.x * xFreq1 + phaseX) + 0.5 * cos(p.y * yFreq1 + phaseY);
    float noise2 = 0.3 * sin(p.x * xFreq2 + p.y * 0.08) + 0.3 * cos(p.y * yFreq2 - p.x * 0.06);
    float noise3 = 0.2 * sin((p.x + p.y) * 0.12) * cos((p.x - p.y) * 0.09);
    
    // Combine the noise components with some non-linear operations
    float combinedNoise = noise1 + noise2 * (1.0 + 0.2 * sin(p.x * 0.3)) + noise3;
    
    // Normalize to 0.0-1.0 range
    return (combinedNoise + 1.5) * 0.33;
}

// Terrain generation matching terrain_simple.glsl
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
            vec2 warp = vec2(
                sin(rotatedCoord.y * 0.5 + uSeed * 0.1),
                cos(rotatedCoord.x * 0.5 + uSeed * 0.2)
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
    // Rotate 180 degrees to the right (around the center): flip both x and y coordinates
    vec2 rotatedCoord = vTerrainCoord;
    rotatedCoord.x = rotatedCoord.x;
    rotatedCoord.y = -rotatedCoord.y;

    float n = calculate_n(rotatedCoord);
    vec3 terrainColor = simple_tile_color(rotatedCoord, n);

    gl_FragColor = vec4(terrainColor, 1.0);
}