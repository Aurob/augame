precision highp float;
uniform float u_value;
out vec4 fragColor;

void main() {
    float result = u_value * 2.0;  // Example calculation
    fragColor = vec4(result, 0.0, 0.0, 1.0);
}