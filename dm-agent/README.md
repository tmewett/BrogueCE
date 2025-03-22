# Brogue Dungeon Master AI Agent

This project implements a Dungeon Master AI agent for BrogueMCP, enhancing gameplay with dynamic narrative elements powered by Ollama's local LLM.

## Overview

The Dungeon Master AI enhances the Brogue roguelike experience by:

1. Generating atmospheric narratives for significant game events
2. Maintaining memory of player encounters and discoveries
3. Providing contextually relevant descriptions based on game state
4. Creating a more immersive dungeon crawling experience

## Project Structure

```
BrogueMCP/
├── src/
│   ├── mcp/                # MCP integration code (C)
│   │   ├── mcp_client.c    # Client for communicating with DM server
│   │   ├── event_hooks.c   # Game event hooks
│   │   └── dm_main.c       # Main integration point
├── dm-agent/               # DM Agent server (Node.js)
│   ├── server/             # Express server
│   ├── memory/             # Memory management
│   ├── narrative/          # Narrative generation
│   ├── ollama/             # Ollama integration
│   └── memory-bank/        # Stored memories
```

## Requirements

### Game Integration
- C compiler (gcc/clang)
- BrogueMCP source code

### DM Agent Server
- Node.js 14+
- Ollama with llama3 model

## Setup Instructions

### 1. Install Ollama

First, install Ollama following the instructions at [ollama.ai](https://ollama.ai).

Then pull the llama3 model:

```bash
ollama pull llama3
```

### 2. Set up the DM Agent Server

```bash
cd BrogueMCP/dm-agent
cp .env.sample .env  # Edit if needed
npm install
npm start
```

This will start the DM agent server on port 3000 (or as configured in .env).

### 3. Build BrogueMCP with DM Agent Support

#### 3.1 Update the Makefile

Add the MCP client files to the build:

```makefile
# Add to Makefile
MCP_SOURCES = src/mcp/mcp_client.c src/mcp/event_hooks.c src/mcp/dm_main.c
MCP_OBJECTS = $(MCP_SOURCES:.c=.o)

# Add to BROGUE_OBJECTS
BROGUE_OBJECTS += $(MCP_OBJECTS)

# Add required libraries
LDLIBS += -lpthread
```

#### 3.2 Build Brogue

Follow the standard build instructions for your platform in BUILD.md, for example:

```bash
make bin/brogue
```

## Usage

1. Start the DM agent server:
   ```bash
   cd BrogueMCP/dm-agent
   npm start
   ```

2. Start Brogue:
   ```bash
   cd BrogueMCP
   ./bin/brogue
   ```

3. Play normally - the DM agent will automatically enhance your gameplay with narrative elements.

## Customization

### Narrative Style

You can customize the narrative style by editing the system prompts in `dm-agent/narrative/generator.js`.

### Event Selection

Control which events trigger narratives by modifying the `shouldEnhanceEvent` function in `dm-agent/server/server.js`.

### Memory Management

Adjust how memories are stored and retrieved in `dm-agent/memory/manager.js`.

## Troubleshooting

### No DM Narration

- Check that the DM server is running (`npm start` in dm-agent directory)
- Ensure Ollama is running with llama3 model loaded
- Check console logs for errors

### Server Connection Issues

- Verify server URL in the client config (src/mcp/mcp_client.c)
- Check for firewall or network issues
- Try running `curl http://localhost:3000` to test the API

## License

MIT 