
precision mediump float;

attribute vec2 position;
varying vec2 v_position;
uniform vec2 u_position;
uniform float u_scale;
uniform float u_grid_spacing;

void main() {
    vec2 scaledPosition = position * u_scale;
    vec2 finalPosition = scaledPosition + u_position;
    v_position = u_position; // Declare and assign value to v_position
    gl_Position = vec4(finalPosition, 0.0, 1.0);
}