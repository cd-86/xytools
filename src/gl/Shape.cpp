#include "Shape.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <ext/matrix_transform.hpp>
#include <glad/glad.h>

#include "Global.h"
#include "xy2/was.h"

const char* VERTEX_CODE = R"(
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

const char* FRAGMENT_CODE = R"(
    #version 330 core

    out vec4 FragColor;

    in vec2 vTexCoord;

    uniform sampler2D uTexture;

    void main()
    {
	    FragColor = texture(uTexture, vTexCoord);
    }
)";

const char* LINE_VERTEX_CODE = R"(
    #version 330 core

    layout (location = 0) in vec2 aPos;

    uniform mat4 uMatrix;

    void main()
    {
	    gl_Position = uMatrix * vec4(aPos, 0.0, 1.0);
    }
)";

const char* LINE_FRAGMENT_CODE = R"(
    #version 330 core

    out vec4 FragColor;

    void main()
    {
	    FragColor = vec4(1.0, 0.0, 1.0, 0.6);
    }
)";

Shape::Shape(): m_spriteShader(&VERTEX_CODE, &FRAGMENT_CODE),
                m_lineShader(&LINE_VERTEX_CODE, &LINE_FRAGMENT_CODE),
                m_position(0.f),
                m_scale(1.f),
                m_matrix(1.f)
{
    m_uMatrixLocation = m_spriteShader.getUniformLocation("uMatrix");
    m_uTextureLocation = m_spriteShader.getUniformLocation("uTexture");
    m_uLineMatrixLocation = m_lineShader.getUniformLocation("uMatrix");

    float vertices[] = {
        -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_spriteVAO);
    glGenBuffers(1, &m_spriteVBO);
    glBindVertexArray(m_spriteVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_spriteVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float vertices2[] = {
        -80.f, 30.f, 80.f, -30.f,
        -80.f, -30.f, 80.f, 30.f
    };

    glGenVertexArrays(1, &m_lineVAO);
    glGenBuffers(1, &m_lineVBO);
    glBindVertexArray(m_lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Shape::~Shape()
{
    clear();
    glDeleteVertexArrays(1, &m_spriteVAO);
    glDeleteBuffers(1, &m_spriteVBO);

    glDeleteVertexArrays(1, &m_lineVAO);
    glDeleteBuffers(1, &m_lineVBO);
}

void Shape::loadWdf(const std::string& wdfPath)
{
    if (!m_wdf.load(wdfPath))
        return;
    m_wasList.clear();
    for (auto& was : m_wdf.getWasInfos())
    {
        std::stringstream ss;
        switch (was.second.type)
        {
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

void Shape::loadWas(uint32_t hash)
{
    if (!m_wdf.getWasInfos().contains(hash))
        return;
    if (m_wdf.getWasInfos()[hash].type != WT_PS)
        return;

    clear();

    Was was(m_wdf.getPath(), m_wdf.getWasInfos()[hash]);
    for (auto i : was.times())
    {
        std::cout << i << " ";
    }
    int width = was.header().width * was.header().frameNum;
    int height = was.header().height * was.header().directionNum;
    m_sprite.texture = addTexture((void*)was.pixels().data(), width, height, 4);
    m_sprite.matrix = glm::mat4(1.f);
    m_sprite.matrix = glm::translate(m_sprite.matrix, {-width / 2, height / 2, 0});
    m_sprite.matrix = glm::scale(m_sprite.matrix, {width, height, 1.f});


    const auto& fullFrames = was.fullFrames();
    m_frameList.resize(fullFrames.size());

    struct Pos
    {
        float x{0.f};
        float y{0.f};
    };
    Pos ps[10];
    switch (fullFrames.size())
    {
    case 2:
        ps[0].x = -100.f;
        ps[1].x = 100.f;
        break;
    case 4:
        ps[0] = {100.f, -100.f}; // rb
        ps[1] = {-100.f, -100.f}; // lb
        ps[2] = {-100.f, 100.f}; // lt
        ps[3] = {100.f, 100.f}; // rt
        break;
    case 8:
        ps[0] = {200.f, -200.f}; // rb
        ps[1] = {-200.f, -200.f}; // lb
        ps[2] = {-200.f, 200.f}; // lt
        ps[3] = {200.f, 200.f}; // rt
        ps[4] = {0.f, -200.f}; // b
        ps[5] = {-200.f, 0.f}; // l
        ps[6] = {0.f, 200.f}; // t
        ps[7] = {200.f, 0.f}; // r
        break;
    default:
        break;
    }

    for (int i = 0; i < fullFrames.size(); i++)
    {
        m_frameList[i].resize(fullFrames[i].size());

        for (int j = 0; j < fullFrames[i].size(); j++)
        {
            uint32_t texture = addTexture((void*)fullFrames[i][j].pixels.data(), fullFrames[i][j].width,
                                          fullFrames[i][j].height, 4);
            glm::mat4 mat = glm::mat4(1);
            float x = ((float)fullFrames[i][j].width / 2.f) - fullFrames[i][j].x;
            float y = -((float)fullFrames[i][j].height / 2.f) + fullFrames[i][j].y;
            mat = translate(mat, {x + ps[i].x, y + ps[i].y, 0});
            mat = scale(mat, {fullFrames[i][j].width, fullFrames[i][j].height, 1});

            glm::mat4 orimat = glm::mat4(1);
            orimat = translate(orimat, {ps[i].x, ps[i].y, 0});
            m_frameList[i][j] = {orimat, mat, texture};
        }
    }
}

void Shape::setPosition(const glm::vec2& position)
{
    m_position = position;
    m_matrix = translate(glm::mat4(1.f), glm::vec3(m_position.x, m_position.y, 0));
    m_matrix = scale(m_matrix, glm::vec3(m_scale.x, m_scale.y, 1));
}

void Shape::setScale(const glm::vec2& scale)
{
    m_scale = scale;
    m_matrix = translate(glm::mat4(1.f), glm::vec3(m_position.x, m_position.y, 0));
    m_matrix = glm::scale(m_matrix, glm::vec3(m_scale.x, m_scale.y, 1));
}

void Shape::clear()
{
    for (auto& frames : m_frameList)
    {
        for (auto& frame : frames)
        {
            glDeleteTextures(1, &frame.texture);
        }
    }
    m_frameList.clear();

    // glDeleteTextures(1, &m_spirit.texture);
    // m_spirit.texture = 0;
}

void Shape::draw(const glm::mat4& matrix)
{
    glm::mat4 mat = matrix * m_matrix;
    int f = Global::time / 0.1f;
    for (auto& frames : m_frameList)
    {
        int i = f % frames.size();
        m_spriteShader.use();
        glBindVertexArray(m_spriteVAO);
        glBindTexture(GL_TEXTURE_2D, frames[i].texture);
        glActiveTexture(GL_TEXTURE0);
        // m_shader.setUniform(m_uTextureLocation, 0);
        m_spriteShader.setUniform(m_uMatrixLocation, mat * frames[i].matrix);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        m_lineShader.use();
        glBindVertexArray(m_lineVAO);
        m_lineShader.setUniform(m_uLineMatrixLocation, mat * frames[i].oriMatrix);
        glDrawArrays(GL_LINES, 0, 4);
    }
}

uint32_t Shape::addTexture(void* buf, int width, int height, int channels)
{
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
