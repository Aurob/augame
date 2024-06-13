attribute vec2 position;

uniform vec2 instancePosition;
uniform float instanceScale;

void main() {
    vec2 scaledPosition = position * instanceScale;
    vec2 finalPosition = scaledPosition + instancePosition;
    gl_Position = vec4(finalPosition, 0.0, 1.0);
}