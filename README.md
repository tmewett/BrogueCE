
# BrogueMCP: AI-Enhanced Dungeon Adventures 🧙‍♂️🔮

> *Countless adventurers before you have descended this torch-lit staircase,
> seeking the promised riches below. As you reach the bottom and step into
> the wide cavern, the doors behind you seal with a powerful magic...*
>
> ***Welcome to the Dungeons of Doom — Now with an AI Dungeon Master!***

<img src="docs/images/broguemcp-banner.png" alt="BrogueMCP Banner" width="800"/>

## 🌟 What is BrogueMCP?

BrogueMCP takes the beloved roguelike Brogue and transforms it with an **AI-powered Dungeon Master** that narrates your adventures with rich, atmospheric descriptions. Imagine playing Brogue, but with a storyteller that brings your journey to life - describing eerie caverns, horrifying monsters, and powerful artifacts with vivid detail!

- 🎮 **Classic Roguelike Gameplay** - All the strategic depth of the original Brogue
- 🤖 **AI Storytelling** - Dynamic narrative that responds to your actions
- 🧠 **Customizable Narrator** - Choose how your story is told (Gandalf, anyone?)
- 🔄 **Persistent Memory** - The DM remembers your adventures for coherent storytelling

*"As you enter the dark chamber, the air grows thick with the stench of decay. A pink jelly quivers in the corner, its gelatinous form illuminated by an otherworldly glow..."*

## 🚀 Quick Start

### Playing the Game

1. **[Download the latest release](https://github.com/yourusername/BrogueMCP/releases)**
2. **Install [Ollama](https://ollama.ai) and download the llama3 model**:
   ```
   ollama pull llama3
   ```
3. **Start the DM Agent**:
   ```
   cd BrogueMCP/dm-agent
   npm install  # First time only
   npm start
   ```
4. **Launch BrogueMCP**:
   - **Windows**: Run `brogue.exe`
   - **Mac**: Open the app (right-click and choose "Open" the first time)
   - **Linux**: Run `./brogue` (ensure you have SDL2 and SDL2_image installed)

5. **Begin your adventure!** The DM will narrate key moments as you play.

## ✨ Special Features

### 🔮 The AI Dungeon Master

Your digital Dungeon Master enhances gameplay by adding:

- **Rich Descriptions** of monsters, items, and environments
- **Atmospheric Narration** of key events in your adventure
- **Progressive Storytelling** that builds a coherent narrative
- **Enhanced Immersion** without changing core gameplay

### 🧙‍♂️ Narrator Personality System

<img src="docs/images/narrator-ui.png" alt="Narrator Settings UI" width="600"/>

Press `N` during gameplay to customize your storytelling experience:

- **Choose Presets** like Gandalf, Galadriel, or Aragorn from *Lord of the Rings*
- **Adjust Attributes** such as wisdom, verbosity, and formality
- **Fine-tune Themes** like nature references, metaphor use, and cosmic awareness
- **Create Your Own** storyteller voice and save it as a preset

*"You would be wise to tread carefully in these depths, for not all darkness yields to the simple flame of a torch..."* — Gandalf-style narration

## 🛠️ For Developers

BrogueMCP is perfect for developers interested in game AI and procedural storytelling:

### Project Architecture

```
BrogueMCP/
├── src/              # Core game code (C)
│   └── mcp/          # MCP integration code
├── dm-agent/         # DM Agent server (Node.js)
│   ├── server/       # Express server
│   ├── memory/       # Memory management
│   ├── narrative/    # Narrative generation
│   └── public/       # UI for narrator settings
└── memory-bank/      # Stored game memories
```

### Getting Started with Development

1. **Clone the repository**:
   ```
   git clone https://github.com/yourusername/BrogueMCP.git
   cd BrogueMCP
   ```

2. **Set up the DM Agent**:
   ```
   cd dm-agent
   cp .env.sample .env
   npm install
   ```

3. **Build the game**:
   ```
   # See BUILD.md for detailed instructions
   make bin/brogue
   ```

4. **Test the DM Agent separately**:
   ```
   # Windows
   ./test-dm.bat
   
   # Linux/Mac
   bash test-dm.sh
   ```

### Contributing

We welcome contributions! See our [contribution guide](CONTRIBUTING.md) for all the ways you can help make BrogueMCP better.

## 📚 Documentation

- [DM AI Documentation](BrogueMCP/README-DM-AI.md) - Technical details about the AI system
- [Narrator System Guide](BrogueMCP/dm-agent/README-NARRATOR.md) - How to customize your storyteller
- [Playtest Guide](BrogueMCP/README-PLAYTEST.md) - How to test and provide feedback
- [Wiki](https://brogue.fandom.com/wiki/Brogue_Wiki) - Original Brogue wiki
- [Original website](https://sites.google.com/site/broguegame/)

## 👥 Community

- [Forum (Reddit)](https://www.reddit.com/r/brogueforum/)
- [Roguelikes Discord](https://discord.gg/9pmFGKx) (we have a #brogue channel)
- [##brogue on Libera Chat](https://kiwiirc.com/nextclient/irc.libera.chat/##brogue)

## 🎮 Screenshots

<div style="display: flex; justify-content: space-between;">
    <img src="docs/images/gameplay-1.png" alt="Gameplay with narration" width="45%"/>
    <img src="docs/images/narrator-settings.png" alt="Narrator settings" width="45%"/>
</div>

## 📝 License

MIT License - See LICENSE.txt for details.

---

> *The dungeon beckons, adventurer. What tales will be told of your journey?*
