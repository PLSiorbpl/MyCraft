#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform vec3 sunDir;
uniform float dayfactor;
uniform mat4 invView;
uniform mat4 invProj;
uniform float Time; // seconds elapsed
uniform vec3 CameraPos;

// ------------------------------------------------------------------
// Palette (kept in sync with SkyBox_frag.glsl)
// ------------------------------------------------------------------
const vec3 SUN_COLOR     = vec3(1.0, 0.9, 0.6);
const vec3 SUNSET_COLOR  = vec3(1.0, 0.42, 0.15);
const vec3 SKY_TOP       = vec3(0.15, 0.35, 0.75);
const vec3 SKY_BOTTOM    = vec3(0.6, 0.75, 1.0);
const vec3 NIGHT_SKY     = vec3(0.02, 0.03, 0.08);
const vec3 MOON_COLOR    = vec3(0.85, 0.9, 1.0);
const vec3 MOONSET_COLOR = vec3(0.694, 0.878, 0.835);

// ------------------------------------------------------------------
// Cloud shape
// ------------------------------------------------------------------
const float cloudBottom  = 120.0;
const float cloudTop     = 200.0;
const int   STEPS        = 48;      // primary raymarch samples
const int   LIGHT_STEPS  = 4;       // samples marched toward the light
const int   OCTAVES      = 3;       // full detail fbm octaves
const float COVERAGE     = 0.48;    // 0..1, higher = more clouds
const float MAX_DISTANCE = 2600.0;  // march clamp (matches the distance fade)

// ------------------------------------------------------------------
// Lighting (tune to taste)
// ------------------------------------------------------------------
const float PI               = 3.14159265;
const float DENSITY_SCALE    = 0.16;  // extinction per unit density * step length
const float LIGHT_ABSORPTION = 0.10;  // how dark shaded cloud bases get
const float POWDER_STRENGTH  = 0.455;  // darkening of thin sunlit edges
const float SCATTER_STRENGTH = 2.6;   // silver lining strength
const float PHASE_AMBIENT    = 0.18;  // isotropic scattering floor
const float PHASE_FORWARD_G  = 0.72;  // forward scattering lobe (toward the light)
const float PHASE_BACKWARD_G = -0.10; // backward scattering lobe
const float PHASE_MIX        = 0.55;  // backward lobe blend

vec3 L = normalize(sunDir);

// Lighting environment for this frame, filled in main()
vec3 g_lightDir;      // toward the dominant light (sun by day, moon by night)
vec3 g_lightColor;    // its color * intensity
vec3 g_ambientTop;    // sky ambient arriving on cloud tops
vec3 g_ambientBottom; // sky ambient arriving on cloud bases

float hash(vec3 p) {
    p = fract(p * 0.3183099 + 0.1);
    p *= 17.0;
    return fract(p.x*p.y*p.z * (p.x+p.y+p.z));
}

float noise(const vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);

    return mix(mix(mix(hash(p+vec3(0,0,0)), hash(p+vec3(1,0,0)),f.x),
                   mix(hash(p+vec3(0,1,0)), hash(p+vec3(1,1,0)),f.x),f.y),
               mix(mix(hash(p+vec3(0,0,1)), hash(p+vec3(1,0,1)),f.x),
                   mix(hash(p+vec3(0,1,1)), hash(p+vec3(1,1,1)),f.x),f.y),f.z);
}

float fbm(vec3 p, const int octaves) {
    float f = 0.0;
    float weight = 0.5;
    for(int i = 0; i < octaves; ++i) {
        f += weight * noise(p);
        weight *= 0.5;
        p *= 2.0;
    }
    return f;
}

float remap(float x, float a, float b, float c, float d) {
    return c + (x - a) / (b - a) * (d - c);
}

// Density fades out at the bottom and the top of the slab:
// flatter cloud bases, billowy rounded tops
float heightGradient(float h) {
    return smoothstep(0.0, 0.12, h) * (1.0 - smoothstep(0.45, 1.0, h));
}

// Cloud density field (0..1). 'octaves' lets the light march run a cheaper LOD.
float getDensity(vec3 p, const int octaves) {
    float h = clamp((p.y - cloudBottom) / (cloudTop - cloudBottom), 0.0, 1.0);

    p *= vec3(0.008, 0.015, 0.008);
    vec3 wind = vec3(0.4, -0.15, 1.0) * Time * 0.05;
    vec3 q = p + wind;

    float f = fbm(q, octaves);

    // Large scale weather systems break the layer into patchy clusters
    float weather = noise(q * 0.06 + vec3(37.0, 3.7, 11.0));
    float patchy  = mix(0.55, 1.25, smoothstep(0.35, 0.7, weather));

    float shape = clamp(remap(f, 1.0 - COVERAGE, 0.9, 0.0, 1.0), 0.0, 1.0);
    return clamp(shape * patchy * heightGradient(h), 0.0, 1.0);
}

// Two lobe Henyey-Greenstein phase: strong forward scattering (silver linings)
float hgPhase(float cosTheta, float g) {
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * PI * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

float phase(float cosTheta) {
    float hg = mix(hgPhase(cosTheta, PHASE_FORWARD_G),
                   hgPhase(cosTheta, PHASE_BACKWARD_G), PHASE_MIX);
    return PHASE_AMBIENT + hg * SCATTER_STRENGTH;
}

vec2 rayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 invRaydir) {
    vec3 t0 = (boundsMin - rayOrigin) * invRaydir;
    vec3 t1 = (boundsMax - rayOrigin) * invRaydir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);

    float dstA = max(max(tmin.x, tmin.y), tmin.z);
    float dstB = min(tmax.x, min(tmax.y, tmax.z));

    float dstToBox = max(0, dstA);
    float dstInsideBox = max(0, dstB - dstToBox);
    return vec2(dstToBox, dstInsideBox);
}

// Transmittance toward the light through the cloud slab (1.0 = fully lit)
float lightMarch(vec3 pos) {
    // epsilon avoids 0 * inf = NaN when a component of the light dir is zero
    vec3 invLightDir = 1.0 / (g_lightDir + vec3(1e-5));

    float dstInsideBox = rayBoxDst(vec3(0.0, cloudBottom, 0.0),
                                   vec3(0.0, cloudTop, 0.0), pos, invLightDir).y;
    dstInsideBox = min(dstInsideBox, (cloudTop - cloudBottom) * 2.0); // grazing light
    if (dstInsideBox <= 0.0)
        return 1.0;

    float stepSize = dstInsideBox / float(LIGHT_STEPS);
    float totalDensity = 0.0;

    for (int i = 0; i < LIGHT_STEPS; i++) {
        pos += g_lightDir * stepSize;
        totalDensity += getDensity(pos, 2) * stepSize;
    }

    return exp(-totalDensity * LIGHT_ABSORPTION);
}

// Final lit color of a single cloud sample
vec3 cloudLighting(vec3 pos, float density, float h, float cosView) {
    float lightT = lightMarch(pos);

    // Beer-powder: thin sunlit edges darken slightly before the bright rim
    float powder = 1.0 - exp(-density * 4.0);
    vec3 direct = g_lightColor * (lightT * phase(cosView));
    direct *= mix(1.0, clamp(powder * 2.0, 0.0, 1.0), POWDER_STRENGTH);

    // Sky ambient: brighter on tops, darker on cloud bases
    vec3 ambient = mix(g_ambientBottom, g_ambientTop, h);

    return ambient + direct;
}

// Gentle grading so clouds match the stylized skybox
vec3 stylizeClouds(vec3 c) {
    float lum = dot(c, vec3(0.2126, 0.7152, 0.0722));
    c = mix(vec3(lum), c, 1.15);                               // saturation
    c = mix(c, c * vec3(0.90, 0.95, 1.10), (1.0 - lum) * 0.6); // cool shadows
    c = mix(c, c * vec3(1.08, 1.00, 0.90), lum * 0.6);         // warm highlights
    return c;
}

// Front-to-back energy conserving integration.
// rgb = premultiplied scattered light, a = total opacity
vec4 rayMarch(const float start, const float end, const vec3 rayDir, const float jitter, const float cosView) {
    float stepSize = (end - start) / float(STEPS);

    vec3 scattered = vec3(0.0);
    float transmittance = 1.0;
    float t = start + stepSize * jitter; // jittered start kills banding

    for (int i = 0; i < STEPS; i++) {
        if (t >= end || transmittance < 0.01)
            break;

        float distanceFade = 1.0 - smoothstep(0.0, 2500.0, t);
        vec3 p = CameraPos + rayDir * t;

        float density = getDensity(p, OCTAVES) * distanceFade;

        if (density > 0.001) {
            float h = clamp((p.y - cloudBottom) / (cloudTop - cloudBottom), 0.0, 1.0);
            vec3 color = cloudLighting(p, density, h, cosView);

            float alpha = 1.0 - exp(-density * stepSize * DENSITY_SCALE);
            scattered += color * alpha * transmittance;
            transmittance *= 1.0 - alpha;
        }

        t += stepSize;
    }

    return vec4(scattered, 1.0 - transmittance);
}



void main() {
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 clip = vec4(ndc, 1.0, 1.0);

    vec4 view = invProj * clip;
    view /= view.w;

    vec3 rayDir = normalize((invView * vec4(view.xyz, 0.0)).xyz);

    // ------------------------------------------------------------------
    // Lighting environment for this frame
    // ------------------------------------------------------------------
    float dayW = smoothstep(-0.1, 0.15, L.y); // sun <-> moon crossfade
    float horizonFactor = 1.0 - smoothstep(0.0, 0.4, abs(L.y));

    g_lightDir = L.y >= 0.0 ? L : -L; // sun by day, moon by night

    vec3 sunLight  = mix(SUN_COLOR, SUNSET_COLOR, horizonFactor)
                   * mix(0.55, 1.15, smoothstep(0.0, 0.3, L.y));
    vec3 moonLight = mix(MOON_COLOR, MOONSET_COLOR, horizonFactor) * 0.22;
    g_lightColor = mix(moonLight, sunLight, dayW);

    vec3 dayAmbTop      = mix(SKY_TOP, SKY_BOTTOM, 0.55) * 0.90;
    vec3 dayAmbBottom   = mix(SKY_TOP, SKY_BOTTOM, 0.25) * 0.55;
    vec3 nightAmbTop    = NIGHT_SKY * 2.5 + MOON_COLOR * 0.04;
    vec3 nightAmbBottom = NIGHT_SKY * 1.2;

    g_ambientTop    = mix(nightAmbTop,    dayAmbTop,    dayfactor) + SUNSET_COLOR * horizonFactor * 0.22;
    g_ambientBottom = mix(nightAmbBottom, dayAmbBottom, dayfactor) + SUNSET_COLOR * horizonFactor * 0.10;

    float cosView = dot(rayDir, g_lightDir);

    // ------------------------------------------------------------------
    // Cloud slab intersection
    // ------------------------------------------------------------------
    float ry = abs(rayDir.y) < 1e-4 ? 1e-4 : rayDir.y; // avoid /0 at the exact horizon
    float t0 = (cloudBottom - CameraPos.y) / ry;
    float t1 = (cloudTop - CameraPos.y) / ry;

    float start = max(min(t0, t1), 0.0);
    float end = max(t0, t1);
    end = min(end, start + MAX_DISTANCE);

    if (end <= start)
        discard;

    // ------------------------------------------------------------------
    // March
    // ------------------------------------------------------------------
    float jitter = hash(vec3(gl_FragCoord.xy, 0.0));
    vec4 sum = rayMarch(start, end, rayDir, jitter, cosView);

    if (sum.w <= 0.0)
        discard;

    // Convert premultiplied result to straight alpha for the fixed function
    // blend (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    vec3 color = sum.rgb / max(sum.w, 1e-4);
    color = min(color, vec3(0.99));
    color = stylizeClouds(color);

    float dither = (hash(vec3(gl_FragCoord.xy, 0.0)) - 0.5) / 255.0;
    color += dither;

    FragColor = vec4(color, sum.w);
}