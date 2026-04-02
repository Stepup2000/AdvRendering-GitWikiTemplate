#pragma once

#include <vector>
#include <glm/ext/matrix_float4x4.hpp>
#include "mesh.h"

namespace core {

    class Model {
    private:
        std::vector<core::Mesh> meshes;   // All meshes of this model
        glm::mat4 modelMatrix;             // Transform matrix

    public:
        // Constructor: takes a list of meshes
        Model(std::vector<core::Mesh> meshes)
            : meshes(meshes), modelMatrix(1.0f) {}

        GLuint textureId = 0;
        bool hasTexture = false;

        // Render all meshes
        void render() const;

        // Transformations
        void translate(glm::vec3 translation);
        void rotate(glm::vec3 axis, float radians);
        void scale(glm::vec3 scale);

        // Get the model matrix
        glm::mat4 getModelMatrix() const;

        // --- Texture handling ---
        void setTexture(GLuint tex);        // Assign a texture
        bool usesTexture() const;           // Check if model has texture
        GLuint getTextureId() const;        // Retrieve texture ID
    };

} // namespace core