#pragma once
#include "pch.h"
#include "primitives.h"
#include "debug.h"
#include "shader.h"
#include "graphics_headers.h"

namespace graphics {

class PrimitiveDrawer {
public:
	PrimitiveDrawer();

	static void useShader(Shader& shader);

	static void drawLine(const vec3& pos1, const Color::FColor color1, const vec3& pos2, const Color::FColor color2);

	static void drawTrig(const vec3& v1, const Color::FColor color1, const vec3& v2, const Color::FColor color2, const vec3& v3, const Color::FColor color3);

	static void fillTrig(const vec3& v1, const Color::FColor color1, const vec3& v2, const Color::FColor color2, const vec3& v3, const Color::FColor color3);

	static void fillTrig(const vec2& v1, const vec2& uv1, const vec2& v2, const vec2& uv2, const vec2& v3, const vec2& uv3, const Texture& texture);

	static void beginFrame() { }

	static void endFrame() {
		instance().pushAll();
	}

	static void pushAll() {
		instance().pushLines();
		instance().pushTrigs();
	}

private:
	GLuint		m_vao;

	GLuint		m_linesVbo;
	constexpr static unsigned s_maxLinesCount = 0x400;
	unsigned	m_linesCount = 0;

	GLuint		m_trigsVbo;
	constexpr static unsigned s_maxTrigsCount = 0x400;
	unsigned	m_trigsCount = 0;

	GLuint		m_texturedTrigsVbo;

	enum class DrawType { NONE = 0x00, LINE, TRIG, TEX_TRIG };

	GLuint		m_currentShaderID = 0;
	DrawType	m_previouslyDrawnPrimitive = DrawType::NONE;

	static PrimitiveDrawer& instance() {
		static PrimitiveDrawer c_instance;
		return c_instance;
	}

	void pushLines();

	void pushTrigs();

	void setDrawType(DrawType type);
};

struct ShaderSourceWrapperImpl {
	std::string value;

	ShaderSourceWrapperImpl(const std::string& _value)
		: value(_value)
	{ }

	operator std::string() {
		return value;
	}
};

}
