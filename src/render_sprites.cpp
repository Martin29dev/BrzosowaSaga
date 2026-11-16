// render_sprites.cpp — postacie i grafiki zdarzeń
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "game.h"

namespace rpg {

void Game::drawHumanoid(int pal, float X, float Y, int dir, float walk, float t, int seed) {
    const PalDef& p = PALS[pal];
    const bool ghost = p.style == 4;
    const float a = ghost ? 0.62f + 0.08f * std::sin(t * 2.f) : 1.f;
    const float cx = X + TS * 0.5f, foot = Y + TS - 5;
    const float swing = std::sin(walk * PI);
    float bob = std::fabs(swing) * 1.6f;
    if (ghost) bob = 4.f + std::sin(t * 2.6f + seed) * 2.5f;
    const Color ol = fade(rgba(34, 26, 36), a);
    auto F = [&](Color c) { return fade(c, a); };
    ellipse(cx, foot, ghost ? 9.f : 12.f, ghost ? 3.f : 4.f, rgba(0, 0, 0, ghost ? 40 : 75));
    const bool vert = dir == DIR_DOWN || dir == DIR_UP;
    const float side = dir == DIR_LEFT ? -1.f : 1.f;
    // nogi
    if (!ghost) {
        if (vert) {
            float l1 = std::max(0.f, swing) * 3.f, l2 = std::max(0.f, -swing) * 3.f;
            orect(cx - 7, foot - 10 - l1, 5, 9, p.legs, ol, 1.3f);
            orect(cx + 2, foot - 10 - l2, 5, 9, p.legs, ol, 1.3f);
        } else {
            orect(cx - 2.5f - swing * 4.f, foot - 10, 5, 9, shade(p.legs, 0.8f), ol, 1.3f);
            orect(cx - 2.5f + swing * 4.f, foot - 10, 5, 9, p.legs, ol, 1.3f);
        }
    }
    const float by = foot - 26 - bob, hy = foot - 35 - bob;
    // peleryna (bohater) z tyłu
    if (p.style == 0 && dir != DIR_UP) oround(rect(cx - 11, by - 1, 22, 19), 0.3f, shade(p.accent, 0.75f), ol);
    // ciało
    if (ghost) {
        oround(rect(cx - 10, by, 20, 20), 0.6f, F(p.body), ol);
        for (int i = 0; i < 3; ++i) DrawCircleV(v2(cx - 6 + i * 6.f, by + 20 + std::sin(t * 6.f + i) * 1.5f), 3.6f, F(p.body));
    } else {
        oround(rect(cx - 10, by, 20, 16), 0.35f, p.body, ol);
        DrawRectangleRec(rect(cx - 10, by + 10, 20, 2.5f), shade(p.body, 0.6f));
        if (p.style == 1) DrawRectangleRec(rect(cx - 2, by, 4, 16), p.accent);
    }
    // ręce
    if (vert) {
        float aw = swing * 2.f;
        ocircle(cx - 12, by + 7 + aw, 3.2f, F(shade(p.body, 0.85f)), ol, 1.3f);
        ocircle(cx + 12, by + 7 - aw, 3.2f, F(shade(p.body, 0.85f)), ol, 1.3f);
    } else {
        ocircle(cx + side * 1.f - swing * 5.f * side, by + 8, 3.4f, F(shade(p.body, 0.85f)), ol, 1.3f);
    }
    // głowa
    ocircle(cx, hy, 11.5f, F(p.skin), ol);
    Color hc = F(p.hair);
    if (p.style == 2) {   // warkocze
        if (dir != DIR_RIGHT) ocircle(cx - 12, hy + 4, 4.f, hc, ol, 1.2f);
        if (dir != DIR_LEFT) ocircle(cx + 12, hy + 4, 4.f, hc, ol, 1.2f);
    }
    switch (dir) {
        case DIR_DOWN: DrawCircleSector(v2(cx, hy - 1), 12.2f, 180, 360, 20, hc); DrawRectangleRec(rect(cx - 11, hy - 2, 22, 3), hc); break;
        case DIR_UP: DrawCircleV(v2(cx, hy), 12.2f, hc); break;
        case DIR_LEFT: DrawCircleSector(v2(cx, hy - 1), 12.2f, 170, 450, 24, hc); break;
        case DIR_RIGHT: DrawCircleSector(v2(cx, hy - 1), 12.2f, 90, 370, 24, hc); break;
    }
    // twarz
    if (dir != DIR_UP) {
        bool blink = std::fmod(t + seed * 0.73f, 4.f) < 0.12f;
        float eh = (blink || p.style == 1) ? 1.2f : 4.f;
        Color ec = ghost ? fade(rgba(40, 60, 120), a) : rgba(40, 30, 40);
        if (dir == DIR_DOWN) {
            DrawRectangleRec(rect(cx - 5, hy + 1, 2.4f, eh), ec);
            DrawRectangleRec(rect(cx + 2.6f, hy + 1, 2.4f, eh), ec);
            if (!ghost) { DrawCircleV(v2(cx - 7, hy + 6), 1.8f, rgba(255, 150, 150, 120)); DrawCircleV(v2(cx + 7, hy + 6), 1.8f, rgba(255, 150, 150, 120)); }
        } else {
            DrawRectangleRec(rect(cx + side * 5.5f - 1.2f, hy + 1, 2.4f, eh), ec);
        }
    }
    if (p.style == 1 && dir != DIR_UP) {   // broda
        float bx = dir == DIR_DOWN ? cx : cx + side * 4.f;
        DrawCircleSector(v2(bx, hy + 4), 8.5f, 0, 180, 16, rgba(245, 245, 250));
        DrawRectangleRec(rect(bx - 5, hy + 3, 10, 2.5f), rgba(245, 245, 250));
    }
    if (p.style == 3) {   // kapelusz
        ellipse(cx, hy - 7, 16, 4.5f, ol);
        ellipse(cx, hy - 7, 14.5f, 3.5f, p.accent);
        oround(rect(cx - 8, hy - 17, 16, 10), 0.4f, shade(p.accent, 0.85f), ol);
        DrawRectangleRec(rect(cx - 8, hy - 10, 16, 2.5f), rgba(150, 60, 50));
    }
    if (p.style == 0 && dir == DIR_UP) oround(rect(cx - 11, by - 1, 22, 20), 0.3f, p.accent, ol);
}

void Game::drawEvent(const Event& e, float X, float Y, float t) {
    if (!e.pg) return;
    const Color ol = rgba(34, 26, 36);
    const float cx = X + TS * 0.5f;
    switch (e.pg->gfx) {
        case G::None: break;
        case G::Char: drawHumanoid(e.pg->palId, X, Y, e.dir, e.walk, t, e.id); break;
        case G::Chest: case G::ChestOpen: {
            ellipse(cx, Y + 42, 18, 4.5f, rgba(0, 0, 0, 70));
            const Color wood = rgba(156, 96, 46), gold = rgba(236, 196, 80);
            oround(rect(X + 8, Y + 20, 32, 21), 0.15f, wood, ol);
            DrawRectangleRec(rect(X + 8, Y + 31, 32, 2), shade(wood, 0.7f));
            if (e.pg->gfx == G::Chest) {
                oround(rect(X + 8, Y + 12, 32, 12), 0.45f, rgba(178, 112, 54), ol);
                DrawRectangleRec(rect(X + 13, Y + 12, 4, 29), gold);
                DrawRectangleRec(rect(X + 31, Y + 12, 4, 29), gold);
                orect(X + 21, Y + 21, 6, 8, gold, ol, 1);
                DrawRectangleRec(rect(X + 23.2f, Y + 23.5f, 1.6f, 3.5f), ol);
            } else {
                DrawRectangleRec(rect(X + 10, Y + 20, 28, 6), rgba(40, 24, 16));
                oround(rect(X + 8, Y + 6, 32, 9), 0.4f, shade(rgba(178, 112, 54), 0.8f), ol);
                DrawRectangleRec(rect(X + 13, Y + 20, 4, 21), gold);
                DrawRectangleRec(rect(X + 31, Y + 20, 4, 21), gold);
            }
        } break;
        case G::Sign: {
            ellipse(cx, Y + 43, 10, 3, rgba(0, 0, 0, 70));
            orect(X + 21, Y + 24, 6, 19, rgba(120, 82, 48), ol);
            oround(rect(X + 7, Y + 9, 34, 19), 0.2f, rgba(180, 132, 74), ol);
            for (int i = 0; i < 3; ++i) DrawRectangleRec(rect(X + 12, Y + 13 + i * 5.f, 24 - i * 6.f, 1.8f), rgba(110, 70, 40));
        } break;
        case G::Door: case G::DoorOpen: {
            const Color frame = rgba(96, 70, 50);
            DrawRectangleRec(rect(X + 3, Y + 12, 42, 36), frame);
            DrawCircleSector(v2(cx, Y + 13), 21, 180, 360, 24, frame);
            if (e.pg->gfx == G::Door) {
                const Color wood = rgba(134, 86, 48);
                DrawRectangleRec(rect(X + 7, Y + 13, 34, 35), wood);
                DrawCircleSector(v2(cx, Y + 13), 17, 180, 360, 24, wood);
                for (int i = 1; i < 4; ++i) DrawRectangleRec(rect(X + 7 + i * 8.5f, Y + 0, 1.5f, 48), shade(wood, 0.75f));
                DrawRectangleRec(rect(X + 7, Y + 18, 34, 3), rgba(64, 62, 70));
                DrawRectangleRec(rect(X + 7, Y + 38, 34, 3), rgba(64, 62, 70));
                DrawRing(v2(X + 33, Y + 29), 2.f, 3.6f, 0, 360, 16, rgba(236, 196, 80));
            } else {
                DrawRectangleRec(rect(X + 7, Y + 13, 34, 35), rgba(18, 12, 14));
                DrawCircleSector(v2(cx, Y + 13), 17, 180, 360, 24, rgba(18, 12, 14));
                DrawRectangleGradientV((int)(X + 7), (int)(Y + 30), 34, 18, rgba(18, 12, 14, 0), rgba(60, 50, 50, 200));
            }
        } break;
        case G::Gate: {
            ellipse(cx, Y + 46, 22, 4, rgba(0, 0, 0, 80));
            for (int i = 0; i < 5; ++i) {
                float bx = X + 5 + i * 9.f;
                orect(bx, Y + 1, 4, 44, rgba(62, 62, 74), rgba(26, 24, 30), 1);
                DrawRectangleRec(rect(bx, Y + 1, 1.3f, 44), rgba(130, 130, 150));
                tri(v2(bx - 1, Y + 45), v2(bx + 5, Y + 45), v2(bx + 2, Y + 49), rgba(62, 62, 74));
            }
            orect(X + 2, Y + 9, 44, 4, rgba(74, 74, 88), rgba(26, 24, 30), 1);
            orect(X + 2, Y + 31, 44, 4, rgba(74, 74, 88), rgba(26, 24, 30), 1);
        } break;
        case G::GateOpen: {
            for (int i = 0; i < 5; ++i) orect(X + 5 + i * 9.f, Y - 2, 4, 7, rgba(62, 62, 74), rgba(26, 24, 30), 1);
            orect(X + 2, Y + 1, 44, 4, rgba(74, 74, 88), rgba(26, 24, 30), 1);
            DrawRectangleGradientV((int)X, (int)(Y + 6), TS, 10, rgba(0, 0, 0, 90), rgba(0, 0, 0, 0));
        } break;
        case G::LeverOff: case G::LeverOn: {
            bool on = e.pg->gfx == G::LeverOn;
            ellipse(cx, Y + 41, 14, 4, rgba(0, 0, 0, 70));
            oround(rect(X + 11, Y + 30, 26, 10), 0.4f, rgba(96, 92, 104), ol);
            Vector2 piv = v2(cx, Y + 33), tip = v2(on ? X + 35.f : X + 13.f, Y + 12);
            DrawLineEx(piv, tip, 6.f, ol);
            DrawLineEx(piv, tip, 3.5f, rgba(150, 108, 64));
            ocircle(tip.x, tip.y, 4.8f, on ? rgba(90, 210, 100) : rgba(220, 70, 70), ol);
            DrawCircleV(v2(tip.x - 1.5f, tip.y - 1.5f), 1.5f, rgba(255, 255, 255, 150));
        } break;
        case G::Plate: {
            bool pr = e.pressed;
            float d = pr ? 3.f : 0.f;
            if (!pr) DrawRectangleRec(rect(X + 6, Y + 40, 36, 4), rgba(0, 0, 0, 70));
            oround(rect(X + 6, Y + 6 + d, 36, 34 - d), 0.25f, pr ? rgba(122, 118, 112) : rgba(156, 152, 144), ol);
            if (!pr) DrawRectangleRec(rect(X + 8, Y + 34, 32, 5), rgba(120, 116, 110));
            Color rc = pr ? rgba(120, 220, 255) : rgba(96, 94, 90);
            DrawRing(v2(cx, Y + 22 + d * 0.5f), 6.f, 8.f, 0, 360, 24, rc);
            DrawCircleV(v2(cx, Y + 22 + d * 0.5f), 2.5f, rc);
            if (pr) DrawCircleV(v2(cx, Y + 22 + d * 0.5f), 12.f, rgba(120, 220, 255, 40));
        } break;
        case G::Boulder: {
            ellipse(cx, Y + 42, 18, 5, rgba(0, 0, 0, 90));
            ocircle(cx, Y + 25, 18.5f, rgba(134, 128, 120), ol, 1.8f);
            DrawCircleSector(v2(cx, Y + 25), 18.5f, -20, 160, 20, rgba(110, 104, 98));
            DrawCircleV(v2(cx - 6, Y + 18), 6.f, rgba(172, 168, 160));
            DrawCircleV(v2(cx - 8, Y + 16), 2.5f, rgba(206, 202, 196));
            DrawLineEx(v2(cx + 2, Y + 14), v2(cx + 6, Y + 24), 1.5f, rgba(80, 76, 72));
            DrawLineEx(v2(cx + 6, Y + 24), v2(cx + 12, Y + 28), 1.5f, rgba(80, 76, 72));
        } break;
        case G::Bridge: {
            DrawRectangleRec(rect(X, Y + 44, TS, 4), rgba(20, 40, 80, 90));
            DrawRectangleRec(rect(X, Y + 6, TS, 38), rgba(158, 110, 64));
            for (int i = 0; i < 6; ++i) DrawRectangleRec(rect(X + i * 8.f, Y + 6, 1.6f, 38), rgba(116, 78, 44));
            DrawRectangleRec(rect(X, Y + 3, TS, 5), rgba(104, 70, 40));
            DrawRectangleRec(rect(X, Y + 40, TS, 5), rgba(104, 70, 40));
            DrawRectangleRec(rect(X, Y + 3, TS, 1.5f), rgba(170, 120, 74));
        } break;
        case G::CrystalOff: case G::CrystalOn: {
            bool on = e.pg->gfx == G::CrystalOn;
            float fl = on ? std::sin(t * 2.5f + e.id) * 2.f : 0.f;
            ellipse(cx, Y + 43, 15, 4, rgba(0, 0, 0, 80));
            oround(rect(X + 11, Y + 33, 26, 10), 0.3f, rgba(110, 106, 120), ol);
            Color cc = on ? mixc(rgba(110, 220, 255), rgba(200, 245, 255), 0.5f + 0.5f * std::sin(t * 3.f)) : rgba(88, 98, 128);
            if (on) DrawCircleV(v2(cx, Y + 17 + fl), 20.f, rgba(110, 220, 255, 40));
            Vector2 top = v2(cx, Y + 1 + fl), l = v2(cx - 10, Y + 17 + fl), r = v2(cx + 10, Y + 17 + fl), b = v2(cx, Y + 33 + fl);
            tri(top, l, r, ol); tri(l, b, r, ol);
            Vector2 top2 = v2(cx, Y + 3.5f + fl), l2 = v2(cx - 8, Y + 17 + fl), r2 = v2(cx + 8, Y + 17 + fl), b2 = v2(cx, Y + 30.5f + fl);
            tri(top2, l2, r2, cc); tri(l2, b2, r2, shade(cc, 0.8f));
            tri(top2, l2, v2(cx - 1, Y + 17 + fl), shade(cc, 1.25f));
            if (on) for (int i = 0; i < 3; ++i) {
                float ang = t * 1.5f + i * 2.1f;
                DrawCircleV(v2(cx + std::cos(ang) * 15.f, Y + 17 + std::sin(ang) * 9.f + fl), 1.5f, rgba(220, 250, 255, 200));
            }
        } break;
        case G::Coin: {
            float bob = std::sin(t * 3.f + e.id) * 2.5f;
            float w = std::fabs(std::cos(t * 3.2f)) * 9.f + 1.5f;
            ellipse(cx, Y + 40, 8, 2.5f, rgba(0, 0, 0, 70));
            ellipse(cx, Y + 24 + bob, w + 1.5f, 10.5f, rgba(120, 90, 30));
            ellipse(cx, Y + 24 + bob, w, 9, rgba(240, 200, 70));
            ellipse(cx, Y + 24 + bob, w * 0.55f, 5.5f, rgba(255, 228, 130));
            if (std::fmod(t, 1.4f) < 0.25f) {
                float s = 5.f;
                DrawLineEx(v2(cx + 6 - s, Y + 16 + bob), v2(cx + 6 + s, Y + 16 + bob), 1.5f, WHITE);
                DrawLineEx(v2(cx + 6, Y + 16 - s + bob), v2(cx + 6, Y + 16 + s + bob), 1.5f, WHITE);
            }
        } break;
        case G::Statue: {
            ellipse(cx, Y + 44, 18, 4, rgba(0, 0, 0, 90));
            oround(rect(X + 8, Y + 34, 32, 10), 0.2f, rgba(110, 108, 120), ol);
            oround(rect(X + 13, Y + 10, 22, 26), 0.5f, rgba(150, 150, 160), ol);
            ocircle(cx, Y + 8, 7.5f, rgba(164, 164, 174), ol);
            DrawRectangleRec(rect(cx - 1, Y + 14, 2, 20), rgba(120, 120, 130));
            DrawCircleV(v2(cx - 6, Y + 30), 2.5f, rgba(90, 130, 80, 180));
            DrawCircleV(v2(cx + 5, Y + 20), 2.f, rgba(90, 130, 80, 160));
        } break;
        case G::Well: {
            ellipse(cx, Y + 34, 22, 12, rgba(0, 0, 0, 70));
            orect(X + 4, Y + 2, 4, 28, rgba(120, 82, 48), ol);
            orect(X + 40, Y + 2, 4, 28, rgba(120, 82, 48), ol);
            ellipse(cx, Y + 30, 21.5f, 13.5f, ol);
            ellipse(cx, Y + 30, 20, 12, rgba(136, 132, 126));
            ellipse(cx, Y + 29, 14, 7, rgba(26, 46, 80));
            ellipse(cx - 3, Y + 28, 4 + std::sin(t * 2.f) * 1.5f, 1.5f, rgba(120, 170, 230, 160));
            tri(v2(cx, Y - 10), v2(X - 1, Y + 5), v2(X + 49, Y + 5), ol);
            tri(v2(cx, Y - 8), v2(X + 2, Y + 4), v2(X + 46, Y + 4), rgba(160, 62, 50));
            DrawRectangleRec(rect(X + 8, Y + 8, 32, 2.5f), rgba(100, 70, 40));
            DrawLineEx(v2(cx, Y + 10), v2(cx, Y + 20), 1.2f, rgba(200, 190, 160));
        } break;
        case G::Stairs: {
            for (int i = 0; i < 4; ++i) {
                float sh = 0.6f + i * 0.13f;
                DrawRectangleRec(rect(X + 6 + i * 1.f, Y + 6 + i * 9.f, 36 - i * 2.f, 8), shade(rgba(140, 138, 154), sh));
                DrawRectangleRec(rect(X + 6 + i * 1.f, Y + 12 + i * 9.f, 36 - i * 2.f, 2), rgba(40, 38, 48));
            }
            tri(v2(cx, Y + 2), v2(cx - 5, Y + 8), v2(cx + 5, Y + 8), rgba(255, 240, 200, 120));
        } break;
        case G::Seal: case G::SealOpen: {
            DrawRectangleRec(rect(X + 2, Y + 2, 44, 46), rgba(66, 64, 78));
            if (e.pg->gfx == G::Seal) {
                DrawRectangleRec(rect(X + 6, Y + 6, 36, 42), rgba(102, 98, 116));
                DrawRectangleRec(rect(X + 23.5f, Y + 6, 1.2f, 42), rgba(70, 66, 80));
                float pw = ((sw[SW_CRYSTAL_W] ? 1 : 0) + (sw[SW_CRYSTAL_E] ? 1 : 0) + (sw[SW_CRYPT_PLATE] ? 1 : 0)) / 3.f;
                Color rc = mixc(rgba(70, 66, 86), rgba(190, 140, 255), pw * (0.75f + 0.25f * std::sin(t * 3.f)));
                DrawRing(v2(cx, Y + 22), 8.f, 11.f, 0, 360, 28, rc);
                tri(v2(cx, Y + 15), v2(cx - 5, Y + 25), v2(cx + 5, Y + 25), rc);
                const bool lit[3] = {sw[SW_CRYSTAL_W], sw[SW_CRYPT_PLATE], sw[SW_CRYSTAL_E]};
                for (int i = 0; i < 3; ++i) {
                    Vector2 pp = v2(X + 14 + i * 10.f, Y + 40);
                    if (lit[i]) DrawCircleV(pp, 5.f, rgba(160, 120, 255, 80));
                    DrawCircleV(pp, 2.6f, lit[i] ? rgba(210, 180, 255) : rgba(50, 48, 60));
                }
            } else {
                DrawRectangleRec(rect(X + 6, Y + 6, 36, 42), rgba(16, 12, 22));
                DrawRectangleGradientV((int)(X + 6), (int)(Y + 26), 36, 22, rgba(16, 12, 22, 0), rgba(90, 70, 140, 150));
            }
        } break;
    }
}

}  // namespace rpg
