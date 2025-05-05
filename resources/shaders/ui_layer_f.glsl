precision mediump float;

varying vec2 vTexCoord;
uniform sampler2D textTexture;

void main() {
    vec4 sampled = texture2D(textTexture, vTexCoord);
    gl_FragColor = vec4(sampled.rgb, sampled.a);
}