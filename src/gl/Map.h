#ifndef MAP_H
#define MAP_H
#include <string>
#include <glm.hpp>
#include <vector>

#include "Shader.h"

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

private:
    void drawTile(const glm::mat4 &matrix);
    void drawMask(const glm::mat4 &matrix);
    void drawCell(const glm::mat4 &matrix);
    unsigned int addTexture(void *buf, int width, int height, int channels);

public:
    int mapWidth{0};
    int mapHeight{0};

    int mapBockRowCount{0};
    int mapBockColCount{0};
    int mapBlockWidth{0};
    int mapBlockHeight{0};

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

    std::vector<Tile> m_tiles;
    std::vector<Tile> m_masks;
};

#endif //MAP_H
