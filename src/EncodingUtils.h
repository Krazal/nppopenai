#pragma once
#include <string>
#include <windows.h>

// Convert std::string to std::wstring
std::wstring stringToWstring(const std::string &str);

// Convert wide string to UTF-8
std::string toUTF8(const std::wstring &wide);

// Allow passing wide C-strings directly
inline std::string toUTF8(const wchar_t *w) { return toUTF8(std::wstring(w)); }

// Convert multi-byte UTF-8 string to wide string (allocate new TCHAR[])
TCHAR *multiByteToWideChar(const char *utf8);

// Map legacy calls if needed
inline TCHAR *myMultiByteToWideChar(const char *utf8) { return multiByteToWideChar(utf8); }
