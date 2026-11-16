# Brzozowa Saga — Pieczęć Świtu

Mała gra RPG w stylu **RPG Maker MV**, napisana w C++17 na **raylib**.
Nie potrzebuje żadnych plików zewnętrznych: grafika, dźwięki i mapy powstają w kodzie.

## Szybki start (Windows + MSVC)

1. Zainstaluj **Visual Studio 2019/2022** lub **Build Tools for Visual Studio**
   z komponentem *Programowanie aplikacji klasycznych w języku C++*.
2. Kliknij dwukrotnie **`compile_release_and_run.bat`**.

Skrypt sam:
- znajduje MSVC (przez `vswhere`) i ładuje środowisko x64,
- przy pierwszym uruchomieniu pobiera raylib 5.5 (MSVC x64) do `third_party\raylib`,
- kompiluje grę do `build\release\BrzozowaSaga.exe` (`/O2 /GL /LTCG`, bez okna konsoli),
- uruchamia grę.

Jeśli masz już raylib, przed uruchomieniem skryptu ustaw `set RAYLIB_DIR=C:\sciezka\do\raylib`.
Ten folder musi zawierać podfoldery `include\` (z `raylib.h` i `rlgl.h`) oraz `lib\` (z `raylib.lib`).

| Skrypt | Co robi |
|---|---|
| `compile_release_and_run.bat` | build Release (optymalizacje, LTCG, bez konsoli) + uruchomienie |
| `compile_debug_and_run.bat` | build Debug (`/Od /Zi /RTC1`, z konsolą i logami raylib) + uruchomienie |
| `clean.bat` | usuwa folder `build\` |
| `tools\env.bat` | wspólna konfiguracja MSVC i raylib, wywoływana przez powyższe skrypty |

## Sterowanie

| Klawisz | Akcja |
|---|---|
| Strzałki / WASD / Numpad | ruch po siatce |
| Shift | bieg |
| Z / Enter / Spacja | akcja, dalej (przytrzymaj, aby przyspieszyć tekst) |
| X / Esc / PPM | menu, anuluj |
| LPM | idź we wskazane miejsce (pathfinding) |
| F5 / F9 | szybki zapis / odczyt |
| F1 | tryb debugowania (kolizje, FPS, kroki logiki) |
| F2 | limit FPS: VSync / 30 / 144 / bez limitu (prędkość gry się nie zmienia) |
| F11, Alt+Enter | pełny ekran (także w menu i na ekranie tytułowym; ustawienie jest zapamiętywane) |

## Ikona i pełny ekran

- `res/app.ico` to ikona w rozmiarach 16–256 px. `res/app.rc` dołącza ją do pliku exe jako zasób `GLFW_ICON`.
  GLFW (którego używa raylib) sam ładuje zasób o tej nazwie, więc ikona pojawia się:
  w lewym górnym rogu okna, na pasku zadań, w Eksploratorze, na pulpicie i na skrótach.
  `res/app.rc` dodaje też informacje o wersji, widoczne we *Właściwościach* pliku exe.
- Pełny ekran działa w trybie *borderless windowed*: okno bez ramki na cały monitor, bez zmiany trybu graficznego,
  więc przełączanie jest szybkie, a Alt+Tab działa bez problemów. Obraz jest skalowany z zachowaniem proporcji.
- Ustawienia (pełny ekran, limit FPS) są zapisywane w pliku `brzozowa_settings.txt` obok pliku exe.
- Windows może pamiętać starą ikonę w pamięci podręcznej. Jeśli na pulpicie nadal widać starą, usuń skrót i utwórz go ponownie.
- Jeśli budujesz w projekcie Visual Studio zamiast skryptem, dodaj do niego plik `res/app.rc` (*Add → Existing Item*).

## Struktura kodu

```
res/
  app.ico                ikona aplikacji (16-256 px)
  app.rc                 zasoby Windows: ikona GLFW_ICON + informacje o wersji
  icon.png               podgląd ikony
src/
  common.h / .cpp        stałe, matematyka, RNG, prymitywy rysowania
  audio.h  / .cpp        synteza efektów dźwiękowych (SE)
  text.h   / .cpp        czcionka, kolory ^0-^9, zawijanie wierszy
  data.h                 dane gry: zdarzenia, strony, polecenia, przedmioty
  maps.cpp               ZAWARTOŚĆ GRY: mapy i zdarzenia (odpowiednik edytora MV)
  runtime.h              postacie, zdarzenia w grze, wejście, interpreter
  game.h                 główna klasa Game
  game_world.cpp         ładowanie map, strony zdarzeń, kolizje
  game_interpreter.cpp   interpreter poleceń + okno wiadomości
  game_player.cpp        ruch gracza, BFS, NPC, płyty naciskowe, autorun
  game_flow.cpp          przejścia, zapis/odczyt, menu, tytuł, krok logiki
  render_tiles.cpp       kafle proceduralne, wypiekanie map do tekstur
  render_sprites.cpp     postacie i obiekty
  render_ui.cpp          okna w stylu MV, menu, ekran tytułowy
  render.cpp             potok renderowania, światła, letterbox
  main.cpp               okno + pętla ze stałym krokiem
```

## Pętla i delta time

- Logika działa ze **stałym krokiem 1/60 s**. Akumulator czasu i interpolacja (`alpha`) dają płynny obraz przy dowolnym FPS.
- Czas jednej klatki jest obcinany do 0,25 s, więc np. przeciąganie okna nie rozpędza gry („spiral of death”).
- Wciśnięcia klawiszy są zapamiętywane między klatkami i odczytywane także z kolejki `GetKeyPressed()`. Dzięki temu żadne krótkie wciśnięcie nie ginie.
- Gdy postać dochodzi do środka kafla, niewykorzystana część kroku przechodzi na następny ruch. Ruch jest przez to płynny i nie zatrzymuje się na granicach kafli.

## Jak dodawać zawartość (`maps.cpp`)

Mapa to tablica napisów. Legenda kafli:
`.` trawa, `:` kwiaty, `=` ścieżka, `~` woda, `T` drzewo, `F` płot, `#` mur, `C` posadzka,
`R` dach, `H` ściana domu, `X` pustka, `*` pochodnia, `P` kolumna.

- **Małe litery** to zdarzenia, które definiujesz funkcją `ev('litera', "Nazwa", {strony...})`.
  Ta sama litera użyta kilka razy bierze kolejne definicje.
- **Cyfry i `@`** to punkty przybycia (`@` to start nowej gry). Na nie wskazuje polecenie `Transfer(mapa, '1', kierunek)`.

Strony działają jak w MV: aktywna jest strona o **najwyższym numerze**, której warunki są spełnione.

```cpp
E.push_back(ev('k', "Skrzynia", {
    page(G::Chest, Trig::Action, Prio::Same).cmds({
        Se(SE_CHEST), Self('A', true), Give(IT_KEY, 1),
        Text("", "Zdobyto ^3Stary klucz^0!")}),
    page(G::ChestOpen, Trig::Action, Prio::Same).self('A').cmds({
        Text("", "Skrzynia jest pusta.")})}));
```

Dostępne polecenia: `Text`, `Ask` (wybór), `IfSw`, `IfSelf`, `IfVar`, `IfItem`, `Goto`, `Label`,
`Sw`, `Self`, `Var`, `VarAdd`, `Give`, `Transfer`, `Wait`, `Shake`, `Flash`, `Se`.
Polecenia warunkowe skaczą do podanej etykiety, gdy warunek **nie** jest spełniony.

Zapis gry trafia do pliku `brzozowa_save.txt` obok pliku exe.
