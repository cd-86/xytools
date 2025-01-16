#include "Scene.h"

#include <gtc/matrix_transform.hpp>
#include "GLFW/glfw3.h"


Scene::Scene() {
    m_projection = glm::mat4(1.0f);
    m_view = glm::mat4(1.0f);
}

Scene::~Scene() {
}

void Scene::drawScene() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_frameID++;
    auto mat = m_projection * m_view;
    if (mapTileVisible)
        m_map.drawTile(mat);

    if (mapMaskVisible)
        m_map.drawMask(mat);

    if (mapCellVisible)
        m_map.drawCell(mat);

    m_shape.draw(mat);
}

void Scene::resetCamera() {
    m_scale.x = 1.0f;
    m_scale.y = 1.0f;

    m_translate.x = 0.0f;
    m_translate.y = 0.0f;

    updateMatrices();
}

void Scene::resizeEvent(int width, int height) {
    m_width = width;
    m_height = height;

    glViewport(0, 0, width, height);
    updateMatrices();
}

void Scene::mouseButtonEvent(int button, int action, int mods, double xpos, double ypos) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            m_pressFlag = true;
            m_pressPos.x = xpos;
            m_pressPos.y = ypos;
        } else if (action == GLFW_RELEASE) {
            m_pressFlag = false;
            updateMatrices();
        }
    }
}

void Scene::mouseMoveEvent(double xpos, double ypos) {
    if (m_pressFlag) {
        float xoff = (m_pressPos.x - xpos) * m_scale.x;
        float yoff = (ypos - m_pressPos.y) * m_scale.y;

        m_pressPos.x = xpos;
        m_pressPos.y = ypos;

        m_translate.x += xoff;
        m_translate.y += yoff;
        updateMatrices();
    }
}

void Scene::wheelEvent(double xoffset, double yoffset) {
    m_scale *= xoffset + yoffset > 0 ? 0.9 : 1.1;
    updateMatrices();
}

void Scene::updateMatrices() {
    float w = m_width / 2.0f * m_scale.x;
    float h = m_height / 2.0f * m_scale.y;
    m_projection = glm::ortho(-w, w, -h, h, 0.1f, 100.f);
    m_view = glm::lookAt(glm::vec3(m_translate, 1.f), glm::vec3(m_translate, -1.f), glm::vec3(0, 1, 0));
}
