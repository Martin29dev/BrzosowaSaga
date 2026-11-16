// text.h — czcionka, układ tekstu z kolorami ^n, zawijanie
// Brzozowa Saga (RPG Maker MV-like, raylib + C++17)
#pragma once
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "common.h"

namespace rpg {

// ================================================================= TEKST ===
struct Glyph { int cp; int col; float x, y; };

Color textColor(int i, Color base);

class TextSys {
public:
    Font font{};
    bool unicode = false, custom = false;

    void init();
    void shutdown() { if (custom) UnloadFont(font); }

    // gdy brak czcionki z polskimi znakami -> transliteracja
    int mapCp(int cp) const;
    float advance(int cp, float size) const;
    float wordWidth(const char* p, const char* end, float size) const;
    // układ tekstu z kodami kolorów ^n i zawijaniem słów
    std::vector<Glyph> layout(const std::string& s, float size, float maxW, float lineH,
                              float* outW = nullptr, float* outH = nullptr) const;
    void drawGlyphs(const std::vector<Glyph>& g, size_t count, float ox, float oy, float size,
                    Color base, float alpha = 1.0f, bool outline = true) const;
    void draw(const std::string& s, float x, float y, float size, Color c, float alpha = 1.f, bool outline = true) const;
    float drawWrapped(const std::string& s, float x, float y, float size, float maxW, Color c, float alpha = 1.f) const;
    float measure(const std::string& s, float size) const { float w = 0; layout(s, size, 0, size, &w); return w; }
};
extern TextSys g_text;

}  // namespace rpg
