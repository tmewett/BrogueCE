# Brogue Dungeon Master AI

This project implements a Dungeon Master AI agent for BrogueMCP, enhancing the gameplay experience with dynamic narrative elements powered by Ollama's local LLM capabilities.

## Overview

The Dungeon Master AI enhances the Brogue roguelike experience by:

1. Generating atmospheric narratives for significant game events
2. Maintaining memory of player encounters and discoveries
3. Providing contextually relevant descriptions based on game state
4. Creating a more immersive dungeon crawling experience

## Features

- **AI-Powered Descriptions**: The DM agent generates rich, atmospheric descriptions for key game events
- **Memory System**: Maintains memory of past events to create coherent narratives
- **Narrator Personality System**: Customizable storytelling voice with presets like Gandalf, Galadriel, and Aragorn
- **Integration with Brogue**: Seamless integration with the game through the MCP system
- **Adjustable Depth**: Configure how much narrative content is generated

## Project Architecture

The system consists of two main components:

1. **Game Integration (C)** - Hooks into the BrogueMCP codebase to capture events and display narratives
2. **DM Agent Server (Node.js)** - Processes game events, manages memory, and generates narratives with Ollama

```
BrogueMCP/
├── src/
│   ├── mcp/                # MCP integration code (C)
│   │   ├── mcp_client.c    # HTTP client for communicating with DM server
│   │   ├── event_hooks.c   # Game event capture and message handling
│   │   ├── dm_main.c       # Main integration API
│   │   └── hooks.c         # Function hooks into the game
├── dm-agent/               # DM Agent server (Node.js)
│   ├── server/             # Express server
│   ├── memory/             # Memory management
│   ├── narrative/          # Narrative generation
│   ├── ollama/             # Ollama integration
│   └── memory-bank/        # Stored memories
```

## Technical Details

### Game Events

The system captures these key game events:

- **Monster Encounters**: First-time or rare monster sightings
- **Monster Deaths**: Especially for rare or powerful creatures  
- **Item Discoveries**: Finding important or rare items
- **Level Transitions**: Entering new dungeon levels
- **Player Death**: End-of-game narrative

### Memory System

The memory system maintains:

1. **Short-term Memory**: Recent events for immediate context
2. **Long-term Memory**: Significant events stored persistently
3. **Knowledge Bases**: Information about monsters and items

### Narrative Generation

Narratives are generated using:

1. **Context-Aware Prompts**: Built from game state and memory
2. **Ollama LLM**: Local language model for text generation
3. **Fallback System**: Pre-written responses if LLM unavailable

## Setup and Installation

### Prerequisites

- Node.js 14+
- Ollama with llama3 model
- C compiler (gcc/clang)
- BrogueMCP source code

### Installation Steps

1. **Install Ollama**:
   - Download from [ollama.ai](https://ollama.ai)
   - Pull the llama3 model: `ollama pull llama3`

2. **Set up the DM Agent Server**:
   ```bash
   cd BrogueMCP/dm-agent
   cp .env.sample .env  # Edit if needed
   npm install
   ```

3. **Build BrogueMCP with DM Agent Support**:
   - The Makefile has been modified to include our MCP code
   - Build as normal: `make bin/brogue`

## Usage

1. **Start the DM Agent Server**:
   ```bash
   cd BrogueMCP/dm-agent
   npm start
   ```

2. **Start Brogue**:
   ```bash
   cd BrogueMCP
   ./bin/brogue  # or bin\brogue.exe on Windows
   ```

3. **Play as normal** - the DM agent will automatically enhance gameplay with narrative elements.

## Testing

To test the DM agent without running the full game:

```bash
cd BrogueMCP
./test-dm.bat  # Windows
# OR
bash test-dm.sh  # Linux/Mac
```

This will:
1. Start the DM agent server
2. Send test events to verify it's working
3. Display the responses

## Configuration

### DM Agent Server

Edit `.env` file in the `dm-agent` directory:

```
# Server configuration
PORT=3000

# Ollama configuration
OLLAMA_URL=http://localhost:11434
OLLAMA_MODEL=llama3
OLLAMA_TEMPERATURE=0.7
OLLAMA_MAX_TOKENS=512

# Memory bank configuration
MEMORY_BANK_PATH=../memory-bank
```

### Narrative Style

Modify the system prompts in `dm-agent/narrative/generator.js` to change the tone and style of the narrative responses.

## Implementation Notes

### C Integration

The project uses function hooking to integrate with BrogueMCP:
- Hooks into `initializeRogue()` for initialization
- Captures events through game functions
- Communicates via HTTP with the DM agent server

### DM Agent Server

The server implements:
- RESTful API for game communication
- Memory management for persistence
- Ollama integration for narrative generation

## Extending the System

### Adding New Event Types

1. Add a new event handler in `event_hooks.c`
2. Add event processing in `server.js`
3. Create a system prompt in `generator.js`

### Customizing Memory System

Modify `manager.js` to change how memories are:
- Selected for storage
- Prioritized for retrieval
- Used for context building

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

## Narrator Personality System

The narrator personality system allows players to customize the storytelling voice of the game through the following features:

- **Adjustable Personality Traits**: Fine-tune attributes like voice tone, wisdom level, verbosity, and more
- **Thematic Tendencies**: Control nature references, metaphor complexity, cosmic awareness, etc.
- **Speech Pattern Control**: Adjust question frequency, syntax complexity, archaic language use, and more 
- **Preset Personalities**: Choose from built-in presets inspired by Tolkien's characters
- **Custom Presets**: Create and save your own narrator personalities
- **In-Game Access**: Press 'N' during gameplay to open the settings UI in your browser

For more information, see the [Narrator System Documentation](../dm-agent/README-NARRATOR.md). 