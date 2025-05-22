/**
 * PromptManager.cpp - System prompt management functionality
 *
 * This file handles reading, parsing, and presenting system prompts that can be
 * used as instructions for AI requests. It supports multiple named prompts in an
 * INI-style format and provides a UI for users to select which prompt to use.
 */

#include <windows.h>
#include <commctrl.h> // for TASKDIALOG_BUTTON, TaskDialogIndirect
#pragma comment(lib, "comctl32.lib")

#include "PromptManager.h"
#include <fstream>
#include <regex>
#include <cstdio>

/**
 * Parses the instructions file containing system prompts
 *
 * The file can contain multiple prompts in INI-style format:
 * [Prompt:name]
 * Prompt content here...
 *
 * If no section headers are found, the entire file content is treated as a single prompt.
 *
 * @param filePath Path to the instructions/prompts file
 * @param prompts Output vector that will be filled with parsed prompts
 */
void parseInstructionsFile(const WCHAR *filePath, std::vector<Prompt> &prompts)
{
    FILE *file = _wfopen(filePath, L"r, ccs=UNICODE");
    if (!file)
        return;

    std::wstring line;
    WCHAR buffer[4096];
    std::wregex headerPattern(LR"(^\[Prompt:([^\]]+)\])");
    std::wsmatch match;
    Prompt current;
    bool hasHeader = false;

	// Check if the file is empty or contains only BOM characters
    fseek(file, 0, SEEK_END);  
    std::streamsize fileSize = ftell(file);
    rewind(file);
    fseek(file, 0, SEEK_SET);
    if (fileSize == 0) {
        prompts.push_back(current);
        fclose(file);
        return;
    }
    if (fileSize == 2) {  
        std::vector<uint8_t> buffer(2);  
        if (fread(buffer.data(), 1, 2, file) == 2) {  
            if (buffer[0] == 0xFF && buffer[1] == 0xFE) {
                prompts.push_back(current);
                fclose(file);  
                return;  
            }  
        }  
    }

    while (fgetws(buffer, _countof(buffer), file))
    {
        line = buffer;
        if (!line.empty())
            line.erase(line.find_last_not_of(L"\r\n") + 1);

        if (std::regex_match(line, match, headerPattern))
        {
            // If we found a new header and already have a prompt in progress,
            // save the current one before starting a new one
            if (hasHeader)
                prompts.push_back(current);
            current = Prompt();
            current.name = match[1].str();
            hasHeader = true;
        }
        else if (hasHeader)
        {
            // Add line to current named prompt
            current.content += line + L"\n";
        }
        else
        {
            // No headers found yet, add to default prompt
            current.content += line + L"\n";
        }
    }
    fclose(file);
    // Save the final prompt
    if (hasHeader || !current.content.empty())
        prompts.push_back(current);
}

/**
 * Displays a dialog for the user to choose a system prompt
 *
 * This function uses TaskDialogIndirect to present a list of available prompts
 * to the user. The user can select one prompt, and the function returns the index
 * of the selected prompt.
 *
 * @param owner Handle to the parent window
 * @param prompts Vector of available prompts
 * @param lastUsedIndex Index of the last used prompt, used to preselect a default
 * @return Index of the selected prompt, or -1 if the dialog was canceled
 */
int choosePrompt(HWND owner, const std::vector<Prompt> &prompts, int lastUsedIndex)
{
    size_t count = prompts.size();
    if (count <= 1)
        return 0;

    std::vector<std::wstring> labels(count);
    std::vector<TASKDIALOG_BUTTON> buttons(count);
    for (size_t i = 0; i < count; ++i)
    {
        buttons[i].nButtonID = 1000 + int(i);
        labels[i] = prompts[i].name.empty() ? L"(default)" : prompts[i].name;
        buttons[i].pszButtonText = labels[i].c_str();
    }

    TASKDIALOGCONFIG config = {};
    config.cbSize = sizeof(config);
    config.hwndParent = owner;
    config.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION;
    config.pszWindowTitle = L"NppOpenAI: Choose Prompt";
    config.pszMainInstruction = L"Select a system prompt:";
    config.cButtons = UINT(count);
    config.pButtons = buttons.data();
    config.nDefaultButton = 1000 + (lastUsedIndex >= 0 && lastUsedIndex < int(count) ? lastUsedIndex : 0);

    int pressed = 0;
    if (FAILED(TaskDialogIndirect(&config, &pressed, nullptr, nullptr)) || pressed == 0)
        return -1;
    return pressed - 1000;
}
