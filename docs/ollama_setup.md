# Using NppOpenAI with Ollama

This guide explains how to use NppOpenAI with [Ollama](https://ollama.com/), a tool for running large language models locally.

## Configuration for Ollama

Add the following to your `NppOpenAI.ini` file:

```ini
[API]
secret_key=  # No key needed for local Ollama
api_url=http://localhost:11434/
route_chat_completions=api/generate
response_type=ollama
model=llama3  # or any model you have pulled in Ollama
```

## Notes

1. Ollama needs to be running on your machine when using NppOpenAI
2. Make sure you've pulled your preferred model with `ollama pull llama3` (or another model)
3. The `response_type=ollama` setting enables:
   - Automatic formatting of requests to match Ollama's native API format
   - Proper parameter conversion (e.g., max_tokens → num_predict)
   - Parsing responses from Ollama's native format
4. No API key is required for local Ollama instances

## Advanced Usage

For specialized use cases, you can create different configurations for different models:

```ini
# For creative writing tasks
api_url=http://localhost:11434/
route_chat_completions=api/generate
response_type=ollama
model=mistral
temperature=0.9

# For code generation
api_url=http://localhost:11434/
route_chat_completions=api/generate
response_type=ollama
model=codellama
temperature=0.3
```

## Troubleshooting

If you encounter issues:

1. Make sure Ollama is running (`ollama serve` in a terminal)
2. Check that your model is installed (`ollama list`)
3. Ensure the port (11434) is not blocked by firewall
4. Try a simple query through the Ollama web UI first to verify model functionality
