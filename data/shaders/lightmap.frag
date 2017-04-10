#version 450

layout(binding = 1) uniform sampler2D lightmap;

in vec2 fTexcoord;
out vec4 fragmentColor;

void main()
{
    fragmentColor = texture(lightmap, fTexcoord);
}
