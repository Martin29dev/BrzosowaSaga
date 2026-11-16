// render_ui.cpp — okna w stylu MV, wiadomości, menu, ekran tytułowy
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "game.h"

namespace rpg {

void Game::drawWindow(Rectangle r, float open) {
    if (open <= 0.f) return;
    float h = r.height * open;
    Rectangle rr = rect(r.x, r.y + (r.height - h) * 0.5f, r.width, h);
    DrawRectangleGradientV((int)rr.x, (int)rr.y, (int)rr.width, (int)rr.height, rgba(30, 36, 80, 228), rgba(10, 12, 34, 238));
    DrawRectangleLinesEx(rr, 2.5f, rgba(236, 238, 255, 235));
    DrawRectangleLinesEx(rect(rr.x + 3.5f, rr.y + 3.5f, rr.width - 7, rr.height - 7), 1.f, rgba(110, 120, 200, 110));
}

void Game::drawCursorRect(Rectangle r) {
    float p = 0.5f + 0.5f * std::sin(rtime * 5.f);
    DrawRectangleRec(r, fade(rgba(170, 190, 255), 0.16f + 0.14f * p));
    DrawRectangleLinesEx(r, 1.5f, fade(rgba(224, 232, 255), 0.55f + 0.35f * p));
}

void Game::drawMessage() {
    if (msg.open <= 0.f) return;
    Rectangle r = msgRect();
    drawWindow(r, msg.open);
    if (msg.open < 1.f) return;
    if (!msg.name.empty()) {
        float w = g_text.measure(msg.name, 22) + 36;
        Rectangle nr = rect(r.x, r.y - 46, w, 42);
        drawWindow(nr);
        g_text.draw(msg.name, nr.x + 18, nr.y + 8, 22, rgba(255, 226, 110));
    }
    g_text.drawGlyphs(msg.glyphs, (size_t)msg.shown, r.x + 24, r.y + 18, 24, WHITE);
    if (msg.done && !msg.choosing) {
        float b = std::sin(rtime * 6.f) * 2.5f, mx = r.x + r.width * 0.5f;
        tri(v2(mx - 8, r.y + r.height - 20 + b), v2(mx, r.y + r.height - 11 + b), v2(mx + 8, r.y + r.height - 20 + b), rgba(240, 240, 255));
    }
    if (msg.choosing) {
        drawWindow(choiceRect());
        for (int i = 0; i < (int)msg.choices.size(); ++i) {
            Rectangle row = choiceRow(i);
            if (i == msg.cursor) drawCursorRect(row);
            g_text.draw(msg.choices[i], row.x + 16, row.y + 6, 22, WHITE);
        }
    }
}

void Game::drawItemIcon(int it, float x, float y) {
    Color c = ITEMS[it].col, ol = rgba(20, 16, 24);
    if (it == IT_KEY) {
        DrawRing(v2(x + 6, y + 10), 3.f, 6.f, 0, 360, 20, ol);
        DrawRing(v2(x + 6, y + 10), 3.5f, 5.2f, 0, 360, 20, c);
        orect(x + 10, y + 9, 12, 3, c, ol, 1);
        DrawRectangleRec(rect(x + 17, y + 12, 2.5f, 4), c);
        DrawRectangleRec(rect(x + 20, y + 12, 2.5f, 3), c);
    } else if (it == IT_COIN) {
        ocircle(x + 11, y + 10, 8, c, ol); DrawCircleV(v2(x + 11, y + 10), 4.5f, shade(c, 1.12f));
    } else {
        DrawCircleV(v2(x + 11, y + 10), 11, fade(c, 0.25f + 0.1f * std::sin(rtime * 3.f)));
        tri(v2(x + 11, y), v2(x + 3, y + 10), v2(x + 19, y + 10), c);
        tri(v2(x + 3, y + 10), v2(x + 11, y + 21), v2(x + 19, y + 10), shade(c, 0.8f));
    }
}

void Game::drawMenu() {
    float a = menuAnim;
    DrawRectangle(0, 0, VW, VH, rgba(0, 0, 10, (int)(130 * a)));
    const char* cmds[MENU_ITEMS] = {"Wróć", "Zapisz grę", "Wczytaj grę",
                                    fullscreen ? "Pełny ekran: Wł." : "Pełny ekran: Wył.", "Do tytułu"};
    Rectangle cw = rect(16, 16, 232, MENU_ITEMS * 44 + 28);
    drawWindow(cw, a);
    Rectangle sw2 = rect(264, 16, VW - 280, VH - 32);
    drawWindow(sw2, a);
    if (a < 1.f) return;
    for (int i = 0; i < MENU_ITEMS; ++i) {
        Rectangle row = menuCmdRow(i);
        if (i == menuCur) drawCursorRect(row);
        g_text.draw(cmds[i], row.x + 14, row.y + 8, 23, WHITE);
    }
    Rectangle gw = rect(16, cw.y + cw.height + 12, 232, 150);
    drawWindow(gw);
    int sec = (int)playTime;
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", sec / 3600, (sec / 60) % 60, sec % 60);
    g_text.draw("Czas gry", gw.x + 18, gw.y + 16, 19, rgba(96, 184, 255));
    g_text.draw(buf, gw.x + 18, gw.y + 40, 24, WHITE);
    g_text.draw("Lokacja", gw.x + 18, gw.y + 78, 19, rgba(96, 184, 255));
    g_text.draw(maps[mapId].name, gw.x + 18, gw.y + 102, 21, WHITE);

    float x = sw2.x + 24, y = sw2.y + 20, w = sw2.width - 48;
    g_text.draw("Ekwipunek", x, y, 24, rgba(96, 184, 255)); y += 40;
    bool any = false;
    for (int i = 0; i < IT_COUNT; ++i) {
        if (items[i] <= 0) continue;
        any = true;
        drawItemIcon(i, x, y + 2);
        g_text.draw(ITEMS[i].name, x + 32, y, 22, WHITE);
        std::snprintf(buf, sizeof(buf), "×%d", items[i]);
        g_text.draw(buf, x + w - g_text.measure(buf, 22), y, 22, WHITE);
        g_text.draw(ITEMS[i].desc, x + 32, y + 28, 17, rgba(170, 176, 200));
        y += 62;
    }
    if (!any) { g_text.draw("(pusto)", x, y, 20, rgba(160, 160, 172)); y += 40; }
    y += 10;
    DrawRectangleRec(rect(x, y, w, 1.5f), rgba(110, 120, 200, 140)); y += 16;
    g_text.draw("Aktualny cel", x, y, 24, rgba(96, 184, 255)); y += 38;
    y += g_text.drawWrapped(objective(var[V_STAGE]), x, y, 20, w, WHITE) + 16;
    DrawRectangleRec(rect(x, y, w, 1.5f), rgba(110, 120, 200, 140)); y += 16;
    g_text.draw("Sterowanie", x, y, 20, rgba(96, 184, 255)); y += 30;
    g_text.drawWrapped("Strzałki/WASD — ruch · Shift — bieg · Z/Enter — akcja · X/Esc — menu · "
                       "LPM — idź · F5/F9 — zapis/odczyt · F1 — debug · F2 — limit FPS · F11 / Alt+Enter — pełny ekran",
                       x, y, 16, w, rgba(176, 180, 200));
}

void Game::drawTitle() {
    float a = titleAnim;
    DrawRectangleGradientV(0, 0, VW, VH, rgba(6, 8, 24, 120), rgba(6, 8, 24, 210));
    float bob = std::sin(rtime * 1.3f) * 4.f;
    const std::string t1 = "Brzozowa Saga", t2 = "~ Pieczęć Świtu ~";
    float w1 = g_text.measure(t1, 72), w2 = g_text.measure(t2, 28);
    g_text.draw(t1, VW * 0.5f - w1 * 0.5f + 3, 150 + bob + 4, 72, rgba(0, 0, 0), a * 0.5f, false);
    g_text.draw(t1, VW * 0.5f - w1 * 0.5f, 150 + bob, 72, rgba(255, 236, 170), a);
    g_text.draw(t2, VW * 0.5f - w2 * 0.5f, 240 + bob, 28, rgba(190, 220, 255), a);
    if (a < 0.6f) return;
    Rectangle win = rect(VW * 0.5f - 132, 380, 264, TITLE_ITEMS * 44 + 24);
    drawWindow(win);
    const char* opts[TITLE_ITEMS] = {"Nowa gra", "Kontynuuj",
                                     fullscreen ? "Pełny ekran: Wł." : "Pełny ekran: Wył.", "Wyjście"};
    bool hasSave = FileExists(SAVE_FILE);
    for (int i = 0; i < TITLE_ITEMS; ++i) {
        Rectangle row = titleRow(i);
        if (i == titleCur) drawCursorRect(row);
        float tw = g_text.measure(opts[i], 24);
        g_text.draw(opts[i], row.x + row.width * 0.5f - tw * 0.5f, row.y + 7, 24, (i == 1 && !hasSave) ? rgba(130, 130, 145) : WHITE);
    }
    const std::string foot = "C++ · raylib · stały krok logiki 60 Hz z interpolacją";
    g_text.draw(foot, VW * 0.5f - g_text.measure(foot, 15) * 0.5f, VH - 26, 15, rgba(170, 176, 200), 0.8f);
}

}  // namespace rpg
