#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <memory>
#endif // GRAPHICS_PCH

#include "framebuffer.h"

namespace graphics {

class RenderTarget {
public:
	RenderTarget(unsigned width, unsigned height);

	Texture& texture();

	void resize(unsigned width, unsigned height);

private:
	Texture		m_texture;
	FrameBuffer	m_colorBuffer;

};

}
