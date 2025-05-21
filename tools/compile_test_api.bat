@echo off
REM Compile the test_api utility
echo Compiling test_api.cpp...

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

REM Compile
cl.exe /EHsc /std:c++17 /I"..\vs.proj\include" test_api.cpp /link /LIBPATH:"..\vs.proj\x64\lib" libcurl.lib

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed
    exit /b 1
)

echo.
echo Compilation successful! Usage examples:

echo.
echo Test OpenAI API:
echo test_api.exe https://api.openai.com/v1/ chat/completions openai YOUR_API_KEY

echo.
echo Test Ollama API (local):
echo test_api.exe http://localhost:11434/ api/generate ollama

echo.
echo Test LM Studio API (local):
echo test_api.exe http://localhost:1234/ v1/chat/completions openai

echo.
echo Test Anthropic Claude API:
echo test_api.exe https://api.anthropic.com/v1/ messages claude YOUR_API_KEY
echo.
