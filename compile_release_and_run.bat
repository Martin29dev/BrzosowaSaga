@echo off
rem ===========================================================================
rem  compile_release_and_run.bat - kompiluje Brzozowa Saga (RELEASE, MSVC x64)
rem  i uruchamia gre. Wystarczy dwuklik.
rem ===========================================================================
setlocal EnableExtensions
cd /d "%~dp0"

set "APP=BrzozowaSaga"
set "OUT=build\release"

call "%~dp0tools\env.bat"
if errorlevel 1 goto :fail

if not exist "%OUT%\obj" mkdir "%OUT%\obj"

echo [INFO] Zasoby (ikona, wersja)...
rc.exe /nologo /fo "%OUT%\obj\app.res" res\app.rc
if errorlevel 1 goto :fail

echo [INFO] Kompilacja (release)...
cl.exe /nologo /std:c++17 /utf-8 /EHsc /MP /W3 /wd4244 /wd4267 /wd4305 /Isrc /I"%RAYLIB_DIR%\include" /MD /O2 /Oi /GL /Gy /DNDEBUG /Fo%OUT%\obj\ /Fd%OUT%\obj\ /Fe%OUT%\%APP%.exe src\*.cpp "%OUT%\obj\app.res" /link /LTCG /OPT:REF /OPT:ICF /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /LIBPATH:"%RAYLIB_DIR%\lib" raylib.lib opengl32.lib gdi32.lib winmm.lib user32.lib shell32.lib
if errorlevel 1 goto :fail

echo.
echo [OK] Zbudowano: %OUT%\%APP%.exe

echo [INFO] Uruchamiam gre...
start "" /D "%OUT%" "%OUT%\%APP%.exe"
exit /b 0

:fail
echo.
echo [BLAD] Kompilacja nie powiodla sie. Sprawdz komunikaty powyzej.
pause
exit /b 1
