/**
 * ConfigManager.h - Configuration management interface for NppOpenAI
 *
 * This file declares functions for loading, saving, and managing the plugin's
 * configuration settings. It handles reading from and writing to the INI file,
 * as well as opening configuration files for user editing.
 */

#pragma once

#include <windows.h>
#include <string>

/**
 * Loads configuration settings from the INI file
 *
 * @param loadPluginSettings If true, also loads plugin-specific settings like
 *                         chat settings and UI preferences. If false, only loads
 *                         API-related configuration.
 */
void loadConfig(bool loadPluginSettings);

/**
 * Writes default configuration settings to the INI file
 *
 * Creates a new INI file with documentation comments and default
 * values for all API and plugin settings.
 */
void writeDefaultConfig();

/**
 * Opens the configuration INI file in Notepad++ for editing
 *
 * Allows users to directly modify plugin settings through
 * the Notepad++ interface.
 */
void openConfigFile();

/**
 * Opens the instructions file (system prompts) in Notepad++ for editing
 *
 * Allows users to create or modify system messages that provide context
 * for AI requests.
 */
void openInstructionsFile();
