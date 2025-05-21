# NppOpenAI with Different LLM Backends

NppOpenAI supports various LLM backends through its flexible endpoint configuration, request formatting, and response parsing system. The plugin can communicate directly with each backend using its native API format, without requiring an adapter layer.

> **For developers:** If you want to add support for additional LLM APIs, see the [API Integration Guide](api_integration_guide.md).

## Supported LLM Server Types

| Server           | API URL                                 | Endpoint Path       | Response Type |
| ---------------- | --------------------------------------- | ------------------- | ------------- |
| OpenAI           | https://api.openai.com/v1/              | chat/completions    | openai        |
| Azure OpenAI     | https://{resource}.openai.azure.com/... | (empty)             | openai        |
| Ollama           | http://localhost:11434/                 | api/generate        | ollama        |
| Anthropic Claude | https://api.anthropic.com/v1/           | messages            | claude        |
| LM Studio        | http://localhost:1234/                  | v1/chat/completions | openai        |
| vLLM             | http://localhost:8000/                  | v1/completions      | openai        |
| Local AI         | http://localhost:8080/                  | v1/chat/completions | openai        |
| Simple API       | http://localhost:5000/                  | api/generate        | simple        |

## Configuration Examples

### OpenAI API (Default)

```ini
[API]
api_url=https://api.openai.com/v1/
chat_completions_route=chat/completions
response_type=openai
model=gpt-4o-mini
```

### Azure OpenAI

```ini
[API]
api_url=https://your-resource.openai.azure.com/openai/deployments/your-deployment-name/
chat_completions_route=
response_type=openai
model=gpt-35-turbo
```

### Ollama (Direct)

```ini
[API]
api_url=http://localhost:11434/
chat_completions_route=api/generate
response_type=ollama
model=llama3
```

When using Ollama API (`response_type=ollama`), the plugin will:

1. Format requests using Ollama's specific format with `prompt` instead of messages
2. Map system prompts to Ollama's `system` parameter
3. Convert parameters like `max_tokens` to Ollama's `num_predict`
4. Parse responses from Ollama's `response` field

### Anthropic Claude API

```ini
[API]
api_url=https://api.anthropic.com/v1/
chat_completions_route=messages
response_type=claude
model=claude-3-opus-20240229
```

When using Claude API (`response_type=claude`), the plugin will:

1. Format requests using Claude's specific API format
2. Use `x-api-key` header instead of Bearer token
3. Add the required `anthropic-version` header
4. Parse responses using Claude's content array format

### LM Studio

```ini
[API]
api_url=http://localhost:1234/
chat_completions_route=v1/chat/completions
response_type=openai
model=llama3
```

### vLLM / FastChat / LocalAI (OpenAI Compatible Servers)

```ini
[API]
api_url=http://localhost:8000/
chat_completions_route=v1/chat/completions
response_type=openai
model=TheBloke/zephyr-7B-beta
```

## Advanced: Creating Custom Response Parsers

If you're using a server with a non-standard JSON response format, you can add a custom parser by:

1. Adding a new parser function in `ResponseParsers.cpp`
2. Registering the parser in the `getParserForEndpoint` function
3. Adding the new response type to the configuration documentation

This allows for maximum flexibility when working with custom or experimental LLM server implementations.
