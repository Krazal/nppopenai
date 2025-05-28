/**
 * DebugUtils.h - Debugging utilities for NppOpenAI
 *
 * This file declares functions for debugging and troubleshooting the plugin,
 * including functions to display debug information, examine binary data,
 * and toggle debugging features. These functions are primarily intended
 * for development and troubleshooting rather than end-user functionality.
 */

#pragma once
#include <windows.h>
#include <string>
#include "EncodingUtils.h"    // for toUTF8, multiByteToWideChar
#include "../api/OpenAIClient.h" // for replaceSelected

/**
 * Toggles the plugin's debug mode on or off
 *
 * When debug mode is enabled, the plugin will output additional information
 * that can be helpful for diagnosing issues.
 */
void toggleDebugMode();

/**
 * Displays a simple text message in a dialog box for debugging
 *
 * @param text The text to display in the debug dialog
 */
void debugText(const wchar_t *text);

/**
 * Displays a text message with its hexadecimal representation
 *
 * This is useful for examining text data that might contain invisible
 * or non-printable characters, or for diagnosing encoding issues.
 *
 * @param text The text to display and analyze
 */
void debugTextBinary(const wchar_t *text);

/**
 * Displays each character of a string in a separate dialog
 *
 * This allows examining each character individually, which can be helpful
 * for identifying problematic characters in a string.
 *
 * @param text The text to analyze character by character
 */
void debugTextCharByChar(const wchar_t *text);

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
std::string hexDump(const char *data, size_t size);
