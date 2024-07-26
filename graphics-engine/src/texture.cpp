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

	~ID() {
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

std::shared_ptr<Texture> Texture::loadFromFile(const std::string& path, MagFilter minFilter, MagFilter magFilter) {
    std::shared_ptr<Texture> texture = std::make_shared<Texture>();

    // Load from file
    Bitmap bitmap(path);
    if (!bitmap.loaded()) {
        debug::cout << "Failed to load texture from " << path << ": " 
            << Bitmap::getErrorString(bitmap.getStatus()) << std::endl;
        return texture;
    }

    std::shared_ptr<Color[]> buffer = bitmap.getPixelBuffer<Color>(Bitmap::RGBA);
    texture->m_width = bitmap.width();
    texture->m_height = bitmap.height();

    // Create a OpenGL texture identifier
    glBindTexture(GL_TEXTURE_2D, texture->id());

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.width(), bitmap.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)buffer.get());

    texture->m_empty = false;
    return texture;
}
