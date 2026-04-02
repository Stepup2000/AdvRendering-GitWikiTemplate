#version 400 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D diffuseMap;
uniform bool useTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform float constant;
uniform float linear;
uniform float quadratic;

void main() {
    vec3 baseColor = useTexture ? texture(diffuseMap, TexCoord).rgb : vec3(0.8,0.5,0.3);
    vec3 ambient = 0.1 * baseColor;

    // normalize normal and get light direction
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    // diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * baseColor * lightColor;

    // specular lighting
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = 0.5 * spec * lightColor;

    // distance attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);

    // combine lighting
    vec3 result = ambient + (diffuse + specular) * attenuation;

    //Output color
    FragColor = vec4(result, 1.0);
}