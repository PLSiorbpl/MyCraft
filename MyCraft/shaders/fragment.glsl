#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
//in vec3 VColor;

uniform sampler2D BaseTexture;
uniform vec3 ViewPos;
uniform int RenderDist;

out vec4 FragColor;

void main() {
    // ----------------------------
    // Constants
    // ----------------------------
    float roughness = pow(1 - 0.15, 2.0);
    float metalness = 1.0; // soon in texture

    vec3 Sun = vec3(0.7f, -1.0f, 0.7f);
    vec3 lightColor = vec3(1.0,1.0,0.8);

    // ----------------------------
    // Normalization
    // ----------------------------
    vec3 WorldNormal = normalize(Normal);
    if (length(WorldNormal) < 0.5) discard;
    vec3 SunDir = normalize(Sun);
    vec3 viewDir = normalize(ViewPos - FragPos);
    vec3 lightDir = normalize(-SunDir);

    // ----------------------------
    // Albedo
    // ----------------------------
    vec4 baseColorData = texture(BaseTexture, TexCoord);
    vec3 FinalColor = pow(baseColorData.rgb, vec3(2.2));

    float AmbientLight = 0.2;


    // ----------------------------
    // Diffuse
    // ----------------------------
    float DiffuseLight = max(metalness * dot(WorldNormal, lightDir), 0.0);


    // ----------------------------
    // Specular
    // ----------------------------
    float SpecularLight = 0.0;
     if (DiffuseLight > 0.0) {
        float shininess = 2/pow(roughness, 2) - 2;
        vec3 halfVector = normalize(lightDir+viewDir);

        SpecularLight = pow(max(dot(halfVector, WorldNormal), 0.0), shininess);
    }

    // ----------------------------
    // Fog
    // ----------------------------
    vec3 fogColor = vec3(0.5, 0.7, 1.0);//vec3(0.7, 0.8, 0.9);

    float distance = length(FragPos - ViewPos);

    float fogEnd   = (RenderDist*16)+128;
    float fogStart = (RenderDist*16)-64;

    float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);


    // ----------------------------
    // Final
    // ----------------------------
    float LightBritness = clamp(DiffuseLight + SpecularLight + AmbientLight, 0.0, 1.0);


    FinalColor *= lightColor;
    FinalColor *= LightBritness;
    //FinalColor *= VColor;
    FinalColor = mix(fogColor, FinalColor, fogFactor);

    FragColor = vec4(pow(FinalColor, vec3(1/2.2)), baseColorData.a);

    // ----------------------------
    // Debug
    // ----------------------------
    //FragColor = vec4(vec3(SpecularLight), 1);
}
