#include "pch.h"
#include "texture.h"
#include "graphics_headers.h"
#include "bitmap.h"
#include "debug.h"

using namespace graphics;

class Texture::ID {
public:
	ID() {
		glGenTextures(1, &m_id);
	}

    ID(unsigned id)
        : m_id(id)
    { }

	~ID() {
        if (m_id != 0)
		    glDeleteTextures(1, &m_id);
	}

	operator unsigned int() const {
		return m_id;
	}

private:
	unsigned int m_id;
};

Texture::Texture() 
	: m_id(std::make_shared<ID>())
{ }

unsigned int Texture::id() const {
	return *m_id;
}

void Texture::resize(unsigned width, unsigned height) {
    glBindTexture(GL_TEXTURE_2D, *m_id);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    m_width = width;
    m_height = height;

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture Texture::loadFromFile(const std::string& path, MagFilter minFilter, MagFilter magFilter) {
    Texture texture;

    // Load from file
    Bitmap bitmap(path);
    if (!bitmap.loaded()) {
        debug::cout << "Failed to load texture from " << path << ": " 
            << Bitmap::getErrorString(bitmap.getStatus()) << std::endl;
        return texture;
    }

    std::shared_ptr<Color[]> buffer = bitmap.getPixelBuffer<Color>(Bitmap::RGBA);
    texture.m_width = bitmap.width();
    texture.m_height = bitmap.height();

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, texture.id());

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.width(), bitmap.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)buffer.get());

    texture.m_empty = false;
    return texture;
}

Texture Texture::createTexture(unsigned width, unsigned height, MagFilter minFilter, MagFilter magFilter) {
    Texture texture;

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, texture.id());

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Allocate memory on gpu
    texture.resize(width, height);
    texture.m_empty = false;

    return texture;
}

Texture graphics::Texture::null() {
    return Texture(0);
}

Texture::Texture(unsigned id)
    : m_id(std::make_shared<ID>(0))
{ }
