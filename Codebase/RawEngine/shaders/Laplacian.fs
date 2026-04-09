#version 400 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D sceneTex;
uniform int kernelSize = 7; // 3, 5, 7

const float w3[3] = float[](1, 2, 1);
const float w5[5] = float[](1, 4, 6, 4, 1);
const float w7[7] = float[](1, 6, 15, 20, 15, 6, 1);

float getWeight(int i)
{
    if(kernelSize == 3) return w3[i];
    if(kernelSize == 5) return w5[i];
    return w7[i];
}

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(sceneTex,0));
    float lap = 0.0;
    int radius = kernelSize/2;

    // Sum of weights for normalization
    float sum = 0.0;
    for(int i = 0 ; i < kernelSize ; i++) sum += getWeight(i);
    sum = sum * sum;

    for(int y = -radius; y <= radius; y++)
    {
        for(int x = -radius; x<= radius; x++)
        {
            vec2 offset = vec2(x,y) * texelSize;
            float lum = dot(texture(sceneTex, TexCoords+offset).rgb, vec3(0.299,0.587,0.114));

            float w = getWeight(x + radius) * getWeight(y + radius);

            // Laplacian: center positive, neighbors negative
            if(x == 0 && y == 0) lap += lum * float(kernelSize * kernelSize);
            else lap -= lum;
        }
    }

    lap = clamp(lap/float(kernelSize * kernelSize),0.0,1.0);
    FragColor = vec4(vec3(lap),1.0);
}