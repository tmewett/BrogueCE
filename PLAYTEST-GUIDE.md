# Brogue Dungeon Master AI - Playtester Guide

Welcome to the Brogue Dungeon Master AI playtest! This guide will help you set up and test the DM AI system, which enhances the Brogue roguelike experience with dynamic narrative elements.

## What is the DM AI?

The Dungeon Master AI enhances the Brogue gameplay by:

1. Generating atmospheric narratives for key game events
2. Maintaining memory of your encounters and discoveries
3. Creating a more immersive dungeon crawling experience

## Getting Started

### Prerequisites

- Windows 10/11 with PowerShell
- Node.js installed (download from [nodejs.org](https://nodejs.org/))
- Ollama (optional, for LLM-powered narratives)

### Quick Start

1. **Setup the DM Agent**:
   ```powershell
   cd BrogueMCP\playtest
   .\start-server.ps1
   ```

2. **Run a Gameplay Simulation**:
   ```powershell
   cd BrogueMCP\playtest
   .\simulate-gameplay.ps1
   ```

3. **View the Logs**:
   ```powershell
   cd BrogueMCP\playtest
   .\view-logs.ps1
   ```

## Testing Features

### 1. Narrative Generation

The DM AI generates narratives for five key event types:

- **Monster Encounters**: When you meet creatures for the first time
- **Monster Defeats**: After you slay a monster (especially rare ones)
- **Item Discoveries**: When finding interesting or rare items
- **Level Transitions**: Upon entering new dungeon depths
- **Player Death**: A dramatic epitaph for your fallen adventurer

### 2. Memory System

The DM AI maintains:

- **Short-term Memory**: Recent events that inform new narratives
- **Long-term Memory**: Significant events stored persistently
- **Knowledge Bases**: Information about creatures and items you've encountered

### 3. Detailed Logs

Logs are recorded in two formats:

- **Raw logs**: Technical details in `session_[timestamp].log`
- **Adventure logs**: Beautiful, themed narratives in `events_[timestamp].md`

The log viewer (via `view-logs.ps1`) provides an attractive web interface to browse your adventures.

## Advanced Testing

### Testing with Different Adventure Paths

Try the different adventure scenarios:

```powershell
.\simulate-gameplay.ps1 -adventure "The Crystal Caverns"
```

Available adventures:
- The Goblin Caves (beginner)
- The Crystal Caverns (intermediate)
- The Undead Catacombs (advanced)

### Custom Events

You can trigger specific events manually:

```powershell
# Monster encounter
.\monster.ps1 -name "ancient dragon" -level 15 -rare $true

# Item discovery
.\item.ps1 -name "Amulet of Yendor" -rare $true

# Level transition
.\level.ps1 -depth 26 -environment "obsidian vault"
```

## Giving Feedback

As a playtester, we'd like your feedback on:

1. **Narrative Quality**: Are the descriptions atmospheric and enjoyable?
2. **Integration Feel**: Does the DM feel like a natural part of gameplay?
3. **Memory System**: Does the system seem to remember past events?
4. **Bugs**: Any issues you encounter while testing

Please record your thoughts in the `FEEDBACK.md` file in the `playtest` directory.

## Troubleshooting

### Server Not Starting

If the server fails to start:

1. Check that Node.js is installed (`node --version`)
2. Ensure npm packages are installed (`cd BrogueMCP\dm-agent && npm install`)
3. Check for port conflicts on 3001

### No Narrative Responses

If you're not getting narrative responses:

1. Check server console for errors
2. Ensure the event types have `isRare` or `isFirstEncounter` set to true
3. Verify server is running on port 3001

### Log Viewer Issues

If the log viewer isn't working:

1. Make sure there are log files in `BrogueMCP\playtest\logs`
2. Try opening the HTML files directly from `BrogueMCP\playtest\html`

## Thank You!

Your feedback is crucial for developing this system. Thank you for helping make the Brogue experience more immersive and engaging! 