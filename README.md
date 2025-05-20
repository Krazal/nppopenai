```
   \  |              _ \                      \   _ _| 
    \ | __ \  __ \  |   | __ \   _ \ __ \    _ \    |  
  |\  | |   | |   | |   | |   |  __/ |   |  ___ \   |  
 _| \_| .__/  .__/ \___/  .__/ \___|_|  _|_/    _\___| 
       _|    _|          _|                            
```

# NppOpenAI — AI Augmentation Without Leaving the Keyboard

> "Because true nerds shouldn't have to reach for the mouse" 🧠⚡

Transform your Notepad++ into a command-line style AI assistant that responds at the speed of thought. Built for developers who value efficiency and keyboard-driven workflows.

## ⌨️ Keyboard-First Workflow

```bash
# SELECT TEXT -> Ctrl+Shift+O -> GET RESULT
# No menus. No dialogs. No mouse.
```

[//]: # "[TODO] Create a fine banner. This was previously used:  ![NppOpenAI Plugin Banner](https://github.com/andrea-tomassi/nppopenai/blob/bfa8e318cb91a7a780a485f3a2bd9743709a3d5a/src/Resources/toolbar_icon_chat_32x32.ico)"

### Pure Text Power

- **Code Generation**: `Ctrl+Shift+O` on a comment → get working code
- **Instant Translation**: `Ctrl+Shift+O` on foreign text → English output
- **Document Analysis**: `Ctrl+Shift+O` on long text → concise summary
- **Command Memory**: Your last used prompt stays active - chain commands efficiently
- **Keyboard Navigation**: Pop-up remembers your last prompt, navigate with arrow keys, activate with Enter - zero mouse required

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
# Drop-in YAML repair workflow:
1. Ctrl+V (paste problematic YAML)
2. Ctrl+A (select all)
3. Ctrl+Shift+O
=> BOOM! Correctly formatted YAML replaces broken content
```

```
# Generate SQL from natural language:
Create a query that finds all users who logged in this week
[SELECT] → [Ctrl+Shift+O]
```

```
# Complete fix-and-paste workflow:
1. Ctrl+V (paste problematic code/config)
2. Ctrl+A (select all)
3. Ctrl+Shift+O (fix automatically)
4. Ctrl+A (select fixed content)
5. Ctrl+C (copy solution)
6. Ctrl+V (paste back in source system)
=> Job done in seconds!
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

| Shortcut                                   | Action                                         | Time Saved                     |
| ------------------------------------------ | ---------------------------------------------- | ------------------------------ |
| **Ctrl+Shift+O**                           | Process selected text                          | ~15s vs. copy/paste to browser |
| **Arrow keys + Enter**                     | Navigate and select prompts without mouse      | ~8s per prompt selection       |
| **Ctrl+V, Ctrl+A, Ctrl+Shift+O, Ctrl+A+C** | Complete paste→fix→copy workflow               | ~45s per troubleshooting cycle |
| **Last prompt memory**                     | Automatic reuse of previous prompt             | ~5s per operation              |
| **Context-aware responses**                | Get exactly what you need                      | Countless minutes              |
| **Replace-mode**                           | Responses replace queries for seamless editing | ~3s per edit                   |

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

Check out our [advanced prompt examples](INSTRUCTIONS_EXAMPLES.txt) for more sophisticated AI interactions, including technical writing, code fixing, and Node-RED function development.

## 💾 Power User Techniques

- **Keep instructions file open** in one tab for reference
- **Toggle replacement mode** to seamlessly integrate AI into editing
- **Chain prompts** for multi-step processing
- **Watch token count** in the status to optimize prompts

## 🔄 The Rapid-Fire AI Workflow

> _"When keyboard warriors meet AI acceleration"_

Picture this: A senior developer faces a complex refactoring task involving legacy code in an unfamiliar language, documentation that needs translation, and configuration files that require debugging.

Instead of context-switching between browsers, translators, and documentation sites, they unleash the full power of NppOpenAI with Notepad++'s native features:

1. **Multi-Tab AI Orchestra**:  
   With config files in one tab, code in another, and docs in a third, they rapidly switch contexts without losing focus
2. **Prompt Chaining Sequence**:

   ```
   [Tab 1] Select error message → Ctrl+Shift+O (Fix) → Ctrl+Z (Compare) → Ctrl+Y (Reapply)
   [Tab 2] Select foreign comment → Ctrl+Shift+O (Translate) → Arrow down → Enter (Switch prompt) → Ctrl+Shift+O (Analyze)
   [Tab 3] Select entire file → Ctrl+Shift+O (Reformat) → Select section → Ctrl+Shift+O (Optimize)
   ```

3. **Trial and Error Without Fear**:  
   When one AI approach doesn't yield perfect results, Ctrl+Z steps back, then a slight prompt adjustment with arrow keys and Enter tries a new angle—all without touching the mouse or losing context

In minutes, what would have taken hours of research and context-switching is done. The developer has translated documentation, fixed configuration errors, and optimized code—all through rapid-fire keyboard commands, fluid tab navigation, and AI augmentation working in perfect harmony.

![NppOpenAI in action](https://github.com/andrea-tomassi/nppopenai/blob/f90c9d16a6940ee17d920daeaa9253c8ef1c5674/src/Resources/npp_openai_screen.png)

---

<div align="center">
<code>while(coding){ useAI(); improveCode(); keepHands(ON_KEYBOARD); }</code>
</div>
