#pragma once
#include <windows.h>
#include <string>
#include "Sci_Position.h"
#include "Scintilla.h"

/**
 * EditorInterface - A module for interacting with the Scintilla editor
 *
 * This namespace contains functions for working with the Notepad++ Scintilla editor,
 * including getting and setting text, handling selections, and cursor positioning.
 * It abstracts the details of sending Scintilla messages to make the main code cleaner.
 */
namespace EditorInterface
{
    // Get the current Scintilla editor handle
    HWND getCurrentScintilla();

    // Get selected text from editor (returns the text or empty string if no selection)
    std::string getSelectedText(HWND editor);

    // Replace selected text in editor
    void replaceSelectedText(HWND editor, const std::string &text);

    // Insert text at current cursor position
    void insertTextAtCursor(HWND editor, const std::string &text);

    // Move cursor to specific position
    void moveCursorTo(HWND editor, Sci_Position position);

    // Set cursor at the end of the document
    void setCursorAtEnd(HWND editor);

    // Prepare editor for streaming response
    void prepareForStreamingResponse(HWND editor, const std::string, bool keepQuestion, const std::wstring &responseType); // & selectedText
}
