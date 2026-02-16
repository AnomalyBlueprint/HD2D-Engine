#include "Services/OpenGLShaderService.h"
#include "Services/ServiceLocator.h"
#include "Services/ILoggerService.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

void OpenGLShaderService::Init()
{
    logger()->Log("Shader Service Initialized.");
}

void OpenGLShaderService::Clean()
{
    // In a real engine, we would store a list of IDs and glDeleteProgram them here
}

void OpenGLShaderService::UseShader(unsigned int shaderID)
{
    glUseProgram(shaderID);
}

// 1. READ FILE -> COMPILE -> LINK
unsigned int OpenGLShaderService::LoadShader(const std::string &vertexPath, const std::string &fragmentPath)
{
    // A. Read Source Code
    std::string vCode = ReadFile(vertexPath);
    std::string fCode = ReadFile(fragmentPath);

    if (vCode.empty() || fCode.empty())
        return 0;

    // B. Compile Shaders
    unsigned int vertex = CompileShader(GL_VERTEX_SHADER, vCode);
    unsigned int fragment = CompileShader(GL_FRAGMENT_SHADER, fCode);

    if (vertex == 0 || fragment == 0)
        return 0;

    // C. Link Program
    unsigned int programID = glCreateProgram();
    glAttachShader(programID, vertex);
    glAttachShader(programID, fragment);
    glLinkProgram(programID);

    // D. Check Linking Errors
    int success;
    char infoLog[512];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        logger()->LogError("Shader Linking Failed: " + std::string(infoLog));
        return 0;
    }

    // Cleanup (Shaders are linked now, we don't need the individual objects)
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    logger()->Log("Shader Loaded Successfully: " + vertexPath);
    return programID;
}

// Helper: Read text file into string
std::string OpenGLShaderService::ReadFile(const std::string &path)
{
    std::ifstream file;
    std::stringstream buffer;

    file.open(path);
    if (!file.is_open())
    {
        logger()->LogError("Failed to open shader file: " + path);
        return "";
    }

    buffer << file.rdbuf();
    return buffer.str();
}

// Helper: Compile raw string
unsigned int OpenGLShaderService::CompileShader(unsigned int type, const std::string &source)
{
    unsigned int id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);

    // Check Errors
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        std::string typeStr = (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
        logger()->LogError(typeStr + " Shader Error: " + std::string(infoLog));
        return 0;
    }

    return id;
}
void OpenGLShaderService::SetMat4(unsigned int shaderID, const std::string &name, const glm::mat4 &mat)
{
    UseShader(shaderID); // Ensure shader is active before setting data

    // Find where the variable "name" lives in the shader memory
    int loc = glGetUniformLocation(shaderID, name.c_str());

    // Send the data (1 matrix, false = don't transpose, pointer to data)
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

void OpenGLShaderService::SetVec3(unsigned int shaderID, const std::string &name, const glm::vec3 &value)
{
    UseShader(shaderID);
    glUniform3fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}

void OpenGLShaderService::SetVec4(unsigned int shaderID, const std::string &name, const glm::vec4 &value)
{
    UseShader(shaderID);
    glUniform4fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}

void OpenGLShaderService::SetInt(unsigned int shaderID, const std::string &name, int value)
{
    UseShader(shaderID);
    glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
}

void OpenGLShaderService::SetFloat(unsigned int shaderID, const std::string &name, float value)
{
    UseShader(shaderID);
    glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
}

void OpenGLShaderService::SetBool(unsigned int shaderID, const std::string &name, bool value)
{
    UseShader(shaderID);
    glUniform1i(glGetUniformLocation(shaderID, name.c_str()), (int)value);
}