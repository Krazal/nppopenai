/**
 * DebugUtils.cpp - Debugging utilities for NppOpenAI
 *
 * This file provides functions for debugging purposes, including displaying debug messages,
 * examining binary data, and toggling the plugin's debug mode. These functions are primarily
 * intended for development and troubleshooting rather than end-user functionality.
 */

#include "DebugUtils.h"
#include "external_globals.h"
#include <windows.h>
#include <sstream>
#include <iomanip>

/**
 * Toggles the plugin's debug mode on/off
 *
 * When debug mode is enabled, the plugin will output additional information
 * that can be helpful for diagnosing issues.
 */
void toggleDebugMode()
{
    debugMode = !debugMode;
    MessageBox(nppData._nppHandle,
               debugMode ? TEXT("Debug mode enabled.") : TEXT("Debug mode disabled."),
               TEXT("Debug Mode"), MB_OK);
}

/**
 * Displays a simple text message in a dialog box for debugging
 *
 * @param text The text to display in the debug dialog
 */
void debugText(const wchar_t *text)
{
    MessageBox(nppData._nppHandle, text, TEXT("Debug"), MB_OK);
}

/**
 * Displays a text message with its hexadecimal representation
 *
 * This is useful for examining text data that might contain invisible
 * or non-printable characters, or for diagnosing encoding issues.
 *
 * @param text The text to display and analyze
 */
void debugTextBinary(const wchar_t *text)
{
    std::wstring msg(text);
    std::string utf8 = toUTF8(msg);
    std::string dump = hexDump(utf8.c_str(), utf8.size());
    MessageBoxA(nppData._nppHandle, dump.c_str(), "Debug Binary", MB_OK);
}

/**
 * Displays each character of a string in a separate dialog
 *
 * This allows examining each character individually, which can be helpful
 * for identifying problematic characters in a string.
 *
 * @param text The text to analyze character by character
 */
void debugTextCharByChar(const wchar_t *text)
{
    std::wstring msg(text);
    for (wchar_t c : msg)
    {
        wchar_t ch[2] = {c, 0};
        MessageBox(nppData._nppHandle, ch, TEXT("Debug Char"), MB_OK);
    }
}

/**
 * Creates a hexadecimal representation of binary data
 *
 * Formats the data as a hex dump with bytes grouped in rows of 16,
 * showing both hex values and ASCII representation when possible.
 *
 * @param data Pointer to the data buffer
 * @param size Size of the data in bytes
 * @return Formatted string containing the hex dump
 */
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
