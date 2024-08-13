#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <numbers>
#endif // GRAPHICS_PCH

#include "primitives.h"
#include "shader.h"
#include "window.h"

namespace graphics {

class ICameraController;

class Camera {
public:
	enum ProjectionType { PERSPECTIVE = 0x00, ORTHOGRAPHIC };

	mat4 getViewMatrix() const;

	mat4 getProjectionMatrix() const;

	void setAspectRatio(float width, float height);

	void setAspectRatio(const Window& window);

	void setAspectRatio(const Viewport& viewport);

	void setFoV(float fov);

	void setFoVDeg(float fov);

	void setNearAndFarPanes(float np, float fp);

	void lookatFrom(const vec3& position, const vec3& target);

	bool updated();

	vec3 getPosition() const;

	void setPosition(const vec3& position);

	void setVUp(const vec3& direction);

	void setProjectionType(ProjectionType type);

	ProjectionType getProjectionType() const;

	Ray castRay(vec2 ndc) const;

protected:
	vec3 m_eye; 
	vec3 m_lookat{ 0, 0, 1 };
	vec3 m_vup{ 0, 1, 0 };

	float m_fov = 75.f * std::numbers::pi_v<float> / 180.0f;
	float m_asp = 1.f;
	float m_np = .01f;
	float m_fp = 1000.f;

	bool  m_updated = true;

	ProjectionType m_projection = PERSPECTIVE;

	friend class ICameraController;
};

class ICameraController {
public:
	ICameraController(std::weak_ptr<Camera> camera)
		: m_camera(camera)
	{ }

	virtual void move(const vec2&) = 0;

	virtual void tilt(const vec2&) = 0;

	virtual void zoom(float direction, float speed) = 0;

	void swapProjectionMode();

protected:
	std::weak_ptr<Camera> m_camera;
};

class BlenderCameraController : public ICameraController {
public:
	BlenderCameraController(std::weak_ptr<Camera> camera);

	virtual void move(const vec2& displacement) override;

	virtual void tilt(const vec2& tilt) override;

	virtual void zoom(float direction, float speed = 0.1) override;

	vec3 getPosition() const;

	void setPosition(const vec3& position);

	vec3 getTarget() const;

	float getZoom() const;

	void setPivot(const vec3& pivot);

private:
	vec3	m_target;
	float	m_distance = 1.f;
	float	m_pitch = 0.f;
	float	m_yaw = std::numbers::pi_v<float> / 2.f;

	void setCamera();
};

}
