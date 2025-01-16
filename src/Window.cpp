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

#include "Global.h"
#include "xy2/mapx.h"

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

    m_window = glfwCreateWindow(1200, 800, "XY Tools", nullptr, nullptr);
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

    updateFileList(std::filesystem::current_path());

    glfwSetWindowSizeCallback(m_window, WindowSizeCallBack);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetCursorPosCallback(m_window, CursorPosCallback);
    glfwSetScrollCallback(m_window, ScrollCallback);

    glfwSetDropCallback(m_window, DropCallback);

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
            if (ImGui::MenuItem("WDF", nullptr, m_wdfWindowVisible)) {
                m_wdfWindowVisible = !m_wdfWindowVisible;
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
            std::stringstream ss;
            ss << "x: " << m_scene->getTranslate().x << " y: " << m_scene->getTranslate().y << " | " << (
                1 / m_scene->getScale().x * 100) << "%%" << " | " << Global::fpsStr;
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
        ImGui::SetNextWindowSize(ImVec2(250.f, ImGui::GetMainViewport()->Size.y - ImGui::GetFrameHeight()),
                                 ImGuiCond_Appearing);

        ImGui::Begin("文件列表", &m_fileListWindowVisible);
        if (ImGui::Button("<", {30.f, 0.f})) {
            updateFileList(m_fileListStatus.currentDirectory.parent_path());
        }
        ImGui::SameLine();

        ImGui::Text((char *) m_fileListStatus.currentDirectory.u8string().c_str());
        ImGui::Separator();
        ImGui::BeginChild("FilesWindow");
        std::filesystem::path path;
        int flag = 0;
        for (int i = 0; i < m_fileListStatus.fileList.size(); i++) {
            if (ImGui::Selectable(m_fileListStatus.fileList[i].first.c_str(), i == m_fileListStatus.seletedIndex)) {
                if (m_fileListStatus.fileList[i].first.starts_with("[D]")) {
                    path = m_fileListStatus.fileList[i].second;
                    flag = 1;
                }
                m_fileListStatus.seletedIndex = i;
                if (m_fileListStatus.fileList[i].first.ends_with(".map")) {
                    path = m_fileListStatus.fileList[i].second;
                    flag = 2;
                }
                auto p = ++m_fileListStatus.fileList[i].first.crbegin();
                if (*p == 'd' && *(++p) == 'w') {
                    path = m_fileListStatus.fileList[i].second;
                    flag = 3;
                }
            }
        }

        if (flag == 1) {
            updateFileList(path);
        } else if (flag == 2) {
            m_scene->getMap().loadMap(path.string());
        } else if (flag == 3) {
            m_scene->getShape().loadWdf(path.string());
        }

        ImGui::EndChild();
        ImGui::End();
    }

    // Wdf 文件列表
    if (m_wdfWindowVisible) {
        // 设置窗口的位置和大小
        ImGui::SetNextWindowPos(ImVec2(250, ImGui::GetFrameHeight()), ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(200.f, ImGui::GetMainViewport()->Size.y - ImGui::GetFrameHeight()),
                                 ImGuiCond_Appearing);

        if (ImGui::Begin("WDF", &m_wdfWindowVisible)) {
            for (int i = 0; i < m_scene->getShape().wasList().size(); i++) {
                if (ImGui::Selectable(m_scene->getShape().wasList()[i].first.c_str())) {
                    m_scene->getShape().loadWas(m_scene->getShape().wasList()[i].second);
                }
            }
            ImGui::End();
        }
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
        ImGui::LabelText("地图宽", "%d", m_scene->getMap().mapWidth());
        ImGui::LabelText("地图高", "%d", m_scene->getMap().mapHeight());
        ImGui::LabelText("Block 行", "%d", m_scene->getMap().mapBockRowCount());
        ImGui::LabelText("Block 列", "%d", m_scene->getMap().mapBockColCount());
        ImGui::LabelText("Block 宽", "%d", m_scene->getMap().mapBlockWidth());
        ImGui::LabelText("Block 高", "%d", m_scene->getMap().mapBlockHeight());

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
        Global::update();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_scene->drawScene();

        drawUI();

        glfwSwapBuffers(m_window);

        glfwPollEvents();
    }

    clearn();

    return 0;
}

void Window::updateFileList(const std::filesystem::path &dir) {
    try {
        std::vector<std::pair<std::string, std::filesystem::path> > files;
        for (const auto &file: std::filesystem::directory_iterator(dir)) {
            std::stringstream ss;
            ss << (file.is_directory() ? "[D] " : "[F] ");
            ss << (char *) file.path().filename().u8string().c_str();
            files.emplace_back(ss.str(), file);
        }
        std::ranges::sort(files);
        m_fileListStatus.currentDirectory = dir;
        m_fileListStatus.seletedIndex = 0;
        m_fileListStatus.fileList = files;
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
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

void Window::DropCallback(GLFWwindow *window, int path_count, const char *paths[]) {
    std::string path = paths[0];
    if (path.ends_with(".map")) {
        std::cout << "MAP: " << path << std::endl;
        return;
    }
    auto p = ++path.crbegin();
    if (*p == 'd' && *(++p) == 'w') {
        std::cout << "WDF: " << path << std::endl;
    }
}
