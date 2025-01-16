#ifndef SHAPE_H
#define SHAPE_H
#include <vector>
#include <glm.hpp>
#include "Shader.h"
#include "xy2/wdf.h"

struct ShapeFrame {
    glm::mat4 matrix;
    unsigned int texture;
};

class Shape {
public:
    Shape();

    ~Shape();

    void loadWdf(const std::string &wdfPath);

    void loadWas(uint32_t hash);

    const std::vector<std::pair<std::string, uint32_t> > &wasList() { return m_wasList; }

    void setPosition(const glm::vec2 &position);

    void setScale(const glm::vec2 &scale);

    void clear();

    void draw(const glm::mat4 &matrix);

private:
    uint32_t addTexture(void *buf, int width, int height, int channels);

private:
    Shader m_shader;
    unsigned int m_VAO;
    unsigned int m_VBO;

    int m_uMatrixLocation;
    int m_uTextureLocation;

    glm::vec2 m_position;
    glm::vec2 m_scale;
    glm::mat4 m_matrix;

    std::vector<std::vector<ShapeFrame>> m_frameList;
    ShapeFrame m_sprite;

    Wdf m_wdf;
    std::vector<std::pair<std::string, uint32_t> > m_wasList;
};


#endif //SHAPE_H
