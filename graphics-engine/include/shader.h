#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#include <string>
#include <unordered_map>
#include <memory>
#endif // GRAPHICS_PCH

#include "primitives.h"
#include "texture.h"

namespace graphics {

class Camera;

template <typename T>
concept ShaderSource = requires(T t) {
	{ t.operator std::string() };
};

class Shader {
public:
	template <ShaderSource T>
	Shader(T& vertexSource, T& fragmentSource, T& geometrySource);

	template <ShaderSource T>
	Shader(T& vertexSource, T& fragmentSource);

	unsigned int id() const;

	/**
	 * \brief Selects shader for draw calls and updates uniforms. 	
	*/
	void use();

	void setUniform(const std::string& name, const int& value);

	void setUniform(const std::string& name, const float& value);

	void setUniform(const std::string& name, const double& value);

	void setUniform(const std::string& name, const vec2& vec);

	void setUniform(const std::string& name, const vec3& vec);

	void setUniform(const std::string& name, const vec4& vec);

	void setUniform(const std::string& name, const mat4& mat);

	void setUniform(const Camera& camera);

	void setUniform(const std::string& name, const Camera& camera);

	void setUniform(const std::string& name, const Texture& texture);

	void bindUniform(const std::string& name, std::weak_ptr<Camera> camera);

	static unsigned int currentID();

	static Shader loadFromTextFiles(const char* vertexSourcePath, const char* fragmentSourcePath, const char* geometrySourcePath = nullptr);

	~Shader();

private:
	unsigned m_id = 0;
	bool	 m_compiled = false;

	inline static unsigned s_currentShaderID = 0;

	struct IShaderSourceWrapper;

	template <ShaderSource T>
	struct ShaderSourceWrapper;

	IShaderSourceWrapper* m_vertexSource = nullptr;
	IShaderSourceWrapper* m_fragmentSource = nullptr;
	IShaderSourceWrapper* m_geometrySource = nullptr;

	class IUniformSource {
	public:
		virtual void set(const Shader& shader, const std::string& name) const = 0;
	};

	template <typename T>
	class StaticUniformSource;

	template <typename T>
	class DynamicUniformSource;

	std::shared_ptr<std::unordered_map<std::string, std::unique_ptr<IUniformSource>>> m_uniforms 
		= std::make_shared<std::unordered_map<std::string, std::unique_ptr<IUniformSource>>>();

	Shader();

	void compile();

	void attachAndLinkShaders();

	void insertOrUpdatedUniform(const std::string& name, std::unique_ptr<IUniformSource>&& uniform);
};

extern Shader colorShader;
extern Shader textureShader;

struct Shader::IShaderSourceWrapper {
	IShaderSourceWrapper(int type)
		: m_type(type)
	{ }

	virtual operator std::string() const = 0;

	bool compile();

	unsigned id() const {
		return m_id;
	}

private:
	unsigned m_id = 0;
	int		 m_type = 0;

	bool checkShaderCompilation();
};

template <ShaderSource T>
struct Shader::ShaderSourceWrapper : public Shader::IShaderSourceWrapper {
	ShaderSourceWrapper(T& origin, int shaderType)
		: IShaderSourceWrapper(shaderType)
		, m_origin(origin)
	{ }

	virtual operator std::string() const override {
		return m_origin.operator std::string().c_str();
	}

private:
	T& m_origin;
};

template<ShaderSource T>
inline Shader::Shader(T& vertexSource, T& fragmentSource, T& geometrySource) {
	m_vertexSource   = new ShaderSourceWrapper<T>(vertexSource, 0x8B31);
	m_fragmentSource = new ShaderSourceWrapper<T>(fragmentSource, 0x8B30);
	m_geometrySource = new ShaderSourceWrapper<T>(geometrySource, 0x8DD9);
}

template<ShaderSource T>
inline Shader::Shader(T& vertexSource, T& fragmentSource) {
	m_vertexSource   = new ShaderSourceWrapper<T>(vertexSource, 0x8B31);
	m_fragmentSource = new ShaderSourceWrapper<T>(fragmentSource, 0x8B30);
}

}
