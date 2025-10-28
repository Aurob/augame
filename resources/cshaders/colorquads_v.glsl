precision mediump float;

attribute vec2 position;

uniform vec2 instancePosition;
uniform vec2 entityScale;
uniform float uSeed;

varying vec2 vPosition;
varying float vSeed;

void main() {
    vec2 scaledPosition = position * entityScale;
    vec2 finalPosition = vec2(scaledPosition.x - instancePosition.x, scaledPosition.y + instancePosition.y);

    // Use uSeed in a way that has no effect (prevents optimization/removal)
    finalPosition += vec2(uSeed * 0.0);

    vPosition = position;
    gl_Position = vec4(finalPosition, 0.0, 1.0);
    vSeed = uSeed;
}