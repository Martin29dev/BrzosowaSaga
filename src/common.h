// common.h — stałe, matematyka, RNG, prymitywy rysowania
// Brzozowa Saga (RPG Maker MV-like, raylib + C++17)
#pragma once
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "raylib.h"
#include "rlgl.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace rpg {

// =============================================================== STAŁE =====
constexpr int    TS = 48;              // rozmiar kafla (jak w MV)
constexpr int    VW = 816;             // wirtualna rozdzielczość MV
constexpr int    VH = 624;
constexpr int    SS = 2;               // supersampling (antyaliasing kształtów)
constexpr double STEP = 1.0 / 60.0;    // stały krok logiki
constexpr double MAX_FRAME = 0.25;     // maks. czas klatki (anty spiral-of-death)
constexpr float  WALK_SPEED = 3.75f;   // MV: speed 4 -> 2^4/256 kafla/klatkę*60
constexpr float  DASH_SPEED = 7.5f;    // MV: dash = speed + 1
constexpr float  PUSH_SPEED = 2.8f;
constexpr float  NPC_SPEED = 1.9f;     // MV: speed 3
constexpr int    MAX_SW = 64;
constexpr int    MAX_VAR = 64;
constexpr const char* SAVE_FILE = "brzozowa_save.txt";
constexpr const char* SETTINGS_FILE = "brzozowa_settings.txt";
constexpr int MENU_ITEMS = 5;    // pozycje menu w grze
constexpr int TITLE_ITEMS = 4;   // pozycje ekranu tytułowego

enum Dir { DIR_DOWN = 0, DIR_LEFT = 1, DIR_RIGHT = 2, DIR_UP = 3 };
const int DX[4] = {0, -1, 1, 0};
const int DY[4] = {1, 0, 0, -1};

// ============================================================ POMOCNICZE ===
inline float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline float signf(float v) { return v > 0.f ? 1.f : (v < 0.f ? -1.f : 0.f); }
inline Color rgba(int r, int g, int b, int a = 255) {
    Color c; c.r = (unsigned char)r; c.g = (unsigned char)g; c.b = (unsigned char)b; c.a = (unsigned char)a; return c;
}
inline Color fade(Color c, float a) { c.a = (unsigned char)clampf(c.a * a, 0.f, 255.f); return c; }
inline Color shade(Color c, float k) {
    return rgba((int)clampf(c.r * k, 0, 255), (int)clampf(c.g * k, 0, 255), (int)clampf(c.b * k, 0, 255), c.a);
}
inline Color mixc(Color a, Color b, float t) {
    return rgba((int)lerpf(a.r, b.r, t), (int)lerpf(a.g, b.g, t), (int)lerpf(a.b, b.b, t), (int)lerpf(a.a, b.a, t));
}
inline Rectangle rect(float x, float y, float w, float h) { Rectangle r; r.x = x; r.y = y; r.width = w; r.height = h; return r; }
inline Vector2 v2(float x, float y) { Vector2 v; v.x = x; v.y = y; return v; }
inline float snapPx(float v) { return std::round(v * SS) / SS; }

struct Rng {
    uint32_t s;
    explicit Rng(uint32_t seed = 0x12345678u) : s(seed ? seed : 1u) {}
    uint32_t next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
    float f() { return (float)(next() >> 8) * (1.0f / 16777216.0f); }
    int range(int a, int b) { return a + (int)(next() % (uint32_t)(b - a + 1)); }
    float rangef(float a, float b) { return a + (b - a) * f(); }
};
extern Rng g_rng;   // globalny RNG (common.cpp)

inline uint32_t hash3(int x, int y, int z) {
    uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u + (uint32_t)z * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return (h ^ (h >> 16)) | 1u;
}

// Rysowanie z kontrolą nawinięcia trójkątów (raylib wymaga CCW)
inline void tri(Vector2 a, Vector2 b, Vector2 c, Color col) {
    float cr = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (cr > 0) std::swap(b, c);
    DrawTriangle(a, b, c, col);
}
inline void ellipse(float cx, float cy, float rx, float ry, Color col) {
    const int N = 22;
    Vector2 prev = v2(cx + rx, cy);
    for (int i = 1; i <= N; ++i) {
        float a = (float)i / N * 2.f * PI;
        Vector2 p = v2(cx + std::cos(a) * rx, cy + std::sin(a) * ry);
        tri(v2(cx, cy), prev, p, col);
        prev = p;
    }
}
inline void orect(float x, float y, float w, float h, Color fill, Color ol, float t = 1.5f) {
    DrawRectangleRec(rect(x - t, y - t, w + 2 * t, h + 2 * t), ol);
    DrawRectangleRec(rect(x, y, w, h), fill);
}
inline void oround(Rectangle r, float rnd, Color fill, Color ol, float t = 1.5f) {
    DrawRectangleRounded(rect(r.x - t, r.y - t, r.width + 2 * t, r.height + 2 * t), rnd, 8, ol);
    DrawRectangleRounded(r, rnd, 8, fill);
}
inline void ocircle(float x, float y, float r, Color fill, Color ol, float t = 1.5f) {
    DrawCircleV(v2(x, y), r + t, ol);
    DrawCircleV(v2(x, y), r, fill);
}
// Radialny gradient (światło) – własna wersja przez rlgl, niezależna od wersji raylib
inline void glow(float cx, float cy, float r, Color inner) {
    const int N = 40;
    rlBegin(RL_TRIANGLES);
    for (int i = 0; i < N; ++i) {
        float a0 = (float)i / N * 2.f * PI, a1 = (float)(i + 1) / N * 2.f * PI;
        rlColor4ub(inner.r, inner.g, inner.b, inner.a); rlVertex2f(cx, cy);
        rlColor4ub(0, 0, 0, 0); rlVertex2f(cx + std::cos(a1) * r, cy + std::sin(a1) * r);
        rlColor4ub(0, 0, 0, 0); rlVertex2f(cx + std::cos(a0) * r, cy + std::sin(a0) * r);
    }
    rlEnd();
}

}  // namespace rpg
