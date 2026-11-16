// text.cpp — implementacja systemu tekstu
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "text.h"

namespace rpg {

TextSys g_text;

Color textColor(int i, Color base) {
    static const Color T[10] = {
        {255, 255, 255, 255}, {96, 184, 255, 255}, {255, 120, 90, 255}, {120, 220, 90, 255},
        {160, 210, 255, 255}, {210, 180, 255, 255}, {255, 226, 110, 255}, {160, 160, 172, 255},
        {255, 170, 60, 255}, {255, 140, 200, 255}};
    if (i == 0) return base;
    return T[std::max(0, std::min(9, i))];
}

void TextSys::init() {
    std::vector<int> cps;
    for (int c = 32; c < 127; ++c) cps.push_back(c);
    const char* extra = "ąćęłńóśźżĄĆĘŁŃÓŚŹŻ„”…—–•▼▶×·";
    for (const char* p = extra; *p;) {
        int sz = 0; int cp = GetCodepointNext(p, &sz);
        if (sz <= 0) break;
        cps.push_back(cp); p += sz;
    }
    const char* paths[] = {
        "font.ttf", "C:/Windows/Fonts/segoeuib.ttf", "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arialbd.ttf", "C:/Windows/Fonts/arial.ttf", "C:/Windows/Fonts/tahoma.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf"};
    Font def = GetFontDefault();
    for (const char* path : paths) {
        if (!FileExists(path)) continue;
        Font f = LoadFontEx(path, 64, cps.data(), (int)cps.size());
        if (f.texture.id != 0 && f.texture.id != def.texture.id && f.glyphCount > 0) {
            font = f; custom = unicode = true;
            GenTextureMipmaps(&font.texture);
            SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
            TraceLog(LOG_INFO, "FONT: %s", path);
            break;
        }
    }
    if (!custom) font = def;
}

int TextSys::mapCp(int cp) const {
    if (unicode) return cp;
    switch (cp) {
        case 0x105: return 'a'; case 0x107: return 'c'; case 0x119: return 'e'; case 0x142: return 'l';
        case 0x144: return 'n'; case 0xF3: return 'o'; case 0x15B: return 's'; case 0x17A: case 0x17C: return 'z';
        case 0x104: return 'A'; case 0x106: return 'C'; case 0x118: return 'E'; case 0x141: return 'L';
        case 0x143: return 'N'; case 0xD3: return 'O'; case 0x15A: return 'S'; case 0x179: case 0x17B: return 'Z';
        case 0x201E: case 0x201D: return '"'; case 0x2026: return '.'; case 0x2014: case 0x2013: return '-';
        case 0x2022: case 0xB7: return '*'; case 0x25BC: return 'v'; case 0x25B6: return '>'; case 0xD7: return 'x';
        default: return cp < 128 ? cp : '?';
    }
}

float TextSys::advance(int cp, float size) const {
    int i = GetGlyphIndex(font, cp);
    float sc = size / (float)font.baseSize;
    float a = font.glyphs[i].advanceX != 0 ? (float)font.glyphs[i].advanceX : font.recs[i].width;
    return a * sc + (custom ? 0.0f : size / 10.0f);
}

float TextSys::wordWidth(const char* p, const char* end, float size) const {
    float w = 0;
    while (p < end && *p != ' ' && *p != '\n') {
        if (*p == '^' && p + 1 < end && p[1] >= '0' && p[1] <= '9') { p += 2; continue; }
        int sz = 0; int cp = GetCodepointNext(p, &sz); if (sz <= 0) sz = 1;
        w += advance(mapCp(cp), size); p += sz;
    }
    return w;
}

std::vector<Glyph> TextSys::layout(const std::string& s, float size, float maxW, float lineH, float* outW, float* outH) const {
    std::vector<Glyph> out;
    float x = 0, y = 0, mx = 0; int col = 0; bool wordStart = true;
    const char* p = s.c_str(); const char* end = p + s.size();
    while (p < end) {
        char ch = *p;
        if (ch == '^' && p + 1 < end && p[1] >= '0' && p[1] <= '9') { col = p[1] - '0'; p += 2; continue; }
        if (ch == '\n') { x = 0; y += lineH; ++p; wordStart = true; continue; }
        if (ch == ' ') { x += advance(' ', size); ++p; wordStart = true; continue; }
        if (wordStart && maxW > 0) {
            float w = wordWidth(p, end, size);
            if (x > 0 && x + w > maxW) { x = 0; y += lineH; }
            wordStart = false;
        }
        int sz = 0; int cp = GetCodepointNext(p, &sz); if (sz <= 0) sz = 1;
        cp = mapCp(cp);
        Glyph g; g.cp = cp; g.col = col; g.x = x; g.y = y;
        out.push_back(g);
        x += advance(cp, size); mx = std::max(mx, x); p += sz;
    }
    if (outW) *outW = mx;
    if (outH) *outH = y + lineH;
    return out;
}

void TextSys::drawGlyphs(const std::vector<Glyph>& g, size_t count, float ox, float oy, float size, Color base, float alpha, bool outline) const {
    count = std::min(count, g.size());
    for (size_t i = 0; i < count; ++i) {
        Vector2 p = v2(ox + g[i].x, oy + g[i].y);
        if (outline) {
            Color oc = rgba(0, 0, 0, (int)(150 * alpha));
            const float o = size * 0.06f;
            DrawTextCodepoint(font, g[i].cp, v2(p.x - o, p.y - o), size, oc);
            DrawTextCodepoint(font, g[i].cp, v2(p.x + o, p.y - o), size, oc);
            DrawTextCodepoint(font, g[i].cp, v2(p.x - o, p.y + o), size, oc);
            DrawTextCodepoint(font, g[i].cp, v2(p.x + o, p.y + o), size, oc);
        }
        DrawTextCodepoint(font, g[i].cp, p, size, fade(textColor(g[i].col, base), alpha));
    }
}

void TextSys::draw(const std::string& s, float x, float y, float size, Color c, float alpha, bool outline) const {
    std::vector<Glyph> g = layout(s, size, 0, size * 1.3f);
    drawGlyphs(g, g.size(), x, y, size, c, alpha, outline);
}

float TextSys::drawWrapped(const std::string& s, float x, float y, float size, float maxW, Color c, float alpha) const {
    float h = 0;
    std::vector<Glyph> g = layout(s, size, maxW, size * 1.35f, nullptr, &h);
    drawGlyphs(g, g.size(), x, y, size, c, alpha);
    return h;
}

}  // namespace rpg
