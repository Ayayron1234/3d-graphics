#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <fstream>
#include <string>
#include <memory>
#endif // GRAPHICS_PCH

namespace graphics {

class Bitmap {
public:
    Bitmap(std::string path);

    unsigned width() const;
    
    unsigned height() const;

    bool loaded() const {
        return m_loadStatus == LoadStatus::OK;
    }

    enum class LoadStatus { 
        OK = 0x00, 
        FILE_NOT_FOUND, 
        NOT_BMP, 
        UNLOADED 
    };

    LoadStatus getStatus() const {
        return m_loadStatus;
    }

    static const char* getErrorString(LoadStatus status);

    enum Layout : uint8_t { 
        ORIGINAL = 0x00, 
        RGBA = 0b11100100, 
        ARGB = 0b00111001, 
        BGRA = 0b01101100 
    };

    uint32_t getPixel(uint32_t x, uint32_t y, Layout layout = RGBA) const;
    
    template <typename T>
        requires(sizeof(T) == sizeof(uint32_t))
    std::shared_ptr<T[]> getPixelBuffer(Layout layout = RGBA) const;

    Bitmap(const Bitmap&) = default;
    ~Bitmap();

private:
    struct BitmapFileHeader;
    struct DIBHeader;

    std::unique_ptr<BitmapFileHeader>   m_fileHeader;
    std::unique_ptr<DIBHeader>          m_dibHeader;

    std::shared_ptr<unsigned char[]>    m_pixelArray;

    LoadStatus  m_loadStatus = LoadStatus::UNLOADED;

    unsigned    m_channelOffsets[4]{};

    constexpr static size_t s_maxSize = 10000;

    void setChanelOffsets();

    int calcOffsetFromMask(uint32_t mask) const;

    uint32_t convertPixel(uint32_t origin, uint32_t mask1, uint32_t mask2, uint32_t mask3, uint32_t mask4) const;
};

template<typename T>
    requires(sizeof(T) == sizeof(uint32_t))
inline std::shared_ptr<T[]> Bitmap::getPixelBuffer(Layout layout) const {
    if (width() > s_maxSize || height() > s_maxSize)
        throw std::runtime_error("Incorrect width or height");

    std::shared_ptr<T[]> buffer = std::make_shared<T[]>(width() * height());
    for (int i = 0; i < width() * height(); ++i)
        *(uint32_t*)&buffer[i] = getPixel(i % width(), i / width(), layout);

    return buffer;
}

}
