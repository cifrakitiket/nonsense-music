@echo off
setlocal

echo ==========================================
echo Nonsense Music - Build and Package Script
echo ==========================================

echo [1/2] Building NonsenseMusic in Release mode...
cmake --version >nul 2>&1
if %errorlevel% == 0 (
    if not exist "build" mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . --config Release
    cd ..
) else (
    echo WARNING: 'cmake' not found in PATH. Skipping build step.
    echo Make sure you have compiled the project in your IDE so that 'NonsenseMusic.exe' exists in the 'build' folder.
)

set EXEPATH=build\Release\NonsenseMusic.exe
if not exist "%EXEPATH%" set EXEPATH=build\NonsenseMusic.exe
if not exist "%EXEPATH%" (
    echo ERROR: Could not find compiled executable. Please compile the project first.
    pause
    exit /b 1
)

echo [2/2] Compiling installer with Inno Setup...

:: Try to find ISCC in PATH
where iscc >nul 2>&1
if %errorlevel% == 0 (
    set ISCC=iscc
    goto run_iscc
)

:: Common Inno Setup paths
set "ISCC=%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe"
if exist "%ISCC%" goto run_iscc

set "ISCC=%ProgramFiles%\Inno Setup 6\ISCC.exe"
if exist "%ISCC%" goto run_iscc

set "ISCC=%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe"
if exist "%ISCC%" goto run_iscc

set "ISCC=%ProgramFiles(x86)%\Inno Setup 5\ISCC.exe"
if exist "%ISCC%" goto run_iscc

echo ERROR: Inno Setup (ISCC.exe) not found.
echo Winget says it is installed, but it is not in the default folders.
echo Please add Inno Setup to your system PATH or compile 'installer.iss' manually by double-clicking it.
pause
exit /b 1

:run_iscc
"%ISCC%" installer.iss
if %errorlevel% == 0 (
    echo.
    echo ==========================================
    echo SUCCESS: Installer created in Release_Installer\NonsenseMusic_Setup.exe
    echo ==========================================
) else (
    echo.
    echo ERROR: Inno Setup failed to compile the script.
)

pause
