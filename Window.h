//
// Created by chend on 2025/1/2.
//

#ifndef WINDOW_H
#define WINDOW_H

struct GLFWwindow;
class Scene;

class Window
{
public:
    Window();
    ~Window();

    bool initWindow();

    void clearn();

    void drawUI();

    int run();

private:
    int eventLoop();

    static void ErrorCallback(int error_code, const char* description);
    static void WindowSizeCallBack(GLFWwindow* window, int width, int height);

private:
    GLFWwindow* m_window;
    Scene* m_scene;

    // 是否需要退出
    bool m_shouldClose{false};

    struct
    {
        int count{0};
        int fps {0};
        double lastFrameTime{0};
        char fpsStr[16];
    } m_fps;
};


#endif //WINDOW_H
