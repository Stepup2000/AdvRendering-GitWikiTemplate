#version 400 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D sceneTex;
uniform int kernelSize = 7; // 3, 5, 7

// Predefined weights (Pascal triangle rows)
const float w3[3] = float[](1, 2, 1);
const float w5[5] = float[](1, 4, 6, 4, 1);
const float w7[7] = float[](1, 6, 15, 20, 15, 6, 1);

float getWeight(int i)
{
    if(kernelSize == 3) return w3[i];
    if(kernelSize == 5) return w5[i];
    return w7[i]; // default to 7
}

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(sceneTex, 0));

    float gx = 0.0;
    float gy = 0.0;

    int radius = kernelSize / 2;

    for(int y = -radius; y <= radius; y++)
    {
        for(int x = -radius; x <= radius; x++)
        {
            vec2 offset = vec2(x, y) * texelSize;

            vec3 c = texture(sceneTex, TexCoords + offset).rgb;
            float lum = dot(c, vec3(0.299, 0.587, 0.114));

            float wx = getWeight(x + radius);
            float wy = getWeight(y + radius);

            float w = wx * wy;

            gx += lum * float(x) * w;
            gy += lum * float(y) * w;
        }
    }

    float edge = abs(gx) + abs(gy);

    // Normalize based on kernel size (rough scaling)
    edge = clamp(edge / float(kernelSize * kernelSize), 0.0, 1.0);

    FragColor = vec4(vec3(edge), 1.0);
}