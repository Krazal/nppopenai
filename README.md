```
  _   _             ___                   _    ___
 | \ | |_ __  _ __ / _ \ _ __   ___ _ __ / \  |_ _|
 |  \| | '_ \| '_ \ | | | '_ \ / _ \ '__/ _ \  | |
 | |\  | |_) | |_) | |_| | |_) |  __/ | / ___ \ | |
 |_| \_| .__/| .__/ \___/| .__/ \___|_|/_/   \_\___|
       |_|   |_|         |_|
```

# NppOpenAI — AI Augmentation Without Leaving the Keyboard

> "Because true nerds shouldn't have to reach for the mouse" 🧠⚡

Transform your Notepad++ into a command-line style AI assistant that responds at the speed of thought. Built for developers who value efficiency and keyboard-driven workflows.

## ⌨️ Keyboard-First Workflow

```bash
# SELECT TEXT -> Ctrl+Shift+O -> GET RESULT
# No menus. No dialogs. No mouse.
```

![NppOpenAI Plugin Banner](https://github.com/andrea-tomassi/nppopenai/blob/bfa8e318cb91a7a780a485f3a2bd9743709a3d5a/src/Resources/toolbar_icon_chat_32x32.ico)

### Pure Text Power

- **Code Generation**: `Ctrl+Shift+O` on a comment → get working code
- **Instant Translation**: `Ctrl+Shift+O` on foreign text → English output
- **Document Analysis**: `Ctrl+Shift+O` on long text → concise summary
- **Command Memory**: Your last used prompt stays active - chain commands efficiently

## 🖥️ Terminal-Style Usage Examples

```
# Quick code fix:
// BUG: This function sometimes returns NaN
function add(a,b) { return a+b }
[SELECT] → [Ctrl+Shift+O]
```

```
# Fast translation:
Denne tekst skal oversættes hurtigt.
[SELECT] → [Ctrl+Shift+O]
```

```
# Generate SQL from natural language:
Create a query that finds all users who logged in this week
[SELECT] → [Ctrl+Shift+O]
```

## 🔧 Power User Configuration

Edit your `NppOpenAI.ini` directly for maximum control:

```ini
[API]
secret_key=sk-...
model=gpt-4o-mini
temperature=0.7

[PLUGIN]
keep_question=0  # Replace text vs. append responses
```

## ⚡ Keyboard Efficiency Features

| Shortcut                    | Action                                         | Time Saved                     |
| --------------------------- | ---------------------------------------------- | ------------------------------ |
| **Ctrl+Shift+O**            | Process selected text                          | ~15s vs. copy/paste to browser |
| **Last prompt memory**      | Automatic reuse of previous prompt             | ~5s per operation              |
| **Context-aware responses** | Get exactly what you need                      | Countless minutes              |
| **Replace-mode**            | Responses replace queries for seamless editing | ~3s per edit                   |

## 📦 Quick Setup for the Impatient

```bash
1. Plugins → NppOpenAI → Edit Config
2. secret_key=YOUR_API_KEY
3. Ctrl+S
4. Ready to use
```

## 🧙‍♂️ Custom Prompt Wizardry

Create keyboard-accessible AI personas in your instructions file:

```ini
[Prompt:sql]
Convert this description into a PostgreSQL query.

[Prompt:cpp]
Optimize this C++ code for performance.

[Prompt:regex]
Create a regular expression that matches the described pattern.
```

## 💾 Power User Techniques

- **Keep instructions file open** in one tab for reference
- **Toggle replacement mode** to seamlessly integrate AI into editing
- **Chain prompts** for multi-step processing
- **Watch token count** in the status to optimize prompts

![NppOpenAI in action](https://github.com/andrea-tomassi/nppopenai/blob/f90c9d16a6940ee17d920daeaa9253c8ef1c5674/src/Resources/npp_openai_screen.png)

---

<div align="center">
<code>while(coding){ useAI(); improveCode(); keepHands(ON_KEYBOARD); }</code>
</div>
