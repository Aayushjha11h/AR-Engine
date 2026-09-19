#include "Texture.h"
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../ThirdParty/stb/stb_image.h"
#include <iostream>

namespace ar {

    bool Texture::Load(const std::string& path) {
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &m_Width, &m_Height, nullptr, 4);
        if (!data) {
            std::cerr << "[Texture] Failed: " << path << "\n";
            return false;
        }
        glGenTextures(1, &m_ID);
        glBindTexture(GL_TEXTURE_2D, m_ID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
        std::cout << "[Texture] Loaded: " << path << " (" << m_Width << "x" << m_Height << ")\n";
        return true;
    }

    void Texture::Bind(unsigned int slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_ID);
    }

} // namespace ar