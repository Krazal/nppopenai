# NppOpenAI — OpenAI (aka. ChatGPT) plugin for Notepad++
A simple Notepad++ plugin to communicate with OpenAI directly from your favorite code editor.

How it works?
-------------

Simply select your text in Notepad++, press `Ctrl + Shift + O`, and you'll see the AI generated response in seconds. (Additional settings required.) Some examples: "Please create a Fibonacci function in PHP"; "Mi az árvíztűrő tükörfúrógép?" (Hungarian Unicode test) etc. This plugin requires an active internet connection and an OpenAI registration / API key (handles it confidential).

How to configure?
-----------------

To remove your original question/request, please uncheck Plugins » NppOpenAI » Keep my question option.

For additional settings please open the configuration file (`NppOpenAI.ini`) from Plugins » NppOpenAI » Edit Config, and edit the `[API]` section. For available plugin settings see [Open AI API Reference](https://platform.openai.com/docs/api-reference/completions). Besides, you can track your token usage by `total_tokens_used` setting in `[PLUGIN]` section.

After editing and saving `NppOpenAI.ini` please load your settings: Plugins menu » NppOpenAI » Load Config.

Additional information
----------------------

Some help how to build cURL with OpenSSL and zlib:
https://developers.refinitiv.com/en/article-catalog/article/how-to-build-openssl--zlib--and-curl-libraries-on-windows

After manual building this plugin, please copy `*.dll` AND `cacert.pem` (source: https://curl.se/ca/cacert.pem) files from `vs.proj/Debug` directory to your Notepad++ plugin folder.

This plugin is built for Notepad++ 32-bit.
