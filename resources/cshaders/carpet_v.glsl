attribute vec2 position;
uniform vec2 instancePosition;
uniform vec2 entityScale;

varying vec2 vPosition;
varying vec2 vLocalPosition;

void main() {
    // Scale the position by entity scale and add instance position with inverted x
    vec2 scaledPosition = position * entityScale + vec2(-instancePosition.x, instancePosition.y);
    
    // Convert to clip space coordinates (-1 to 1)
    gl_Position = vec4(scaledPosition, 0.0, 1.0);
    
    // Pass the world position to fragment shader
    vPosition = scaledPosition;
    
    // Pass local UV coordinates (0 to 1) for tiling
    vLocalPosition = (position + 1.0) * 0.5;
}