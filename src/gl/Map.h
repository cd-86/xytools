#ifndef MAP_H
#define MAP_H
#include <string>
#include <glm.hpp>
#include <map>
#include <vector>

#include "Shader.h"
#include "xy2/mapx.h"

class MapX;

class Map {
    friend class Scene;

    struct Tile {
        glm::mat4 matrix;
        unsigned int texture;
    };

public:
    Map();

    ~Map();

    void loadMap(const std::string &mapPath);

    void clear();

    void setPosition(const glm::vec2 &position);

    void setScale(const glm::vec2 &scale);

    int mapWidth() const { return m_map ? m_map->GetWidth() : 0; }
    int mapHeight() const { return m_map ? m_map->GetHeight() : 0; }

    int mapBockRowCount() const { return m_map ? m_map->GetRowCount() : 0; }
    int mapBockColCount() const { return m_map ? m_map->GetColCount() : 0; }
    int mapBlockWidth() const { return m_map ? m_map->GetBlockWidth() : 0; }
    int mapBlockHeight() const { return m_map ? m_map->GetBlockHeight() : 0; }

private:
    void updateFrameProp(const glm::mat4 &matrix, int frameNum) {
        m_frame.matrix = matrix * m_matrix;
        glm::mat4 inverMat = glm::inverse(m_frame.matrix);
        glm::vec4 topLeft = inverMat * glm::vec4(-1, 1, 0, 1);
        glm::vec4 bottomRight = inverMat * glm::vec4(1, -1, 0, 1);
        m_frame.left = topLeft.x;
        m_frame.top = topLeft.y;
        m_frame.right = bottomRight.x;
        m_frame.bottom = bottomRight.y;
    }

    void drawTile(const glm::mat4 &matrix);

    void drawMask(const glm::mat4 &matrix);

    void drawCell(const glm::mat4 &matrix);

    unsigned int addTexture(void *buf, int width, int height, int channels);

public:
    int pointSize{2};

private:
    Shader m_tileShader;
    Shader m_pointShader;
    unsigned int m_tileVAO;
    unsigned int m_tileVBO;
    int m_uTileMatrixLocation;
    int m_uTileTextureLocation;

    unsigned int m_pointVAO;
    unsigned int m_pointVBO;
    unsigned int m_pointCount{0};
    int m_uPointMatrixLocation;
    int m_uPointSizeLocation;

    glm::vec2 m_position;
    glm::vec2 m_scale;
    glm::mat4 m_matrix;

    struct {
        int frameID;
        glm::mat4 matrix;
        float left;
        float top;
        float right;
        float bottom;
    } m_frame;

    MapX *m_map{nullptr};
    std::map<int, Tile> m_tiles;
    std::vector<Tile> m_masks;
};

#endif //MAP_H
