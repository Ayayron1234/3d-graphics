#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <memory>
#include <string>
#endif // GRAPHICS_PCH

namespace graphics {

class Texture {
public:
	enum MagFilter { NEAREST = 0x2600, LINEAR = 0x2601 };

	Texture();

	unsigned int id() const;

	unsigned width() const {
		return m_width;
	}

	unsigned height() const {
		return m_height;
	}

	bool empty() const {
		return m_empty;
	}

	// allocates memory on the gpu
	void resize(unsigned width, unsigned height);

	static Texture loadFromFile(const std::string& path, MagFilter minFilter = LINEAR, MagFilter magFilter = LINEAR);

	static Texture createTexture(unsigned width, unsigned height, MagFilter minFilter = LINEAR, MagFilter magFilter = LINEAR);

	static Texture null();

private:
	class ID;
	
	std::shared_ptr<ID> m_id;

	Texture(unsigned id);

	unsigned			m_width = 0;
	unsigned			m_height = 0;
	bool				m_empty = true;
};

}
