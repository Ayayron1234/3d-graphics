#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <memory>
#endif // GRAPHICS_PCH

#include "shader.h"
#include "mesh.h"

namespace graphics {

class Object {
public:
	struct Shaders {
		static Shader matt;
		static Shader normal;
		static Shader wireframe;
		static Shader textured;
	};

	Object(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> texture = nullptr);

	mat4 getModelMatrix() const;

	void draw(Shader& shader = Shaders::textured) const;

	vec3 getPosition() const;

	void setPosition(const vec3& position);

	void move(const vec3& direction);

	vec3 getScale() const;

	void setScale(const vec3& scale);

	void setRotation(const vec3& rotation);

	void tilt(const vec3& rotation);

	void tilt(float yaw, float pitch = 0, float roll = 0);

	Mesh::BoundingBox getBoundingBox() const;

	void drawBoundingBox() const;

	void drawWireframe(const Color& color = Color::black(), Shader& shader = Shaders::wireframe) const;

	Ray::Hit intersectRay(const Ray& ray) const;

private:
	template <typename T>
	using sptr = std::shared_ptr<T>;

	sptr<Mesh>		m_mesh;
	sptr<Texture>	m_texture;
	
	vec3			m_position;
	vec3			m_scale{ 1, 1, 1 };
	vec3			m_rotation; // yaw pitch roll
};

}
