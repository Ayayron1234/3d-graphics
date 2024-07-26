#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <memory>
#include <string>
#endif // GRAPHICS_PCH

#include "primitives.h"

namespace graphics {

enum class MouseButton : unsigned char { LEFT = 1, MIDDLE, RIGHT };

class Window {
public:
	Window(const std::string& title, unsigned width, unsigned height);

	void open();
	
	void close();

	bool isOpen();

	void render();

	vec2 getSize() const;

	bool resized() const;

	vec2 getMousePosition() const;

	vec2 getMouseMotion() const;

	vec2 getMouseMotionNds() const;

	bool mouseMoved() const;

	bool mouseClicked(MouseButton button = MouseButton::LEFT) const;

	bool isMouseButtonDown(MouseButton button = MouseButton::LEFT) const;

	float getMouseWheel() const;

	~Window();

private:
	std::string m_title;

	struct Impl; std::unique_ptr<Impl> impl;
};

}
