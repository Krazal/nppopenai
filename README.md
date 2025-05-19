# NppOpenAI — Bring AI Power to Notepad++

Transform your Notepad++ experience with direct access to OpenAI’s powerful AI models—unleash your coding potential with swift code generation, on-demand translations, and intelligent summaries right inside your favorite editor. Develop faster, dream bigger!

![NppOpenAI Plugin Banner](https://github.com/andrea-tomassi/nppopenai/blob/f90c9d16a6940ee17d920daeaa9253c8ef1c5674/src/Resources/npp_openai_screen.png)

## ✨ Quick Start

1. Select text in Notepad++
2. Press `Ctrl + Shift + O`
3. Get AI-generated responses instantly!
   > A loading dialog will appear, spinning with anticipation as your chosen AI model tackles questions, translations, and more. Get ready to supercharge your workflow!

## 🚀 What Can You Do With NppOpenAI?

![NppOpenAI Plugin Banner](https://github.com/andrea-tomassi/nppopenai/blob/bfa8e318cb91a7a780a485f3a2bd9743709a3d5a/src/Resources/toolbar_icon_chat_32x32.ico)

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

Whether it’s refactoring tricky code, brainstorming for your next project, or exploring new concepts—NppOpenAI has your back. Join a global community of devs discovering limitless possibilities with AI assistance at their fingertips!

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

### English Technical Writing

```ini
[Prompt:Write_in_En]
Produce grammatically correct, well-structured English text for IT/Cybersecurity contexts.

# Output Format
Provide the text directly in English without extra commentary.

# Notes
- Maintain technical fidelity and clarity
- Use standardized cybersecurity terminology (NIST, ISO 27001)
```

### Italian Technical Writing

```ini
[Prompt:Write_in_It]
Produci testo italiano chiaro e tecnicamente accurato per contesti IT/Cybersecurity.

# Formato di Output
Fornisci direttamente il testo in italiano senza commenti aggiuntivi.

# Note
- Rispetta la terminologia standardizzata (GDPR, Clusit, NIS2)
- Mantieni la struttura e l’accuratezza del contenuto originale
```

### Node-RED Flow Assistant

```ini
[Prompt:Node-RED]
You are a Node-RED expert. Generate production-ready function node code.

# Core Guidelines
1. Use async operations carefully
2. Handle errors with structured payloads
3. Only reference Setup Tab modules, never use require()
```

## 🆘 Need Help?

Have a question or experiencing an issue? Visit our [FAQ page](https://github.com/Krazal/nppopenai/wiki/FAQ) for solutions to common problems.

---

**💡 Pro Tip**: Keep your instructions file open in one tab while working in another for seamless AI assistance without disrupting your workflow.

Unleash the power of AI in your favorite editor with NppOpenAI—where creativity meets productivity anytime, anywhere!
