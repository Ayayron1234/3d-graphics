#include "shader.h"
#include "texture.h"
#include "window.h"
#include "debug.h"
#include "bitmap.h"
#include "mesh.h"
#include "keyboard.h"
#include "object.h"
#include "imgui.h"

#include <iostream>
#include <filesystem>
#include <fstream>

using namespace graphics;

class FileResource {
public:
	FileResource(const std::string& path)
		: m_path(path)
	{ }

	FileResource(const FileResource& resource)
		: m_path(resource.m_path)
		, m_lastModified(resource.m_lastModified)
	{ }

	bool updated() const {
		return m_swapped
			|| m_lastModified != std::filesystem::last_write_time(m_path);
	}

	void swap(const std::string& path) {
		m_path = path;
		m_swapped = true;
	}

protected:
	using time_type = std::filesystem::file_time_type;

	std::string	m_path;
	time_type			m_lastModified;
	bool				m_swapped = false;

	void loaded() {
		m_lastModified = std::filesystem::last_write_time(m_path);
		m_swapped = false;
	}

};

class TextFileResource : public FileResource {
public:
	TextFileResource(const std::string& path)
		: FileResource(path)
	{
		load();
		loaded();
	}

	void load() {
		m_ifs.open(m_path);

		// Load value and set last write time
		m_value = std::string((std::istreambuf_iterator<char>(m_ifs)), std::istreambuf_iterator<char>());
		loaded();

		m_ifs.close();
	}

	operator std::string() const {
		return m_value;
	}

private:
	std::ifstream		m_ifs;
	std::string			m_value;

};

Shader createShader(const char* vertexSourcePath, const char* fragmentSourcePath) {
	return Shader(*new TextFileResource(vertexSourcePath), *new TextFileResource(fragmentSourcePath));
}

void hitObjects(const Ray& ray, const std::vector<Object*>& objects, std::vector<Ray>& hits) {
	Ray::Hit nearest = Ray::Hit::noHit();
	for (auto object : objects) {
		auto hit = object->intersectRay(ray);
		if (hit.t > 0 && hit.t < nearest.t)
			nearest = hit;
	}

	if (nearest.t > 0)
		hits.push_back(Ray{ nearest.position, nearest.normal });
}

/*
 *	TODO: 
 *		- imgui integration
 */

int main(void) {
	Window window("3d-graphics", 1280, 800);
	window.open();
	window.initImGui();

	auto earthTexture = Texture::loadFromFile("data/earth.bmp");
	auto wallTexture = Texture::loadFromFile("data/wall.bmp");
	auto houseTexture = Texture::loadFromFile("data/house.bmp");

	BlenderCameraController camController(debug::camera);

	auto cubeMesh = Mesh::loadFromObjFile("data/cube.obj");
	auto houseMesh = Mesh::loadFromObjFile("data/house.obj");
	auto attractorMesh = Mesh::loadFromObjFile("data/attractors.obj");
	auto sphereMesh = Mesh::loadFromObjFile("data/sphere.obj");
	auto wallMesh = Mesh::loadFromObjFile("data/wall.obj");
	auto planeMesh = Mesh::loadFromObjFile("data/plane.obj");

	std::vector<Object*> objects{};
	Object house(houseMesh, houseTexture); objects.push_back(&house);
	Object cube(cubeMesh); objects.push_back(&cube);
	Object sphere(sphereMesh, earthTexture); objects.push_back(&sphere);
	Object wall(wallMesh, wallTexture); objects.push_back(&wall);
	Object plane(planeMesh); objects.push_back(&plane);

	cube.setPosition({ 14 });
	sphere.setPosition({ -14 });
	wall.setScale({ .4, .4, .4 });
	house.setPosition({ 0, 0.01, -5 });
	house.setScale({ .4, .4, .4 });
	house.tilt(0.f, 0.5f);
	plane.setScale({ 100.f, 1.f, 100.f });

	std::vector<Ray> rays{};

	while(window.isOpen()) {
		// camera
		if (window.isMouseButtonDown(MouseButton::MIDDLE) && !isModPressed(KeyMod::LSHIFT))
			camController.tilt(window.getMouseMotion() * vec2(.007f, -.01f));
		if (window.isMouseButtonDown(MouseButton::MIDDLE) && isModPressed(KeyMod::LSHIFT))
			camController.move(window.getMouseMotion() * vec2(.002f, -.002f) * camController.getZoom());
		if (keyPressed(Key::SPACE)) {
			camController.setPivot({ 0, 0, 0 });
			camController.setPosition({ 0, 4, 5 });
		}
		camController.zoom(window.getMouseWheel());

		Object::Shaders::matt.setUniform("camPosition", camController.getPosition());

		wall.draw(Object::Shaders::matt);
		cube.draw(Object::Shaders::normal);
		cube.drawBoundingBox();
		sphere.draw();
		sphere.drawWireframe();
		plane.draw(Object::Shaders::matt);
		house.draw(Object::Shaders::matt);

		cube.tilt(0.f, 0.004f);

		if (window.mouseClicked(MouseButton::RIGHT))
			camController.swapProjectionMode();

		static std::vector<std::vector<Ray>> hits{};
		if (keyPressed(Key::H)) {
			hits.clear();

			int stripCount = 20;
			int columnCount = 20;
			float yInc = 2.f / stripCount;
			float xInc = 2.f / columnCount;
			for (float y = -1.f; y < 1.f; y += yInc) {
				hits.push_back(std::vector<Ray>{});
				for (float x = -1.f; x < 1.f; x += xInc)
					hitObjects(debug::camera->castRay({ x, y }), objects, hits.back());
			}
		}

		for (int j = 0; j < hits.size(); ++j) {
			for (int i = 0; i < hits.at(j).size(); ++i) {
				if (j == hits.size() - 1 || i == hits.at(0).size() - 1)
					continue;

				float t = 0.01f;
				debug::drawTrig(hits.at(j).at(i).at(t), hits.at(j).at(i + 1).at(t), hits.at(j + 1).at(i).at(t));
				debug::drawTrig(hits.at(j + 1).at(i).at(t), hits.at(j + 1).at(i + 1).at(t), hits.at(j).at(i + 1).at(t));
			}
		}

		if (keyPressed(Key::C)) {
			hits.clear();
			rays.clear();
		}


		if (window.isMouseButtonDown() && window.mouseMoved()) {
			hitObjects(debug::camera->castRay(window.getMousePosition()), objects, rays);
		}
		for (auto& ray : rays)
			debug::drawLine(ray.origin, ray.origin + ray.direction, Color::red());

		debug::drawLine(camController.getTarget(), camController.getTarget() + vec3(.1f, 0, 0) * camController.getZoom(), Color::red());
		debug::drawLine(camController.getTarget(), camController.getTarget() + vec3(0, .1f, 0) * camController.getZoom(), Color::green());
		debug::drawLine(camController.getTarget(), camController.getTarget() + vec3(0, 0, .1f) * camController.getZoom(), Color::lightBlue());

		ImGui::ShowDemoWindow();

		window.render();
	}

	return 0;
}
