# Dungeon Master AI Playtest Guide

This guide explains how to conduct a playtest of the Dungeon Master AI system with BrogueMCP.

## Prerequisites

- [x] Ollama installed and running with llama3 model
- [x] DM agent server set up and running
- [ ] BrogueMCP compiled with DM agent integration (not completed yet)

## Current State

We've successfully implemented and tested:

1. ✅ DM agent server
2. ✅ Memory management system
3. ✅ Ollama integration with narrative generation
4. ✅ Test event handling

We're still working on:

1. ❌ Compiling BrogueMCP with the C integration code
2. ❌ In-game event capture and display

## Manual Playtest Procedure

Since the full integration isn't complete, here's how to conduct a manual "simulated" playtest:

1. **Start the DM agent server**:
   ```
   cd BrogueMCP/dm-agent
   node server/server.js
   ```

2. **Start Brogue**:
   ```
   cd BrogueMCP/bin
   brogue.exe
   ```

3. **Play the game normally, and at key moments, manually trigger events**:

   In a separate terminal:
   ```
   # When encountering a new monster
   curl -X POST http://localhost:3001/api/event -H "Content-Type: application/json" -d "{\"eventType\":\"MONSTER_ENCOUNTERED\",\"eventData\":{\"monsterName\":\"pink jelly\",\"isFirstEncounter\":true,\"locationDesc\":\"dark chamber\"},\"context\":{\"playerLevel\":1}}"
   
   # When finding a new item
   curl -X POST http://localhost:3001/api/event -H "Content-Type: application/json" -d "{\"eventType\":\"ITEM_DISCOVERED\",\"eventData\":{\"itemName\":\"Staff of Lightning\",\"isRare\":true},\"context\":{\"playerLevel\":1}}"
   
   # When entering a new level
   curl -X POST http://localhost:3001/api/event -H "Content-Type: application/json" -d "{\"eventType\":\"NEW_LEVEL\",\"eventData\":{\"depth\":2,\"environmentType\":\"cavern\"},\"context\":{\"playerLevel\":1}}"
   ```

4. **Review the narrative responses** in the server console.

5. **Check memory storage** in the `memory-bank` directory after gameplay.

## Expected Behaviors

During a proper playtest, you should observe:

1. **Contextual Narratives**: AI-generated descriptions that match the game state
2. **Memory Accumulation**: Growing knowledge of encountered entities
3. **Progressive Context**: Later narratives reference earlier encounters
4. **Fallback Reliability**: System functions even if AI is temporarily unavailable

## Next Steps for Full Integration

1. Update main.c to call our initialization code
2. Resolve build system issues to compile with the MCP code
3. Test hooking of game functions 
4. Implement message rendering in-game

## Reporting Playtest Results

Document your observations, especially:

- Narrative quality and appropriateness
- Response times and performance impact
- Any system errors or failures
- Suggestions for improvement 