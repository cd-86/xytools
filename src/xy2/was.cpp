#include "was.h"

#include <fstream>
#include <iostream>
#include <vector>

Was::Was(const std::string &path, const WasInfo &info) {
    std::fstream file(path, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Failed to open file " << path << std::endl;
        return;
    }

    file.seekg(info.offset, std::ios::beg);
    file.read((char *) &m_header, sizeof(m_header));

    if (m_header.headSize > 12) {
        m_times.resize(m_header.headSize - 12);
        file.read((char *) m_times.data(), m_times.size());
    }

    std::vector<RGBA> palette(256);
    for (int i = 0; i < 256; ++i) {
        uint16_t rgb565;
        file.read((char *) &rgb565, sizeof(rgb565));
        RGB565ToRGBA8888(rgb565, 255, palette[i]);
    }

    int picNum = m_header.directionNum * m_header.frameNum;
    std::vector<uint32_t> picOffsets(picNum);
    for (int i = 0; i < picNum; ++i) {
        uint32_t offset;
        file.read((char *) &offset, sizeof(uint32_t));
        picOffsets[i] = offset + info.offset + 4 + m_header.headSize;
    }

    m_fullFrames.resize(m_header.directionNum);
    m_pixels.resize(m_header.width * m_header.frameNum * m_header.height * m_header.directionNum * 2);

    for (int i = 0; i < m_header.directionNum; ++i) {
        m_fullFrames[i].resize(m_header.frameNum);
        for (int j = 0; j < m_header.frameNum; ++j) {
            int index = i * m_header.frameNum + j;
            int frameSize = index < picNum - 1
                                ? picOffsets[index + 1] - picOffsets[index]
                                : info.size + info.offset - picOffsets[index];
            frameSize += 16;
            file.seekg(picOffsets[index], std::ios::beg);
            std::vector<uint8_t> buf(frameSize);
            file.read((char *) buf.data(), frameSize);
            auto &frame = m_fullFrames[i][j];
            readFrame(buf, palette, frame);

            // for (int r = 0; r < frame.height; ++r) {
            //     int poff = (i * m_header.height + frame.y + r) * (m_header.width * m_header.frameNum) + j * m_header.width + frame.x;
            //     int foff = r * frame.width;
            //     for (int c = 0; c < frame.width; ++c) {
            //         m_pixels[poff + c] = frame.pixels[foff + c];
            //     }
            // }
        }
    }
    file.close();
}

void Was::readFrame(std::vector<uint8_t> &buf, const std::vector<RGBA> &palette, Frame &frame) {
    memcpy(&frame, buf.data(), offsetof(Frame, pixels));
    uint32_t offset = offsetof(Frame, pixels);

    std::vector<uint32_t> frameLineOffset(frame.height);
    memcpy(frameLineOffset.data(), buf.data() + offset, sizeof(uint32_t) * frame.height);
    // offset += sizeof(uint32_t) * frame.height;

    frame.pixels.resize(frame.height * frame.width);
    // std::vector<RGBA> rgba(frame.height * frame.width);
    uint32_t pos = 0;
    for (uint32_t h = 0; h < frame.height; h++) {
        uint32_t linePixels = 0;
        bool lineNotOver = true;
        uint8_t *pData = buf.data() + frameLineOffset[h];

        while (*pData != 0 && lineNotOver) {
            uint8_t level = 0; // Alpha
            uint8_t repeat = 0; // 重复次数
            RGBA color; //重复颜色
            uint8_t style = (*pData & 0xc0) >> 6; // 取字节的前两个比特
            switch (style) {
                case 0: // {00******}
                    if (*pData & 0x20) {
                        // {001*****} 表示带有Alpha通道的单个像素
                        // {001 +5bit Alpha}+{1Byte Index}, 表示带有Alpha通道的单个像素。
                        // {001 +0~31层Alpha通道}+{1~255个调色板引索}
                        level = (*pData) & 0x1f; // 0x1f=(11111) 获得Alpha通道的值
                        if (*(pData - 1) == 0xc0) {
                            //特殊处理
                            //Level = 31;
                            //Pixels--;
                            //pos--;
                            if (linePixels <= frame.width) {
                                frame.pixels[pos] = frame.pixels[pos - 1];
                                linePixels++;
                                pos++;
                                pData += 2;
                                break;
                            } else {
                                lineNotOver = false;
                            }
                        }
                        pData++; // 下一个字节
                        if (linePixels <= frame.width) {
                            frame.pixels[pos] = palette[*pData];
                            frame.pixels[pos].A = (level << 3) | 7 - 1;
                            linePixels++;
                            pos++;
                            pData++;
                        } else {
                            lineNotOver = false;
                        }
                    } else {
                        // {000*****} 表示重复n次带有Alpha通道的像素
                        // {000 +5bit Times}+{1Byte Alpha}+{1Byte Index}, 表示重复n次带有Alpha通道的像素。
                        // {000 +重复1~31次}+{0~255层Alpha通道}+{1~255个调色板引索}
                        // 注: 这里的{00000000} 保留给像素行结束使用，所以只可以重复1~31次。
                        repeat = (*pData) & 0x1f; // 获得重复的次数
                        pData++;
                        level = *pData; // 获得Alpha通道值
                        pData++;
                        color = palette[*pData];
                        color.A = (level << 3) | 7 - 1;
                        for (int i = 1; i <= repeat; i++) {
                            if (linePixels <= frame.width) {
                                frame.pixels[pos] = color;
                                pos++;
                                linePixels++;
                            } else {
                                lineNotOver = false;
                            }
                        }
                        pData++;
                    }
                    break;
                case 1: // {01******} 表示不带Alpha通道不重复的n个像素组成的数据段
                    // {01  +6bit Times}+{nByte Datas},表示不带Alpha通道不重复的n个像素组成的数据段。
                    // {01  +1~63个长度}+{n个字节的数据},{01000000}保留。
                    repeat = (*pData) & 0x3f; // 获得数据组中的长度
                    pData++;
                    for (int i = 1; i <= repeat; i++) {
                        if (linePixels <= frame.width) {
                            frame.pixels[pos] = palette[*pData];
                            pos++;
                            linePixels++;
                            pData++;
                        } else {
                            lineNotOver = false;
                        }
                    }
                    break;
                case 2: // {10******} 表示重复n次像素
                    // {10  +6bit Times}+{1Byte Index}, 表示重复n次像素。
                    // {10  +重复1~63次}+{0~255个调色板引索},{10000000}保留。
                    repeat = (*pData) & 0x3f; // 获得重复的次数
                    pData++;
                    color = palette[*pData];
                    for (int i = 1; i <= repeat; i++) {
                        if (linePixels <= frame.width) {
                            frame.pixels[pos] = color;
                            pos++;
                            linePixels++;
                        } else {
                            lineNotOver = false;
                        }
                    }
                    pData++;
                    break;
                case 3: // {11******} 表示跳过n个像素，跳过的像素用透明色代替
                    // {11  +6bit Times}, 表示跳过n个像素，跳过的像素用透明色代替。
                    // {11  +跳过1~63个像素},{11000000}保留。
                    repeat = (*pData) & 0x3f; // 获得重复次数
                    if (repeat == 0) {
                        if (linePixels <= frame.width) {
                            //特殊处理
                            pos--;
                            linePixels--;
                        } else {
                            lineNotOver = false;
                        }
                    } else {
                        for (int i = 1; i <= repeat; i++) {
                            if (linePixels <= frame.width) {
                                pos++;
                                linePixels++;
                            } else {
                                lineNotOver = false;
                            }
                        }
                    }
                    pData++;
                    break;
                default: // 一般不存在这种情况
                    printf("WAS ERROR\n");
                    break;
            }
        }
        if (*pData == 0 || !lineNotOver) {
            uint32_t repeat = frame.width - linePixels;
            if (h > 0 && !linePixels) {
                //法术处理
                const uint8_t *last = buf.data() + frameLineOffset[h - 1];
                if (*last != 0) {
                    memcpy(frame.pixels.data() + pos, frame.pixels.data() + pos - frame.width,
                           frame.width * sizeof(RGBA));
                    pos += frame.width;
                }
            } else if (repeat > 0) {
                pos += repeat;
            }
        }
    }
    std::vector<uint32_t>().swap(frameLineOffset);

    // frame.pixels.resize(frame.height * frame.width);
    // memcpy(frame.pixels.data(), rgba.data(), frame.height* frame.width * sizeof(RGBA));
}


void Was::RGB565ToRGBA8888(uint16_t src, uint8_t alpha, RGBA &dst) {
    uint8_t r = (src >> 11) & 0x1f;
    uint8_t g = (src >> 5) & 0x3f;
    uint8_t b = (src) & 0x1f;

    dst.R = (r << 3) | (r >> 2);
    dst.G = (g << 2) | (g >> 4);
    dst.B = (b << 3) | (b >> 2);
    dst.A = alpha;
}
