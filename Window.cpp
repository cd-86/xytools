//
// Created by chend on 2025/1/2.
//

#include "Window.h"

#include "Scene.h"

// 翻译
#include "trans_zh_cn.h"
#include <implot.h>
#include <iostream>
#include <ostream>
#include <glad/glad.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "ImGuiFileDialog.h"

Window::Window()
{
    glfwSetErrorCallback(ErrorCallback);
}

Window::~Window()
{
}

bool Window::initWindow()
{
    if (!glfwInit())
    {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(640, 480, "XY Tools", nullptr, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(m_window, this);

    glfwMakeContextCurrent(m_window);

    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
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

    glfwSetWindowSizeCallback(m_window, WindowSizeCallBack);

    return true;
}

void Window::clearn()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    ImPlot::DestroyContext();

    delete m_scene;

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::drawUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    // 菜单栏
    {
        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("文件"))
        {
            if (ImGui::MenuItem("打开文件目录"))
            {
                IGFD::FileDialogConfig config;
                config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_DisableCreateDirectoryButton;
                config.path = ".";
                ImGuiFileDialog::Instance()->OpenDialog("ChooseDirectory", "选择文件目录", nullptr, config);
            }
            if (ImGui::MenuItem("退出"))
            {
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
        // 帧数
        {
            m_fps.count++;
            double deltaTime = glfwGetTime() - m_fps.lastFrameTime;
            if (deltaTime >= 0.25f)
            {
                m_fps.fps = m_fps.count / deltaTime;
                sprintf(m_fps.fpsStr, "FPS: %d", m_fps.fps);
                m_fps.count = 0;
                m_fps.lastFrameTime = glfwGetTime();
            }
            ImGui::SameLine(
                ImGui::GetWindowWidth() - ImGui::CalcTextSize(m_fps.fpsStr).x - ImGui::GetStyle().FramePadding.x * 4);
            ImGui::Text(m_fps.fpsStr);
        }

        ImGui::EndMainMenuBar();
    }

    // 左侧树形列表
    {
        // 设置窗口的位置和大小
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()), ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(250.f,  ImGui::GetWindowHeight()), ImGuiCond_Appearing);

        ImGui::Begin("文件列表");

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
        if (ImGui::BeginPopupModal("退出", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("是否退出？");
            ImGui::Separator();

            if (ImGui::Button("确定", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                m_shouldClose = true;
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("取消", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(m_window, GLFW_FALSE);
            }
            ImGui::EndPopup();
        }
    }
    else if (ImGuiFileDialog::Instance()->Display("ChooseDirectory",  ImGuiWindowFlags_NoCollapse, {500, 400})) // 文件对话框
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            // action if OK
            // std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            std::cout << filePath << std::endl;
        }

        // close
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int Window::run()
{
    if (!initWindow())
    {
        return -1;
    }

    glClearColor(0.2, 0.2, 0.2, 0.2);

    return eventLoop();
}

int Window::eventLoop()
{

    // TEST
    m_scene->addTexture("img", R"(C:\Users\chend\Desktop\70.jpg)");
    m_scene->addTile("tile", "img", glm::mat4(1));


    while (!m_shouldClose)
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_scene->drawScene();

        drawUI();

        glfwSwapBuffers(m_window);

        glfwPollEvents();
    }

    clearn();

    return 0;
}

void Window::ErrorCallback(int error_code, const char* description)
{
    std::cerr << "[" << error_code << "] " << description << std::endl;
}

void Window::WindowSizeCallBack(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
