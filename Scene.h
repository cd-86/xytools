#ifndef SCENE_H
#define SCENE_H

#include <iostream>
#include <map>
#include <glm.hpp>

struct Tile
{
    std::string texture;
    glm::mat4 matrix;
};


class Scene
{
public:
    Scene();
    ~Scene();

    void clear();

    void addTexture(const std::string& key, const std::string& imgPath);

    void addTexture(const std::string& key, unsigned char* buf, int size);

    void addTile(const std::string& key, const std::string& textureKey, const glm::mat4& mat);

    void drawScene();

private:
    unsigned int m_program;
    unsigned int m_vbo;
    unsigned int m_vao;
    int m_uMatrixLocation;
    int m_uTextureLocation;

    glm::mat4 m_projection;
    glm::mat4 m_view;

    std::map<std::string, unsigned int> m_textureMap;
    std::map<std::string, Tile> m_tileMap;
};


#endif //SCENE_H
