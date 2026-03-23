#pragma once
#include <glad/glad.h>

class FullscreenQuad {
public:
    // Constructor: sets up the quad VAO/VBO
    FullscreenQuad();

    // Destructor: cleans up OpenGL resources
    ~FullscreenQuad();

    // Render the quad
    void render() const;

private:
    GLuint VAO = 0;
    GLuint VBO = 0;
};
