#include "Map.h"

#include <iostream>
#include <sstream>
#include <ext/matrix_transform.hpp>
#include <glad/glad.h>

#include "Global.h"
#include "xy2/mapx.h"

const char *TILE_VERTEX_CODE = R"(
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

const char *TILE_FRAGMENT_CODE = R"(
    #version 330 core

    out vec4 FragColor;

    in vec2 vTexCoord;

    uniform sampler2D uTexture;

    void main()
    {
	    FragColor = texture(uTexture, vTexCoord);
    }
)";

const char *POINT_VERTEX_CODE = R"(
    #version 330 core

    layout (location = 0) in vec3 aPos;

    uniform int uPointSize;
    uniform mat4 uMatrix;

    out vec4 vColor;

    void main()
    {
        gl_PointSize = uPointSize;
	    gl_Position = uMatrix * vec4(aPos.xy, 0.0, 1.0);

        switch (int(aPos.z)) {
            case 0:
                vColor = vec4(1.0, 1.0, 1.0, 1.0); // 白色
                break;
            case 1:
                vColor = vec4(1.0, 0.0, 0.0, 1.0); // 红色
                break;
            case 2:
                vColor = vec4(1.0, 1.0, 0.0, 1.0); // 黄色
                break;
            default:
                vColor = vec4(0.0, 0.0, 0.0, 1.0); // 黑色
        }
    }
)";

const char *POINT_FRAGMENT_CODE = R"(
    #version 330 core

    out vec4 FragColor;

    in vec4 vColor;

    void main()
    {

	    FragColor = vColor;
    }
)";

Map::Map(): m_tileShader(&TILE_VERTEX_CODE, &TILE_FRAGMENT_CODE),
            m_pointShader(&POINT_VERTEX_CODE, &POINT_FRAGMENT_CODE),
            m_position(0.f),
            m_scale(1.f),
            m_matrix(1.f) {
    m_uTileMatrixLocation = m_tileShader.getUniformLocation("uMatrix");
    m_uTileTextureLocation = m_tileShader.getUniformLocation("uTexture");

    m_uPointMatrixLocation = m_pointShader.getUniformLocation("uMatrix");
    m_uPointSizeLocation = m_pointShader.getUniformLocation("uPointSize");

    float vertices[] = {
        -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_tileVAO);
    glGenBuffers(1, &m_tileVBO);
    glBindVertexArray(m_tileVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_tileVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &m_pointVAO);
    glGenBuffers(1, &m_pointVBO);
    glBindVertexArray(m_pointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_pointVBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Map::~Map() {
    clear();
    glDeleteVertexArrays(1, &m_tileVAO);
    glDeleteBuffers(1, &m_tileVBO);
    glDeleteVertexArrays(1, &m_pointVAO);
    glDeleteBuffers(1, &m_pointVBO);
}

void Map::loadMap(const std::string &mapPath) {
    clear();
    m_map = new MapX(mapPath, 0);

    for (int i = 0; i < m_map->GetMaskCount(); i++) {
        // map.ReadMask(i); // 这个会越界崩溃
        m_map->ReadMaskOrigin(i);

        auto info = m_map->GetMaskInfo(i);

        unsigned int texture = addTexture(m_map->GetMaskRGBA(i), info->Width, info->Height, 4);

        glm::mat4 mat = glm::mat4(1);
        mat = translate(mat, {info->StartX + info->Width / 2.f, -info->StartY - info->Height / 2.f, 0});
        mat = scale(mat, {info->Width, info->Height, 1});

        m_masks.push_back({mat, texture});
        m_map->EraseMaskRGB(i);
    }

    std::vector<glm::vec3> vertices(m_map->GetCellColCount() * m_map->GetCellRowCount());
    uint32_t *cell = m_map->GetCell();
    for (int row = 0; row < m_map->GetCellRowCount(); row++) {
        for (int col = 0; col < m_map->GetCellColCount(); col++) {
            vertices[row * m_map->GetCellColCount() + col] = glm::vec3(col * 20 + 10, -row * 20 - 10, *cell);
            cell++;
        }
    }
    m_pointCount = vertices.size();
    glBindBuffer(GL_ARRAY_BUFFER, m_pointVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 让地图居中
    // setPosition({-m_map->GetWidth() / 2.f, m_map->GetHeight() / 2.f});
}

void Map::clear() {
    m_pointCount = 0;
    if (m_map) {
        delete m_map;
        m_map = nullptr;
    }
    for (auto &tile: m_tiles) {
        glDeleteTextures(1, &tile.second.texture);
    }
    m_tiles.clear();
    for (auto &mask: m_masks) {
        glDeleteTextures(1, &mask.texture);
    }
    m_masks.clear();
}

void Map::setPosition(const glm::vec2 &position) {
    m_position = position;
    m_matrix = translate(glm::mat4(1.f), glm::vec3(m_position.x, m_position.y, 0));
    m_matrix = scale(m_matrix, glm::vec3(m_scale.x, m_scale.y, 1));
}

void Map::setScale(const glm::vec2 &scale) {
    m_scale = scale;
    m_matrix = translate(glm::mat4(1.f), glm::vec3(m_position.x, m_position.y, 0));
    m_matrix = glm::scale(m_matrix, glm::vec3(m_scale.x, m_scale.y, 1));
}

void Map::drawTile(const glm::mat4 &matrix) {
    if (!m_map)
        return;
    if (Global::frameID != m_frame.frameID)
        updateFrameProp(matrix, Global::frameID);

    m_tileShader.use();
    glBindVertexArray(m_tileVAO);

    int left = glm::clamp((int) m_frame.left / m_map->GetBlockWidth(), 0, m_map->GetColCount());
    int top = glm::clamp((int) -m_frame.top / m_map->GetBlockHeight(), 0, m_map->GetRowCount());
    int right = glm::clamp((int) ceil(m_frame.right / m_map->GetBlockWidth()), 0, m_map->GetColCount());
    int bottom = glm::clamp((int) ceil(-m_frame.bottom / m_map->GetBlockHeight()), 0, m_map->GetRowCount());

    for (int t = top; t < bottom; t++) {
        for (int l = left; l < right; l++) {
            int i = t * m_map->GetColCount() + l;
            if (!m_tiles.contains(i)) {
                if (!m_map->ReadJPEG(i)) {
                    m_tiles[i] = {0,0};
                    continue;
                }
                auto texture = addTexture(m_map->GetJPEGRGB(i), m_map->GetBlockWidth(), m_map->GetBlockHeight(),
                                          3);
                glm::mat4 mat = glm::mat4(1);
                mat = translate(mat, {
                                    l * m_map->GetBlockWidth() + m_map->GetBlockWidth() / 2.0,
                                    -t * m_map->GetBlockHeight() - m_map->GetBlockHeight() / 2.0, 0
                                });
                mat = scale(mat, {m_map->GetBlockWidth(), m_map->GetBlockHeight(), 1});
                m_tiles[i] = {mat, texture};

                m_map->EraseJPEGRGB(i);
            }
            if (!m_tiles[i].texture)
                continue;
            glBindTexture(GL_TEXTURE_2D, m_tiles[i].texture);
            glActiveTexture(GL_TEXTURE0);
            // m_tileShader.setUniform(m_uTextureLocation, 0);
            m_tileShader.setUniform(m_uTileMatrixLocation, m_frame.matrix * m_tiles[i].matrix);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }
}

void Map::drawMask(const glm::mat4 &matrix) {
    if (!m_map)
        return;
    if (Global::frameID != m_frame.frameID)
        updateFrameProp(matrix, Global::frameID);
    m_tileShader.use();
    glBindVertexArray(m_tileVAO);
    for (auto &tile: m_masks) {
        glBindTexture(GL_TEXTURE_2D, tile.texture);
        glActiveTexture(GL_TEXTURE0);
        // m_tileShader.setUniform(m_uTextureLocation, 0);
        m_tileShader.setUniform(m_uTileMatrixLocation, m_frame.matrix * tile.matrix);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void Map::drawCell(const glm::mat4 &matrix) {
    if (!m_map)
        return;
    if (Global::frameID != m_frame.frameID)
        updateFrameProp(matrix, Global::frameID);
    m_pointShader.use();
    glBindVertexArray(m_pointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_pointVBO);
    m_pointShader.setUniform(m_uPointMatrixLocation, m_frame.matrix);
    m_pointShader.setUniform(m_uPointSizeLocation, pointSize);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS, 0, m_pointCount);
    glDisable(GL_PROGRAM_POINT_SIZE);
}

unsigned int Map::addTexture(void *buf, int width, int height, int channels) {
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
