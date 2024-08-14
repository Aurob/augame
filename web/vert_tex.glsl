attribute vec2 position;
attribute vec2 texCoord;
varying vec2 vTexCoord;

uniform vec2 instancePosition;
uniform vec2 instanceScale;
uniform vec2 cropStart;
uniform vec2 cropSize;

void main() {
    vec2 scaledPosition = position * instanceScale;
    vec2 finalPosition = vec2(scaledPosition.x - instancePosition.x, scaledPosition.y + instancePosition.y);
    gl_Position = vec4(finalPosition, 0.0, 1.0);
    vTexCoord = cropStart + texCoord * cropSize;
}