#include "pch.h"
#include "framebuffer.h"

#include "graphics_headers.h"
#include "primitive_drawer.h"

using namespace graphics;

class FrameBuffer::FBID {
public:
	FBID() {
		glGenFramebuffers(1, &m_id);
	}

	FBID(unsigned id)
		: m_id(id)
	{ }

	~FBID() {
		glDeleteFramebuffers(1, &m_id);
	}

	operator unsigned int() const {
		return m_id;
	}

private:
	unsigned int m_id;
};

class FrameBuffer::RBID {
public:
	RBID() {
		glGenRenderbuffers(1, &m_id);
	}

	RBID(unsigned id)
		: m_id(id)
	{ }

	~RBID() {
		glDeleteRenderbuffers(1, &m_id);
	}

	operator unsigned int() const {
		return m_id;
	}

private:
	unsigned int m_id;
};

FrameBuffer::FrameBuffer() 
	: m_frameBufferID(std::make_shared<FBID>())
	, m_renderBufferID(std::make_shared<RBID>())
{ }

graphics::FrameBuffer::FrameBuffer(const Texture & texture)
	: m_frameBufferID(std::make_shared<FBID>())
	, m_renderBufferID(std::make_shared<RBID>())
{
	setOutput(texture);
}

void graphics::FrameBuffer::bind() const {
	if (m_texture.empty())
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, *m_frameBufferID);
	glBindTexture(GL_TEXTURE_2D, m_texture.id());

	glClearColor(.2, .2, .2, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void graphics::FrameBuffer::unbind() const {
	PrimitiveDrawer::endFrame();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBuffer::setOutput(const Texture& texture) {
	m_texture = texture;

	// Attach texture to frame buffer
	glBindTexture(GL_TEXTURE_2D, texture.id());
	glBindFramebuffer(GL_FRAMEBUFFER, *m_frameBufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id(), 0);

	// Setup render buffer
	glBindRenderbuffer(GL_RENDERBUFFER, *m_renderBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, texture.width(), texture.height());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *m_renderBufferID);

	// Check for errors
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	// Unbind buffers
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FrameBuffer::resize(unsigned width, unsigned height) {
	m_texture.resize(width, height);

	// Bind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, *m_frameBufferID);

	// Attach texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture.id(), 0);

	// Setup render buffer
	glBindRenderbuffer(GL_RENDERBUFFER, *m_renderBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *m_renderBufferID);

	// Unbind buffers
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

Texture& graphics::FrameBuffer::texture() {
	return m_texture;
}

const Texture& graphics::FrameBuffer::texture() const {
	return m_texture;
}

FrameBuffer graphics::FrameBuffer::backbuffer() {
	return FrameBuffer(0, 0);
}

FrameBuffer::FrameBuffer(unsigned fbId, unsigned rbId)
	: m_frameBufferID(std::make_shared<FBID>(fbId))
	, m_renderBufferID(std::make_shared<RBID>(rbId))
{ }
