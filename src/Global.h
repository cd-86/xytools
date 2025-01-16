#ifndef CORE_H
#define CORE_H
#include <string>

class Global {
public:
    static void update();

public:
    static int frameID;
    static int fps;
    static double time;
    static std::string fpsStr;

private:
    static int m_count;
    static double m_lastFrameTime;
};


#endif //CORE_H
