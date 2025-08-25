#version 330 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D tex;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
    // someshit
    float roughness = pow(1 - 0.2, 2.0);

    // Sun
    vec3 SunDir = normalize(vec3(0.7f, -1.0f, 0.7f));
    vec3 lightColor = vec3(1.0,1.0,0.8);

    // Base Texture
    vec4 baseColor = texture(tex, TexCoord);
    vec3 outputColor = pow(baseColor.rgb, vec3(2.2));


    vec3 WorldNormal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(-SunDir);

    // Lighting
    vec3 halfVector = normalize(lightDir+viewDir);
    //vec3 reflectionDirection = reflect(-lightDir, WorldNormal);
    float shininess = 2/pow(roughness, 2) - 2;


    float DiffuseLight = max(roughness * dot(lightDir, WorldNormal), 0.0);
    float SpecularLight = pow(max(dot(halfVector, WorldNormal), 0.0), shininess);
    float AmbientLight = 0.2;

    float LightBritness = DiffuseLight + SpecularLight + AmbientLight;

    outputColor *= lightColor;
    outputColor *= LightBritness;

    // Output Color
    FragColor = vec4(pow(outputColor, vec3(1/2.2)), baseColor.a);
    //FragColor = vec4(vec3(SpecularLight), 1);
}
