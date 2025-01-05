#ifndef WINDOW_H
#define WINDOW_H
#include <string>
#include <vector>

struct GLFWwindow;
class Scene;

struct FileInfo {
    std::string name;
    std::string path;
    bool isDirectory;
};

class Window {
public:
    Window();

    ~Window();

    bool initWindow();

    void clearn();

    void drawUI();

    int run();

private:
    int eventLoop();

    void updateFileList(const std::string &dir = std::string());

    static void ErrorCallback(int error_code, const char *description);

    static void WindowSizeCallBack(GLFWwindow *window, int width, int height);

    static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    static void CursorPosCallback(GLFWwindow *window, double xpos, double ypos);

    static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);

private:
    GLFWwindow *m_window;
    Scene *m_scene;

    // 是否需要退出
    bool m_shouldClose{false};
    bool m_fileListWindowVisible{true};
    bool m_attributeWindowVisible{true};

    struct {
        int count{0};
        int fps{0};
        double lastFrameTime{0};
        std::string fpsStr;
    } m_fps;

    struct {
        std::string currentDirectory;
        std::vector<FileInfo> fileList;
        int seletedIndex{-1};
    } m_fileListStatus;
};

#endif //WINDOW_H
