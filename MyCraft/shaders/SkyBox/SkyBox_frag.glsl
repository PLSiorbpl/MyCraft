#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform vec3 sunDir;
uniform float dayfactor;
uniform mat4 invView;
uniform mat4 invProj;
uniform float Time; // seconds elapsed

const vec3 SUN_COLOR    = vec3(1.0, 0.9, 0.6);
const vec3 SUNSET_COLOR = vec3(1.0, 0.42, 0.15);
const vec3 SKY_TOP      = vec3(0.15, 0.35, 0.75);
const vec3 SKY_BOTTOM   = vec3(0.6, 0.75, 1.0);
const vec3 NIGHT_SKY    = vec3(0.02, 0.03, 0.08);
const vec3 MOON_COLOR   = vec3(0.85, 0.9, 1.0);
const vec3 MOONSET_COLOR = vec3(0.694, 0.878, 0.835);
const vec3 STAR_COLOR_1 = vec3(0.7,0.8,1.0);
const vec3 STAR_COLOR_2 = vec3(1.0, 0.643, 0.827);

const float ROUGHNESS = 0.15;
const float DIFFUSE_MULT = 1.0;

const float FOG_END = 1.0;

const float PI = 3.14159265;
const float two_PI = 2.0*3.14159265;

// ----------------------------
// Stars
// ----------------------------
float hash(vec3 p3) {
    p3 = fract(p3 * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

vec3 starField(vec3 dir) {
    vec3 p = dir * 300.0;
    vec3 cell = floor(p);
    vec3 local = fract(p) - 0.5;

    float h = hash(cell);
    float star = smoothstep(0.98, 1.0, h); // how many
    star *= smoothstep(0.7, 0.0, length(local)); // Size

    float phase = hash(cell + 17.0);
    float speed = 1.0 + hash(cell + 91.0) * 2.0; // 1.0–4.0
    float twinkle = 0.6 + 0.4 * sin(Time * speed + phase * two_PI);

    star *= twinkle;
    vec3 starColor = mix(STAR_COLOR_1, STAR_COLOR_2, hash(cell + 5.0));

    return starColor * star;
}

vec3 stylize(vec3 c) {
    float lum = dot(c, vec3(0.2126, 0.7152, 0.0722)); // BT.709 weights or smt
    c = mix(vec3(lum), c, 1.35); // saturation boost
    c = mix(c, c * vec3(0.85, 0.9, 1.15), 1.0 - lum); // cold shadows
    c = mix(c, c * vec3(1.1, 1.0, 0.85), lum); // warm highlights
    return c;
}

void main() {
    // ----------------------------
    // Ray dir
    // ----------------------------
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 clip = vec4(ndc, 1.0, 1.0);

    vec4 view = invProj * clip;
    view = vec4(view.xyz, 0.0);

    vec3 rayDir = normalize((invView * view).xyz);
    vec3 L = normalize(sunDir);

    float sunHeight = L.y;
    vec3 moonDir = -L;

    float sunVisibility  = smoothstep(-0.4, 0.0, sunHeight);
    float moonVisibility = smoothstep(-0.4, 0.0, moonDir.y);

    float horizonFactor = 1.0 - smoothstep(0.0, 0.4, abs(sunHeight));
    float nightFactor = smoothstep(0.6, 0.0, dayfactor);

    // ----------------------------
    // Sky gradient
    // ----------------------------
    float t = max(rayDir.y, 0.0);
    vec3 sky = mix(SKY_BOTTOM, SKY_TOP, t);
    sky = mix(NIGHT_SKY, sky, dayfactor);

    float viewNearHorizon = 1.0 - abs(rayDir.y);
    vec2 sunDirXZ  = normalize(L.xz + vec2(1e-4));
    vec2 viewDirXZ = normalize(rayDir.xz + vec2(1e-4));
    float azimuthAlign = dot(sunDirXZ, viewDirXZ) * 0.5 + 0.5;
    float sunsetGlow = horizonFactor * viewNearHorizon * mix(0.3, 1.0, azimuthAlign);
    sky = mix(sky, SUNSET_COLOR, sunsetGlow * 0.7);

    // ----------------------------
    // Stars
    // ----------------------------
    float altitudeFade = smoothstep(-0.06, 0.3, rayDir.y);
    float starVisibility = max(nightFactor, horizonFactor * 0.4) * altitudeFade;
    if (starVisibility > 0.001) {
        vec3 star = starField(rayDir);
        sky += star * starVisibility;
    }

    // ----------------------------
    // Moon
    // ----------------------------
    float dm = dot(rayDir, moonDir);
    float moonDisc = smoothstep(0.9997, 0.9999, dm) * moonVisibility;
    float moonGlow = pow(max(dm, 0.0), 50.0) * moonVisibility;

    vec3 dynamicMoonColor = mix(MOON_COLOR, MOONSET_COLOR, horizonFactor);
    sky += dynamicMoonColor * moonDisc * 1.0; // disc
    sky += dynamicMoonColor * moonGlow * 0.5; // aura

    // ----------------------------
    // Sun
    // ----------------------------
    float d = dot(rayDir, L);
    float sun  = smoothstep(0.9991, 1.0, d) * sunVisibility;
    float glow = pow(max(d, 0.0), 20.0) * sunVisibility;

    vec3 dynamicSunColor = mix(SUN_COLOR, SUNSET_COLOR, horizonFactor);
    sky += dynamicSunColor * sun * 1.0;  // disc
    sky += dynamicSunColor * glow * 0.5;  // aura

    float dither = (hash(vec3(gl_FragCoord.xy, 0.0)) - 0.5) / 255.0;
    sky += dither;
    sky = stylize(sky);

    FragColor = vec4(sky, 1.0);
}
