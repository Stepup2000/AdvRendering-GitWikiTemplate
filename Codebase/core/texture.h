#pragma once

#include <glad/glad.h>
#include <string>

namespace core {

    class Texture {
    public:
        Texture(const std::string& path);
        ~Texture();
        GLuint getId() const;
    private:
        GLuint id{};
    };


}
