# Building NppOpenAI Plugin

This document provides instructions on how to build the NppOpenAI plugin for Notepad++ from source code.

## Prerequisites

Before you can build the plugin, you need to have the following tools installed:

1. **Visual Studio Build Tools** (or full Visual Studio)

   - Download from: https://visualstudio.microsoft.com/downloads/ under "Tools for Visual Studio"
   - Required components:
     - MSVC C++ build tools
     - Windows SDK
   - The minimum required version is Visual Studio 2017

2. **Git** (optional, for cloning the repository)
   - Download from: https://git-scm.com/downloads

## Build Methods

There are three ways to build the NppOpenAI plugin:

### Method 1: Using the Batch File

This is the simplest method:

1. Open a Command Prompt or PowerShell window
2. Navigate to the project directory:
   ```
   cd path\to\nppopenai
   ```
3. Run the batch file:
   ```
   build-x64.bat
   ```
4. The batch script will:
   - Locate your Visual Studio installation
   - Set up the build environment
   - Build the x64 Release configuration
   - Tell you where to find the output files

### Method 2: Using Visual Studio Code Tasks

If you're using Visual Studio Code:

1. Open the project folder in VS Code
2. Press `Ctrl+Shift+P` to open the Command Palette
3. Type "Tasks: Run Build Task" and select it
4. Choose one of the available build configurations:
   - `build-x86-debug` - For 32-bit debug build
   - `build-x64-debug` - For 64-bit debug build
   - `build-x86-release` - For 32-bit release build
   - `build-x64-release` - For 64-bit release build

### Method 3: Using Developer Command Prompt

1. Open the "Developer Command Prompt for VS" from the Start menu
2. Navigate to the project directory:
   ```
   cd path\to\nppopenai\vs.proj
   ```
3. Build using MSBuild:
   ```
   MSBuild.exe NppPluginTemplate.sln /p:configuration=Release /p:platform=x64
   ```

## Output Files

After a successful build, you will find the output files in:

- For x64 Release build: `vs.proj\x64\Release\NppOpenAI.dll` and `bin64\NppOpenAI.zip`
- For x86 Release build: `vs.proj\Release\NppOpenAI.dll` and `bin\NppOpenAI.zip`
- For x64 Debug build: `vs.proj\x64\Debug\NppOpenAI.dll`
- For x86 Debug build: `vs.proj\Debug\NppOpenAI.dll`

The build process also creates a zip file containing all necessary files for distribution.

## Installing the Plugin

To install the plugin in Notepad++:

### Using the ZIP file (Recommended)

1. Locate the `NppOpenAI.zip` file in the `bin64` (for x64) or `bin` (for x86) directory
2. Extract this zip file to your Notepad++ plugins directory:
   - For 64-bit Notepad++: `C:\Program Files\Notepad++\plugins\NppOpenAI\`
   - For 32-bit Notepad++: `C:\Program Files (x86)\Notepad++\plugins\NppOpenAI\`
3. Restart Notepad++

### Manual Installation

1. Create a folder named `NppOpenAI` in your Notepad++ plugins directory
2. Copy the built DLL to this folder
3. Copy all helper files from `vs.proj\helper_files_x64\` (for x64) or `vs.proj\helper_files_x86\` (for x86) to the same folder
4. Restart Notepad++

## Troubleshooting

### Build Tools Not Found

If you receive an error that MSBuild.exe or other build tools cannot be found, ensure that:

- Visual Studio Build Tools are properly installed
- You're using the Developer Command Prompt that comes with Visual Studio
- The PATH environment variable includes the directory containing MSBuild.exe

### Missing DLL Dependencies

If the plugin fails to load in Notepad++, check that all the helper DLL files are in the correct location:

- libcrypto-3-x64.dll (or libcrypto-3.dll for x86)
- libcurl.dll
- libssl-3-x64.dll (or libssl-3.dll for x86)
- zlib1.dll
- cacert.pem (required for SSL connections)

## Required Dependencies

The NppOpenAI plugin requires the following dependencies:

- cURL with OpenSSL and zlib
- nlohmann/json library (header-only, included in the project)

These libraries are included in the project repository in the `vs.proj\helper_files_x64\` and `vs.proj\helper_files_x86\` directories, so you don't need to install them separately.
