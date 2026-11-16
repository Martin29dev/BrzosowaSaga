// game_world.cpp — ładowanie map, strony zdarzeń, kolizje
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "game.h"

namespace rpg {

void Game::init() {
    maps = buildMaps();
    screenRT = LoadRenderTexture(VW * SS, VH * SS);
    SetTextureFilter(screenRT.texture, TEXTURE_FILTER_BILINEAR);
    lightRT = LoadRenderTexture(VW, VH);
    SetTextureFilter(lightRT.texture, TEXTURE_FILTER_BILINEAR);
    loadMap(0);
    placeAtSpot('@', DIR_DOWN);
    fadeA = 1; fadeState = 2;
    bannerT = 0;
    loadSettings();   // pełny ekran / limit FPS z poprzedniej sesji
}

void Game::shutdown() {
    UnloadRenderTexture(screenRT); UnloadRenderTexture(lightRT);
    if (baked) { UnloadRenderTexture(baseRT); UnloadRenderTexture(overRT); }
}

void Game::placeAtSpot(char s, int dir) {
    auto it = spots.find(s);
    if (it != spots.end()) hero.place(it->second.first, it->second.second);
    hero.dir = dir;
}

void Game::resetState() {
    for (int i = 0; i < MAX_SW; ++i) sw[i] = false;
    for (int i = 0; i < MAX_VAR; ++i) var[i] = 0;
    for (int i = 0; i < IT_COUNT; ++i) items[i] = 0;
    selfSw.clear(); playTime = 0; interp = Interp(); msg = Message();
}

void Game::loadMap(int id) {
    mapId = id;
    const MapDef& md = maps[id];
    H = (int)md.rows.size(); W = 0;
    for (const auto& r : md.rows) W = std::max(W, (int)r.size());
    tiles.assign((size_t)W * H, md.defGround);
    events.clear(); spots.clear();
    auto raw = [&](int x, int y) -> char {
        if (x < 0 || y < 0 || x >= W || y >= H) return 0;
        const std::string& r = md.rows[y];
        return x < (int)r.size() ? r[x] : md.defGround;
    };
    // podłoże pod znacznikiem: najczęstszy PRZECHODNI sąsiad
    auto guessGround = [&](int x, int y) -> char {
        char cand[4]; int cnt[4]; int n = 0;
        const int order[4] = {DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN};
        for (int k = 0; k < 4; ++k) {
            char c = raw(x + DX[order[k]], y + DY[order[k]]);
            if (!c || isMarker(c) || !passChar(c)) continue;
            int j = 0; while (j < n && cand[j] != c) ++j;
            if (j == n) { cand[n] = c; cnt[n] = 0; ++n; }
            cnt[j]++;
        }
        if (n == 0) return md.defGround;
        int best = 0; for (int j = 1; j < n; ++j) if (cnt[j] > cnt[best]) best = j;
        return cand[best];
    };
    std::map<char, int> used;
    int nextId = 1;
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            char c = raw(x, y);
            char& t = tiles[(size_t)y * W + x];
            if (c >= 'a' && c <= 'z') {
                std::vector<const EventDef*> list;
                for (const auto& d : md.events) if (d.marker == c) list.push_back(&d);
                if (list.empty()) { TraceLog(LOG_WARNING, "MAPA: brak definicji zdarzenia '%c'", c); t = guessGround(x, y); continue; }
                int k = used[c]++;
                const EventDef* d = list[std::min(k, (int)list.size() - 1)];
                t = d->ground ? d->ground : guessGround(x, y);
                Event e; e.id = nextId++; e.def = d; e.place(x, y); e.homeX = x; e.homeY = y;
                e.moveTimer = g_rng.rangef(0.5f, 2.0f);
                events.push_back(e);
            } else if (isMarker(c)) {
                spots[c] = std::make_pair(x, y);
                t = guessGround(x, y);
            } else t = c;
        }
    refreshEvents(false);
    for (auto& e : events) if (e.pg && e.pg->plateSw >= 0) sw[e.pg->plateSw] = false;
    refreshEvents(false);
    bake();
    parts.clear(); spawnAcc = 0;
    destX = -1; pushing = false; pushT = 0;
    bannerT = 3.4f;
}

void Game::refreshEvents(bool sounds) {
    for (auto& e : events) {
        const auto& P = e.def->pages;
        int best = -1;
        for (int i = (int)P.size() - 1; i >= 0; --i) if (pageOk(e, P[i])) { best = i; break; }
        if (best == e.pageIdx) continue;
        bool first = e.pageIdx == -2;
        e.pageIdx = best;
        e.pg = best >= 0 ? &P[best] : nullptr;
        if (e.pg) {
            if (e.pg->dir >= 0 && !e.locked) e.dir = e.pg->dir;
            e.speed = e.pg->speed;
            if (!first && sounds && e.pg->enterSe >= 0) g_audio.play(e.pg->enterSe);
        }
    }
}

bool Game::pageOk(const Event& e, const Page& p) const {
    for (int s : p.condSw) if (!sw[s]) return false;
    if (p.condSelf >= 0 && !getSelf(e.id, p.condSelf)) return false;
    if (p.condVar >= 0 && var[p.condVar] < p.condVarMin) return false;
    return true;
}

bool Game::bridgeAt(int x, int y) const {
    for (const auto& e : events) if (e.pg && e.pg->bridge && e.x == x && e.y == y) return true;
    return false;
}

bool Game::charBlocked(const Character* self, int x, int y) const {
    for (const auto& e : events) {
        if (&e == self || !e.pg || e.pg->through || e.pg->prio != Prio::Same) continue;
        if (e.x == x && e.y == y) return true;
    }
    if (self != &hero && hero.x == x && hero.y == y) return true;
    return false;
}

bool Game::canPass(const Character& c, int x, int y, int d) const {
    int nx = x + DX[d], ny = y + DY[d];
    if (!inMap(nx, ny) || !tilePass(nx, ny)) return false;
    return !charBlocked(&c, nx, ny);
}

bool Game::canBoulderMove(const Event& b, int d) const {
    int nx = b.x + DX[d], ny = b.y + DY[d];
    if (!inMap(nx, ny) || !tilePass(nx, ny) || charBlocked(&b, nx, ny)) return false;
    for (const auto& e : events)
        if (e.pg && e.x == nx && e.y == ny && e.pg->prio != Prio::Same && e.pg->plateSw < 0 &&
            !e.pg->bridge && e.pg->gfx != G::None) return false;   // nie na drzwi / monety / schody
    return true;
}

Event* Game::sameAt(int x, int y) {
    for (auto& e : events) if (e.pg && e.pg->prio == Prio::Same && e.x == x && e.y == y) return &e;
    return nullptr;
}

int Game::dirTowards(int fx, int fy, int tx, int ty) {
    int dx = tx - fx, dy = ty - fy;
    if (std::abs(dx) > std::abs(dy)) return dx < 0 ? DIR_LEFT : DIR_RIGHT;
    return dy < 0 ? DIR_UP : DIR_DOWN;
}

bool Game::nearTile(const Character& c, int x, int y) {
    return c.x == x && c.y == y && std::fabs(c.rx - x) + std::fabs(c.ry - y) < 0.5f;
}

}  // namespace rpg
