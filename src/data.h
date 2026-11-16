// data.h — definicje danych: zdarzenia, strony, polecenia, przedmioty
// Brzozowa Saga (RPG Maker MV-like, raylib + C++17)
#pragma once
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "common.h"
#include "audio.h"

namespace rpg {

// ======================================================== DANE GRY =========
enum class Trig { None, Action, Touch, Autorun };
enum class Prio { Below, Same, Above };
enum class Move { Fixed, Random };
enum class G { None, Char, Chest, ChestOpen, Sign, Door, DoorOpen, Gate, GateOpen, LeverOff, LeverOn,
               Plate, Boulder, Bridge, CrystalOff, CrystalOn, Coin, Statue, Well, Stairs, Seal, SealOpen };
enum PalId { PAL_HERO, PAL_ELDER, PAL_GIRL, PAL_FISHER, PAL_GHOST };
enum ItemId { IT_KEY, IT_COIN, IT_AMULET, IT_COUNT };
enum class Op { EQ, GE, LT };

struct ItemDef { const char* name; const char* desc; Color col; };
const ItemDef ITEMS[IT_COUNT] = {
    {"Stary klucz", "Zardzewiały klucz z wyrytym symbolem słońca.", {236, 196, 80, 255}},
    {"Moneta", "Stara srebrna moneta. Podobno przynosi szczęście.", {200, 210, 226, 255}},
    {"Amulet Świtu", "Pulsuje ciepłym światłem. Serce Pieczęci.", {255, 150, 70, 255}},
};

struct PalDef { Color hair, skin, body, legs, accent; int style; };
const PalDef PALS[] = {
    {{250, 214, 120, 255}, {255, 220, 186, 255}, {60, 108, 196, 255}, {70, 60, 86, 255}, {196, 52, 60, 255}, 0},
    {{236, 236, 240, 255}, {240, 204, 170, 255}, {118, 80, 160, 255}, {80, 60, 100, 255}, {230, 190, 80, 255}, 1},
    {{150, 82, 46, 255}, {255, 222, 190, 255}, {70, 160, 96, 255}, {110, 70, 50, 255}, {250, 240, 220, 255}, 2},
    {{90, 70, 60, 255}, {232, 190, 150, 255}, {80, 120, 150, 255}, {70, 70, 80, 255}, {200, 170, 100, 255}, 3},
    {{200, 230, 255, 255}, {214, 238, 255, 255}, {150, 200, 255, 255}, {150, 200, 255, 255}, {255, 255, 255, 255}, 4},
};

// ------------------------------------------------------------- polecenia --
enum class C { Text, IfSw, IfSelf, IfVar, IfItem, Goto, Label, Sw, Self, Var, VarAdd, Item,
               Transfer, Wait, Shake, Flash, Se };

struct Cmd {
    C type = C::Text;
    int a = 0, b = 0, c = 0;
    float f = 0;
    std::string s1, s2;
    std::vector<std::string> opts;
    Color col{255, 255, 255, 255};
};

namespace cmd {
inline Cmd mk(C t) { Cmd c; c.type = t; return c; }
inline Cmd Text(const std::string& name, const std::string& text) { Cmd c = mk(C::Text); c.s1 = name; c.s2 = text; c.a = -1; return c; }
inline Cmd Ask(const std::string& name, const std::string& text, std::vector<std::string> opts, int var) {
    Cmd c = mk(C::Text); c.s1 = name; c.s2 = text; c.opts = std::move(opts); c.a = var; return c;
}
// Warunki: jeśli warunek NIE jest spełniony -> skok do etykiety elseLabel
inline Cmd IfSw(int id, bool v, const std::string& elseL) { Cmd c = mk(C::IfSw); c.a = id; c.b = v; c.s1 = elseL; return c; }
inline Cmd IfSelf(char l, bool v, const std::string& elseL) { Cmd c = mk(C::IfSelf); c.a = l - 'A'; c.b = v; c.s1 = elseL; return c; }
inline Cmd IfVar(int id, Op op, int val, const std::string& elseL) { Cmd c = mk(C::IfVar); c.a = id; c.b = (int)op; c.c = val; c.s1 = elseL; return c; }
inline Cmd IfItem(int it, int n, const std::string& elseL) { Cmd c = mk(C::IfItem); c.a = it; c.b = n; c.s1 = elseL; return c; }
inline Cmd Goto(const std::string& l) { Cmd c = mk(C::Goto); c.s1 = l; return c; }
inline Cmd Label(const std::string& l) { Cmd c = mk(C::Label); c.s1 = l; return c; }
inline Cmd Sw(int id, bool v) { Cmd c = mk(C::Sw); c.a = id; c.b = v; return c; }
inline Cmd Self(char l, bool v) { Cmd c = mk(C::Self); c.a = l - 'A'; c.b = v; return c; }
inline Cmd Var(int id, int v) { Cmd c = mk(C::Var); c.a = id; c.b = v; return c; }
inline Cmd VarAdd(int id, int d) { Cmd c = mk(C::VarAdd); c.a = id; c.b = d; return c; }
inline Cmd Give(int it, int n) { Cmd c = mk(C::Item); c.a = it; c.b = n; return c; }
inline Cmd Transfer(int map, int spot, int dir) { Cmd c = mk(C::Transfer); c.a = map; c.b = spot; c.c = dir; return c; }
inline Cmd Wait(float s) { Cmd c = mk(C::Wait); c.f = s; return c; }
inline Cmd Shake(int power, float s) { Cmd c = mk(C::Shake); c.a = power; c.f = s; return c; }
inline Cmd Flash(Color col, float s) { Cmd c = mk(C::Flash); c.col = col; c.f = s; return c; }
inline Cmd Se(int id) { Cmd c = mk(C::Se); c.a = id; return c; }
}  // namespace cmd

// ------------------------------------------------------- strony zdarzeń ---
struct Page {
    std::vector<int> condSw;
    int condSelf = -1, condVar = -1, condVarMin = 0;
    G gfx = G::None;
    int palId = 0;
    Trig trig = Trig::None;
    Prio prio = Prio::Same;
    Move move = Move::Fixed;
    float speed = NPC_SPEED;
    int dir = -1;
    bool through = false, bridge = false, pushable = false, dirFix = false;
    int plateSw = -1, enterSe = -1;
    std::vector<Cmd> list;

    Page& sw(int s) { condSw.push_back(s); return *this; }
    Page& self(char l) { condSelf = l - 'A'; return *this; }
    Page& varGE(int v, int m) { condVar = v; condVarMin = m; return *this; }
    Page& pal(int p) { palId = p; return *this; }
    Page& random(float spd = NPC_SPEED) { move = Move::Random; speed = spd; return *this; }
    Page& face(int d) { dir = d; return *this; }
    Page& push() { pushable = true; return *this; }
    Page& plate(int s) { plateSw = s; return *this; }
    Page& asBridge() { bridge = true; return *this; }
    Page& onEnter(int se) { enterSe = se; return *this; }
    Page& cmds(std::vector<Cmd> l) { list = std::move(l); return *this; }
};
inline Page page(G g, Trig t, Prio p) { Page pg; pg.gfx = g; pg.trig = t; pg.prio = p; return pg; }

struct EventDef { char marker = 0; std::string name; char ground = 0; std::vector<Page> pages; };
inline EventDef ev(char m, const char* name, std::vector<Page> pages, char ground = 0) {
    EventDef e; e.marker = m; e.name = name; e.pages = std::move(pages); e.ground = ground; return e;
}

struct MapDef {
    std::string name;
    std::vector<std::string> rows;
    char defGround = '.';
    bool dark = false;
    int weather = 0;   // 0 brak, 1 liście, 2 kurz
    std::vector<EventDef> events;
};

// Przełączniki / zmienne
enum { SW_PLATE_A = 1, SW_PLATE_B = 2, SW_BRIDGE = 3, SW_CRYSTAL_W = 4, SW_CRYSTAL_E = 5, SW_CRYPT_PLATE = 6 };
enum { V_STAGE = 1, V_CHOICE = 2, V_WISHES = 3, V_GIRL = 4 };

inline const char* objective(int stage) {
    switch (stage) {
        case 0: return "Porozmawiaj ze ^3Starszym Witem^0 — stoi przy swoim domu.";
        case 1: return "Otwórz kratę świątyni: połóż ciężar na ^1dwóch płytach^0 w Ogrodzie Płyt.";
        case 2: return "Otwórz drzwi świątyni ^3Starym kluczem^0.";
        case 3: return "Zbadaj Starą Kryptę i odnajdź ^3Amulet Świtu^0.";
        case 4: return "Wróć z amuletem do ^3Starszego Wita^0.";
        default: return "Pieczęć odnowiona! Zwiedzaj do woli.";
    }
}

// Zdefiniowane w maps.cpp
std::vector<MapDef> buildMaps();

}  // namespace rpg
