
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

    // Star
    float starPoints = 5.0;
    float starRadius = 0.5 + 0.5 * cos(starPoints * angle);
    
    if (distanceFromCenter < maxDistance * starRadius) {
       gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0); // Set the color output to yellow
    } else {
       gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); // Set the color output to transparent
    }
}

void rectangleCursor() {
    vec2 center = resolution * 0.5;
    vec2 pos = gl_FragCoord.xy - center;
    
    // Rectangle dimensions (half-sizes for simplicity)
    float rectWidth = 100.0; // Width of the rectangle
    float rectHeight = 50.0; // Height of the rectangle
    
    if (abs(pos.x) < rectWidth && abs(pos.y) < rectHeight) {
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); // Set the color to green
    } else {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); // Set the color to transparent
    }
}

void treeCursor() {
    vec2 center = resolution * 0.5;
    vec2 pos = gl_FragCoord.xy - center;

    // Trunk dimensions (half-sizes for simplicity)
    float trunkWidth = 20.0;
    float trunkHeight = 100.0;

    // Leaves dimensions (half-sizes for simplicity)
    float leavesWidth = 80.0;
    float leavesHeight = 100.0;

    bool isTrunk = abs(pos.x) < trunkWidth && 
                   pos.y > -trunkHeight && 
                   pos.y < 0.0;

    bool isLeaves = abs(pos.x) < leavesWidth && 
                    pos.y > 0.0 && 
                    pos.y < leavesHeight;

    if (isTrunk) {
        gl_FragColor = vec4(0.55, 0.27, 0.07, 1.0); // Brown for trunk
    } else if (isLeaves) {
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);   // Green for leaves
    } else {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);   // Transparent
    }
}

void main() {
    starCursor();
    // rectangleCursor();
    // treeCursor();
}