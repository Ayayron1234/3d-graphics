#include "pch.h"
#include "render_target.h"

using namespace graphics;

RenderTarget::RenderTarget(unsigned width, unsigned height) 
	: m_texture(Texture::createTexture(width, height))
	, m_colorBuffer(FrameBuffer())
{

}

Texture& RenderTarget::texture() {
	return m_texture;
}

void graphics::RenderTarget::resize(unsigned width, unsigned height) {
	m_texture.resize(width, height);
}
