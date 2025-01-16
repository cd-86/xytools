#include "Shape.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <ext/matrix_transform.hpp>
#include <glad/glad.h>

#include "Global.h"
#include "xy2/was.h"

const char *VERTEX_CODE = R"(
    #version 330 core

    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 vTexCoord;

    uniform mat4 uMatrix;

    void main()
    {
	    gl_Position = uMatrix * vec4(aPos, 0.0, 1.0);
	    vTexCoord = aTexCoord;
    }
)";

const char *FRAGMENT_CODE = R"(
    #version 330 core

    out vec4 FragColor;

    in vec2 vTexCoord;

    uniform sampler2D uTexture;

    void main()
    {
	    FragColor = texture(uTexture, vTexCoord);
        if (FragColor.a < 0.001)
            FragColor += vec4(1,1,1,0.4);
    }
)";

Shape::Shape(): m_shader(&VERTEX_CODE, &FRAGMENT_CODE),
                m_position(0.f),
                m_scale(1.f),
                m_matrix(1.f) {
    m_uMatrixLocation = m_shader.getUniformLocation("uMatrix");
    m_uTextureLocation = m_shader.getUniformLocation("uTexture");

    float vertices[] = {
        -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Shape::~Shape() {
    clear();
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
}

void Shape::loadWdf(const std::string &wdfPath) {
    if (!m_wdf.load(wdfPath))
        return;
    m_wasList.clear();
    for (auto &was: m_wdf.getWasInfos()) {
        std::stringstream ss;
        switch (was.second.type) {
            case WT_PS:
                ss << "[PS] ";
                break;
            case WT_PK:
                ss << "[PK] ";
                break;
            case WT_MP3:
                ss << "[MP3] ";
                break;
            case WT_WAVE:
                ss << "[WAVE] ";
                break;
            case WT_FSB4:
                ss << "[FSB4] ";
                break;
            case WT_JPEG:
                ss << "[JPEG] ";
                break;
            case WT_TGA:
                ss << "[TGA] ";
                break;
            case WT_PNG:
                ss << "[PNG] ";
                break;
            case WT_RAR:
                ss << "[RAR] ";
            break;
            default:
                ss << "[???] ";
                break;
        }
        ss << std::uppercase << std::hex << was.second.hash;
        m_wasList.emplace_back(ss.str(), was.second.hash);
    }
    std::ranges::sort(m_wasList);
}

void Shape::loadWas(uint32_t hash) {
    if (!m_wdf.getWasInfos().contains(hash))
        return;
    if (m_wdf.getWasInfos()[hash].type != WT_PS)
        return;

    clear();

    Was was(m_wdf.getPath(), m_wdf.getWasInfos()[hash]);
    for (auto i: was.times()) {
        std::cout << i << " ";
    }
    int width = was.header().width * was.header().frameNum;
    int height = was.header().height * was.header().directionNum;
    m_sprite.texture = addTexture((void*)was.pixels().data(), width, height, 4);
    m_sprite.matrix = glm::mat4(1.f);
    m_sprite.matrix = glm::translate(m_sprite.matrix, {-width / 2, height / 2, 0});
    m_sprite.matrix = glm::scale(m_sprite.matrix, {width, height, 1.f});


    const auto &fullFrames = was.fullFrames();
    m_frameList.resize(fullFrames.size());
    for (int i = 0; i < fullFrames.size(); i++) {
        m_frameList[i].resize(fullFrames[i].size());
        for (int j = 0; j < fullFrames[i].size(); j++) {
            uint32_t texture = addTexture((void*)fullFrames[i][j].pixels.data(), fullFrames[i][j].width, fullFrames[i][j].height, 4);

            glm::mat4 mat = glm::mat4(1);

            float x = ((float)fullFrames[i][j].width / 2.f) - fullFrames[i][j].x;
            float y =  -((float)fullFrames[i][j].height / 2.f) + fullFrames[i][j].y;
            std::cout << i << " " << j << " " << fullFrames[i][j].y << " " << fullFrames[i][j].height << std::endl;
            mat = translate(mat, { x + i * 300, y, 0});
            mat = scale(mat, {fullFrames[i][j].width, fullFrames[i][j].height, 1});
            m_frameList[i][j] = {mat, texture};
        }
    }
}

void Shape::setPosition(const glm::vec2 &position) {
    m_position = position;
    m_matrix = translate(glm::mat4(1.f), glm::vec3(m_position.x, m_position.y, 0));
    m_matrix = scale(m_matrix, glm::vec3(m_scale.x, m_scale.y, 1));
}

void Shape::setScale(const glm::vec2 &scale) {
    m_scale = scale;
    m_matrix = translate(glm::mat4(1.f), glm::vec3(m_position.x, m_position.y, 0));
    m_matrix = glm::scale(m_matrix, glm::vec3(m_scale.x, m_scale.y, 1));
}

void Shape::clear() {
    for (auto &frames: m_frameList) {
        for (auto &frame: frames) {
            glDeleteTextures(1, &frame.texture);
        }
    }
    m_frameList.clear();

    // glDeleteTextures(1, &m_spirit.texture);
    // m_spirit.texture = 0;
}

void Shape::draw(const glm::mat4 &matrix) {
    glm::mat4 mat = matrix * m_matrix;
    int f = Global::time / 0.111111f;
    m_shader.use();
    glBindVertexArray(m_VAO);
    for (auto &frames: m_frameList) {
        int i = f % frames.size();
        for (int i = 0; i < frames.size(); i++) {
            glBindTexture(GL_TEXTURE_2D, frames[i].texture);
            glActiveTexture(GL_TEXTURE0);
            // m_shader.setUniform(m_uTextureLocation, 0);
            glm::mat4 m = glm::mat4(1);
            m = translate(m, { i*100, 0, 0});
            m_shader.setUniform(m_uMatrixLocation, mat * m * frames[i].matrix);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
        // glBindTexture(GL_TEXTURE_2D, frames[0].texture);
        // glActiveTexture(GL_TEXTURE0);
        // // m_shader.setUniform(m_uTextureLocation, 0);
        // m_shader.setUniform(m_uMatrixLocation, mat * frames[0].matrix);
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // glBindTexture(GL_TEXTURE_2D, m_sprite.texture);
    // glActiveTexture(GL_TEXTURE0);
    // // m_shader.setUniform(m_uTextureLocation, 0);
    // m_shader.setUniform(m_uMatrixLocation, mat * m_sprite.matrix);
    // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

uint32_t Shape::addTexture(void *buf, int width, int height, int channels) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    auto fmt = channels == 4 ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, buf);
    glGenerateMipmap(GL_TEXTURE_2D);
    return texture;
}
