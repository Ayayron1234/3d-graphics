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