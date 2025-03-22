#include "event_hooks.h"
#include <stdio.h>
#include "dm_main.h"
#include "mcp_client.h"
#include <string.h>
#include <stdlib.h>

/**
 * Initialize the Dungeon Master AI integration
 * 
 * This should be called during game initialization
 */
void initialize_dungeon_master_ai(void) {
    printf("Initializing Dungeon Master AI...\n");
    initialize_dm_hooks();
}

/**
 * Cleanup the Dungeon Master AI integration
 * 
 * This should be called during game shutdown
 */
void cleanup_dungeon_master_ai(void) {
    printf("Cleaning up Dungeon Master AI...\n");
    cleanup_dm_hooks();
}

/**
 * Process monster discovery event
 * 
 * This should be called when the player sees a monster
 */
void dm_on_monster_discovered(creature *monst) {
    if (monst) {
        handle_monster_encountered(monst);
    }
}

/**
 * Process monster killed event
 * 
 * This should be called when a monster is killed
 */
void dm_on_monster_killed(creature *monst, char *killedBy) {
    if (monst) {
        handle_monster_killed(monst, killedBy);
    }
}

/**
 * Process item discovery event
 * 
 * This should be called when the player discovers an item
 */
void dm_on_item_discovered(item *theItem) {
    if (theItem) {
        handle_item_discovered(theItem);
    }
}

/**
 * Process new level event
 * 
 * This should be called when the player enters a new level
 */
void dm_on_new_level(short depth, short stairDirection) {
    handle_new_level(depth, stairDirection);
}

/**
 * Process player death event
 * 
 * This should be called when the player dies
 */
void dm_on_player_died(char *killedBy) {
    handle_player_died(killedBy);
}

// Function to open a URL in the default browser
void open_narrator_settings() {
    #ifdef _WIN32
    system("start http://localhost:3001/narrator");
    #elif __APPLE__
    system("open http://localhost:3001/narrator");
    #else // Linux and others
    system("xdg-open http://localhost:3001/narrator");
    #endif
}

// Main initialization function for DM agent
boolean initialize_dm_agent() {
    // Initialize event hooks
    initialize_dm_hooks();
    
    printf("Dungeon Master AI agent initialized\n");
    return true;
}

// Handle a keypress event for DM agent
boolean dm_handle_keypress(int key) {
    // 'N' key opens the narrator settings
    if (key == 'N') {
        printf("Opening Narrator Settings UI...\n");
        open_narrator_settings();
        return true;
    }
    
    return false;
}

// Cleanup function for DM agent
void cleanup_dm_agent() {
    cleanup_dm_hooks();
    printf("Dungeon Master AI agent cleaned up\n");
}

// Handle game events
void dm_handle_player_move(short x, short y) {
    // Not needed for now
}

void dm_handle_monster_seen(int monsterID, char *monsterName, short x, short y) {
    // Not needed for now
}

void dm_handle_item_discovered(int itemID, char *itemName) {
    // Not needed for now
} 