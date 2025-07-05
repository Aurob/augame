#define time_divamt 30000.0
precision mediump float;

uniform vec3 rgb;

void main() {
    // Use the rgb uniform for the color
    vec3 finalColor = rgb;
    gl_FragColor = vec4(finalColor, 1.0);
}
