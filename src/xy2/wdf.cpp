#include "wdf.h"
#include <fstream>
#include <iostream>
#include <vector>

// PFDW
#define WDFP 0x57444650
// SP
#define PS 0x5053
// KP
#define PK 0x504B
// MP3
#define MP3 0xF3FF
// RIFF WAVE
#define IR 0x4952
// RIFF FSB4 0x34425346
#define FSB4 0x5346
// JPEG
#define JPEG 0xD8FF
// TGA
#define TGA 0x0000
// PNG
#define PNG 0x5089
// RAR 0x726152
#define RAR 0x6152

Wdf::Wdf() {
}

Wdf::Wdf(const std::string &path) {
    load(path);
}

Wdf::~Wdf() {
}

bool Wdf::load(const std::string &path) {
    m_path = path;
    m_isValid = false;
    m_wasInfos.clear();

    std::fstream file(path, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Failed to open file " << path << std::endl;
        return m_isValid;
    }

    std::cout << "WDF init: " << path << std::endl;

    file.read((char *) (&m_header), sizeof(m_header));

    if (m_header.flag != WDFP) {
        std::cerr << "File is not a WDF file" << std::endl;
        file.close();
        return m_isValid;
    }
    file.seekg(m_header.offset, std::ios::beg);
    WasInfo was;
    for (int i = 0; i < m_header.wasCount; i++) {
        file.read((char *) (&was), offsetof(WasInfo, type));
        m_wasInfos[was.hash] = was;
    }
    for (auto &d: m_wasInfos) {
        file.seekg(d.second.offset, std::ios::beg);
        file.read((char *) (&d.second.flag), sizeof(d.second.flag));
        switch (d.second.flag) {
            case PS:
                d.second.type = WT_PS;
                break;
            case PK:
                d.second.type = WT_PK;
                break;
            case MP3:
                d.second.type = WT_MP3;
            break;
            case IR:
                d.second.type = WT_WAVE;
                break;
            case FSB4:
                d.second.type = WT_FSB4;
                break;
            case JPEG:
                d.second.type = WT_JPEG;
                break;
            case TGA:
                d.second.type = WT_TGA;
                break;
            case PNG:
                d.second.type = WT_PNG;
            break;
            case RAR:
                d.second.type = WT_RAR;
            break;
            default:
                d.second.type = WT_UNKNOWN;
        }
    }
    file.close();
    m_isValid = true;
    std::cout << "WDF init success!" << std::endl;
    return m_isValid;
}
