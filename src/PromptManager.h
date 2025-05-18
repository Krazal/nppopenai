#pragma once
#include <vector>
#include <string>
#include <windows.h>

// Represents a named system prompt
struct Prompt
{
    std::wstring name;
    std::wstring content;
};

// Parse the instructions file into Prompt entries
void parseInstructionsFile(const WCHAR *filePath, std::vector<Prompt> &prompts);

// Present a dialog to select one of the prompts; returns index or -1
int choosePrompt(HWND owner, const std::vector<Prompt> &prompts, int lastUsedIndex);
