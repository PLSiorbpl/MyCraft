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

    float sun = smoothstep(0.999, 1.0, d);
    float glow = smoothstep(0.98, 1.0, d);

    const vec3 sunColor = vec3(1.0, 0.9, 0.6);

    sky += sunColor * sun * 12.0;  // Disk
    sky += sunColor * glow * 0.6;  // Aura

    float height = 2;
    int samples = 20;
    vec3 color;

    for (int i = 0; i < samples; i++) {
        height += 0.01;
        float dist_to_height = height / rayDir.y;
        vec3 world_pos = rayDir * dist_to_height;
        color = smoothstep(0.15, 0, abs(vec3(fract(world_pos.x)) - 0.5f));
        float upfactor = rayDir.y;
        color *= upfactor;
    }

    //sky = mix(color, sky, dayfactor);

    FragColor = vec4(sky, 1.0);
}