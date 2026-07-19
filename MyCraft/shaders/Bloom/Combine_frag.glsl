#version 330 core

out vec4 FragColor;
in vec2 uv;

uniform sampler2D scene;
uniform sampler2D bloom;
uniform float Exposure;

void main()
{
    vec3 sceneColor = texture(scene, uv).rgb;
    vec3 bloomColor = texture(bloom, uv).rgb;

    vec3 result = sceneColor + bloomColor * Exposure;

    // Reinhard tonemap
    //result = result / (result + vec3(1.0));

    // Gamma correction
    //result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}