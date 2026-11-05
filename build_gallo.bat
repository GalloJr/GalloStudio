@echo off
setlocal

REM Localiza o Strawberry Perl e CMake do Visual Studio caso nao estejam no PATH
if exist "C:\Strawberry\perl\bin\perl.exe" (
    set "PATH=C:\Strawberry\perl\bin;%PATH%"
)

set "VSCMAKE=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
if exist "%VSCMAKE%\cmake.exe" (
    set "PATH=%VSCMAKE%;%PATH%"
)

echo ========================================================
echo               GalloStudio Build Tool
echo ========================================================

if "%~1"=="" goto default_action

call .\build_win.bat %*
goto end

:default_action
if exist "deps\build\OrcaSlicer_dep\usr\local\lib\libssl.lib" (
    echo [INFO] Dependencias completas encontradas.
    echo [INFO] Compilando GalloStudio Release...
    call .\build_win.bat -s
) else (
    echo [INFO] Finalizando Dependencias e Compilando GalloStudio...
    call .\build_win.bat -ds
)

:end
