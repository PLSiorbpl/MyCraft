#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform vec3 sunDir;
uniform float dayfactor;
uniform mat4 invView;
uniform mat4 invProj;
uniform float Time; // seconds elapsed
uniform vec3 CameraPos;

const vec3 SUN_COLOR    = vec3(1.0, 0.9, 0.6);
const vec3 SUNSET_COLOR = vec3(1.0, 0.42, 0.15);
const vec3 SKY_TOP      = vec3(0.15, 0.35, 0.75);
const vec3 SKY_BOTTOM   = vec3(0.6, 0.75, 1.0);
const vec3 NIGHT_SKY    = vec3(0.02, 0.03, 0.08);
const vec3 MOON_COLOR   = vec3(0.85, 0.9, 1.0);
const vec3 MOONSET_COLOR = vec3(0.694, 0.878, 0.835);
const vec3 STAR_COLOR_1 = vec3(0.7,0.8,1.0);
const vec3 STAR_COLOR_2 = vec3(1.0, 0.643, 0.827);

const float cloudBottom = 120.0;
const float cloudTop = 200.0;
const int STEPS = 32;
const int OCTAVES = 3;

vec3 L = normalize(sunDir);

float hash(vec3 p) {
    p = fract(p * 0.3183099 + 0.1);
    p *= 17.0;
    return fract(p.x*p.y*p.z * (p.x+p.y+p.z));
}

float hash_noise(const vec3 x) {
    vec3 p = floor(x);
    return hash(p);
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

float getDensity(vec3 p) {
    p *= vec3(0.008, 0.02, 0.008);
    vec3 wind = vec3(0.4, -0.15, 1.0) * Time * 0.05;
    vec3 q = p + wind;
    float f = fbm(q, OCTAVES);
    return f;
}

//vec3 lighting(const vec3 pos, const float cloudDensity, const vec3 backgroundColor, const float pathLength ) {
//    float densityLightDir = getDensity(pos + 0.3 * L);
//    float gradientLightDir = clamp(cloudDensity - densityLightDir, 0.0, 1.0);
//
//    vec3 litColor = MOON_COLOR + vec3(1.0, 0.6, 0.3) * 2.0 * gradientLightDir;
//    vec3 cloudAlbedo = mix( vec3(1.0, 0.95, 0.8), vec3(0.4, 0.4, 0.4), cloudDensity );
//
//    const float extinction = 0;
//    float transmittance = exp(-extinction * pathLength);
//    return mix(backgroundColor, cloudAlbedo * litColor, transmittance );
//}

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

float lighting(vec3 pos) {
    vec3 dirToLight = L;
    float dstInsideBox = rayBoxDst(vec3(0, cloudBottom, 0), vec3(0, cloudTop, 0), pos, 1/dirToLight).y;

    float stepSize = dstInsideBox/4;
    float totalDensity = 0.0;

    for (int step = 0; step < 4; step ++) {
        pos += dirToLight * stepSize;
        totalDensity += max(0.0, getDensity(pos) * stepSize);
    }

    const float darknessThreshold = 1.0;
    const float lightAbsorptionTowardSun = 1.0;

    float transmittance = exp(-totalDensity * lightAbsorptionTowardSun);
    return darknessThreshold + transmittance * (1-darknessThreshold);
}

//vec4 rayMarch(const float start, const float end, const vec3 rayDir, const vec3 bgcolor) {
//    float stepSize = (end - start) / float(STEPS);
//    vec4 sum = vec4(0.0);
//
//    for (int i = 0; i < STEPS; i++) {
//        if (0.99 < sum.a) break;
//        float t = start + (float(i) + 0.5) * stepSize;
//        float distanceFade = 1.0 - smoothstep(0.0, 2500.0, t);
//        if (distanceFade >= 2450)
//            break;
//
//        vec3 p = CameraPos + rayDir * t;
//        float cloudDensity = clamp(getDensity(p) - 0.5, 0.0, 1.0);
//        cloudDensity = cloudDensity * stepSize * 0.25 * distanceFade;
//
//        if (0.01 < cloudDensity) {
//            vec3 colorRGB = lighting(p, cloudDensity, bgcolor, i);
//            float alpha = cloudDensity * 0.4;
//            vec4 color = vec4(colorRGB, alpha);
//            sum += color * (1.0 - sum.a);
//        }
//    }
//
//    return clamp(sum, 0.0, 1.0);
//}

float hg(float a, float g) {
    float g2 = g*g;
    return (1-g2) / (4*3.1415*pow(1+g2-2*g*(a), 1.5));
}

float phase(float a) {
    float blend = .5;
    float hgBlend = hg(a,phaseParams.x) * (1-blend) + hg(a,-phaseParams.y) * blend;
    return phaseParams.z + hgBlend*phaseParams.w;
}

vec4 rayMarch(const float start, const float end, const vec3 rayDir, const vec3 bgcolor) {
    float stepSize = (end - start) / float(STEPS);
    float transmittance = 1;
    vec3 lightEnergy = vec3(0);
    float dstTravelled = 0;

    float cosAngle = dot(rayDir, L);
    float phaseVal = phase(cosAngle);
    const float lightAbsorptionThroughCloud = 1;

    while (dstTravelled < stepSize*STEPS) {
        vec3 rayPos = start + rayDir * dstTravelled;
        float density = getDensity(rayPos);

        if (density > 0) {
            float lightTransmittance = lighting(rayPos);
            lightEnergy += density * stepSize * transmittance * lightTransmittance * phaseVal;
            transmittance *= exp(-density * stepSize * lightAbsorptionThroughCloud);

            // Exit early if T is close to zero as further samples won't affect the result much
            if (transmittance < 0.01) {
                break;
            }
        }
        dstTravelled += stepSize;
    }
}

void main() {
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 clip = vec4(ndc, 1.0, 1.0);

    vec4 view = invProj * clip;
    view /= view.w;

    vec3 rayDir = normalize((invView * vec4(view.xyz, 0.0)).xyz);

    float sunHeight = L.y;
    float horizonFactor = 1.0 - smoothstep(0.0, 0.4, abs(sunHeight));

    float t0 = (cloudBottom - CameraPos.y) / rayDir.y;
    float t1 = (cloudTop - CameraPos.y) / rayDir.y;

    float start = max(min(t0, t1), 0.0);
    float end = max(t0, t1);

    if (end <= start)
        discard;

    float t = max(rayDir.y, 0.0);
    vec3 sky = mix(SKY_BOTTOM, SKY_TOP, t);
    sky = mix(NIGHT_SKY, sky, dayfactor);

    float viewNearHorizon = 1.0 - abs(rayDir.y);
    vec2 sunDirXZ = normalize(L.xz + vec2(1e-4));
    vec2 viewDirXZ = normalize(rayDir.xz + vec2(1e-4));
    float azimuthAlign = dot(sunDirXZ, viewDirXZ) * 0.5 + 0.5;
    float sunsetGlow = horizonFactor * viewNearHorizon * mix(0.3, 1.0, azimuthAlign);
    sky = mix(sky, SUNSET_COLOR, sunsetGlow * 0.7);

    vec4 sum = rayMarch(start, end, rayDir, sky);

    if (sum.w <= 0.0)
        discard;

    float dither = (hash(vec3(gl_FragCoord.xy, 0.0)) - 0.5) / 255.0;
    sum += dither;

    FragColor = vec4(sum);
}