#pragma once
#include "Services/IShaderService.h"
#include "Services/ServiceLocator.h"
#include "Services/ILoggerService.h"
#include <GL/glew.h>
#include <string>

class OpenGLShaderService : public IShaderService
{
public:
    void Init() override;
    void Clean() override;

    unsigned int LoadShader(const std::string &vertexPath, const std::string &fragmentPath) override;
    void UseShader(unsigned int shaderID) override; 
    void SetMat4(unsigned int shaderID, const std::string &name, const glm::mat4 &mat) override;

private:
    // Helpers
    std::string ReadFile(const std::string &path);
    unsigned int CompileShader(unsigned int type, const std::string &source);
    auto logger() { return ServiceLocator::Get().GetService<ILoggerService>(); }
};