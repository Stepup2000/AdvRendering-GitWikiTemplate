#version 400 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;
uniform bool horizontal;
uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(image, 0));

    //Weight center pixel
    vec3 result = texture(image, TexCoords).rgb * weight[0];

    //Apply blur in horizontal and vertical direction
    for(int i = 1; i < 5; i++) {
        vec2 offset = horizontal ? vec2(texelSize.x * i, 0.0) : vec2(0.0, texelSize.y * i);
        result += texture(image, TexCoords + offset).rgb * weight[i];
        result += texture(image, TexCoords - offset).rgb * weight[i];
    }
    FragColor = vec4(result, 1.0);
}