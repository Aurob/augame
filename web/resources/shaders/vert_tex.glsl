attribute vec2 position;
attribute vec2 texCoord;
varying vec2 vTexCoord;

uniform vec2 instancePosition;
uniform vec2 instanceScale;
uniform vec2 cropStart;
uniform vec2 cropSize;
uniform float angle;

void main() {
    vec2 scaledPosition = position * instanceScale;
    
    // Apply rotation
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);
    vec2 rotatedPosition = vec2(
        scaledPosition.x * cosTheta - scaledPosition.y * sinTheta,
        scaledPosition.x * sinTheta + scaledPosition.y * cosTheta
    );
    
    vec2 finalPosition = vec2(rotatedPosition.x - instancePosition.x, rotatedPosition.y + instancePosition.y);
    gl_Position = vec4(finalPosition, 0.0, 1.0);
    vTexCoord = cropStart + texCoord * cropSize;
}