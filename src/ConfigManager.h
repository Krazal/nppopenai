#pragma once

#include <windows.h>
#include <string>

// Load or (re)create configuration and plugin settings from INI
void loadConfig(bool loadPluginSettings);

// Write default [API] and [PLUGIN] settings to the INI file
void writeDefaultConfig();

// Open the configuration INI file in Notepad++
void openConfigFile();

// Open the instructions (system prompt) file in Notepad++
void openInstructionsFile();
