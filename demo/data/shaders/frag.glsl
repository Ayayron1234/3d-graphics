#version 330 core

uniform sampler2D tex2D;

in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

void main()
{
    FragColor = vec4((Normal + vec3(1.f, 1.f, 1.f)) * .5f, 1.f);
}
