#pragma once
#include <string>
#include <glm/glm.hpp>

namespace ar {

    class Shader {
    public:
        Shader() : m_ID(0) {}
        bool Load(const char* vertSrc, const char* fragSrc);
        void Use() const;
        void SetInt(const std::string& name, int val) const;
        void SetFloat(const std::string& name, float val) const;
        void SetVec4(const std::string& name, const glm::vec4& v) const;
        void SetMat4(const std::string& name, const glm::mat4& m) const;
        unsigned int GetID() const { return m_ID; }
    private:
        unsigned int m_ID;
        bool Compile(unsigned int shader, const char* src);
    };

} // namespace ar