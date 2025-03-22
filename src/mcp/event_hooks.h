/**
 * Event hooks for Brogue Dungeon Master AI
 *
 * Hooks into key Brogue functions to capture game events
 */

#ifndef EVENT_HOOKS_H
#define EVENT_HOOKS_H

#include "../brogue/Rogue.h"

// Initialize event hooks for DM agent
void initialize_dm_hooks(void);

// Cleanup event hooks
void cleanup_dm_hooks(void);

// Event handler functions
void handle_monster_encountered(creature *monst);
void handle_monster_killed(creature *monst, char *killedBy);
void handle_item_discovered(item *theItem);
void handle_new_level(short depth, short stairDirection);
void handle_player_died(char *killedBy);

// Response handler
void handle_dm_response(const char *response);

// Set a pending message to be displayed
void set_pending_dm_message(const char *message);

// Display any pending message
void display_pending_dm_messages();

#endif /* EVENT_HOOKS_H */ 