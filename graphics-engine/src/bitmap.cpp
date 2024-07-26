#include "pch.h"
#include "bitmap.h"

using namespace graphics;

constexpr static char signature[2] = { 'B', 'M' };

struct ChannelOffsetIndex {
    uint8_t c1 : 2;
    uint8_t c2 : 2;
    uint8_t c3 : 2;
    uint8_t c4 : 2;

    ChannelOffsetIndex(Bitmap::Layout layout) {
        *(uint8_t*)this = layout;
    }
};

struct Bitmap::BitmapFileHeader {
    short   signature;
    short   _fileSize[2];
    short : 16; short : 16;             // reserved
    short   _offsetToPixelArray[2];

    int fileSize() const {
        return *(unsigned int*)&_fileSize;
    }

    int offsetToPixelArray() const {
        return *(unsigned int*)&_offsetToPixelArray;
    }

    BitmapFileHeader& operator<<(std::ifstream& ifs) {
        ifs.read((char*)this, sizeof(*this));
        return *this;
    }
};

struct Bitmap::DIBHeader {
    uint32_t dibHeaderSize;
    uint32_t imageWidth, imageHeight;
    uint16_t : 16;

    uint16_t bitsPerPixel;
    uint32_t : 32;

    uint32_t imageSize;
    uint32_t xPixelsPerMeter;
    uint32_t yPixelsPerMeter;
    uint32_t : 32;
    uint32_t : 32;
    
    uint32_t redChannelBitmask;
    uint32_t greenChannelBitmask;
    uint32_t blueChannelBitmask;
    uint32_t alphaChannelBitmask;

    uint32_t getBytesPerPixel() const {
        return bitsPerPixel / 8;
    }

    uint32_t getPadding() const {
        return (4 - ((bitsPerPixel / 8 * imageWidth) % 4)) % 4;
    }

    uint32_t getRowSize() const {
        return imageWidth * getBytesPerPixel() + getPadding();
    }

    uint32_t getOffset(uint32_t x, uint32_t y) const {
        return (imageHeight - y - 1) * getRowSize() + x * getBytesPerPixel();
    }

    DIBHeader& operator<<(std::ifstream& ifs) {
        ifs.read((char*)this, sizeof(*this));
        return *this;
    }
};

Bitmap::Bitmap(std::string path)
    : m_fileHeader(std::make_unique<BitmapFileHeader>())
    , m_dibHeader(std::make_unique<DIBHeader>())
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) {
        m_loadStatus = LoadStatus::FILE_NOT_FOUND;
        return;
    }

    *m_fileHeader << ifs;
    *m_dibHeader << ifs;

    if (*(short*)signature != m_fileHeader->signature) {
        m_loadStatus = LoadStatus::NOT_BMP;
        return;
    }

    m_pixelArray = std::make_shared<unsigned char[]>(m_dibHeader->imageSize);
    ifs.seekg(ifs.beg);
    ifs.seekg(m_fileHeader->offsetToPixelArray());
    ifs.read((char*)m_pixelArray.get(), m_dibHeader->imageSize);

    setChanelOffsets();

    m_loadStatus = LoadStatus::OK;
}

unsigned Bitmap::width() const {
    return m_dibHeader->imageWidth;
}

unsigned Bitmap::height() const {
    return m_dibHeader->imageHeight;
}

const char* graphics::Bitmap::getErrorString(LoadStatus status) {
    switch (status)
    {
    case graphics::Bitmap::LoadStatus::FILE_NOT_FOUND:
        return "File not found.";
    case graphics::Bitmap::LoadStatus::NOT_BMP:
        return "File is not a bitmap (signature incorrect).";
    case graphics::Bitmap::LoadStatus::UNLOADED:
        return "Bitmap didn't load yet.";
    default:
        return "";
    }
}

uint32_t Bitmap::getPixel(uint32_t x, uint32_t y, Layout layout) const {
    if (x >= width() || y >= height())
        return 0;

    uint32_t pixel;
    memcpy(&pixel, &m_pixelArray.get()[m_dibHeader->getOffset(x, y)], m_dibHeader->getBytesPerPixel());
    
    if (layout == ORIGINAL)
        return pixel;

    ChannelOffsetIndex offsetIndex(layout);
    return convertPixel(pixel, m_channelOffsets[offsetIndex.c1], m_channelOffsets[offsetIndex.c2],
                        m_channelOffsets[offsetIndex.c3], m_channelOffsets[offsetIndex.c4]);
}

Bitmap::~Bitmap() { }

void Bitmap::setChanelOffsets() {
    m_channelOffsets[0] = calcOffsetFromMask(m_dibHeader->redChannelBitmask);
    m_channelOffsets[1] = calcOffsetFromMask(m_dibHeader->greenChannelBitmask);
    m_channelOffsets[2] = calcOffsetFromMask(m_dibHeader->blueChannelBitmask);
    m_channelOffsets[3] = calcOffsetFromMask(m_dibHeader->alphaChannelBitmask);
}

int graphics::Bitmap::calcOffsetFromMask(uint32_t mask) const {
    mask = mask;
    unsigned offset = 0;
    while (mask != 0) { offset += 1; mask = mask >> 8; }
    return offset - 1;
}

uint32_t graphics::Bitmap::convertPixel(uint32_t origin, uint32_t mask1, uint32_t mask2, uint32_t mask3, uint32_t mask4) const {
    uint32_t res = 0;
    ((uint8_t*)&res)[0] = ((uint8_t*)&origin)[mask1];
    ((uint8_t*)&res)[1] = ((uint8_t*)&origin)[mask2];
    ((uint8_t*)&res)[2] = ((uint8_t*)&origin)[mask3];
    ((uint8_t*)&res)[3] = ((uint8_t*)&origin)[mask4];
    return res;
}
