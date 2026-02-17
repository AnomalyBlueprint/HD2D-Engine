#pragma once
#include "Engine/Services/IFontService.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <vendor/stb_truetype.h> // Need definition for bakedchar

class FontService : public IFontService
{
public:
    FontService();
    ~FontService();

    void Clean() override;
    
    bool LoadFont(const std::string& fontName, float fontSize) override;
    void RenderText(RenderService* renderer, const std::string& text, float x, float y, float scale, const glm::vec4& color, const std::string& fontName = "") override;
    float GetTextWidth(const std::string& text, float scale, const std::string& fontName = "") override;

protected:
    void OnInitialize() override;

private:
    struct FontData {
        unsigned int textureID = 0;
        stbtt_bakedchar cdata[96]; // ASCII 32..126
        float fontSize = 32.0f;
        int bitmapW = 512;
        int bitmapH = 512;
    };

    void RenderTextInternal(RenderService* renderer, const FontData& font, const std::string& text, float x, float y, float scale, const glm::vec4& color);
    
    std::unordered_map<std::string, FontData> m_fonts;
    std::string m_defaultFont;
};
