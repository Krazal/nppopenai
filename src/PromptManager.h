/**
 * PromptManager.h - System prompt management for NppOpenAI
 *
 * This file defines the data structure and functions for handling
 * system prompts (instructions) that can be used with OpenAI requests.
 * It supports parsing multiple named prompts from a file and
 * presenting them to the user for selection.
 */

#pragma once
#include <vector>
#include <string>
#include <windows.h>

/**
 * Represents a named system prompt
 *
 * Each prompt has a name (displayed in the selection dialog)
 * and content (the actual text sent to the OpenAI API).
 */
struct Prompt
{
    std::wstring name;    // Display name of the prompt
    std::wstring content; // Full text content of the prompt
};

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
void parseInstructionsFile(const WCHAR *filePath, std::vector<Prompt> &prompts);

/**
 * Displays a dialog for the user to select one of the available prompts
 *
 * @param owner Parent window handle for the dialog
 * @param prompts Vector of available prompts to choose from
 * @param lastUsedIndex Index of the last used prompt (for default selection)
 * @return Index of the selected prompt, or -1 if canceled or no selection
 */
int choosePrompt(HWND owner, const std::vector<Prompt> &prompts, int lastUsedIndex);
