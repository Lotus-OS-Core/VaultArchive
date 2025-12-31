@echo off
REM
REM VaultArchive Installation Script for Windows
REM Installs VaultArchive binaries and libraries
REM

setlocal EnableDelayedExpansion

REM Configuration
set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."
set "BUILD_DIR=%PROJECT_DIR%\build"
set "INSTALL_PREFIX=%ProgramFiles%\VaultArchive"
set "BIN_DIR=%INSTALL_PREFIX%\bin"
set "LIB_DIR=%INSTALL_PREFIX%\lib"
set "INCLUDE_DIR=%INSTALL_PREFIX%\include"

REM Version info
set "VERSION=0.3.27"

REM Colors for output (Windows CMD doesn't support all colors, so we use basic output)
set "BLUE=[94m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "RED=[91m"
set "NC=[0m"

:Main
echo.
echo ========================================
echo   VaultArchive %VERSION% Installer (Windows)
echo ========================================
echo.

REM Parse arguments
:ParseArgs
if "%~1"=="" goto :Parsed
if /i "%~1"=="--help" goto :ShowHelp
if /i "%~1"=="-h" goto :ShowHelp
if /i "%~1"=="--prefix" (
    set "INSTALL_PREFIX=%~2"
    shift
    goto :ParseArgs
)
if /i "%~1"=="--skip-build" (
    set "SKIP_BUILD=true"
    goto :ParseArgs
)
if /i "%~1"=="--skip-confirmation" (
    set "SKIP_CONFIRMATION=true"
    goto :ParseArgs
)
echo Unknown option: %~1
goto :ShowHelp

:Parsed

echo Installation prefix: %INSTALL_PREFIX%
echo Binary directory: %BIN_DIR%
echo Library directory: %LIB_DIR%
echo.

if not defined SKIP_CONFIRMATION (
    set /p confirm="Proceed with installation? [y/N]: "
    if /i not "!confirm!"=="y" (
        echo Installation cancelled.
        exit /b 0
    )
)

REM Check dependencies
echo [+] Checking dependencies...
where cmake >nul 2>&1
if errorlevel 1 (
    echo [x] CMake not found. Please install CMake first.
    echo    Download from: https://cmake.org/download/
    exit /b 1
)

where cl >nul 2>&1
if errorlevel 1 (
    where gcc >nul 2>&1
    if errorlevel 1 (
        echo [x] C++ compiler not found. Please install Visual Studio or GCC.
        exit /b 1
    )
)

echo [+] All dependencies satisfied

REM Build project
if not defined SKIP_BUILD (
    echo [+] Building VaultArchive...

    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

    cd /d "%BUILD_DIR%"

    cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%"

    if errorlevel 1 (
        echo [x] CMake configuration failed
        exit /b 1
    )

    cmake --build . --config Release

    if errorlevel 1 (
        echo [x] Build failed
        exit /b 1
    )

    echo [+] Build completed successfully
) else (
    echo [!] Skipping build step
    cd /d "%BUILD_DIR%"
)

REM Create directories
echo [+] Creating installation directories...
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
if not exist "%LIB_DIR%" mkdir "%LIB_DIR%"
if not exist "%INCLUDE_DIR%" mkdir "%INCLUDE_DIR%"

REM Install files
echo [+] Installing VaultArchive...

echo    Installing varc_tool.exe to %BIN_DIR%
copy /y "%BUILD_DIR%\Release\varc_tool.exe" "%BIN_DIR%\" >nul

echo    Installing libvarc.lib to %LIB_DIR%
if exist "%BUILD_DIR%\Release\libvarc.lib" (
    copy /y "%BUILD_DIR%\Release\libvarc.lib" "%LIB_DIR%\" >nul
)

echo    Installing headers to %INCLUDE_DIR%
if exist "%PROJECT_DIR%\src\include\*.hpp" (
    copy /y "%PROJECT_DIR%\src\include\*.hpp" "%INCLUDE_DIR%\" >nul
)
if exist "%PROJECT_DIR%\src\include\varc\" (
    if not exist "%INCLUDE_DIR%\varc\" mkdir "%INCLUDE_DIR%\varc\"
    copy /y "%PROJECT_DIR%\src\include\varc\*.hpp" "%INCLUDE_DIR%\varc\" >nul
)

REM Create batch file for easy use
echo [+] Creating wrapper scripts...

(
    echo @echo off
    echo REM VaultArchive wrapper script
    echo "%BIN_DIR%\varc_tool.exe" %%*
) > "%BIN_DIR%\varc.bat"

echo [+] Installation complete

REM Print summary
echo.
echo ========================================
echo   Installation Complete!
echo ========================================
echo.
echo   Version:      %VERSION%
echo   Binary:       %BIN_DIR%\varc_tool.exe
echo   Library:      %LIB_DIR%\libvarc.lib
echo   Headers:      %INCLUDE_DIR%
echo.
echo Quick Start:
echo   Create archive:   varc_tool create archive.varc .^\files
echo   Extract archive:  varc_tool extract archive.varc .^\output
echo   List contents:    varc_tool list archive.varc
echo   Get help:         varc_tool --help
echo.
echo Documentation:
echo   User Guide:       %PROJECT_DIR%\docs\USER_GUIDE.md
echo   API Reference:    %PROJECT_DIR%\docs\API_REFERENCE.md
echo.

endlocal
exit /b 0

:ShowHelp
echo Usage: install.bat [OPTIONS]
echo.
echo Options:
echo   --prefix=^<path^>  Installation prefix (default: %%ProgramFiles%%\VaultArchive)
echo   --skip-build       Skip build step (assume already built)
echo   --skip-confirmation Skip confirmation prompt
echo   --help, -h         Show this help
echo.
echo Examples:
echo   install.bat
echo   install.bat --prefix=C:\Apps\VaultArchive
echo   install.bat --skip-build --skip-confirmation
echo.
exit /b 0
