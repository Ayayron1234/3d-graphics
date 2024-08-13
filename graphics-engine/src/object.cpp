#include "pch.h"
#include "object.h"

#include "primitive_drawer.h"

using namespace graphics;

const char* g_normalObjectVertSource = R"(
#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

uniform mat4 M;
uniform mat4 VP;

out vec3 Normal;
out vec2 UV;

void main()
{
    gl_Position = vec4(vertex.xyz, 1.0) * M * VP;

    Normal = normalize(normal * mat3(transpose(inverse(M))));
    UV = vec2(uv.x, 1.f - uv.y);
}
)";

const char* g_normalObjectFragSource = R"(
#version 330 core

uniform sampler2D tex2D;

in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

void main()
{
    FragColor = vec4((Normal + vec3(1.f, 1.f, 1.f)) * .5f, 1.f);
}
)";

const char* g_texturedObjectVertSource = R"(
#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

uniform mat4 M;
uniform mat4 VP;

out vec3 Normal;
out vec2 UV;

void main()
{
    gl_Position = vec4(vertex.xyz, 1.0) * M * VP;

    Normal = normal * mat3(transpose(inverse(M)));
    UV = vec2(uv.x, 1.f - uv.y);
}
)";

const char* g_texturedObjectFragSource = R"(
#version 330 core

uniform sampler2D tex2D;

in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

void main()
{
    FragColor = texture(tex2D, UV);
}
)";

const char* g_mattObjectVertSource = R"(
#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

uniform mat4 M;
uniform mat4 VP;

out vec3 Normal;
out vec3 Position;

void main()
{
    gl_Position = vec4(vertex.xyz, 1.0) * M * VP;
    
    vec4 modelTransformed = vec4(vertex.xyz, 1.0) * M;
    Position = vec3(modelTransformed.x / modelTransformed.w, modelTransformed.y / modelTransformed.w, modelTransformed.z / modelTransformed.w);

    Normal = normal * mat3(transpose(inverse(M)));
}
)";

const char* g_mattObjectFragSource = R"(
#version 330 core

uniform sampler2D textureSampler;
uniform mat4 VP;
uniform vec3 camPosition;

in vec3 Normal;
in vec3 Position;

out vec4 FragColor;

void main()
{
    float camPosDot = dot(normalize(camPosition - Position), normalize(Normal));
    if (camPosDot < 0)
        camPosDot *= -1;

    float diffuse = .45 + camPosDot / 3;

    //FragColor = vec4(Position.x - round(Position.x), Position.y - round(Position.y), Position.z - round(Position.z), 1.f);
    FragColor = vec4(diffuse, diffuse, diffuse, 1.f);
}
)";

const char* g_objectWireframeVertSource = R"(
#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

uniform mat4 M;
uniform mat4 VP;

void main()
{
    gl_Position = vec4(vertex.xyz, 1.0) * M * VP;
}
)";

const char* g_objectWireframeFragSource = R"(
#version 330 core

uniform vec4 color;

out vec4 FragColor;

void main()
{
    FragColor = color;
}
)";

static ShaderSourceWrapperImpl mattObjectShaderVertexSource = ShaderSourceWrapperImpl(g_mattObjectVertSource);
static ShaderSourceWrapperImpl mattObjectShaderFragmentSource = ShaderSourceWrapperImpl(g_mattObjectFragSource);

Shader Object::DefaultShaders::matt = Shader(mattObjectShaderVertexSource, mattObjectShaderFragmentSource);

static ShaderSourceWrapperImpl normalObjectShaderVertexSource = ShaderSourceWrapperImpl(g_normalObjectVertSource);
static ShaderSourceWrapperImpl normalObjectShaderFragmentSource = ShaderSourceWrapperImpl(g_normalObjectFragSource);

Shader Object::DefaultShaders::normal = Shader(normalObjectShaderVertexSource, normalObjectShaderFragmentSource);

static ShaderSourceWrapperImpl objectWireframeShaderVertexSource = ShaderSourceWrapperImpl(g_objectWireframeVertSource);
static ShaderSourceWrapperImpl objectWireframeShaderFragmentSource = ShaderSourceWrapperImpl(g_objectWireframeFragSource);

Shader Object::DefaultShaders::wireframe = Shader(objectWireframeShaderVertexSource, objectWireframeShaderFragmentSource);

static ShaderSourceWrapperImpl texturedObjectShaderVertexSource = ShaderSourceWrapperImpl(g_texturedObjectVertSource);
static ShaderSourceWrapperImpl texturedObjectShaderFragmentSource = ShaderSourceWrapperImpl(g_texturedObjectFragSource);

Shader Object::DefaultShaders::textured = Shader(texturedObjectShaderVertexSource, texturedObjectShaderFragmentSource);

Object::Object(std::shared_ptr<MeshBase> mesh, const Texture& texture)
	: m_mesh(mesh)
	, m_texture(texture)
{ }

mat4 Object::getModelMatrix() const {
	return mat4::scale(scale) * mat4::rotate(rotation) * mat4::translation(position);
}

void Object::draw(Shader& shader) const {
    shader.setUniform("M", getModelMatrix());
    if (!m_texture.empty())
        shader.setUniform("tex2D", m_texture);
    m_mesh->draw(shader);
}

BoundingBox graphics::Object::getBoundingBox() const {
    BoundingBox boundingBox;
    
    mat4 M = getModelMatrix();

    vec3 vertices[8]{};
    m_mesh->getBoundingBox().getVertices(vertices);

    for (int i = 0; i < 8; ++i) {
        vec4 _v = vec4(vertices[i].x, vertices[i].y, vertices[i].z, 1.f);
        _v = _v * M;
        boundingBox.update(vec3(_v.x / _v.w, _v.y / _v.w, _v.z / _v.w));
    }

    return boundingBox;
}

void graphics::Object::drawBoundingBox() const {
    auto boundingBox = getBoundingBox();
    vec3 vertices[8]{};
    getBoundingBox().getVertices(vertices);

    debug::drawLine(vertices[0], vertices[1], Color::lightBlue());
    debug::drawLine(vertices[0], vertices[2], Color::red());
    debug::drawLine(vertices[1], vertices[3]);
    debug::drawLine(vertices[2], vertices[3]);
    
    debug::drawLine(vertices[0], vertices[4], Color::green());
    debug::drawLine(vertices[1], vertices[5]);
    debug::drawLine(vertices[2], vertices[6]);
    debug::drawLine(vertices[3], vertices[7]);

    debug::drawLine(vertices[4], vertices[5]);
    debug::drawLine(vertices[4], vertices[6]);
    debug::drawLine(vertices[5], vertices[7]);
    debug::drawLine(vertices[6], vertices[7]);
}

std::shared_ptr<MeshBase> graphics::Object::getMesh() const {
    return m_mesh;
}

//void Object::drawWireframe(const Color& color, Shader& shader) const {
//    shader.setUniform("M", getModelMatrix());
//    m_mesh->drawWireframe(color, shader);
//}

Ray::Hit graphics::Object::intersectRay(const Ray& ray) const {
    mat4 inverseTransform = mat4::inverseScale(scale) * mat4::inverseRotation(rotation);

    Ray transformed = ray;
    transformed.origin = vec4(ray.origin - position, 1.f) * inverseTransform;
    transformed.direction = normalize(vec4(ray.direction, 0.f) * inverseTransform);

    Ray::Hit hit = m_mesh->intersectRay(transformed);

    mat4 transform = mat4::scale(scale) * mat4::rotate(rotation);
    hit.position = vec4(hit.position, 1.f) * transform + position;
    hit.normal = normalize(vec4(hit.normal, 0.f) * transform);
    hit.t = hit.t * scale.length();

    return hit;
}
