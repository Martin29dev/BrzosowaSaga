@echo off
rem Usuwa katalog build\ (pliki obiektowe i exe). Zapisy gry w build\ tez znikna!
cd /d "%~dp0"
if exist build rmdir /s /q build
echo [OK] Wyczyszczono build\
pause
