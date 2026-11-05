@echo off
setlocal
cd /d "%~dp0"

echo Iniciando o GalloStudio...
if exist "build\src\Release\GalloStudio.exe" (
    start "" "build\src\Release\GalloStudio.exe" %*
) else if exist "build\src\Release\orca-slicer.exe" (
    start "" "build\src\Release\orca-slicer.exe" %*
) else (
    echo [ERRO] Executavel nao encontrado. Execute build_gallo.bat primeiro.
    pause
)
