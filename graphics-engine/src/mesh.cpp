#include "pch.h"
#include "mesh.h"
#include "graphics_headers.h"
#include "primitive_drawer.h"

using namespace graphics;

const char* g_meshVertSource = R"(
#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

uniform mat4 VP;

out vec3 Position;
out vec2 UV;
out vec3 Normal;

void main()
{
    gl_Position = vec4(vertex.xyz, 1.0) * VP;

	Position = vertex;
    UV = uv;
    Normal = normal;
}
)";

const char* g_meshFragSource = R"(
#version 330 core

uniform sampler2D textureSampler;

in vec3 Position;
in vec2 UV;
in vec3 Normal;

out vec4 FragColor;

void main()
{
    // FragColor = texture(textureSampler, vertexTexCoord);
    FragColor = vec4((Normal + vec3(1.f, 1.f, 1.f)) * .5f, 1.f);
}
)";

const char* g_meshWireframeVertSource = R"(
#version 330 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

uniform mat4 VP;

void main()
{
    gl_Position = vec4(vertex.xyz, 1.0) * VP;
}
)";

const char* g_meshWireframeFragSource = R"(
#version 330 core

uniform vec4 color;

out vec4 FragColor;

void main()
{
    // FragColor = texture(textureSampler, vertexTexCoord);
    FragColor = color;
}
)";

static ShaderSourceWrapperImpl meshShaderVertexSource = ShaderSourceWrapperImpl(g_meshVertSource);
static ShaderSourceWrapperImpl meshShaderFragmentSource = ShaderSourceWrapperImpl(g_meshFragSource);

Shader Mesh::Shaders::matt = Shader(meshShaderVertexSource, meshShaderFragmentSource);

static ShaderSourceWrapperImpl meshWireframeShaderVertexSource = ShaderSourceWrapperImpl(g_meshWireframeVertSource);
static ShaderSourceWrapperImpl meshWireframeShaderFragmentSource = ShaderSourceWrapperImpl(g_meshWireframeFragSource);

Shader Mesh::Shaders::wireframe = Shader(meshWireframeShaderVertexSource, meshWireframeShaderFragmentSource);

#define MESH_CACHE_VERSION 3
class Mesh::CacheFileHeader {
public:
    using file_time = std::filesystem::file_time_type;
    
    const unsigned int version = MESH_CACHE_VERSION;
    file_time   lastSaveTime;
    BoundingBox boundingBox;
};

class Mesh::Impl {
public:
    void init(const std::vector<Face>& faces) {
        if (s_vao == 0)
            glGenVertexArrays(1, &s_vao);
        glBindVertexArray(s_vao);

        if (m_vbo == 0)
            glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        glBufferData(GL_ARRAY_BUFFER, faces.size() * sizeof(Face), faces.data(), GL_STATIC_DRAW);

        m_changed = false;
    }

    void draw(const std::vector<Face>& faces, Shader& shader) {
        if (m_changed)
            init(faces);

        shader.use();

        glBindVertexArray(s_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        // vertex
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Face) / 3, (void*)0);
        glEnableVertexAttribArray(0);
        // uv
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Face) / 3, (void*)sizeof(vec3));
        glEnableVertexAttribArray(1);
        // normal
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Face) / 3, (void*)(sizeof(vec3) + sizeof(vec2)));
        glEnableVertexAttribArray(2);

        glDrawArrays(GL_TRIANGLES, 0, 3 * faces.size());
    }

    void update() {
        m_changed = true;
    }

private:
    inline static GLuint s_vao = 0u;

    GLuint  m_vbo = 0;
    bool	m_changed = true;

};

std::vector<char> readBinaryFile(std::ifstream& ifs) {
    std::vector<char> buffer(std::istreambuf_iterator<char>(ifs), {});
    return buffer;
}

std::string getCachePath(const std::string& path) {
    size_t fileNamePos = path.find_last_of('/') + 1;

    std::string directory = path.substr(0, fileNamePos);
    std::string filename = path.substr(fileNamePos);

    std::filesystem::create_directories(directory + ".cache/");
    return directory + ".cache/" + filename + ".bin";
}

bool Mesh::tryLoadCache(const std::string& path) {
    std::string cachePath = getCachePath(path);

    std::ifstream ifs(cachePath, std::ios::binary);
    if (!ifs.is_open())
        return false;

    using file_time = std::filesystem::file_time_type;

    file_time fileLastModified = std::filesystem::last_write_time(path);

    // Read header
    CacheFileHeader header;
    ifs.read((char*)&header, sizeof(CacheFileHeader));

    // If cache version doesn't match current reload the mesh
    if (MESH_CACHE_VERSION != header.version)
        return false;

    // If file changed reload mesh
    if (fileLastModified != header.lastSaveTime)
        return false;

    m_boundingBox = header.boundingBox;

    // Read binary file to buffer
    ifs.seekg(0, std::ios::beg);
    std::vector<char> buffer = readBinaryFile(ifs);
    size_t faceCount = (buffer.size() - sizeof(CacheFileHeader)) / sizeof(Face);

    ifs.close();

    // Read faces
    m_faces.resize(faceCount);
    memcpy_s(m_faces.data(), faceCount * sizeof(Face), &buffer.data()[sizeof(CacheFileHeader)], faceCount * sizeof(Face));

    m_status = Status::OK;

    debug::cout << "Loaded mesh from cache: " << cachePath << std::endl;
    return true;
}

void Mesh::saveCache(const std::string& path) const {
    std::string cachePath = getCachePath(path);

    std::ofstream ofs(cachePath, std::ios::binary);
    if (!ofs.is_open())
        return;

    using file_time = std::filesystem::file_time_type;

    CacheFileHeader header;
    header.lastSaveTime = std::filesystem::last_write_time(path);
    header.boundingBox = m_boundingBox;

    ofs.write((char*)&header, sizeof(CacheFileHeader));
    ofs.write((char*)m_faces.data(), m_faces.size() * sizeof(Face));

    ofs.close();

    debug::cout << "Saved mesh cache: " << cachePath << std::endl;
}

void graphics::Mesh::BoundingBox::getVertices(vec3 vertices[8]) const {
    vertices[0] = min;
    vertices[1] = vec3(min.x, min.y, max.z);
    vertices[2] = vec3(max.x, min.y, min.z);
    vertices[3] = vec3(max.x, min.y, max.z);
    vertices[4] = vec3(min.x, max.y, min.z);
    vertices[5] = vec3(min.x, max.y, max.z);
    vertices[6] = vec3(max.x, max.y, min.z);
    vertices[7] = max;
}

void graphics::Mesh::BoundingBox::update(const vec3& vertex) {
    if (vertex.x < min.x)
        min.x = vertex.x;
    if (vertex.y < min.y)
        min.y = vertex.y;
    if (vertex.z < min.z)
        min.z = vertex.z;

    if (vertex.x > max.x)
        max.x = vertex.x;
    if (vertex.y > max.y)
        max.y = vertex.y;
    if (vertex.z > max.z)
        max.z = vertex.z;
}

std::shared_ptr<Mesh> Mesh::loadFromObjFile(const std::string& path) {
    auto mesh = std::make_shared<Mesh>();

    // Try to load from binary cache
    if (mesh->tryLoadCache(path))
        return mesh;

    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        mesh->m_status = Status::FILE_NOT_FOUND;
        return mesh;
    }

    size_t lineCount = std::count(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>(), '\n');
    ifs.seekg(0, std::ios::beg);
    bool isLargeFile = (lineCount > 500000);
    if (isLargeFile) {
        debug::cout << "Loading large mesh: [";
        for (int i = 0; i < 10; ++i)
            debug::cout << (char)177u;
        debug::cout << "]\b\b\b\b\b\b\b\b\b\b\b";
    }

    std::vector<vec3> tmpVertices{};
    std::vector<vec2> tmpUVs{};
    std::vector<vec3> tmpNormals{};
    std::vector<FaceIndices> tmpIndices{};

    size_t i = 0;
    int done = 0;
    std::string line;
    while (std::getline(ifs, line)) {
        std::istringstream iss(line);

        std::string word;
        iss >> word;

        if (word.empty()) {
            continue;
        }

        if (std::strcmp(word.c_str(), "v") == 0) {
            vec3 vertex;
            iss >> vertex.x; iss >> vertex.y; iss >> vertex.z;
            mesh->m_boundingBox.update(vertex);
            tmpVertices.push_back(vertex);
        }
        else if (std::strcmp(word.c_str(), "vt") == 0) {
            vec2 uv;
            iss >> uv.x; iss >> uv.y;
            tmpUVs.push_back(uv);
        }
        else if (std::strcmp(word.c_str(), "vn") == 0) {
            vec3 normal;
            iss >> normal.x; iss >> normal.y; iss >> normal.z;
            tmpNormals.push_back(normal);
        }
        else if (std::strcmp(word.c_str(), "f") == 0) {
            std::string vertex1, vertex2, vertex3;
            int vertexIndex[3]{}, uvIndex[3]{}, normalIndex[3]{};
            char c;
            iss >> vertexIndex[0]; iss >> c; iss >> uvIndex[0]; iss >> c; iss >> normalIndex[0]; 
            iss >> vertexIndex[1]; iss >> c; iss >> uvIndex[1]; iss >> c; iss >> normalIndex[1]; 
            iss >> vertexIndex[2]; iss >> c; iss >> uvIndex[2]; iss >> c; iss >> normalIndex[2]; 

            FaceIndices face{
                { vertexIndex[0] - 1, uvIndex[0] - 1, normalIndex[0] - 1 },
                { vertexIndex[1] - 1, uvIndex[1] - 1, normalIndex[1] - 1 },
                { vertexIndex[2] - 1, uvIndex[2] - 1, normalIndex[2] - 1 },
            };
            tmpIndices.push_back(face);
        }
        else {
            continue;
        }

        if (iss.fail()) {
            mesh->m_status = Status::FAILED;
            if (isLargeFile)
                debug::cout << std::endl;
            return mesh;
        }

        ++i;
        if (isLargeFile && i % (lineCount / 10) == 0) {
            ++done;
            debug::cout << (char)219u;
        }
    }

    mesh->constructFaces(tmpVertices, tmpUVs, tmpNormals, tmpIndices);

    if (isLargeFile)
        debug::cout << ((done < 10) ? (char)219u : ' ') << std::endl;

    debug::cout << "Loaded mesh from: " << path << std::endl;
    mesh->saveCache(path);
        
    mesh->m_status = Status::OK;
    return mesh;
}

graphics::Mesh::Face graphics::Mesh::getFace(
    const std::vector<vec3>& vertices, const std::vector<vec2>& uvs,
    const std::vector<vec3>& normals, const std::vector<FaceIndices>& indices, size_t index) const
{
    const FaceIndices& face = indices.at(index);
    return Face {
        vertices.at(face.v1.x), uvs.at(face.v1.y), normals.at(face.v1.z),
        vertices.at(face.v2.x), uvs.at(face.v2.y), normals.at(face.v2.z),
        vertices.at(face.v3.x), uvs.at(face.v3.y), normals.at(face.v3.z),
    };
}

void graphics::Mesh::constructFaces(
    const std::vector<vec3>& vertices, const std::vector<vec2>& uvs, 
    const std::vector<vec3>& normals, const std::vector<FaceIndices>& indices)
{
    m_faces.clear();
    m_faces.resize(indices.size());

    for (int i = 0; i < indices.size(); ++i)
        m_faces.push_back(getFace(vertices, uvs, normals, indices, i));
    
    impl->update();
}

void Mesh::draw(Shader& shader) const {
    impl->draw(m_faces, shader);
}

void Mesh::drawWireframe(const Color& color, Shader& shader) const {
    shader.setUniform("color", (vec4)color);

    for (auto& face : m_faces) {
        debug::drawLine(face.vertex1, face.vertex2, color, shader);
        debug::drawLine(face.vertex2, face.vertex3, color, shader);
        debug::drawLine(face.vertex3, face.vertex1, color, shader);
    }
}

Mesh::BoundingBox graphics::Mesh::getBoundingBox() const {
    return m_boundingBox;
}

Ray::Hit graphics::Mesh::intersectRay(const Ray& ray) const {
    Ray::Hit nearest = Ray::Hit::noHit();
    for (auto& face : m_faces) {
        Ray::Hit hit = ray.intersectTrig(face.vertex1, face.vertex2, face.vertex3);
        if (hit.t < nearest.t)
            nearest = hit;
    }
    return nearest;
}

graphics::Mesh::Mesh()
    : impl(std::make_shared<Impl>())
{ }

graphics::Mesh::~Mesh() {

}
