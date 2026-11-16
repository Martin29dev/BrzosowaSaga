// render_tiles.cpp — proceduralne kafle mapy i ich wypiekanie do tekstur
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "game.h"

namespace rpg {

void Game::drawGrass(float X, float Y, Rng& r) {
    Color base = shade(rgba(94, 162, 76), 0.95f + 0.08f * r.f());
    DrawRectangleRec(rect(X, Y, TS, TS), base);
    for (int i = 0; i < 3; ++i)
        DrawCircleV(v2(X + r.rangef(8, 40), Y + r.rangef(8, 40)), r.rangef(3, 7), fade(shade(base, r.f() < 0.5f ? 0.93f : 1.06f), 0.7f));
    for (int i = 0; i < 9; ++i) {
        float bx = X + r.rangef(4, 43), by = Y + r.rangef(3, 42);
        Color c = r.f() < 0.5f ? rgba(70, 130, 58) : rgba(128, 190, 96);
        DrawLineEx(v2(bx, by + 4), v2(bx - 1.5f, by), 1.4f, c);
        DrawLineEx(v2(bx + 1, by + 4), v2(bx + 2.5f, by + 0.5f), 1.4f, c);
    }
}

void Game::drawTileBase(int x, int y) {
    const char c = tile(x, y);
    const float X = (float)x * TS, Y = (float)y * TS;
    Rng r(hash3(x, y, mapId * 7 + 1));
    const Color ol = rgba(40, 30, 30);
    switch (c) {
        case '.': case ':': case 'T': case 'F': {
            drawGrass(X, Y, r);
            if (c == ':') {
                const Color pc[4] = {rgba(250, 250, 250), rgba(255, 214, 80), rgba(240, 130, 170), rgba(170, 140, 240)};
                for (int i = 0; i < 4; ++i) {
                    float fx = X + r.rangef(8, 40), fy = Y + r.rangef(8, 40); Color col = pc[r.range(0, 3)];
                    for (int k = 0; k < 4; ++k) DrawCircleV(v2(fx + DX[k] * 2.2f, fy + DY[k] * 2.2f), 2.f, col);
                    DrawCircleV(v2(fx, fy), 1.4f, rgba(255, 200, 60));
                }
            }
            if (c == 'T') {
                Rng tr(hash3(x, y, 99));
                bool birch = tr.f() < 0.45f;
                ellipse(X + 24, Y + 42, 17, 5, rgba(0, 0, 0, 60));
                Color trunk = birch ? rgba(232, 230, 220) : rgba(112, 74, 46);
                orect(X + 20, Y + 22, 8, 21, trunk, ol, 1.2f);
                if (birch) for (int i = 0; i < 4; ++i) DrawRectangleRec(rect(X + 20 + (i % 2) * 3, Y + 25 + i * 4.5f, 4, 1.6f), rgba(40, 40, 40));
            }
            if (c == 'F') {
                Color wood = rgba(150, 104, 60), dark = rgba(84, 56, 34);
                bool L = tile(x - 1, y) == 'F', R = tile(x + 1, y) == 'F', U = tile(x, y - 1) == 'F', D = tile(x, y + 1) == 'F';
                if (L) { orect(X, Y + 17, 24, 4, wood, dark, 1); orect(X, Y + 29, 24, 4, wood, dark, 1); }
                if (R) { orect(X + 24, Y + 17, 24, 4, wood, dark, 1); orect(X + 24, Y + 29, 24, 4, wood, dark, 1); }
                if (U) orect(X + 22, Y, 4, 24, wood, dark, 1);
                if (D) orect(X + 22, Y + 24, 4, 24, wood, dark, 1);
                ellipse(X + 24, Y + 39, 7, 2.5f, rgba(0, 0, 0, 60));
                orect(X + 19, Y + 11, 10, 27, rgba(128, 86, 50), dark, 1.5f);
                DrawRectangleRec(rect(X + 19, Y + 11, 10, 3), rgba(176, 126, 76));
            }
        } break;
        case '=': {
            DrawRectangleRec(rect(X, Y, TS, TS), rgba(200, 168, 116));
            for (int i = 0; i < 6; ++i) DrawCircleV(v2(X + r.rangef(5, 43), Y + r.rangef(5, 43)), r.rangef(1.2f, 2.6f), rgba(170, 140, 96));
            for (int i = 0; i < 4; ++i) DrawCircleV(v2(X + r.rangef(5, 43), Y + r.rangef(5, 43)), r.rangef(1.f, 1.8f), rgba(224, 198, 152));
            const Color g = rgba(94, 162, 76), edge = rgba(160, 132, 88);
            for (int d = 0; d < 4; ++d) {
                char n = tile(x + DX[d], y + DY[d]);
                if (n == '=') continue;
                bool grass = n == '.' || n == ':' || n == 'T' || n == 'F';
                Rectangle gr, er;
                if (d == DIR_UP) { gr = rect(X, Y, TS, 3); er = rect(X, Y + 3, TS, 2); }
                else if (d == DIR_DOWN) { gr = rect(X, Y + TS - 3, TS, 3); er = rect(X, Y + TS - 5, TS, 2); }
                else if (d == DIR_LEFT) { gr = rect(X, Y, 3, TS); er = rect(X + 3, Y, 2, TS); }
                else { gr = rect(X + TS - 3, Y, 3, TS); er = rect(X + TS - 5, Y, 2, TS); }
                DrawRectangleRec(er, edge);
                if (grass) {
                    DrawRectangleRec(gr, g);
                    for (int i = 0; i < 4; ++i) {
                        float k = r.rangef(4, 42);
                        Vector2 p = (d == DIR_UP || d == DIR_DOWN) ? v2(X + k, gr.y + (d == DIR_UP ? 3.f : -2.f)) : v2(gr.x + (d == DIR_LEFT ? 3.f : -2.f), Y + k);
                        DrawCircleV(p, 2.f, g);
                    }
                }
            }
        } break;
        case '~': {
            DrawRectangleRec(rect(X, Y, TS, TS), rgba(54, 112, 190));
            for (int i = 0; i < 3; ++i) DrawRectangleRec(rect(X + r.rangef(2, 28), Y + r.rangef(2, 40), r.rangef(10, 18), 3), rgba(46, 98, 172));
            for (int d = 0; d < 4; ++d) {
                char n = tile(x + DX[d], y + DY[d]);
                if (n == '~' || n == 'X') continue;
                Color sh = rgba(92, 156, 218);
                if (d == DIR_UP) DrawRectangleRec(rect(X, Y, TS, 7), sh);
                if (d == DIR_DOWN) DrawRectangleRec(rect(X, Y + TS - 7, TS, 7), sh);
                if (d == DIR_LEFT) DrawRectangleRec(rect(X, Y, 7, TS), sh);
                if (d == DIR_RIGHT) DrawRectangleRec(rect(X + TS - 7, Y, 7, TS), sh);
            }
        } break;
        case '#': case '*': {
            bool face = c == '*' || !wallChar(tile(x, y + 1));
            if (!face) {
                DrawRectangleRec(rect(X, Y, TS, TS), rgba(86, 84, 100));
                for (int i = 0; i < 3; ++i) DrawRectangleRec(rect(X + r.rangef(2, 34), Y + r.rangef(2, 42), r.rangef(6, 12), 2), rgba(76, 74, 90));
                if (!wallChar(tile(x, y - 1))) DrawRectangleRec(rect(X, Y, TS, 3), rgba(128, 126, 144));
                if (!wallChar(tile(x - 1, y))) DrawRectangleRec(rect(X, Y, 3, TS), rgba(112, 110, 128));
                if (!wallChar(tile(x + 1, y))) DrawRectangleRec(rect(X + TS - 3, Y, 3, TS), rgba(64, 62, 76));
            } else {
                DrawRectangleRec(rect(X, Y, TS, TS), rgba(92, 90, 104));
                for (int row = 0; row < 4; ++row) {
                    float yy = Y + row * 12.f, off = (row % 2) ? 12.f : 0.f;
                    for (float bx = -off; bx < TS; bx += 24.f) {
                        float x0 = std::max(X, X + bx), x1 = std::min(X + TS, X + bx + 24.f);
                        if (x1 - x0 < 3) continue;
                        DrawRectangleRec(rect(x0 + 1, yy + 1, x1 - x0 - 2, 10), shade(rgba(134, 130, 144), 0.86f + 0.2f * r.f()));
                    }
                }
                DrawRectangleGradientV((int)X, (int)(Y + TS - 8), TS, 8, rgba(0, 0, 0, 0), rgba(0, 0, 0, 90));
                if (!wallChar(tile(x, y - 1))) DrawRectangleRec(rect(X, Y, TS, 3), rgba(152, 150, 166));
                if (c == '*') {
                    orect(X + 21, Y + 22, 6, 12, rgba(70, 60, 56), rgba(30, 24, 24), 1);
                    orect(X + 17, Y + 20, 14, 4, rgba(90, 80, 70), rgba(30, 24, 24), 1);
                }
            }
        } break;
        case 'C': case 'P': {
            DrawRectangleRec(rect(X, Y, TS, TS), rgba(70, 68, 80));
            for (int j = 0; j < 2; ++j) for (int i = 0; i < 2; ++i)
                DrawRectangleRec(rect(X + i * 24 + 1, Y + j * 24 + 1, 22, 22), shade(rgba(94, 92, 106), 0.86f + 0.2f * r.f()));
            if (r.f() < 0.3f) {
                Vector2 a = v2(X + r.rangef(6, 42), Y + r.rangef(6, 42)), b = v2(a.x + r.rangef(-10, 10), a.y + r.rangef(-10, 10));
                DrawLineEx(a, b, 1.2f, rgba(60, 58, 70)); DrawLineEx(b, v2(b.x + r.rangef(-6, 6), b.y + r.rangef(-6, 6)), 1.f, rgba(60, 58, 70));
            }
            if (wallChar(tile(x, y - 1))) DrawRectangleGradientV((int)X, (int)Y, TS, 12, rgba(0, 0, 0, 120), rgba(0, 0, 0, 0));
            if (c == 'P') {
                ellipse(X + 24, Y + 44, 18, 5, rgba(0, 0, 0, 80));
                orect(X + 8, Y + 36, 32, 8, rgba(138, 136, 150), ol, 1.5f);
                orect(X + 13, Y - 4, 22, 40, rgba(160, 158, 172), ol, 1.5f);
                DrawRectangleRec(rect(X + 13, Y - 4, 6, 40), rgba(190, 188, 200));
                DrawRectangleRec(rect(X + 29, Y - 4, 6, 40), rgba(124, 122, 136));
            }
        } break;
        case 'R': {
            DrawRectangleRec(rect(X, Y, TS, TS), rgba(176, 64, 52));
            for (int row = 0; row < 6; ++row) {
                float yy = Y + row * 8.f, off = (row % 2) ? 6.f : 0.f;
                DrawRectangleRec(rect(X, yy + 6, TS, 2), rgba(132, 44, 38));
                for (float bx = off; bx < TS; bx += 12.f) DrawRectangleRec(rect(X + bx, yy, 1.5f, 6), rgba(146, 50, 42));
            }
            if (tile(x, y - 1) != 'R') { DrawRectangleRec(rect(X, Y, TS, 7), rgba(110, 38, 32)); DrawRectangleRec(rect(X, Y + 1, TS, 2), rgba(210, 100, 84)); }
            if (tile(x, y + 1) != 'R') DrawRectangleRec(rect(X, Y + TS - 5, TS, 5), rgba(90, 30, 26));
            if (tile(x - 1, y) != 'R') DrawRectangleRec(rect(X, Y, 4, TS), rgba(120, 40, 34));
            if (tile(x + 1, y) != 'R') DrawRectangleRec(rect(X + TS - 4, Y, 4, TS), rgba(120, 40, 34));
        } break;
        case 'H': {
            const Color beam = rgba(118, 80, 52);
            DrawRectangleRec(rect(X, Y, TS, TS), rgba(232, 214, 178));
            DrawRectangleRec(rect(X, Y, TS, 5), beam);
            DrawRectangleRec(rect(X, Y + TS - 9, TS, 9), rgba(140, 134, 128));
            for (int i = 0; i < 3; ++i) DrawRectangleRec(rect(X + 4 + i * 16.f, Y + TS - 9, 1.5f, 9), rgba(100, 96, 92));
            bool L = tile(x - 1, y) == 'H', R = tile(x + 1, y) == 'H';
            if (!L) DrawRectangleRec(rect(X, Y, 5, TS - 9), beam);
            if (!R) DrawRectangleRec(rect(X + TS - 5, Y, 5, TS - 9), beam);
            if (L && R) {
                DrawRectangleRec(rect(X + 12, Y + 10, 24, 22), beam);
                DrawRectangleRec(rect(X + 14, Y + 12, 20, 18), rgba(120, 172, 212));
                DrawRectangleRec(rect(X + 14, Y + 12, 8, 6), rgba(190, 220, 240));
                DrawRectangleRec(rect(X + 23, Y + 12, 2, 18), beam);
                DrawRectangleRec(rect(X + 14, Y + 20, 20, 2), beam);
                DrawRectangleRec(rect(X + 10, Y + 32, 28, 5), rgba(130, 84, 50));
                for (int i = 0; i < 4; ++i) DrawCircleV(v2(X + 14 + i * 7.f, Y + 31), 2.2f, i % 2 ? rgba(230, 70, 80) : rgba(250, 200, 70));
            }
        } break;
        case 'X': DrawRectangleRec(rect(X, Y, TS, TS), rgba(14, 12, 20)); break;
        default: DrawRectangleRec(rect(X, Y, TS, TS), MAGENTA); break;
    }
}

void Game::drawTileOver(int x, int y) {
    const char c = tile(x, y);
    const float X = (float)x * TS, Y = (float)y * TS;
    if (c == 'T') {
        Rng tr(hash3(x, y, 99));
        bool birch = tr.f() < 0.45f;
        Color dk = birch ? rgba(96, 150, 58) : rgba(36, 96, 52);
        Color md = birch ? rgba(128, 182, 74) : rgba(52, 124, 64);
        Color lt = birch ? rgba(180, 216, 106) : rgba(86, 160, 82);
        float cx = X + 24, cy = Y + 12;
        Color ol = rgba(24, 44, 28);
        DrawCircleV(v2(cx - 11, cy + 8), 14.5f, ol); DrawCircleV(v2(cx + 11, cy + 8), 14.5f, ol);
        DrawCircleV(v2(cx, cy), 20.5f, ol); DrawCircleV(v2(cx, cy - 10), 13.5f, ol);
        DrawCircleV(v2(cx - 11, cy + 8), 13, dk); DrawCircleV(v2(cx + 11, cy + 8), 13, dk);
        DrawCircleV(v2(cx, cy), 19, dk); DrawCircleV(v2(cx, cy - 10), 12, dk);
        DrawCircleV(v2(cx - 4, cy - 2), 14, md); DrawCircleV(v2(cx + 8, cy + 5), 9, md);
        DrawCircleV(v2(cx - 8, cy - 8), 6, lt); DrawCircleV(v2(cx + 3, cy - 13), 4, lt);
        for (int i = 0; i < 5; ++i) DrawCircleV(v2(cx + tr.rangef(-15, 15), cy + tr.rangef(-14, 14)), 1.6f, lt);
    } else if (c == 'P') {
        Color ol = rgba(40, 30, 30);
        orect(X + 13, Y - 18, 22, 16, rgba(160, 158, 172), ol, 1.5f);
        DrawRectangleRec(rect(X + 13, Y - 18, 6, 16), rgba(190, 188, 200));
        orect(X + 7, Y - 24, 34, 8, rgba(176, 174, 188), ol, 1.5f);
    }
}

void Game::bake() {
    if (baked) { UnloadRenderTexture(baseRT); UnloadRenderTexture(overRT); }
    baseRT = LoadRenderTexture(W * TS * SS, H * TS * SS);
    overRT = LoadRenderTexture(W * TS * SS, H * TS * SS);
    SetTextureFilter(baseRT.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(overRT.texture, TEXTURE_FILTER_BILINEAR);
    baked = true;
    Camera2D c; c.offset = v2(0, 0); c.target = v2(0, 0); c.rotation = 0; c.zoom = (float)SS;
    BeginTextureMode(baseRT);
    ClearBackground(BLACK);
    BeginMode2D(c);
    for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) drawTileBase(x, y);
    EndMode2D();
    EndTextureMode();
    BeginTextureMode(overRT);
    ClearBackground(BLANK);
    BeginMode2D(c);
    for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) drawTileOver(x, y);
    EndMode2D();
    EndTextureMode();
}

void Game::drawAnimatedTile(int x, int y, float t) {
    const char c = tile(x, y);
    const float X = (float)x * TS, Y = (float)y * TS;
    if (c == '~') {
        for (int k = 0; k < 3; ++k) {
            float yy = Y + 9 + k * 15.f + std::sin(t * 1.7f + x * 0.9f + k * 2.1f) * 2.f;
            float off = std::fmod(t * (9.f + k * 3.f) + (float)((hash3(x, y, k) >> 4) % 40), 36.f);
            DrawRectangleRec(rect(X + off, yy, 12, 2), rgba(200, 228, 255, 80));
        }
        float foam = 0.45f + 0.25f * std::sin(t * 2.2f + x + y * 0.7f);
        for (int d = 0; d < 4; ++d) {
            char n = tile(x + DX[d], y + DY[d]);
            if (n == '~' || n == 'X') continue;
            Color fc = fade(rgba(236, 246, 255), foam);
            if (d == DIR_UP) DrawRectangleRec(rect(X, Y, TS, 3), fc);
            if (d == DIR_DOWN) DrawRectangleRec(rect(X, Y + TS - 3, TS, 3), fc);
            if (d == DIR_LEFT) DrawRectangleRec(rect(X, Y, 3, TS), fc);
            if (d == DIR_RIGHT) DrawRectangleRec(rect(X + TS - 3, Y, 3, TS), fc);
        }
        if (((hash3(x, y, (int)(t * 1.3f)) >> 3) % 9) == 0) {
            float sx = X + 10 + (float)((hash3(x, y, 5) >> 5) % 28), sy = Y + 12 + (float)((hash3(x, y, 6) >> 5) % 24);
            DrawCircleV(v2(sx, sy), 1.6f, rgba(255, 255, 255, 170));
        }
    } else if (c == '*') {
        float cx = X + 24, cy = Y + 14, f = std::sin(t * 13.f + x) * 1.5f + std::sin(t * 7.3f) * 1.f;
        DrawCircleV(v2(cx, cy), 14 + f, rgba(255, 150, 60, 40));
        tri(v2(cx, cy - 13 + f), v2(cx - 6, cy + 3), v2(cx + 6, cy + 3), rgba(255, 130, 40));
        DrawCircleV(v2(cx, cy + 2), 6.f, rgba(255, 130, 40));
        tri(v2(cx, cy - 7 + f * 0.6f), v2(cx - 3.5f, cy + 3), v2(cx + 3.5f, cy + 3), rgba(255, 220, 110));
        DrawCircleV(v2(cx, cy + 3), 3.f, rgba(255, 240, 180));
    }
}

}  // namespace rpg
