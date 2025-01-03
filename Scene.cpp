#include "Scene.h"

#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

auto VERTEX_CODE = R"(
    #version 330 core

    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 vTexCoord;

    uniform mat4 uMatrix;

    void main()
    {
	    gl_Position = vec4(aPos, 0.0, 1.0);
	    vTexCoord = aTexCoord;
    }
)";

auto FRAGMENT_CODE = R"(
    #version 330 core

    out vec4 FragColor;

    in vec2 vTexCoord;

    uniform sampler2D uTexture;

    void main()
    {
	    FragColor = texture(uTexture, vTexCoord);
    }
)";

// void checkCompileErrors(unsigned int shader,  std::string type)
// {
//     int success;
//     char infoLog[1024];
//     if (type != "PROGRAM")
//     {
//         glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//         if (!success)
//         {
//             glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
//             std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog <<
//                 "\n -- --------------------------------------------------- -- " << std::endl;
//         }
//     }
//     else
//     {
//         glGetProgramiv(shader, GL_LINK_STATUS, &success);
//         if (!success)
//         {
//             glGetProgramInfoLog(shader, 1024, NULL, infoLog);
//             std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog <<
//                 "\n -- --------------------------------------------------- -- " << std::endl;
//         }
//     }
// }

Scene::Scene()
{
    m_projection = glm::mat4(1.0f);
    m_view = glm::mat4(1.0f);

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &VERTEX_CODE, nullptr);
    glCompileShader(vertex);
    // checkCompileErrors(vertex, "VERTEX");

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &FRAGMENT_CODE, nullptr);
    glCompileShader(fragment);
    // checkCompileErrors(fragment, "FRAGMENT");

    m_program = glCreateProgram();
    glAttachShader(m_program, vertex);
    glAttachShader(m_program, fragment);
    glLinkProgram(m_program);
    // checkCompileErrors(m_program, "PROGRAM");
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    m_uMatrixLocation = glGetUniformLocation(m_program, "uMatrix");
    m_uTextureLocation = glGetUniformLocation(m_program, "uTexture");

    float vertices[] = {
        -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Scene::~Scene()
{
    clear();
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteProgram(m_program);
}

void Scene::clear()
{
    for (auto &texture: m_textureMap)
    {
        glDeleteTextures(1, &texture.second);
    }
    m_textureMap.clear();
    m_tileMap.clear();
}

void Scene::addTexture(const std::string& key, const std::string& imgPath)
{
    if (m_textureMap.contains(key))
    {
        glDeleteTextures(1, &m_textureMap[key]);
        m_textureMap.erase(key);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;
    stbi_uc* data = stbi_load(imgPath.c_str(), &width, &height, &channels, 0);
    if (!data)
    {
        std::cerr << "Failed to load texture: " << imgPath << std::endl;
        return;
    }

    auto fmt = channels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);
    m_textureMap[key] = texture;
}

void Scene::addTexture(const std::string& key, unsigned char* buf, int size)
{
    if (m_textureMap.contains(key))
    {
        glDeleteTextures(1, &m_textureMap[key]);
        m_textureMap.erase(key);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;
    stbi_uc* data = stbi_load_from_memory(buf, size, &width, &height, &channels, 0);
    if (!data)
    {
        std::cerr << "Failed to load texture " << std::endl;
        return;
    }
    auto fmt = channels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    m_textureMap[key] = texture;
}

void Scene::addTile(const std::string& key, const std::string& textureKey, const glm::mat4& mat)
{
    m_tileMap[key] = {textureKey, mat};
}

void Scene::drawScene()
{
    glUseProgram(m_program);
    glBindVertexArray(m_vao);

    auto mat = m_projection * m_view;
    for (const auto& tile : m_tileMap)
    {
        if (m_textureMap.contains(tile.second.texture))
        {
            glBindTexture(GL_TEXTURE_2D, m_textureMap[tile.second.texture]);
            glActiveTexture(GL_TEXTURE0);
            // glUniform1i(m_uTextureLocation, 0);
            glUniformMatrix4fv(m_uMatrixLocation, 1, GL_FALSE, value_ptr(mat * tile.second.matrix));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);
}
