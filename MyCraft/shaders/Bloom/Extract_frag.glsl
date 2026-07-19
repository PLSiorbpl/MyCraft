#version 330 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D scene;
uniform float Threshold;

void main()  {
    vec3 color = texture(scene, uv).rgb;

    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    if (brightness > Threshold)
    FragColor = vec4(color, 1.0);
    else
    FragColor = vec4(0.0);
}