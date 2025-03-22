/**
 * Main integration for Brogue Dungeon Master AI
 *
 * Provides public interface functions for game integration
 */

#ifndef DM_MAIN_H
#define DM_MAIN_H

#include "../brogue/Rogue.h"

// Initialization and cleanup
void initialize_dungeon_master_ai(void);
void cleanup_dungeon_master_ai(void);

// Event handlers for game events
void dm_on_monster_discovered(creature *monst);
void dm_on_monster_killed(creature *monst, char *killedBy);
void dm_on_item_discovered(item *theItem);
void dm_on_new_level(short depth, short stairDirection);
void dm_on_player_died(char *killedBy);

#endif /* DM_MAIN_H */ 