/**
 * EncodingUtils.cpp - String encoding conversion utilities
 *
 * This file provides utility functions for converting between different string encodings,
 * particularly between UTF-8 and wide character (UTF-16) strings. These functions are
 * essential for proper handling of Unicode text in the Windows API and when communicating
 * with the OpenAI API.
 */

#include "EncodingUtils.h"
#include <codecvt>
#include <locale>

/**
 * Converts a UTF-8 encoded std::string to a UTF-16 (wide) std::wstring
 *
 * Uses the standard library's codecvt facilities with a fallback mechanism
 * in case the conversion fails (for non-valid UTF-8 sequences).
 *
 * @param str UTF-8 encoded string to convert
 * @return UTF-16 encoded wide string
 */
std::wstring stringToWstring(const std::string &str)
{
    try
    {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.from_bytes(str);
    }
    catch (...)
    {
        // Fallback conversion for non-UTF8 input
        std::wstring result;
        for (unsigned char c : str)
        {
            result += static_cast<wchar_t>(c);
        }
        return result;
    }
}

/**
 * Converts a UTF-16 (wide) std::wstring to a UTF-8 encoded std::string
 *
 * Uses the standard library's codecvt facilities with a manual fallback
 * implementation for cases where the conversion fails.
 *
 * @param wide UTF-16 encoded wide string to convert
 * @return UTF-8 encoded string
 */
std::string toUTF8(const std::wstring &wide)
{
    try
    {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.to_bytes(wide);
    }
    catch (...)
    {
        // Manual UTF-8 encoding as fallback
        // This implements the UTF-8 encoding algorithm:
        // - Characters 0-127 are encoded as single bytes
        // - Characters 128-2047 are encoded as two bytes
        // - Characters 2048-65535 are encoded as three bytes
        std::string result;
        for (wchar_t c : wide)
        {
            if (c <= 0x7F)
            {
                result += static_cast<char>(c);
            }
            else if (c <= 0x7FF)
            {
                result += static_cast<char>(0xC0 | ((c >> 6) & 0x1F));
                result += static_cast<char>(0x80 | (c & 0x3F));
            }
            else if (c <= 0xFFFF)
            {
                result += static_cast<char>(0xE0 | ((c >> 12) & 0x0F));
                result += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
                result += static_cast<char>(0x80 | (c & 0x3F));
            }
            else
            {
                result += '?';
            }
        }
        return result;
    }
}

/**
 * Converts a UTF-8 encoded char array to a wide character (UTF-16) TCHAR array
 *
 * Uses the Windows API's MultiByteToWideChar function to perform the conversion.
 *
 * @param utf8 UTF-8 encoded char array to convert
 * @return Pointer to a newly allocated TCHAR array containing the wide character string
 */
TCHAR *multiByteToWideChar(const char *utf8)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    TCHAR *wide = new TCHAR[len];
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wide, len);
    return wide;
}
