#pragma once
#include <glm/glm.hpp>

namespace ar {
    class Texture;

    struct Sprite {
        glm::vec2 Position = { 0,0 };
        glm::vec2 Size = { 64,64 };
        glm::vec4 Color = { 1,1,1,1 };
        Texture* Tex = nullptr;
    };

} // namespace ar