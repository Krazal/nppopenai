# NppOpenAI — Bring AI Power to Notepad++

Transform your Notepad++ experience with direct access to OpenAI's powerful AI models. Ask questions, translate text, generate code, summarize content, and more - without ever leaving your editor.

![NppOpenAI Plugin Banner](https://github.com/Krazal/nppopenai/raw/master/vs.proj/Resources/toolbar_icon_chat_32x32.ico)

## ✨ Quick Start

1. Select text in Notepad++
2. Press `Ctrl + Shift + O`
3. Get AI-generated responses instantly!

## 🚀 What Can You Do With NppOpenAI?

### 💻 Code Like a Pro

```
// Select this comment and press Ctrl+Shift+O
// Write a function that calculates the Fibonacci sequence in PHP
```

### 🌐 Translate Anything

```
Mi az árvíztűrő tükörfúrógép?
(Select this Hungarian text and press Ctrl+Shift+O to see it translated)
```

### 📝 Summarize Documents

Select long texts, press `Ctrl+Shift+O`, and get concise summaries that retain the key points.

### 🤔 Get Answers to Complex Questions

Whether it's technical explanations, algorithms, or creative ideas - just ask!

## 🛠️ Setup Made Simple

1. **Configure Your API Key**:

   - Go to Plugins » NppOpenAI » Edit Config
   - Add your OpenAI API key in the [API] section
   - Set other preferences like model type and temperature

2. **Load Your Settings**:

   - Click Plugins » NppOpenAI » Load Config

3. **Start Using AI**:
   - Select text
   - Press `Ctrl+Shift+O`
   - Watch the magic happen!

## ✅ Smart Customization

### Create Custom AI Assistants

NppOpenAI supports multiple AI personalities through custom prompts. Create specialized helpers for:

- **Code Reviews** 🔍
- **Language Translation** 🌍
- **Technical Writing Assistance** ✏️
- **Data Analysis** 📊
- **Creative Writing** 📚

Simply open your instructions file (Plugins » NppOpenAI » Edit Instructions) and add prompts like:

```ini
[Prompt:translate]
Translate the selected text to English.

[Prompt:code_review]
Review this code for bugs and suggest improvements.

[Prompt:summarize]
Summarize the selected text in bullet points.
```

### Chat Functionality

Enable chat mode to maintain conversation context between requests:

1. Click Plugins » NppOpenAI » Chat: off
2. Check "Use chat" and set your preferred history limit
3. Enjoy contextual conversations with the AI!

## ⚙️ Additional Settings

- **Remove Original Questions**: Uncheck Plugins » NppOpenAI » Keep my question
- **Adjust Response Style**: Modify temperature and other parameters in the config file
- **Track Token Usage**: Monitor your API usage in the [PLUGIN] section of NppOpenAI.ini

## 📚 Example Prompt Templates

### Translation Assistant

```ini
[Prompt:Translate_2_EN]
Translate the provided text into English while maintaining its original meaning, tone and layout.

# Output Format
The translation should be delivered in English. Start the translation directly without prefacing it with any additional context or commentary.

# Notes
- Ensure that nuances and idiomatic expressions from the original text are accurately translated to retain their original intent and meaning.
- Pay special attention to grammar and style to ensure the translated text is easy to read and understand.
```

### Code Helper

```ini
[Prompt:code_assistant]
Analyze the provided code and help improve it. Consider:
- Bugs or logical errors
- Performance optimizations
- Better coding practices
- Clearer naming conventions

Respond with specific suggestions and example code when appropriate.
```

### Document Summarizer

```ini
[Prompt:Summarize]
Summarize the selected text while retaining its original meaning and key points. Use dynamic reasoning to determine whether to keep the overall structure for long documents or merge the content into a single paragraph for shorter documents.

# Output Format
For longer documents, retain the existing structure in the summary but condensed. For shorter documents, deliver the summary as a single, coherent paragraph in English.

# Notes
- Ensure that the main ideas and critical information from the original text are accurately captured.
- Pay special attention to grammar and style to ensure the summarized text is easy to read and understand.
```

## 🆘 Need Help?

Have a question or experiencing an issue? Visit our [FAQ page](https://github.com/Krazal/nppopenai/wiki/FAQ) for solutions to common problems.

---

**💡 Pro Tip**: Keep your instructions file open in one tab while working in another for seamless AI assistance without disrupting your workflow.

Unleash the power of AI directly in your favorite editor with NppOpenAI!
