attribute vec2 position;

uniform vec2 instancePosition;
uniform vec2 entityScale;
uniform float uSeed;

varying vec2 vPosition;
varying vec2 vEntityScale;

void main() {
    vec2 scaledPosition = position * entityScale;
    vec2 finalPosition = vec2(scaledPosition.x - instancePosition.x, scaledPosition.y + instancePosition.y);

    vPosition = position;
    vEntityScale = entityScale;
    gl_Position = vec4(finalPosition, 0.0, 1.0);
}