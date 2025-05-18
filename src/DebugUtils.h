#pragma once
#include <windows.h>
#include <string>
#include "EncodingUtils.h" // for toUTF8, multiByteToWideChar
#include "OpenAIClient.h"  // for replaceSelected

// Toggle plugin debug mode
void toggleDebugMode();

// Show a simple text debug dialog
void debugText(const wchar_t *text);

// Show debug info with hex dump
void debugTextBinary(const wchar_t *text);

// Show debug info char by char
void debugTextCharByChar(const wchar_t *text);

// Produce hex dump of a buffer
std::string hexDump(const char *data, size_t size);

// Map legacy calls if needed
