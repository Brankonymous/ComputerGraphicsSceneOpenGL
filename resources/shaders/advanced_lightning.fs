#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct PointLight {
    vec3 position;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;

    float shininess;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

#define NUM_LIGHTS 3

uniform SpotLight spotLight[NUM_LIGHTS];
uniform PointLight pointLight[NUM_LIGHTS];
uniform Material material;

uniform vec3 cameraPos;
uniform vec3 viewPosition;

uniform bool shadow_flag;
uniform float far_plane;
uniform samplerCube depthMap;

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow)
{

    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords).xxx);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + shadow * diffuse + shadow * specular);
}
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow)
 {
     vec3 lightDir = normalize(light.position - fragPos);
     // diffuse shading
     float diff = max(dot(normal, lightDir), 0.0);
     // specular shading
     vec3 halfwayDir = normalize(lightDir + viewDir);
     float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
     // attenuation
     float distance = length(light.position - fragPos);
     float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
     // spotlight intensity
     float theta = dot(lightDir, normalize(-light.direction));
     float epsilon = light.cutOff - light.outerCutOff;
     float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
     // combine results
     vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
     vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
     vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
     ambient *= attenuation * intensity;
     diffuse *= attenuation * intensity;
     specular *= attenuation * intensity;
     return (ambient + shadow * diffuse + shadow * specular);
 }

 float CalcShadow(vec3 fragPos, int depthMapId, vec3 lightPosition) {

    vec3 fragToLight = fragPos - lightPosition;
    float shadow = 0.0;
    float bias = 0.01;
    float samples = 10.0;
    float offset = 0.15;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5)) {
     for(float y = -offset; y < offset; y += offset / (samples * 0.5)) {
         for(float z = -offset; z < offset; z += offset / (samples * 0.5)) {
             float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z) * 0.05).r;

             closestDepth *= far_plane;
             if(length(fragToLight) - bias > closestDepth)
             shadow += 1.0;
         }
     }
    }
    shadow /= (samples * samples * samples);
    shadow = 1 - shadow;

    return shadow;
 }

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 result = vec3(0,0,0);

    for(int i = 0; i < NUM_LIGHTS; i++){
        float pointLightShadow = 1.0;
        float spotLightShadow = 1.0;

        if (shadow_flag) {
            pointLightShadow = CalcShadow(FragPos, i, pointLight[i].position);
            spotLightShadow = CalcShadow(FragPos, i, spotLight[i].position);
        }
        vec3 pointLightColor = CalcPointLight(pointLight[i], normal, FragPos, viewDir, pointLightShadow);
        vec3 spotLightColor = CalcSpotLight(spotLight[i],normal,FragPos,viewDir, spotLightShadow);

        result += pointLightColor;
        result += spotLightColor;
    }

    // Camera distance blending
    float cameraDistance = min(1.0, length(FragPos-cameraPos) / 1.5);

    FragColor = vec4(result, cameraDistance);
}