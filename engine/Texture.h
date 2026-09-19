#pragma once
#include <string>

namespace ar {

    class Texture {
    public:
        Texture() : m_ID(0), m_Width(0), m_Height(0) {}
        bool Load(const std::string& path);
        void Bind(unsigned int slot = 0) const;
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        unsigned int GetID() const { return m_ID; }
    private:
        unsigned int m_ID;
        int m_Width, m_Height;
    };

} // namespace ar