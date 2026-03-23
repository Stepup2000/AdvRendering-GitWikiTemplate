#version 400 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D sceneTex;

void main() {
    // Kernels for horizontal and vertical edges
    float kernelX[9] = float[]
    (-1,0,1,
    -2,0,2,
    -1,0,1);

    float kernelY[9] = float[]
    (-1,-2,-1,
    0,0,0,
    1,2,1);

    vec2 texelSize = 1.0 / vec2(textureSize(sceneTex,0));
    int id = 0;
    float sumX = 0.0, sumY = 0.0;

    // iterate over 3x3 neighborhood
    for(int y = -1; y <= 1; y++){
        for(int x = -1; x <= 1; x++){
            // Convert color to grayscale
            vec3 pix = texture(sceneTex, TexCoords + vec2(x,y) * texelSize).rgb;
            float lum = dot(pix, vec3(0.299,0.587,0.114));

            // apply kernels
            sumX += lum * kernelX[id];
            sumY += lum * kernelY[id];
            id++;
        }
    }

    // combine horizontal and vertical gradients
    float edge = sqrt(sumX * sumX + sumY * sumY);

    // output edge intensity
    FragColor = vec4(vec3(edge), 1.0);
}