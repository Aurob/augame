precision mediump float;

attribute vec2 position;

uniform vec2 instancePosition;
uniform vec2 entityScale;
uniform vec2 uCenterPos;
uniform float uSeed;

varying vec2 vPosition;
varying vec2 vTerrainCoord;
varying float vSeed;

void main() {
    // Flip vertically by negating the y component of position
    vec2 flippedPosition = vec2(position.x, -position.y);
    vec2 scaledPosition = flippedPosition * entityScale;
    vec2 finalPosition = vec2(scaledPosition.x - instancePosition.x, scaledPosition.y + instancePosition.y);

    // Use uSeed in a way that has no effect (prevents optimization/removal)
    finalPosition += vec2(uSeed * 0.0);

    vPosition = flippedPosition;
    // Use center position + local position for terrain coordinate, with flipped y
    vTerrainCoord = uCenterPos + (flippedPosition * 2.0);
    gl_Position = vec4(finalPosition, 0.0, 1.0);
    vSeed = uSeed;
}