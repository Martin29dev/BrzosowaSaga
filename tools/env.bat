@echo off
rem ===========================================================================
rem  tools\env.bat - przygotowuje srodowisko MSVC x64 oraz biblioteke raylib.
rem  Wywolywany przez compile_*.bat (nie uruchamiaj bezposrednio).
rem
rem  Zmienne, ktore mozesz ustawic wczesniej:
rem    RAYLIB_DIR  - folder z raylib (podfoldery include\ i lib\)
rem                  domyslnie: third_party\raylib (pobierany automatycznie)
rem ===========================================================================

if not defined RAYLIB_DIR set "RAYLIB_DIR=%~dp0..\third_party\raylib"
set "RAYLIB_URL=https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_msvc16.zip"

rem ---------------------------------------------------------------- MSVC ---
if /i "%VSCMD_ARG_TGT_ARCH%"=="x64" goto :have_msvc

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :err_no_vs

set "VSPATH="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
if not defined VSPATH goto :err_no_vs
if not exist "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" goto :err_no_vs

echo [INFO] Ladowanie srodowiska MSVC: %VSPATH%
call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" >nul
where cl.exe >nul 2>nul
if errorlevel 1 goto :err_no_vs

:have_msvc
rem -------------------------------------------------------------- raylib ---
if exist "%RAYLIB_DIR%\include\raylib.h" if exist "%RAYLIB_DIR%\include\rlgl.h" if exist "%RAYLIB_DIR%\lib\raylib.lib" goto :have_raylib

echo [INFO] Nie znaleziono raylib w "%RAYLIB_DIR%".
echo [INFO] Pobieram raylib 5.5 (MSVC x64)...
where curl.exe >nul 2>nul
if errorlevel 1 goto :err_no_raylib
where tar.exe >nul 2>nul
if errorlevel 1 goto :err_no_raylib

set "RL_ZIP=%TEMP%\raylib_msvc_%RANDOM%.zip"
set "RL_TMP=%TEMP%\raylib_extract_%RANDOM%"
curl.exe -L --fail --progress-bar -o "%RL_ZIP%" "%RAYLIB_URL%"
if errorlevel 1 goto :err_no_raylib
mkdir "%RL_TMP%"
tar.exe -xf "%RL_ZIP%" -C "%RL_TMP%"
if errorlevel 1 goto :err_no_raylib

set "RL_SRC="
for /d %%d in ("%RL_TMP%\raylib-*") do set "RL_SRC=%%d"
if not defined RL_SRC goto :err_no_raylib
if not exist "%RAYLIB_DIR%" mkdir "%RAYLIB_DIR%"
xcopy /e /i /q /y "%RL_SRC%\include" "%RAYLIB_DIR%\include" >nul
xcopy /e /i /q /y "%RL_SRC%\lib" "%RAYLIB_DIR%\lib" >nul
del /q "%RL_ZIP%" >nul 2>nul
rmdir /s /q "%RL_TMP%" >nul 2>nul
if not exist "%RAYLIB_DIR%\lib\raylib.lib" goto :err_no_raylib
echo [INFO] raylib zainstalowany w "%RAYLIB_DIR%".

:have_raylib
exit /b 0

:err_no_vs
echo.
echo [BLAD] Nie znaleziono kompilatora MSVC (x64).
echo        Zainstaluj Visual Studio 2019/2022 lub "Build Tools for Visual Studio"
echo        z komponentem "Programowanie aplikacji klasycznych w jezyku C++"
echo        (Desktop development with C++), albo uruchom skrypt z
echo        "x64 Native Tools Command Prompt for VS".
exit /b 1

:err_no_raylib
echo.
echo [BLAD] Nie udalo sie przygotowac raylib.
echo        Pobierz recznie: %RAYLIB_URL%
echo        i rozpakuj tak, aby istnialy pliki:
echo          "%RAYLIB_DIR%\include\raylib.h"
echo          "%RAYLIB_DIR%\lib\raylib.lib"
echo        Mozesz tez ustawic wlasna sciezke: set RAYLIB_DIR=C:\sciezka\do\raylib
exit /b 1
