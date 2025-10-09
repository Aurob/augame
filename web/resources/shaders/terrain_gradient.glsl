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
uniform vec4 terrain_bounds; // minX, minY, maxX, maxY (0,0,0,0 = no bounds)

const float frequency = 9.5;
const float amplitude = 0.70;

// Hardcoded drawgrid variable
const bool drawgrid = false;

// Planet curvature parameters
const float PLANET_TRANSITION_START = 0.8; // When curvature starts to appear
const float PLANET_FULL_VIEW = 0.2;        // When full planet view is active
const float PLANET_RADIUS = 0.45;          // Normalized radius in screen space (0.5 = half screen)
const float ATMOSPHERE_FADE = 0.15;        // How much atmosphere fading at edges

vec3 simple_tile_color(vec2 _coord, float n) {
    vec3 color;
    if (n < 0.1) {
        // Water - slightly lighter ocean
        float depth = 0.1 - n;
        float depthFactor = depth / 0.1;
        color = vec3(
            mix(0.12, 0.08, depthFactor),
            mix(0.16, 0.12, depthFactor),
            mix(0.24, 0.20, depthFactor)
        );
    } else if (n < 0.3) {
        // Sand
        float sandFactor = (n - 0.1) / 0.2;
        sandFactor = pow(sandFactor, 2.5);
        color = vec3(
            mix(0.96, 0.82, sandFactor),
            mix(0.94, 0.76, sandFactor),
            mix(0.82, 0.62, sandFactor)
        );
    } else if (n < 0.6) {
        // Grass
        float grassFactor = (n - 0.3) / 0.3;
        color = vec3(
            mix(0.2, 0.15, grassFactor),
            mix(0.6, 0.5, grassFactor),
            mix(0.3, 0.2, grassFactor)
        );
    } else if (n < 0.8) {
        // Transition zone
        float transitionFactor = (n - 0.6) / 0.2;
        color = vec3(
            mix(0.4, 0.35, transitionFactor),
            mix(0.4, 0.35, transitionFactor),
            mix(0.3, 0.25, transitionFactor)
        );
    } else {
        // Stone/mountain
        float stoneFactor = (n - 0.8) / 0.2;
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
    const float xFreq1 = 0.1;
    const float yFreq1 = 0.1;
    const float xFreq2 = 0.05;
    const float yFreq2 = 0.07;
    
    float noise1 = 0.5 * sin(p.x * xFreq1) + 0.5 * cos(p.y * yFreq1);
    float noise2 = 0.3 * sin(p.x * xFreq2 + p.y * 0.08) + 0.3 * cos(p.y * yFreq2 - p.x * 0.06);
    float noise3 = 0.2 * sin((p.x + p.y) * 0.12) * cos((p.x - p.y) * 0.09);
    
    float combinedNoise = noise1 + noise2 * (1.0 + 0.2 * sin(p.x * 0.3)) + noise3;
    
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
            vec2 warp = vec2(
                sin(rotatedCoord.y * 0.5 + float(i) * 0.1),
                cos(rotatedCoord.x * 0.5 + float(i) * 0.2)
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

// --- New helpers for sphere mapping ---

vec3 screenToSphereNormal(vec2 screenPos, float planetRadiusNorm, out bool onSphere, out float r01) {
    // screenPos in [0,1]; planetRadiusNorm = PLANET_RADIUS
    vec2 centered = (screenPos - 0.5) / planetRadiusNorm;  // edge at length=1
    float r2 = dot(centered, centered);
    onSphere = (r2 <= 1.0);
    r01 = sqrt(max(r2, 0.0));
    if (!onSphere) return vec3(0.0);
    float z = sqrt(max(1.0 - r2, 0.0));
    return normalize(vec3(centered.x, centered.y, z));
}

// Convert unit normal to lon/lat (radians)
vec2 normalToLonLat(vec3 n) {
    float lon = atan(n.x, n.z); // [-pi, pi]
    float lat = asin(clamp(n.y, -1.0, 1.0)); // [-pi/2, pi/2]
    return vec2(lon, lat);
}

// Add simple atmosphere/edge effect
vec3 addAtmosphere(vec3 color, vec2 screenPos, float planetness) {
    vec2 centered = (screenPos - 0.5) * 2.0;
    float dist = length(centered);
    float edge = smoothstep(PLANET_RADIUS * 0.95, PLANET_RADIUS, dist);
    float atmosphere = pow(edge, 1.5) * planetness;
    vec3 atmosphereColor = vec3(0.4, 0.6, 1.0) * atmosphere * 0.3;
    return mix(color, color + atmosphereColor, atmosphere);
}

void main() {
    vec2 coord = gl_FragCoord.xy;
    coord.y = resolution.y - coord.y;

    float planetness = 0.0;
    if (grid_spacing < PLANET_TRANSITION_START) {
        planetness = smoothstep(PLANET_TRANSITION_START, PLANET_FULL_VIEW, grid_spacing);
    }

    vec2 screenPos = coord / resolution;

    // Background + planet mask
    if (planetness > 0.01) {
        vec2 centered = (screenPos - 0.5) * 2.0;
        float dist = length(centered);
        float planetEdge = PLANET_RADIUS * (1.0 + planetness * 0.2);
        if (dist > planetEdge) {
            vec3 spaceColor = vec3(0.02, 0.02, 0.05);
            float starNoise = smoothNoise(coord * 0.01);
            if (starNoise > 0.98) spaceColor += vec3(0.8);
            gl_FragColor = vec4(spaceColor, 1.0);
            return;
        }
    }

    // Planar world coords (what you had)
    vec2 generationOffset = vec2(generationSize.x / 2.0, generationSize.y / 2.0);
    vec2 planarCoord = (coord / grid_spacing) + toplefttile + (offset / grid_spacing) + generationOffset;

    // Sphere world coords (new) – sample world on a sphere, anchored at playerPos
    vec2 sphereCoord = planarCoord; // fallback
    if (planetness > 0.01) {
        bool onSphere; float r01;
        vec3 n = screenToSphereNormal(screenPos, PLANET_RADIUS, onSphere, r01);

        // Freeze world-per-radian when planet mode starts so the pattern stays static as you zoom out
        float pxRadius = PLANET_RADIUS * min(resolution.x, resolution.y);
        float worldPerRad = pxRadius / PLANET_TRANSITION_START; // tiles (or world units) per radian

        // Center of the sphere in world space: anchor to player
        // Make sure playerPos is in the same units as your noise/world (tile coords).
        vec2 worldCenter = playerPos;

        vec2 lonlat = normalToLonLat(n); // radians
        sphereCoord = worldCenter + lonlat * worldPerRad;
    }

    // Blend during transition (keeps your nice "curvature reveal" but fixes the lensing)
    vec2 sampleCoord = mix(planarCoord, sphereCoord, clamp(planetness, 0.0, 1.0));

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

    // Atmosphere/lighting still based on screen-space distance
    if (planetness > 0.01) {
        terrainColor = addAtmosphere(terrainColor, screenPos, planetness);
        vec2 centered = (screenPos - 0.5) * 2.0;
        float dist = length(centered);
        float edgeDarkening = 1.0 - (dist * dist * planetness * 0.4);
        terrainColor *= edgeDarkening;
        vec2 lightDir2D = normalize(vec2(-0.5, -0.5));
        float lighting = max(0.0, dot(normalize(centered), lightDir2D));
        terrainColor += vec3(0.1) * lighting * planetness;
    }

    vec3 finalColor = terrainColor;

    // Optional grid (still off in planet view)
    if (drawgrid && planetness < 0.5) {
        vec2 worldGridPos = mod(sampleCoord, 1.0);
        float worldGridThickness = 1.0 / grid_spacing;
        float lineX = step(worldGridPos.x, worldGridThickness) + step(1.0 - worldGridPos.x, worldGridThickness);
        float lineY = step(worldGridPos.y, worldGridThickness) + step(1.0 - worldGridPos.y, worldGridThickness);
        float gridLine = clamp(lineX + lineY, 0.0, 1.0);
        vec3 gridColor = vec3(0.85);
        finalColor = mix(finalColor, gridColor, gridLine * 0.7 * (1.0 - planetness));
    }

    gl_FragColor = vec4(finalColor, 1.0);
}