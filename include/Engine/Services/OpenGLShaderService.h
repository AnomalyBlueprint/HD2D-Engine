#pragma once
#include "Engine/Services/IShaderService.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/ILoggerService.h"
#include <GL/glew.h>
#include <string>

/// <summary>
/// OpenGL implementation of the Shader Service.
/// Handles standard text-based shader compilation.
/// </summary>
class OpenGLShaderService : public IShaderService
{
public:
    void Init() override;
    void Clean() override;

    unsigned int LoadShader(const std::string &vertexPath, const std::string &fragmentPath) override;
    void UseShader(unsigned int shaderID) override; 
    void SetMat4(unsigned int shaderID, const std::string &name, const glm::mat4 &mat) override;
    void SetVec3(unsigned int shaderID, const std::string &name, const glm::vec3 &value) override;
    void SetVec4(unsigned int shaderID, const std::string &name, const glm::vec4 &value) override;
    void SetInt(unsigned int shaderID, const std::string &name, int value) override;
    void SetFloat(unsigned int shaderID, const std::string &name, float value) override;
    void SetBool(unsigned int shaderID, const std::string &name, bool value) override;

private:
    std::string ReadFile(const std::string &path);
    unsigned int CompileShader(unsigned int type, const std::string &source);
    auto logger() { return ServiceLocator::Get().GetService<ILoggerService>(); }
};