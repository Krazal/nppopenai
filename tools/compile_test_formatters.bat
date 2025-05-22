@echo off
REM Compile the test_formatters utility
echo Compiling test_formatters.cpp...

REM Find Visual Studio installation
if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools" (
    set VCVARSALL="C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools" (
    set VCVARSALL="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2019\BuildTools" (
    set VCVARSALL="C:\Program Files\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools" (
    set VCVARSALL="C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
) else (
    echo Could not find Visual Studio BuildTools
    exit /b 1
)

REM Set up Visual Studio environment
call %VCVARSALL% x64

REM Compile formatter tests
cl.exe /EHsc /std:c++17 /I"..\vs.proj\include" test_formatters.cpp ..\src\RequestFormatters.cpp ..\src\EncodingUtils.cpp /link /LIBPATH:"..\vs.proj\x64\lib" libcurl.lib

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed
    exit /b 1
)

echo.
echo Compilation successful! Running tests...
echo.

REM Run the tests
test_formatters.exe
