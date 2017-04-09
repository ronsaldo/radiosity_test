#version 450

layout(binding = 0, std140) uniform ObjectState
{
    mat4 modelMatrix;
    mat4 inverseModelMatrix;

    mat4 normalModelMatrix;
    mat4 inverseNormalModelMatrix;
};

layout(binding = 1, std140) uniform CameraState
{
    mat4 inverseViewMatrix;
    mat4 viewMatrix;

    mat4 inverseNormalViewMatrix;
    mat4 normalViewMatrix;

    mat4 projectionMatrix;
};

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec2 vTexcoord;
layout(location = 3) in vec3 vNormal;

out vec3 fPosition;
out vec4 fColor;
out vec2 fTexcoord;
out vec3 fNormal;

void main()
{
    vec4 viewPosition = viewMatrix * (modelMatrix * vec4(vPosition, 1.0));
    fNormal = (normalViewMatrix * (normalModelMatrix * vec4(vNormal, 0.0))).xyz;

    fPosition = viewPosition.xyz;
    fColor = vColor;
    fTexcoord = vTexcoord;
    gl_Position = projectionMatrix * viewPosition;
}
