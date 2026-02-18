#pragma once
#include "Engine/Core/IService.h"
#include <string>
#include <glm/glm.hpp>

class RenderService;

class IFontService : public IService
{
public:
    virtual ~IFontService() = default;

    virtual bool LoadFont(const std::string& fontName, float fontSize) = 0;
    virtual void RenderText(RenderService* renderer, const std::string& text, float x, float y, float scale, const glm::vec4& color, const std::string& fontName = "") = 0;
    virtual float GetTextWidth(const std::string& text, float scale, const std::string& fontName = "") = 0;
};
