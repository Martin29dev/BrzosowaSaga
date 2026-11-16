// game_interpreter.cpp — interpreter poleceń zdarzeń i okno wiadomości
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "game.h"

namespace rpg {

void Game::startEvent(Event& e, bool fromPlayer) {
    if (!e.pg || e.pg->list.empty()) return;
    interp = Interp();
    interp.list = &e.pg->list; interp.eventId = e.id;
    destX = -1;
    if (fromPlayer) {
        e.locked = true; e.prevDir = e.dir;
        if (e.pg->gfx == G::Char && !e.pg->dirFix) e.dir = dirTowards(e.x, e.y, hero.x, hero.y);
    }
}

void Game::endInterp() {
    if (Event* e = findEvent(interp.eventId)) {
        if (e->locked) { e->locked = false; if (e->pg && e->pg->gfx == G::Char) e->dir = e->prevDir; }
    }
    interp = Interp();
}

void Game::jump(const std::string& label) {
    const auto& L = *interp.list;
    for (int i = 0; i < (int)L.size(); ++i)
        if (L[i].type == C::Label && L[i].s1 == label) { interp.ip = i + 1; return; }
    interp.ip = (int)L.size();
}

void Game::updateInterp(float dt) {
    if (!interp.running()) return;
    if (interp.wait > 0) { interp.wait -= dt; if (interp.wait > 0) return; interp.wait = 0; }
    if (msg.active || fadeState != 0) return;
    for (int guard = 0; guard < 1000 && interp.running(); ++guard) {
        const auto& L = *interp.list;
        if (interp.ip >= (int)L.size()) { endInterp(); return; }
        const Cmd& c = L[interp.ip++];
        switch (c.type) {
            case C::Text: startMessage(c.s1, c.s2, c.opts, c.a); return;
            case C::IfSw: if (sw[c.a] != (c.b != 0)) jump(c.s1); break;
            case C::IfSelf: if (getSelf(interp.eventId, c.a) != (c.b != 0)) jump(c.s1); break;
            case C::IfVar: {
                int v = var[c.a]; bool ok = false;
                switch ((Op)c.b) { case Op::EQ: ok = v == c.c; break; case Op::GE: ok = v >= c.c; break; case Op::LT: ok = v < c.c; break; }
                if (!ok) jump(c.s1);
            } break;
            case C::IfItem: if (items[c.a] < c.b) jump(c.s1); break;
            case C::Goto: jump(c.s1); break;
            case C::Label: break;
            case C::Sw: sw[c.a] = c.b != 0; refreshEvents(true); break;
            case C::Self: setSelf(interp.eventId, c.a, c.b != 0); refreshEvents(true); break;
            case C::Var: var[c.a] = c.b; refreshEvents(true); break;
            case C::VarAdd: var[c.a] += c.b; refreshEvents(true); break;
            case C::Item: items[c.a] = std::max(0, items[c.a] + c.b); refreshEvents(true); break;
            case C::Transfer: startTransfer(c.a, c.b, -1, -1, c.c); return;
            case C::Wait: interp.wait = c.f; return;
            case C::Shake: shakePow = (float)c.a; shakeDur = shakeT = c.f; break;
            case C::Flash: flashCol = c.col; flashDur = flashT = c.f; break;
            case C::Se: g_audio.play(c.a); break;
        }
    }
}

Rectangle Game::choiceRect() const {
    float w = 0;
    for (const auto& o : msg.choices) w = std::max(w, g_text.measure(o, 22));
    w += 64;
    float h = msg.choices.size() * 38.f + 20.f;
    return rect(VW - 12 - w, msgRect().y - h - 6, w, h);
}

void Game::startMessage(const std::string& name, const std::string& text, const std::vector<std::string>& opts, int v) {
    msg.active = true; msg.done = false; msg.choosing = false;
    msg.name = name;
    msg.glyphs = g_text.layout(text, 24, msgRect().width - 48, 34);
    msg.shown = 0; msg.blip = 0;
    msg.choices = opts; msg.choiceVar = v; msg.cursor = 0;
}

void Game::chooseOption(int i, bool cancel) {
    if (msg.choiceVar >= 0) var[msg.choiceVar] = i;
    g_audio.play(cancel ? SE_CANCEL : SE_OK);
    msg.choosing = false; msg.active = false;
}

void Game::updateMessage(float dt) {
    msg.open = msg.active ? std::min(1.f, msg.open + dt * 7.5f) : std::max(0.f, msg.open - dt * 7.5f);
    if (!msg.active || msg.open < 1.f) return;
    const float n = (float)msg.glyphs.size();
    if (!msg.done) {
        if (in.ok || in.mouse) { msg.shown = n; in.ok = in.mouse = false; }
        else {
            float before = msg.shown;
            msg.shown += (in.okHeld ? 260.f : 55.f) * dt;
            msg.blip += msg.shown - before;
            if (msg.blip >= 2.5f) { msg.blip = 0; g_audio.play(SE_BLIP, g_rng.rangef(0.94f, 1.06f)); }
        }
        if (msg.shown >= n) {
            msg.shown = n; msg.done = true;
            if (!msg.choices.empty()) { msg.choosing = true; msg.cursor = 0; }
        }
        return;
    }
    if (msg.choosing) {
        int cnt = (int)msg.choices.size();
        if (in.navUp) { msg.cursor = (msg.cursor + cnt - 1) % cnt; g_audio.play(SE_CURSOR); }
        if (in.navDown) { msg.cursor = (msg.cursor + 1) % cnt; g_audio.play(SE_CURSOR); }
        for (int i = 0; i < cnt; ++i) {
            if (!CheckCollisionPointRec(in.mouseV, choiceRow(i))) continue;
            if (in.mouseMoved && msg.cursor != i) { msg.cursor = i; g_audio.play(SE_CURSOR); }
            if (in.mouse) { in.mouse = in.ok = false; chooseOption(i, false); return; }
        }
        if (in.ok) { in.ok = in.mouse = false; chooseOption(msg.cursor, false); }
        else if (in.cancel) { in.cancel = false; chooseOption(cnt - 1, true); }
        in.mouse = false;
        return;
    }
    if (in.ok || in.mouse) { in.ok = in.mouse = false; msg.active = false; }
}

}  // namespace rpg
