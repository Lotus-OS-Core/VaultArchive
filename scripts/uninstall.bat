@echo off
REM
REM VaultArchive Uninstallation Script for Windows
REM Removes all installed VaultArchive files
REM

setlocal EnableDelayedExpansion

REM Configuration
set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."
set "INSTALL_PREFIX=%ProgramFiles%\VaultArchive"
set "BIN_DIR=%INSTALL_PREFIX%\bin"
set "LIB_DIR=%INSTALL_PREFIX%\lib"
set "INCLUDE_DIR=%INSTALL_PREFIX%\include"

REM Files to remove
set "BINARY_FILES=varc_tool.exe varc.bat varc.exe"
set "LIBRARY_FILES=libvarc.lib"

:Main
echo.
echo ========================================
echo   VaultArchive Uninstaller (Windows)
echo ========================================
echo.

REM Parse arguments
:ParseArgs
if "%~1"=="" goto :Parsed
if /i "%~1"=="--purge" (
    set "PURGE=true"
    goto :ParseArgs
)
if /i "%~1"=="--skip-confirmation" (
    set "SKIP_CONFIRMATION=true"
    goto :ParseArgs
)
if /i "%~1"=="--help" goto :ShowHelp
if /i "%~1"=="-h" goto :ShowHelp
echo Unknown option: %~1
goto :ShowHelp

:Parsed

REM Check if installation exists
set "FILES_FOUND=0"
for %%F in (%BINARY_FILES%) do (
    if exist "%BIN_DIR%\%%F" set /a FILES_FOUND+=1
)
for %%F in (%LIBRARY_FILES%) do (
    if exist "%LIB_DIR%\%%F" set /a FILES_FOUND+=1
)

if "%FILES_FOUND%"=="0" (
    echo [!] No VaultArchive files found to remove
    echo.
    echo The library may not be installed, or installed to a different location.
    echo.
    echo You can specify a custom installation prefix:
    echo   set INSTALL_PREFIX=C:\Apps\VaultArchive
    echo   uninstall.bat
    exit /b 0
)

echo Found %FILES_FOUND% file(s) to remove

if not defined SKIP_CONFIRMATION (
    set /p confirm="Remove VaultArchive installation? [y/N]: "
    if /i not "!confirm!"=="y" (
        echo Uninstallation cancelled.
        exit /b 0
    )
)

REM Uninstallation steps
echo.
echo [-] Removing files...

set "TOTAL_REMOVED=0"

REM Remove binary files
echo.
echo    Binary files:
for %%F in (%BINARY_FILES%) do (
    if exist "%BIN_DIR%\%%F" (
        del /f /q "%BIN_DIR%\%%F" >nul 2>&1
        if not exist "%BIN_DIR%\%%F" (
            echo [+] Removed: %BIN_DIR%\%%F
            set /a TOTAL_REMOVED+=1
        )
    )
)

REM Remove library files
echo.
echo    Library files:
for %%F in (%LIBRARY_FILES%) do (
    if exist "%LIB_DIR%\%%F" (
        del /f /q "%LIB_DIR%\%%F" >nul 2>&1
        if not exist "%LIB_DIR%\%%F" (
            echo [+] Removed: %LIB_DIR%\%%F
            set /a TOTAL_REMOVED+=1
        )
    )
)

REM Remove header files
echo.
echo    Header files:
set "HEADERS_REMOVED=0"
for %%F in (VarcHeader.hpp VarcEntry.hpp CryptoEngine.hpp CompressionEngine.hpp Archive.hpp) do (
    if exist "%INCLUDE_DIR%\%%F" (
        del /f /q "%INCLUDE_DIR%\%%F" >nul 2>&1
        if not exist "%INCLUDE_DIR%\%%F" (
            set /a HEADERS_REMOVED )
    )
   +=1
        if exist "%INCLUDE_DIR%\varc\%%F" (
        del /f /q "%INCLUDE_DIR%\varc\%%F" >nul 2>&1
        if not exist "%INCLUDE_DIR%\varc\%%F" (
            set /a HEADERS_REMOVED+=1
        )
    )
)
echo [+] Removed %HEADERS_REMOVED% header file(s)

REM Remove empty directories
echo.
echo    Directories:
if exist "%BIN_DIR%" (
    for /f %%i in ('dir /b "%BIN_DIR%" 2^>nul') do set "DIR_EMPTY=false"
    if not defined DIR_EMPTY (
        rmdir "%BIN_DIR%" 2>nul
        if not exist "%BIN_DIR%" echo [+] Removed empty directory: %BIN_DIR%
    )
)

if exist "%INCLUDE_DIR%\varc" (
    for /f %%i in ('dir /b "%INCLUDE_DIR%\varc" 2^>nul') do set "INCLUDE_VAR_EMPTY=false"
    if not defined INCLUDE_VAR_EMPTY (
        rmdir "%INCLUDE_DIR%\varc" 2>nul
        if not exist "%INCLUDE_DIR%\varc" echo [+] Removed empty directory: %INCLUDE_DIR%\varc
    )
)

REM Print summary
echo.
echo ========================================
echo   Uninstallation Complete!
echo ========================================
echo.
echo   Files removed: %TOTAL_REMOVED%
echo.
echo Note:
echo   - Some configuration files may remain
echo   - Your archives (.varc files) are not affected
echo   - To reinstall, run: scripts\install.bat
echo.

endlocal
exit /b 0

:ShowHelp
echo Usage: uninstall.bat [OPTIONS]
echo.
echo Options:
echo   --purge             Also remove configuration files
echo   --skip-confirmation Skip confirmation prompt
echo   --help, -h          Show this help
echo.
echo Examples:
echo   uninstall.bat
echo   uninstall.bat --skip-confirmation
echo.
exit /b 0
