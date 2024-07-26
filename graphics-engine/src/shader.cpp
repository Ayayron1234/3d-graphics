#include "pch.h"
#include "shader.h"
#include "graphics_headers.h"
#include "primitive_drawer.h"

const char* g_colorVertSource = R"(
#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;

out vec4 vertexColor;

uniform mat4 VP;

void main()
{
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0) * VP;
	vertexColor = color;
}
)";

const char* g_colorFragSource = R"(
#version 330 core

in vec4 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vertexColor;
}
)";

const char* g_textureVertSource = R"(
#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;

uniform mat4 VP;

out vec4 vertexColor;
out vec2 vertexTexCoord;

void main()
{
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0) * VP;
	vertexTexCoord = texCoord;
}
)";

const char* g_textureFragSource = R"(
#version 330 core

uniform sampler2D textureSampler;

in vec2 vertexTexCoord;
out vec4 FragColor;

void main()
{
    FragColor = texture(textureSampler, vertexTexCoord);
}
)";

using namespace graphics;

static ShaderSourceWrapperImpl colorShaderVertexSource = ShaderSourceWrapperImpl(g_colorVertSource);
static ShaderSourceWrapperImpl colorShaderFragmentSource = ShaderSourceWrapperImpl(g_colorFragSource);

static ShaderSourceWrapperImpl textureShaderVertexSource = ShaderSourceWrapperImpl(g_textureVertSource);
static ShaderSourceWrapperImpl textureShaderFragmentSource = ShaderSourceWrapperImpl(g_textureFragSource);

Shader graphics::colorShader = Shader(colorShaderVertexSource, colorShaderFragmentSource);
Shader graphics::textureShader = Shader(textureShaderVertexSource, textureShaderFragmentSource);

GLint getLocation(const Shader& shader, const std::string& name) {
	return glGetUniformLocation(shader.id(), name.c_str());
}

template <typename T>
void setUniform(const Shader& shader, const std::string& name, const T& value) {
}

template <>
void setUniform(const Shader& shader, const std::string& name, const int& value) {
	glUniform1i(getLocation(shader, name), value);
}

template <>
void setUniform(const Shader& shader, const std::string& name, const float& value) {
	glUniform1f(getLocation(shader, name), value);
}

template <>
void setUniform(const Shader& shader, const std::string& name, const double& value) {
	glUniform1d(getLocation(shader, name), value);
}

template <>
void setUniform(const Shader& shader, const std::string& name, const vec2& value) {
	glUniform2fv(getLocation(shader, name), 1, (float*)&value);
}

template <>
void setUniform(const Shader& shader, const std::string& name, const vec3& value) {
	glUniform3fv(getLocation(shader, name), 1, (float*)&value);
}

template <>
void setUniform(const Shader& shader, const std::string& name, const vec4& value) {
	glUniform4fv(getLocation(shader, name), 1, (float*)&value);
}

template <>
void setUniform(const Shader& shader, const std::string& name, const mat4& value) {
	glUniformMatrix4fv(getLocation(shader, name), 1, GL_TRUE, (float*)&value.rows[0]);
}

template <>
void setUniform(const Shader& shader, const std::string& name, const Texture& texture) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.id());
	glUniform1i(getLocation(shader, name), 0);
}

template <>
void setUniform(const Shader& shader, const std::string& name, const Camera& camera) {
	mat4 vp = camera.getViewMatrix() * camera.getProjectionMatrix();
	glUniformMatrix4fv(getLocation(shader, name), 1, GL_TRUE, (float*)&vp.rows[0]);
}

template <typename T>
class Shader::StaticUniformSource : public Shader::IUniformSource {
public:
	StaticUniformSource(const T& data)
		: m_data(data)
	{ }

	virtual void set(const Shader& shader, const std::string& name) const override {
		::setUniform(shader, name, m_data);
	}
private:
	T m_data;
};

template <typename T>
class Shader::DynamicUniformSource : public Shader::IUniformSource {
public:
	DynamicUniformSource(std::weak_ptr<T> ptr) 
		: m_data(ptr)
	{ }

	void set(const Shader& shader, const std::string& name) const override {
		if (std::shared_ptr<T> data = m_data.lock()) {
			::setUniform(shader, name, *data);
		}
	}

private:
	std::weak_ptr<T> m_data;
};

unsigned int Shader::id() const {
	return m_id;
}

void Shader::use() {
	if (!m_compiled)
		compile();

	if (s_currentShaderID != m_id)
		PrimitiveDrawer::pushAll();

	glUseProgram(m_id);
	s_currentShaderID = id();

	for (auto& [uniformName, uniformSource] : m_uniforms) {
		uniformSource->set(*this, uniformName);
	}
}

void Shader::setUniform(const std::string& name, const int& value) {
	insertOrUpdatedUniform(name, std::make_unique<StaticUniformSource<int>>(value));
}

void Shader::setUniform(const std::string& name, const float& value) {
	insertOrUpdatedUniform(name, std::make_unique<StaticUniformSource<float>>(value));
}

void graphics::Shader::setUniform(const std::string& name, const double& value) {
	insertOrUpdatedUniform(name, std::make_unique<StaticUniformSource<double>>(value));
}

void Shader::setUniform(const std::string& name, const vec2& vec) {
	insertOrUpdatedUniform(name, std::make_unique<StaticUniformSource<vec2>>(vec));
}

void Shader::setUniform(const std::string& name, const vec3& vec) {
	insertOrUpdatedUniform(name, std::make_unique<StaticUniformSource<vec3>>(vec));
}

void Shader::setUniform(const std::string& name, const vec4& vec) {
	insertOrUpdatedUniform(name, std::make_unique<StaticUniformSource<vec4>>(vec));
}

void graphics::Shader::setUniform(const std::string& name, const mat4& mat) {
	insertOrUpdatedUniform(name, std::make_unique<StaticUniformSource<mat4>>(mat));
}

void graphics::Shader::setUniform(const Camera& camera) {
	mat4 vp = camera.getViewMatrix() * camera.getProjectionMatrix();
	insertOrUpdatedUniform("VP", std::make_unique<StaticUniformSource<mat4>>(vp));
}

void graphics::Shader::setUniform(const std::string& name, const Camera& camera) {
	mat4 vp = camera.getViewMatrix() * camera.getProjectionMatrix();
	insertOrUpdatedUniform(name, std::make_unique<StaticUniformSource<mat4>>(vp));
}

void graphics::Shader::setUniform(const std::string& name, const Texture& texture) {
	insertOrUpdatedUniform(name, std::make_unique<StaticUniformSource<Texture>>(texture));
}

void graphics::Shader::bindUniform(const std::string& name, std::weak_ptr<Camera> camera) {
	insertOrUpdatedUniform(name, std::make_unique<DynamicUniformSource<Camera>>(camera));
}

unsigned int graphics::Shader::currentID() {
	return s_currentShaderID;
}

graphics::Shader::Shader() { }

graphics::Shader::~Shader() { }

void Shader::compile() {
	m_vertexSource->compile();
	m_fragmentSource->compile();

	if (m_geometrySource)
		m_geometrySource->compile();

	// Create program
	m_id = glCreateProgram();
	if (!id()) {
		std::cerr << "Error in shader program creation" << std::endl;
		exit(1);
	}

	attachAndLinkShaders();

	m_compiled = true;
}

void writeShaderCompilationErrorInfo(unsigned int handle) {
	int logLen, written;
	glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLen);

	if (logLen > 0) {
		std::string log(logLen, '\0');
		glGetShaderInfoLog(handle, logLen, &written, &log[0]);

		debug::cout << "Shader log:\n" << log << std::endl;
	}
}

bool checkLinking(unsigned int program) {
	int OK;
	glGetProgramiv(program, GL_LINK_STATUS, &OK);
	if (!OK) {
		std::cerr << "Failed to link shader program!" << std::endl;
		writeShaderCompilationErrorInfo(program);
		return false;
	}
	return true;
}

void Shader::attachAndLinkShaders() {
	// Attach shaders
	glAttachShader(id(), m_vertexSource->id());
	glAttachShader(id(), m_fragmentSource->id());
	if (m_geometrySource)
		glAttachShader(id(), m_geometrySource->id());

	// Connect the fragmentColor to the frame buffer memory
	glBindFragDataLocation(id(), 0, "outColor");

	// program packaging
	glLinkProgram(id());
	if (!checkLinking(id()))
		exit(1);

	glUseProgram(id());
	s_currentShaderID = id();
}

void graphics::Shader::insertOrUpdatedUniform(const std::string& name, std::unique_ptr<IUniformSource>&& uniform) {
	auto it = m_uniforms.find(name);
	if (it == m_uniforms.end())
		m_uniforms.insert({ name, std::move(uniform) });
	else
		m_uniforms[name] = std::move(uniform);
}

inline bool Shader::IShaderSourceWrapper::compile() {
	// Create shader if not created yet
	if (m_id == 0)
		m_id = glCreateShader(m_type);

	// Check for errors
	if (!m_id) {
		std::cerr << "Error creating vertex shader" << std::endl;
		exit(1);
	}

	// Compile shader
	std::string sourceStr = operator std::string();
	const char* source = sourceStr.c_str();
	glShaderSource(m_id, 1, (const GLchar**)&source, NULL);
	glCompileShader(m_id);

	// Return compilation status
	return checkShaderCompilation();
}

bool Shader::IShaderSourceWrapper::checkShaderCompilation() {
	int OK;
	glGetShaderiv(m_id, GL_COMPILE_STATUS, &OK);
	if (!OK) {
		std::cerr << "Failed to compile shader!" << std::endl;
		writeShaderCompilationErrorInfo(m_id);
		return false;
	}
	return true;
}
