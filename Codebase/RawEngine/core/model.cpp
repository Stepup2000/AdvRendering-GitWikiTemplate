#include "model.h"
#include <glm/gtc/matrix_transform.hpp>

namespace core {
    void Model::render() const {
        for (int i = 0; i < meshes.size(); ++i) {
            meshes[i].render();
        }
    }

    void Model::translate(glm::vec3 translation) {
        modelMatrix = glm::translate(modelMatrix, translation);
    }

    void Model::rotate(glm::vec3 axis, float radians) {
        modelMatrix = glm::rotate(modelMatrix, radians, axis);
    }

    void Model::scale(glm::vec3 scale) {
        modelMatrix = glm::scale(modelMatrix, scale);
    }

    glm::mat4 Model::getModelMatrix() const {
        return this->modelMatrix;
    }

    // Optional: Assign a texture to this model
    void Model::setTexture(GLuint tex) {
        textureId = tex;
        hasTexture = true;
    }

    // Optional: Check if this model has a texture
    bool Model::usesTexture() const {
        return hasTexture;
    }

    // Optional: Get the texture ID
    GLuint Model::getTextureId() const {
        return textureId;
    }
}