#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace core {

    Texture::Texture(const std::string &path) {
        stbi_set_flip_vertically_on_load(true);

        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);

        if (!data) {
            printf("Texture failed to load: %s\n", path.c_str());
            return;
        }

        GLenum format, internalFormat;
        if (nrComponents == 1) {
            format = internalFormat = GL_RED;
        } else if (nrComponents == 3) {
            format = GL_RGB;
            internalFormat = GL_SRGB;
        } else {
            format = GL_RGBA;
            internalFormat = GL_SRGB_ALPHA;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                     width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture::~Texture() {
        glDeleteTextures(1, &id);
    }

    GLuint Texture::getId() const {
        return id;
    }

}
