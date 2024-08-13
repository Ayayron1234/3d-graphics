#include "pch.h"
#include "viewport.h"

#include "mouse.h"
#include "imgui.h"
#include "primitive_drawer.h"

#include "graphics_headers.h"

using namespace graphics;

graphics::Viewport::Viewport(const FrameBuffer& frameBuffer)
	: m_frameBuffer(frameBuffer)
	, m_width(frameBuffer.texture().width())
	, m_height(frameBuffer.texture().height())
{ }

void graphics::Viewport::use() {
	if (s_activeViewport == this)
		return;

	PrimitiveDrawer::pushAll();
	if (s_activeViewport) 
		s_activeViewport->m_frameBuffer.unbind();

	m_frameBuffer.bind();
	glViewport(0, 0, m_width, m_height);

	// Set backface culling
	if (m_properties.backFaceCullingEnabled)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

}

vec2 graphics::Viewport::position() {
	return m_position;
}

vec2 graphics::Viewport::size() const {
	return vec2(m_width, m_height);
}

void graphics::Viewport::renderAsImGuiImage() {
	ImVec2 size = ImGui::GetContentRegionAvail();
	m_width = size.x;
	m_height = size.y;

	ImGui::Image(
		(ImTextureID)m_frameBuffer.texture().id(),
		size,
		ImVec2(0, 1),
		ImVec2(1, 0)
	);
}

void graphics::Viewport::renderAsImGuiWindow(const std::string& windowName, bool hideTabBar) {
	static ImGuiWindowClass noTabBar{};
	noTabBar.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;

	if (hideTabBar)
		ImGui::SetNextWindowClass(&noTabBar);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	
	ImGui::Begin(windowName.c_str());
	{
		// Resize viewport
		m_frameBuffer.resize(m_width, m_height);
		m_position = vec2i(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);

		renderAsImGuiImage();
	}
	ImGui::End();
	
	ImGui::PopStyleVar();
}

const graphics::Viewport::Properties& graphics::Viewport::properties() const {
	return m_properties;
}

graphics::Viewport::Properties& graphics::Viewport::properties() {
	return m_properties;
}

vec2 graphics::Viewport::windowToNdc(const vec2i& point) const {
	return vec2(
		((float)(point.x) / (float)m_width) * 2.f - 1.f, 
		1.f - ((float)(point.y) / (float)m_height) * 2.f
	);
}

vec2i graphics::Viewport::globalToWindow(const vec2i& point) const {
	return point - m_position;
}

vec2 graphics::Viewport::globalToNdc(const vec2i& point) const {
	return vec2(
		((float)(point.x - m_position.x) / (float)m_width) * 2.f - 1.f,
		1.f - ((float)(point.y - m_position.y) / (float)m_height) * 2.f
	);
}

vec2 graphics::Viewport::scaleVec(const vec2i& vec) const {
	return vec2(
		((float)(vec.x) / (float)m_width) * 2.f,
		- ((float)(vec.y) / (float)m_height) * 2.f
	);
}

bool graphics::Viewport::isPointInside(const vec2i& point) const {
	return (point.x >= m_position.x && point.y >= m_position.y
		&& point.x - m_position.x < m_width && point.y - m_position.y < m_height);
}

void graphics::Viewport::resize(unsigned width, unsigned height) {
	m_width = width;
	m_height = height;

	if (m_frameBuffer.texture().id() != 0)
		m_frameBuffer.resize(width, height);
}

void graphics::Viewport::useBackbuffer() {
	PrimitiveDrawer::pushAll();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::pair<graphics::vec2, graphics::vec2> graphics::Viewport::getDrag(graphics::Mouse::Button button) const {
	updateDrag();
	vec2i cursor = Mouse::windowPosition();

	if (std::get<0>(m_mouseDrag.at(button)))
		return { std::get<1>(m_mouseDrag.at(button)), windowToNdc(cursor) - std::get<1>(m_mouseDrag.at(button)) };
}

vec2 graphics::Viewport::getDragDelta(Mouse::Button button) const {
	updateDrag();
	vec2i delta = Mouse::motion();

	if (std::get<0>(m_mouseDrag.at(button)))
		return scaleVec(delta);

	return vec2();
}

bool graphics::Viewport::getDragMod(Mouse::Button button, KeyMod mod) const {
	return ((uint8_t)std::get<2>(m_mouseDrag.at(button)) & (uint8_t)mod);
}

void graphics::Viewport::updateDrag() const {
	vec2i cursor = Mouse::windowPosition();
	bool inside = isPointInside(cursor);
	std::pair<vec2, vec2> res;

	for (int i = 0; i < 3; ++i) {
		Mouse::Button button = Mouse::Button(i);

		// Drag start
		if (Mouse::clicked(button) && inside)
			m_mouseDrag.at(i) = std::make_tuple(true, windowToNdc(cursor), KeyMod(getMods()));

		// Drag end
		if (!Mouse::isButtonDown(button))
			std::get<0>(m_mouseDrag.at(i)) = false;
	}
}
