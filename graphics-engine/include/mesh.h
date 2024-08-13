#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <string>
#include <vector>
#include <tuple>
#endif // GRAPHICS_PCH

#include "primitives.h"
#include "shader.h"

namespace graphics {

//extern Shader meshShader;
//
//extern Shader meshWireframeShader;
// TODO: generalize mesh. make vertex data into a template and in place of the current mesh create a class UVMesh : Mesh<vec2, vec3>

class MeshBase {
public:
//	void constructFaces(const std::vector<vec3>& vertices, const std::vector<vec2>& uvs,
//		const std::vector<vec3>& normals, const std::vector<FaceIndices>& indices);
//
//
//	BoundingBox getBoundingBox() const;
//
//	Ray::Hit intersectRay(const Ray& ray) const;

	BoundingBox getBoundingBox() const;

	virtual void draw(Shader& shader) const = 0;

	virtual Ray::Hit intersectRay(const Ray& ray) const = 0;

	MeshBase();
	MeshBase(const MeshBase&) = default;
	~MeshBase();

protected:
	class Impl; std::unique_ptr<Impl> m_impl;

	enum class ShaderValueType { 
		FLOAT = 0x00, 
		DOUBLE, 
		BYTE, UNSIGNED_BYTE, 
		SHORT, UNSIGNED_SHORT, 
		INT, UNSIGNED_INT };

	enum class Status { 
		OK = 0x00, 
		UNLOADED, 
		FILE_NOT_FOUND, 
		FAILED };

	Status		 m_status = Status::UNLOADED;
	BoundingBox	 m_boundingBox;

	bool tryLoadCache(const std::string& path, size_t faceSizeBytes, void** faces, size_t* faceCount);

	void saveCache(const std::string& path, void* faces, size_t faceSizeBytes, unsigned faceCount) const;

	void draw(Shader& shader, void* faces, size_t faceSize, unsigned faceCount, 
		const std::vector<ShaderValueType>& faceTypes, const std::vector<unsigned>& faceValueCounts) const;

	template <typename T>
	static ShaderValueType getValueType();

	template <typename T>
	static unsigned getValueCount();

	void update();
};

// Provide template arguments in reverse order
template <typename... Args>
class Mesh : public MeshBase {
public:
	using VertexData = std::tuple<Args...>;
	using VertexIndices = std::array<int, sizeof...(Args) + 1>;

	struct Face {
		vec3 vertex1; VertexData data1;
		vec3 vertex2; VertexData data2;
		vec3 vertex3; VertexData data3;
	};

	using FaceIndices = std::array<VertexIndices, 3>;

public:
	void constructFaces(
		const std::vector<vec3>& vertices,
		const std::vector<FaceIndices>& indices,
		const std::vector<Args>& ...vertexData);

	virtual void draw(Shader& shader) const override {
		MeshBase::draw(shader, (void*)m_faces.data(), sizeof(Face), m_faces.size(), m_types, m_counts);
	}

	Mesh() {
		m_types.push_back(getValueType<vec3>());
		m_counts.push_back(getValueCount<vec3>());

		(m_types.push_back(getValueType<Args>()), ...);
		(m_counts.push_back(getValueCount<Args>()), ...);
	}

	virtual Ray::Hit intersectRay(const Ray& ray) const override;

protected:
	std::vector<Face>			 m_faces;

	std::vector<ShaderValueType> m_types;
	std::vector<unsigned>		 m_counts;

	Face getFace(
		const FaceIndices& faceIndices,
		const std::vector<vec3>& vertices,
		const std::vector<Args>&... vertexData);

	template <size_t... Is>
	auto getVertexDataAt(const VertexIndices& vertexIndices, std::index_sequence<Is...>, const std::vector<Args>& ...vertexData);
};

struct UVMesh : public Mesh<vec3, vec2> {
	static std::shared_ptr<UVMesh> loadObjFile(const std::string& path);

	struct DefaultShaders {
		static Shader matt;
	};

	virtual void draw(Shader& shader = DefaultShaders::matt) const override {
		Mesh::draw(shader);
	}
};

//
//class Mesh {
//public:
//	struct Shaders {
//		static Shader matt;
//		static Shader wireframe;
//	};
//
//	struct BoundingBox {
//		vec3 min = vec3((unsigned)-1, (unsigned)-1, (unsigned)-1);
//		vec3 max = vec3(-min.x, -min.x, -min.x);
//
//		void getVertices(vec3 vertices[8]) const;
//
//		void update(const vec3& vertex);
//	};
//
//	struct Face {
//		vec3 vertex1; vec2 uv1; vec3 normal1;
//		vec3 vertex2; vec2 uv2; vec3 normal2;
//		vec3 vertex3; vec2 uv3; vec3 normal3;
//	};
//
//	struct FaceIndices {
//		vec3i v1;
//		vec3i v2;
//		vec3i v3;
//	};
//
//	static std::shared_ptr<Mesh> loadFromObjFile(const std::string& path);
//
//	size_t faceCount() const;
//
//	void constructFaces(const std::vector<vec3>& vertices, const std::vector<vec2>& uvs,
//		const std::vector<vec3>& normals, const std::vector<FaceIndices>& indices);
//
//	void draw(Shader& shader = Shaders::matt) const;
//
//	void drawWireframe(const Color& color = Color::black(), Shader& shader = Shaders::wireframe) const;
//
//	BoundingBox getBoundingBox() const;
//
//	Ray::Hit intersectRay(const Ray& ray) const;
//
//	Mesh();
//	Mesh(const Mesh&) = default;
//	~Mesh();
//
//private:
//
//	enum class Status { OK = 0x00, UNLOADED, FILE_NOT_FOUND, FAILED };
//
//	std::vector<Face>		 m_faces;
//
//	Status					 m_status = Status::UNLOADED;
//
//	BoundingBox				 m_boundingBox;
//
//	class CacheFileHeader;
//
//	Face getFace(const std::vector<vec3>& vertices, const std::vector<vec2>& uvs,
//		const std::vector<vec3>& normals, const std::vector<FaceIndices>& indices, size_t index) const;
//
//	bool tryLoadCache(const std::string& path);
//
//	void saveCache(const std::string& path) const;
//
//	class Impl; std::shared_ptr<Impl> impl;
//};

template<typename ...Args>
inline void Mesh<Args...>::constructFaces(
	const std::vector<vec3>& vertices, 
	const std::vector<FaceIndices>& indices,
	const std::vector<Args>& ...vertexData)
{
	m_faces.clear();
	m_faces.resize(indices.size());

	m_boundingBox = BoundingBox();
	for (auto& vertex : vertices)
		m_boundingBox.update(vertex);

	for (int i = 0; i < indices.size(); ++i)
		m_faces.at(i) = getFace(indices.at(i), vertices, vertexData...);

	MeshBase::update();
}

template<typename ...Args>
template <size_t... Is>
auto Mesh<Args...>::getVertexDataAt(
	const VertexIndices& vertexIndices,
	std::index_sequence<Is...>,
	const std::vector<Args>& ...vertexData) 
{
	return std::make_tuple(vertexData.at(std::get<Is + 1>(vertexIndices))...);
}

template<typename ...Args>
inline Ray::Hit Mesh<Args...>::intersectRay(const Ray& ray) const {
	Ray::Hit nearest = Ray::Hit::noHit();
	
	Ray::Hit aabbHit = ray.intersectAABB(m_boundingBox.min, m_boundingBox.max);
	if (!aabbHit.didHit())
	    return nearest;
	
	for (auto& face : m_faces) {
	    Ray::Hit hit = ray.intersectTrig(face.vertex1, face.vertex2, face.vertex3);
	    if (hit.t < nearest.t)
	        nearest = hit;
	}
	return nearest;
}

template<typename ...Args>
inline Mesh<Args...>::Face Mesh<Args...>::getFace(
	const FaceIndices& faceIndices, 
	const std::vector<vec3>& vertices, 
	const std::vector<Args>& ...vertexData) 
{
    return Face {
		vertices.at(std::get<0>(std::get<0>(faceIndices))),
		getVertexDataAt(std::get<0>(faceIndices), std::make_index_sequence<sizeof...(Args)>(), vertexData...),
		vertices.at(std::get<0>(std::get<1>(faceIndices))),
		getVertexDataAt(std::get<1>(faceIndices), std::make_index_sequence<sizeof...(Args)>(), vertexData...),
		vertices.at(std::get<0>(std::get<2>(faceIndices))),
		getVertexDataAt(std::get<2>(faceIndices), std::make_index_sequence<sizeof...(Args)>(), vertexData...)
    };
}

template<typename T>
inline MeshBase::ShaderValueType MeshBase::getValueType() {
	
}

template<typename T>
inline unsigned MeshBase::getValueCount() {
	return 1u;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<float>() {
	return ShaderValueType::FLOAT;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<double>() {
	return ShaderValueType::DOUBLE;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<char>() {
	return ShaderValueType::BYTE;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<unsigned char>() {
	return ShaderValueType::UNSIGNED_BYTE;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<short>() {
	return ShaderValueType::SHORT;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<unsigned short>() {
	return ShaderValueType::UNSIGNED_SHORT;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<int>() {
	return ShaderValueType::INT;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<unsigned int>() {
	return ShaderValueType::UNSIGNED_INT;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<vec2>() {
	return ShaderValueType::FLOAT;
}

template<>
inline unsigned MeshBase::getValueCount<vec2>() {
	return 2u;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<vec3>() {
	return ShaderValueType::FLOAT;
}

template<>
inline unsigned MeshBase::getValueCount<vec3>() {
	return 3u;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<vec4>() {
	return ShaderValueType::FLOAT;
}

template<>
inline unsigned MeshBase::getValueCount<vec4>() {
	return 4u;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<Color::FColor>() {
	return ShaderValueType::FLOAT;
}

template<>
inline unsigned MeshBase::getValueCount<Color::FColor>() {
	return 4u;
}

template<>
inline MeshBase::ShaderValueType MeshBase::getValueType<vec2i>() {
	return ShaderValueType::INT;
}

template<>
inline unsigned MeshBase::getValueCount<vec2i>() {
	return 2u;
}


}
