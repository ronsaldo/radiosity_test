#version 450

in vec3 fNormal;
out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(fNormal*0.5 + 0.5, 1.0);
}
