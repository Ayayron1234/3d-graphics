#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <string>
#include <vector>
#endif // GRAPHICS_PCH

#include "primitives.h"
#include "shader.h"

namespace graphics {

//extern Shader meshShader;
//
//extern Shader meshWireframeShader;

class Mesh {
public:
	struct Shaders {
		static Shader matt;
		static Shader wireframe;
	};

	struct BoundingBox {
		vec3 min = vec3((unsigned)-1, (unsigned)-1, (unsigned)-1);
		vec3 max = vec3(-min.x, -min.x, -min.x);

		void getVertices(vec3 vertices[8]) const;

		void update(const vec3& vertex);
	};

	static std::shared_ptr<Mesh> loadFromObjFile(const std::string& path);

	void draw(Shader& shader = Shaders::matt) const;

	void drawWireframe(const Color& color = Color::black(), Shader& shader = Shaders::wireframe) const;

	BoundingBox getBoundingBox() const;

	Ray::Hit intersectRay(const Ray& ray) const;

	Mesh();
	Mesh(const Mesh&) = default;
	~Mesh();

private:
	struct FaceIndices {
		vec3i v1;
		vec3i v2;
		vec3i v3;
	};

	struct Face {
		vec3 vertex1; vec2 uv1; vec3 normal1;
		vec3 vertex2; vec2 uv2; vec3 normal2;
		vec3 vertex3; vec2 uv3; vec3 normal3;
	};

	enum class Status { OK = 0x00, UNLOADED, FILE_NOT_FOUND, FAILED };
	
	std::vector<Face>	m_faces;

	Status				m_status = Status::UNLOADED;

	BoundingBox			m_boundingBox;

	class CacheFileHeader;

	bool tryLoadCache(const std::string& path);

	void saveCache(const std::string& path) const;

	class Impl; std::shared_ptr<Impl> impl;
};

}
