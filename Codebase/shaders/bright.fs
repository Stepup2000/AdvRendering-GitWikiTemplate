#version 400 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D sceneTex;
uniform float threshold;

void main() {
    vec3 color = texture(sceneTex, TexCoords).rgb;

    // keep only bright parts above threshold
    vec3 bright = max(color - vec3(threshold), 0.0);

    FragColor = vec4(bright, 1.0);
}