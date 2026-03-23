#version 400 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float intensity;

void main() {
    vec3 sceneCol = texture(scene, TexCoords).rgb;
    vec3 bloomCol = texture(bloomBlur, TexCoords).rgb;
    FragColor = vec4(sceneCol + bloomCol * intensity, 1.0);
}