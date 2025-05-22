# Using NppOpenAI with Anthropic Claude

This guide explains how to use NppOpenAI with [Claude](https://www.anthropic.com/claude), a family of AI assistants created by Anthropic.

## Configuration for Claude API

Add the following to your `NppOpenAI.ini` file:

```ini
[API]
secret_key=YOUR_CLAUDE_API_KEY  # Claude API key
api_url=https://api.anthropic.com/v1/
chat_completions_route=messages
response_type=claude
model=claude-3-haiku-20240307  # or another Claude model
```

## Notes

1. You need an API key from Anthropic to use Claude
2. The `response_type=claude` setting enables:
   - Automatic formatting of requests to match Claude's API requirements
   - Proper authentication headers (x-api-key and anthropic-version)
   - Parsing of Claude's unique response format with content arrays
3. Claude models include:
   - claude-3-opus-20240229 (most capable)
   - claude-3-sonnet-20240229 (balanced)
   - claude-3-haiku-20240307 (faster, more efficient)

## Advanced Usage

For specialized use cases, you can create different configurations:

```ini
# For technical documents and complex reasoning
api_url=https://api.anthropic.com/v1/
chat_completions_route=messages
response_type=claude
model=claude-3-opus-20240229
temperature=0.5
max_tokens=4000

# For creative writing
api_url=https://api.anthropic.com/v1/
chat_completions_route=messages
response_type=claude
model=claude-3-sonnet-20240229
temperature=0.9
max_tokens=2000
```

## Troubleshooting

If you encounter issues:

1. Check that your Claude API key is valid and active
2. Ensure you're using a valid model name
3. Verify that `response_type=claude` is correctly set
4. Try the test_api utility in the tools directory to debug API connections
