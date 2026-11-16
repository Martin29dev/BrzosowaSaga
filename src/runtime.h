// runtime.h — postacie, zdarzenia w grze, wejście, interpreter
// Brzozowa Saga (RPG Maker MV-like, raylib + C++17)
#pragma once
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "data.h"
#include "text.h"

namespace rpg {

// ============================================================ RUNTIME ======
struct Character {
    int x = 0, y = 0, dir = DIR_DOWN;
    float rx = 0, ry = 0, px = 0, py = 0;     // pozycja rzeczywista + poprzednia (interpolacja)
    float speed = WALK_SPEED, walk = 0, anim = 0;

    void place(int nx, int ny) { x = nx; y = ny; rx = px = (float)nx; ry = py = (float)ny; }
    bool moving() const { return rx != (float)x || ry != (float)y; }
    void savePrev() { px = rx; py = ry; }
    void begin(int d) { dir = d; x += DX[d]; y += DY[d]; }
    // przesuwa postać; zwraca niewykorzystany czas (do płynnego łączenia kroków)
    float advance(float dt) {
        if (!moving()) return dt;
        float dist = speed * dt;
        float ex = (float)x - rx, ey = (float)y - ry;
        float rem = std::fabs(ex) + std::fabs(ey);
        if (dist >= rem) {
            walk += rem; rx = (float)x; ry = (float)y;
            if (walk >= 2.f) walk = std::fmod(walk, 2.f);
            return (dist - rem) / speed;
        }
        walk += dist;
        rx += signf(ex) * dist; ry += signf(ey) * dist;
        return 0.f;
    }
    float ix(float a) const { return lerpf(px, rx, a); }
    float iy(float a) const { return lerpf(py, ry, a); }
};

struct Event : Character {
    int id = 0;
    const EventDef* def = nullptr;
    int pageIdx = -2;
    const Page* pg = nullptr;
    bool locked = false, pressed = false;
    int prevDir = DIR_DOWN, homeX = 0, homeY = 0;
    float moveTimer = 1.f;
};

struct Interp {
    const std::vector<Cmd>* list = nullptr;
    int ip = 0, eventId = -1;
    float wait = 0;
    bool running() const { return list != nullptr; }
};

struct Message {
    bool active = false, done = false, choosing = false;
    float open = 0, shown = 0, blip = 0;
    std::string name;
    std::vector<Glyph> glyphs;
    std::vector<std::string> choices;
    int choiceVar = -1, cursor = 0;
};

struct Particle { float x, y, vx, vy, life, max, size, rot; Color col; int kind; };

struct Input {
    bool ok = false, cancel = false, mouse = false, mouseMoved = false, f5 = false, f9 = false;
    bool navUp = false, navDown = false, okHeld = false, dash = false;
    bool held[4] = {false, false, false, false}, pressed[4] = {false, false, false, false};
    uint64_t stamp[4] = {0, 0, 0, 0}, counter = 0;
    Vector2 mouseV{0, 0}, lastMouse{0, 0};
    int repDir = -1; float repT = 0;

    // wywoływane co klatkę renderu: zdarzenia "wciśnięte" są zatrzaskiwane,
    // aby nie zginęły, gdy w danej klatce nie wykona się żaden krok logiki
    // Kolejka GetKeyPressed() łapie także "tapnięcia" krótsze niż jedna klatka
    // (IsKeyPressed porównuje tylko stan klatka-do-klatki i takie by zgubił).
    std::set<int> frameKeys;
    bool hit(int k) const { return frameKeys.count(k) > 0 || IsKeyPressed(k); }

    void poll(Vector2 vm) {
        frameKeys.clear();
        for (int k = GetKeyPressed(); k != 0; k = GetKeyPressed()) frameKeys.insert(k);
        ok |= hit(KEY_Z) || hit(KEY_ENTER) || hit(KEY_SPACE) || hit(KEY_KP_ENTER);
        cancel |= hit(KEY_X) || hit(KEY_ESCAPE) || hit(KEY_BACKSPACE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
        mouse |= IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        f5 |= hit(KEY_F5);
        f9 |= hit(KEY_F9);
        okHeld = IsKeyDown(KEY_Z) || IsKeyDown(KEY_ENTER) || IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        dash = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        static const int K[4][3] = {{KEY_DOWN, KEY_S, KEY_KP_2}, {KEY_LEFT, KEY_A, KEY_KP_4},
                                    {KEY_RIGHT, KEY_D, KEY_KP_6}, {KEY_UP, KEY_W, KEY_KP_8}};
        for (int d = 0; d < 4; ++d) {
            bool h = false, p = false;
            for (int k = 0; k < 3; ++k) { h = h || IsKeyDown(K[d][k]); p = p || hit(K[d][k]); }
            if (p || (h && !held[d])) stamp[d] = ++counter;
            if (p) pressed[d] = true;
            held[d] = h;
        }
        if (vm.x != lastMouse.x || vm.y != lastMouse.y) mouseMoved = true;
        lastMouse = vm; mouseV = vm;
    }
    // ostatnio wciśnięty kierunek ma priorytet (jak w MV)
    int dir4() const {
        int best = -1; uint64_t bs = 0;
        for (int d = 0; d < 4; ++d)
            if ((held[d] || pressed[d]) && stamp[d] > bs) { bs = stamp[d]; best = d; }
        return best;
    }
    void logic(float dt) {   // auto-powtarzanie w menu
        navUp = pressed[DIR_UP]; navDown = pressed[DIR_DOWN];
        int h = held[DIR_UP] ? DIR_UP : (held[DIR_DOWN] ? DIR_DOWN : -1);
        if (h != repDir) { repDir = h; repT = 0; }
        else if (h >= 0) {
            repT += dt;
            if (repT >= 0.32f) { repT -= 0.075f; if (h == DIR_UP) navUp = true; else navDown = true; }
        }
    }
    void consume() {
        ok = cancel = mouse = mouseMoved = f5 = f9 = false;
        for (int d = 0; d < 4; ++d) pressed[d] = false;
    }
};

struct SaveData {
    int map = 0, x = 0, y = 0, dir = 0;
    bool sw[MAX_SW] = {};
    int var[MAX_VAR] = {};
    int items[IT_COUNT] = {};
    std::set<long long> self;
    double playTime = 0;
};

struct PathRes { int dir = -1; bool arrived = false; int face = -1; };

}  // namespace rpg
