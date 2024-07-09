precision mediump float;
uniform vec2 resolution;
uniform float grid_spacing;
uniform vec2 toplefttile;
uniform vec2 cursorPos;

void main() {
    vec2 coord = gl_FragCoord.xy;

    // Invert the y-coordinate
    coord.y = resolution.y - coord.y;

    // Adjust the coordinates with grid spacing and toplefttile
    vec2 adjustedCoord = (coord / grid_spacing) + toplefttile;

    // Add cursor indicator
    vec2 cursorWorldPos = cursorPos / grid_spacing + toplefttile;
    float cursorRadius = 5.0 / grid_spacing; // Adjust the size as needed
    float distToCursor = length(adjustedCoord - cursorWorldPos);
    
    vec3 finalColor = vec3(0.0, 0.0, 0.0); // Default to transparent black
    
    if (distToCursor < cursorRadius) {
        float t = smoothstep(cursorRadius, cursorRadius - 1.0 / grid_spacing, distToCursor);
        finalColor = mix(finalColor, vec3(1.0, 0.0, 0.0), t * 0.7); // Red cursor with 70% opacity
    }

    gl_FragColor = vec4(finalColor, finalColor == vec3(0.0) ? 0.0 : 1.0);
}
