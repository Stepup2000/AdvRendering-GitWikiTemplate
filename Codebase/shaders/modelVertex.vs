#version 400
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNor;
layout (location = 2) in vec2 aUv;
uniform mat4 mvpMatrix;
out vec3 fPos;
out vec3 fNor;
out vec2 uv;

void main() {
    //Pass pos, normal and uv to fragment shader
    fPos = aPos;
    fNor = aNor;
    uv = aUv;

    //Go to clip space, model -> world -> camera -> clip
    gl_Position = mvpMatrix * vec4(aPos, 1.0);
}