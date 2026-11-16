// game_flow.cpp — przejścia, zapis/odczyt, menu, tytuł, efekty, krok logiki
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "game.h"

#include <fstream>

namespace rpg {

void Game::startTransfer(int map, int spot, int tx, int ty, int dir) {
    pend = Pending();
    pend.map = map < 0 ? mapId : map; pend.spot = spot; pend.x = tx; pend.y = ty; pend.dir = dir;
    fadeState = 1; destX = -1;
}

void Game::performTransfer() {
    if (pend.toTitle) {
        title = true; titleCur = 0; menuOpen = false; resetState();
        loadMap(0); placeAtSpot('@', DIR_DOWN); bannerT = 0; return;
    }
    if (pend.newGame) {
        resetState(); title = false;
        loadMap(0); placeAtSpot('@', DIR_DOWN); return;
    }
    if (pend.load) {
        const SaveData& s = loaded;
        for (int i = 0; i < MAX_SW; ++i) sw[i] = s.sw[i];
        for (int i = 0; i < MAX_VAR; ++i) var[i] = s.var[i];
        for (int i = 0; i < IT_COUNT; ++i) items[i] = s.items[i];
        selfSw = s.self; playTime = s.playTime;
        interp = Interp(); msg = Message(); title = false; menuOpen = false;
    }
    int x = hero.x, y = hero.y, d = pend.dir >= 0 ? pend.dir : hero.dir;
    loadMap(pend.map);
    if (pend.spot >= 0) {
        auto it = spots.find((char)pend.spot);
        if (it != spots.end()) { x = it->second.first; y = it->second.second; }
    } else if (pend.x >= 0) { x = pend.x; y = pend.y; }
    hero.place(x, y); hero.dir = d;
    interp.eventId = -1;
}

void Game::updateFade(float dt) {
    if (fadeState == 1) {
        fadeA += dt / 0.3f;
        if (fadeA >= 1.f) { fadeA = 1.f; performTransfer(); fadeState = 2; }
    } else if (fadeState == 2) {
        fadeA -= dt / 0.3f;
        if (fadeA <= 0.f) { fadeA = 0.f; fadeState = 0; }
    }
}

void Game::saveGame() {
    std::ofstream f(SAVE_FILE);
    if (!f) { toast("Nie udało się zapisać gry!"); g_audio.play(SE_BUZZER); return; }
    f << "BRZOZOWA 1\n" << mapId << ' ' << hero.x << ' ' << hero.y << ' ' << hero.dir << ' ' << playTime << '\n';
    for (int i = 0; i < MAX_SW; ++i) f << (sw[i] ? 1 : 0) << ' ';
    f << '\n';
    for (int i = 0; i < MAX_VAR; ++i) f << var[i] << ' ';
    f << '\n';
    for (int i = 0; i < IT_COUNT; ++i) f << items[i] << ' ';
    f << '\n' << selfSw.size();
    for (long long k : selfSw) f << ' ' << k;
    f << '\n';
    g_audio.play(SE_SAVE);
    toast("Zapisano grę.");
}

bool Game::loadGame() {
    std::ifstream f(SAVE_FILE);
    std::string magic; int ver = 0;
    if (!f || !(f >> magic >> ver) || magic != "BRZOZOWA") { toast("Brak zapisu."); g_audio.play(SE_BUZZER); return false; }
    SaveData s;
    f >> s.map >> s.x >> s.y >> s.dir >> s.playTime;
    for (int i = 0; i < MAX_SW; ++i) { int v = 0; f >> v; s.sw[i] = v != 0; }
    for (int i = 0; i < MAX_VAR; ++i) f >> s.var[i];
    for (int i = 0; i < IT_COUNT; ++i) f >> s.items[i];
    size_t n = 0; f >> n;
    for (size_t i = 0; i < n && f; ++i) { long long k; f >> k; s.self.insert(k); }
    if (!f || s.map < 0 || s.map >= (int)maps.size()) { toast("Zapis jest uszkodzony."); g_audio.play(SE_BUZZER); return false; }
    loaded = s;
    startTransfer(s.map, -1, s.x, s.y, s.dir);
    pend.load = true;
    g_audio.play(SE_SAVE);
    toast("Wczytano grę.");
    return true;
}

void Game::updateMenu(float dt) {
    menuAnim = std::min(1.f, menuAnim + dt * 7.5f);
    const int N = MENU_ITEMS;
    if (in.navUp) { menuCur = (menuCur + N - 1) % N; g_audio.play(SE_CURSOR); }
    if (in.navDown) { menuCur = (menuCur + 1) % N; g_audio.play(SE_CURSOR); }
    bool click = false;
    for (int i = 0; i < N; ++i) if (CheckCollisionPointRec(in.mouseV, menuCmdRow(i))) {
        if (in.mouseMoved && menuCur != i) { menuCur = i; g_audio.play(SE_CURSOR); }
        if (in.mouse) { menuCur = i; click = true; }
    }
    if (in.cancel) { menuOpen = false; g_audio.play(SE_CANCEL); return; }
    if (in.ok || click) {
        switch (menuCur) {
            case 0: menuOpen = false; g_audio.play(SE_CANCEL); break;
            case 1: saveGame(); break;
            case 2: if (loadGame()) menuOpen = false; break;
            case 3: g_audio.play(SE_OK); toggleFullscreen(); break;
            case 4: g_audio.play(SE_OK); startTransfer(0, -1, -1, -1, -1); pend.toTitle = true; menuOpen = false; break;
        }
    }
}

void Game::updateTitle(float dt) {
    titleAnim = std::min(1.f, titleAnim + dt * 1.5f);
    if (fadeState != 0) return;
    const int N = TITLE_ITEMS;
    if (in.navUp) { titleCur = (titleCur + N - 1) % N; g_audio.play(SE_CURSOR); }
    if (in.navDown) { titleCur = (titleCur + 1) % N; g_audio.play(SE_CURSOR); }
    bool click = false;
    for (int i = 0; i < N; ++i) if (CheckCollisionPointRec(in.mouseV, titleRow(i))) {
        if (in.mouseMoved && titleCur != i) { titleCur = i; g_audio.play(SE_CURSOR); }
        if (in.mouse) { titleCur = i; click = true; }
    }
    if (in.ok || click) {
        if (titleCur == 0) { g_audio.play(SE_OK); startTransfer(0, -1, -1, -1, -1); pend.newGame = true; }
        else if (titleCur == 1) { loadGame(); }
        else if (titleCur == 2) { g_audio.play(SE_OK); toggleFullscreen(); }
        else quit = true;
    }
}

void Game::updateTimers(float dt) {
    if (bannerT > 0) bannerT -= dt;
    if (toastT > 0) toastT -= dt;
    if (flashT > 0) flashT -= dt;
    if (shakeT > 0) {
        shakeT -= dt;
        float k = clampf(shakeT / shakeDur, 0.f, 1.f);
        shakeX = std::sin((float)time * 52.f) * shakePow * k;
    } else shakeX = 0;
}

void Game::updateParticles(float dt) {
    const MapDef& md = maps[mapId];
    Vector2 f = title ? focusTile(1.f) : v2(hero.rx, hero.ry);
    Vector2 ct = camFor(f.x, f.y);
    float left = ct.x - VW * 0.5f, top = ct.y - VH * 0.5f;
    spawnAcc += dt * (md.weather == 1 ? 3.5f : md.weather == 2 ? 8.f : 0.f);
    while (spawnAcc >= 1.f && parts.size() < 140) {
        spawnAcc -= 1.f;
        Particle p;
        if (md.weather == 1) {
            p.kind = 0; p.x = left + g_rng.rangef(-80, (float)VW); p.y = top + g_rng.rangef(-20, VH * 0.8f);
            p.vx = g_rng.rangef(10, 26); p.vy = g_rng.rangef(14, 30); p.size = g_rng.rangef(3, 5);
            p.col = g_rng.f() < 0.6f ? rgba(236, 206, 90) : rgba(150, 200, 90);
            p.life = p.max = g_rng.rangef(5, 9);
        } else {
            p.kind = 1; p.x = left + g_rng.rangef(0, (float)VW); p.y = top + g_rng.rangef(0, (float)VH);
            p.vx = g_rng.rangef(-5, 5); p.vy = g_rng.rangef(-9, -2); p.size = g_rng.rangef(1.2f, 2.4f);
            p.col = rgba(255, 222, 170); p.life = p.max = g_rng.rangef(3, 6);
        }
        p.rot = g_rng.rangef(0, 360);
        parts.push_back(p);
    }
    if (spawnAcc > 1.f) spawnAcc = 0.f;
    for (auto& p : parts) {
        p.life -= dt;
        p.x += (p.vx + (p.kind == 0 ? std::sin((float)time * 2.f + p.rot) * 14.f : 0.f)) * dt;
        p.y += p.vy * dt;
        p.rot += dt * 110.f;
    }
    parts.erase(std::remove_if(parts.begin(), parts.end(), [](const Particle& p) { return p.life <= 0; }), parts.end());
}

void Game::step(float dt) {
    time += dt;
    in.logic(dt);
    hero.savePrev();
    for (auto& e : events) e.savePrev();
    updateTimers(dt);
    updateFade(dt);
    if (title) {
        updateTitle(dt); updateEvents(dt); updatePlates(); refreshEvents(false); updateParticles(dt);
        return;
    }
    playTime += dt;
    if (in.f5 && canControl()) saveGame();
    if (in.f9 && fadeState == 0 && !interp.running() && !menuOpen) loadGame();
    if (menuOpen) { updateMenu(dt); return; }
    updateMessage(dt);
    updateInterp(dt);
    refreshEvents(true);
    updateHero(dt);
    updateEvents(dt);
    updatePlates();
    refreshEvents(true);
    checkAutorun();
    updateParticles(dt);
}

void Game::pollInput() {
    float sw_ = (float)GetScreenWidth(), sh_ = (float)GetScreenHeight();
    float sc = std::min(sw_ / VW, sh_ / VH);
    Vector2 m = GetMousePosition();
    Vector2 vm = v2((m.x - (sw_ - VW * sc) * 0.5f) / sc, (m.y - (sh_ - VH * sc) * 0.5f) / sc);
    in.poll(vm);
    if (in.hit(KEY_F1)) debug = !debug;
    if (in.hit(KEY_F2)) cycleFps();
    if (in.hit(KEY_F11) || ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && in.hit(KEY_ENTER))) toggleFullscreen();
}

void Game::cycleFps() {
    static const char* names[4] = {"VSync", "30 FPS", "144 FPS", "bez limitu"};
    applyFps((fpsMode + 1) % 4);
    saveSettings();
    toast(std::string("Limit klatek: ") + names[fpsMode] + " — prędkość gry bez zmian");
}

void Game::applyFps(int mode) {
    fpsMode = mode;
    switch (fpsMode) {
        case 1: ClearWindowState(FLAG_VSYNC_HINT); SetTargetFPS(30); break;
        case 2: ClearWindowState(FLAG_VSYNC_HINT); SetTargetFPS(144); break;
        case 3: ClearWindowState(FLAG_VSYNC_HINT); SetTargetFPS(0); break;
        default: fpsMode = 0; SetWindowState(FLAG_VSYNC_HINT); SetTargetFPS(0); break;
    }
}

// Pełny ekran = "borderless windowed": okno bez ramki na cały monitor.
// Brak zmiany trybu graficznego -> szybkie przełączanie i bezproblemowy Alt+Tab.
// Obraz jest skalowany z zachowaniem proporcji (letterbox) w Game::render().
void Game::setFullscreen(bool on, bool notify) {
    bool now = IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
    if (on != now) ToggleBorderlessWindowed();
    fullscreen = IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
    if (notify) {
        saveSettings();
        toast(fullscreen ? "Pełny ekran: włączony (F11)" : "Pełny ekran: wyłączony (F11)");
    }
}

void Game::toggleFullscreen() { setFullscreen(!fullscreen, true); }

void Game::loadSettings() {
    std::ifstream f(SETTINGS_FILE);
    std::string k; int v = 0; int fs = 0, fps = 0;
    while (f >> k >> v) {
        if (k == "fullscreen") fs = v;
        else if (k == "fps") fps = v;
    }
    applyFps(fps);
    if (fs) setFullscreen(true, false);
}

void Game::saveSettings() const {
    std::ofstream f(SETTINGS_FILE);
    if (f) f << "fullscreen " << (fullscreen ? 1 : 0) << "\nfps " << fpsMode << "\n";
}

}  // namespace rpg
