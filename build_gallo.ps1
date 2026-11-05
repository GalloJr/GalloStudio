<#
.SYNOPSIS
    Script automatizado de compilação do GalloStudio para Windows.
.DESCRIPTION
    Configura o PATH com o CMake integrado do Visual Studio e executa
    o build_win.bat para compilar o GalloStudio, dependências e testes.
.EXAMPLE
    .\build_gallo.ps1 -BuildSlicer
    .\build_gallo.ps1 -BuildDeps
    .\build_gallo.ps1 -RunTests
    .\build_gallo.ps1 -All
#>

[CmdletBinding()]
param (
    [switch]$BuildDeps,
    [switch]$BuildSlicer,
    [switch]$RunTests,
    [switch]$All,
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

# 1. Localiza e configura o CMake do Visual Studio no PATH
$vsCMakeDir = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
if (Test-Path $vsCMakeDir) {
    if ($env:PATH -notlike "*$vsCMakeDir*") {
        Write-Host "Configurando CMake do Visual Studio no PATH..." -ForegroundColor Cyan
        $env:PATH = "$vsCMakeDir;$env:PATH"
    }
} else {
    Write-Warning "CMake do Visual Studio nao encontrado no caminho padrao. Certifique-se de que o CMake esta no PATH."
}

# 2. Determina os parâmetros para o build_win.bat
$argsList = @()

if ($DryRun) {
    $argsList += "-D"
}

if ($All) {
    $argsList += "-ds"
    $argsList += "--run-tests"
} else {
    if ($BuildDeps) {
        $argsList += "-d"
    }
    if ($BuildSlicer) {
        $argsList += "-s"
    }
    if ($RunTests) {
        $argsList += "--run-tests"
    }
}

if ($argsList.Count -eq 0 -or ($argsList.Count -eq 1 -and $DryRun)) {
    # Default: compila o slicer
    $argsList += "-s"
}

Write-Host "Iniciando processo de build do GalloStudio..." -ForegroundColor Green
Write-Host "Comando: .\build_win.bat $($argsList -join ' ')" -ForegroundColor Yellow

& .\build_win.bat @argsList
