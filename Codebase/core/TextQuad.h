#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class TextQuad {
public:
    // Constructor: takes texture ID
    TextQuad(GLuint textureId);

    // Destructor: cleans up OpenGL resources
    ~TextQuad();

    // Set position/scale/rotation
    void setPosition(const glm::vec3& pos);
    void setScale(const glm::vec3& s);
    void setRotation(float angleRadians, const glm::vec3& axis);

    // Render the quad using a shader with a "model" uniform
    void render(GLuint shaderProgram);

private:
    GLuint VAO, VBO;
    GLuint texture;

    glm::vec3 position;
    glm::vec3 scale;
    glm::mat4 rotation = glm::mat4(1.0f);

    glm::mat4 getModelMatrix() const;
};
