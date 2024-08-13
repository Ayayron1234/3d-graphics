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
	vec3			position;
	vec3			scale{ 1, 1, 1 };
	vec3			rotation; // yaw pitch roll

	Object(std::shared_ptr<MeshBase> mesh, const Texture& texture = Texture::null());

	struct DefaultShaders {
		static Shader matt;
		static Shader normal;
		static Shader wireframe;
		static Shader textured;
	};
	
	void draw(Shader& shader = DefaultShaders::textured) const;

	BoundingBox getBoundingBox() const;

	void drawBoundingBox() const;

	Ray::Hit intersectRay(const Ray& ray) const;

	mat4 getModelMatrix() const;

	std::shared_ptr<MeshBase> getMesh() const;

private:
	template <typename T>
	using sptr = std::shared_ptr<T>;

	sptr<MeshBase>	m_mesh;
	Texture			m_texture;
};

}
