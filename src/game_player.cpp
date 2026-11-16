// game_player.cpp — ruch gracza, pathfinding, NPC, płyty, autorun
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "game.h"

namespace rpg {

Vector2 Game::camFor(float tx, float ty) const {
    float cx = tx * TS + TS * 0.5f, cy = ty * TS + TS * 0.5f;
    float mw = (float)W * TS, mh = (float)H * TS;
    cx = (mw <= VW) ? mw * 0.5f : clampf(cx, VW * 0.5f, mw - VW * 0.5f);
    cy = (mh <= VH) ? mh * 0.5f : clampf(cy, VH * 0.5f, mh - VH * 0.5f);
    return v2(cx, cy);
}

Vector2 Game::focusTile(float alpha) const {
    if (title) return v2(W * 0.5f + std::sin(rtime * 0.11f) * 9.f, H * 0.5f - 1.f);
    return v2(hero.ix(alpha), hero.iy(alpha));
}

PathRes Game::pathStep(int gx, int gy) {
    PathRes res;
    if (hero.x == gx && hero.y == gy) { res.arrived = true; return res; }
    const int N = W * H, s = hero.y * W + hero.x;
    std::vector<int> visited(N, 0), first(N, -1), q;
    q.reserve(N); q.push_back(s); visited[s] = 1;
    int best = s, bestH = std::abs(gx - hero.x) + std::abs(gy - hero.y);
    for (size_t qi = 0; qi < q.size(); ++qi) {
        int cur = q[qi], cx = cur % W, cy = cur / W;
        for (int d = 0; d < 4; ++d) {
            int nx = cx + DX[d], ny = cy + DY[d];
            if (!inMap(nx, ny)) continue;
            bool pass = canPass(hero, cx, cy, d);
            if (nx == gx && ny == gy && !pass) {        // cel zablokowany (np. NPC) – stań obok
                if (cur == s) { res.arrived = true; res.face = d; return res; }
                res.dir = first[cur]; return res;
            }
            int ni = ny * W + nx;
            if (visited[ni] || !pass) continue;
            visited[ni] = 1;
            first[ni] = cur == s ? d : first[cur];
            if (nx == gx && ny == gy) { res.dir = first[ni]; return res; }
            int h = std::abs(gx - nx) + std::abs(gy - ny);
            if (h < bestH) { bestH = h; best = ni; }
            q.push_back(ni);
        }
    }
    if (best != s) res.dir = first[best]; else res.arrived = true;
    return res;
}

bool Game::tryAction() {
    for (auto& e : events)   // zdarzenia pod/nad graczem
        if (e.pg && e.x == hero.x && e.y == hero.y && e.pg->prio != Prio::Same &&
            (e.pg->trig == Trig::Action)) { startEvent(e, true); return true; }
    int fx = hero.x + DX[hero.dir], fy = hero.y + DY[hero.dir];
    for (auto& e : events)   // zdarzenia przed graczem
        if (e.pg && e.x == fx && e.y == fy && e.pg->prio == Prio::Same &&
            (e.pg->trig == Trig::Action || e.pg->trig == Trig::Touch)) { startEvent(e, true); return true; }
    return false;
}

void Game::onHeroArrive() {
    pushing = false; hero.speed = WALK_SPEED;
    if (destX == hero.x && destY == hero.y) destX = -1;
    for (auto& e : events)
        if (e.pg && e.x == hero.x && e.y == hero.y && e.pg->prio != Prio::Same && e.pg->trig == Trig::Touch) {
            startEvent(e, false); return;
        }
}

bool Game::tryHeroMove(float dt) {
    int d = in.dir4();
    bool fromPath = false;
    if (d >= 0) destX = -1;
    else if (destX >= 0) {
        PathRes r = pathStep(destX, destY);
        if (r.arrived) {
            destX = -1;
            if (r.face >= 0) { hero.dir = r.face; tryAction(); }
            return false;
        }
        if (r.dir < 0) { destX = -1; return false; }
        d = r.dir; fromPath = true;
    }
    if (d < 0) { pushT = 0; lastPushDir = -1; return false; }
    if (canPass(hero, hero.x, hero.y, d)) {
        hero.speed = in.dash ? DASH_SPEED : WALK_SPEED;
        hero.begin(d); pushT = 0; lastPushDir = -1;
        return true;
    }
    hero.dir = d;   // zablokowany -> obrót w miejscu (jak w MV)
    if (fromPath) { destX = -1; return false; }
    Event* e = sameAt(hero.x + DX[d], hero.y + DY[d]);
    if (e && e->pg->trig == Trig::Touch) { startEvent(*e, true); return false; }
    if (e && e->pg->pushable) {
        pushT += dt;
        if (pushT >= 0.16f || lastPushDir == d) {
            pushT = 0;
            if (canBoulderMove(*e, d)) {
                e->speed = PUSH_SPEED; e->begin(d);
                hero.speed = PUSH_SPEED; hero.begin(d);
                pushing = true; lastPushDir = d;
                g_audio.play(SE_PUSH, g_rng.rangef(0.9f, 1.1f));
                return true;
            }
            if (lastPushDir != -2) g_audio.play(SE_PLATE, 0.6f, 0.5f);
            lastPushDir = -2;
        }
        return false;
    }
    pushT = 0;
    return false;
}

void Game::updateHero(float dt) {
    if (canControl() && in.mouse) {
        Vector2 ct = camFor(hero.rx, hero.ry);
        float wx = in.mouseV.x - VW * 0.5f - shakeX + ct.x, wy = in.mouseV.y - VH * 0.5f + ct.y;
        int tx = (int)std::floor(wx / TS), ty = (int)std::floor(wy / TS);
        if (inMap(tx, ty) && in.mouseV.x >= 0 && in.mouseV.y >= 0 && in.mouseV.x < VW && in.mouseV.y < VH) {
            if (tx == hero.x && ty == hero.y) destX = -1; else { destX = tx; destY = ty; }
        }
        in.mouse = false;
    }
    if (hero.moving() && !pushing) hero.speed = in.dash ? DASH_SPEED : WALK_SPEED;
    float t = dt;
    for (int guard = 0; guard < 4 && t > 0.f; ++guard) {
        if (hero.moving()) {
            t = hero.advance(t);
            if (hero.moving()) break;
            onHeroArrive();
        }
        if (!canControl() || !tryHeroMove(dt)) break;
    }
    if (!hero.moving() && canControl()) {
        if (in.ok) { in.ok = false; if (tryAction()) destX = -1; }
        else if (in.cancel) { in.cancel = false; menuOpen = true; menuCur = 0; menuAnim = 0; g_audio.play(SE_OK); }
    }
}

void Game::updateEvents(float dt) {
    for (auto& e : events) {
        e.anim += dt;
        if (!e.pg) continue;
        if (e.moving()) { e.advance(dt); continue; }
        if (e.locked || e.pg->move != Move::Random) continue;
        e.moveTimer -= dt;
        if (e.moveTimer > 0) continue;
        e.moveTimer = g_rng.rangef(0.9f, 2.8f);
        int d = g_rng.range(0, 3);
        int nx = e.x + DX[d], ny = e.y + DY[d];
        if (std::abs(nx - e.homeX) + std::abs(ny - e.homeY) > 3) d = dirTowards(e.x, e.y, e.homeX, e.homeY);
        if (canPass(e, e.x, e.y, d)) { e.speed = e.pg->speed; e.begin(d); }
        else e.dir = d;
    }
}

void Game::updatePlates() {
    for (auto& p : events) {
        if (!p.pg || p.pg->plateSw < 0) continue;
        bool occ = !title && nearTile(hero, p.x, p.y);
        for (const auto& o : events)
            if (!occ && o.pg && o.pg->prio == Prio::Same && (o.pg->pushable || o.pg->gfx == G::Char) && nearTile(o, p.x, p.y)) occ = true;
        if (occ != p.pressed) {
            p.pressed = occ; sw[p.pg->plateSw] = occ;
            g_audio.play(SE_PLATE, occ ? 1.0f : 0.8f);
        }
    }
}

void Game::checkAutorun() {
    if (title || interp.running() || msg.active || fadeState != 0 || menuOpen) return;
    for (auto& e : events) if (e.pg && e.pg->trig == Trig::Autorun) { startEvent(e, false); return; }
}

}  // namespace rpg
