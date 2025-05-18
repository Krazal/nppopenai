#include <windows.h>
#include <commctrl.h> // for TASKDIALOG_BUTTON, TaskDialogIndirect
#pragma comment(lib, "comctl32.lib")

#include "PromptManager.h"
#include <fstream>
#include <regex>
#include <cstdio>

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

    while (fgetws(buffer, _countof(buffer), file))
    {
        line = buffer;
        if (!line.empty())
            line.erase(line.find_last_not_of(L"\r\n") + 1);

        if (std::regex_match(line, match, headerPattern))
        {
            if (hasHeader)
                prompts.push_back(current);
            current = Prompt();
            current.name = match[1].str();
            hasHeader = true;
        }
        else if (hasHeader)
        {
            current.content += line + L"\n";
        }
        else
        {
            current.content += line + L"\n";
        }
    }
    fclose(file);
    if (hasHeader || !current.content.empty())
        prompts.push_back(current);
}

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
