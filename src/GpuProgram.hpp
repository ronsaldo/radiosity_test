#ifndef RADIOSITY_TEST_GPU_PROGRAM_HPP
#define RADIOSITY_TEST_GPU_PROGRAM_HPP

#include "Object.hpp"
#include "GLCommon.hpp"
#include <string>
#include <vector>

namespace RadiosityTest
{
DECLARE_CLASS(ShaderStage);
DECLARE_CLASS(GpuProgram);

/**
 * A shader stage
 */
class ShaderStage : public Object
{
public:
    ShaderStage(GLenum type);
    ~ShaderStage();

    void setFileName(const std::string &newFileName)
    {
        fileName = newFileName;
    }

    const std::string &getFileName() const
    {
        return fileName;
    }

    void setSource(const std::string &sourceCode);
    void setSourceFromFile(const std::string &fileName);
    bool compile();

    GLuint getHandle() const
    {
        return handle;
    }

private:
    void printInfoLog();

    GLuint handle;
    std::string fileName;
};

/**
 * A program that runs on the GPU.
 */
class GpuProgram : public Object
{
public:
    GpuProgram();
    ~GpuProgram();

    GpuProgram &attachVertexFromFile(const std::string &fileName);
    GpuProgram &attachFragmentFromFile(const std::string &fileName);
    GpuProgram &attachFromFile(GLenum type, const std::string &fileName);
    GpuProgram &attachShader(const ShaderStagePtr &shader);

    GLuint getHandle() const
    {
        return handle;
    }

    bool link();
    bool isValid() const
    {
        return valid;
    }

    void activate() const;

private:
    void printInfoLog();

    GLuint handle;
    std::vector<ShaderStagePtr> stages;
    bool valid;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_GPU_PROGRAM_HPP
