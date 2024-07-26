#include "pch.h"
#include "camera.h"
#include "graphics_headers.h"
#include "sdl.h"

#include "debug.h"

using namespace graphics;

std::shared_ptr<Camera> debug::camera = std::make_shared<Camera>();

std::ostream debug::cout(nullptr);

mat4 Camera::getViewMatrix() const {
	vec3 w = normalize(m_eye - m_lookat);
	vec3 u = normalize(cross(m_vup, w));
	vec3 v = cross(w, u);
	return mat4::translation(-m_eye)
		* mat4({ u.x, v.x, w.x, 0 },
			{ u.y, v.y, w.y, 0 },
			{ u.z, v.z, w.z, 0 },
			{ 0, 0, 0, 1 });
}

mat4 Camera::getProjectionMatrix() const {
	if (m_projection == PERSPECTIVE)
		return mat4(
			{ 1 / (tanf(m_fov / 2) * m_asp), 0, 0, 0 },
			{ 0, 1 / tanf(m_fov / 2), 0, 0 },
			{ 0, 0, -(m_np + m_fp) / (m_fp - m_np), -1 },
			{ 0, 0, -2 * m_np * m_fp / (m_fp - m_np), 0 });
	
	float size = (m_lookat - m_eye).length();
	float l = -m_asp * size; // left
	float r = m_asp * size; // right
	return mat4({
		{2.0f / (r - l), 0.0f, 0.0f, 0.0f,},
		{0.0f, 2.0f / (size + size), 0.0f, 0.0f,},
		{0.0f, 0.0f, -2.0f / (m_fp - m_np), 0.0f,},
		{-(r + l) / (r - l), -(size - size) / (size + size), -(m_fp + m_np) / (m_fp - m_np), 1.0f} });
}

void Camera::setAspectRatio(float width, float height) {
	m_asp = width / height;
	m_updated = true;
}

void Camera::setAspectRatio(const Window& window) {
	vec2 size = window.getSize();
	m_asp = size.x / size.y;
	m_updated = true;
}

void graphics::Camera::setFoV(float fov) {
	m_fov = fov;
	m_updated = true;
}

void Camera::setFoVDeg(float fovInDegrees) {
	m_fov = fovInDegrees * (float)M_PI / 180.0f;
	m_updated = true;
}

void Camera::setNearAndFarPanes(float np, float fp) {
	m_np = np; m_fp = fp;
	m_updated = true;
}

void Camera::lookatFrom(const vec3& position, const vec3& target) {
	m_eye = position;
	m_lookat = target;
	m_updated = true;
}

bool graphics::Camera::updated() {
	return m_updated;
	m_updated = false;
}

vec3 graphics::Camera::getPosition() const {
	return m_eye;
}

void graphics::Camera::setPosition(const vec3& position) {
	m_eye = position;
	m_updated = true;
}

void graphics::Camera::setVUp(const vec3& direction) {
	m_vup = normalize(direction);
}

void Camera::setProjectionType(ProjectionType type) {
	m_projection = type;
}

Camera::ProjectionType Camera::getProjectionType() const {
	return m_projection;
}

Ray graphics::Camera::castRay(vec2 ndc) const {
	// Calculate camera basis vectors
	vec3 w = normalize(m_eye - m_lookat);
	vec3 u = normalize(cross(m_vup, w));
	vec3 v = cross(w, u);

	// Convert NDC to camera space
	float tanFov = tanf(m_fov / 2.0f);
	float x = ndc.x * tanFov * m_asp;
	float y = ndc.y * tanFov;

	vec3 dir = normalize(u * x + v * y - w);

	return Ray{ m_eye, dir };
}

graphics::BlenderCameraController::BlenderCameraController(std::weak_ptr<Camera> camera)
	: ICameraController(camera)
{
	if (auto cam = camera.lock())
		cam->lookatFrom({ 0, 0, 1 }, { 0, 0, 0 });
}

void graphics::BlenderCameraController::move(const vec2& displacement) {
	vec3 w = normalize(getPosition() - m_target);
	vec3 u = normalize(cross({0, 1, 0}, w));
	vec3 v = cross(w, u);

	m_target = m_target - u * displacement.x * (cosf(m_pitch) < 0 ? -1 : 1) - v * displacement.y * (cosf(m_pitch) < 0 ? -1 : 1);
	setCamera();
}

void graphics::BlenderCameraController::tilt(const vec2& tilt) {
	static bool c_upsideDownAtTiltBegin = false; 
	static unsigned c_prevTiltTick = 0;

	if (getTicks() > c_prevTiltTick + 2)
		c_upsideDownAtTiltBegin = cosf(m_pitch) < 0;
	c_prevTiltTick = getTicks();

	m_yaw += tilt.x * (c_upsideDownAtTiltBegin ? -1 : 1);
	m_pitch -= tilt.y;
	setCamera();
}

void graphics::BlenderCameraController::zoom(float direction, float speed) {
	if (direction == 0)
		return;
	m_distance *= direction > 0 ? 1 / (1.f + speed) : 1.f + speed;
	setCamera();
}

vec3 graphics::BlenderCameraController::getPosition() const {
	vec3 direction(cosf(m_yaw) * cosf(m_pitch), sinf(m_pitch), sinf(m_yaw) * cosf(m_pitch));
	return m_target + direction * m_distance;
}

void graphics::BlenderCameraController::setPosition(const vec3& position) {
	vec3 direction = position - m_target;
	m_distance = direction.length();
	direction = normalize(direction);

	m_pitch = asinf(direction.y);
	m_yaw = atan2f(direction.z, direction.x);
	setCamera();
}

vec3 graphics::BlenderCameraController::getTarget() const {
	return m_target;
}

void graphics::BlenderCameraController::setCamera() {
	if (auto camera = m_camera.lock()) {
		camera->lookatFrom(getPosition(), m_target);
		camera->setVUp(cosf(m_pitch) < 0 ? vec3(0, -1, 0) : vec3(0, 1, 0));
	}
}

float graphics::BlenderCameraController::getZoom() const {
	return m_distance;
}

void graphics::BlenderCameraController::setPivot(const vec3& pivot) {
	m_target = pivot;
	setCamera();
}

void ICameraController::swapProjectionMode() {
	if (auto camera = m_camera.lock())
		camera->setProjectionType(Camera::ProjectionType(!(bool)camera->getProjectionType()));
}
