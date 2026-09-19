#include "Shader.h"
#include <GL/glew.h>
#include <iostream>

namespace ar {

    bool Shader::Compile(unsigned int shader, const char* src) {
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        int success; char log[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::cerr << "[Shader] Compile error: " << log << "\n";
            return false;
        }
        return true;
    }

    bool Shader::Load(const char* vertSrc, const char* fragSrc) {
        unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
        unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
        if (!Compile(vs, vertSrc) || !Compile(fs, fragSrc)) {
            glDeleteShader(vs); glDeleteShader(fs); return false;
        }
        m_ID = glCreateProgram();
        glAttachShader(m_ID, vs);
        glAttachShader(m_ID, fs);
        glLinkProgram(m_ID);
        int success; char log[512];
        glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_ID, 512, nullptr, log);
            std::cerr << "[Shader] Link error: " << log << "\n";
            glDeleteProgram(m_ID); m_ID = 0;
        }
        glDeleteShader(vs); glDeleteShader(fs);
        return success == GL_TRUE;
    }

    void Shader::Use() const { glUseProgram(m_ID); }

    void Shader::SetInt(const std::string& name, int val) const {
        glUniform1i(glGetUniformLocation(m_ID, name.c_str()), val);
    }
    void Shader::SetFloat(const std::string& name, float val) const {
        glUniform1f(glGetUniformLocation(m_ID, name.c_str()), val);
    }
    void Shader::SetVec4(const std::string& name, const glm::vec4& v) const {
        glUniform4f(glGetUniformLocation(m_ID, name.c_str()), v.x, v.y, v.z, v.w);
    }
    void Shader::SetMat4(const std::string& name, const glm::mat4& m) const {
        glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &m[0][0]);
    }

} // namespace ar