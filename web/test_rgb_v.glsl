attribute vec2 position;

uniform vec2 instancePosition;
uniform vec2 instanceScale; // Changed from float to vec2 to accommodate width and height

void main() {
    vec2 scaledPosition = position * instanceScale; // This will now scale according to width and height
    vec2 finalPosition = vec2(scaledPosition.x - instancePosition.x, scaledPosition.y + instancePosition.y);
    
    gl_Position = vec4(finalPosition, 0.0, 1.0);
}