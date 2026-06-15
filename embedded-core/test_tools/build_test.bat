@echo off

if "%1"=="" (
    echo Usage:
    echo.
    echo   build_test.bat file_system
    echo   build_test.bat spiffs
    echo   build_test.bat internet
    echo   build_test.bat arena
    echo   build_test.bat random
    echo   build_test.bat i2c
    echo   build_test.bat all
    echo.
    pause
    exit /b
)


if "%1"=="file_system" goto FILE_SYSTEM
if "%1"=="spiffs" goto SPIFFS
if "%1"=="internet" goto INTERNET
if "%1"=="arena" goto ARENA
if "%1"=="random" goto RANDOM
if "%1"=="i2c" goto I2C
if "%1"=="all" goto ALL

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
file_system.c ^
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
file_system.c ^
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

:ARENA

cd /d "%~dp0..\allocator"

gcc -DUNIT_TEST ^
-I. ^
-Imocks ^
-I..\unity ^
..\unity\unity.c ^
arena.c ^
test\main.c ^
test\test_arena.c ^
-o run_arena_tests.exe

if errorlevel 1 goto BUILD_FAILED

echo.
echo Running Arena tests...
echo.

run_arena_tests.exe

goto END

:RANDOM

cd /d "%~dp0..\random"

gcc -DUNIT_TEST ^
-I. ^
-Imocks ^
-I..\unity ^
..\unity\unity.c ^
mocks\mock_deps.c ^
random.c ^
test\main.c ^
test\test_random.c ^
-o run_random_tests.exe

if errorlevel 1 goto BUILD_FAILED

echo.
echo Running Random tests...
echo.

run_random_tests.exe

goto END

:I2C

cd /d "%~dp0..\i2c"

gcc -DUNIT_TEST ^
-I. ^
-Imocks ^
-I..\unity ^
..\unity\unity.c ^
mocks\mock_deps.c ^
i2c.c ^
test\main.c ^
test\test_i2c.c ^
-o run_i2c_tests.exe

if errorlevel 1 goto BUILD_FAILED

echo.
echo Running I2C tests...
echo.

run_i2c_tests.exe

goto END

:ALL

call "%~f0" file_system nopause
if errorlevel 1 goto END

call "%~f0" spiffs nopause
if errorlevel 1 goto END

call "%~f0" internet nopause
if errorlevel 1 goto END

call "%~f0" arena nopause
if errorlevel 1 goto END

call "%~f0" random nopause
if errorlevel 1 goto END

call "%~f0" i2c nopause
if errorlevel 1 goto END

echo.
echo All unit tests completed successfully!

goto END


:BUILD_FAILED
echo.
echo Build FAILED.

:END
echo.

if "%2"=="nopause" exit /b

pause