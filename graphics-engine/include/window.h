#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <memory>
#include <string>
#endif // GRAPHICS_PCH

#include "primitives.h"
#include "viewport.h"

namespace graphics {

class Window {
public:
	Window(const std::string& title, unsigned width, unsigned height);

	void open();

	void initImGui();
	
	void close();

	bool isOpen();

	void render();

	vec2 getSize() const;

	bool resized() const;

	//vec2 getMousePosition() const;

	//vec2 getMouseMotion() const;

	//vec2 getMouseMotionNds() const;

	//bool mouseMoved() const;

	//bool mouseClicked(MouseButton button = MouseButton::LEFT) const;

	//bool isMouseButtonDown(MouseButton button = MouseButton::LEFT) const;

	//float getMouseWheel() const;

	//void setBackfaceCulling(bool isEnabled) const;

	Viewport& viewport();

	void setBorder(bool isEnabled) const;

	enum class TittlebarTheme { LIGHT = 0, DARK = 1 };

	void setTittlebarTheme(TittlebarTheme theme);

	~Window();

private:
	std::string m_title;


	struct Impl; std::unique_ptr<Impl> impl;
};

}
