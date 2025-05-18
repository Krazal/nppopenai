// filepath: src/DebugUtils.cpp
#include "DebugUtils.h"
#include "external_globals.h"
#include <windows.h>
#include <sstream>
#include <iomanip>

// Toggle plugin debug mode flag
void toggleDebugMode()
{
    debugMode = !debugMode;
    MessageBox(nppData._nppHandle,
               debugMode ? TEXT("Debug mode enabled.") : TEXT("Debug mode disabled."),
               TEXT("Debug Mode"), MB_OK);
}

// Show a simple text debug dialog
void debugText(const wchar_t *text)
{
    MessageBox(nppData._nppHandle, text, TEXT("Debug"), MB_OK);
}

// Show debug info with hex dump
void debugTextBinary(const wchar_t *text)
{
    std::wstring msg(text);
    std::string utf8 = toUTF8(msg);
    std::string dump = hexDump(utf8.c_str(), utf8.size());
    MessageBoxA(nppData._nppHandle, dump.c_str(), "Debug Binary", MB_OK);
}

// Show debug info char by char
void debugTextCharByChar(const wchar_t *text)
{
    std::wstring msg(text);
    for (wchar_t c : msg)
    {
        wchar_t ch[2] = {c, 0};
        MessageBox(nppData._nppHandle, ch, TEXT("Debug Char"), MB_OK);
    }
}

// Produce hex dump of a buffer
std::string hexDump(const char *data, size_t size)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < size; ++i)
    {
        oss << std::setw(2) << (static_cast<unsigned int>(static_cast<unsigned char>(data[i]))) << ' ';
        if ((i + 1) % 16 == 0)
            oss << '\n';
    }
    return oss.str();
}
