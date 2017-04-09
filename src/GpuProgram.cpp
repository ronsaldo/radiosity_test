#include "GpuProgram.hpp"
#include "File.hpp"
#include <stdlib.h>

namespace RadiosityTest
{

ShaderStage::ShaderStage(GLenum type)
{
    handle = glCreateShader(type);
}

ShaderStage::~ShaderStage()
{
    glDeleteShader(handle);
}

void ShaderStage::setSource(const std::string &sourceCode)
{
    auto stringPtr = sourceCode.c_str();
    glShaderSource(handle, 1, &stringPtr, nullptr);
}

void ShaderStage::setSourceFromFile(const std::string &fileName)
{
    setFileName(fileName);
    setSource(readWholeTextFile(fileName));
}

bool ShaderStage::compile()
{
    glCompileShader(handle);

    GLint status;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE)
    {
        printInfoLog();
    }

    return status == GL_TRUE;
}

void ShaderStage::printInfoLog()
{
    GLint infoLogLength;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);

    std::vector<char> infoLog(infoLogLength + 1);
    GLsizei returnedLength;
    glGetShaderInfoLog(handle, infoLogLength + 1, &returnedLength, &infoLog[0]);
    infoLog[returnedLength] = 0;

    fprintf(stderr, "Shader compilation error:\n%s\n", &infoLog[0]);
}

GpuProgram::GpuProgram()
{
    handle = glCreateProgram();
}

GpuProgram::~GpuProgram()
{
    glDeleteProgram(handle);
}

GpuProgram &GpuProgram::attachVertexFromFile(const std::string &fileName)
{
    return attachFromFile(GL_VERTEX_SHADER, fileName);
}

GpuProgram &GpuProgram::attachFragmentFromFile(const std::string &fileName)
{
    return attachFromFile(GL_FRAGMENT_SHADER, fileName);
}

GpuProgram &GpuProgram::attachFromFile(GLenum type, const std::string &fileName)
{
    auto shader = std::make_shared<ShaderStage> (type);
    shader->setSourceFromFile(fileName);
    if(!shader->compile())
    {
        abort();
    }

    return attachShader(shader);
}

GpuProgram &GpuProgram::attachShader(const ShaderStagePtr &shader)
{
    glAttachShader(handle, shader->getHandle());
    stages.push_back(shader);
    return *this;
}

bool GpuProgram::link()
{
    glLinkProgram(handle);

    GLint status;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    if(status != GL_TRUE)
    {
        printInfoLog();
    }

    return (valid = (status == GL_TRUE));
}

void GpuProgram::printInfoLog()
{
    GLint infoLogLength;
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);

    std::vector<char> infoLog(infoLogLength + 1);
    GLsizei returnedLength;
    glGetProgramInfoLog(handle, infoLogLength + 1, &returnedLength, &infoLog[0]);
    infoLog[returnedLength] = 0;

    fprintf(stderr, "Program linking error:\n%s\n", &infoLog[0]);
}

void GpuProgram::activate() const
{
    glUseProgram(handle);
}

} // End of namespace RadiosityTest
