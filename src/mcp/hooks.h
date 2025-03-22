/**
 * Hooks for integrating Dungeon Master AI into Brogue
 *
 * Provides hooks into key Brogue functions
 */

#ifndef HOOKS_H
#define HOOKS_H

// Setup hooks into the game
void setupDMHooks(void);

// Install DM AI - called from main.c
void installDungeonMasterAI(void);

#endif /* HOOKS_H */ 