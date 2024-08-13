#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#endif // GRAPHICS_PCH

#include "primitives.h"

namespace graphics {


class Mouse {
public:
	enum Button : unsigned char { LEFT = 1, MIDDLE, RIGHT };
	
	static vec2i globalPosition();

	static vec2i windowPosition();

	static bool isButtonDown(Button button = LEFT);

	static bool clicked(Button button = LEFT);

	static bool released(Button button = LEFT);

	static vec2i motion();

	static bool didMove();

	static float wheel();

	static void setPosition(const vec2i& position);
};

}
