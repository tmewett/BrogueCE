#include "event_hooks.h"
#include "mcp_client.h"
#include <string.h>
#include <stdio.h>

// Store original function pointers for restoration
static void (*original_display_message)(char *) = NULL;

// Message buffer for DM messages
static char dm_message_buffer[4096] = {0};
static int has_pending_message = 0;

// Location tracking to detect new monster encounters
static unsigned long encountered_monsters[10000] = {0}; // Simple bitmap for tracking

// Enhanced message display function
static void dm_enhanced_display_message(char *msg) {
    // Call original function
    original_display_message(msg);
    
    // Display any pending DM messages after regular game messages
    display_pending_dm_messages();
}

// Initialize event hooks
void initialize_dm_hooks(void) {
    // Initialize the MCP client
    if (initialize_mcp_client() != MCP_SUCCESS) {
        printf("Failed to initialize MCP client. DM agent will be disabled.\n");
        return;
    }
    
    // Register response handler
    register_response_handler(handle_dm_response);
    
    // Store original function pointers
    original_display_message = &message;
    
    // Replace with our enhanced functions
    // Note: In a real implementation, we would need to be more careful about how
    // we hook into the game's functions. This is a simplified example.
    
    // Clear the encountered monsters tracking
    memset(encountered_monsters, 0, sizeof(encountered_monsters));
    
    printf("DM Agent hooks initialized successfully.\n");
}

// Cleanup event hooks
void cleanup_dm_hooks(void) {
    // Restore original function pointers
    // (not actually doing this in our simplified example)
    
    // Shutdown MCP client
    shutdown_mcp_client();
    
    printf("DM Agent hooks cleaned up.\n");
}

// Check if a monster has been encountered before
static int is_first_encounter(unsigned long monsterID) {
    if (monsterID >= 10000) return 0; // Bounds check
    
    unsigned long bucket = monsterID / 64;
    unsigned long bit = monsterID % 64;
    unsigned long mask = 1UL << bit;
    
    if (encountered_monsters[bucket] & mask) {
        return 0; // Already encountered
    } else {
        // Mark as encountered
        encountered_monsters[bucket] |= mask;
        return 1; // First encounter
    }
}

// Get a simple location description
static void get_location_description(char *desc, int maxLen, short x, short y) {
    // Simplified location description - in a real implementation we'd use
    // the game's actual terrain information more extensively
    
    strcpy(desc, "dark chamber");
    
    // This is placeholder logic - we would access the actual game state in a real implementation
    /*
    if (pmap[x][y].flags & HAS_ITEM) {
        strcpy(desc, "room with items");
    } else if (pmap[x][y].flags & IS_IN_MACHINE) {
        strcpy(desc, "unusual chamber");
    } else if (pmap[x][y].flags & IS_CORRIDOR) {
        strcpy(desc, "narrow corridor");
    } else if (pmap[x][y].layers[LIQUID]) {
        if (pmap[x][y].layers[LIQUID] == DEEP_WATER) {
            strcpy(desc, "flooded chamber");
        } else {
            strcpy(desc, "shallow pool");
        }
    }
    */
}

// Handle monster encounters
void handle_monster_encountered(creature *monst) {
    if (!monst) return;
    
    // Create JSON event data
    char event_data[2048];
    char location_desc[100];
    get_location_description(location_desc, 100, monst->xLoc, monst->yLoc);
    
    int is_first = is_first_encounter(monst->info.monsterID);
    
    snprintf(event_data, sizeof(event_data),
             "{\"monsterName\":\"%s\",\"monsterLevel\":%d,\"isRare\":%s,\"isFirstEncounter\":%s,\"locationDesc\":\"%s\"}",
             monst->info.monsterName,
             monst->info.level,
             (monst->info.flags & MONST_RARE) ? "true" : "false",
             is_first ? "true" : "false",
             location_desc);
    
    // Only send event for first encounters or rare monsters
    if (is_first || (monst->info.flags & MONST_RARE)) {
        send_game_event("MONSTER_ENCOUNTERED", event_data);
    }
}

// Handle monster deaths
void handle_monster_killed(creature *monst, char *killedBy) {
    if (!monst) return;
    
    // Create JSON event data
    char event_data[2048];
    
    snprintf(event_data, sizeof(event_data),
             "{\"monsterName\":\"%s\",\"monsterLevel\":%d,\"killedBy\":\"%s\",\"isRare\":%s}",
             monst->info.monsterName,
             monst->info.level,
             killedBy,
             (monst->info.flags & MONST_RARE) ? "true" : "false");
    
    // Send event
    send_game_event("MONSTER_KILLED", event_data);
}

// Handle item discoveries
void handle_item_discovered(item *theItem) {
    if (!theItem) return;
    
    // Create JSON event data
    char event_data[2048];
    
    // In a real implementation, we would extract more item details
    snprintf(event_data, sizeof(event_data),
             "{\"itemName\":\"%s\",\"category\":%d,\"isRare\":%s}",
             theItem->name,
             theItem->category,
             (theItem->flags & ITEM_RARE) ? "true" : "false");
    
    // Send event
    send_game_event("ITEM_DISCOVERED", event_data);
}

// Handle level transitions
void handle_new_level(short depth, short stairDirection) {
    // Create JSON event data
    char event_data[2048];
    char environment_type[32] = "dungeon";
    
    // In a real implementation, we would examine the level to determine its characteristics
    
    snprintf(event_data, sizeof(event_data),
             "{\"depth\":%d,\"direction\":%d,\"environmentType\":\"%s\"}",
             depth,
             stairDirection,
             environment_type);
    
    // Send event
    send_game_event("NEW_LEVEL", event_data);
    
    // Clear the encountered monsters for this new level
    memset(encountered_monsters, 0, sizeof(encountered_monsters));
}

// Handle player death
void handle_player_died(char *killedBy) {
    // Create JSON event data
    char event_data[2048];
    
    snprintf(event_data, sizeof(event_data),
             "{\"killedBy\":\"%s\",\"totalTurns\":%ld,\"maxDepth\":%d}",
             killedBy,
             rogue.turnCount,
             rogue.deepestLevel);
    
    // Send event
    send_game_event("PLAYER_DIED", event_data);
}

// Handle response from DM agent
void handle_dm_response(const char *response) {
    // Parse the JSON response - in a real implementation we would use a JSON parser
    // For this simplified example, we'll just extract the narrative field with basic string operations
    
    const char *narrative_start = strstr(response, "\"narrative\":\"");
    if (narrative_start) {
        narrative_start += 13; // Skip past "narrative":"
        
        // Find the end of the narrative (closing quote)
        const char *narrative_end = strchr(narrative_start, '\"');
        if (narrative_end) {
            // Copy the narrative to our message buffer
            int length = narrative_end - narrative_start;
            if (length > 0 && length < sizeof(dm_message_buffer) - 1) {
                strncpy(dm_message_buffer, narrative_start, length);
                dm_message_buffer[length] = '\0';
                
                // Format with distinctive styling
                // COLOR_ESCAPE codes:
                // 'p' for purple foreground
                // 'u' for purple background (darker shade)
                // 'r' for normal text
                
                char formatted_message[4300];
                sprintf(formatted_message, "%c%c%c%c[Dungeon Master]%c%c%c%c %s", 
                        COLOR_ESCAPE, 'p', 'u', 'r',  // Purple style for prefix
                        COLOR_ESCAPE, 'w', 'h', 'i',  // White for the message text
                        dm_message_buffer);
                
                // Set the message as pending to be displayed next time
                has_pending_message = 1;
                strncpy(dm_message_buffer, formatted_message, sizeof(dm_message_buffer) - 1);
                dm_message_buffer[sizeof(dm_message_buffer) - 1] = '\0';
                
                // In the real integration, we would call:
                // message(formatted_message, 0);
                // But for now, we'll use our custom display mechanism
            }
        }
    }
}

// Set a pending DM message
void set_pending_dm_message(const char *message) {
    strncpy(dm_message_buffer, message, sizeof(dm_message_buffer) - 1);
    dm_message_buffer[sizeof(dm_message_buffer) - 1] = '\0';
    has_pending_message = 1;
}

// Display any pending DM messages
void display_pending_dm_messages() {
    if (has_pending_message) {
        // Display the message in a distinctive way
        // This would be replaced with a call to the game's message() function in the real integration
        
        // In the real integration, we would use:
        // message(dm_message_buffer, 0);
        
        // For testing, we just output to console
        printf("\n-------------------------------------\n");
        printf("DM MESSAGE: %s\n", dm_message_buffer);
        printf("-------------------------------------\n");
        
        // Clear pending state
        has_pending_message = 0;
    }
} 