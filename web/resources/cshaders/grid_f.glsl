precision mediump float;

uniform float uSeed;
varying vec2 vPosition;

void main() {
    // Simple grid pattern with seed-based variation
    vec2 grid = abs(vPosition * (10.0 + sin(uSeed) * 2.0));
    vec2 gridLines = step(0.9, mod(grid, 1.0));
    float gridMask = max(gridLines.x, gridLines.y);
    
    // Grid color varies with seed
    vec3 gridColor = vec3(
        0.8 + 0.2 * sin(uSeed),
        0.8 + 0.2 * sin(uSeed * 1.3),
        0.8 + 0.2 * sin(uSeed * 1.7)
    );
    vec3 backgroundColor = vec3(0.1, 0.1, 0.1);
    
    vec3 finalColor = mix(backgroundColor, gridColor, gridMask);
    gl_FragColor = vec4(finalColor, 1.0);
}