@echo off
rem ===========================================================================
rem  compile_debug_and_run.bat - kompiluje Brzozowa Saga (DEBUG, MSVC x64)
rem  i uruchamia gre. Wystarczy dwuklik.
rem ===========================================================================
setlocal EnableExtensions
cd /d "%~dp0"

set "APP=BrzozowaSaga"
set "OUT=build\debug"

call "%~dp0tools\env.bat"
if errorlevel 1 goto :fail

if not exist "%OUT%\obj" mkdir "%OUT%\obj"

echo [INFO] Zasoby (ikona, wersja)...
rc.exe /nologo /fo "%OUT%\obj\app.res" res\app.rc
if errorlevel 1 goto :fail

echo [INFO] Kompilacja (debug)...
cl.exe /nologo /std:c++17 /utf-8 /EHsc /MP /W3 /wd4244 /wd4267 /wd4305 /Isrc /I"%RAYLIB_DIR%\include" /MD /Od /Zi /RTC1 /D_DEBUG_GAME /Fo%OUT%\obj\ /Fd%OUT%\obj\ /Fe%OUT%\%APP%.exe src\*.cpp "%OUT%\obj\app.res" /link /DEBUG /SUBSYSTEM:CONSOLE /LIBPATH:"%RAYLIB_DIR%\lib" raylib.lib opengl32.lib gdi32.lib winmm.lib user32.lib shell32.lib
if errorlevel 1 goto :fail

echo.
echo [OK] Zbudowano: %OUT%\%APP%.exe

echo [INFO] Uruchamiam gre (logi raylib w tym oknie)...
echo.
pushd "%OUT%"
"%APP%.exe"
set "GAME_EXIT=%ERRORLEVEL%"
popd
echo.
echo [INFO] Gra zakonczona (kod %GAME_EXIT%).
pause
exit /b %GAME_EXIT%

:fail
echo.
echo [BLAD] Kompilacja nie powiodla sie. Sprawdz komunikaty powyzej.
pause
exit /b 1
