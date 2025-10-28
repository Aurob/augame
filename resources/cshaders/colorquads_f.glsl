precision mediump float;

uniform float uSeed;
varying vec2 vPosition;
varying vec2 vWorldPos;
varying float vSeed;

// Use seed as a time-like offset, similar to getTime() in water1_f.glsl
float getTime() {
    return uSeed + 0.001;
}

void main() {
    float time = getTime();

    vec2 grid = floor(vPosition * 6.0 + 3.0);

    // Use smooth sin/cos functions for color, so color changes smoothly in space and time
    float r = 0.5 + 0.5 * sin(grid.x * 1.2 + grid.y * 0.7 + time * 1.1);
    float g = 0.5 + 0.5 * cos(grid.x * 0.9 - grid.y * 1.3 + time * 1.3);
    float b = 0.5 + 0.5 * sin(grid.x * 0.6 + grid.y * 1.8 - time * 0.9);

    // Make colors more vibrant
    vec3 color = vec3(r, g, b);
    color = color * 0.8 + 0.2; // Brighten colors

    gl_FragColor = vec4(color, 1.0);
}