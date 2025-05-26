# NppOpenAI Developer Cheatsheet

## Quick Build Commands

```bash
# For Command Prompt
cd "C:\Users\andre\VSCode Workspace\nppopenai\vs.proj"
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" "NppPluginTemplate.sln" "/p:configuration=Release" "/p:platform=x64" /v:minimal

# For PowerShell
cd "C:\Users\andre\VSCode Workspace\nppopenai\vs.proj"
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" "NppPluginTemplate.sln" "/p:configuration=Release" "/p:platform=x64"
```

## Project Structure Overview

```
src/api/          # API clients and HTTP functionality
src/editor/       # Scintilla editor abstraction
src/config/       # Configuration management
src/core/         # Core plugin functionality
src/ui/           # User interface components
src/utils/        # Utility functions
```

## Common Development Tasks

### Debugging

1. Set `debugMode = true` in `PluginDefinition.cpp` for verbose logging
2. Use status bar messages for quick debug info:
   ```cpp
   ::SendMessage(nppData._nppHandle, NPPM_SETSTATUSBAR, STATUSBAR_DOC_TYPE, (LPARAM)TEXT("Debug message"));
   ```
3. Use `MessageBox` for critical debugging:
   ```cpp
   MessageBoxA(nppData._nppHandle, "Debug content", "Debug Title", MB_OK);
   ```

### Working with Streaming Responses

1. Ensure `StreamParser.cpp` can handle the API response format
2. For streaming to work:
   - Set `configAPIValue_streaming = L"1"` in the INI file
   - `s_streamTargetScintilla` global must be properly set
   - `WM_OPENAI_STREAM_CHUNK` messages must be processed correctly

### Adding a New API Provider

1. Add request formatter in `RequestFormatters.cpp`
2. Add response parser in `ResponseParsers.cpp`
3. Add streaming support in `StreamParser.cpp`
4. Update HTTP headers in `HTTPClient.cpp`

## Key Components

### 1. HTTPClient

- Handles all cURL communications
- Functions: `performRequest()`, `performStreamingRequest()`
- Set proper headers based on API type

### 2. StreamParser

- Parses different streaming response formats
- Functions: `extractContent()`, `parseOpenAIChunk()`, etc.
- Handles completion detection for different APIs

### 3. EditorInterface

- Abstracts Scintilla editor operations
- Functions: `getCurrentScintilla()`, `getSelectedText()`, `replaceSelectedText()`

### 4. OpenAIClient

- Coordinates the overall request/response process
- Manages streaming callbacks and user interface updates
- Uses global references like `s_streamTargetScintilla`

## Common Errors & Solutions

### 1. Missing Symbols

- Make sure to include `external_globals.h` whenever using global variables
- Check `extern` declarations match actual variable types

### 2. Build Errors

- Verify include paths in VS project settings
- Check library paths for cURL and other dependencies
- Ensure all headers are properly included

### 3. Streaming Not Working

- Check `configAPIValue_streaming = L"1"` is set
- Verify `s_streamTargetScintilla` is initialized correctly
- Enable debug mode to see if chunks are being received
- Make sure `WM_OPENAI_STREAM_CHUNK` handler works correctly
- Check cURL connection settings in `HTTPClient.cpp`

### 4. API Errors

- Verify API key is correct
- Check URL formats in `APIUtils::buildApiUrl()`
- Look for HTTP status codes in error responses
- Enable debug mode to trace request/response details

## Testing After Changes

1. Build the plugin
2. Copy `bin64/NppOpenAI.dll` to your test Notepad++ plugins folder
3. Test both streaming and non-streaming modes
4. Test with different API providers (OpenAI, Claude, Ollama)
5. Check different text selection and insertion behaviors

## Configuration Files

- Main config: Notepad++ plugin INI file
- Instructions/prompts: Text file with system prompts
- Important settings:
  - `streaming=1` for streaming mode
  - `responseType` for API provider selection
  - `debugMode=1` for verbose logging

## Dependencies

- cURL (libcurl.dll) - HTTP requests
- nlohmann/json - JSON parsing
- Windows SDK - Notepad++ plugin interface
- Scintilla - Editor interaction
