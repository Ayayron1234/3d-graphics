#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <array>
#include <tuple>
#endif // GRAPHICS_PCH

#include "primitives.h"
#include "framebuffer.h"
#include "keyboard.h"
#include "mouse.h"

namespace graphics {

class Viewport {
public:
	struct Properties {
		bool backFaceCullingEnabled = true;
	};

	Viewport(const FrameBuffer& frameBuffer);

	void use();

	vec2 position();
	
	vec2 size() const;

	void renderAsImGuiImage();

	void renderAsImGuiWindow(const std::string& windowName, bool hideTabBar = true);

	const Properties& properties() const;
	
	Properties& properties();

	vec2 windowToNdc(const vec2i& point) const;

	vec2i globalToWindow(const vec2i& point) const;

	vec2 globalToNdc(const vec2i& point) const;

	vec2 scaleVec(const vec2i& vec) const;

	bool isPointInside(const vec2i& point) const;

	void resize(unsigned width, unsigned height);

	static void useBackbuffer();

	// { startPos, drag }
	std::pair<vec2, vec2> getDrag(Mouse::Button button = Mouse::LEFT) const;

	vec2 getDragDelta(Mouse::Button button = Mouse::LEFT) const;

	bool getDragMod(Mouse::Button button = Mouse::LEFT, KeyMod mod = KeyMod::NONE) const;

private:
	FrameBuffer			m_frameBuffer;

	unsigned			m_width;
	unsigned			m_height;
	vec2i				m_position;

	Properties			m_properties{};

	using DragValues = std::array<std::tuple<bool, vec2, KeyMod>, 3>;

	mutable DragValues	m_mouseDrag{};

	inline static Viewport* s_activeViewport = nullptr;

	void updateDrag() const;
};

}
