#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <memory>
#endif // GRAPHICS_PCH

#include "texture.h"

namespace graphics {

class FrameBuffer {
public:
	FrameBuffer();

	FrameBuffer(const Texture& texture);

	void bind() const;

	void unbind() const;

	void setOutput(const Texture& texture);

	void resize(unsigned width, unsigned height);

	Texture& texture();

	const Texture& texture() const;

	static FrameBuffer backbuffer();

private:
	class FBID;
	class RBID;

	std::shared_ptr<FBID>	m_frameBufferID;
	std::shared_ptr<RBID>	m_renderBufferID;
	Texture					m_texture = Texture::null();

	FrameBuffer(unsigned fbId, unsigned rbId);
};

}
