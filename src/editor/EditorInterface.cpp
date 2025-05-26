#include "EditorInterface.h"
#include "core/external_globals.h"
#include "EncodingUtils.h"

/**
 * Get the handle to the current Scintilla editor
 *
 * @return Handle to the current Scintilla editor instance
 */
HWND EditorInterface::getCurrentScintilla()
{
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return NULL;

    return (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
}

/**
 * Get the currently selected text from a Scintilla editor
 *
 * @param editor Handle to the Scintilla editor
 * @return The selected text or empty string if no selection
 */
std::string EditorInterface::getSelectedText(HWND editor)
{
    Sci_Position selStart = ::SendMessage(editor, SCI_GETSELECTIONSTART, 0, 0);
    Sci_Position selEnd = ::SendMessage(editor, SCI_GETSELECTIONEND, 0, 0);
    Sci_Position selLen = selEnd - selStart;

    if (selLen <= 0)
        return "";

    std::string selectedText(selLen, '\0');
    Sci_TextRangeFull tr;
    tr.chrg.cpMin = selStart;
    tr.chrg.cpMax = selEnd;
    tr.lpstrText = &selectedText[0];
    ::SendMessage(editor, SCI_GETTEXTRANGEFULL, 0, (LPARAM)&tr);

    return selectedText;
}

/**
 * Replace the currently selected text in a Scintilla editor
 *
 * @param editor Handle to the Scintilla editor
 * @param text The text that will replace the selected text
 */
void EditorInterface::replaceSelectedText(HWND editor, const std::string &text)
{
    // Get current selection range
    Sci_Position selStart = ::SendMessage(editor, SCI_GETSELECTIONSTART, 0, 0);
    Sci_Position selEnd = ::SendMessage(editor, SCI_GETSELECTIONEND, 0, 0);

    // Set target range for replacement
    ::SendMessage(editor, SCI_SETTARGETSTART, selStart, 0);
    ::SendMessage(editor, SCI_SETTARGETEND, selEnd, 0);

    // Replace target with new text
    ::SendMessageA(editor, SCI_REPLACETARGET, static_cast<WPARAM>(text.size()),
                   reinterpret_cast<LPARAM>(text.c_str()));
}

/**
 * Insert text at the current cursor position
 *
 * @param editor Handle to the Scintilla editor
 * @param text The text to insert
 */
void EditorInterface::insertTextAtCursor(HWND editor, const std::string &text)
{
    ::SendMessageA(editor, SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(text.c_str()));
}

/**
 * Move the cursor to a specific position
 *
 * @param editor Handle to the Scintilla editor
 * @param position The position to move the cursor to
 */
void EditorInterface::moveCursorTo(HWND editor, Sci_Position position)
{
    ::SendMessage(editor, SCI_GOTOPOS, position, 0);
}

/**
 * Set cursor at the end of the current selection or document
 *
 * @param editor Handle to the Scintilla editor
 */
void EditorInterface::setCursorAtEnd(HWND editor)
{
    Sci_Position currentPos = ::SendMessage(editor, SCI_GETCURRENTPOS, 0, 0);
    ::SendMessage(editor, SCI_SETSEL, currentPos, currentPos);
}

/**
 * Prepare the editor for a streaming response
 *
 * @param editor Handle to the Scintilla editor
 * @param selectedText The selected text (user query)
 * @param keepQuestion Whether to keep the user's question in the response
 * @param responseType The type of response (openai, claude, ollama, etc.)
 */
void EditorInterface::prepareForStreamingResponse(HWND editor, const std::string &selectedText,
                                                  bool keepQuestion, const std::wstring &responseType)
{
    // Get selection range
    Sci_Position selStart = ::SendMessage(editor, SCI_GETSELECTIONSTART, 0, 0);
    ::SendMessage(editor, SCI_SETTARGETSTART, selStart, 0);
    ::SendMessage(editor, SCI_SETTARGETEND, ::SendMessage(editor, SCI_GETSELECTIONEND, 0, 0), 0);

    // Prepare initial text
    std::string initialText = "";
    if (keepQuestion)
    {
        // Use only a single newline for Ollama responses to reduce excessive spacing
        if (responseType == L"ollama")
        {
            initialText = selectedText + "\n";
        }
        else
        {
            initialText = selectedText + "\n\n";
        }
    }

    // Replace the selection with the initial text
    ::SendMessage(editor, SCI_REPLACETARGET, initialText.length(),
                  reinterpret_cast<LPARAM>(initialText.c_str()));

    // Set cursor at the end of the replacement text to ensure streaming content is appended correctly
    Sci_Position textEnd = selStart + initialText.length();
    ::SendMessage(editor, SCI_SETSEL, textEnd, textEnd);
}
