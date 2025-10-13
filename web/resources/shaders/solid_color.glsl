precision mediump float;
uniform vec2 offset;
uniform vec2 resolution;
uniform vec2 toplefttile;
uniform vec2 generationSize;
uniform float grid_spacing;
uniform vec4 terrain_bounds; // minX, minY, maxX, maxY (0,0,0,0 = no bounds)
uniform vec3 terrain_color; // RGB color for terrain area
uniform vec3 void_color; // RGB color for void/outside area

void main() {
    vec2 coord = gl_FragCoord.xy;
    coord.y = resolution.y - coord.y;

    // Calculate world coordinates
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

    vec3 finalColor;
    if (outOfBounds) {
        // Outside bounds - render void color
        finalColor = void_color;
    } else {
        // Inside bounds or no bounds - render terrain color
        finalColor = terrain_color;
    }

    gl_FragColor = vec4(finalColor, 1.0);
}
