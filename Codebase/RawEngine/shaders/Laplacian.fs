#version 400 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D sceneTex;
uniform int kernelSize = 7;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(sceneTex, 0));

    int radius = kernelSize / 2;

    float center = 0.0;
    float neighbors = 0.0;

    for(int y = -radius; y <= radius; y++)
    {
        for(int x = -radius; x <= radius; x++)
        {
            vec2 offset = vec2(x, y) * texelSize;

            vec3 color = texture(sceneTex, TexCoords + offset).rgb;

            float lum = dot(color, vec3(0.299, 0.587, 0.114));

            if(x == 0 && y == 0)
            {
                center += lum;
            }
            else
            {
                neighbors += lum;
            }
        }
    }

    float lap = center * float(kernelSize * kernelSize - 1) - neighbors;
    float edge = abs(lap);

    edge = clamp(edge / 10.0, 0.0, 1.0);

    FragColor = vec4(vec3(edge), 1.0);
}