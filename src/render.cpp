// render.cpp — potok renderowania: świat, światła, cząsteczki, UI, letterbox
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "game.h"

namespace rpg {

void Game::renderLights(const Camera2D& cam1, float alpha) {
    BeginTextureMode(lightRT);
    ClearBackground(rgba(34, 30, 54));
    BeginMode2D(cam1);
    BeginBlendMode(BLEND_ADDITIVE);
    float hx = hero.ix(alpha) * TS + 24, hy = hero.iy(alpha) * TS + 18;
    glow((float)((int)hx), (float)((int)hy), 200.f, rgba(200, 176, 140));
    for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) if (tile(x, y) == '*') {
        float fl = 0.88f + 0.08f * std::sin(rtime * 11.f + x) + 0.04f * std::sin(rtime * 23.f + y);
        glow((float)(x * TS + 24), (float)(y * TS + 20), 170.f * fl, rgba(230, 140, 70));
    }
    for (const auto& e : events) {
        if (!e.pg) continue;
        int ex = (int)(e.ix(alpha) * TS + 24), ey = (int)(e.iy(alpha) * TS + 22);
        switch (e.pg->gfx) {
            case G::CrystalOn: glow((float)(ex), (float)(ey), 150.f + std::sin(rtime * 3.f) * 10.f, rgba(70, 170, 230)); break;
            case G::CrystalOff: glow((float)(ex), (float)(ey), 40.f, rgba(40, 50, 80)); break;
            case G::Chest: glow((float)(ex), (float)(ey), 70.f, rgba(110, 90, 30)); break;
            case G::Coin: glow((float)(ex), (float)(ey), 50.f, rgba(110, 90, 30)); break;
            case G::Stairs: glow((float)(ex), (float)(ey), 110.f, rgba(70, 76, 110)); break;
            case G::Seal: {
                float pw = ((sw[SW_CRYSTAL_W] ? 1 : 0) + (sw[SW_CRYSTAL_E] ? 1 : 0) + (sw[SW_CRYPT_PLATE] ? 1 : 0)) / 3.f;
                if (pw > 0) glow((float)(ex), (float)(ey), 60.f + 60.f * pw, rgba((int)(110 * pw), (int)(70 * pw), (int)(170 * pw)));
            } break;
            case G::SealOpen: glow((float)(ex), (float)(ey), 110.f, rgba(100, 70, 160)); break;
            case G::Plate: if (e.pressed) glow((float)(ex), (float)(ey), 70.f, rgba(60, 130, 160)); break;
            case G::Char: if (e.pg->palId == PAL_GHOST) glow((float)(ex), (float)(ey - 10), 100.f, rgba(70, 110, 170)); break;
            default: break;
        }
    }
    if (flashT > 0) DrawRectangle(-10000, -10000, 20000, 20000, fade(flashCol, 0.5f * flashT / flashDur));
    EndBlendMode();
    EndMode2D();
    EndTextureMode();
}

void Game::drawWorld(float alpha) {
    const float t = rtime;
    DrawTexturePro(baseRT.texture, rect(0, 0, (float)baseRT.texture.width, -(float)baseRT.texture.height),
                   rect(0, 0, (float)W * TS, (float)H * TS), v2(0, 0), 0, WHITE);
    Vector2 ft = focusTile(alpha);
    Vector2 ct = camFor(ft.x, ft.y);
    int x0 = std::max(0, (int)((ct.x - VW * 0.5f) / TS) - 1), x1 = std::min(W - 1, (int)((ct.x + VW * 0.5f) / TS) + 1);
    int y0 = std::max(0, (int)((ct.y - VH * 0.5f) / TS) - 1), y1 = std::min(H - 1, (int)((ct.y + VH * 0.5f) / TS) + 1);
    for (int y = y0; y <= y1; ++y) for (int x = x0; x <= x1; ++x) drawAnimatedTile(x, y, t);
    // pod postaciami
    for (const auto& e : events)
        if (e.pg && e.pg->prio == Prio::Below) drawEvent(e, snapPx(e.ix(alpha) * TS), snapPx(e.iy(alpha) * TS), t);
    // cel kliknięcia
    if (destX >= 0 && !title) {
        float p = 0.5f + 0.5f * std::sin(t * 8.f);
        Rectangle r = rect(destX * (float)TS + 3, destY * (float)TS + 3, TS - 6, TS - 6);
        DrawRectangleRec(r, rgba(255, 255, 255, (int)(30 + 30 * p)));
        DrawRectangleLinesEx(r, 2.5f, rgba(255, 255, 255, (int)(120 + 100 * p)));
    }
    // Y-sorting: gracz + zdarzenia "jak postacie"
    std::vector<std::pair<float, int>> order;
    if (!title) order.push_back(std::make_pair(hero.iy(alpha), -1));
    for (int i = 0; i < (int)events.size(); ++i)
        if (events[i].pg && events[i].pg->prio == Prio::Same) order.push_back(std::make_pair(events[i].iy(alpha), i));
    std::sort(order.begin(), order.end());
    for (const auto& o : order) {
        if (o.second < 0) drawHumanoid(PAL_HERO, snapPx(hero.ix(alpha) * TS), snapPx(hero.iy(alpha) * TS), hero.dir, hero.walk, t, 0);
        else { const Event& e = events[o.second]; drawEvent(e, snapPx(e.ix(alpha) * TS), snapPx(e.iy(alpha) * TS), t); }
    }
    for (const auto& e : events)
        if (e.pg && e.pg->prio == Prio::Above) drawEvent(e, snapPx(e.ix(alpha) * TS), snapPx(e.iy(alpha) * TS), t);
    DrawTexturePro(overRT.texture, rect(0, 0, (float)overRT.texture.width, -(float)overRT.texture.height),
                   rect(0, 0, (float)W * TS, (float)H * TS), v2(0, 0), 0, WHITE);
    if (debug) {
        for (int y = y0; y <= y1; ++y) for (int x = x0; x <= x1; ++x) {
            if (!tilePass(x, y)) DrawRectangleRec(rect(x * (float)TS, y * (float)TS, TS, TS), rgba(255, 40, 40, 70));
            DrawRectangleLinesEx(rect(x * (float)TS, y * (float)TS, TS, TS), 0.5f, rgba(255, 255, 255, 40));
        }
        for (const auto& e : events) {
            char b[32]; std::snprintf(b, sizeof(b), "%d:%d", e.id, e.pageIdx + 1);
            g_text.draw(b, e.x * (float)TS + 2, e.y * (float)TS + 2, 12, rgba(255, 255, 120));
        }
        DrawRectangleLinesEx(rect(hero.x * (float)TS, hero.y * (float)TS, TS, TS), 2, rgba(80, 255, 120, 200));
    }
}

void Game::drawParticles() {
    for (const auto& p : parts) {
        float lf = p.life / p.max, a = std::min(1.f, std::min(lf * 3.f, (1.f - lf) * 4.f));
        if (p.kind == 0) DrawRectanglePro(rect(p.x, p.y, p.size * 1.6f, p.size), v2(p.size * 0.8f, p.size * 0.5f), p.rot, fade(p.col, a * 0.9f));
        else {
            float tw = 0.6f + 0.4f * std::sin(p.rot * 0.05f);
            DrawCircleV(v2(p.x, p.y), p.size * 2.2f, fade(p.col, a * 0.12f * tw));
            DrawCircleV(v2(p.x, p.y), p.size, fade(p.col, a * 0.7f * tw));
        }
    }
}

void Game::render(float alpha) {
    rtime = (float)(time + alpha * STEP);
    Vector2 ft = focusTile(alpha);
    Vector2 ct = camFor(ft.x, ft.y);
    ct.x = snapPx(ct.x); ct.y = snapPx(ct.y);
    Camera2D cam; cam.target = ct; cam.offset = v2((VW * 0.5f + shakeX) * SS, VH * 0.5f * SS); cam.rotation = 0; cam.zoom = (float)SS;
    Camera2D cam1 = cam; cam1.offset = v2(VW * 0.5f + shakeX, VH * 0.5f); cam1.zoom = 1.f;
    const bool dark = maps[mapId].dark;
    if (dark) renderLights(cam1, alpha);

    BeginTextureMode(screenRT);
    ClearBackground(BLACK);
    BeginMode2D(cam);
    drawWorld(alpha);
    EndMode2D();
    if (dark) {
        BeginBlendMode(BLEND_MULTIPLIED);
        DrawTexturePro(lightRT.texture, rect(0, 0, (float)VW, -(float)VH), rect(0, 0, (float)VW * SS, (float)VH * SS), v2(0, 0), 0, WHITE);
        EndBlendMode();
    }
    BeginMode2D(cam);
    drawParticles();
    EndMode2D();

    Camera2D ui; ui.target = v2(0, 0); ui.offset = v2(0, 0); ui.rotation = 0; ui.zoom = (float)SS;
    BeginMode2D(ui);
    if (flashT > 0 && !dark) DrawRectangle(0, 0, VW, VH, fade(flashCol, 0.7f * flashT / flashDur));
    if (title) drawTitle();
    if (fadeA > 0) DrawRectangle(0, 0, VW, VH, rgba(0, 0, 0, (int)(255 * fadeA)));
    if (!title && bannerT > 0 && fadeState != 1) {
        float ba = std::min(1.f, std::min((3.4f - bannerT) * 2.5f, bannerT * 1.5f));
        DrawRectangleGradientH(0, 22, 420, 46, rgba(0, 0, 0, (int)(170 * ba)), rgba(0, 0, 0, 0));
        g_text.draw(maps[mapId].name, 28, 30, 27, WHITE, ba);
    }
    if (!title) drawMessage();
    if (menuOpen) drawMenu();
    if (toastT > 0) {
        float ta = std::min(1.f, toastT * 2.f);
        float w = g_text.measure(toastText, 21) + 40;
        Rectangle r = rect(VW * 0.5f - w * 0.5f, 14, w, 42);
        DrawRectangleRec(r, rgba(10, 12, 34, (int)(220 * ta)));
        DrawRectangleLinesEx(r, 2, rgba(236, 238, 255, (int)(220 * ta)));
        g_text.draw(toastText, r.x + 20, r.y + 9, 21, WHITE, ta);
    }
    if (debug) {
        static const char* modes[4] = {"VSync", "30 FPS", "144 FPS", "bez limitu"};
        char b[256];
        std::snprintf(b, sizeof(b), "FPS %d (%s)  klatka %.2f ms  kroki logiki %d  alpha %.2f\n"
                      "mapa %d  gracz %d,%d  dir %d  zdarzenia %zu  czastki %zu",
                      GetFPS(), modes[fpsMode], lastFrame * 1000.f, lastSteps, alpha,
                      mapId, hero.x, hero.y, hero.dir, events.size(), parts.size());
        DrawRectangle(0, VH - 56, VW, 56, rgba(0, 0, 0, 160));
        g_text.draw(b, 10, VH - 50, 16, rgba(140, 255, 160));
    }
    EndMode2D();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    float sw_ = (float)GetScreenWidth(), sh_ = (float)GetScreenHeight();
    float sc = std::min(sw_ / VW, sh_ / VH);
    Rectangle dst = rect((sw_ - VW * sc) * 0.5f, (sh_ - VH * sc) * 0.5f, VW * sc, VH * sc);
    DrawTexturePro(screenRT.texture, rect(0, 0, (float)VW * SS, -(float)VH * SS), dst, v2(0, 0), 0, WHITE);
    EndDrawing();
}

}  // namespace rpg
