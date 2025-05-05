precision mediump float;

varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec4 uTextColor;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture2D(uTexture, vTexCoord).a);
    gl_FragColor = vec4(1.0 - uTextColor.rgb, uTextColor.a) * sampled;
}