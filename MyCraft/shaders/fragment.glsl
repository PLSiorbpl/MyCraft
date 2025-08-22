#version 330 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D tex;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
    // someshit
    float roughness = 0.8;//pow(1 - 0.5, 2.0);
    float smoothness = 1.0 - roughness;

    // Sun
    vec3 SunDir = normalize(vec3(-0.5f, -1.0f, 0.0f));
    vec3 lightColor = vec3(1,1,0.8);
    lightColor = pow(lightColor, vec3(2.2));

    // Base Texture
    vec4 baseColor = texture(tex, TexCoord);
    vec3 outputColor = pow(baseColor.rgb, vec3(2.2));


    vec3 WorldNormal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(-SunDir);

    // Lighting
    vec3 reflectionDirection = reflect(-lightDir, WorldNormal);
    float shininess = (1+smoothness) * 32;

    float DiffuseLight = clamp(dot(lightDir, WorldNormal), 0.0, 1.0);
    float SpecularLight = clamp(smoothness*pow(dot(reflectionDirection, viewDir), shininess), 0, 1);
    //float facing = max(pow(dot(WorldNormal, viewDir), 100), 0.0);
    //SpecularLight *= facing;
    float AmbientLight = 0.2;

    float LightBritness = DiffuseLight + SpecularLight + AmbientLight;

    outputColor *= LightBritness;
    outputColor *= lightColor;

    // Output Color
    //FragColor = vec4(pow(outputColor, vec3(1/2.2)), baseColor.a);
    //FragColor = vec4(vec3(SpecularLight), 1.0);
    FragColor = vec4(viewDir, 1.0);
}
