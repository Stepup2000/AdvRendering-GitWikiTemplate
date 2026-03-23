#version 400 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D sceneTex;

void main()
{
    //Dull color based on brightness
    vec3 color = texture(sceneTex, TexCoords).rgb;
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    FragColor = vec4(vec3(gray), 1.0);
}