// maps.cpp — zawartość gry: mapy i zdarzenia (jak edytor MV)
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "data.h"

namespace rpg {

// ============================================================ BUDOWA MAP ===
//  Legenda kafli:  . trawa  : kwiaty  = ścieżka  ~ woda  T drzewo  F płot
//                  # mur  C posadzka  R dach  H ściana domu  X pustka
//                  * pochodnia  P kolumna
//  Małe litery = zdarzenia (definicje niżej), cyfry/@ = punkty przybycia.
std::vector<MapDef> buildMaps() {
    using namespace cmd;
    std::vector<MapDef> M;
    const Color C_GOLD = rgba(255, 226, 150), C_CYAN = rgba(140, 220, 255);

    auto coin = [&]() {
        return ev('o', "Moneta", {
            page(G::Coin, Trig::Touch, Prio::Below).cmds({
                Se(SE_ITEM), Give(IT_COIN, 1), Self('A', true), Text("", "Znaleziono ^3Monetę^0!")}),
            page(G::None, Trig::None, Prio::Below).self('A')});
    };

    // ----------------------------------------------------------- MAPA 0 --
    {
        MapDef m;
        m.name = "Wioska Brzozowa"; m.defGround = '.'; m.weather = 1;
        m.rows = {
            "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTT",
            "TTTTTTTTTT##a##TTTTTT~~~TTTTTT",
            "TFFFFFFFT##C1C##TTT..~~~....TT",
            "TF..:..FT#CCCCC#T....~~~..o..T",
            "TF.b.p.FT#CkCCC#T.:..~~~.....T",
            "TF.....FT###g###T....~~~..:..T",
            "TF.p.b.F....=....l...jjj...m.T",
            "TF.....F....=........~~~.....T",
            "TFFF.FFF....=........~~~TTTTTT",
            "T.....s.....=........~~~TTTTTT",
            "T..==========......v.~~~TTTTTT",
            "T..=.....=.....:.....~~~TTTTTT",
            "T.RRRRR..=..RRRRR....~~~TTTTTT",
            "T.RRRRR..=..RRRRR..e.~~~TTTTTT",
            "T.HHcHH..=..HHdHH....~~~TTTTTT",
            "T...=....=....=......~~~TTTTTT",
            "T...=========@=....:.~~~TTTTTT",
            "T.:......=.....w.....~~~TTTTTT",
            "T........=..:........~~~TTTTTT",
            "T..TT....=.......TT..~~~TTTTTT",
            "Ti.......=...........~~~TTTTTT",
            "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTT",
        };
        auto& E = m.events;
        E.push_back(ev('i', "Intro", {
            page(G::None, Trig::Autorun, Prio::Below).cmds({
                Wait(0.5f),
                Text("", "^6Brzozowa Saga^0 — niewielka wioska na skraju Puszczy Szeptów.\n"
                         "Od kilku nocy las szepcze coraz głośniej..."),
                Text("Podpowiedź", "Poruszaj się ^1strzałkami^0 lub ^1WASD^0, przytrzymaj ^1Shift^0, aby biec. "
                                   "^1Z / Enter / Spacja^0 to interakcja, ^1Esc / X^0 otwiera menu."),
                Text("Podpowiedź", "Możesz też ^1kliknąć myszą^0 miejsce lub postać — bohater sam znajdzie drogę.\n"
                                   "Porozmawiaj ze ^3Starszym Witem^0, który stoi przy swoim domu."),
                Self('A', true)}),
            page(G::None, Trig::None, Prio::Below).self('A')}));

        E.push_back(ev('e', "Starszy Wit", {
            page(G::Char, Trig::Action, Prio::Same).pal(PAL_ELDER).face(DIR_LEFT).cmds({
                IfVar(V_STAGE, Op::GE, 5, "lt5"),
                Text("Starszy Wit", "Las znów śpiewa zamiast szeptać. Odpocznij, bohaterze."),
                Goto("end"),
                Label("lt5"),
                IfVar(V_STAGE, Op::GE, 4, "lt4"),
                Se(SE_ITEM), Flash(C_GOLD, 0.9f),
                Text("Starszy Wit", "To... ^3Amulet Świtu^0! Czuję, jak Pieczęć znów jaśnieje!"),
                Text("Starszy Wit", "Dziękuję ci, wędrowcze. Wioska Brzozowa nigdy tego nie zapomni."),
                Var(V_STAGE, 5),
                Text("", "^6~ KONIEC ~^0\nDziękujemy za grę! Możesz dalej zwiedzać okolicę."),
                Goto("end"),
                Label("lt4"),
                IfVar(V_STAGE, Op::GE, 3, "lt3"),
                Text("Starszy Wit", "Podobno w krypcie trzeba obudzić ^1dwa kryształy^0...\nUważaj na siebie."),
                Goto("end"),
                Label("lt3"),
                IfVar(V_STAGE, Op::GE, 2, "lt2"),
                Text("Starszy Wit", "Masz ^3Stary klucz^0! Drzwi świątyni są tuż za kratą."),
                Goto("end"),
                Label("lt2"),
                IfVar(V_STAGE, Op::GE, 1, "lt1"),
                Text("Starszy Wit", "Płyty w zachodnim ogrodzie reagują na ciężar.\n"
                                    "Głazy są ciężkie... ale i ty swoje ważysz, he, he."),
                Goto("end"),
                Label("lt1"),
                Text("Starszy Wit", "Ach, wędrowiec! Dawno nikt nie zaglądał do naszej wioski."),
                Text("Starszy Wit", "Pod świątynią na północy śpi ^2Pieczęć Świtu^0.\n"
                                    "Jej blask słabnie, a las szepcze coraz głośniej..."),
                Ask("Starszy Wit", "Czy pomożesz nam ją odnowić?", {"Oczywiście!", "Może później..."}, V_CHOICE),
                IfVar(V_CHOICE, Op::EQ, 0, "no"),
                Se(SE_OK), Var(V_STAGE, 1),
                Text("Starszy Wit", "Wspaniale! Wejścia strzeże ^1żelazna krata^0. Otworzysz ją, kładąc "
                                    "ciężar na ^1dwóch kamiennych płytach^0 w Ogrodzie Płyt na zachodzie."),
                Text("Starszy Wit", "Za kratą stoi skrzynia ze ^3Starym kluczem^0.\nNiech Świt prowadzi twoje kroki."),
                Goto("end"),
                Label("no"),
                Text("Starszy Wit", "Rozumiem... Gdybyś zmienił zdanie — będę tutaj."),
                Label("end")})}));

        E.push_back(ev('v', "Hania", {
            page(G::Char, Trig::Action, Prio::Same).pal(PAL_GIRL).random().cmds({
                VarAdd(V_GIRL, 1),
                IfVar(V_GIRL, Op::EQ, 1, "v2"),
                Text("Hania", "Dzień dobry! Pięknie dziś pachną brzozy, prawda?"), Goto("end"),
                Label("v2"), IfVar(V_GIRL, Op::EQ, 2, "v3"),
                Text("Hania", "Rybak Bogdan łowi na wysepce za rzeką.\nPodobno most da się opuścić ^1dźwignią^0 na brzegu."),
                Goto("end"),
                Label("v3"), IfVar(V_GIRL, Op::EQ, 3, "v4"),
                Text("Hania", "Do studni w centrum wioski wrzuca się monety na szczęście.\nJa wrzuciłam już chyba sto!"),
                Goto("end"),
                Label("v4"), Var(V_GIRL, 0),
                Text("Hania", "Biegnij, biegnij! Z ^1Shiftem^0 będziesz szybszy niż wiatr!"),
                Label("end")})}));

        E.push_back(ev('m', "Bogdan", {
            page(G::Char, Trig::Action, Prio::Same).pal(PAL_FISHER).face(DIR_LEFT).cmds({
                IfSelf('A', false, "again"),
                Text("Bogdan", "O! Ktoś przeszedł przez most? Dawno nikt mnie nie odwiedzał."),
                Text("Bogdan", "Masz, weź tę ^3monetę^0 — wyłowiłem ją wczoraj.\nWrzuć ją do studni, może spełni się twoje życzenie."),
                Se(SE_ITEM), Give(IT_COIN, 1), Self('A', true), Goto("end"),
                Label("again"),
                Text("Bogdan", "Ryby dziś nie biorą... ale widok jest piękny."),
                Label("end")})}));

        E.push_back(coin());

        E.push_back(ev('w', "Studnia", {
            page(G::Well, Trig::Action, Prio::Same).cmds({
                IfItem(IT_COIN, 1, "nocoin"),
                Ask("", "Stara studnia. Wrzucić ^3monetę^0?", {"Tak", "Nie"}, V_CHOICE),
                IfVar(V_CHOICE, Op::EQ, 0, "end"),
                Give(IT_COIN, -1), Se(SE_CRYSTAL), VarAdd(V_WISHES, 1),
                IfVar(V_WISHES, Op::GE, 3, "plain"),
                Flash(C_CYAN, 0.7f), Se(SE_ITEM),
                Text("", "Z głębi studni bije delikatne światło...\n^6Osiągnięcie: Szczodre serce^0 — wrzucono 3 monety!"),
                Goto("end"),
                Label("plain"),
                Text("", "Plusk! Moneta znika w głębinie.\nCzujesz, że los się do ciebie uśmiecha."),
                Goto("end"),
                Label("nocoin"),
                Text("", "Stara kamienna studnia. Na dnie połyskują monety."),
                Label("end")})}));

        E.push_back(ev('s', "Tabliczka", {
            page(G::Sign, Trig::Action, Prio::Same).cmds({
                Text("Tabliczka", "^1Ogród Płyt^0\n„Ciężar na obu kamieniach otworzy drogę do Świątyni.”"),
                Ask("Tabliczka", "Utknąłeś? Głazy można ustawić od nowa.", {"Zresetuj głazy", "Zostaw"}, V_CHOICE),
                IfVar(V_CHOICE, Op::EQ, 0, "end"),
                Se(SE_OK), Transfer(-1, -1, -1),
                Label("end")})}));

        E.push_back(ev('c', "Drzwi", {
            page(G::Door, Trig::Action, Prio::Same).cmds({
                Se(SE_BUZZER), Text("", "Drzwi są zamknięte. Ze środka dobiega donośne chrapanie...")})}, 'H'));
        E.push_back(ev('d', "Drzwi", {
            page(G::Door, Trig::Action, Prio::Same).cmds({
                Se(SE_BUZZER), Text("", "Na drzwiach wisi kartka:\n„Poszłam na grzyby. Wracam wieczorem. — Hania”")})}, 'H'));

        // płyty naciskowe (gracz lub głaz) -> przełączniki 1 i 2
        E.push_back(ev('p', "Płyta A", {page(G::Plate, Trig::None, Prio::Below).plate(SW_PLATE_A)}));
        E.push_back(ev('p', "Płyta B", {page(G::Plate, Trig::None, Prio::Below).plate(SW_PLATE_B)}));
        E.push_back(ev('b', "Głaz", {page(G::Boulder, Trig::None, Prio::Same).push()}));

        E.push_back(ev('g', "Krata", {
            page(G::Gate, Trig::Action, Prio::Same).cmds({
                Text("", "Masywna żelazna krata. Gołymi rękami się jej nie podniesie.")}),
            page(G::Gate, Trig::Autorun, Prio::Same).sw(SW_PLATE_A).sw(SW_PLATE_B).cmds({
                Wait(0.35f), Se(SE_RUMBLE), Shake(6, 0.9f), Wait(0.9f), Se(SE_DOOR), Self('A', true),
                Text("", "Krata z łoskotem unosi się do góry!")}),
            page(G::GateOpen, Trig::None, Prio::Below).self('A')}, 'C'));

        E.push_back(ev('k', "Skrzynia", {
            page(G::Chest, Trig::Action, Prio::Same).cmds({
                Se(SE_CHEST), Self('A', true), Wait(0.25f), Se(SE_ITEM), Give(IT_KEY, 1),
                Text("", "Zdobyto ^3Stary klucz^0!"),
                IfVar(V_STAGE, Op::LT, 2, "end"), Var(V_STAGE, 2), Label("end")}),
            page(G::ChestOpen, Trig::Action, Prio::Same).self('A').cmds({Text("", "Skrzynia jest pusta.")})}));

        E.push_back(ev('a', "Drzwi świątyni", {
            page(G::Door, Trig::Action, Prio::Same).cmds({
                IfItem(IT_KEY, 1, "locked"),
                Se(SE_DOOR), Self('A', true),
                Text("", "Klucz z trudem obraca się w zamku... Drzwi ustępują."),
                IfVar(V_STAGE, Op::LT, 3, "go"), Var(V_STAGE, 3), Label("go"),
                Transfer(1, '1', DIR_UP), Goto("end"),
                Label("locked"), Se(SE_BUZZER),
                Text("", "Ciężkie drzwi świątyni. Na zamku wyryto symbol słońca."),
                Label("end")}),
            page(G::DoorOpen, Trig::Touch, Prio::Below).self('A').cmds({Transfer(1, '1', DIR_UP)})}, 'C'));

        E.push_back(ev('l', "Dźwignia", {
            page(G::LeverOff, Trig::Action, Prio::Same).cmds({
                Se(SE_SWITCH), Sw(SW_BRIDGE, true), Wait(0.2f), Se(SE_RUMBLE), Shake(3, 0.5f), Wait(0.5f),
                Text("", "Mechanizm zgrzyta... Drewniany most opada nad rzeką!")}),
            page(G::LeverOn, Trig::Action, Prio::Same).sw(SW_BRIDGE).cmds({
                Ask("", "Podnieść most z powrotem?", {"Tak", "Nie"}, V_CHOICE),
                IfVar(V_CHOICE, Op::EQ, 0, "end"),
                Se(SE_SWITCH), Sw(SW_BRIDGE, false), Text("", "Most unosi się z powrotem."),
                Label("end")})}));

        E.push_back(ev('j', "Most", {
            page(G::None, Trig::None, Prio::Below),
            page(G::Bridge, Trig::None, Prio::Below).sw(SW_BRIDGE).asBridge()}, '~'));

        M.push_back(m);
    }

    // ----------------------------------------------------------- MAPA 1 --
    {
        MapDef m;
        m.name = "Stara Krypta"; m.defGround = 'C'; m.dark = true; m.weather = 2;
        m.rows = {
            "XXXXXXXXXXXXXXXXXXXX",
            "X######*###*#######X",
            "X#q....#.y.#.....r#X",
            "X#.....#...#......#X",
            "X#..P..##z##..P...#X",
            "X#................#X",
            "X#..P....u....P...#X",
            "X#................#X",
            "X##.#####.#####.###X",
            "X#...#XX#.#XX#....#X",
            "X#.b.#XX#.#XX#..n.#X",
            "X#p..#XX#1#XX#.o..#X",
            "X#####XX#x#XX######X",
            "XXXXXXXXXXXXXXXXXXXX",
        };
        for (auto& r : m.rows) std::replace(r.begin(), r.end(), '.', 'C');
        auto& E = m.events;
        auto crystal = [&](char mk, int s) {
            return ev(mk, "Kryształ", {
                page(G::CrystalOff, Trig::Action, Prio::Same).cmds({
                    Se(SE_CRYSTAL), Sw(s, true), Flash(C_CYAN, 0.45f),
                    Text("", "Dotykasz kryształu... rozbłyska jasnym, błękitnym światłem!")}),
                page(G::CrystalOn, Trig::Action, Prio::Same).sw(s).cmds({
                    Text("", "Kryształ pulsuje spokojną energią.")})});
        };
        E.push_back(crystal('q', SW_CRYSTAL_W));
        E.push_back(crystal('r', SW_CRYSTAL_E));
        E.push_back(ev('x', "Wyjście", {
            page(G::Stairs, Trig::Touch, Prio::Below).cmds({Transfer(0, '1', DIR_DOWN)})}));
        E.push_back(ev('u', "Posąg", {
            page(G::Statue, Trig::Action, Prio::Same).cmds({
                Text("Inskrypcja", "„Dwa kryształy przebudzą moc.\nKamień pod ciężarem otworzy drogę.\nGdy ciężar zniknie — wrota zasną.”")})}));
        E.push_back(ev('p', "Płyta", {page(G::Plate, Trig::None, Prio::Below).plate(SW_CRYPT_PLATE)}));
        E.push_back(ev('b', "Głaz", {page(G::Boulder, Trig::None, Prio::Same).push()}));
        E.push_back(ev('z', "Wrota", {
            page(G::Seal, Trig::Action, Prio::Same).cmds({
                Se(SE_BUZZER),
                Text("", "Kamienne wrota pokryte runami. Trzy znaki u dołu ledwo się żarzą...")}),
            page(G::SealOpen, Trig::None, Prio::Below).sw(SW_CRYSTAL_W).sw(SW_CRYSTAL_E).sw(SW_CRYPT_PLATE).onEnter(SE_DOOR)}, 'C'));
        E.push_back(ev('y', "Skarbiec", {
            page(G::Chest, Trig::Action, Prio::Same).cmds({
                Se(SE_CHEST), Wait(0.3f), Se(SE_ITEM), Flash(C_GOLD, 1.0f), Shake(2, 0.4f),
                Self('A', true), Give(IT_AMULET, 1),
                Text("", "Zdobyto ^3Amulet Świtu^0!"),
                Text("", "Ciepłe światło rozlewa się po krypcie.\nCzas wrócić do ^3Starszego Wita^0."),
                IfVar(V_STAGE, Op::LT, 4, "end"), Var(V_STAGE, 4), Label("end")}),
            page(G::ChestOpen, Trig::Action, Prio::Same).self('A').cmds({Text("", "Pusto. Amulet jest już twój.")})}));
        E.push_back(ev('n', "Duch", {
            page(G::Char, Trig::Action, Prio::Same).pal(PAL_GHOST).random(1.4f).cmds({
                Text("Duch", "Uuuuu... Nie bój się, wędrowcze. Strzegę tego miejsca od wieków."),
                Text("Duch", "Kryształy budzą się pod dotykiem... a płyta lubi ^1ciężkie^0 towarzystwo.\n"
                             "Ty nie możesz stać na niej i jednocześnie przejść przez wrota, prawda?")})}));
        E.push_back(coin());
        M.push_back(m);
    }
    return M;
}

}  // namespace rpg
