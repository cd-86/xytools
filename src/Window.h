#ifndef WINDOW_H
#define WINDOW_H
#include <filesystem>
#include <string>
#include <vector>

#include "xy2/wdf.h"

struct GLFWwindow;
class Scene;

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

    void updateFileList(const std::filesystem::path &dir);

    void loadWdf(const std::string &wdf);

    static void ErrorCallback(int error_code, const char *description);

    static void WindowSizeCallBack(GLFWwindow *window, int width, int height);

    static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    static void CursorPosCallback(GLFWwindow *window, double xpos, double ypos);

    static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);

    static void DropCallback(GLFWwindow* window, int path_count, const char* paths[]);

private:
    GLFWwindow *m_window;
    Scene *m_scene;

    // 是否需要退出
    bool m_shouldClose{false};
    bool m_fileListWindowVisible{true};
    bool m_attributeWindowVisible{true};
    bool m_wdfWindowVisible{true};

    struct {
        std::filesystem::path currentDirectory;
        std::vector<std::pair<std::string, std::filesystem::path>> fileList;
        int seletedIndex{-1};
    } m_fileListStatus;
};

#endif //WINDOW_H
