#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "GlobalsBase.h"

#define BRH_ABI_VERSION 7
#define BRH_OBS_CELLS (COLS * ROWS)
#define BRH_MAP_CELLS (DCOLS * DROWS)
#define BRH_MAP_LAYER_CELLS (DCOLS * DROWS * NUMBER_TERRAIN_LAYERS)
#define BRH_COLOR_CELLS (COLS * ROWS * 3)
#define BRH_MAP_COLOR_CELLS (DCOLS * DROWS * 3)
#define BRH_BLSTATS_SIZE 21
#define BRH_MESSAGE_SIZE 256
#define BRH_PROGRAM_STATE_SIZE 8
#define BRH_INVENTORY_SIZE 26
#define BRH_INVENTORY_STR_LENGTH 80
#define BRH_INVENTORY_STR_CELLS (BRH_INVENTORY_SIZE * BRH_INVENTORY_STR_LENGTH)
#define BRH_THREAD_STACK_SIZE (16 * 1024 * 1024)
#define BRH_STEP_OK 0
#define BRH_STEP_INVALID_KEY 1
#define BRH_UNKNOWN_SHORT INT16_MIN
#define BRH_OBS_CELL_FLAGS (DISCOVERED | VISIBLE | HAS_PLAYER | HAS_MONSTER | HAS_ITEM \
                            | IN_FIELD_OF_VIEW | WAS_VISIBLE | HAS_STAIRS | MAGIC_MAPPED \
                            | ITEM_DETECTED | CLAIRVOYANT_VISIBLE | WAS_CLAIRVOYANT_VISIBLE \
                            | TELEPATHIC_VISIBLE | WAS_TELEPATHIC_VISIBLE | IS_IN_PATH \
                            | KNOWN_TO_BE_TRAP_FREE)
#define BRH_OBS_ITEM_FLAGS (ITEM_IDENTIFIED | ITEM_EQUIPPED | ITEM_PROTECTED \
                            | ITEM_RUNIC_HINTED | ITEM_RUNIC_IDENTIFIED \
                            | ITEM_CAN_BE_IDENTIFIED | ITEM_MAGIC_DETECTED \
                            | ITEM_MAX_CHARGES_KNOWN | ITEM_IS_KEY | ITEM_PLAYER_AVOIDS)

typedef struct brh_observation {
    int16_t glyphs[BRH_OBS_CELLS];
    uint32_t chars[BRH_OBS_CELLS];
    uint8_t colors_fg[BRH_COLOR_CELLS];
    uint8_t colors_bg[BRH_COLOR_CELLS];
    uint8_t specials[BRH_OBS_CELLS];
    uint16_t map_layers[BRH_MAP_LAYER_CELLS];
    uint64_t map_flags[BRH_MAP_CELLS];
    uint16_t map_volume[BRH_MAP_CELLS];
    uint8_t map_machine[BRH_MAP_CELLS];
    int16_t map_light[BRH_MAP_COLOR_CELLS];
    uint8_t map_has_item[BRH_MAP_CELLS];
    uint16_t map_item_category[BRH_MAP_CELLS];
    int16_t map_item_kind[BRH_MAP_CELLS];
    int16_t map_item_quantity[BRH_MAP_CELLS];
    uint64_t map_item_flags[BRH_MAP_CELLS];
    uint8_t map_has_monster[BRH_MAP_CELLS];
    int16_t map_monster_kind[BRH_MAP_CELLS];
    int16_t map_monster_hp[BRH_MAP_CELLS];
    int16_t map_monster_state[BRH_MAP_CELLS];
    uint8_t inventory_present[BRH_INVENTORY_SIZE];
    uint8_t inventory_letters[BRH_INVENTORY_SIZE];
    uint8_t inventory_strs[BRH_INVENTORY_STR_CELLS];
    uint16_t inventory_category[BRH_INVENTORY_SIZE];
    int16_t inventory_kind[BRH_INVENTORY_SIZE];
    int16_t inventory_quantity[BRH_INVENTORY_SIZE];
    uint64_t inventory_flags[BRH_INVENTORY_SIZE];
    int16_t inventory_enchant1[BRH_INVENTORY_SIZE];
    int16_t inventory_enchant2[BRH_INVENTORY_SIZE];
    int16_t inventory_charges[BRH_INVENTORY_SIZE];
    int64_t blstats[BRH_BLSTATS_SIZE];
    uint8_t message[BRH_MESSAGE_SIZE];
    uint64_t program_state[BRH_PROGRAM_STATE_SIZE];
} brh_observation;

int brh_reset(uint64_t seed, brh_observation *out);
int brh_step(long key, int control, int shift, brh_observation *out);
uint32_t brh_abi_version(void);
size_t brh_observation_size(void);
int brh_screen_cols(void);
int brh_screen_rows(void);
int brh_map_cols(void);
int brh_map_rows(void);
int brh_inventory_size(void);
int brh_inventory_str_length(void);
void brh_close(void);
void brh_set_data_dir(const char *path);
const char *brh_last_error(void);
void brh_mark_invalid_key(void);

struct brogueConsole currentConsole;
char dataDirectory[BROGUE_FILENAME_MAX] = STRINGIFY(DATADIR);
boolean serverMode = false;
boolean nonInteractivePlayback = false;
boolean hasGraphics = false;
enum graphicsModes graphicsMode = TEXT_GRAPHICS;

static pthread_mutex_t bridgeMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t bridgeCond = PTHREAD_COND_INITIALIZER;
static pthread_t bridgeThread;
static boolean bridgeThreadStarted = false;
static boolean bridgeWaitingForAction = false;
static boolean bridgeActionReady = false;
static boolean bridgeCloseRequested = false;
static boolean bridgeExited = false;
static boolean bridgeActiveControl = false;
static boolean bridgeActiveShift = false;
static int bridgeExitCode = 0;
static int bridgeLastStepStatus = BRH_STEP_OK;
static char bridgeLastError[256] = "";
static rogueEvent bridgePendingEvent = {0};
static brh_observation bridgeLastObservation = {0};
static int64_t bridgeEndScore = 0;
static boolean bridgeEndWon = false;

static void *bridge_thread_main(void *unused);
static void bridge_gameLoop(void);
static boolean bridge_pauseForMilliseconds(short milliseconds, PauseBehavior behavior);
static void bridge_nextKeyOrMouseEvent(rogueEvent *returnEvent,
                                       boolean textInput,
                                       boolean colorsDance);
static void bridge_plotChar(enum displayGlyph inputChar,
                            short xLoc,
                            short yLoc,
                            short foreRed,
                            short foreGreen,
                            short foreBlue,
                            short backRed,
                            short backGreen,
                            short backBlue);
static void bridge_remap(const char *input_name, const char *output_name);
static boolean bridge_modifierHeld(int modifier);
static void bridge_notifyEvent(short eventId,
                               int data1,
                               int data2,
                               const char *str1,
                               const char *str2);
static enum graphicsModes bridge_setGraphicsMode(enum graphicsModes mode);
static void bridge_fill_observation(brh_observation *out);
static void bridge_fill_unknown_semantics(brh_observation *out);
static void bridge_fill_map_semantics(brh_observation *out);
static void bridge_fill_item_semantics(brh_observation *out);
static void bridge_fill_monster_semantics(brh_observation *out);
static void bridge_fill_inventory_semantics(brh_observation *out);
static void bridge_set_error(const char *message);
static void bridge_set_error_locked(const char *message);
static int bridge_wait_for_observation_locked(boolean allowExit);
static void bridge_copy_observation(brh_observation *to, const brh_observation *from);
static void bridge_initialize_launch_state(uint64_t seed);
static int bridge_append_message_line(brh_observation *out, int offset, const char *line);
static int bridge_copy_plain_text(uint8_t *target, int targetLength, const char *source);
static int bridge_map_index(short x, short y);
static boolean bridge_loc_is_in_map(pos loc);
static int bridge_inventory_slot(const brh_observation *out, char letter);
static int16_t bridge_observed_item_kind(const item *theItem);
static int16_t bridge_observed_remembered_item_kind(uint16_t category, short kind);
static int16_t bridge_observed_enchant(const item *theItem, short enchant);
static int16_t bridge_observed_charges(const item *theItem);
static void bridge_store_map_item(brh_observation *out, int cellIndex, const item *theItem);
static void bridge_store_inventory_item(brh_observation *out, int slot, item *theItem);
static void bridge_store_monster(brh_observation *out, const creature *monst);
static uint8_t bridge_color_component(char component);
static int16_t bridge_glyph_component(enum displayGlyph glyph);
static uint32_t bridge_char_component(enum displayGlyph glyph);

static struct brogueConsole bridgeConsole = {
    bridge_gameLoop,
    bridge_pauseForMilliseconds,
    bridge_nextKeyOrMouseEvent,
    bridge_plotChar,
    bridge_remap,
    bridge_modifierHeld,
    bridge_notifyEvent,
    NULL,
    bridge_setGraphicsMode
};

int brh_reset(uint64_t seed, brh_observation *out) {
    int rc;
    pthread_attr_t threadAttr;

    if (out == NULL) {
        bridge_set_error("observation pointer must not be null");
        return -1;
    }

    brh_close();

    pthread_mutex_lock(&bridgeMutex);
    bridgeLastError[0] = '\0';
    bridgeWaitingForAction = false;
    bridgeActionReady = false;
    bridgeCloseRequested = false;
    bridgeExited = false;
    bridgeExitCode = 0;
    bridgeLastStepStatus = BRH_STEP_OK;
    bridgeActiveControl = false;
    bridgeActiveShift = false;
    memset(&bridgeLastObservation, 0, sizeof(bridgeLastObservation));
    memset(&bridgePendingEvent, 0, sizeof(bridgePendingEvent));
    bridgeEndScore = 0;
    bridgeEndWon = false;

    currentConsole = bridgeConsole;
    serverMode = true;
    nonInteractivePlayback = false;
    hasGraphics = false;
    graphicsMode = TEXT_GRAPHICS;
    gameVariant = VARIANT_BROGUE;
    bridge_initialize_launch_state(seed);

    rc = pthread_attr_init(&threadAttr);
    if (rc != 0) {
        bridgeThreadStarted = false;
        bridge_set_error_locked("failed to initialize Brogue bridge thread attributes");
        pthread_mutex_unlock(&bridgeMutex);
        return -1;
    }
    rc = pthread_attr_setstacksize(&threadAttr, BRH_THREAD_STACK_SIZE);
    if (rc != 0) {
        pthread_attr_destroy(&threadAttr);
        bridgeThreadStarted = false;
        bridge_set_error_locked("failed to configure Brogue bridge thread stack");
        pthread_mutex_unlock(&bridgeMutex);
        return -1;
    }
    rc = pthread_create(&bridgeThread, &threadAttr, bridge_thread_main, NULL);
    pthread_attr_destroy(&threadAttr);
    if (rc != 0) {
        bridgeThreadStarted = false;
        bridge_set_error_locked("failed to start Brogue bridge thread");
        pthread_mutex_unlock(&bridgeMutex);
        return -1;
    }
    bridgeThreadStarted = true;

    rc = bridge_wait_for_observation_locked(false);
    if (rc == 0) {
        bridge_copy_observation(out, &bridgeLastObservation);
    }
    pthread_mutex_unlock(&bridgeMutex);
    return rc;
}

int brh_step(long key, int control, int shift, brh_observation *out) {
    int rc;

    if (out == NULL) {
        bridge_set_error("observation pointer must not be null");
        return -1;
    }

    pthread_mutex_lock(&bridgeMutex);
    if (!bridgeThreadStarted || bridgeExited) {
        bridge_set_error_locked("Brogue bridge is not running");
        pthread_mutex_unlock(&bridgeMutex);
        return -1;
    }
    if (!bridgeWaitingForAction) {
        bridge_set_error_locked("Brogue bridge is not waiting for an action");
        pthread_mutex_unlock(&bridgeMutex);
        return -1;
    }

    bridgePendingEvent.eventType = KEYSTROKE;
    bridgePendingEvent.param1 = key;
    bridgePendingEvent.param2 = 0;
    bridgePendingEvent.controlKey = control ? true : false;
    bridgePendingEvent.shiftKey = shift ? true : false;
    bridgeLastStepStatus = BRH_STEP_OK;
    bridgeActionReady = true;
    bridgeWaitingForAction = false;
    pthread_cond_broadcast(&bridgeCond);

    rc = bridge_wait_for_observation_locked(true);
    if (rc == 0) {
        bridge_copy_observation(out, &bridgeLastObservation);
        rc = bridgeLastStepStatus;
    }
    pthread_mutex_unlock(&bridgeMutex);
    return rc;
}

uint32_t brh_abi_version(void) {
    return BRH_ABI_VERSION;
}

size_t brh_observation_size(void) {
    return sizeof(brh_observation);
}

int brh_screen_cols(void) {
    return COLS;
}

int brh_screen_rows(void) {
    return ROWS;
}

int brh_map_cols(void) {
    return DCOLS;
}

int brh_map_rows(void) {
    return DROWS;
}

int brh_inventory_size(void) {
    return BRH_INVENTORY_SIZE;
}

int brh_inventory_str_length(void) {
    return BRH_INVENTORY_STR_LENGTH;
}

void brh_close(void) {
    boolean shouldJoin;

    pthread_mutex_lock(&bridgeMutex);
    shouldJoin = bridgeThreadStarted;
    if (shouldJoin) {
        bridgeCloseRequested = true;
        bridgeActionReady = true;
        pthread_cond_broadcast(&bridgeCond);
    }
    pthread_mutex_unlock(&bridgeMutex);

    if (shouldJoin) {
        pthread_join(bridgeThread, NULL);
        pthread_mutex_lock(&bridgeMutex);
        bridgeThreadStarted = false;
        bridgeWaitingForAction = false;
        bridgeActionReady = false;
        bridgeCloseRequested = false;
        pthread_mutex_unlock(&bridgeMutex);
    }
}

void brh_set_data_dir(const char *path) {
    if (path == NULL || path[0] == '\0') {
        return;
    }

    pthread_mutex_lock(&bridgeMutex);
    strncpy(dataDirectory, path, BROGUE_FILENAME_MAX - 1);
    dataDirectory[BROGUE_FILENAME_MAX - 1] = '\0';
    pthread_mutex_unlock(&bridgeMutex);
}

const char *brh_last_error(void) {
    return bridgeLastError;
}

void brh_mark_invalid_key(void) {
    pthread_mutex_lock(&bridgeMutex);
    if (bridgeThreadStarted && !bridgeExited) {
        bridgeLastStepStatus = BRH_STEP_INVALID_KEY;
    }
    pthread_mutex_unlock(&bridgeMutex);
}

boolean tryParseUint64(char *str, uint64_t *num) {
    unsigned long long n;
    char buf[100];

    if (strlen(str)
        && sscanf(str, "%llu", &n)
        && sprintf(buf, "%llu", n)
        && !strcmp(buf, str)) {
        *num = (uint64_t) n;
        return true;
    }
    return false;
}

static void *bridge_thread_main(void *unused) {
    int exitCode;

    (void) unused;
    exitCode = rogueMain();

    pthread_mutex_lock(&bridgeMutex);
    bridgeExitCode = exitCode;
    bridgeExited = true;
    bridgeWaitingForAction = false;
    /*
     * rogueMain() frees level-owned lists before returning. Keep the last
     * observation captured at an input boundary instead of walking freed
     * terrain/item/monster globals during shutdown.
     */
    bridgeLastObservation.program_state[1] = 1;
    bridgeLastObservation.program_state[2] = bridgeEndWon ? 1 : 0;
    bridgeLastObservation.program_state[6] = (uint64_t) bridgeEndScore;
    pthread_cond_broadcast(&bridgeCond);
    pthread_mutex_unlock(&bridgeMutex);
    return NULL;
}

static void bridge_gameLoop(void) {
    (void) rogueMain();
}

static boolean bridge_pauseForMilliseconds(short milliseconds, PauseBehavior behavior) {
    (void) milliseconds;
    (void) behavior;
    return false;
}

static void bridge_nextKeyOrMouseEvent(rogueEvent *returnEvent,
                                       boolean textInput,
                                       boolean colorsDance) {
    (void) textInput;
    (void) colorsDance;

    pthread_mutex_lock(&bridgeMutex);
    bridge_fill_observation(&bridgeLastObservation);
    bridgeWaitingForAction = true;
    bridgeActionReady = false;
    pthread_cond_broadcast(&bridgeCond);

    while (!bridgeActionReady && !bridgeCloseRequested) {
        pthread_cond_wait(&bridgeCond, &bridgeMutex);
    }

    if (bridgeCloseRequested) {
        rogue.quit = true;
        rogue.gameHasEnded = true;
        returnEvent->eventType = KEYSTROKE;
        returnEvent->param1 = ACKNOWLEDGE_KEY;
        returnEvent->param2 = 0;
        returnEvent->controlKey = false;
        returnEvent->shiftKey = false;
        bridgeActionReady = false;
        bridgeWaitingForAction = false;
        pthread_mutex_unlock(&bridgeMutex);
        return;
    }

    *returnEvent = bridgePendingEvent;
    bridgeActiveControl = bridgePendingEvent.controlKey;
    bridgeActiveShift = bridgePendingEvent.shiftKey;
    bridgeActionReady = false;
    pthread_mutex_unlock(&bridgeMutex);
}

static void bridge_plotChar(enum displayGlyph inputChar,
                            short xLoc,
                            short yLoc,
                            short foreRed,
                            short foreGreen,
                            short foreBlue,
                            short backRed,
                            short backGreen,
                            short backBlue) {
    (void) inputChar;
    (void) xLoc;
    (void) yLoc;
    (void) foreRed;
    (void) foreGreen;
    (void) foreBlue;
    (void) backRed;
    (void) backGreen;
    (void) backBlue;
}

static void bridge_remap(const char *input_name, const char *output_name) {
    (void) input_name;
    (void) output_name;
}

static boolean bridge_modifierHeld(int modifier) {
    if (modifier == 0) {
        return bridgeActiveShift;
    }
    if (modifier == 1) {
        return bridgeActiveControl;
    }
    return false;
}

static void bridge_notifyEvent(short eventId,
                               int data1,
                               int data2,
                               const char *str1,
                               const char *str2) {
    (void) data2;
    (void) str1;
    (void) str2;
    if (eventId == GAMEOVER_VICTORY || eventId == GAMEOVER_SUPERVICTORY) {
        bridgeEndWon = true;
        bridgeEndScore = (int64_t) data1;
    } else if (eventId == GAMEOVER_DEATH || eventId == GAMEOVER_QUIT) {
        bridgeEndWon = false;
        bridgeEndScore = (int64_t) data1;
    } else {
        (void) eventId;
        (void) data1;
    }
}

static enum graphicsModes bridge_setGraphicsMode(enum graphicsModes mode) {
    return mode;
}

static void bridge_fill_observation(brh_observation *out) {
    int x;
    int y;
    int messageOffset = 0;

    memset(out, 0, sizeof(*out));
    bridge_fill_unknown_semantics(out);

    for (y = 0; y < ROWS; y++) {
        for (x = 0; x < COLS; x++) {
            int cellIndex = y * COLS + x;
            int colorIndex = cellIndex * 3;
            cellDisplayBuffer *cell = &displayBuffer.cells[x][y];

            out->glyphs[cellIndex] = bridge_glyph_component(cell->character);
            out->chars[cellIndex] = bridge_char_component(cell->character);
            out->colors_fg[colorIndex] = bridge_color_component(cell->foreColorComponents[0]);
            out->colors_fg[colorIndex + 1] = bridge_color_component(cell->foreColorComponents[1]);
            out->colors_fg[colorIndex + 2] = bridge_color_component(cell->foreColorComponents[2]);
            out->colors_bg[colorIndex] = bridge_color_component(cell->backColorComponents[0]);
            out->colors_bg[colorIndex + 1] = bridge_color_component(cell->backColorComponents[1]);
            out->colors_bg[colorIndex + 2] = bridge_color_component(cell->backColorComponents[2]);
        }
    }

    for (y = 0; y < MESSAGE_LINES && messageOffset < BRH_MESSAGE_SIZE - 1; y++) {
        messageOffset = bridge_append_message_line(out, messageOffset, displayedMessage[y]);
        if (messageOffset < BRH_MESSAGE_SIZE - 1) {
            out->message[messageOffset++] = (uint8_t) '\n';
        }
    }

    out->blstats[0] = player.loc.x;
    out->blstats[1] = player.loc.y;
    out->blstats[2] = rogue.strength;
    out->blstats[3] = player.currentHP;
    out->blstats[4] = player.info.maxHP;
    out->blstats[5] = rogue.depthLevel;
    out->blstats[6] = (int64_t) rogue.gold;
    out->blstats[7] = (int64_t) rogue.playerTurnNumber;
    out->blstats[8] = (int64_t) rogue.absoluteTurnNumber;
    out->blstats[9] = rogue.stealthRange;
    out->blstats[10] = player.status[STATUS_NUTRITION];

    out->program_state[0] = rogue.playerTurnNumber;
    out->program_state[1] = rogue.gameHasEnded ? 1 : 0;
    out->program_state[2] = 0;       /* won: always 0 mid-game; patched to 1 at GAMEOVER_VICTORY */
    out->program_state[3] = rogue.depthLevel;
    out->program_state[4] = rogue.seed;
    out->program_state[5] = rogue.gold;
    out->program_state[6] = rogue.gold; /* score proxy: rogue.gold mid-game; patched to final score at game end */
    out->program_state[7] = (rogue.gameInProgress ? 1 : 0)
                          | (rogue.quit ? 2 : 0)
                          | (rogue.disturbed ? 4 : 0);

    bridge_fill_map_semantics(out);
    bridge_fill_item_semantics(out);
    bridge_fill_monster_semantics(out);
    bridge_fill_inventory_semantics(out);
}

static void bridge_fill_unknown_semantics(brh_observation *out) {
    int i;

    for (i = 0; i < BRH_MAP_CELLS; i++) {
        out->map_item_kind[i] = BRH_UNKNOWN_SHORT;
        out->map_item_quantity[i] = BRH_UNKNOWN_SHORT;
        out->map_monster_kind[i] = BRH_UNKNOWN_SHORT;
        out->map_monster_hp[i] = BRH_UNKNOWN_SHORT;
        out->map_monster_state[i] = BRH_UNKNOWN_SHORT;
    }
    for (i = 0; i < BRH_INVENTORY_SIZE; i++) {
        out->inventory_kind[i] = BRH_UNKNOWN_SHORT;
        out->inventory_quantity[i] = BRH_UNKNOWN_SHORT;
        out->inventory_enchant1[i] = BRH_UNKNOWN_SHORT;
        out->inventory_enchant2[i] = BRH_UNKNOWN_SHORT;
        out->inventory_charges[i] = BRH_UNKNOWN_SHORT;
    }
}

static void bridge_fill_map_semantics(brh_observation *out) {
    short x;
    short y;
    int layer;

    for (y = 0; y < DROWS; y++) {
        for (x = 0; x < DCOLS; x++) {
            int cellIndex = bridge_map_index(x, y);
            int layerIndex = cellIndex * NUMBER_TERRAIN_LAYERS;
            int colorIndex = cellIndex * 3;
            pcell *cell = &pmap[x][y];
            boolean visible = (cell->flags & ANY_KIND_OF_VISIBLE) ? true : false;
            boolean known = visible || (cell->flags & (DISCOVERED | MAGIC_MAPPED));

            if (visible) {
                for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                    out->map_layers[layerIndex + layer] = (uint16_t) cell->layers[layer];
                }
                out->map_volume[cellIndex] = cell->volume;
                out->map_machine[cellIndex] = cell->machineNumber;
                out->map_light[colorIndex] = tmap[x][y].light[0];
                out->map_light[colorIndex + 1] = tmap[x][y].light[1];
                out->map_light[colorIndex + 2] = tmap[x][y].light[2];
                out->map_flags[cellIndex] = (uint64_t) (cell->flags & BRH_OBS_CELL_FLAGS);
            } else if (known) {
                out->map_layers[layerIndex + DUNGEON] = (uint16_t) cell->rememberedTerrain;
                out->map_flags[cellIndex] = (uint64_t) (cell->rememberedCellFlags & BRH_OBS_CELL_FLAGS);
                if (cell->rememberedItemCategory) {
                    out->map_has_item[cellIndex] = 1;
                    out->map_item_category[cellIndex] = (uint16_t) cell->rememberedItemCategory;
                    out->map_item_kind[cellIndex] = bridge_observed_remembered_item_kind(
                        out->map_item_category[cellIndex],
                        cell->rememberedItemKind);
                    out->map_item_quantity[cellIndex] = cell->rememberedItemQuantity;
                }
            }
        }
    }
}

static void bridge_fill_item_semantics(brh_observation *out) {
    item *theItem;

    if (floorItems == NULL) {
        return;
    }
    for (theItem = floorItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (bridge_loc_is_in_map(theItem->loc)
            && (pmapAt(theItem->loc)->flags & (ANY_KIND_OF_VISIBLE | ITEM_DETECTED))) {
            bridge_store_map_item(out, bridge_map_index(theItem->loc.x, theItem->loc.y), theItem);
        }
    }
}

static void bridge_fill_monster_semantics(brh_observation *out) {
    if (bridge_loc_is_in_map(player.loc)) {
        bridge_store_monster(out, &player);
    }
    if (monsters == NULL) {
        return;
    }
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (bridge_loc_is_in_map(monst->loc)
            && (pmapAt(monst->loc)->flags & ANY_KIND_OF_VISIBLE)) {
            bridge_store_monster(out, monst);
        }
    }
}

static void bridge_fill_inventory_semantics(brh_observation *out) {
    item *theItem;

    if (packItems == NULL) {
        return;
    }
    for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        int slot = bridge_inventory_slot(out, theItem->inventoryLetter);
        if (slot >= 0) {
            bridge_store_inventory_item(out, slot, theItem);
        }
    }
}

static void bridge_set_error(const char *message) {
    pthread_mutex_lock(&bridgeMutex);
    bridge_set_error_locked(message);
    pthread_mutex_unlock(&bridgeMutex);
}

static void bridge_set_error_locked(const char *message) {
    strncpy(bridgeLastError, message, sizeof(bridgeLastError) - 1);
    bridgeLastError[sizeof(bridgeLastError) - 1] = '\0';
}

static int bridge_wait_for_observation_locked(boolean allowExit) {
    while (!bridgeWaitingForAction && !bridgeExited) {
        pthread_cond_wait(&bridgeCond, &bridgeMutex);
    }
    if (bridgeExited && !bridgeWaitingForAction) {
        if (allowExit) {
            return 0;
        }
        snprintf(bridgeLastError,
                 sizeof(bridgeLastError),
                 "Brogue bridge exited with status %d",
                 bridgeExitCode);
        return -1;
    }
    return 0;
}

static void bridge_copy_observation(brh_observation *to, const brh_observation *from) {
    memcpy(to, from, sizeof(*to));
}

static void bridge_initialize_launch_state(uint64_t seed) {
    memset((void *) &rogue, 0, sizeof(playerCharacter));
    rogue.nextGame = seed == 0 ? NG_NEW_GAME : NG_NEW_GAME_WITH_SEED;
    rogue.nextGameSeed = seed;
    rogue.mode = GAME_MODE_NORMAL;
    rogue.displayStealthRangeMode = false;
    rogue.trueColorMode = false;
}

static int bridge_append_message_line(brh_observation *out, int offset, const char *line) {
    if (offset >= BRH_MESSAGE_SIZE - 1) {
        return BRH_MESSAGE_SIZE - 1;
    }
    offset += bridge_copy_plain_text(&out->message[offset], BRH_MESSAGE_SIZE - offset, line);
    return offset;
}

static int bridge_copy_plain_text(uint8_t *target, int targetLength, const char *source) {
    int inputOffset = 0;
    int outputOffset = 0;

    if (targetLength <= 0) {
        return 0;
    }
    memset(target, 0, targetLength);
    while (source[inputOffset] != '\0' && outputOffset < targetLength - 1) {
        if ((uint8_t) source[inputOffset] == COLOR_ESCAPE) {
            int skip;

            inputOffset++;
            for (skip = 0; skip < 3 && source[inputOffset] != '\0'; skip++) {
                inputOffset++;
            }
            continue;
        }
        target[outputOffset++] = (uint8_t) source[inputOffset++];
    }
    return outputOffset;
}

static int bridge_map_index(short x, short y) {
    return y * DCOLS + x;
}

static boolean bridge_loc_is_in_map(pos loc) {
    return loc.x >= 0 && loc.x < DCOLS && loc.y >= 0 && loc.y < DROWS;
}

static int bridge_inventory_slot(const brh_observation *out, char letter) {
    int i;

    if (letter >= 'a' && letter <= 'z') {
        return letter - 'a';
    }
    for (i = 0; i < BRH_INVENTORY_SIZE; i++) {
        if (out->inventory_letters[i] == 0) {
            return i;
        }
    }
    return -1;
}

static int16_t bridge_observed_item_kind(const item *theItem) {
    if (theItem->category & (FOOD | WEAPON | ARMOR | CHARM | GOLD | AMULET | GEM | KEY)) {
        return theItem->kind;
    }
    if (theItem->flags & ITEM_IDENTIFIED) {
        return theItem->kind;
    }
    return BRH_UNKNOWN_SHORT;
}

static int16_t bridge_observed_remembered_item_kind(uint16_t category, short kind) {
    if (category & (FOOD | WEAPON | ARMOR | CHARM | GOLD | AMULET | GEM | KEY)) {
        return kind;
    }
    return BRH_UNKNOWN_SHORT;
}

static int16_t bridge_observed_enchant(const item *theItem, short enchant) {
    if (theItem->flags & ITEM_IDENTIFIED) {
        return enchant;
    }
    return BRH_UNKNOWN_SHORT;
}

static int16_t bridge_observed_charges(const item *theItem) {
    if (theItem->flags & (ITEM_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN)) {
        return theItem->charges;
    }
    return BRH_UNKNOWN_SHORT;
}

static void bridge_store_map_item(brh_observation *out, int cellIndex, const item *theItem) {
    out->map_has_item[cellIndex] = 1;
    out->map_item_category[cellIndex] = (uint16_t) theItem->category;
    out->map_item_kind[cellIndex] = bridge_observed_item_kind(theItem);
    out->map_item_quantity[cellIndex] = theItem->quantity;
    out->map_item_flags[cellIndex] = (uint64_t) (theItem->flags & BRH_OBS_ITEM_FLAGS);
}

static void bridge_store_inventory_item(brh_observation *out, int slot, item *theItem) {
    char name[COLS * 3] = "";

    out->inventory_present[slot] = 1;
    out->inventory_letters[slot] = (uint8_t) theItem->inventoryLetter;
    itemName(theItem, name, true, true, NULL);
    bridge_copy_plain_text(&out->inventory_strs[slot * BRH_INVENTORY_STR_LENGTH],
                           BRH_INVENTORY_STR_LENGTH,
                           name);
    out->inventory_category[slot] = (uint16_t) theItem->category;
    out->inventory_kind[slot] = bridge_observed_item_kind(theItem);
    out->inventory_quantity[slot] = theItem->quantity;
    out->inventory_flags[slot] = (uint64_t) (theItem->flags & BRH_OBS_ITEM_FLAGS);
    out->inventory_enchant1[slot] = bridge_observed_enchant(theItem, theItem->enchant1);
    out->inventory_enchant2[slot] = bridge_observed_enchant(theItem, theItem->enchant2);
    out->inventory_charges[slot] = bridge_observed_charges(theItem);
}

static void bridge_store_monster(brh_observation *out, const creature *monst) {
    int cellIndex = bridge_map_index(monst->loc.x, monst->loc.y);

    out->map_has_monster[cellIndex] = 1;
    out->map_monster_kind[cellIndex] = monst->info.monsterID;
    out->map_monster_hp[cellIndex] = monst->currentHP;
    out->map_monster_state[cellIndex] = monst->creatureState;
}

static uint8_t bridge_color_component(char component) {
    int value = (int) component;

    if (value <= 0) {
        return 0;
    }
    if (value >= 100) {
        return UINT8_MAX;
    }
    return (uint8_t) ((value * UINT8_MAX + 50) / 100);
}

static int16_t bridge_glyph_component(enum displayGlyph glyph) {
    if (glyph < 0 || glyph > INT16_MAX) {
        return 0;
    }
    return (int16_t) glyph;
}

static uint32_t bridge_char_component(enum displayGlyph glyph) {
    if (glyph == 0) {
        return 0;
    }
    return glyphToUnicode(glyph);
}
