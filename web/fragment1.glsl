#define time_divamt 30000.0
precision mediump float;
uniform float grid_spacing;
uniform vec2 offset;
uniform vec2 resolution;
varying vec3 color;
uniform vec2 playerPos;
uniform vec2 toplefttile;
uniform float waterMax;
uniform float sandMax;
uniform float dirtMax;
uniform float grassMax;
uniform float stoneMax;
uniform float snowMax;
uniform float time;
uniform vec2 cursorPos;
uniform float frequency;
uniform float amplitude;
uniform float persistence;
uniform float lacunarity;
uniform int octaves;

float tempn;

float getNormalizedTime(float _time) {
    return mod(_time, time_divamt) / time_divamt;
}

vec3 getLightnessFactor(vec3 color, float _time) {
    // Calculate lightness factor: simulate day and night light changes with an even fade
    float lightnessFactor = 0.1 + (sin(getNormalizedTime(_time) * 3.14159 * 2.0 - 3.14159 / 2.0) + 1.0) * 0.45;
    return vec3(lightnessFactor);
}

vec3 adjustLightness(vec3 color) {
    // Adjust color lightness based on the time of day
    return color * getLightnessFactor(color, time);
}   

vec3 makeLighter(vec3 color, float factor) {
    return color + factor;
}

vec3 tile_color(vec2 _coord, float n) {
    vec3 color;
    if (n < waterMax) {
        // Water
        vec3 deepColor = vec3(20.0 / 255.0, 24.0 / 255.0, 34.0 / 255.0); // Darker deep water color
        vec3 shallowColor = vec3(27.0 / 255.0, 57.0 / 255.0, 68.0 / 255.0); // Lighter shallow water color
        // Interpolate between deep and shallow water colors based on n
        color = mix(deepColor, shallowColor, n / waterMax);

    } else if (n < sandMax) {
        // Sand 
        vec3 lightSand = vec3(0.95, 0.87, 0.70); // Lighter sand color
        vec3 darkSand = vec3(0.70, 0.60, 0.50); // Darker sand color
        // Interpolate between light and dark sand colors based on n
        color = mix(lightSand, darkSand, (n - waterMax) / (sandMax - waterMax));
    } else if (n < dirtMax) {
        // Dirt
        vec3 lightestDirtColor = vec3(164.0 / 255.0, 158.0 / 255.0, 130.0 / 255.0); // Lightest dirt color
        vec3 darkestDirtColor = vec3(90.0 / 255.0, 80.0 / 255.0, 54.0 / 255.0); // Darkest dirt color
        // Smoothly transition between lightest and darkest dirt colors based on n
        color = mix(lightestDirtColor, darkestDirtColor, (n - sandMax) / (dirtMax - sandMax));
    } else if (n < grassMax) {
        // Grass
        vec3 darkestGrassColor = vec3(48.0 / 255.0, 71.0 / 255.0, 40.0 / 255.0); // Darkest grass color
        vec3 lightestGrassColor = vec3(93.0 / 255.0, 109.0 / 255.0, 73.0 / 255.0); // Lightest grass color
        // Smoothly transition between darkest and lightest grass colors based on n
        color = mix(darkestGrassColor, lightestGrassColor, (n - dirtMax) / (grassMax - dirtMax));
    } else if (n < stoneMax) {
        // Stone
        vec3 lightestColor = vec3(144.0 / 255.0, 144.0 / 255.0, 144.0 / 255.0); // Lightest stone color
        vec3 darkestColor = vec3(112.0 / 255.0, 112.0 / 255.0, 112.0 / 255.0); // Darkest stone color
        // Interpolate between lightest and darkest stone colors based on n
        color = mix(lightestColor, darkestColor, (n - grassMax) / (stoneMax - grassMax));
    } else {
        // Snow
        vec3 lightSnow = vec3(1.0, 1.0, 1.0); // Lighter snow color
        vec3 darkSnow = vec3(0.8, 0.8, 0.8); // Darker snow color
        // Interpolate between light and dark snow colors based on n
        color = mix(lightSnow, darkSnow, (n - stoneMax) / (snowMax - stoneMax));
    }
    return color;
}
// Improved GLSL implementation for smooth noise with scale parameter

uniform float scale; // Add scale as a uniform to control zoom level
uniform float seed; // Add seed as a uniform to influence noise generation


vec4 permute(in vec4 x) {return mod(((x * 34.0) + 1.0) * x, 289.0);}
vec4 taylorInvSqrt(in vec4 r) {return 1.79284291400159 - 0.85373472095314 * r;}

float snoise3D(vec3 v) { 

    v *= scale; // Scale the input position to control zoom level

    const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0) ;
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);

    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod(i + seed, 289.0); 
    vec4 p = permute(permute(permute(i.z + vec4(0.0, i1.z, i2.z, 1.0))
        + i.y + vec4(0.0, i1.y, i2.y, 1.0)) 
        + i.x + vec4(0.0, i1.x, i2.x, 1.0));
    
    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    float n_ = 0.142857142857; // 1.0/7.0
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,N*N)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);

    vec4 s0 = floor(b0) * 2.0 + 1.0;
    vec4 s1 = floor(b1) * 2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(a1.xy, h.z);
    vec3 p3 = vec3(a1.zw, h.w);
    
    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;
    
    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m*m, vec4(dot(p0, x0), dot(p1, x1),
                                dot(p2, x2), dot(p3, x3)));
}

#define OCTAVES 162
float fBm(float x, float y, float z, float frequency, float amplitude, 
            float persistence, float lacunarity, float scale, int octaves) {
    float total = 0.0;
    // float frequency = 1.0;
    // float amplitude = 1.0;
    float maxValue = 0.0; // Used for normalizing result to 0.0 - 1.0
    for (int i = 0; i < OCTAVES; i++) {
        if (i > octaves) break;
        // Modify x, y, z by frequency for scale adjustment
        total += snoise3D(vec3(x * frequency / scale, y * frequency / scale, z * frequency / scale)) * amplitude;
        
        maxValue += amplitude;
        
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return total / maxValue;
}

void main() {
    float line_width = 1.0; // Line width in pixels
    vec2 coord1 = gl_FragCoord.xy;
    vec2 coord = (((coord1/grid_spacing) - toplefttile) * grid_spacing) - offset;
    vec2 _coord = coord / grid_spacing;

    // subtract half the width and height of the screen to center the noise
    _coord -= resolution * 1.0 / grid_spacing;
    
    float n = fBm(_coord.x, _coord.y, seed, frequency, amplitude, persistence, lacunarity, scale, octaves);

    vec3 color = tile_color(_coord, n);

    // Add grid lines only if grid_spacing is < 8
//    if (grid_spacing < 1024.0) {
//        float grid_line_width = 0.02; // Width of the grid lines
//        vec3 grid_color = vec3(0.0); // Color of the grid lines (black)
//        vec2 grid_coord = fract(coord / grid_spacing);
//        if (grid_coord.x < grid_line_width || grid_coord.y < grid_line_width) {
//            color = vec3(1.0, 1.0, 1.0);
//        }
//    }

    gl_FragColor = vec4(color, 1.0);
    
    // if pixel in area around mouse scaled by grid_spacing, lighten the color to simulate a flashlight
    //float scaledRadius = grid_spacing * .1;
    //float distance = distance(vec2(coord1.x, resolution.y - coord1.y), cursorPos);
    //if (distance < scaledRadius) {
    //    gl_FragColor = vec4(makeLighter(gl_FragColor.rgb, 0.2), 1.0);
    //}
}