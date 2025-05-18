# NppOpenAI — OpenAI (aka. ChatGPT) plugin for Notepad++

A simple Notepad++ plugin to communicate with OpenAI directly from your favorite code editor.

## How it works?

Simply select your text in Notepad++, press `Ctrl + Shift + O`, and you'll see the AI generated response in seconds. (Additional settings required.) Two examples: "Please create a Fibonacci function in PHP"; "Mi az árvíztűrő tükörfúrógép?" (Hungarian Unicode test). This plugin requires an active internet connection and an OpenAI registration / API key (handles it confidentially).

## Can I install (update) it manually?

Sure! If the latest release here in GitHub is newer than the NppOpenAI plugin in the Plugin Manager, [follow this guide](https://github.com/Krazal/nppopenai/wiki/FAQ#question-ive-an-old-buggy-nppopenai-release-how-should-i-update) to install it manually.

## How to configure?

**To remove your original question/request,** please uncheck Plugins » NppOpenAI » Keep my question option.

For additional settings, please open the configuration file (`NppOpenAI.ini`) from Plugins » NppOpenAI » Edit Config, and edit the `[API]` section. For available plugin settings, see [OpenAI API Reference](https://platform.openai.com/docs/api-reference/completions). You can track your token usage via the `total_tokens_used` setting in `[PLUGIN]` section.

**To send a system message** (I call this “instructions”) along with your question, please open the instructions file (`NppOpenAI_instructions`) from Plugins » NppOpenAI » Edit Instructions. How to use? For example:

- Click Plugins » NppOpenAI » Edit Instructions
- Enter any instruction, like: Please translate the received text into English.
- Save the file
- Click Plugins » NppOpenAI » Load Config
- Open an empty file and enter e.g. Kérlek, mondd, hogy ez egy teszt
- Select the text and press `Ctrl + Shift + O`
- You should get the following result: “Please, say that this is a test”, instead of “Ez egy teszt” (“This is a test”).

## Multiple Prompts

You can now define more than one named system prompt in your `NppOpenAI_instructions` file. Use INI-style sections, for example:

```ini
[Prompt:greet]
You are a helpful assistant. Greet the user warmly.

[Prompt:translate]
Translate the selected text to English.

[Prompt:summary]
Summarize the selected text in bullet points.
```

After saving the file and clicking **Plugins » NppOpenAI » Load Config**, the next time you press `Ctrl + Shift + O` a dialog will appear letting you choose which prompt to apply. Select a prompt, and its content will be sent as the system message to OpenAI.

**Note:** If you define only one prompt (or none), the plugin will apply that prompt (or no system message) automatically without showing a selection dialog.

If you don't want to use “instructions”, please leave the `NppOpenAI_instructions` file empty.

After editing and saving `NppOpenAI.ini` and/or `NppOpenAI_instructions`, please always load your settings: Plugins menu » NppOpenAI » Load Config.

**To enable chat,** please click Plugins » NppOpenAI » Chat: off menu item, and check in the Use chat. You may also increase/decrease chat limit for optimal token usage. To turn off chat, please click Plugins » NppOpenAI » Chat limit: [numeric limit] and turn off Use chat.

The chat can even be used in conjunction with the “instructions”. However, the chat history to be displayed is not (yet) available.

## Have a question?

If you experience an error or are interested in suggestions, please visit the [FAQ page](https://github.com/Krazal/nppopenai/wiki/FAQ)!

## Additional information

Some help how to build cURL with OpenSSL and zlib:
https://developers.refinitiv.com/en/article-catalog/article/how-to-build-openssl--zlib--and-curl-libraries-on-windows

After manually building this plugin, please copy `*.dll` and `cacert.pem` (source: https://curl.se/ca/cacert.pem) files from `vs.proj/helper_files_[platform]` directory to your Notepad++ plugin folder (`C:\Program Files (x86)\Notepad++\plugins\NppOpenAI` by default).

ARM platforms are not supported.
