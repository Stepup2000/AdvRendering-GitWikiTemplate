#include "TextQuad.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

TextQuad::TextQuad(GLuint texId)
    : texture(texId), position(0.0f), scale(1.0f)
{
    float vertices[] = {
        // positions       // normals    // UVs
        -0.5f,  0.5f, 0,   0,0,1,     0,1,
        -0.5f, -0.5f, 0,   0,0,1,     0,0,
         0.5f, -0.5f, 0,   0,0,1,     1,0,
        -0.5f,  0.5f, 0,   0,0,1,     0,1,
         0.5f, -0.5f, 0,   0,0,1,     1,0,
         0.5f,  0.5f, 0,   0,0,1,     1,1
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // UVs
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

TextQuad::~TextQuad() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void TextQuad::setPosition(const glm::vec3& pos) {
    position = pos;
}

void TextQuad::setScale(const glm::vec3& s) {
    scale = s;
}

void TextQuad::setRotation(float angleRadians, const glm::vec3& axis) {
    rotation = glm::rotate(glm::mat4(1.0f), angleRadians, axis);
}

glm::mat4 TextQuad::getModelMatrix() const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model *= rotation;
    model = glm::scale(model, scale);
    return model;
}

void TextQuad::render(GLuint shaderProgram) {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(getModelMatrix()));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
