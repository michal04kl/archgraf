#include "Texture.h"
#include <glad/glad.h>

// Implementacja stb_image tylko raz w calym projekcie
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

Texture::~Texture() {
    if (id) glDeleteTextures(1, &id);
}

Texture::Texture(Texture&& o) noexcept
    : id(o.id), width(o.width), height(o.height), channels(o.channels)
{ o.id = 0; }

Texture& Texture::operator=(Texture&& o) noexcept {
    if (this != &o) {
        if (id) glDeleteTextures(1, &id);
        id=o.id; width=o.width; height=o.height; channels=o.channels;
        o.id = 0;
    }
    return *this;
}

bool Texture::load(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "[Texture] Nie mozna zaladowac: " << path << "\n";
        return false;
    }

    GLenum fmt = (channels == 4) ? GL_RGBA :
                 (channels == 3) ? GL_RGB  : GL_RED;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    std::cout << "[Texture] Zaladowano: " << path
              << " (" << width << "x" << height << ")\n";
    return true;
}

void Texture::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}