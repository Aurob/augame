precision mediump float;

uniform float uSeed;
varying vec2 vPosition;
varying vec2 vWorldPos;

// Improved hash function for better noise quality
vec2 hash22(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)),
             dot(p, vec2(269.5, 183.3)));
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

// Simplex-style noise for better performance and quality
float noise(vec2 p) {
    const float K1 = 0.366025404; // (sqrt(3)-1)/2
    const float K2 = 0.211324865; // (3-sqrt(3))/6
    
    vec2 i = floor(p + (p.x + p.y) * K1);
    vec2 a = p - i + (i.x + i.y) * K2;
    vec2 o = (a.x > a.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec2 b = a - o + K2;
    vec2 c = a - 1.0 + 2.0 * K2;
    
    vec3 h = max(0.5 - vec3(dot(a,a), dot(b,b), dot(c,c)), 0.0);
    vec3 n = h * h * h * h * vec3(dot(a, hash22(i + 0.0)), dot(b, hash22(i + o)), dot(c, hash22(i + 1.0)));
    
    return dot(n, vec3(70.0));
}

// Fractal Brownian Motion for more natural patterns
float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 4.5;
    float frequency = .20;
    
    for (int i = 0; i < 4; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 22.0;
    }
    
    return value;
}

// Time-based animation
float getTime() {
    return uSeed * 0.01; // Use seed as time if uTime not available
}

// Gerstner wave function for realistic ocean waves
vec2 gerstnerWave(vec2 pos, vec2 direction, float wavelength, float amplitude, float speed, float time, float steepness) {
    float k = 2.0 * 3.14159 / wavelength;
    float phase = speed * k;
    float qi = steepness / (k * amplitude);
    
    float f = k * dot(direction, pos) + phase * time;
    float sinF = sin(f);
    float cosF = cos(f);
    
    return vec2(
        qi * amplitude * direction.x * cosF,
        qi * amplitude * direction.y * cosF
    );
}

// Calm sea with slow rolling waves in specified direction
vec2 waterDisplacement(vec2 pos, float time) {
    vec2 displacement = vec2(0.0);
    
    // Get primary wave direction from uniform
    vec2 primaryDir = vec2(cos(3.14159), sin(3.14159));
    
    // Create slight variations of the main direction for natural look
    vec2 dir1 = normalize(primaryDir + vec2(0.1, 0.0));
    vec2 dir2 = normalize(primaryDir + vec2(-0.05, 0.1));
    vec2 dir3 = normalize(primaryDir + vec2(0.0, -0.08));
    
    // Primary slow rolling waves - much calmer
    displacement += gerstnerWave(pos, dir1, 8.0, 0.04, 0.6, time, 0.3);
    displacement += gerstnerWave(pos, dir2, 6.5, 0.03, 0.5, time, 0.25);
    displacement += gerstnerWave(pos, dir3, 5.0, 0.025, 0.7, time, 0.2);
    
    // Very subtle secondary waves
    displacement += gerstnerWave(pos, primaryDir, 3.0, 0.015, 0.8, time, 0.15);
    
    // Minimal turbulence for calm water
    displacement += vec2(fbm(pos * 1.5 + time * 0.1)) * 0.008;
    
    return displacement;
}

// Calculate normal from displacement for lighting
vec3 calculateNormal(vec2 pos, float time) {
    float epsilon = 0.01;
    vec2 d1 = waterDisplacement(pos + vec2(epsilon, 0.0), time);
    vec2 d2 = waterDisplacement(pos - vec2(epsilon, 0.0), time);
    vec2 d3 = waterDisplacement(pos + vec2(0.0, epsilon), time);
    vec2 d4 = waterDisplacement(pos - vec2(0.0, epsilon), time);
    
    vec3 dx = vec3(2.0 * epsilon, 0.0, d1.x - d2.x);
    vec3 dy = vec3(0.0, 2.0 * epsilon, d3.y - d4.y);
    
    return normalize(cross(dx, dy));
}

// Fresnel effect for realistic water reflectance
float fresnel(vec3 normal, vec3 viewDir, float f0) {
    float cosTheta = max(dot(normal, viewDir), 0.0);
    return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

// Calm water color without foam
vec3 waterColor(vec2 pos, vec3 normal, float time) {
    // Calm sea colors
    vec3 deepColor = vec3(0.05, 0.12, 0.25);
    vec3 shallowColor = vec3(0.15, 0.4, 0.65);
    
    // Gentle depth variation
    float depth = 0.6 + 0.2 * fbm(pos * 0.3 + time * 0.05);
    depth = clamp(depth, 0.0, 1.0);
    
    // No foam for calm water
    return mix(deepColor, shallowColor, depth);
}

// Simplified lighting for calm sea
vec3 calculateLighting(vec3 color, vec3 normal, vec2 pos, float time) {
    // Soft light direction for calm conditions
    vec3 lightDir = normalize(vec3(0.3, 0.6, 0.8));
    
    // Gentle diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0) * 0.6 + 0.4;
    
    // Subtle specular highlights
    vec3 viewDir = vec3(0.0, 0.0, 1.0);
    vec3 halfDir = normalize(lightDir + viewDir);
    float specular = pow(max(dot(normal, halfDir), 0.0), 32.0) * 0.3;
    
    // Soft ambient lighting
    vec3 ambient = vec3(0.15, 0.2, 0.25);
    vec3 diffuseColor = color * diffuse;
    vec3 specularColor = vec3(0.9, 0.95, 1.0) * specular;
    
    // Very subtle caustics for calm water
    float caustics = max(0.0, sin(pos.x * 8.0 + time * 1.5) * sin(pos.y * 6.0 + time * 1.2));
    caustics *= noise(pos * 3.0 + time * 0.2) * 0.5 + 0.5;
    caustics = pow(caustics, 4.0) * 0.1;
    
    return ambient + diffuseColor + specularColor + vec3(caustics);
}

// Sub-surface scattering approximation
vec3 subsurfaceScattering(vec3 color, vec3 normal, vec2 pos, float time) {
    vec3 lightDir = normalize(vec3(0.5, 0.8, 0.6));
    
    // Light penetration through water surface
    float penetration = max(0.0, -dot(normal, lightDir));
    vec3 scatterColor = vec3(0.1, 0.4, 0.6) * penetration * 0.3;
    
    // Add some variation with noise
    float scatterNoise = noise(pos * 4.0 + time * 0.2) * 0.5 + 0.5;
    scatterColor *= scatterNoise;
    
    return color + scatterColor;
}

void main() {
    float time = getTime();
    
    // Scale position for water simulation
    vec2 waterPos = vPosition * 2.0;
    
    // Get water surface displacement
    vec2 displacement = waterDisplacement(waterPos, time);
    vec2 animatedPos = waterPos + displacement;
    
    // Calculate surface normal for lighting
    vec3 normal = calculateNormal(waterPos, time);
    
    // Get base water color
    vec3 color = waterColor(animatedPos, normal, time);
    
    // Apply realistic lighting
    color = calculateLighting(color, normal, animatedPos, time);
    
    // Add subsurface scattering
    color = subsurfaceScattering(color, normal, animatedPos, time);
    
    // Dynamic transparency based on viewing angle and depth
    float fresnelAlpha = fresnel(normal, vec3(0.0, 0.0, 1.0), 0.1);
    float depthAlpha = 0.85 + 0.1 * fbm(animatedPos * 2.0 + time * 0.1);
    float alpha = mix(depthAlpha, 0.95, fresnelAlpha);
    alpha = clamp(alpha, 0.7, 0.95);
    
    // Color grading for more realistic appearance
    color = pow(color, vec3(0.9)); // Slight gamma correction
    color = mix(color, vec3(dot(color, vec3(0.299, 0.587, 0.114))), -0.1); // Slight saturation boost
    
    gl_FragColor = vec4(color, alpha);
}