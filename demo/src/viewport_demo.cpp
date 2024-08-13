#include "shader.h"
#include "texture.h"
#include "window.h"
#include "debug.h"
#include "bitmap.h"
#include "mesh.h"
#include "keyboard.h"
#include "mouse.h"
#include "object.h"
#include "imgui.h"
#include "debug.h"

using namespace graphics;

class Scene {
public:
	Scene(const std::string& name) 
		: m_name(name)
		, m_frameBuffer(Texture::createTexture(100, 100))
		, m_viewport(m_frameBuffer)
		, m_camera(std::make_shared<Camera>())
		, m_camController(m_camera)
		, m_shader(Object::DefaultShaders::matt)
	{ 
		m_viewport.properties().backFaceCullingEnabled = false;
	}
		
	void draw() {
		updateCamera();

		m_shader.setUniform(*m_camera);
		m_shader.setUniform("camPosition", m_camera->getPosition());
		m_camera->setAspectRatio(m_viewport);
		colorShader.setUniform(*m_camera);

		m_viewport.use();

		for (auto object : m_objects) {
			object->draw(m_shader);
			debug::drawLine(object->position, object->position + vec3{ 2, 0, 0 }, Color::red());
		}

		Viewport::useBackbuffer();

		m_viewport.renderAsImGuiWindow(m_name.c_str());
	}

	void addObject(Object* object) {
		m_objects.push_back(object);
	}

	void useShader(const Shader& shader) {
		m_shader = shader;
	}

private:
	std::string			 m_name;

	FrameBuffer			 m_frameBuffer;
	Viewport			 m_viewport;
	Shader				 m_shader;

	std::shared_ptr<Camera>	m_camera;
	BlenderCameraController m_camController;
	std::vector<Object*>	m_objects;

	void updateCamera() {
		bool shift = m_viewport.getDragMod(Mouse::MIDDLE, KeyMod::SHIFT);

		if (shift)
			m_camController.move(m_viewport.getDragDelta(Mouse::MIDDLE) * m_camController.getZoom());
		else
			m_camController.tilt(m_viewport.getDragDelta(Mouse::MIDDLE) * 2.f);

		if (m_viewport.isPointInside(Mouse::windowPosition()))
			m_camController.zoom(Mouse::wheel());
	}
};

int main(void) {
	Window window("viewport demo", 1280, 800);
	window.open();
	window.initImGui();


	//Mesh<vec2i, vec3> mesh;
	//std::vector<vec3> vertices {
	//	{ 0, 0, 0 },
	//	{ 1, 0, 0 },
	//	{ 1, 0, 1 },
	//};
	//std::vector<vec2i> uvs{
	//	vec2i{ 0, 0 },
	//	vec2i{ 1, 0 },
	//	vec2i{ 1, 1 }
	//};
	//std::vector<vec3> normals{
	//	vec3{ 2, 2, 2 },
	//	vec3{ 2, 3, 2 },
	//	vec3{ 2, 2, 3 },
	//};
	//std::vector<Mesh<vec2i, vec3>::FaceIndices> indices{
	//	Mesh<vec2i, vec3>::FaceIndices {
	//		Mesh<vec2i, vec3>::VertexIndices{ 0, 2, 0 },
	//		Mesh<vec2i, vec3>::VertexIndices{ 1, 1, 1 },
	//		Mesh<vec2i, vec3>::VertexIndices{ 2, 0, 2 }
	//	}
	//};
	//mesh.constructFaces(vertices, indices, uvs, normals);

	auto sphereMesh = UVMesh::loadObjFile("data/sphere.obj");
	auto cubeMesh = UVMesh::loadObjFile("data/cube.obj");
	auto cube = new Object(cubeMesh);
	auto sphere = new Object(sphereMesh);
	sphere->scale = sphere->scale * 50.f;

	Scene scene1("Scene 1");
	scene1.addObject(sphere);
	scene1.addObject(cube);
	scene1.useShader(UVMesh::DefaultShaders::matt);
	scene1.useShader(Object::DefaultShaders::normal);

	Scene scene2("Scene 2");
	scene2.addObject(cube);
	scene2.useShader(Object::DefaultShaders::matt);

	while (window.isOpen()) {
		scene1.draw();
		scene2.draw();

		window.viewport().use();
		window.render();
	}

	return 0;
}
