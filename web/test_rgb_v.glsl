attribute vec2 position;

uniform vec2 instancePosition;
uniform vec2 entityScale;
uniform float angle; // Added uniform for angle

void main() {
    vec2 scaledPosition = position * entityScale;
    
    // Apply angle
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);
    vec2 rotatedPosition = vec2(
        scaledPosition.x * cosTheta - scaledPosition.y * sinTheta,
        scaledPosition.x * sinTheta + scaledPosition.y * cosTheta
    );
    
    vec2 finalPosition = vec2(rotatedPosition.x - instancePosition.x, rotatedPosition.y + instancePosition.y);
    
    gl_Position = vec4(finalPosition, 0.0, 1.0);
}