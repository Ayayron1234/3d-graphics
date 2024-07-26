#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <memory>
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

	static std::shared_ptr<Texture> loadFromFile(const std::string& path, MagFilter minFilter = LINEAR, MagFilter magFilter = LINEAR);

private:
	class ID;
	
	std::shared_ptr<ID> m_id;

	unsigned			m_width;
	unsigned			m_height;
	bool				m_empty = true;
};

}
