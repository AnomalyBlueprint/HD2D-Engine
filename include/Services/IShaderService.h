#pragma once
#include "Services/IService.h"
#include <string>
#include <glm/glm.hpp>

class IShaderService : public IService {
public:
    virtual ~IShaderService() = default;

    // Load a Vertex/Fragment pair and return the GPU ID
    virtual unsigned int LoadShader(const std::string& vertexPath, const std::string& fragmentPath) = 0;
    
    // Tell the GPU to "Use" a specific shader
    virtual void UseShader(unsigned int shaderID) = 0;
    virtual void SetMat4(unsigned int shaderID, const std::string &name, const glm::mat4 &mat) = 0;
};