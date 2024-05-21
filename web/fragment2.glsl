
precision mediump float;
uniform vec2 resolution;
uniform float grid_spacing;

void cursor() {
    vec2 center = resolution * 0.5;
    float distanceFromCenter = length(gl_FragCoord.xy - center);
    float maxDistance = min(resolution.x, resolution.y) * 0.01;
    
    if (distanceFromCenter < maxDistance) {
        gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0); // Set the color output to yellow
    } else {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); // Set the color output to transparent
    }
}

void starCursor() {
    vec2 center = resolution * 0.5;
    vec2 pos = gl_FragCoord.xy - center;
    float angle = atan(pos.y, pos.x);
    float distanceFromCenter = length(pos);
    float maxDistance = min(resolution.x, resolution.y) * grid_spacing*.00001;
    float starPoints = 5.0;
    float starRadius = 0.5 + 0.5 * cos(starPoints * angle);
    
    if (distanceFromCenter < maxDistance * starRadius) {
        gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0); // Set the color output to yellow
    } else {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); // Set the color output to transparent
    }
}
void main() {
    starCursor();
}
