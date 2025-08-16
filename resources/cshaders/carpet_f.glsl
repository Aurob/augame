precision highp float;

varying vec2 vPosition;
varying vec2 vLocalPosition;

uniform float uSeed;
uniform vec4 uColor;

// Smoother noise function using Perlin-like interpolation
float random(vec2 st) {
    vec2 s = st * 0.01;
    float a = sin(dot(s, vec2(12.9898, 78.233)));
    float b = cos(dot(s, vec2(39.3467, 11.135)));
    float c = sin(dot(s, vec2(27.157, 45.678)));
    float d = cos(dot(s, vec2(91.123, 67.890)));
    float e = sin(dot(s, vec2(14.123, 56.789)));
    float f = cos(dot(s, vec2(67.890, 12.345)));
    return smoothstep(0.0, 1.0, a * b * c * d * e * f);
}

void main() {
    // Generate noise pattern
    float noiseValue = random(vLocalPosition * 100000.0);

    // Map noise to a deep carpet red color range
    vec3 baseColor = vec3(0.5, 0.1, 0.1); // Base deep red
    vec3 highlightColor = vec3(0.7, 0.2, 0.2); // Highlight red

    // Interpolate between base and highlight colors based on noise
    vec3 carpetColor = mix(baseColor, highlightColor, noiseValue);

    gl_FragColor = vec4(carpetColor, uColor.a);
}