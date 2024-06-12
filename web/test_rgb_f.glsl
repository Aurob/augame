precision mediump float;
uniform vec4 uColor; // Added RGB uniform for color

void main() {
    gl_FragColor = uColor; // Use the RGB uniform for color
}