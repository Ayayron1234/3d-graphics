#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#endif // GRAPHICS_PCH

#include "primitives.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"

namespace graphics::debug {

extern std::shared_ptr<Camera> camera;

extern std::ostream cout;

void drawLine(const vec2& v1, const vec2& v2, const Color& color = Color::white(), Shader& shader = colorShader);

void drawLine(const vec3& v1, const vec3& v2, const Color& color = Color::white(), Shader& shader = colorShader);

void drawLine(const vec2& v1, const Color& color1,
	const vec2& v2, const Color& color2, Shader& shader = colorShader);

void drawTrig(const vec2& v1, const vec2& v2, const vec2& v3,
	const Color& color = Color::white(), Shader& shader = colorShader);

void drawTrig(const vec3& v1, const vec3& v2, const vec3& v3,
	const Color& color = Color::white(), Shader& shader = colorShader);

void fillTrig(const vec2& v1, const vec2& v2, const vec2& v3,
	const Color& color = Color::white(), Shader& shader = colorShader);

void fillTrig(
	const vec3& v1, const vec3& v2, const vec3& v3,
	const Color& color, Shader& shader = colorShader);

void drawTrig(const vec2& v1, const Color& color1,
	const vec2& v2, const Color& color2,
	const vec2& v3, const Color& color3, Shader& shader = colorShader);

void fillTrig(const vec2& v1, const Color& color1,
	const vec2& v2, const Color& color2,
	const vec2& v3, const Color& color3, Shader& shader = colorShader);

void fillTrig(const vec2& v1, const vec2& uv1,
	const vec2& v2, const vec2& uv2,
	const vec2& v3, const vec2& uv3,
	const Texture& texture, Shader& shader = textureShader);

}
