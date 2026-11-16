// game.h — główna klasa gry (scena mapy, tytuł, menu)
// Brzozowa Saga (RPG Maker MV-like, raylib + C++17)
#pragma once
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "runtime.h"

namespace rpg {

// ================================================================ GRA ======
struct Game {
    // --- świat
    std::vector<MapDef> maps;
    int mapId = 0, W = 0, H = 0;
    std::vector<char> tiles;
    std::vector<Event> events;
    std::map<char, std::pair<int, int>> spots;
    Character hero;
    // --- stan gry
    bool sw[MAX_SW] = {};
    int var[MAX_VAR] = {};
    int items[IT_COUNT] = {};
    std::set<long long> selfSw;
    Interp interp;
    Message msg;
    Input in;
    // --- gracz
    int destX = -1, destY = -1;
    float pushT = 0; bool pushing = false; int lastPushDir = -1;
    // --- przejścia / efekty
    int fadeState = 0; float fadeA = 0;
    struct Pending { int map = 0, spot = -1, x = -1, y = -1, dir = -1; bool load = false, newGame = false, toTitle = false; } pend;
    SaveData loaded;
    float shakeT = 0, shakeDur = 1, shakePow = 0, shakeX = 0;
    float flashT = 0, flashDur = 1; Color flashCol{255, 255, 255, 255};
    float bannerT = 0;
    std::string toastText; float toastT = 0;
    std::vector<Particle> parts; float spawnAcc = 0;
    // --- UI
    bool title = true; int titleCur = 0; float titleAnim = 0;
    bool menuOpen = false; int menuCur = 0; float menuAnim = 0;
    // --- czas
    double time = 0, playTime = 0; float rtime = 0;
    // --- render
    RenderTexture2D screenRT{}, lightRT{}, baseRT{}, overRT{};
    bool baked = false;
    // --- meta
    bool debug = false, quit = false, fullscreen = false;
    int fpsMode = 0, lastSteps = 0, winW = VW, winH = VH;
    float lastFrame = 0;

    // =========================================================== init =====
    void init();
    void shutdown();
    void placeAtSpot(char s, int dir);
    void resetState();

    // ============================================================ mapa ====
    static bool passChar(char c) { return c == '.' || c == ':' || c == '=' || c == 'C'; }
    static bool wallChar(char c) { return c == '#' || c == '*' || c == 'X'; }
    static bool isMarker(char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '@'; }
    bool inMap(int x, int y) const { return x >= 0 && y >= 0 && x < W && y < H; }
    char tile(int x, int y) const { return inMap(x, y) ? tiles[y * W + x] : 'X'; }

    void loadMap(int id);

    // ========================================================= zdarzenia ===
    long long selfKey(int eid, int letter) const { return ((long long)mapId << 20) | ((long long)eid << 4) | letter; }
    bool getSelf(int eid, int l) const { return selfSw.count(selfKey(eid, l)) > 0; }
    void setSelf(int eid, int l, bool v) { if (eid < 0) return; if (v) selfSw.insert(selfKey(eid, l)); else selfSw.erase(selfKey(eid, l)); }

    bool pageOk(const Event& e, const Page& p) const;
    // jak w MV: aktywna jest strona o NAJWYŻSZYM numerze z spełnionymi warunkami
    void refreshEvents(bool sounds);
    Event* findEvent(int id) { for (auto& e : events) if (e.id == id) return &e; return nullptr; }

    bool bridgeAt(int x, int y) const;
    bool tilePass(int x, int y) const { return inMap(x, y) && (passChar(tile(x, y)) || bridgeAt(x, y)); }
    bool charBlocked(const Character* self, int x, int y) const;
    bool canPass(const Character& c, int x, int y, int d) const;
    bool canBoulderMove(const Event& b, int d) const;
    Event* sameAt(int x, int y);
    static int dirTowards(int fx, int fy, int tx, int ty);

    void startEvent(Event& e, bool fromPlayer);
    void endInterp();
    void jump(const std::string& label);

    // ========================================================= interpreter =
    void updateInterp(float dt);

    // ========================================================== wiadomości =
    Rectangle msgRect() const { return rect(8, VH - 8 - 176, VW - 16, 176); }
    Rectangle choiceRect() const;
    Rectangle choiceRow(int i) const { Rectangle cr = choiceRect(); return rect(cr.x + 8, cr.y + 10 + i * 38.f, cr.width - 16, 36); }

    void startMessage(const std::string& name, const std::string& text, const std::vector<std::string>& opts, int v);
    void chooseOption(int i, bool cancel);
    void updateMessage(float dt);

    // ========================================================= przejścia ===
    void startTransfer(int map, int spot, int tx, int ty, int dir);
    void performTransfer();
    void updateFade(float dt);

    // ======================================================== zapis/odczyt =
    void toast(const std::string& s) { toastText = s; toastT = 2.4f; }
    void saveGame();
    bool loadGame();

    // ======================================================== gracz =======
    bool canControl() const { return !title && !interp.running() && !msg.active && fadeState == 0 && !menuOpen; }

    Vector2 camFor(float tx, float ty) const;
    Vector2 focusTile(float alpha) const;

    // BFS (jak "touch to move" w MV): zwraca pierwszy krok do celu;
    // gdy cel nieosiągalny – idzie do najbliższego osiągalnego pola
    PathRes pathStep(int gx, int gy);

    bool tryAction();
    void onHeroArrive();
    bool tryHeroMove(float dt);
    void updateHero(float dt);

    void updateEvents(float dt);
    static bool nearTile(const Character& c, int x, int y);
    void updatePlates();
    void checkAutorun();

    // ===================================================== menu / tytuł ===
    Rectangle menuCmdRow(int i) const { return rect(24, 30 + i * 44.f, 216, 40); }
    void updateMenu(float dt);
    Rectangle titleRow(int i) const { return rect(VW * 0.5f - 120, 392 + i * 44.f, 240, 40); }
    void updateTitle(float dt);

    // ================================================== efekty/cząsteczki ==
    void updateTimers(float dt);
    void updateParticles(float dt);

    // ============================================================ STEP =====
    void step(float dt);

    // ============================================== RYSOWANIE KAFLI ======
    void drawGrass(float X, float Y, Rng& r);
    void drawTileBase(int x, int y);
    void drawTileOver(int x, int y);
    void bake();
    void drawAnimatedTile(int x, int y, float t);

    // ========================================== RYSOWANIE POSTACI ========
    void drawHumanoid(int pal, float X, float Y, int dir, float walk, float t, int seed);

    void drawEvent(const Event& e, float X, float Y, float t);

    // ================================================== OKNA / UI ========
    void drawWindow(Rectangle r, float open = 1.f);
    void drawCursorRect(Rectangle r);
    void drawMessage();
    void drawItemIcon(int it, float x, float y);
    void drawMenu();
    void drawTitle();

    // ================================================== RENDER ===========
    void renderLights(const Camera2D& cam1, float alpha);

    void drawWorld(float alpha);
    void drawParticles();
    void render(float alpha);

    // ================================================== META =============
    void pollInput();
    void cycleFps();
    void toggleFullscreen();
    void setFullscreen(bool on, bool notify);
    void applyFps(int mode);
    void loadSettings();
    void saveSettings() const;
};

}  // namespace rpg
