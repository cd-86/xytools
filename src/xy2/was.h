#ifndef WAS_H
#define WAS_H
#include <vector>

#include "wdf.h"

struct WasHeader {
    uint16_t flag;
    int16_t headSize;
    int16_t directionNum;
    int16_t frameNum;
    int16_t width;
    int16_t height;
    int16_t x;
    int16_t y;
};

struct RGBA {
    uint8_t R{0};
    uint8_t G{0};
    uint8_t B{0};
    uint8_t A{0};
};

struct Frame {
    int32_t x{0};
    int32_t y{0};
    uint32_t width;
    uint32_t height;
    std::vector<RGBA> pixels;
};


class Was {
public:
    Was(const std::string &path, const WasInfo &info);

    const WasHeader &header() { return m_header; }
    std::vector<std::vector<Frame> > fullFrames() const { return m_fullFrames; }
    std::vector<uint8_t> times() const { return m_times; }
    const std::vector<RGBA> &pixels() { return m_pixels; }

private:
    void readFrame(std::vector<uint8_t> &buf, const std::vector<RGBA> &palette, Frame &frame);

    void RGB565ToRGBA8888(uint16_t src, uint8_t alpha, RGBA &dst);

private:
    WasHeader m_header;
    std::vector<uint8_t> m_times;
    std::vector<std::vector<Frame> > m_fullFrames;
    std::vector<RGBA> m_pixels;
};


#endif //WAS_H
