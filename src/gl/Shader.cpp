#include "Shader.h"

#include <iostream>
#include <glad/glad.h>
#include <gtc/type_ptr.hpp>

void checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog <<
                    "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog <<
                    "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

Shader::Shader(const char *const *vertexCode, const char *const *fragmentCode, const char *const *geometryCode) {
    if (vertexCode == nullptr || fragmentCode == nullptr) {
        std::cerr << "ERROR::SHADER_CODE_UNDEFINED" << std::endl;
        return;
    }

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, vertexCode, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, fragmentCode, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    GLuint geometry = 0;
    if (geometryCode != nullptr) {
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, geometryCode, nullptr);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "GEOMETRY");
    }

    m_id = glCreateProgram();
    glAttachShader(m_id, vertex);
    glAttachShader(m_id, fragment);
    if (geometry != 0) {
        glAttachShader(m_id, geometry);
    }

    glLinkProgram(m_id);
    checkCompileErrors(m_id, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometry != 0) {
        glDeleteShader(geometry);
    }
}

Shader::~Shader() {
    glDeleteProgram(m_id);
}

void Shader::use() {
    glUseProgram(m_id);
}

int Shader::getUniformLocation(const char *name) const {
    return glGetUniformLocation(m_id, name);
}

void Shader::setUniform(const char *name, int value) const {
    glUniform1i(glGetUniformLocation(m_id, name), value);
}

void Shader::setUniform(const char *name, float value) const {
    glUniform1f(glGetUniformLocation(m_id, name), value);
}

void Shader::setUniform(const char *name, const glm::vec2 &value) const {
    glUniform2fv(glGetUniformLocation(m_id, name), 1, &value[0]);
}

void Shader::setUniform(const char *name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(m_id, name), 1, &value[0]);
}

void Shader::setUniform(const char *name, const glm::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(m_id, name), 1, &value[0]);
}

void Shader::setUniform(const char *name, const glm::mat3 &value) const {
    glUniformMatrix3fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, &value[0][0]);
}

void Shader::setUniform(const char *name, const glm::mat4 &value) const {
    glUniformMatrix4fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, &value[0][0]);
}

void Shader::setUniform(int loc, int value) {
    glUniform1i(loc, value);
}

void Shader::setUniform(int loc, float value) {
    glUniform1f(loc, value);
}

void Shader::setUniform(int loc, const glm::vec2 &value) {
    glUniform2fv(loc, 1, &value[0]);
}

void Shader::setUniform(int loc, const glm::vec3 &value) {
    glUniform3fv(loc, 1, &value[0]);
}

void Shader::setUniform(int loc, const glm::vec4 &value) {
    glUniform4fv(loc, 1, &value[0]);
}

void Shader::setUniform(int loc, const glm::mat3 &value) {
    glUniformMatrix3fv(loc, 1, GL_FALSE, &value[0][0]);
}

void Shader::setUniform(int loc, const glm::mat4 &value) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]);
}
