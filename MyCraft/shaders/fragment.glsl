#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D BaseTexture;
uniform vec3 ViewPos;
uniform int RenderDist;
uniform vec3 Sun;
uniform float dayfactor; // 0 = night, 1 = day

out vec4 FragColor;

// ----------------------------
// Constants
// ----------------------------
const vec3 SUN_COLOR    = vec3(1.0, 0.9, 0.6);
const vec3 SUNSET_COLOR = vec3(1.0, 0.42, 0.15);
const vec3 SKY_TOP      = vec3(0.15, 0.35, 0.75);
const vec3 SKY_BOTTOM   = vec3(0.6, 0.75, 1.0);
const vec3 NIGHT_SKY    = vec3(0.02, 0.03, 0.08);
const vec3 MOON_COLOR   = vec3(0.85, 0.9, 1.0);
const vec3 MOONSET_COLOR = vec3(0.694, 0.878, 0.835);

const float ROUGHNESS = 0.5;
const float DIFFUSE_MULT = 1.0;

const float FOG_END = 1.0;

const float PI = 3.14159265;
const float two_PI = 2.0*3.14159265;

vec3 stylize(vec3 c) {
    float lum = dot(c, vec3(0.2126, 0.7152, 0.0722)); // BT.709 weights or smt
    c = mix(vec3(lum), c, 1.35); // saturation boost
    c = mix(c, c * vec3(0.85, 0.9, 1.15), 1.0 - lum); // cold shadows
    c = mix(c, c * vec3(1.1, 1.0, 0.85), lum); // warm highlights
    return c;
}

void main() {
    // ----------------------------
    // Normalization
    // ----------------------------
    if (length(Normal) < 0.5) discard;
    vec3 N = normalize(Normal);
    vec3 V = normalize(ViewPos - FragPos);
    vec3 L = normalize(Sun);
    vec3 H = normalize(L + V);

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    float sunHeight = L.y;
    float sunVisibility = smoothstep(0.0, 0.1, sunHeight);
    float horizonFactor = 1.0 - smoothstep(0.0, 0.5, abs(sunHeight));
    vec3 dynamicSunColor = mix(SUN_COLOR, SUNSET_COLOR, horizonFactor);

    // ----------------------------
    // Albedo
    // ----------------------------
    vec4 baseColor = texture(BaseTexture, TexCoord);
    vec3 albedo = pow(baseColor.rgb, vec3(2.2));

    // ----------------------------
    // Diffuse
    // ----------------------------
    float diffuse = DIFFUSE_MULT * NdotL * sunVisibility;

    // ----------------------------
    // Specular
    // ----------------------------
    float roughness2 = max(ROUGHNESS * ROUGHNESS, 0.001);
    float shininess  = 2.0 / roughness2 - 2.0;
    float normFactor = (shininess + 8.0) / (8.0 * PI);
    float specular = (NdotL > 0.0) ? normFactor * pow(NdotH, shininess) * sunVisibility : 0.0;

    // ----------------------------
    // Ambient
    // ----------------------------
    float ambientStrength = mix(0.02, 0.2, dayfactor);

    // ----------------------------
    // Combine
    // ----------------------------
    vec3 direct = (diffuse + specular) * dynamicSunColor;
    vec3 totalLight = clamp(direct + vec3(ambientStrength), 0.0, 1.0);
    vec3 litColor = albedo * totalLight;

    float rim = pow(1.0 - max(0.0, dot(N, V)), 3.0);
    vec3 rimColor = mix(vec3(0.6, 0.85, 1.0), SUNSET_COLOR, horizonFactor);
    litColor += rimColor * rim * 0.05 * sunVisibility;

    // ----------------------------
    // Sky
    // ----------------------------
    vec3 skyDir = -V;
    float skyBlend = clamp(skyDir.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 sky = mix(SKY_BOTTOM, SKY_TOP, skyBlend);
    sky = mix(NIGHT_SKY, sky, dayfactor);

    float viewNearHorizon = 1.0 - abs(skyDir.y);
    vec2 sunDirXZ = normalize(Sun.xz + vec2(1e-4));
    vec2 viewDirXZ = normalize(skyDir.xz + vec2(1e-4));
    float azimuthAlign = dot(sunDirXZ, viewDirXZ) * 0.5 + 0.5;
    float sunsetGlow = horizonFactor * viewNearHorizon * mix(0.3, 1.0, azimuthAlign);
    sky = mix(sky, SUNSET_COLOR, sunsetGlow * 0.7);

    // ----------------------------
    // Fog
    // ----------------------------
    float FOG_DENSITY = mix(2.5, 0.9, dayfactor);
    float fogStart = mix(0.4, 0.7, dayfactor);
    float dist = length(FragPos - ViewPos) / max(float(RenderDist) * 16.0, 1.0);
    dist *= FOG_DENSITY;
    float fogFactor = clamp(smoothstep(fogStart, FOG_END, dist), 0.0, 1.2);

    // ----------------------------
    // Final
    // ----------------------------
    vec3 finalColor = pow(litColor, vec3(1.0 / 2.2));
    finalColor = mix(finalColor, sky, fogFactor);
    finalColor = stylize(finalColor);

    FragColor = vec4(finalColor, baseColor.a);
}
