/**
 * EncodingUtils.h - String encoding utilities for NppOpenAI
 *
 * This file declares functions for converting between different string formats,
 * particularly between UTF-8 and UTF-16 (wide character) strings, which
 * is essential for proper text handling in the Windows environment.
 */

#pragma once
#include <string>
#include <windows.h>

/**
 * Converts a UTF-8 encoded std::string to a UTF-16 (wide) std::wstring
 *
 * @param str UTF-8 encoded string to convert
 * @return UTF-16 encoded wide string
 */
std::wstring stringToWstring(const std::string &str);

/**
 * Converts a UTF-16 (wide) std::wstring to a UTF-8 encoded std::string
 *
 * @param wide UTF-16 encoded wide string to convert
 * @return UTF-8 encoded string
 */
std::string toUTF8(const std::wstring &wide);

/**
 * Converts a multi-byte string to wide string using Windows API
 *
 * This is an alternative conversion for cases where the standard library
 * conversion might not handle all character sets properly.
 *
 * @param mbstr Multi-byte string to convert
 * @return Wide character string
 */
wchar_t *myMultiByteToWideChar(const char *mbstr);

/**
 * Safe cleanup for dynamically allocated strings
 *
 * @param str Pointer to the string that should be freed
 */
void safeFree(void *str);

/**
 * Allow passing wide C-strings directly
 *
 * @param w Wide character string to convert
 * @return UTF-8 encoded string
 */
inline std::string toUTF8(const wchar_t *w) { return toUTF8(std::wstring(w)); }

/**
 * Convert multi-byte UTF-8 string to wide string (allocate new TCHAR[])
 *
 * @param utf8 UTF-8 encoded string to convert
 * @return Wide character string
 */
TCHAR *multiByteToWideChar(const char *utf8);

/**
 * Map legacy calls if needed
 *
 * @param utf8 UTF-8 encoded string to convert
 * @return Wide character string
 */
inline wchar_t *myMultiByteToWideChar(const char *utf8) { return multiByteToWideChar(utf8); }
