//
// Created by chend on 2025/1/2.
//

#include "Window.h"
#include <filesystem>

#include "gl/Scene.h"

#include <implot.h>
#include <iostream>
#include <ext/matrix_transform.hpp>
#include <glad/glad.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

Window::Window() {
    glfwSetErrorCallback(ErrorCallback);
}

Window::~Window() {
}

bool Window::initWindow() {
    if (!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(640, 480, "XY Tools", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(m_window, this);

    glfwMakeContextCurrent(m_window);

    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        glfwTerminate();
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    updateFileList();

    glfwSetWindowSizeCallback(m_window, WindowSizeCallBack);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetCursorPosCallback(m_window, CursorPosCallback);
    glfwSetScrollCallback(m_window, ScrollCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    // 禁用保存界面布局
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();
    // ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init();

    io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\msyh.ttc)", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());

    ImPlot::CreateContext();

    m_scene = new Scene();

    int w, h;
    glfwGetWindowSize(m_window, &w, &h);
    m_scene->resizeEvent(w, h);
    m_scene->resetCamera();

    return true;
}

void Window::clearn() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    ImPlot::DestroyContext();

    delete m_scene;

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::drawUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 菜单栏
    {
        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("文件")) {
            if (ImGui::MenuItem("退出")) {
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("视图")) {
            if (ImGui::MenuItem("文件列表", nullptr, m_fileListWindowVisible)) {
                m_fileListWindowVisible = !m_fileListWindowVisible;
            }
            if (ImGui::MenuItem("属性", nullptr, m_attributeWindowVisible)) {
                m_attributeWindowVisible = !m_attributeWindowVisible;
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::Button("重置视图")) {
            m_scene->resetCamera();
        }

        // 帧数 和 scene 缩放比例
        {
            m_fps.count++;
            double deltaTime = glfwGetTime() - m_fps.lastFrameTime;
            if (deltaTime >= 0.25f) {
                m_fps.fps = m_fps.count / deltaTime;
                std::stringstream ss;
                ss << "FPS: " << m_fps.fps;
                m_fps.fpsStr = ss.str();
                m_fps.count = 0;
                m_fps.lastFrameTime = glfwGetTime();
            }
            std::stringstream ss;
            ss << "x: " << m_scene->getTranslate().x << " y: " << m_scene->getTranslate().y << " | " << (
                1 / m_scene->getScale().x * 100) << "%%" << " | " << m_fps.fpsStr;
            auto s = ss.str();
            ImGui::SameLine(
                ImGui::GetWindowWidth() - ImGui::CalcTextSize(s.c_str()).x - ImGui::GetStyle().FramePadding.x * 4);
            ImGui::Text(s.c_str());
        }

        ImGui::EndMainMenuBar();
    }

    // 左侧树形列表
    if (m_fileListWindowVisible) {
        // 设置窗口的位置和大小
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()), ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(250.f, ImGui::GetWindowHeight()), ImGuiCond_Appearing);

        ImGui::Begin("文件列表", &m_fileListWindowVisible);
        if (ImGui::Button("<", {30.f, 0.f})) {
            updateFileList(std::filesystem::path(m_fileListStatus.currentDirectory).parent_path().string());
        }
        ImGui::SameLine();
        ImGui::Text(m_fileListStatus.currentDirectory.c_str());
        ImGui::Separator();
        ImGui::BeginChild("FilesWindow");
        for (int i = 0; i < m_fileListStatus.fileList.size(); i++) {
            if (ImGui::Selectable(m_fileListStatus.fileList[i].name.c_str(), i == m_fileListStatus.seletedIndex)) {
                if (m_fileListStatus.fileList[i].isDirectory) {
                    updateFileList(m_fileListStatus.fileList[i].path);
                    break;
                }
                m_fileListStatus.seletedIndex = i;
                if (m_fileListStatus.fileList[i].name.ends_with(".map")) {
                    m_scene->getMap().loadMap(m_fileListStatus.fileList[i].path);
                }
            }
        }
        ImGui::EndChild();
        ImGui::End();
    }

    // 右侧属性列表
    if (m_attributeWindowVisible) {
        int w, h;
        glfwGetWindowSize(m_window, &w, &h);
        // 设置窗口的位置和大小
        ImGui::SetNextWindowPos(ImVec2(w, ImGui::GetFrameHeight()), ImGuiCond_Appearing, {1, 0});
        ImGui::SetNextWindowSize(ImVec2(250.f, ImGui::GetWindowHeight()), ImGuiCond_Appearing);

        ImGui::Begin("属性", &m_attributeWindowVisible);
        // 地图
        ImGui::SeparatorText("地图:");
        ImGui::Checkbox("显示地图", &m_scene->mapTileVisible);
        ImGui::Checkbox("显示遮罩", &m_scene->mapMaskVisible);
        ImGui::Checkbox("显示 Cell", &m_scene->mapCellVisible);
        ImGui::SliderInt("Cell 点大小", &m_scene->getMap().pointSize, 1, 10);
        ImGui::Separator();
        ImGui::LabelText("地图宽", "%d", m_scene->getMap().mapWidth);
        ImGui::LabelText("地图高", "%d", m_scene->getMap().mapHeight);
        ImGui::LabelText("Block 行", "%d", m_scene->getMap().mapBockRowCount);
        ImGui::LabelText("Block 列", "%d", m_scene->getMap().mapBockColCount);
        ImGui::LabelText("Block 宽", "%d", m_scene->getMap().mapBlockWidth);
        ImGui::LabelText("Block 高", "%d", m_scene->getMap().mapBlockHeight);

        ImGui::End();
    }


    // ImGui::ShowDemoWindow();
    //
    // ImPlot::ShowDemoWindow();

    // 模态窗口
    if (glfwWindowShouldClose(m_window)) // 退出对话框
    {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        ImGui::OpenPopup("退出");
        if (ImGui::BeginPopupModal("退出", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("是否退出？");
            ImGui::Separator();

            if (ImGui::Button("确定", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                m_shouldClose = true;
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("取消", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(m_window, GLFW_FALSE);
            }
            ImGui::EndPopup();
        }
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int Window::run() {
    if (!initWindow()) {
        return -1;
    }

    glClearColor(0.2, 0.2, 0.2, 0.2);

    return eventLoop();
}

int Window::eventLoop() {
    while (!m_shouldClose) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_scene->drawScene();

        drawUI();

        glfwSwapBuffers(m_window);

        glfwPollEvents();
    }

    clearn();

    return 0;
}

void Window::updateFileList(const std::string &dir) {
    m_fileListStatus.currentDirectory = dir.empty() ? std::filesystem::current_path().string() : dir;
    m_fileListStatus.seletedIndex = -1;
    m_fileListStatus.fileList.clear();
    for (const auto &file: std::filesystem::directory_iterator(m_fileListStatus.currentDirectory)) {
        m_fileListStatus.fileList.emplace_back(
            file.path().filename().string(),
            file.path().string(),
            file.is_directory()
        );
    }
}

void Window::ErrorCallback(int error_code, const char *description) {
    std::cerr << "[" << error_code << "] " << description << std::endl;
}

void Window::WindowSizeCallBack(GLFWwindow *window, int width, int height) {
    auto *w = static_cast<Window *>(glfwGetWindowUserPointer(window));
    w->m_scene->resizeEvent(width, height);
}

void Window::MouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    auto *w = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS && ImGui::GetIO().WantCaptureMouse)
        return;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    w->m_scene->mouseButtonEvent(button, action, mods, xpos, ypos);
}

void Window::CursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
    auto *w = static_cast<Window *>(glfwGetWindowUserPointer(window));
    w->m_scene->mouseMoveEvent(xpos, ypos);
}

void Window::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    auto *w = static_cast<Window *>(glfwGetWindowUserPointer(window));
    if (ImGui::GetIO().WantCaptureMouse)
        return;
    w->m_scene->wheelEvent(xoffset, yoffset);
}
