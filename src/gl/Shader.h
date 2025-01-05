#ifndef SHADER_H
#define SHADER_H
#include <fwd.hpp>
#include <string>
#include <vec2.hpp>
#include <vec3.hpp>
#include <vec4.hpp>

class Shader {
public:
    Shader(const char *const *vertexCode, const char *const *fragmentCode, const char *const *geometryCode = nullptr);

    ~Shader();

    void use();

    int getUniformLocation(const char *name) const;
    void setUniform(const char *name, int value) const;
    void setUniform(const char *name, float value) const;
    void setUniform(const char *name, const glm::vec2 &value) const;
    void setUniform(const char *name, const glm::vec3 &value) const;
    void setUniform(const char *name, const glm::vec4 &value) const;
    void setUniform(const char *name, const glm::mat3 &value) const;
    void setUniform(const char *name, const glm::mat4 &value) const;

    static void setUniform(int loc, int value);
    static void setUniform(int loc, float value);
    static void setUniform(int loc, const glm::vec2 &value);
    static void setUniform(int loc, const glm::vec3 &value);
    static void setUniform(int loc, const glm::vec4 &value);
    static void setUniform(int loc, const glm::mat3 &value);
    static void setUniform(int loc, const glm::mat4 &value);

private:
    unsigned int m_id = 0;
};

#endif //SHADER_H
