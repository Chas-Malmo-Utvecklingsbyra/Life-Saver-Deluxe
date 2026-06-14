@echo off

if "%1"=="" (
    echo Usage:
    echo.
    echo   build_test.bat file_system
    echo   build_test.bat spiffs
    echo.
    pause
    exit /b
)


if "%1"=="file_system" goto FILE_SYSTEM
if "%1"=="spiffs" goto SPIFFS
if "%1"=="internet" goto INTERNET

echo Unknown test: %1
pause
exit /b

:FILE_SYSTEM

cd /d "%~dp0..\file_system"

gcc -DUNIT_TEST ^
-I. ^
-Imocks ^
-I..\unity ^
..\unity\unity.c ^
mocks\mock_deps.c ^
spiffs\spiffs.c ^
test\main.c ^
test\test_file_system.c ^
-o run_fs_tests.exe

if errorlevel 1 goto BUILD_FAILED

echo.
echo Running File System tests...
echo.

run_fs_tests.exe

goto END


:SPIFFS

cd /d "%~dp0..\file_system"

gcc -DUNIT_TEST ^
-I. ^
-Imocks ^
-I..\unity ^
..\unity\unity.c ^
mocks\mock_deps.c ^
spiffs\spiffs.c ^
test\test_spiffs.c ^
-o run_spiffs_tests.exe

if errorlevel 1 goto BUILD_FAILED

echo.
echo Running SPIFFS tests...
echo.

run_spiffs_tests.exe

goto END

:INTERNET

cd /d "%~dp0..\internet"

gcc -DUNIT_TEST ^
-I. ^
-Imocks ^
-I..\unity ^
..\unity\unity.c ^
mocks\mock_deps.c ^
internet.c ^
test\main.c ^
test\test_internet.c ^
-o run_internet_tests.exe

if errorlevel 1 goto BUILD_FAILED

echo.
echo Running Internet tests...
echo.

run_internet_tests.exe

goto END


:BUILD_FAILED
echo.
echo Build FAILED.

:END
echo.
pause