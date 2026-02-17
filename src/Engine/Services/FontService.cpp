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
    if (m_fontTextureID != 0) {
        glDeleteTextures(1, &m_fontTextureID);
        m_fontTextureID = 0;
    }
}

void FontService::OnInitialize() {
    // Load Default Font
    LoadFont("Kenney Pixel", 32.0f);
}

void FontService::LoadFont(const std::string& fontName, float fontSize) {
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    auto pathRegistry = ServiceLocator::Get().GetService<PathRegistryService>();
    if (!pathRegistry) return;

    auto repo = pathRegistry->GetRepository<FontPathRepository>();
    if (!repo) return;

    std::string path = repo->GetPath(fontName);
    if (path.empty()) {
        if (log) log->LogError("Font not found: " + fontName);
        return;
    }

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
         if (log) log->LogError("Failed to open font file: " + path);
         return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> ttf_buffer(size);
    if (!file.read((char*)ttf_buffer.data(), size)) {
         if (log) log->LogError("Failed to read font file");
         return;
    }

    // Allocate temp bitmap
    std::vector<unsigned char> temp_bitmap(m_bitmapW * m_bitmapH);

    // Bake font
    // returns -1 on failure
    int result = stbtt_BakeFontBitmap(ttf_buffer.data(), 0, fontSize, temp_bitmap.data(), m_bitmapW, m_bitmapH, 32, 96, m_cdata);
    
    if (result <= 0) {
        if (log) log->LogError("Failed to bake font bitmap! Try increasing bitmap size.");
        // Could retry with larger bitmap...
    } else {
         if (log) log->Log("Font Loaded: " + fontName);
    }

    // Upload to GPU
    if (m_fontTextureID == 0) glGenTextures(1, &m_fontTextureID);
    glBindTexture(GL_TEXTURE_2D, m_fontTextureID);
    
    // Single channel alpha
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_bitmapW, m_bitmapH, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap.data());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // We might need swizzle to use RED as ALPHA in shader if shader expects RGBA
    // But RenderService uses basic shader which samples texture(sampler, uv).
    // If texture is single channel (Red), then (r, 0, 0, 1) is sampled.
    // If we want transparency, we need (1, 1, 1, r)
    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

    m_fontSize = fontSize;
    m_fontLoaded = true;
}

void FontService::RenderText(RenderService* renderer, const std::string& text, float x, float y, float scale, const glm::vec4& color) {
    if (!m_fontLoaded) return;
    
    // Y-Offset to align baseline?
    // stb_truetype baked quad assumes y=0 is top? or baseline?
    // It seems to be baseline relative for pixel fonts usually.
    // If we want top-left alignment passed in (x,y), we might need to adjust.
    // However, let's stick to the logic: x,y is the baseline start position.
    
    for (unsigned char c : text) {
        if (c < 32 || c >= 128) continue;
        
        float x_cursor = 0.0f;
        float y_cursor = 0.0f;
        
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(m_cdata, m_bitmapW, m_bitmapH, c - 32, &x_cursor, &y_cursor, &q, 1);
        
        // Check local cursor advance
        float advanceX = x_cursor;
        
        // Scale Quad
        float w = (q.x1 - q.x0) * scale;
        float h = (q.y1 - q.y0) * scale;
        float x_off = q.x0 * scale; // Offset from cursor
        float y_off = q.y0 * scale; 
        
        Sprite s;
        // Position is Center for DrawSprite
        s.Position = glm::vec2(x + x_off + w*0.5f, y + y_off + h*0.5f);
        s.Size = glm::vec2(w, h);
        s.MinUV = glm::vec2(q.s0, q.t0);
        s.MaxUV = glm::vec2(q.s1, q.t1);
        s.Color = color;
        s.TextureID = m_fontTextureID;
        s.Rotation = 0.0f;
        
        renderer->DrawSprite(s);
        
        // Advance external cursor
        x += advanceX * scale;
    }
}

float FontService::GetTextWidth(const std::string& text, float scale) {
    float x = 0;
    for (unsigned char c : text) {
        if (c < 32 || c >= 128) continue;
        float x_cursor = 0, y_cursor = 0;
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(m_cdata, m_bitmapW, m_bitmapH, c - 32, &x_cursor, &y_cursor, &q, 1);
        
        x += x_cursor * scale;
    }
    return x;
}
