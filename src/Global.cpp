//
// Created by chend on 2025/1/12.
//

#include "Global.h"

#include <sstream>

#include "GLFW/glfw3.h"

int Global::frameID{0};
int Global::fps{0};
double Global::time{0.0};
std::string Global::fpsStr;

int Global::m_count{0};
double Global::m_lastFrameTime;

void Global::update() {
    frameID++;
    m_count++;
    time = glfwGetTime();
    double deltaTime = time - m_lastFrameTime;
    if (deltaTime >= 0.25f) {
        fps = m_count / deltaTime;
        std::stringstream ss;
        ss << "FPS: " << fps;
        fpsStr = ss.str();
        m_count = 0;
        m_lastFrameTime = time;
    }
}
