@echo off
echo Building NppOpenAI plugin (x64 Release)...

:: Find the Visual Studio installation path
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
  set VS_PATH=%%i
)

if "%VS_PATH%"=="" (
  echo Could not find Visual Studio installation path.
  echo Make sure Visual Studio Build Tools are properly installed.
  pause
  exit /b 1
)

:: Setup the environment
call "%VS_PATH%\Common7\Tools\VsDevCmd.bat"

:: Build the solution
echo Building solution...
MSBuild.exe "%~dp0vs.proj\NppPluginTemplate.sln" /p:configuration=Release /p:platform=x64

if %ERRORLEVEL% NEQ 0 (
  echo Build failed with error code %ERRORLEVEL%.
  pause
  exit /b %ERRORLEVEL%
)

echo.
echo Build completed successfully!
echo The plugin DLL should be in: %~dp0vs.proj\x64\Release\NppOpenAI.dll
echo.
echo To install the plugin:
echo 1. Copy %~dp0vs.proj\x64\Release\NppOpenAI.dll to your Notepad++ plugins folder
echo 2. Copy all files from %~dp0vs.proj\helper_files_x64 to the same folder
echo.
echo Recommend creating a subfolder named "NppOpenAI" in the plugins folder for these files.
echo.

pause