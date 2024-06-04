// Fragment Shader
precision mediump float;

varying vec2 v_position;

void main() {
    // if (gl_FragCoord.x < gl_FragCoord.y) {
    //     gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color
    // } else {
    //     gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0); // Blue color
    // }

    if (v_position.x < 0.5) {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color
    } else {
        gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0); // Blue color
    }
}