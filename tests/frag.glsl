#version 330 core

layout(location = 0) out vec4 out_color;

uniform vec4 color;

void main()
{
    out_color = color;
}
