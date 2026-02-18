#include "Engine/Services/FontService.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/ILoggerService.h"
#include "Engine/Services/PathRegistryService.h"
#include "Engine/Services/FontPathRepository.h"
#include "Engine/Services/RenderService.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <GL/glew.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <vendor/stb_truetype.h>

FontService::FontService() {}

FontService::~FontService() {
    Clean();
}

void FontService::Clean() {
    for (auto& [name, font] : m_fonts) {
        if (font.textureID != 0) {
            glDeleteTextures(1, &font.textureID);
        }
    }
    m_fonts.clear();
}

void FontService::OnInitialize() {
    // Load Default Font
    LoadFont("Kenney Pixel", 32.0f);
}

bool FontService::LoadFont(const std::string& fontName, float fontSize) {
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    
    // Check if already loaded
    if (m_fonts.find(fontName) != m_fonts.end()) {
        return true;
    }

    auto pathRegistry = ServiceLocator::Get().GetService<PathRegistryService>();
    if (!pathRegistry) return false;

    auto repo = pathRegistry->GetRepository<FontPathRepository>();
    if (!repo) return false;

    std::string path = repo->GetPath(fontName);
    if (path.empty()) {
        if (log) log->LogError("Font not found in repository: " + fontName);
        return false;
    }

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
         if (log) log->LogError("Failed to open font file: " + path);
         return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> ttf_buffer(size);
    if (!file.read((char*)ttf_buffer.data(), size)) {
         if (log) log->LogError("Failed to read font file");
         return false;
    }

    FontData newFont;
    newFont.fontSize = fontSize;
    
    // Allocate temp bitmap
    std::vector<unsigned char> temp_bitmap(newFont.bitmapW * newFont.bitmapH);

    // Bake font
    // returns -1 on failure
    int result = stbtt_BakeFontBitmap(ttf_buffer.data(), 0, fontSize, temp_bitmap.data(), newFont.bitmapW, newFont.bitmapH, 32, 96, newFont.cdata);
    
    if (result <= 0) {
        if (log) log->LogError("Failed to bake font bitmap for " + fontName);
        return false;
    }

    // Upload to GPU
    glGenTextures(1, &newFont.textureID);
    glBindTexture(GL_TEXTURE_2D, newFont.textureID);
    
    // Single channel alpha
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, newFont.bitmapW, newFont.bitmapH, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap.data());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

    m_fonts[fontName] = newFont;
    
    if (m_defaultFont.empty()) {
        m_defaultFont = fontName;
    }
    
    if (log) log->Log("Font Loaded: " + fontName);
    return true;
}

void FontService::RenderText(RenderService* renderer, const std::string& text, float x, float y, float scale, const glm::vec4& color, const std::string& fontName) {
    std::string nameIdx = fontName;
    if (nameIdx.empty()) nameIdx = m_defaultFont;
    
    if (m_fonts.find(nameIdx) == m_fonts.end()) {
        // Try to load on fly or fallback
        if (!LoadFont(nameIdx, 32.0f)) {
            nameIdx = m_defaultFont;
        }
    }
    
    if (m_fonts.find(nameIdx) == m_fonts.end()) return; // No font available
    
    const FontData& font = m_fonts[nameIdx];

    // 1. Shadow Pass (+2, +2 offset, Black) - REMOVED per user request
    // RenderTextInternal(renderer, font, text, x + 2.0f, y + 2.0f, scale, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    
    // 2. Main Pass
    RenderTextInternal(renderer, font, text, x, y, scale, color);
}

void FontService::RenderTextInternal(RenderService* renderer, const FontData& font, const std::string& text, float x, float y, float scale, const glm::vec4& color) {
    float startX = x;

    for (unsigned char c : text) {
        if (c < 32 || c >= 128) continue;
        
        float x_cursor = 0.0f;
        float y_cursor = 0.0f;
        
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(font.cdata, font.bitmapW, font.bitmapH, c - 32, &x_cursor, &y_cursor, &q, 1);
        
        // Check local cursor advance
        float advanceX = x_cursor;
        
        // Scale Quad
        float w = (q.x1 - q.x0) * scale;
        float h = (q.y1 - q.y0) * scale;
        float x_off = q.x0 * scale; // Offset from cursor
        float y_off = q.y0 * scale; 
        
        Sprite s;
        // Position is Center for DrawSprite
        s.Position = glm::vec2(startX + x_off + w*0.5f, y + y_off + h*0.5f);
        s.Size = glm::vec2(w, h);
        s.MinUV = glm::vec2(q.s0, q.t0);
        s.MaxUV = glm::vec2(q.s1, q.t1);
        s.Color = color;
        s.TextureID = font.textureID;
        s.Rotation = 0.0f;
        
        renderer->DrawSprite(s);
        
        // Advance external cursor
        startX += advanceX * scale;
    }
}

float FontService::GetTextWidth(const std::string& text, float scale, const std::string& fontName) {
    std::string nameIdx = fontName;
    if (nameIdx.empty()) nameIdx = m_defaultFont;
    
    if (m_fonts.find(nameIdx) == m_fonts.end()) {
         if (!LoadFont(nameIdx, 32.0f)) nameIdx = m_defaultFont;
    }
    if (m_fonts.find(nameIdx) == m_fonts.end()) return 0.0f;

    const FontData& font = m_fonts[nameIdx];

    float x = 0;
    for (unsigned char c : text) {
        if (c < 32 || c >= 128) continue;
        float x_cursor = 0, y_cursor = 0;
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(font.cdata, font.bitmapW, font.bitmapH, c - 32, &x_cursor, &y_cursor, &q, 1);
        
        x += x_cursor * scale;
    }
    return x;
}
