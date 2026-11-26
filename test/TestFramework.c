#include "TestFramework.h"
#include "Rogue.h"
#include "Globals.h"
#include "GlobalsBase.h"
#include "platform.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// The master registry of test suites (see Tests.c).
extern const testCase *const testSuites[];

static int testStepCount = 0;
static short testStartDepth = 1;
boolean testFailed = false;
static char failureMessage[COLS * 3];

// Queue of keystrokes that the engine will consume as input.  In test mode the
// engine never reads the real console, so answers to confirmations and other
// prompts are supplied here.  When the queue is empty the engine behaves as if
// the player pressed escape, so a test can never hang on a prompt.
#define TEST_INPUT_QUEUE_SIZE 256
static rogueEvent inputQueue[TEST_INPUT_QUEUE_SIZE];
static int inputQueueHead = 0;
static int inputQueueTail = 0;

// Messages shown to the player during the current test.  An entry must hold
// a whole recorded dialog: in test mode printTextBox() hands its full text to
// recordTestMessage() so tests can assert on it, and TEXT_MAX_LENGTH is the
// largest single string that text can reach.
#define TEST_MESSAGE_LOG_SIZE 64
static char messageLog[TEST_MESSAGE_LOG_SIZE][TEXT_MAX_LENGTH + 1];
static boolean messageLogRequiresAck[TEST_MESSAGE_LOG_SIZE];
static int messageLogCount = 0;

// Snapshot of the messages the engine emitted while the game was starting up
// (the welcome message and its fellows), taken by runTestByName() just before
// it clears the log for the test itself.
static char startupMessageLog[TEST_MESSAGE_LOG_SIZE][TEXT_MAX_LENGTH + 1];
static int startupMessageCount = 0;

// The cause gameOver() gave for the player's death, kept separately from the
// message log because the rendered death sentence is drawn directly to the
// screen rather than sent through message().
static char playerDeathCauseText[TEXT_MAX_LENGTH + 1];

// Whether the high-score display has opened during the current test.  The
// display is drawn and then immediately covered by the save-recording prompt,
// so it cannot be observed in the screen snapshot taken at the last input
// request; the engine reports the display here instead.
static boolean highScoresDisplayed = false;

// Set while runQueuedInputThroughMainLoop() is pumping the engine's main input
// loop.  That loop only ends when the game ends, so the input hook flags the
// game as ended as soon as the queue runs dry, giving the pump control back.
static boolean inputLoopPumpActive = false;

// Snapshot of the whole screen, taken every time the engine asks the test for
// input.  Full-screen displays (the discoveries screen, the message archive,
// the help screen) draw themselves over the map, wait for a key, and then
// restore the previous screen, so the frame at that input request is the only
// moment their text can be observed.  Invalid until the first input request.
static char screenSnapshot[ROWS][COLS + 1];
static boolean screenSnapshotValid = false;

// Background colour components of the same frame, for detecting a transient
// highlight with screenCellColorsChanged().  The foreground is deliberately
// not kept: plotCharWithColor() re-randomizes it on every draw.
static short screenSnapshotBack[ROWS][COLS][3];
static short rememberedBack[ROWS][COLS][3];
static boolean rememberedColorsValid = false;

// Raw glyphs of the same frame.  screenSnapshot keeps only printable
// characters, but some marks the game draws (the discovered-items screen's
// good/bad magic markers) are glyph identities rather than characters.
static int screenGlyphSnapshot[ROWS][COLS];

static void captureScreenSnapshot(void) {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            int glyph = (int) displayBuffer.cells[x][y].character;
            screenGlyphSnapshot[y][x] = glyph;
            screenSnapshot[y][x] = (glyph >= ' ' && glyph < 127) ? (char) glyph : ' ';
            for (int c = 0; c < 3; c++) {
                screenSnapshotBack[y][x][c] = displayBuffer.cells[x][y].backColorComponents[c];
            }
        }
        screenSnapshot[y][COLS] = '\0';
    }
    screenSnapshotValid = true;
}

boolean screenTextContains(const char *substring) {
    if (!screenSnapshotValid) {
        return false;
    }
    for (int y = 0; y < ROWS; y++) {
        if (strstr(screenSnapshot[y], substring) != NULL) {
            return true;
        }
    }
    return false;
}

int discoveryScreenMagicMarkerFor(const char *name) {
    if (!screenSnapshotValid) {
        return 0;
    }
    for (int y = 0; y < ROWS; y++) {
        const char *match = strstr(screenSnapshot[y], name);
        if (match == NULL) {
            continue;
        }
        // The discoveries screen draws the kind's mark two columns left of its
        // name: one for an identified kind (its item glyph) and the good/bad
        // magic marker for an unidentified one.
        const int markColumn = (int) (match - screenSnapshot[y]) - 2;
        if (markColumn >= 0) {
            if (screenGlyphSnapshot[y][markColumn] == G_GOOD_MAGIC) {
                return 1;
            }
            if (screenGlyphSnapshot[y][markColumn] == G_BAD_MAGIC) {
                return -1;
            }
        }
        return 0;
    }
    return 0;
}

void rememberScreenColors(void) {
    memcpy(rememberedBack, screenSnapshotBack, sizeof(rememberedBack));
    rememberedColorsValid = screenSnapshotValid;
}

const char *flavorTextLine(void) {
    static char buf[DCOLS + 1];
    int length = 0;
    for (int i = 0; i < DCOLS; i++) {
        int glyph = (int) displayBuffer.cells[mapToWindowX(i)][ROWS - 2].character;
        buf[length++] = (glyph >= ' ' && glyph < 127) ? (char) glyph : ' ';
    }
    while (length > 0 && buf[length - 1] == ' ') {
        length--;
    }
    buf[length] = '\0';
    return buf;
}

void refreshFlavorText(void) {
    rogue.disturbed = true;
    updateFlavorText();
}

boolean screenCellColorsChanged(pos loc) {
    if (!rememberedColorsValid || !screenSnapshotValid) {
        return false;
    }
    windowpos w = mapToWindow(loc);
    if (!locIsInWindow(w)) {
        return false;
    }
    for (int c = 0; c < 3; c++) {
        int delta = screenSnapshotBack[w.window_y][w.window_x][c]
                    - rememberedBack[w.window_y][w.window_x][c];
        if (delta < 0) {
            delta = -delta;
        }
        if (delta >= 6) {
            return true;
        }
    }
    return false;
}

void queueKeystroke(int key, boolean shift, boolean control) {
    int next = (inputQueueTail + 1) % TEST_INPUT_QUEUE_SIZE;
    if (next == inputQueueHead) {
        return; // Queue full; drop the keystroke.
    }
    rogueEvent *evt = &inputQueue[inputQueueTail];
    memset(evt, 0, sizeof(*evt));
    evt->eventType = KEYSTROKE;
    evt->param1 = key;
    evt->shiftKey = shift;
    evt->controlKey = control;
    inputQueueTail = next;
}

void queueMouseClick(pos loc) {
    int next = (inputQueueTail + 1) % TEST_INPUT_QUEUE_SIZE;
    if (next == inputQueueHead) {
        return; // Queue full; drop the click.
    }
    rogueEvent *evt = &inputQueue[inputQueueTail];
    memset(evt, 0, sizeof(*evt));
    evt->eventType = MOUSE_UP;
    evt->param1 = loc.x;
    evt->param2 = loc.y;
    inputQueueTail = next;
}

void queueMapClick(pos loc) {
    queueMouseClick(posAt(mapToWindowX(loc.x), mapToWindowY(loc.y)));
}

boolean testNextInputEvent(rogueEvent *evt) {
    // The engine is about to hand control to whatever is drawing the current
    // screen, so this is the moment any full-screen display is visible.
    captureScreenSnapshot();
    if (inputQueueHead == inputQueueTail) {
        if (inputLoopPumpActive) {
            // The main input loop has no exit other than the game ending, so
            // flag it ended; the pump clears the flag once the loop returns.
            rogue.gameHasEnded = true;
        }
        return false;
    }
    *evt = inputQueue[inputQueueHead];
    inputQueueHead = (inputQueueHead + 1) % TEST_INPUT_QUEUE_SIZE;
    return true;
}

void queueConfirmYes(void) {
    queueKeystroke('y', false, false);
}

void queueConfirmNo(void) {
    queueKeystroke('n', false, false);
}

boolean askConfirmation(void) {
    return confirm("Are you sure?", false);
}

boolean chooseFileWithSuffix(const char *suffix) {
    char path[BROGUE_FILENAME_MAX];
    path[0] = '\0';
    return dialogChooseFile(path, suffix, "Open file:");
}

void enableAutopilot(void) {
    rogue.autoPlayingLevel = true;
}

const char *gameVersionString(void) {
    return gameConst->versionString;
}

void showDungeonSeed(void) {
    printSeed();
}

void setDungeonSeedHidden(boolean hidden) {
    rogue.hideSeed = hidden;
}

unsigned long long dungeonSeed(void) {
    return rogue.seed;
}

static void clearInputQueue(void) {
    inputQueueHead = inputQueueTail = 0;
}

// See the declaration in TestFramework.h.
int inventoryScreenMagicMarkerFor(char inventoryLetter) {
    // Open the real inventory over the dungeon.  The frame captured at its
    // input request is the inventory as drawn; dismissing it afterwards leaves
    // that frame available for inspection.
    queueKeystroke(ESCAPE_KEY, false, false);
    displayInventory(ALL_ITEMS, 0, 0, true, true);
    clearInputQueue();

    if (!screenSnapshotValid) {
        return -2;
    }

    // Each item's row starts with " <letter>)", followed by a space and then
    // the magic mark cell: the glowing good/bad-magic sigil, a plain '-' for
    // neutral magic, or the item's own glyph when no magic mark is shown.
    char prefix[4] = { ' ', inventoryLetter, ')', '\0' };
    for (int y = 0; y < ROWS; y++) {
        const char *match = strstr(screenSnapshot[y], prefix);
        if (match == NULL) {
            continue;
        }
        const int markColumn = (int) (match - screenSnapshot[y]) + 4;
        if (markColumn >= COLS) {
            return -2;
        }
        const int glyph = screenGlyphSnapshot[y][markColumn];
        if (glyph == G_GOOD_MAGIC) {
            return 1;
        }
        if (glyph == G_BAD_MAGIC) {
            return -1;
        }
        if (glyph == '-') {
            return 0;
        }
        return -2;
    }
    return -2;
}

// The text the most recent text-entry prompt opened with, recorded by the
// engine through recordTestTextInputDefault().
static char textInputDefault[4096];

void recordTestTextInputDefault(const char *text) {
    snprintf(textInputDefault, sizeof(textInputDefault), "%s", text ? text : "");
}

const char *defaultSaveName(void) {
    textInputDefault[0] = '\0';
    // The saving prompt opens with its default name; Escape cancels it, so
    // nothing is written.
    queueKeystroke(ESCAPE_KEY, false, false);
    saveGame();
    clearInputQueue();
    return textInputDefault;
}

void recordTestMessage(const char *msg) {

    // Only record messages once
    if (messageWasShown(msg)) return;

    if (messageLogCount < TEST_MESSAGE_LOG_SIZE) {
        messageLogCount++;
    }
    // Shift older messages down so the newest is always last.
    int start = (messageLogCount == TEST_MESSAGE_LOG_SIZE) ? 1 : 0;
    memmove(messageLog[0], messageLog[start],
            sizeof(messageLog[0]) * (messageLogCount - 1 - start));
    memmove(messageLogRequiresAck, messageLogRequiresAck + start,
            sizeof(messageLogRequiresAck[0]) * (messageLogCount - 1 - start));
    snprintf(messageLog[messageLogCount - 1], sizeof(messageLog[0]), "%s", msg);
    messageLogRequiresAck[messageLogCount - 1] = false;
}

void recordTestMessageAcknowledgment(const char *msg, boolean requiresAck) {
    for (int i = messageLogCount - 1; i >= 0; i--) {
        if (strcmp(messageLog[i], msg) == 0) {
            messageLogRequiresAck[i] = requiresAck;
            return;
        }
    }
}

void clearMessages(void) {
    messageLogCount = 0;
}

boolean messageWasShown(const char *substring) {
    for (int i = 0; i < messageLogCount; i++) {
        if (strstr(messageLog[i], substring) != NULL) {
            return true;
        }
    }
    return false;
}

boolean messageRequiredAcknowledgment(const char *substring) {
    for (int i = messageLogCount - 1; i >= 0; i--) {
        if (strstr(messageLog[i], substring) != NULL) {
            return messageLogRequiresAck[i];
        }
    }
    return false;
}

const char *lastMessage(void) {
    if (messageLogCount == 0) {
        return "(no message)";
    }
    return messageLog[messageLogCount - 1];
}

const char *messageColorOf(const char *substring) {
    // The game encodes a message's colour as a four-byte escape at the head
    // of the recorded text.  Re-encode the two importance colours exactly the
    // way the game does and compare against those.
    char goodEncoded[8] = "";
    char badEncoded[8] = "";

    encodeMessageColor(goodEncoded, 0, &goodMessageColor);
    encodeMessageColor(badEncoded, 0, &badMessageColor);
    for (int i = 0; i < messageLogCount; i++) {
        if (strstr(messageLog[i], substring) != NULL) {
            if (strncmp(messageLog[i], badEncoded, 4) == 0) {
                return "bad";
            }
            if (strncmp(messageLog[i], goodEncoded, 4) == 0) {
                return "good";
            }
            return "white"; // an explicit white, or no colour escape at all
        }
    }
    return NULL;
}

int messageAreaBrightness(const char *substring) {
    char row[COLS + 1];

    // The message area occupies the top MESSAGE_LINES rows of the window.
    for (int y = 0; y < MESSAGE_LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            int glyph = (int) displayBuffer.cells[x][y].character;
            row[x] = (glyph >= ' ' && glyph < 127) ? (char) glyph : ' ';
        }
        row[COLS] = '\0';

        const char *match = strstr(row, substring);
        if (match == NULL) {
            continue;
        }

        const int x = (int) (match - row);
        int brightness = displayBuffer.cells[x][y].foreColorComponents[0];
        if (displayBuffer.cells[x][y].foreColorComponents[1] > brightness) {
            brightness = displayBuffer.cells[x][y].foreColorComponents[1];
        }
        if (displayBuffer.cells[x][y].foreColorComponents[2] > brightness) {
            brightness = displayBuffer.cells[x][y].foreColorComponents[2];
        }
        return brightness;
    }
    return -1;
}

void printTestMessages(void) {
    for (int i = 0; i < messageLogCount; i++) {
        fprintf(stderr,"-- %s\n", messageLog[i]);
    }
}

boolean startupMessageWasShown(const char *substring) {
    return startupMessageWith(substring) != NULL;
}

const char *startupMessageWith(const char *substring) {
    for (int i = 0; i < startupMessageCount; i++) {
        if (strstr(startupMessageLog[i], substring) != NULL) {
            return startupMessageLog[i];
        }
    }
    return NULL;
}

void emitArchivedMessage(const char *msg) {
    // In test mode message() captures raw text and returns before touching the
    // archive, collapsing, or the display.  Run this one message through the
    // normal-mode pipeline so the archived entry, its repeat count, and the
    // rendered line all behave as they do in a real game.
    enum gameMode savedMode = rogue.mode;
    rogue.mode = GAME_MODE_NORMAL;
    message(msg, 0);
    rogue.mode = savedMode;
}

const char *renderedMessageLine(void) {
    static char line[COLS * 2];
    char buffer[MESSAGE_LINES][COLS * 2];

    formatRecentMessages(buffer, MESSAGE_LINES, NULL, NULL);
    snprintf(line, sizeof(line), "%s", buffer[MESSAGE_LINES - 1]);
    return line;
}

boolean sidebarContains(const char *substring) {
    char row[STAT_BAR_WIDTH + 1];

    refreshSideBar(-1, -1, false);
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < STAT_BAR_WIDTH; x++) {
            int glyph = (int) displayBuffer.cells[x][y].character;
            row[x] = (glyph >= ' ' && glyph < 127) ? (char) glyph : ' ';
        }
        row[STAT_BAR_WIDTH] = '\0';
        if (strstr(row, substring) != NULL) {
            return true;
        }
    }
    return false;
}


// Feed the queued input through the engine's main input loop and return as
// soon as the queue runs dry.  See the declaration in TestFramework.h.
void runQueuedInputThroughMainLoop(void) {
    inputLoopPumpActive = true;
    mainInputLoop();
    inputLoopPumpActive = false;
    clearInputQueue();
    rogue.gameHasEnded = false;
}

void runTestByName(const char *name) {
    testStartDepth = rogue.depthLevel;

    for (const testCase *const *suite = testSuites; *suite != NULL; suite++) {
        for (const testCase *currentTest = *suite; currentTest->name != NULL; currentTest++) {
            if (strcmp(currentTest->name, name) != 0) {
                continue;
            }

            testStepCount = 0;
            testFailed = false;
            failureMessage[0] = '\0';
            // Keep what the engine said while the game was starting up: the
            // log below is cleared for the test itself.
            memcpy(startupMessageLog, messageLog, sizeof(messageLog));
            startupMessageCount = messageLogCount;
            clearInputQueue();
            clearMessages();
            playerDeathCauseText[0] = '\0';
            highScoresDisplayed = false;
            screenSnapshotValid = false;
            rememberedColorsValid = false;

            // Setup
            if (currentTest->setup) currentTest->setup();

            // Run steps until test ends or game ends
            while (!rogue.gameHasEnded && !testFailed) {
                if (!currentTest->step || !currentTest->step()) {
                    break; // Test signaled completion
                }
                testStepCount++;
                if (testStepCount > 10000) { // Safety limit
                    testFail("Test exceeded 10000 steps");
                    break;
                }
            }

            // Verify
            boolean passed = true;
            if (currentTest->verify) {
                passed = currentTest->verify();
            }

            printf("\n=== TEST %s: %s ===\n", name, passed && !testFailed ? "PASS" : "FAIL");
            if (testFailed) {
                printf("        Failure: %s\n", failureMessage);
            }

            rogue.gameHasEnded = true; // Exit game after test
            rogue.nextGame = NG_QUIT;
            return;
        }
    }
    printf("Test not found: %s\n", name);
    rogue.nextGame = NG_QUIT;
}

void testFail(const char *msg) {
    if (testFailed) return; // Some prior test already failed, don't overwrite it.
    testFailed = true;
    snprintf(failureMessage, sizeof(failureMessage), "%s", msg);
}

// ---------------------------------------------
// Positions
// ---------------------------------------------

pos posAt(int x, int y) {
    pos loc;
    loc.x = x;
    loc.y = y;
    return loc;
}

pos posOffset(pos origin, int dx, int dy) {
    return posAt(origin.x + dx, origin.y + dy);
}

// ---------------------------------------------
// Event Injection
// ---------------------------------------------

void injectKeystroke(int key, boolean shift, boolean control) {
    rogueEvent evt = {0};
    evt.eventType = KEYSTROKE;
    evt.param1 = key;
    evt.shiftKey = shift;
    evt.controlKey = control;
    executeEvent(&evt);
}

boolean isKeyboardCursorActive(void) {
    return rogue.cursorMode;
}

void injectMouseClick(pos loc, boolean rightClick, boolean control, boolean shift) {
    rogueEvent evt = {0};
    evt.eventType = rightClick ? RIGHT_MOUSE_UP : MOUSE_UP;
    evt.param1 = loc.x;
    evt.param2 = loc.y;
    evt.controlKey = control;
    evt.shiftKey = shift;
    executeMouseClick(&evt);
}

void restOneTurn(void) {
    injectKeystroke(PERIOD_KEY, false, false);
}

void moveDirection(enum directions dir) {
    static const int keys[] = {
        [UP] = UP_KEY, [DOWN] = DOWN_KEY, [LEFT] = LEFT_KEY, [RIGHT] = RIGHT_KEY,
        [UPLEFT] = UPLEFT_KEY, [UPRIGHT] = UPRIGHT_KEY,
        [DOWNLEFT] = DOWNLEFT_KEY, [DOWNRIGHT] = DOWNRIGHT_KEY
    };
    if (dir >= UP && dir <= DOWNRIGHT) {
        injectKeystroke(keys[dir], false, false);
    }
}

void moveRight(void)      { moveDirection(RIGHT); }
void moveLeft(void)       { moveDirection(LEFT); }
void moveUp(void)         { moveDirection(UP); }
void moveDown(void)       { moveDirection(DOWN); }
void moveUpLeft(void)     { moveDirection(UPLEFT); }
void moveUpRight(void)    { moveDirection(UPRIGHT); }
void moveDownLeft(void)   { moveDirection(DOWNLEFT); }
void moveDownRight(void)  { moveDirection(DOWNRIGHT); }

void searchOneTurn(void) {
    injectKeystroke(SEARCH_KEY, false, false);
}

boolean playerAttacksMonster(creature *monst) {
    if (monst == NULL) {
        return false;
    }
    return attack(&player, monst, false);
}

// ---------------------------------------------
// Terrain
// ---------------------------------------------

void clearTerrain(pos loc) {
    pcell *cell = &(pmap[loc.x][loc.y]);
    memset(cell, 0, sizeof(pcell));   // Clears everything (layers, gas, items, etc.)
}

void setTerrain(pos loc, short terrainType) {
    clearTerrain(loc);
    pmap[loc.x][loc.y].layers[DUNGEON] = terrainType;
    refreshDungeonCell(loc);
}

void setTerrainAroundLoc(pos loc, short terrainType) {
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            setTerrain(posOffset(loc, dx, dy), terrainType);
        }
    }
}

void spawnFeature(pos loc, short feature) {
    spawnDungeonFeature(loc.x, loc.y, &dungeonFeatureCatalog[feature], true, false);
}

void revealCell(pos loc) {
    // Mark a cell as seen so tests can exercise the "known terrain" prompts.
    pmapAt(loc)->flags |= (VISIBLE | DISCOVERED);
    refreshDungeonCell(loc);
}

void placeDownStairs(pos loc) {
    setTerrain(loc, DOWN_STAIRS);
    pmapAt(loc)->flags |= HAS_STAIRS;
    rogue.downLoc = loc;
}

void placeUpStairs(pos loc) {
    setTerrain(loc, UP_STAIRS);
    pmapAt(loc)->flags |= HAS_STAIRS;
    rogue.upLoc = loc;
}

short terrainAt(pos loc) {
    return pmapAt(loc)->layers[DUNGEON];
}

boolean cellIsTerrain(pos loc, short terrainType) {
    for (int layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
        if (pmapAt(loc)->layers[layer] == terrainType) {
            return true;
        }
    }
    return false;
}

boolean cellObstructsPassage(pos loc) {
    return cellHasTerrainFlag(loc, T_OBSTRUCTS_PASSABILITY);
}

boolean cellIsVisible(pos loc) {
    // playerCanSee expands to a flag mask; normalize it, since high flag bits
    // would otherwise be lost when truncated to a boolean.
    return (playerCanSee(loc.x, loc.y)) != 0;
}

boolean cellIsDiscovered(pos loc) {
    if (!isPosInMap(loc)) {
        return false;
    }
    return (pmapAt(loc)->flags & DISCOVERED) != 0;
}

boolean cellHasGas(pos loc) {
    return pmapAt(loc)->volume > 0;
}

boolean cellIsOnFire(pos loc) {
    return cellHasTerrainFlag(loc, T_IS_FIRE);
}

boolean cellIsHarmfulTerrain(pos loc) {
    return cellHasTerrainFlag(loc, T_HARMFUL_TERRAIN);
}

boolean cellIsTrap(pos loc) {
    return cellHasTerrainFlag(loc, T_IS_DF_TRAP);
}

boolean isTrapTriggered(pos loc) {
    return (pmapAt(loc)->flags & PRESSURE_PLATE_DEPRESSED) != 0;
}

boolean cellIsKnownTrapFree(pos loc) {
    return (pmapAt(loc)->flags & KNOWN_TO_BE_TRAP_FREE) != 0;
}

boolean cellIsVaultInterior(pos loc) {
    return pmapAt(loc)->machineNumber != 0;
}

boolean cellIsHallway(pos loc) {
    if (!isPosInMap(loc)) {
        return false;
    }
    // Two passable arcs means the tile is pinched between walls, as in a
    // one-tile-wide corridor; open rooms have fewer, junctions more.
    return passableArcCount(loc.x, loc.y) > 1;
}

boolean cellHasLiquid(pos loc) {
    return pmapAt(loc)->layers[LIQUID] != NOTHING;
}

boolean cellIsInShadow(pos loc) {
    if (!isPosInMap(loc)) {
        return false;
    }
    return (pmapAt(loc)->flags & IS_IN_SHADOW) != 0;
}

static const char *colorRoleName(const color *theColor) {
    if (theColor == &torchColor || theColor == &torchLightColor) {
        return "torch orange";
    }
    return "other";
}

const char *terrainDrawColorName(pos loc) {
    if (!isPosInMap(loc)) {
        return "other";
    }
    return colorRoleName(tileCatalog[pmapAt(loc)->layers[DUNGEON]].foreColor);
}

boolean terrainDrawColorFlickers(pos loc) {
    if (!isPosInMap(loc)) {
        return false;
    }
    return tileCatalog[pmapAt(loc)->layers[DUNGEON]].foreColor->colorDances;
}

const char *terrainGlowLightColorName(pos loc) {
    if (!isPosInMap(loc)) {
        return "other";
    }
    const short glowLight = tileCatalog[pmapAt(loc)->layers[DUNGEON]].glowLight;
    if (glowLight <= 0 || glowLight >= NUMBER_LIGHT_KINDS) {
        return "other";
    }
    return colorRoleName(lightCatalog[glowLight].lightColor);
}

// ---------------------------------------------
// Player state
// ---------------------------------------------

pos getPlayerPos(void) {
    return player.loc;
}

void setPlayerPos(pos loc) {
    setMonsterPos(&player, loc);
}

boolean playerIsAt(pos loc) {
    return player.loc.x == loc.x && player.loc.y == loc.y;
}

boolean playerIsOnTerrain(short terrainType) {
    return cellIsTerrain(player.loc, terrainType);
}

boolean playerHasStatus(enum statusEffects status) {
    return player.status[status] > 1;
}

void applyStatusToPlayer(enum statusEffects status, int duration) {
    player.status[status] = player.maxStatus[status] = duration;
}

void removeStatusFromPlayer(enum statusEffects status) {
    player.status[status] = player.maxStatus[status] = 0;
}

void setPlayerHealth(int hp) {
    setMonsterHealth(&player, hp);
}

int playerHealth(void) {
    return player.currentHP;
}

int playerMaxHealth(void) {
    return player.info.maxHP;
}

void damagePlayer(int amount) {
    player.currentHP -= amount;
    if (player.currentHP < 0) {
        player.currentHP = 0;
    }
}

void woundPlayer(int amount) {
    inflictDamage(NULL, &player, amount, NULL, true);
}

void enterEasyMode(void) {
    rogue.mode = GAME_MODE_EASY;
}

// ---------------------------------------------
// Game start-up
// ---------------------------------------------

// See the declaration in TestFramework.h.
void restartGame(void) {
    uint64_t seed = dungeonSeed();
    initializeRogue(seed);
    startLevel(rogue.depthLevel, 1);
}

// ---------------------------------------------
// Game modes and high scores
// ---------------------------------------------

// The main menu waits for input only inside pauseForMilliseconds, which the
// null console always answers with "no input arrived" — the menu's flame loop
// would spin forever in test mode.  While showVariantDialog() runs the menu,
// report input unconditionally so the menu pumps the test's queued keystrokes
// through nextBrogueEvent() instead, exactly as a live console would.
static boolean menuPauseSeesQueuedInput(short milliseconds, PauseBehavior behavior) {
    return true;
}

void showVariantDialog(int answerKey) {
    boolean (*savedPause)(short, PauseBehavior) = currentConsole.pauseForMilliseconds;
    currentConsole.pauseForMilliseconds = menuPauseSeesQueuedInput;

    // The menu path, hidden from the test: open the Play flyout, choose
    // "Change Variant", answer the dialog, then quit the menu.
    rogue.nextGame = NG_NOTHING; // a prior dialog run leaves NG_QUIT behind
    queueKeystroke('p', false, false);
    queueKeystroke('a', false, false);
    queueKeystroke(answerKey, false, false);
    queueKeystroke('q', false, false);
    mainBrogueJunction();

    currentConsole.pauseForMilliseconds = savedPause;
    clearInputQueue();
}

// See the declaration in TestFramework.h.
void showModeDialog(int answerKey) {
    boolean (*savedPause)(short, PauseBehavior) = currentConsole.pauseForMilliseconds;
    currentConsole.pauseForMilliseconds = menuPauseSeesQueuedInput;

    // The menu path, hidden from the test: open the Play flyout, choose
    // "Change Mode", answer the dialog, then quit the menu.
    rogue.nextGame = NG_NOTHING; // a prior dialog run leaves NG_QUIT behind
    queueKeystroke('p', false, false);
    queueKeystroke('m', false, false);
    queueKeystroke(answerKey, false, false);
    queueKeystroke('q', false, false);
    mainBrogueJunction();

    currentConsole.pauseForMilliseconds = savedPause;
    clearInputQueue();
}

// See the declaration in TestFramework.h.
void showTitleScreen(void) {
    boolean (*savedPause)(short, PauseBehavior) = currentConsole.pauseForMilliseconds;
    currentConsole.pauseForMilliseconds = menuPauseSeesQueuedInput;

    // The menu path, hidden from the test: press Quit on the title screen.
    // The menu draws its buttons before it waits for that key, so the frame
    // the engine captures at the input request is the title screen itself.
    rogue.nextGame = NG_NOTHING; // a prior dialog run leaves NG_QUIT behind
    queueKeystroke('q', false, false);
    mainBrogueJunction();

    currentConsole.pauseForMilliseconds = savedPause;
    clearInputQueue();
}

// See the declaration in TestFramework.h.
boolean showOversizedButton(const char *label) {
    brogueButton button;
    initializeButton(&button);
    snprintf(button.text, BUTTON_TEXT_SIZE, "%s", label);
    button.hotkey[0] = RETURN_KEY;

    // Centred like any other dialog: a label wider than the screen is pushed
    // past both edges, which is the situation the clipping rule describes.
    const int width = strLenWithoutEscapes(button.text);
    button.x = (COLS - width) / 2;
    button.y = ROWS / 2;

    queueKeystroke(RETURN_KEY, false, false);
    const short chosen = buttonInputLoop(&button, 1, 0, button.y, COLS, 1, NULL);
    clearInputQueue();
    return chosen == 0;
}

// ---------------------------------------------
// Menu buttons
// ---------------------------------------------

// Opacity the button painter stored in each cell of the most recently drawn
// menu, captured by testRecordButtonCellOpacity() while a test runs.
static short buttonOpacityRecord[COLS][ROWS];
static boolean buttonOpacityRecorded[COLS][ROWS];

static void clearButtonOpacityRecord(void) {
    memset(buttonOpacityRecorded, 0, sizeof(buttonOpacityRecorded));
}

void testRecordButtonCellOpacity(short x, short y, short opacity) {
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS) {
        return;
    }
    buttonOpacityRecord[x][y] = opacity;
    buttonOpacityRecorded[x][y] = true;
}

static int recordedButtonOpacity(void) {
    int found = -1;
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            if (!buttonOpacityRecorded[x][y]) {
                continue;
            }
            if (found >= 0 && found != buttonOpacityRecord[x][y]) {
                return -2;
            }
            found = buttonOpacityRecord[x][y];
        }
    }
    return found;
}

// See the declaration in TestFramework.h.
int buttonBackdropVisibility(int opacity) {
    SavedDisplayBuffer saved = saveDisplayBuffer();
    screenDisplayBuffer dbuf;
    brogueButton button;

    // A solid white screen and a black button make the composited colour a
    // direct reading of how much of the screen survives: white * (100-p).
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            displayBuffer.cells[x][y].character = ' ';
            displayBuffer.cells[x][y].foreColorComponents[0] = 0;
            displayBuffer.cells[x][y].foreColorComponents[1] = 0;
            displayBuffer.cells[x][y].foreColorComponents[2] = 0;
            displayBuffer.cells[x][y].backColorComponents[0] = 100;
            displayBuffer.cells[x][y].backColorComponents[1] = 100;
            displayBuffer.cells[x][y].backColorComponents[2] = 100;
        }
    }

    initializeButton(&button);
    snprintf(button.text, BUTTON_TEXT_SIZE, "%s", "Backdrop");
    button.x = 0;
    button.y = 0;
    button.buttonColor = black;
    button.flags &= ~B_GRADIENT;
    button.opacity = opacity;

    clearDisplayBuffer(&dbuf);
    drawButton(&button, BUTTON_NORMAL, &dbuf);

    // Every cell the label covers must carry the opacity; one unmarked cell
    // would let the screen behind it stay unblended.
    const int width = strLenWithoutEscapes(button.text);
    for (int i = 0; i < width && button.x + i < COLS; i++) {
        if (dbuf.cells[button.x + i][button.y].opacity != opacity) {
            restoreDisplayBuffer(&saved);
            return -1;
        }
    }

    overlayDisplayBuffer(&dbuf);

    const int showing = displayBuffer.cells[0][0].backColorComponents[0];
    restoreDisplayBuffer(&saved);
    return showing;
}

// See the declaration in TestFramework.h.
int inventoryButtonOpacity(void) {
    if (packItemCount() == 0) {
        givePlayerItem(FOOD, RATION, 0);
    }
    clearButtonOpacityRecord();
    queueKeystroke(ESCAPE_KEY, false, false);
    displayInventory(ALL_ITEMS, 0, 0, true, true);
    clearInputQueue();
    return recordedButtonOpacity();
}

// See the declaration in TestFramework.h.
int bottomRowButtonOpacity(void) {
    clearButtonOpacityRecord();
    runQueuedInputThroughMainLoop();
    return recordedButtonOpacity();
}

// See the declaration in TestFramework.h.
void paintMenuButton(const char *label) {
    brogueButton button;
    screenDisplayBuffer dbuf;

    initializeButton(&button);
    snprintf(button.text, BUTTON_TEXT_SIZE, "%s", label);
    button.x = 0;
    button.y = 0;
    // A button colour with a random component makes the painter draw
    // randomness while it works, which is what the rule protects the game's
    // own sequence from.
    button.buttonColor.rand = 50;

    clearDisplayBuffer(&dbuf);
    drawButton(&button, BUTTON_NORMAL, &dbuf);
}

// See the declaration in TestFramework.h.
unsigned long gameplayRandomDraws(void) {
    return randomNumbersGenerated;
}

// See the declaration in TestFramework.h.
boolean gameplayRandomStreamActive(void) {
    return rogue.RNG == RNG_SUBSTANTIVE;
}

// See the declaration in TestFramework.h.
int drawGameplayRandomNumber(void) {
    return (int) rand_range(0, 100);
}

const char *gameVariantName(void) {
    switch (gameVariant) {
        case VARIANT_RAPID_BROGUE:  return "Rapid Brogue";
        case VARIANT_BULLET_BROGUE: return "Bullet Brogue";
        default:                    return "Brogue";
    }
}

void attemptWizardItemCreation(void) {
    dialogCreateItemOrMonster();
}

void enterWizardMode(void) {
    rogue.mode = GAME_MODE_WIZARD;
}

void enterPlaybackMode(void) {
    rogue.playbackMode = true;
}

// See the declaration in TestFramework.h.  A replayed player turn runs the
// engine's whole per-turn machinery — lighting and vision consume
// randomness, and the out-of-sync check reads the recording — so the checks
// are recorded from two real passes over the same steps: the first settles
// the state that only changes on a tile's first visit (discovery, the
// auto-search marker), the second records; the replay then starts from the
// same state and seed as the recording pass and reproduces its consumption
// turn for turn.  In test mode a playback seek never reads events from the
// buffer (nextBrogueEvent answers from the test's queue instead), so only
// the checks are read back, one per replayed player turn.
//
// The steps must mirror what the test queues after this call: alternating
// UPRIGHT/DOWNLEFT keystrokes, which net zero displacement.
void startPlaybackWithRecordedChecks(unsigned int turns) {
    if (turns == 0) {
        testFail("startPlaybackWithRecordedChecks needs at least one turn");
        return;
    }

    // The periodic environment update runs inside a player turn; push it
    // beyond all three passes (30000 ticks covers 300 turns of them) so none
    // of them takes it.  Nothing resynchronizes it while the game stays on
    // one level.
    rogue.ticksTillUpdateEnvironment = 30000;

    // Settling pass: walked once so the recording pass and the replay both
    // see tiles the player has already stepped on and searched.
    for (unsigned int i = 0; i < turns; i++) {
        injectKeystroke((i % 2) ? DOWNLEFT_KEY : UPRIGHT_KEY, false, false);
    }

    // Recording pass, from a fresh generator state.
    locationInRecordingBuffer = 0;
    recordingLocation = 0;
    seedRandomGenerator(rogue.seed);
    for (unsigned int i = 0; i < turns; i++) {
        injectKeystroke((i % 2) ? DOWNLEFT_KEY : UPRIGHT_KEY, false, false);
    }

    // Collect the recorded out-of-sync checks (one per turn, interleaved
    // with the recorded keystrokes, which the replay never reads) and write
    // them back as one contiguous run for the replay to start on.
    unsigned int checkCount = 0;
    unsigned char *buffer = inputRecordBuffer;
    unsigned short bufferLength = locationInRecordingBuffer;
    for (unsigned short p = 0; p + 1 < bufferLength; p++) {
        if (buffer[p] == RNG_CHECK) {
            buffer[checkCount * 2] = RNG_CHECK;
            buffer[checkCount * 2 + 1] = buffer[p + 1];
            checkCount++;
            p++;
        }
    }
    if (checkCount != turns) {
        testFail("recording pass did not record one out-of-sync check per turn");
        return;
    }
    // Rewind so the replay reads the checks back from the start; the file
    // length (not the position) records how much is there.
    locationInRecordingBuffer = 0;
    recordingLocation = 0;
    seedRandomGenerator(rogue.seed);

    lengthOfPlaybackFile = 100000; // so recallChar() never hits end-of-recording
    // Room beyond `turns` so an over-large advance is not clamped down to
    // the expected answer by the recording's length.
    rogue.howManyTurns = rogue.playerTurnNumber + 2 * turns;
    rogue.playbackMode = true;
}

boolean isInPlaybackMode(void) {
    return rogue.playbackMode;
}

boolean playbackIsOutOfSync(void) {
    return rogue.playbackOOS;
}

// See the declaration in TestFramework.h.  A test game has no recording of
// its own, so one is opened mid-game and a couple of turns are played; the
// recording therefore begins with a turn, and the replay's first out-of-sync
// check reads that turn where it expects a check, raising the panic.  The
// queued input acknowledges the panic's message and its input loop, then
// quits.  The final quit is consumed by the menu's playback loop, not by the
// panic loop, so the menu's cleanup is what clears the playback state.
void viewCurrentRecordingAndQuitPlayback(void) {
    char path[BROGUE_FILENAME_MAX];

    if (rogue.playbackMode) {
        testFail("setup: cannot save a recording while already in playback");
        return;
    }

    // Test games are initialized without a recording file, so open one and
    // take a couple of real turns to give the menu something to play.
    if (!rogue.recording) {
        snprintf(currentFilePath, BROGUE_FILENAME_MAX, "test-playback%s", RECORDING_SUFFIX);
        initRecording();
        restOneTurn();
        restOneTurn();
    }

    flushBufferToFile();
    path[0] = '\0';
    saveRecordingNoPrompt(path);
    if (path[0] == '\0' || !fileExists(path)) {
        testFail("setup: the current game's recording could not be saved");
        return;
    }

    // The menu quits after the playback instead of returning to the title
    // screen, so this run never blocks waiting for a menu choice.
    enableServerMode();
    rogue.nextGame = NG_VIEW_RECORDING;
    strcpy(rogue.nextGamePath, path);

    queueKeystroke(ACKNOWLEDGE_KEY, false, false); // leave the opening pause
    queueKeystroke(PERIOD_KEY, false, false);      // one turn, if the replay gets that far
    queueKeystroke(ACKNOWLEDGE_KEY, false, false); // dismiss the out-of-sync message
    queueKeystroke(ACKNOWLEDGE_KEY, false, false); // leave the panic's input loop
    queueKeystroke(QUIT_KEY, false, false);        // quit the playback from the menu

    mainBrogueJunction();
    clearInputQueue();
    remove(path);
}

void enableServerMode(void) {
    serverMode = true;
}

void givePlayerGold(int amount) {
    rogue.gold = amount;
}

// Mirrors the score an ordinary (escape) victory computes: gold plus the
// value of every carried item.  A greater victory doubles the amulet's value
// on top of this, which callers add themselves.
int expectedVictoryScore(void) {
    long total = rogue.gold;
    for (item *theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (itemValue(theItem) > 0) {
            total += itemValue(theItem);
        }
    }
    return (int) total;
}

void recordHighScoresDisplayed(void) {
    highScoresDisplayed = true;
}

boolean highScoresWereDisplayed(void) {
    return highScoresDisplayed;
}

boolean highScoreHasScore(int score) {
    rogueHighScoresEntry list[HIGH_SCORES_COUNT];
    getHighScoresList(list); // re-reads the high-score file each call
    for (int i = 0; i < HIGH_SCORES_COUNT; i++) {
        if (list[i].score == (long) score) {
            return true;
        }
    }
    return false;
}

const char *highScoreDescription(int score) {
    static char description[DCOLS];
    rogueHighScoresEntry list[HIGH_SCORES_COUNT];
    getHighScoresList(list);
    for (int i = 0; i < HIGH_SCORES_COUNT; i++) {
        if (list[i].score == (long) score) {
            strcpy(description, list[i].description);
            return description;
        }
    }
    description[0] = '\0';
    return description;
}

void clearHighScores(void) {
    // Same zeroed layout the engine writes for a missing file, so the next
    // load or save sees a full list of empty entries with a lowest score of 0.
    char highScoresFilename[BROGUE_FILENAME_MAX];
    setHighScoresFilename(highScoresFilename, BROGUE_FILENAME_MAX);
    FILE *scoresFile = fopen(highScoresFilename, "w");
    if (scoresFile == NULL) {
        testFail("could not open the high-score file to clear it");
        return;
    }
    for (int i = 0; i < HIGH_SCORES_COUNT; i++) {
        fprintf(scoresFile, "%li\t%li\t%s", (long) 0, (long) 0, "(empty entry)\n");
    }
    fclose(scoresFile);
}

int playerNutrition(void) {
    return player.status[STATUS_NUTRITION];
}

void setPlayerNutrition(int nutrition) {
    player.status[STATUS_NUTRITION] = nutrition;
}

boolean isPlayerHurt(void) {
    return player.currentHP < player.info.maxHP;
}

boolean isPlayerHealthy(void) {
    return player.currentHP >= player.info.maxHP;
}

boolean isPlayerDead(void) {
    return player.currentHP <= 0 || rogue.gameHasEnded;
}

boolean runHasEnded(void) {
    return rogue.gameHasEnded;
}

void recordPlayerDeathCause(const char *cause) {
    snprintf(playerDeathCauseText, sizeof(playerDeathCauseText), "%s", cause ? cause : "");
}

const char *playerDeathCause(void) {
    return playerDeathCauseText;
}

boolean playerDeathWasAttributedTo(const char *cause) {
    return cause != NULL && strcmp(playerDeathCauseText, cause) == 0;
}

boolean isPlayerLevitating(void) {
    return playerHasStatus(STATUS_LEVITATING);
}

boolean isPlayerBurning(void) {
    return playerHasStatus(STATUS_BURNING);
}

boolean isPlayerConfused(void) {
    return playerHasStatus(STATUS_CONFUSED);
}

boolean isPlayerParalyzed(void) {
    return playerHasStatus(STATUS_PARALYZED);
}

boolean isPlayerStuck(void) {
    return playerHasStatus(STATUS_STUCK);
}

boolean playerIsSeized(void) {
    return (player.bookkeepingFlags & MB_SEIZED) != 0;
}

boolean isPlayerSubmerged(void) {
    // The player's submersion is tracked as a visual state, unlike monsters.
    return rogue.inWater != 0;
}

boolean isPlayerPoisoned(void) {
    return player.status[STATUS_POISONED] > 0;
}

boolean isPlayerWeakened(void) {
    return player.status[STATUS_WEAKENED] > 0;
}

boolean playerIsInDarkness(void) {
    return playerInDarkness();
}

boolean playerHasFeat(short feat) {
    return rogue.featRecord[feat] != 0;
}

int playerStatusTurns(enum statusEffects status) {
    return player.status[status];
}

int playerWeaknessAmount(void) {
    return player.weaknessAmount;
}

int playerPoisonConcentration(void) {
    return player.poisonAmount;
}

int playerShield(void) {
    return player.status[STATUS_SHIELDED];
}

int playerMovementSpeed(void) {
    return player.movementSpeed;
}

int playerAttackSpeed(void) {
    return player.attackSpeed;
}

int playerArmorValue(void) {
    return rogue.armor ? rogue.armor->armor : 0;
}

void weakenPlayer(int duration) {
    weaken(&player, duration);
}

void poisonPlayer(int duration, int concentration) {
    addPoison(&player, duration, concentration);
}

void slowPlayer(int turns) {
    slow(&player, turns);
}

void hastePlayer(int turns) {
    haste(&player, turns);
}

void healPlayerPanacea(void) {
    heal(&player, 100, true);
}

void applyTileEffectsToPlayer(void) {
    applyInstantTileEffectsToCreature(&player);
}

void wearArmor(short kind) {
    item *theItem = givePlayerItem(ARMOR, kind, 0);
    if (theItem != NULL) {
        equipItem(theItem, false, NULL);
    }
}

void takeOffArmor(void) {
    if (rogue.armor != NULL) {
        unequipItem(rogue.armor, true);
    }
}

void applyEquipmentStatusToPlayer(enum statusEffects status, int duration) {
    applyStatusToPlayer(status, duration);
    recalculateEquipmentBonuses();
}

// ---------------------------------------------
// Monsters
// ---------------------------------------------

void clearMonsters(void) {
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *m = nextCreature(&it);
        killCreature(m, true);
    }
    removeDeadMonsters();

    // Don't let new monsters spawn while testing
    rogue.monsterSpawnFuse = 10000;
}

creature *spawnMonster(enum monsterTypes type, pos loc) {
    creature *monst = generateMonster(type, false, false);
    setMonsterPos(monst, loc);
    return monst;
}

// See the declaration in TestFramework.h.
creature *spawnAlliedHordeWithFollowers(pos loc, int followers, int *membersSpawned) {
    const hordeType *savedCatalog = hordeCatalog;
    const int hordeCount = gameConst->numberHordes;
    hordeType *catalog = malloc((hordeCount + 1) * sizeof(hordeType));
    if (catalog == NULL) {
        testFail("could not allocate a horde table");
        return NULL;
    }
    memcpy(catalog, hordeCatalog, hordeCount * sizeof(hordeType));

    hordeType *alliedHorde = &catalog[hordeCount];
    memset(alliedHorde, 0, sizeof(*alliedHorde));
    alliedHorde->leaderType = MK_RAT;
    alliedHorde->numberOfMemberTypes = 1;
    alliedHorde->memberType[0] = MK_RAT;
    alliedHorde->memberCount[0] = (randomRange){ followers, followers, 1 };
    alliedHorde->minLevel = 1;
    alliedHorde->maxLevel = gameConst->deepestLevel;
    alliedHorde->frequency = 100;
    alliedHorde->flags = HORDE_ALLIED_WITH_PLAYER;

    hordeCatalog = catalog;
    const int before = monsterCount();
    creature *leader = spawnHorde(hordeCount, loc, 0, 0);
    hordeCatalog = savedCatalog;
    free(catalog);

    if (membersSpawned != NULL) {
        *membersSpawned = monsterCount() - before - (leader != NULL ? 1 : 0);
    }
    return leader;
}

creature *findMonster(enum monsterTypes type) {
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *m = nextCreature(&it);
        if (m->info.monsterID == type) return m;
    }
    return NULL;
}

int monsterCount(void) {
    int count = 0;
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        nextCreature(&it);
        count++;
    }
    return count;
}

int countMonstersOfType(enum monsterTypes monstTyp) {
    int count = 0;
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *m = nextCreature(&it);
        if (m != NULL && m->info.monsterID == monstTyp) count++;
    }
    return count;
}

creature *monsterAtIndex(int index) {
    int i = 0;
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *m = nextCreature(&it);
        if (i == index) return m;
        i++;
    }
    return NULL;
}

void setMonsterPos(creature *monst, pos loc) {
    unsigned long creatureFlag = (monst == &player ? HAS_PLAYER : HAS_MONSTER);
    pmapAt(monst->loc)->flags &= ~creatureFlag;
    refreshDungeonCell(monst->loc);

    monst->loc = loc;
    pmapAt(monst->loc)->flags |= creatureFlag;
    refreshDungeonCell(monst->loc);
}

void makeMonsterHostile(creature *monst) {
    monst->creatureState = MONSTER_TRACKING_SCENT;
}

void makeMonsterAlly(creature *monst) {
    monst->creatureState = MONSTER_ALLY;
}

int monsterExperience(const creature *monst) {
    return monst->xpxp;
}

void wakeMonster(creature *monst) {
    monst->creatureState = MONSTER_TRACKING_SCENT;
}

void putMonsterToSleep(creature *monst) {
    monst->creatureState = MONSTER_SLEEPING;
}

void makeMonsterUnaware(creature *monst) {
    monst->creatureState = MONSTER_WANDERING;
}

void applyStatusToMonster(creature *monst, enum statusEffects status, int duration) {
    monst->status[status] = monst->maxStatus[status] = duration;
    switch (status) {
        case STATUS_POISONED: monst->poisonAmount = 1; break;
        case STATUS_WEAKENED: monst->weaknessAmount = 1; break;
        default: break;
    }
}

boolean monsterHasStatus(const creature *monst, enum statusEffects status) {
    return monst->status[status] > 0;
}

boolean monsterIsDead(const creature *monst) {
    return monst->currentHP <= 0 || (monst->bookkeepingFlags & MB_IS_DYING);
}

boolean monsterIsAlive(const creature *monst) {
    return !monsterIsDead(monst);
}

boolean monsterIsHealthy(const creature *monst) {
    if (monsterIsDead(monst)) return false;
    return monst->currentHP >= monst->info.maxHP;
}

boolean monsterIsHurt(const creature *monst) {
    return !monsterIsHealthy(monst);
}

boolean monsterIsAlly(const creature *monst) {
    return monst->creatureState == MONSTER_ALLY;
}

boolean monsterIsHostile(const creature *monst) {
    return monst != &player && monst->creatureState != MONSTER_ALLY;
}

boolean monsterIsAsleep(const creature *monst) {
    return monst->creatureState == MONSTER_SLEEPING;
}

boolean monsterIsWandering(const creature *monst) {
    return monst->creatureState == MONSTER_WANDERING;
}

boolean monsterIsTrackingPlayer(const creature *monst) {
    return monst->creatureState == MONSTER_TRACKING_SCENT;
}

boolean monsterIsFleeing(const creature *monst) {
    return monst->creatureState == MONSTER_FLEEING;
}

int monsterTicksUntilTurn(const creature *monst) {
    return monst->ticksUntilTurn;
}

void giveMonsterItem(creature *monst, short category, short kind) {
    monst->carriedItem = generateItem(category, kind);
}

pos monsterPosition(const creature *monst) {
    return monst->loc;
}

int monsterDepth(const creature *monst) {
    return monst->depth;
}

int monsterHealth(const creature *monst) {
    return monst->currentHP;
}

void setMonsterHealth(creature *monst, int hp) {
    if (hp < 1) hp = 1;
    // if (hp > monst->info.maxHP) hp = monst->info.maxHP;
    if (hp > monst->info.maxHP) monst->info.maxHP = hp;
    monst->currentHP = hp;
}

void damageMonster(creature *monst, int amount) {
    inflictDamage(NULL, monst, amount, NULL, true);
}

boolean monsterIsAdjacentToPlayer(const creature *monst) {
    int dx = monst->loc.x - player.loc.x;
    int dy = monst->loc.y - player.loc.y;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    int chebyshev = (dx > dy ? dx : dy);
    return chebyshev == 1;
}

boolean monsterAttacksPlayer(creature *monst) {
    if (monst == NULL) {
        return false;
    }
    return attack(monst, &player, false);
}

boolean attackMonster(creature *attacker, creature *defender) {
    if (attacker == NULL || defender == NULL) {
        return false;
    }
    return attack(attacker, defender, false);
}

void playerZapsAt( item *theItem, creature *monst ) {
    if (theItem->category != STAFF && theItem->category != WAND) return;

    bolt theBolt = boltCatalog[tableForItemCategory(theItem->category)[theItem->kind].power];
    zap(player.loc, monst->loc, &theBolt, false, false);
}

void playerZapsStaffAt( item *theItem, creature *monst ) {
    if (theItem->category != STAFF) return;

    bolt theBolt = boltCatalog[tableForItemCategory(theItem->category)[theItem->kind].power];
    theBolt.magnitude = theItem->enchant1;
    zap(player.loc, monst->loc, &theBolt, false, false);
}


void monsterTakesTurn(creature *monst) {
    if (monst != NULL) {
        monstersTurn(monst);
    }
}

void makeMonsterFollow(creature *follower, creature *leader) {
    if (follower != NULL && leader != NULL) {
        follower->bookkeepingFlags |= MB_FOLLOWER;
        follower->leader = leader;
    }
}

int monsterStatusTurns(const creature *monst, enum statusEffects status) {
    return monst->status[status];
}

int monsterWeaknessAmount(const creature *monst) {
    return monst->weaknessAmount;
}

int monsterMovementSpeed(const creature *monst) {
    return monst->movementSpeed;
}

int monsterAttackSpeed(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->attackSpeed;
}

boolean monsterHasTrait(const creature *monst, unsigned long trait) {
    if (monst == NULL) return false;
    return (monst->info.flags & trait) != 0;
}

boolean monsterHasAbility(const creature *monst, unsigned long ability) {
    if (monst == NULL) return false;
    return (monst->info.abilityFlags & ability) != 0;
}

boolean monsterCastsBolt(const creature *monst, short boltType) {
    if (monst == NULL) return false;
    for (int i = 0; i < 20; i++) {
        if (monst->info.bolts[i] == boltType) return true;
        if (monst->info.bolts[i] == 0) break;
    }
    return false;
}

int monsterMaxHealth(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.maxHP;
}

int monsterBaseDefense(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.defense;
}

int monsterBaseAccuracy(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.accuracy;
}

int monsterBaseDamageLow(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.damage.lowerBound;
}

int monsterBaseDamageHigh(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.damage.upperBound;
}

int monsterBaseMovementSpeed(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.movementSpeed;
}

int monsterBaseAttackSpeed(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.attackSpeed;
}

boolean monsterIsLarge(const creature *monst) {
    if (monst == NULL) return false;
    return monst->info.isLarge;
}

boolean monsterRegenerates(const creature *monst) {
    if (monst == NULL) return false;
    return monst->info.turnsBetweenRegen > 0;
}

int monsterTrailChance(const creature *monst) {
    if (monst == NULL) return 0;
    return monst->info.DFChance;
}

boolean monsterLeavesUrineTrail(const creature *monst) {
    if (monst == NULL) return false;
    return monst->info.DFType == DF_URINE;
}

void weakenMonster(creature *monst, int duration) {
    if (monst != NULL) {
        weaken(monst, duration);
    }
}

void poisonMonster(creature *monst, int duration, int concentration) {
    if (monst != NULL) {
        addPoison(monst, duration, concentration);
    }
}

void slowMonster(creature *monst, int turns) {
    if (monst != NULL) {
        slow(monst, turns);
    }
}

void hasteMonster(creature *monst, int turns) {
    if (monst != NULL) {
        haste(monst, turns);
    }
}

void setMonsterSubmerged(creature *monst, boolean submerged) {
    if (monst == NULL) {
        return;
    }
    if (submerged) {
        monst->bookkeepingFlags |= MB_SUBMERGED;
    } else {
        monst->bookkeepingFlags &= ~MB_SUBMERGED;
    }
}

void makeMonsterCaptive(creature *monst) {
    if (monst != NULL) {
        monst->bookkeepingFlags |= MB_CAPTIVE;
    }
}

// Mark the monster as dying through the engine's own death path — the state
// that stops anyone attacking it — but leave it in the monster chain;
// removeDeadMonsters() only runs on the next turn, so the test can still
// inspect it afterwards.
void makeMonsterDying(creature *monst) {
    if (monst != NULL && !(monst->bookkeepingFlags & MB_IS_DYING)) {
        killCreature(monst, false);
    }
}

void makeMonsterDormant(creature *monst) {
    if (monst != NULL) {
        monst->bookkeepingFlags |= MB_IS_DORMANT;
    }
}

boolean monsterIsHiddenFrom(const creature *monst, const creature *observer) {
    return monsterIsHidden(monst, observer);
}

boolean monsterIsRevealedToPlayer(const creature *monst) {
    return monsterRevealed((creature *)monst);
}

boolean monsterCouldAttack(const creature *attacker, const creature *defender) {
    return monsterWillAttackTarget(attacker, defender);
}

boolean monstersAreAlliesOf(const creature *a, const creature *b) {
    return monstersAreTeammates(a, b);
}

boolean monstersAreFoesOf(const creature *a, const creature *b) {
    return monstersAreEnemies(a, b);
}

boolean monsterAvoidsCell(const creature *monst, pos loc) {
    return monsterAvoids((creature *)monst, loc);
}

boolean monsterIsCarryingItem(const creature *monst) {
    return monst != NULL && monst->carriedItem != NULL;
}

// See the declaration in TestFramework.h.
item *monsterCarriedItem(const creature *monst) {
    return monst != NULL ? monst->carriedItem : NULL;
}

boolean monsterIsPermFleeing(const creature *monst) {
    return monst != NULL && monst->creatureMode == MODE_PERM_FLEEING;
}

void makeMonsterStealAndFlee(creature *monst, short category, short kind) {
    if (monst == NULL) {
        return;
    }
    monst->carriedItem = generateItem(category, kind);
    monst->creatureMode = MODE_PERM_FLEEING;
    monst->creatureState = MONSTER_FLEEING;
}

void monsterLosesCarriedItem(creature *monst) {
    if (monst == NULL || monst->carriedItem == NULL) {
        return;
    }
    makeMonsterDropItem(monst);
}

void makeMonsterFollowPlayer(creature *monst) {
    makeMonsterFollow(monst, &player);
}

// See the declaration in TestFramework.h.
void makeMonsterNotFollowPlayer(creature *monst) {
    if (monst != NULL) {
        monst->bookkeepingFlags |= MB_DOES_NOT_TRACK_LEADER;
    }
}

enum monsterTypes chooseMonsterTypeByProperty(enum boltType bolt, enum monsterBehaviorFlags behaviorFlagsInclude,  enum monsterBehaviorFlags behaviorFlagsExclude, enum monsterAbilityFlags abilityFlagsInclude, enum monsterAbilityFlags abilityFlagsExclude) {
    enum monsterTypes types[NUMBER_MONSTER_KINDS];
    int count = 0;

    for (int i = 0; i < NUMBER_MONSTER_KINDS; i++) {
        const creatureType *cTyp = &monsterCatalog[i];

        boolean includedBehaviors = (cTyp->flags & behaviorFlagsInclude) == behaviorFlagsInclude;
        if (!includedBehaviors) continue;

        boolean excludedBehaviors = (cTyp->flags & behaviorFlagsExclude) != 0;
        if (excludedBehaviors) continue;

        boolean includedAbilities = (cTyp->abilityFlags & abilityFlagsInclude) == abilityFlagsInclude;
        boolean excludedAbilities = (cTyp->abilityFlags & abilityFlagsExclude) != 0;
        if (!includedAbilities || excludedAbilities) continue;

        if (bolt == BOLT_NONE) {
            types[count++] = i;
        } else {
            // Check for the bolt
            for (int b = 0; b < 20; b++) {
                if (cTyp->bolts[b] == BOLT_NONE) break;
                if (cTyp->bolts[b] == bolt) types[count++] = i;
            }
        }
    }

    if (count == 0) return MK_YOU;  // Fall back to rogue if nothing else matches

    // Pick a matching monster type
    assureCosmeticRNG;
    enum monsterTypes monstType = types[rand_range(0, count-1)];
    // fprintf(stderr, "CHOSE: %s: %d\n", monsterCatalog[monstType].monsterName, monstType);
    return monstType;
}

// ---------------------------------------------
// Items
// ---------------------------------------------


void clearFloorItems(void) {
    item *i = floorItems->nextItem;
    while (i != NULL) {
        item *next = i->nextItem;
        if (isPosInMap(i->loc)) {
            pmapAt(i->loc)->flags &= ~HAS_ITEM;
        }
        deleteItem(i);
        i = next;
    }
    floorItems->nextItem = NULL;
}

void removeFloorFood(void) {
    for (item *i = floorItems->nextItem; i != NULL;) {
        item *next = i->nextItem;
        if (i->category == FOOD) {
            removeItemFromChain(i, floorItems);
            deleteItem(i);
        }
        i = next;
    }
}


item *findInventoryItem(short category, short kind) {
    for (item *i = packItems->nextItem; i; i = i->nextItem) {
        if (i->category == category && i->kind == kind) return i;
    }
    return NULL;
}

item *findFloorItem(short category, short kind) {
    for (item *i = floorItems->nextItem; i; i = i->nextItem) {
        if (i->category == category && i->kind == kind) return i;
    }
    return NULL;
}

item *findItem(short category, short kind) {
    item *i = findInventoryItem(category, kind);
    if (i != NULL) return i;
    return findFloorItem(category, kind);
}

item *givePlayerItem(short category, short kind, short enchant) {
    item *theItem = generateItem(category, kind);
    if (enchant) theItem->enchant1 = enchant;
    identify(theItem);
    return addItemToPack(theItem);
}

item *placeItemOnFloor(short category, short kind, pos loc) {
    item *theItem = generateItem(category, kind);
    return placeItemAt(theItem, loc);
}

pos itemPosition(const item *theItem) {
    if (theItem == NULL) return (pos) { -1, -1 };
    return theItem->loc;
}

// See the declaration in TestFramework.h.
int itemOriginDepth(const item *theItem) {
    return theItem != NULL ? theItem->originDepth : 0;
}

int countFloorItems(void) {
    int count = 0;
    for (item *i = floorItems->nextItem; i; i = i->nextItem) {
        if (!(i->category & GOLD)) count++;
    }
    return count;
}

item *floorItemAtIndex(int index) {
    for (item *i = floorItems->nextItem; i; i = i->nextItem) {
        if (i->category & GOLD) {
            continue;
        }
        if (index-- == 0) {
            return i;
        }
    }
    return NULL;
}

void equipItemFromPack(short category, short kind) {
    item *theItem = findInventoryItem(category, kind);
    if (theItem != NULL) {
        equipItem(theItem, true, NULL);
    }
}

void equipItemFromPackAsPlayer(short category, short kind) {
    item *theItem = findInventoryItem(category, kind);
    if (theItem != NULL) {
        equip(theItem);
    }
}

void unequipItemFromPack(short category, short kind) {
    item *theItem = findInventoryItem(category, kind);
    if (theItem != NULL) {
        unequipItem(theItem, true);
    }
}

// The bonuses updateRingBonuses() derives from the worn rings.
int playerClairvoyanceBonus(void)      { return rogue.clairvoyance; }
int playerStealthBonus(void)           { return rogue.stealthBonus; }
int playerAwarenessBonus(void)         { return rogue.awarenessBonus; }
int playerWisdomBonus(void)            { return rogue.wisdomBonus; }
int playerRegenerationBonus(void)      { return rogue.regenerationBonus; }
int playerTransferenceBonus(void)      { return rogue.transference; }
int playerLightMultiplier(void)        { return rogue.lightMultiplier; }
int playerReapingBonus(void)           { return rogue.reaping; }

void drink(enum potionKind kind) {
    // Apply the potion's effect without ending the turn, so that transient
    // results (such as a cloud of gas) can be observed.  A paralyzing cloud
    // would otherwise fully disperse during the turns that automatically pass
    // while you are paralyzed.
    item *potion = givePlayerItem(POTION, kind, 0);
    if (potion != NULL) {
        drinkPotion(potion);
    }
}

void readScrollOfKind(short kind) {
    item *scroll = givePlayerItem(SCROLL, kind, 0);
    apply(scroll);
}

void applyPackItem(short category, short kind) {
    item *theItem = findInventoryItem(category, kind);
    if (theItem != NULL) {
        apply(theItem);
    }
}

int itemQuantity(const item *theItem) {
    return theItem->quantity;
}

int itemCharges(const item *theItem) {
    return theItem->charges;
}

int itemEnchant(const item *theItem) {
    return theItem->enchant1;
}

char itemInventoryLetter(const item *theItem) {
    return theItem->inventoryLetter;
}

short itemKind(const item *theItem) {
    return theItem->kind;
}

short itemCategory(const item *theItem) {
    return theItem->category;
}

boolean itemIsCursed(const item *theItem) {
    return (theItem->flags & ITEM_CURSED) != 0;
}

boolean itemIsEquipped(const item *theItem) {
    return theItem != NULL && (theItem->flags & ITEM_EQUIPPED) != 0;
}

boolean itemIsIdentified(const item *theItem) {
    return (theItem->flags & ITEM_IDENTIFIED) != 0;
}

boolean itemMagicWasDetected(const item *theItem) {
    return (theItem->flags & ITEM_MAGIC_DETECTED) != 0;
}

int itemMagicPolarityValue(const item *theItem) {
    return itemMagicPolarity(theItem);
}

void curseItem(item *theItem) {
    if (theItem != NULL) {
        theItem->flags |= ITEM_CURSED;
    }
}

void uncurseItem(item *theItem) {
    if (theItem != NULL) {
        theItem->flags &= ~ITEM_CURSED;
    }
}

boolean itemIsRunic(const item *theItem) {
    return (theItem->flags & ITEM_RUNIC) != 0;
}

boolean runicIsIdentified(const item *theItem) {
    return itemIsRunic(theItem) && (theItem->flags & ITEM_RUNIC_IDENTIFIED) != 0;
}

boolean runicIsHinted(const item *theItem) {
    return itemIsRunic(theItem) && (theItem->flags & ITEM_RUNIC_HINTED) != 0;
}


int runicActivationChance(const item *theItem) {
    if (theItem->category == WEAPON) {
        return runicWeaponChance(theItem, true, theItem->enchant1 * FP_FACTOR);
    } else if (theItem->category == ARMOR) {
        return 0; // TODO
    }

    return 0;
}

void setItemTargetClass(item *theItem, short classId) {
    if (theItem == NULL) {
        return;
    }
    theItem->vorpalEnemy = classId;
}

item *equippedWeapon(void) {
    return rogue.weapon;
}

item *equippedArmor(void) {
    return rogue.armor;
}

int equippedRingCount(void) {
    return (rogue.ringLeft != NULL) + (rogue.ringRight != NULL);
}

unsigned long itemGoldValue(const item *theItem) {
    return itemValue((item *)theItem);
}

boolean itemKindIsKnown(short category, short kind) {
    itemTable *table = tableForItemCategory(category);
    if (table == NULL) {
        // No kind table means no unknown state: gold, the Amulet, lumenstones
        // and keys have no identity to learn, so the engine treats them as
        // known (it guards every knowledge check with the same null test).
        return true;
    }
    return table[kind].identified != 0;
}

static int matchingPackItems(short category, short kind, boolean sumUnits) {
    int total = 0;
    for (item *i = packItems->nextItem; i != NULL; i = i->nextItem) {
        if (i->category == category && i->kind == kind) {
            total += sumUnits ? i->quantity : 1;
        }
    }
    return total;
}

int packStackCount(short category, short kind) {
    return matchingPackItems(category, kind, false);
}

int packUnitCount(short category, short kind) {
    return matchingPackItems(category, kind, true);
}

int packItemCount(void) {
    return numberOfItemsInPack();
}

void fillPackToCapacity(void) {
    // Potions stack, so each added unit still counts toward the pack limit.
    while (numberOfItemsInPack() < MAX_PACK_ITEMS) {
        givePlayerItem(POTION, POTION_LIFE, 0);
    }
}

int playerGold(void) {
    return rogue.gold;
}

int playerStrength(void) {
    return rogue.strength;
}

int playerDefense(void) {
    return player.info.defense;
}

void wearArmorOfEnchant(short kind, int enchant) {
    item *theItem = generateItem(ARMOR, kind);
    theItem->enchant1 = enchant;
    theItem->enchant2 = 0;
    theItem->flags &= ~(ITEM_CURSED | ITEM_RUNIC | ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    identify(theItem);
    addItemToPack(theItem);
    equipItem(theItem, true, NULL);
}

item *givePlayerPlainWeapon(short kind, int enchant) {
    // generateItem() rolls a random enchant and possibly a runic (quietus or
    // slaying can kill a target outright) even when a specific kind is asked
    // for, and givePlayerItem() ignores an enchant of 0.
    item *theItem = generateItem(WEAPON, kind);
    theItem->enchant1 = enchant;
    theItem->enchant2 = 0;
    theItem->flags &= ~(ITEM_CURSED | ITEM_RUNIC | ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    identify(theItem);
    addItemToPack(theItem);
    return theItem;
}

item *givePlayerRunicWeapon(short kind, short runicKind, short enchant, boolean revealIdentity) {
    item *theItem = generateItem(WEAPON, kind);
    theItem->enchant1 = enchant;
    theItem->enchant2 = runicKind;
    theItem->flags |= ITEM_RUNIC;
    theItem->flags &= ~ITEM_CURSED;
    if (revealIdentity) identify(theItem);
    addItemToPack(theItem);
    return theItem;
}

item *givePlayerRunicArmor(short kind, short runicKind, short enchant, boolean revealIdentity) {
    item *theItem = generateItem(ARMOR, kind);
    theItem->enchant1 = enchant;
    theItem->enchant2 = runicKind;
    theItem->flags |= ITEM_RUNIC;
    theItem->flags &= ~ITEM_CURSED;
    if (revealIdentity) identify(theItem);
    addItemToPack(theItem);
    equipItem(theItem, true, NULL);
    return theItem;
}

boolean playerHasItem(short category, short kind) {
    return findInventoryItem(category, kind) != NULL;
}

void removePackFood(void) {
    // Starving players automatically eat any food in the pack, so a test of
    // starvation has to get rid of it first.
    for (item *i = packItems->nextItem; i != NULL;) {
        item *next = i->nextItem;
        if (i->category == FOOD) {
            removeItemFromChain(i, packItems);
            deleteItem(i);
        }
        i = next;
    }
}

void removePackItems(short category, short kind) {
    for (item *i = packItems->nextItem; i != NULL;) {
        item *next = i->nextItem;
        if (i->category == category && i->kind == kind) {
            if (rogue.weapon == i || rogue.armor == i) {
                if (rogue.weapon == i) rogue.weapon = NULL;
                if (rogue.armor == i) rogue.armor = NULL;
                recalculateEquipmentBonuses();
            }
            removeItemFromChain(i, packItems);
            deleteItem(i);
        }
        i = next;
    }
}

// ---------------------------------------------
// World
// ---------------------------------------------

unsigned long turnNumber(void) {
    return rogue.absoluteTurnNumber;
}

int ticksUntilPlayerActs(void) {
    return player.ticksUntilTurn;
}

int ticksUntilEnvironmentUpdate(void) {
    return rogue.ticksTillUpdateEnvironment;
}

int currentDepth(void) {
    return rogue.depthLevel;
}

void setDepth(int depth) {
    rogue.depthLevel = depth;
}

boolean isSameDepth(void) {
    return rogue.depthLevel == testStartDepth;
}

boolean playerIsCarryingAmulet(void) {
    return findInventoryItem(AMULET, 0) != NULL;
}

int deepestDepth(void) {
    return rogue.deepestLevel;
}

int totalGoldGenerated(void) {
    return rogue.goldGenerated;
}

int playerTurnCount(void) {
    return rogue.playerTurnNumber;
}

// ---------------------------------------------
// Dungeon
// ---------------------------------------------

int levelWidth(void) {
    return DCOLS;
}

int levelHeight(void) {
    return DROWS;
}

int countTerrain(short terrainType) {
    int count = 0;
    for (int i = 0; i < DCOLS; i++) {
        for (int j = 0; j < DROWS; j++) {
            if (pmap[i][j].layers[DUNGEON] == terrainType) {
                count++;
            }
        }
    }
    return count;
}

// The first room the generator carved on the level being built, taken before
// any other room was attached.  The depth it was recorded at is kept too, so a
// level restored from storage reports no first room rather than the room of
// whichever level was generated most recently.
static boolean firstRoomTiles[DCOLS][DROWS];
static int firstRoomRecordedDepth = -1;

void recordFirstRoomForTesting(short **grid) {
    for (int i = 0; i < DCOLS; i++) {
        for (int j = 0; j < DROWS; j++) {
            firstRoomTiles[i][j] = grid[i][j] != 0;
        }
    }
    firstRoomRecordedDepth = rogue.depthLevel;
}

boolean firstRoomCovers(pos loc) {
    if (!isPosInMap(loc) || firstRoomRecordedDepth != rogue.depthLevel) {
        return false;
    }
    return firstRoomTiles[loc.x][loc.y];
}

// ---------------------------------------------
// Vault layouts
// ---------------------------------------------

// The reward-vault blueprints, in catalog order.  A test sees only their
// positions in this list, never the catalog itself.
#define TEST_VAULT_LAYOUT_LIMIT 64
static short vaultLayoutIds[TEST_VAULT_LAYOUT_LIMIT];
static int vaultLayoutCountCache = -1;

static void ensureVaultLayouts(void) {
    if (vaultLayoutCountCache >= 0) {
        return;
    }
    int count = 0;
    for (int i = 1; i < gameConst->numberBlueprints && count < TEST_VAULT_LAYOUT_LIMIT; i++) {
        const unsigned long flags = blueprintCatalog[i].flags;
        // The same eligibility test the raffle applies when it is asked for a
        // reward vault: reward vaults are never adoptive or vestibule machines.
        if ((flags & BP_REWARD) && !(flags & (BP_ADOPT_ITEM | BP_VESTIBULE))) {
            vaultLayoutIds[count++] = i;
        }
    }
    vaultLayoutCountCache = count;
}

static boolean vaultLayoutIsValid(int layout) {
    ensureVaultLayouts();
    return layout >= 0 && layout < vaultLayoutCountCache;
}

int vaultLayoutCount(void) {
    ensureVaultLayouts();
    return vaultLayoutCountCache;
}

int vaultLayoutMinDepth(int layout) {
    if (!vaultLayoutIsValid(layout)) {
        return 0;
    }
    return blueprintCatalog[vaultLayoutIds[layout]].depthRange[0];
}

int vaultLayoutMaxDepth(int layout) {
    if (!vaultLayoutIsValid(layout)) {
        return 0;
    }
    return blueprintCatalog[vaultLayoutIds[layout]].depthRange[1];
}

int vaultLayoutFrequency(int layout) {
    if (!vaultLayoutIsValid(layout)) {
        return 0;
    }
    return blueprintCatalog[vaultLayoutIds[layout]].frequency;
}

int drawVaultLayout(int depth) {
    ensureVaultLayouts();

    const short savedDepth = rogue.depthLevel;
    rogue.depthLevel = depth;
    const short chosen = chooseBlueprintForTesting(BP_REWARD);
    rogue.depthLevel = savedDepth;

    for (int i = 0; i < vaultLayoutCountCache; i++) {
        if (vaultLayoutIds[i] == chosen) {
            return i;
        }
    }
    return -1;
}

boolean buildGuardedDoorVault(boolean childVaultsDisabled) {
    // Work on a private copy of the catalog so the fixture can choose which
    // child-vault kinds are attempted.  The vault in question guards its
    // doorway with a locked-door vestibule and hands that vestibule's key to
    // an adopting child vault.
    blueprint *tweakedBlueprints = malloc(gameConst->numberBlueprints * sizeof(blueprint));
    if (tweakedBlueprints == NULL) {
        testFail("could not allocate the tweaked blueprint table");
        return false;
    }
    memcpy(tweakedBlueprints, blueprintCatalog, gameConst->numberBlueprints * sizeof(blueprint));
    for (int i = 1; i < gameConst->numberBlueprints; i++) {
        if (tweakedBlueprints[i].flags & BP_VESTIBULE) {
            // With child vaults available, offer the ordinary locked-door
            // vestibule so the outcome is observable as a locked doorway.
            tweakedBlueprints[i].frequency =
                (!childVaultsDisabled && i == MT_LOCKED_DOOR_VESTIBULE) ? 100 : 0;
        }
    }

    const blueprint *originalBlueprints = blueprintCatalog;
    blueprintCatalog = tweakedBlueprints;
    boolean built = buildAMachine(MT_REWARD_MULTI_LIBRARY, -1, -1, 0, NULL, NULL, NULL);
    blueprintCatalog = originalBlueprints;
    free(tweakedBlueprints);
    return built;
}

// The room-like reward vault the fixture builds.  It has no interior-reshaping
// flags, so its interior is exactly the doorway plus the blocked-off region
// the doorway leads to.
#define TEST_ROOM_VAULT_BLUEPRINT MT_REWARD_DUNGEON

static boolean chamberFloorTiles[DCOLS][DROWS];
static boolean chamberDoorwayTiles[DCOLS][DROWS];
static int chamberFloorCells = 0;

int roomVaultRoomSizeMin(void) {
    return blueprintCatalog[TEST_ROOM_VAULT_BLUEPRINT].roomSize[0];
}

int roomVaultRoomSizeMax(void) {
    return blueprintCatalog[TEST_ROOM_VAULT_BLUEPRINT].roomSize[1];
}

int chamberFloorCellCount(void) {
    return chamberFloorCells;
}

boolean chamberFloorContains(pos loc) {
    if (!isPosInMap(loc)) {
        return false;
    }
    return chamberFloorTiles[loc.x][loc.y];
}

boolean chamberDoorwayAt(pos loc) {
    if (!isPosInMap(loc)) {
        return false;
    }
    return chamberDoorwayTiles[loc.x][loc.y];
}

static boolean chooseChamberShape(int area, int *width, int *height) {
    // Prefer a room-shaped rectangle, but fall back to a one-tile-tall room
    // so any area of at least four tiles can be tested.
    for (int w = 2; w <= 30; w++) {
        if (area % w != 0) {
            continue;
        }
        int h = area / w;
        if (h >= 1 && h <= 20) {
            *width = w;
            *height = h;
            return true;
        }
    }
    return false;
}

boolean buildRoomVaultAtChamber(int roomCells) {
    int width = 0, height = 0;

    memset(chamberFloorTiles, 0, sizeof(chamberFloorTiles));
    memset(chamberDoorwayTiles, 0, sizeof(chamberDoorwayTiles));
    chamberFloorCells = 0;

    if (!chooseChamberShape(roomCells, &width, &height)) {
        return false;
    }

    // Start from an open floor and wall off one dead-end chamber.  The rest of
    // the level is a single large region, so the chamber's doorway is the
    // level's only gate site and its blocked-off region is exactly the
    // chamber's floor.
    for (int x = 0; x < DCOLS; x++) {
        for (int y = 0; y < DROWS; y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }

    const int x0 = 40 - width / 2;
    const int y0 = 14 - height / 2;
    const int x1 = x0 + width - 1;
    const int y1 = y0 + height - 1;

    for (int x = x0 - 1; x <= x1 + 1; x++) {
        for (int y = y0 - 1; y <= y1 + 1; y++) {
            if (x < x0 || x > x1 || y < y0 || y > y1) {
                setTerrain(posAt(x, y), WALL);
            } else {
                chamberFloorTiles[x][y] = true;
                chamberFloorCells++;
            }
        }
    }

    const int doorX = x0 + width / 2;
    const int doorY = y1 + 1;
    setTerrain(posAt(doorX, doorY), DOOR);
    chamberDoorwayTiles[doorX][doorY] = true;

    setPlayerPos(posAt(1, 1));

    // Build the vault through the engine's own location chooser, so the
    // doorway's blocked-off region is rated and range-checked as usual.  Its
    // features are removed so the outcome depends only on doorway selection
    // and interior growth.
    blueprint *tweakedBlueprints = malloc(gameConst->numberBlueprints * sizeof(blueprint));
    if (tweakedBlueprints == NULL) {
        testFail("could not allocate the tweaked blueprint table");
        return false;
    }
    memcpy(tweakedBlueprints, blueprintCatalog, gameConst->numberBlueprints * sizeof(blueprint));
    tweakedBlueprints[TEST_ROOM_VAULT_BLUEPRINT].featureCount = 0;

    const blueprint *originalBlueprints = blueprintCatalog;
    blueprintCatalog = tweakedBlueprints;
    boolean built = buildAMachine(TEST_ROOM_VAULT_BLUEPRINT, -1, -1, 0, NULL, NULL, NULL);
    blueprintCatalog = originalBlueprints;
    free(tweakedBlueprints);

    return built;
}

// The engine's open-area vault (the amulet holder's statuary area).  It is
// neither a room vault nor a vestibule, so its interior grows along the
// pathing map from a random floor tile.
#define TEST_OPEN_AREA_VAULT_BLUEPRINT MT_AMULET_AREA

static pos openAreaVaultOriginRecorded = {0, 0};

void recordOpenAreaVaultOriginForTesting(short x, short y) {
    openAreaVaultOriginRecorded = posAt(x, y);
}

pos openAreaVaultOrigin(void) {
    return openAreaVaultOriginRecorded;
}

int openAreaVaultTargetMin(void) {
    return blueprintCatalog[TEST_OPEN_AREA_VAULT_BLUEPRINT].roomSize[0];
}

int openAreaVaultTargetMax(void) {
    return blueprintCatalog[TEST_OPEN_AREA_VAULT_BLUEPRINT].roomSize[1];
}

int openAreaVaultPathDistance(pos loc) {
    // A blocked tile is never part of the growth, and seeding a distance map
    // on one would report a spurious distance, so reject it up front.
    if (!isPosInMap(loc) || cellHasTerrainFlag(loc, T_PATHING_BLOCKER)) {
        return -1;
    }
    // The expansion walks a four-way pathing map (eightWays is false), so the
    // distance a test sees must be measured the same way.
    short **distanceMap = allocGrid();
    calculateDistances(distanceMap, loc.x, loc.y, T_PATHING_BLOCKER, NULL, true, false);
    int distance = distanceMap[openAreaVaultOriginRecorded.x][openAreaVaultOriginRecorded.y];
    freeGrid(distanceMap);
    return distance;
}

static boolean buildOpenAreaVaultInternal(int originX, int originY, int targetSize) {
    blueprint *tweakedBlueprints = malloc(gameConst->numberBlueprints * sizeof(blueprint));
    if (tweakedBlueprints == NULL) {
        testFail("could not allocate the tweaked blueprint table");
        return false;
    }
    memcpy(tweakedBlueprints, blueprintCatalog, gameConst->numberBlueprints * sizeof(blueprint));

    blueprint *vault = &tweakedBlueprints[TEST_OPEN_AREA_VAULT_BLUEPRINT];
    vault->featureCount = 0;
    if (targetSize > 0) {
        vault->roomSize[0] = vault->roomSize[1] = targetSize;
    }
    // The raw growth is what is under test.  Keep the interior purge (which
    // leaves the grown tiles as plain floor) but drop the passes that would
    // reshape the interior afterwards, so cellIsVaultInterior() reports
    // exactly the tiles expansion selected.
    vault->flags &= ~(BP_OPEN_INTERIOR | BP_MAXIMIZE_INTERIOR
                      | BP_REDESIGN_INTERIOR | BP_SURROUND_WITH_WALLS);

    const blueprint *originalBlueprints = blueprintCatalog;
    blueprintCatalog = tweakedBlueprints;
    boolean built = buildAMachine(TEST_OPEN_AREA_VAULT_BLUEPRINT, originX, originY,
                                  0, NULL, NULL, NULL);
    blueprintCatalog = originalBlueprints;
    free(tweakedBlueprints);

    return built;
}

boolean buildOpenAreaVault(void) {
    return buildOpenAreaVaultInternal(-1, -1, 0);
}

boolean buildOpenAreaVaultAt(pos origin, int targetSize) {
    return buildOpenAreaVaultInternal(origin.x, origin.y, targetSize);
}

int lightAt(pos loc) {
    return tmapAt(loc)->light[0] + tmapAt(loc)->light[1] + tmapAt(loc)->light[2];
}

void lightColorAt(pos loc, int *red, int *green, int *blue) {
    *red = tmapAt(loc)->light[0];
    *green = tmapAt(loc)->light[1];
    *blue = tmapAt(loc)->light[2];
}

void descendToNextLevel(void) {
    rogue.depthLevel++;
    startLevel(rogue.depthLevel - 1, 1);
    if (rogue.depthLevel > rogue.deepestLevel) {
        rogue.deepestLevel = rogue.depthLevel;
    }
}

void ascendToPreviousLevel(void) {
    useStairs(-1);
}

void descendWithFailedAmuletAltar(void) {
    // Copy the blueprint table and rig the amulet altar machine so that its
    // first feature can never be placed in the required number of instances;
    // every build attempt during generation of the amulet floor then fails,
    // which is the precondition for the engine's fallback amulet placement.
    blueprint *tweakedBlueprints = malloc(gameConst->numberBlueprints * sizeof(blueprint));
    if (tweakedBlueprints == NULL) {
        testFail("could not allocate the tweaked blueprint table");
        return;
    }
    memcpy(tweakedBlueprints, blueprintCatalog, gameConst->numberBlueprints * sizeof(blueprint));
    tweakedBlueprints[MT_AMULET_AREA].feature[0].minimumInstanceCount = 99;

    const blueprint *originalBlueprints = blueprintCatalog;
    blueprintCatalog = tweakedBlueprints;
    setDepth(gameConst->amuletLevel - 1);
    descendToNextLevel();
    blueprintCatalog = originalBlueprints;
    free(tweakedBlueprints);
}

int pathingDistanceBetween(pos from, pos to, unsigned long blockingTerrainFlags) {
    return pathingDistance(from.x, from.y, to.x, to.y, blockingTerrainFlags);
}

int playerPathingDistance(pos from, pos to) {
    short **distanceMap = allocGrid();
    calculateDistances(distanceMap, to.x, to.y, 0, &player, true, true);
    int distance = distanceMap[from.x][from.y];
    freeGrid(distanceMap);
    return distance;
}

int minersLightRadius(void) {
    updateMinersLightRadius();
    return rogue.minersLight.lightRadius.lowerBound;
}

void setLightMultiplier(int multiplier) {
    rogue.lightMultiplier = multiplier;
}

void setPlayerInWater(boolean inWater) {
    rogue.inWater = inWater;
}

void recomputeLighting(void) {
    updateMinersLightRadius();
    updateLighting();
}

const char *playerDrawColorName(void) {
    const color *drawColor = player.info.foreColor;
    if (drawColor == &playerInvisibleColor) return "invisible";
    if (drawColor == &playerInDarknessColor) return "darkness";
    if (drawColor == &playerInShadowColor) return "shadow";
    if (drawColor == &playerInLightColor) return "light";
    return "unknown";
}

unsigned long playerDrawColorId(void) {
    const color *drawColor = player.info.foreColor;
    return ((unsigned long)(unsigned char) drawColor->red << 16)
         | ((unsigned long)(unsigned char) drawColor->green << 8)
         | (unsigned long)(unsigned char) drawColor->blue;
}

void updatePlayerDrawColor(void) {
    updatePlayerForeColor();
}

// Mirrors cellIsPassableOrDoor() in Architect.c, which the generator
// flood-fills with when it decides whether a placement would strand part of
// the level (that helper is static there).
static boolean countsAsConnected(pos loc) {
    if (!cellHasTerrainFlag(loc, T_PATHING_BLOCKER)) {
        return true;
    }
    return (cellHasTMFlag(loc, (TM_IS_SECRET | TM_PROMOTES_WITH_KEY | TM_CONNECTS_LEVEL))
            && cellHasTerrainFlag(loc, T_OBSTRUCTS_PASSABILITY));
}

int countUnreachableFloorFrom(pos from) {
    static const int dirs[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    boolean visited[DCOLS][DROWS];
    pos queue[DCOLS * DROWS];
    int head = 0, tail = 0;
    int unreachable = 0;

    memset(visited, 0, sizeof(visited));
    if (from.x < 0 || from.y < 0 || from.x >= DCOLS || from.y >= DROWS) {
        return 0;
    }
    visited[from.x][from.y] = true;
    queue[tail++] = from;

    while (head < tail) {
        pos here = queue[head++];
        for (int d = 0; d < 4; d++) {
            pos next = posOffset(here, dirs[d][0], dirs[d][1]);
            if (next.x < 0 || next.y < 0 || next.x >= DCOLS || next.y >= DROWS) {
                continue;
            }
            if (visited[next.x][next.y] || !countsAsConnected(next)) {
                continue;
            }
            visited[next.x][next.y] = true;
            queue[tail++] = next;
        }
    }

    // Dry floor is what has to stay connected; floor drowned in deep water or
    // split by a chasm is not part of the walkable region to begin with.
    for (int x = 0; x < DCOLS; x++) {
        for (int y = 0; y < DROWS; y++) {
            pos cell = posAt(x, y);
            if (terrainAt(cell) == FLOOR && countsAsConnected(cell) && !visited[x][y]) {
                unreachable++;
            }
        }
    }
    return unreachable;
}

short vorpalEnemyClassByName(const char *className) {
    for (short i = 0; i < MONSTER_CLASS_COUNT; i++) {
        if (strcmp(monsterClassCatalog[i].name, className) != 0) {
            continue;
        }
        return i;
    }

    // Not found, fail the test
    testFail("Unknown monster class name");
    return 0;
}

short vorpalEnemyClassForMonster(enum monsterTypes monstType) {
    for (short classID = 0; classID < MONSTER_CLASS_COUNT; classID++) {
        for (short i = 0; monsterClassCatalog[classID].memberList[i] != 0; i++) {
            if (monsterClassCatalog[classID].memberList[i] != monstType) {
                continue;
            }
            return classID;
        }
    }

    // Not found, fail the test
    testFail("Monster not found in any monster class");
    return 0;
}


// ---------------------------------------------
// TEST SETUP HELPERS
// ---------------------------------------------

void setArenaRadius(int radius, enum tileType tile) {
    // Clear a wide area so knockback and clone destinations are open floor.
    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
            pos loc = posOffset(player.loc, dx, dy);
            if (loc.x < 0 || loc.x >= levelWidth() || loc.y < 0 || loc.y >= levelHeight()) {
                continue; // never write outside the level
            }
            setTerrain(loc, tile);
        }
    }
    setPlayerPos(player.loc);
    clearMonsters();
    removeFloorFood();
}

void clearArena(void) {
    setArenaRadius(2,FLOOR);
}

// Give a runic weapon AND equip it as rogue.weapon (unlike givePlayerRunicWeapon
// which only adds it to the pack).  Returns the item for convenience.
item *giveAndEquipRunicWeapon(short kind, short runicKind, short enchant, boolean revealIdentity) {
    item *w = givePlayerRunicWeapon(kind, runicKind, enchant, revealIdentity);
    equip(w);

    // // Match player strength to weapon so there is no negative
    rogue.strength = w->strengthRequired;
    recalculateEquipmentBonuses();
    // player.info.accuracy = 100;
    return w;
}


void populateLevelItemsFrom(pos upstairs) {
    populateItems(upstairs);
}

creature *placeHostileMonster(enum monsterTypes type, pos loc) {
    creature *monst = spawnMonster(type, loc);
    makeMonsterHostile(monst);
    revealCell(loc);
    return monst;
}

// See the declaration in TestFramework.h.
creature *spawnMonsterWithItemChance(enum monsterTypes type, pos loc) {
    creature *monst = generateMonster(type, true, false);
    setMonsterPos(monst, loc);
    return monst;
}

// See the declaration in TestFramework.h.
int monsterItemHopperCount(void) {
    int count = 0;
    for (item *i = monsterItemsHopper->nextItem; i != NULL; i = i->nextItem) {
        count++;
    }
    return count;
}

void addDeadAllyToPurgatory(enum monsterTypes type, pos loc) {
    creature *m = spawnMonster(type, loc);
    makeMonsterAlly(m);
    m->leader = &player;
    damageMonster(m, monsterMaxHealth(m) + 100); // reduce HP to 0
    killCreature(m, false);                      // mark dead (MB_HAS_DIED), not administrative
    // Move the dead ally from `monsters` into `purgatory`, where resurrection
    // can find it.
    removeDeadMonsters();
}

boolean resurrectDeadAllyAt(pos loc) {
    return resurrectAlly(loc);
}


// ---------------------------------------------
// TEST STEP HELPERS
// ---------------------------------------------
static int test_turn_number = -1;

boolean wait_until_fall_step(void) {
    if (isSameDepth() && turnNumber() < 1000) {
        restOneTurn();
        return true; // keep going
    }
    return false; // End test
}

boolean wait_until_hurt(void) {
    if (isPlayerHealthy() && turnNumber() < 1000) {
        restOneTurn();
        return true; // keep waiting
    }
    return false; // End test
}


boolean trap_step(void) {
    if (test_turn_number == -1) {
        test_turn_number = turnNumber();
    }

    // Move onto trap, then wait 
    if (test_turn_number == turnNumber()) {
        moveRight();
        return true; // Continue
    }

    // Wait one turn then end the test
    restOneTurn();
    test_turn_number = -1;
    return false; // End test
}

boolean wait_10_turns_step(void) {
    if (test_turn_number == -1) {
        test_turn_number = turnNumber();
    }

    if (turnNumber() - test_turn_number < 10) {
        restOneTurn();
        return true; // continue waiting
    }

    test_turn_number = -1;
    return false; // end test
}


boolean wait_100_turns_step(void) {
    if (test_turn_number == -1) {
        test_turn_number = turnNumber();
    }

    if (turnNumber() - test_turn_number < 100) {
        restOneTurn();
        return true; // continue waiting
    }

    test_turn_number = -1;
    return false; // end test
}

boolean move_right_step(void) {
    moveRight();
    return false;
}


// ---------------------------------------------
// TEST VERIFY HELPERS
// ---------------------------------------------

boolean player_is_hurt_verify(void) {
    TEST_ASSERT(isPlayerHurt(), "player should be hurt");
    return !testFailed;
}
