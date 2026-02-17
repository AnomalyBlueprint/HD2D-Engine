#pragma once
#include "Engine/Services/IFontService.h"
#include <vector>
#include <vendor/stb_truetype.h> // Need definition for bakedchar

class FontService : public IFontService
{
public:
    FontService();
    ~FontService();

    void Clean() override;
    
    void LoadFont(const std::string& fontName, float fontSize) override;
    void RenderText(RenderService* renderer, const std::string& text, float x, float y, float scale, const glm::vec4& color) override;
    float GetTextWidth(const std::string& text, float scale) override;

protected:
    void OnInitialize() override;

private:
    void RenderTextInternal(RenderService* renderer, const std::string& text, float x, float y, float scale, const glm::vec4& color);
    
    unsigned int m_fontTextureID = 0;
    stbtt_bakedchar m_cdata[96]; // ASCII 32..126 is 95 glyphs
    int m_bitmapW = 512;
    int m_bitmapH = 512;
    float m_fontSize = 32.0f;
    bool m_fontLoaded = false;
};
