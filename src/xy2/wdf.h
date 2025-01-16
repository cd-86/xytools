#ifndef WDF_H
#define WDF_H
#include <map>
#include <string>

enum WasType {
    WT_UNKNOWN,
    WT_PS,
    WT_PK,
    WT_MP3,
    WT_WAVE,
    WT_FSB4,
    WT_JPEG,
    WT_TGA,
    WT_PNG,
    WT_RAR,
};

struct WdfHeader {
    uint32_t flag;
    uint32_t wasCount;
    uint32_t offset;
};

struct WasInfo {
    uint32_t hash;
    uint32_t offset;
    uint32_t size;
    uint32_t spaces;

    WasType type;
    uint16_t flag;
};



class Wdf {
public:
    Wdf();
    Wdf(const std::string &path);
    ~Wdf();
    bool load(const std::string &path);
    std::string getPath() const {return m_path;}
    WdfHeader getHeader() const {return m_header;}
    std::map<uint32_t, WasInfo> getWasInfos() const {return m_wasInfos;}
    bool isValid() const {return m_isValid;}

private:
    std::string m_path;
    WdfHeader m_header;
    std::map<uint32_t, WasInfo> m_wasInfos;
    bool m_isValid {false};
};


#endif //WDF_H
