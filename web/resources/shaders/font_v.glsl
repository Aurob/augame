attribute vec2 aPosition;
attribute vec2 aTexCoord;

varying vec2 vTexCoord;

uniform vec2 charPosition;
uniform vec2 charScale;
uniform float angle; // Added uniform for angle

void main() {
    vec2 scaledPosition = aPosition * charScale;
    
    // Apply angle
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);
    vec2 rotatedPosition = vec2(
        scaledPosition.x * cosTheta - scaledPosition.y * sinTheta,
        scaledPosition.x * sinTheta + scaledPosition.y * cosTheta
    );
    
    vec2 finalPosition = vec2(rotatedPosition.x - charPosition.x, rotatedPosition.y + charPosition.y);
    
    gl_Position = vec4(finalPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
}