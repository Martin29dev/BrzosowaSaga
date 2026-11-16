// ============================================================================
//  BRZOZOWA SAGA — Pieczęć Świtu
//  Mini-silnik w stylu RPG Maker MV w C++ (raylib 5.x, MSVC)
//  Zero zewnętrznych assetów: grafika, dźwięki i mapy generowane w kodzie.
// ----------------------------------------------------------------------------
//  KOMPILACJA (MSVC, "x64 Native Tools Command Prompt for VS"):
//
//    cl /nologo /std:c++17 /O2 /EHsc /utf-8 /MD rpg.cpp /I C:\raylib\include ^
//       /link /LIBPATH:C:\raylib\lib raylib.lib opengl32.lib gdi32.lib ^
//       winmm.lib user32.lib shell32.lib
//
//  Visual Studio: projekt C++ (konsola), C++17, dodaj ścieżki include/lib
//  raylib, w C/C++ -> Command Line dopisz /utf-8, linkuj biblioteki jw.
//  (plik zapisany jako UTF-8 z BOM + pragma, więc polskie znaki działają
//   także bez /utf-8).
// ----------------------------------------------------------------------------
//  STEROWANIE
//    Strzałki / WASD / Numpad ...... ruch po siatce (4 kierunki, jak w MV)
//    Shift ......................... bieg (dash)
//    Z / Enter / Spacja ............ akcja / dalej (przytrzymaj = szybki tekst)
//    X / Esc / PPM ................. menu / anuluj
//    LPM ........................... idź tutaj (pathfinding BFS jak w MV)
//    F5 / F9 ....................... szybki zapis / odczyt
//    F1 debug, F2 limit FPS (test delta time), F11 pełny ekran
// ----------------------------------------------------------------------------
//  MECHANIKI (wzorowane na RPG Maker MV)
//    * Ruch kafelkowy z płynną interpolacją, prędkości MV (3.75 / 7.5 kafla/s),
//      bezszwowe przechodzenie między kafelkami (przenoszenie nadmiaru czasu)
//    * Stały krok logiki 60 Hz + akumulator + interpolacja renderu (alpha),
//      zatrzaskiwanie wejścia między klatkami, ochrona przed "spiral of death"
//    * Kolizje: kafelki, zdarzenia "jak postacie", rezerwacja pola docelowego
//    * Zdarzenia ze STRONAMI (warunki: przełączniki, self-switch, zmienne),
//      wyzwalacze: Akcja / Dotyk gracza / Autorun, priorytety Pod/Jak/Nad
//    * Interpreter poleceń: tekst, wybory, warunki, skoki, przełączniki,
//      zmienne, przedmioty, teleport z wygaszaniem, wait, shake, flash, SE
//    * Płyty naciskowe (gracz LUB głaz), dźwignie, kraty, drzwi, skrzynie,
//      przesuwane głazy (sokoban), most opuszczany dźwignią, kryształy
//    * Okno wiadomości MV: maszyna do pisania, kolory ^0-^9, okno imienia,
//      okno wyborów, animacja otwierania; menu, zapis/odczyt, ekran tytułowy
//    * Oświetlenie dynamiczne w krypcie, cząsteczki, Y-sorting, warstwa
//      "nad postaciami" (korony drzew), supersampling 2x, letterboxing
// ============================================================================

#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "game.h"

// ================================================================ MAIN =====
int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(rpg::VW, rpg::VH, "Brzozowa Saga — Pieczęć Świtu");
    SetWindowMinSize(rpg::VW / 2, rpg::VH / 2);
    SetExitKey(KEY_NULL);   // Esc otwiera menu, nie zamyka gry

    rpg::g_audio.init();
    rpg::g_text.init();

    rpg::Game* game = new rpg::Game();
    game->init();

    // Pętla ze stałym krokiem (Glenn Fiedler, "Fix Your Timestep!"):
    //  - logika zawsze dostaje dt = 1/60 s -> deterministyczna, niezależna od FPS
    //  - nadmiar czasu (akumulator) służy do interpolacji renderu (alpha)
    double prev = GetTime(), acc = 0.0;
    while (!WindowShouldClose() && !game->quit) {
        double now = GetTime();
        double frame = now - prev;
        prev = now;
        if (frame > rpg::MAX_FRAME) frame = rpg::MAX_FRAME;   // np. przeciąganie okna
        if (frame < 0.0) frame = 0.0;
        acc += frame;

        game->pollInput();
        int steps = 0;
        while (acc >= rpg::STEP) {
            game->step((float)rpg::STEP);
            game->in.consume();
            acc -= rpg::STEP;
            ++steps;
        }
        game->lastSteps = steps;
        game->lastFrame = (float)frame;
        game->render((float)(acc / rpg::STEP));
    }

    game->shutdown();
    delete game;
    rpg::g_text.shutdown();
    rpg::g_audio.shutdown();
    CloseWindow();
    return 0;
}
