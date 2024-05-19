precision mediump float;
uniform vec2 resolution;
void main() {
    vec2 center = resolution * 0.5;
    float distanceFromCenter = length(gl_FragCoord.xy - center);
    float maxDistance = min(resolution.x, resolution.y) * 0.01;
    
    if (distanceFromCenter < maxDistance) {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Set the color output to red
    } else {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); // Set the color output to transparent
    }
}