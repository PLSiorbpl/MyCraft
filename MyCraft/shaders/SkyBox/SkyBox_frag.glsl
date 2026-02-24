#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform vec3 sunDir;
uniform float dayfactor;
uniform mat4 invView;
uniform mat4 invProj;

void main()  {
    // Initialization
    vec2 ndc = uv * 2.0 - 1.0;

    vec4 clip = vec4(ndc, 1.0, 1.0);

    vec4 view = invProj * clip;
    view = vec4(view.xyz, 0.0);

    vec3 rayDir = normalize((invView * view).xyz);

    // Sky Gradient
    float t = max(rayDir.y, 0.0);

    const vec3 skyTop = vec3(0.15, 0.35, 0.75);
    const vec3 skyBottom = vec3(0.6, 0.75, 1.0);

    vec3 sky = mix(skyBottom, skyTop, t);

    const vec3 nightSky = vec3(0.02, 0.03, 0.08);
    sky = mix(nightSky, sky, dayfactor);

    // Sun
    float d = dot(rayDir, sunDir);

    float sun = smoothstep(0.9995, 1.0, d);
    float glow = smoothstep(0.99, 1.0, d);

    const vec3 sunColor = vec3(1.0, 0.9, 0.6);

    sky += sunColor * sun * 12.0;  // Disk
    sky += sunColor * glow * 0.6;  // Aura

    FragColor = vec4(sky, 1.0);
}