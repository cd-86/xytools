#ifndef SCENE_H
#define SCENE_H

#include "Map.h"
#include "Shape.h"

class Scene {
public:
    Scene();

    ~Scene();

    void drawScene();

    void resetCamera();

    void resizeEvent(int width, int height);

    void mouseButtonEvent(int button, int action, int mods, double xpos, double ypos);

    void mouseMoveEvent(double xpos, double ypos);

    void wheelEvent(double xoffset, double yoffset);

    glm::vec2 getScale() const { return m_scale; }
    glm::vec2 getTranslate() const { return m_translate; }

    Map &getMap() { return m_map; }
    Shape &getShape() { return m_shape; }

private:
    void updateMatrices();

public:
    bool mapTileVisible{true};
    bool mapMaskVisible{false};
    bool mapCellVisible{false};

private:
    Map m_map;
    Shape m_shape;

    int m_width;
    int m_height;

    int m_frameID{0};

    glm::mat4 m_projection;
    glm::mat4 m_view;

    glm::vec2 m_scale;
    glm::vec2 m_translate;
    glm::vec2 m_pressPos;
    bool m_pressFlag{false};
};


#endif //SCENE_H
