#include "pch.h"
#include "primitive_drawer.h"
#include "window.h"
#include "camera.h"

using namespace graphics;

struct Vertex {
	vec3			position;
	Color::FColor   color;
};

struct UVVertex {
	vec3 position;
	vec2 uv;
};

struct Line {
	Vertex v1;
	Vertex v2;
};

struct Trig {
	Vertex v1;
	Vertex v2;
	Vertex v3;
};

struct UVTrig {
	UVVertex v1;
	UVVertex v2;
	UVVertex v3;
};

void debug::drawLine(const vec2& v1, const vec2& v2, const Color& color, Shader& shader) {
	PrimitiveDrawer::useShader(shader);
	PrimitiveDrawer::drawLine(v1, color, v2, color);
}

void graphics::debug::drawLine(const vec3& v1, const vec3& v2, const Color& color, Shader& shader) {
	PrimitiveDrawer::useShader(shader);
	PrimitiveDrawer::drawLine(v1, color, v2, color);

}

void debug::drawLine(
	const vec2& v1, const Color& color1, 
	const vec2& v2, const Color& color2, Shader& shader) 
{
	PrimitiveDrawer::useShader(shader);
	PrimitiveDrawer::drawLine(v1, color1, v2, color2);
}

void debug::drawTrig(
	const vec2& v1, const vec2& v2, const vec2& v3, 
	const Color& color, Shader& shader) 
{
	PrimitiveDrawer::useShader(shader);
	PrimitiveDrawer::drawTrig(v1, color, v2, color, v3, color);
}

void graphics::debug::drawTrig(
	const vec3& v1, const vec3& v2, const vec3& v3, 
	const Color& color, Shader& shader) 
{
	PrimitiveDrawer::useShader(shader);
	PrimitiveDrawer::drawTrig(v1, color, v2, color, v3, color);
}

void debug::fillTrig(
	const vec2& v1, const vec2& v2, const vec2& v3, 
	const Color& color, Shader& shader) 
{
	PrimitiveDrawer::useShader(shader);
	PrimitiveDrawer::fillTrig(v1, color, v2, color, v3, color);
}

void debug::drawTrig(
	const vec2& v1, const Color& color1, 
	const vec2& v2, const Color& color2, 
	const vec2& v3, const Color& color3, Shader& shader) 
{
	PrimitiveDrawer::useShader(shader);
	PrimitiveDrawer::drawTrig(v1, color1, v2, color2, v3, color3);
}

void debug::fillTrig(
	const vec2& v1, const Color& color1,
	const vec2& v2, const Color& color2, 
	const vec2& v3, const Color& color3, Shader& shader) 
{
	PrimitiveDrawer::useShader(shader);
	PrimitiveDrawer::fillTrig(v1, color1, v2, color2, v3, color3);
}

void debug::fillTrig(
	const vec2& v1, const vec2& uv1, 
	const vec2& v2, const vec2& uv2, 
	const vec2& v3, const vec2& uv3, 
	const Texture& texture, Shader& shader) 
{
	PrimitiveDrawer::useShader(shader);
	PrimitiveDrawer::fillTrig(v1, uv1, v2, uv2, v3, uv3, texture);
}

PrimitiveDrawer::PrimitiveDrawer() {
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_linesVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_linesVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Line) * s_maxLinesCount, nullptr, GL_STATIC_DRAW);

	glGenBuffers(1, &m_trigsVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_trigsVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Trig) * s_maxTrigsCount, nullptr, GL_STATIC_DRAW);

	glGenBuffers(1, &m_texturedTrigsVbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_texturedTrigsVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Trig) * s_maxTrigsCount, nullptr, GL_STATIC_DRAW);
}

void PrimitiveDrawer::useShader(Shader& shader) {
	// return if shader is loaded, was used by the last primitive draw and is currently selected globaly
	if (shader.id() && shader.id() == instance().m_currentShaderID && Shader::currentID() == instance().m_currentShaderID)
		return;

	shader.setUniform(*debug::camera);
	shader.use();

	instance().m_currentShaderID = shader.id();
}

void PrimitiveDrawer::drawLine(const vec3& pos1, const Color::FColor color1, const vec3& pos2, const Color::FColor color2) {
	instance().setDrawType(DrawType::LINE);

	if (instance().m_linesCount + 1 > s_maxLinesCount)
		instance().pushLines();

	Line data{ pos1, color1, pos2, color2 };

	glBindVertexArray(instance().m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, instance().m_linesVbo);

	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Line) * instance().m_linesCount, sizeof(Line), &data);
	++instance().m_linesCount;
}

void PrimitiveDrawer::drawTrig(const vec3& v1, const Color::FColor color1, const vec3& v2, const Color::FColor color2, const vec3& v3, const Color::FColor color3) {
	instance().setDrawType(DrawType::LINE);

	if (instance().m_linesCount + 3 > s_maxLinesCount)
		instance().pushLines();

	Line lines[3] = { Line{ v1, color1, v2, color2 },
					  Line{ v2, color2, v3, color3 },
					  Line{ v3, color3, v1, color1 } };

	glBindVertexArray(instance().m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, instance().m_linesVbo);

	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Line) * instance().m_linesCount, sizeof(Line) * 3, lines);
	instance().m_linesCount += 3;
}

void PrimitiveDrawer::fillTrig(const vec3& v1, const Color::FColor color1, const vec3& v2, const Color::FColor color2, const vec3& v3, const Color::FColor color3) {
	instance().setDrawType(DrawType::TRIG);

	if (instance().m_trigsCount + 1 > s_maxTrigsCount)
		instance().pushTrigs();

	Trig trig = { v1, color1, v2, color2, v3, color3 };

	glBindVertexArray(instance().m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, instance().m_trigsVbo);
	
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Trig) * instance().m_trigsCount, sizeof(Trig), &trig);
	++instance().m_trigsCount;
}

void graphics::PrimitiveDrawer::fillTrig(const vec2& v1, const vec2& uv1, const vec2& v2, const vec2& uv2, const vec2& v3, const vec2& uv3, const Texture& texture) {
	instance().setDrawType(DrawType::TEX_TRIG);

	UVTrig trig = { v1, uv1, v2, uv2, v3, uv3 };

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.id());
	glUniform1i(glGetUniformLocation(instance().m_currentShaderID, "textureSampler"), 0);

	glBindVertexArray(instance().m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, instance().m_texturedTrigsVbo);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(UVTrig), &trig);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(UVVertex), (void*)0);
	glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UVVertex), (void*)sizeof(vec3));
	glEnableVertexAttribArray(1);

	// Check for errors
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
	}

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void PrimitiveDrawer::pushLines() {
	if (m_linesCount == 0)
		return;

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_linesVbo);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));
	glEnableVertexAttribArray(1);

	// Check for errors
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
	}

	glDrawArrays(GL_LINES, 0, m_linesCount * 2);
	m_linesCount = 0;
}

void graphics::PrimitiveDrawer::pushTrigs() {
	if (m_trigsCount == 0)
		return;

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_trigsVbo);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));
	glEnableVertexAttribArray(1);

	// Check for errors
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
	}

	glDrawArrays(GL_TRIANGLES, 0, m_trigsCount * 3);
	m_trigsCount = 0;
}

void PrimitiveDrawer::setDrawType(DrawType type) {
	if (m_previouslyDrawnPrimitive != type)
		if (type == DrawType::LINE)
			pushTrigs();
		if (type == DrawType::TRIG)
			pushLines();
		if (type == DrawType::TEX_TRIG)
			pushAll();

	m_previouslyDrawnPrimitive = type;
}
