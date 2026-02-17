#pragma once
#include "Engine/Services/IService.h"
#include <string>
#include <glm/glm.hpp>

class RenderService;

class IFontService : public IService
{
public:
    virtual ~IFontService() = default;

    virtual void LoadFont(const std::string& fontName, float fontSize) = 0;
    virtual void RenderText(RenderService* renderer, const std::string& text, float x, float y, float scale, const glm::vec4& color) = 0;
    virtual float GetTextWidth(const std::string& text, float scale) = 0;
};
