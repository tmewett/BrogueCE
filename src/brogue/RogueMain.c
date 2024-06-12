/*
 *  RogueMain.c
 *  Brogue
 *
 *  Created by Brian Walker on 12/26/08.
 *  Copyright 2012. All rights reserved.
 *
 *  This file is part of Brogue.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "GlobalsBrogue.h"
#include "GlobalsRapidBrogue.h"

#include <time.h>

int rogueMain() {
    previousGameSeed = 0;
    mainBrogueJunction();
    return rogue.gameExitStatusCode;
}

void printBrogueVersion() {
    printf("Brogue version: %s\n", brogueVersion);
    printf("Supports variant (rapid_brogue): %s\n", rapidBrogueVersion);
}

void executeEvent(rogueEvent *theEvent) {
    rogue.playbackBetweenTurns = false;
    if (theEvent->eventType == KEYSTROKE) {
        executeKeystroke(theEvent->param1, theEvent->controlKey, theEvent->shiftKey);
    } else if (theEvent->eventType == MOUSE_UP
               || theEvent->eventType == RIGHT_MOUSE_UP) {
        executeMouseClick(theEvent);
    }
}

boolean fileExists(const char *pathname) {
    FILE *openedFile;
    openedFile = fopen(pathname, "rb");
    if (openedFile) {
        fclose(openedFile);
        return true;
    } else {
        return false;
    }
}

// Player specifies a file; if all goes well, put it into path and return true.
// Otherwise, return false.
boolean chooseFile(char *path, char *prompt, char *defaultName, char *suffix) {

    if (getInputTextString(path,
                           prompt,
                           min(DCOLS-25, BROGUE_FILENAME_MAX - strlen(suffix)),
                           defaultName,
                           suffix,
                           TEXT_INPUT_FILENAME,
                           false)
        && path[0] != '\0') {

        strcat(path, suffix);
        return true;
    } else {
        return false;
    }
}

// If the file exists, copy it into currentFilePath. (Otherwise return false.)
// Then, strip off the suffix, replace it with ANNOTATION_SUFFIX,
// and if that file exists, copy that into annotationPathname. Return true.
boolean openFile(const char *path) {
    short i;
    char buf[BROGUE_FILENAME_MAX];
    boolean retval;

    if (fileExists(path)) {

        strcpy(currentFilePath, path);
        annotationPathname[0] = '\0';

        // Clip off the suffix.
        strcpy(buf, path);
        for (i = strlen(path); buf[i] != '.' && i > 0; i--) continue;
        if (buf[i] == '.'
            && i + strlen(ANNOTATION_SUFFIX) < BROGUE_FILENAME_MAX) {

            buf[i] = '\0'; // Snip!
            strcat(buf, ANNOTATION_SUFFIX);
            strcpy(annotationPathname, buf); // Load the annotations file too.
        }
        retval = true;
    } else {
        retval = false;
    }

    return retval;
}

#ifdef SCREEN_UPDATE_BENCHMARK
static void screen_update_benchmark() {
    short i, j, k;
    const color sparklesauce = {10, 0, 20,  60, 40, 100, 30, true};
    enum displayGlyph theChar;

    unsigned long initialTime = (unsigned long) time(NULL);
    for (k=0; k<500; k++) {
        for (i=0; i<COLS; i++) {
            for (j=0; j<ROWS; j++) {
                theChar = rand_range('!', '~');
                plotCharWithColor(theChar, (windowpos){ i, j }, &sparklesauce, &sparklesauce);
            }
        }
        pauseBrogue(1, PAUSE_BEHAVIOR_DEFAULT);
    }
    printf("\n\nBenchmark took a total of %lu seconds.\n", ((unsigned long) time(NULL)) - initialTime);
}
#endif

static const char *getOrdinalSuffix(int number) {
    // Handle special cases for 11, 12, and 13
    if (number == 11 || number == 12 || number == 13) {
        return "th";
    }

    // Determine the suffix based on the last digit
    int lastDigit = number % 10;
    switch (lastDigit) {
        case 1:
            return "st";
        case 2:
            return "nd";
        case 3:
            return "rd";
        default:
            return "th";
    }
}

static void welcome() {
    char buf[DCOLS*3], buf2[DCOLS*3];
    message("Hello and welcome, adventurer, to the Dungeons of Doom!", 0);
    strcpy(buf, "Retrieve the ");
    encodeMessageColor(buf, strlen(buf), &itemMessageColor);
    strcat(buf, "Amulet of Yendor");
    encodeMessageColor(buf, strlen(buf), &white);
    sprintf(buf2, " from the %i%s floor and escape with it!", gameConst->amuletLevel, getOrdinalSuffix(gameConst->amuletLevel));
    strcat(buf, buf2);
    message(buf, 0);
    if (KEYBOARD_LABELS) {
        messageWithColor("Press <?> for help at any time.", &backgroundMessageColor, 0);
    }
    flavorMessage("The doors to the dungeon slam shut behind you.");
}

void initializeGameVariant() {

    switch (gameVariant) {
        case VARIANT_RAPID_BROGUE:
            initializeGameVariantRapidBrogue();
            break;
        default:
            initializeGameVariantBrogue();
    }
}

// Seed is used as the dungeon seed unless it's zero, in which case generate a new one.
// Either way, previousGameSeed is set to the seed we use.
// None of this seed stuff is applicable if we're playing a recording.
void initializeRogue(uint64_t seed) {
    short i, j, k;
    item *theItem;
    boolean playingback, playbackFF, playbackPaused, wizard, easy, displayStealthRangeMode;
    boolean trueColorMode;
    short oldRNG;
    char currentGamePath[BROGUE_FILENAME_MAX];

    playingback = rogue.playbackMode; // the only animals that need to go on the ark
    playbackPaused = rogue.playbackPaused;
    playbackFF = rogue.playbackFastForward;
    wizard = rogue.wizard;
    easy = rogue.easyMode;
    displayStealthRangeMode = rogue.displayStealthRangeMode;
    trueColorMode = rogue.trueColorMode;

    strcpy(currentGamePath, rogue.currentGamePath);

    if (rogue.meteredItems != NULL) {
        free(rogue.meteredItems);
    }

    memset((void *) &rogue, 0, sizeof( playerCharacter )); // the flood
    rogue.playbackMode = playingback;
    rogue.playbackPaused = playbackPaused;
    rogue.playbackFastForward = playbackFF;
    rogue.wizard = wizard;
    rogue.easyMode = easy;
    rogue.displayStealthRangeMode = displayStealthRangeMode;
    rogue.trueColorMode = trueColorMode;

    rogue.gameHasEnded = false;
    rogue.gameInProgress = true;
    rogue.highScoreSaved = false;
    rogue.cautiousMode = false;
    rogue.milliseconds = 0;

    rogue.meteredItems = calloc(gameConst->numberMeteredItems, sizeof(meteredItem));
    rogue.featRecord = calloc(gameConst->numberFeats, sizeof(boolean));
    strcpy(rogue.currentGamePath, currentGamePath);

    rogue.RNG = RNG_SUBSTANTIVE;
    if (!rogue.playbackMode) {
        rogue.seed = seedRandomGenerator(seed);
        previousGameSeed = rogue.seed;
    }

#ifdef SCREEN_UPDATE_BENCHMARK
    screen_update_benchmark();
#endif

    initRecording();

    levels = malloc(sizeof(levelData) * (gameConst->deepestLevel+1));
    levels[0].upStairsLoc.x = (DCOLS - 1) / 2 - 1;
    levels[0].upStairsLoc.y = DROWS - 2;

    // Set metered item frequencies to initial values.
    for (i = 0; i < gameConst->numberMeteredItems; i++) {
        rogue.meteredItems[i].frequency = meteredItemsGenerationTable[i].initialFrequency;
    }

    // all DF messages are eligible for display
    resetDFMessageEligibility();

    // initialize the levels list
    for (i=0; i<gameConst->deepestLevel+1; i++) {
        if (rogue.seed >> 32) {
            // generate a 64-bit seed
            levels[i].levelSeed = rand_64bits();
        } else {
            // backward-compatible seed
            levels[i].levelSeed = (unsigned long) rand_range(0, 9999);
            levels[i].levelSeed += (unsigned long) 10000 * rand_range(0, 9999);
        }
        if (levels[i].levelSeed == 0) { // seed 0 is not acceptable
            levels[i].levelSeed = i + 1;
        }
        levels[i].monsters = createCreatureList();;
        levels[i].dormantMonsters = createCreatureList();;
        levels[i].items = NULL;
        levels[i].scentMap = NULL;
        levels[i].visited = false;
        levels[i].playerExitedVia = (pos){ .x = 0, .y = 0 };
        do {
            levels[i].downStairsLoc.x = rand_range(1, DCOLS - 2);
            levels[i].downStairsLoc.y = rand_range(1, DROWS - 2);
        } while (distanceBetween(levels[i].upStairsLoc, levels[i].downStairsLoc) < DCOLS / 3);
        if (i < gameConst->deepestLevel) {
            levels[i+1].upStairsLoc.x = levels[i].downStairsLoc.x;
            levels[i+1].upStairsLoc.y = levels[i].downStairsLoc.y;
        }
    }

    // initialize the waypoints list
    for (i=0; i<MAX_WAYPOINT_COUNT; i++) {
        rogue.wpDistance[i] = allocGrid();
        fillGrid(rogue.wpDistance[i], 0);
    }

    rogue.rewardRoomsGenerated = 0;

    // pre-shuffle the random terrain colors
    oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;
    //assureCosmeticRNG;
    for (i=0; i<DCOLS; i++) {
        for( j=0; j<DROWS; j++ ) {
            for (k=0; k<8; k++) {
                terrainRandomValues[i][j][k] = rand_range(0, 1000);
            }
        }
    }
    restoreRNG;

    zeroOutGrid(displayDetail);

    for (i=0; i<NUMBER_MONSTER_KINDS; i++) {
        monsterCatalog[i].monsterID = i;
    }

    shuffleFlavors();

    for (i = 0; i < gameConst->numberFeats; i++) {
        rogue.featRecord[i] = featTable[i].initialValue;
    }

    deleteMessages();
    for (i = 0; i < MESSAGE_ARCHIVE_ENTRIES; i++) { // Clear the message archive.
        messageArchive[i].message[0] = '\0';
    }
    messageArchivePosition = 0;

    // Seed the stacks.
    floorItems = (item *) malloc(sizeof(item));
    memset(floorItems, '\0', sizeof(item));
    floorItems->nextItem = NULL;

    packItems = (item *) malloc(sizeof(item));
    memset(packItems, '\0', sizeof(item));
    packItems->nextItem = NULL;

    monsterItemsHopper = (item *) malloc(sizeof(item));
    memset(monsterItemsHopper, '\0', sizeof(item));
    monsterItemsHopper->nextItem = NULL;

    for (i = 0; i < MAX_ITEMS_IN_MONSTER_ITEMS_HOPPER; i++) {
        theItem = generateItem(ALL_ITEMS & ~FOOD, -1); // Monsters can't carry food: the food clock cannot be cheated!
        theItem->nextItem = monsterItemsHopper->nextItem;
        monsterItemsHopper->nextItem = theItem;
    }

    monsters = &levels[0].monsters;
    dormantMonsters = &levels[0].dormantMonsters;
    purgatory = createCreatureList();

    scentMap            = NULL;
    safetyMap           = allocGrid();
    allySafetyMap       = allocGrid();
    chokeMap            = allocGrid();

    rogue.mapToSafeTerrain = allocGrid();

    // Zero out the dynamic grids, as an essential safeguard against OOSes:
    fillGrid(safetyMap, 0);
    fillGrid(allySafetyMap, 0);
    fillGrid(chokeMap, 0);
    fillGrid(rogue.mapToSafeTerrain, 0);

    // initialize the player

    memset(&player, '\0', sizeof(creature));
    player.info = monsterCatalog[0];
    setPlayerDisplayChar();
    initializeGender(&player);
    player.movementSpeed = player.info.movementSpeed;
    player.attackSpeed = player.info.attackSpeed;
    initializeStatus(&player);
    player.carriedItem = NULL;
    player.currentHP = player.info.maxHP;
    player.creatureState = MONSTER_ALLY;
    player.ticksUntilTurn = 0;
    player.mutationIndex = -1;

    rogue.depthLevel = 1;
    rogue.deepestLevel = 1;
    rogue.scentTurnNumber = 1000;
    rogue.playerTurnNumber = 0;
    rogue.absoluteTurnNumber = 0;
    rogue.previousPoisonPercent = 0;
    rogue.foodSpawned = 0;
    rogue.gold = 0;
    rogue.goldGenerated = 0;
    rogue.disturbed = false;
    rogue.autoPlayingLevel = false;
    rogue.automationActive = false;
    rogue.justRested = false;
    rogue.justSearched = false;
    rogue.inWater = false;
    rogue.creaturesWillFlashThisTurn = false;
    rogue.updatedSafetyMapThisTurn = false;
    rogue.updatedAllySafetyMapThisTurn = false;
    rogue.updatedMapToSafeTerrainThisTurn = false;
    rogue.updatedMapToShoreThisTurn = false;
    rogue.strength = 12;
    rogue.weapon = NULL;
    rogue.armor = NULL;
    rogue.ringLeft = NULL;
    rogue.ringRight = NULL;
    rogue.swappedIn = NULL;
    rogue.swappedOut = NULL;
    rogue.monsterSpawnFuse = rand_range(125, 175);
    rogue.ticksTillUpdateEnvironment = 100;
    rogue.mapToShore = NULL;
    rogue.cursorLoc = INVALID_POS;
    rogue.xpxpThisTurn = 0;

    rogue.yendorWarden = NULL;

    rogue.flares = NULL;
    rogue.flareCount = rogue.flareCapacity = 0;

    rogue.minersLight = lightCatalog[MINERS_LIGHT];

    rogue.clairvoyance = rogue.regenerationBonus
    = rogue.stealthBonus = rogue.transference = rogue.wisdomBonus = rogue.reaping = 0;
    rogue.lightMultiplier = 1;

    theItem = generateItem(FOOD, RATION);
    theItem = addItemToPack(theItem);

    theItem = generateItem(WEAPON, DAGGER);
    theItem->enchant1 = theItem->enchant2 = 0;
    theItem->flags &= ~(ITEM_CURSED | ITEM_RUNIC);
    identify(theItem);
    theItem = addItemToPack(theItem);
    equipItem(theItem, false, NULL);

    theItem = generateItem(WEAPON, DART);
    theItem->enchant1 = theItem->enchant2 = 0;
    theItem->quantity = 15;
    theItem->flags &= ~(ITEM_CURSED | ITEM_RUNIC);
    identify(theItem);
    theItem = addItemToPack(theItem);

    theItem = generateItem(ARMOR, LEATHER_ARMOR);
    theItem->enchant1 = 0;
    theItem->flags &= ~(ITEM_CURSED | ITEM_RUNIC);
    identify(theItem);
    theItem = addItemToPack(theItem);
    equipItem(theItem, false, NULL);
    player.status[STATUS_DONNING] = 0;

    recalculateEquipmentBonuses();

    if (D_OMNISCENCE) {
        rogue.playbackOmniscience = 1;
    }

    DEBUG {
        theItem = generateItem(RING, RING_CLAIRVOYANCE);
        theItem->enchant1 = max(DROWS, DCOLS);
        theItem->flags &= ~ITEM_CURSED;
        identify(theItem);
        theItem = addItemToPack(theItem);

        theItem = generateItem(RING, RING_AWARENESS);
        theItem->enchant1 = 30;
        theItem->flags &= ~ITEM_CURSED;
        identify(theItem);
        theItem = addItemToPack(theItem);

        theItem = generateItem(WEAPON, DAGGER);
        theItem->enchant1 = 50;
        theItem->enchant2 = W_QUIETUS;
        theItem->flags &= ~(ITEM_CURSED);
        theItem->flags |= (ITEM_PROTECTED | ITEM_RUNIC | ITEM_RUNIC_HINTED);
        theItem->damage.lowerBound = theItem->damage.upperBound = 25;
        identify(theItem);
        theItem = addItemToPack(theItem);

        theItem = generateItem(ARMOR, LEATHER_ARMOR);
        theItem->enchant1 = 50;
        theItem->enchant2 = A_REFLECTION;
        theItem->flags &= ~(ITEM_CURSED | ITEM_RUNIC_HINTED);
        theItem->flags |= (ITEM_PROTECTED | ITEM_RUNIC);
        identify(theItem);
        theItem = addItemToPack(theItem);

        theItem = generateItem(STAFF, STAFF_FIRE);
        theItem->enchant1 = 10;
        theItem->charges = 300;
        identify(theItem);
        theItem = addItemToPack(theItem);

        theItem = generateItem(STAFF, STAFF_LIGHTNING);
        theItem->enchant1 = 10;
        theItem->charges = 300;
        identify(theItem);
        theItem = addItemToPack(theItem);

        theItem = generateItem(STAFF, STAFF_TUNNELING);
        theItem->enchant1 = 10;
        theItem->charges = 3000;
        identify(theItem);
        theItem = addItemToPack(theItem);

        theItem = generateItem(STAFF, STAFF_OBSTRUCTION);
        theItem->enchant1 = 10;
        theItem->charges = 300;
        identify(theItem);
        theItem = addItemToPack(theItem);

        theItem = generateItem(STAFF, STAFF_ENTRANCEMENT);
        theItem->enchant1 = 10;
        theItem->charges = 300;
        identify(theItem);
        theItem = addItemToPack(theItem);

        theItem = generateItem(WAND, WAND_BECKONING);
        theItem->charges = 3000;
        identify(theItem);
        theItem = addItemToPack(theItem);

        theItem = generateItem(WAND, WAND_DOMINATION);
        theItem->charges = 300;
        identify(theItem);
        theItem = addItemToPack(theItem);

        theItem = generateItem(WAND, WAND_PLENTY);
        theItem->charges = 300;
        identify(theItem);
        theItem = addItemToPack(theItem);

        theItem = generateItem(WAND, WAND_NEGATION);
        theItem->charges = 300;
        identify(theItem);
        theItem = addItemToPack(theItem);

    }
    clearMessageArchive();
    blackOutScreen();
    welcome();
}

// call this once per level to set all the dynamic colors as a function of depth
static void updateColors() {
    short i;

    for (i=0; i<NUMBER_DYNAMIC_COLORS; i++) {
        *(dynamicColors[i]) = *(dynamicColorsBounds[i][0]);
        applyColorAverage(dynamicColors[i], dynamicColorsBounds[i][1], min(100, max(0, rogue.depthLevel * 100 / gameConst->amuletLevel)));
    }
}

void startLevel(short oldLevelNumber, short stairDirection) {
    uint64_t oldSeed;
    item *theItem;
    short i, j, x, y, px, py, flying, dir;
    boolean placedPlayer;
    enum dungeonLayers layer;
    unsigned long timeAway;
    short **mapToStairs;
    short **mapToPit;
    boolean connectingStairsDiscovered;

    if (oldLevelNumber == gameConst->deepestLevel && stairDirection != -1) {
        return;
    }

    synchronizePlayerTimeState();

    rogue.updatedSafetyMapThisTurn          = false;
    rogue.updatedAllySafetyMapThisTurn      = false;
    rogue.updatedMapToSafeTerrainThisTurn   = false;

    rogue.cursorLoc = INVALID_POS;
    rogue.lastTarget = NULL;

    connectingStairsDiscovered = (pmapAt(rogue.downLoc)->flags & (DISCOVERED | MAGIC_MAPPED) ? true : false);
    if (stairDirection == 0) { // fallen
        levels[oldLevelNumber-1].playerExitedVia = (pos){ .x = player.loc.x, .y = player.loc.y };
    }

    if (oldLevelNumber != rogue.depthLevel) {
        px = player.loc.x;
        py = player.loc.y;
        if (cellHasTerrainFlag(player.loc, T_AUTO_DESCENT)) {
            for (i=0; i<8; i++) {
                if (!cellHasTerrainFlag((pos){ player.loc.x+nbDirs[i][0], player.loc.y+nbDirs[i][1] }, (T_PATHING_BLOCKER))) {
                    px = player.loc.x+nbDirs[i][0];
                    py = player.loc.y+nbDirs[i][1];
                    break;
                }
            }
        }
        mapToStairs = allocGrid();
        fillGrid(mapToStairs, 0);
        for (flying = 0; flying <= 1; flying++) {
            fillGrid(mapToStairs, 0);
            calculateDistances(mapToStairs, px, py, (flying ? T_OBSTRUCTS_PASSABILITY : T_PATHING_BLOCKER) | T_SACRED, NULL, true, true);
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *monst = nextCreature(&it);
                x = monst->loc.x;
                y = monst->loc.y;
                if (((monst->creatureState == MONSTER_TRACKING_SCENT && (stairDirection != 0 || monst->status[STATUS_LEVITATING]))
                     || monst->creatureState == MONSTER_ALLY || monst == rogue.yendorWarden)
                    && (stairDirection != 0 || monst->currentHP > 10 || monst->status[STATUS_LEVITATING])
                    && ((flying != 0) == ((monst->status[STATUS_LEVITATING] != 0)
                                          || cellHasTerrainFlag((pos){ x, y }, T_PATHING_BLOCKER)
                                          || cellHasTerrainFlag((pos){ px, py }, T_AUTO_DESCENT)))
                    && !(monst->bookkeepingFlags & MB_CAPTIVE)
                    && !(monst->info.flags & (MONST_WILL_NOT_USE_STAIRS | MONST_RESTRICTED_TO_LIQUID))
                    && !(cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY))
                    && !monst->status[STATUS_ENTRANCED]
                    && !monst->status[STATUS_PARALYZED]
                    && (mapToStairs[monst->loc.x][monst->loc.y] < 30000 || monst->creatureState == MONSTER_ALLY || monst == rogue.yendorWarden)) {

                    monst->status[STATUS_ENTERS_LEVEL_IN] = clamp(mapToStairs[monst->loc.x][monst->loc.y] * monst->movementSpeed / 100 + 1, 1, 150);
                    switch (stairDirection) {
                        case 1:
                            monst->bookkeepingFlags |= MB_APPROACHING_DOWNSTAIRS;
                            break;
                        case -1:
                            monst->bookkeepingFlags |= MB_APPROACHING_UPSTAIRS;
                            break;
                        case 0:
                            monst->bookkeepingFlags |= MB_APPROACHING_PIT;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        freeGrid(mapToStairs);
    }

    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (monst->mapToMe) {
            freeGrid(monst->mapToMe);
            monst->mapToMe = NULL;
        }
    }
    levels[oldLevelNumber-1].items = floorItems->nextItem;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (pmap[i][j].flags & ANY_KIND_OF_VISIBLE) {
                // Remember visible cells upon exiting.
                storeMemories(i, j);
            }
            for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                levels[oldLevelNumber - 1].mapStorage[i][j].layers[layer] = pmap[i][j].layers[layer];
            }
            levels[oldLevelNumber - 1].mapStorage[i][j].volume = pmap[i][j].volume;
            levels[oldLevelNumber - 1].mapStorage[i][j].flags = (pmap[i][j].flags & PERMANENT_TILE_FLAGS);
            levels[oldLevelNumber - 1].mapStorage[i][j].machineNumber = pmap[i][j].machineNumber;
            levels[oldLevelNumber - 1].mapStorage[i][j].rememberedAppearance = pmap[i][j].rememberedAppearance;
            levels[oldLevelNumber - 1].mapStorage[i][j].rememberedItemCategory = pmap[i][j].rememberedItemCategory;
            levels[oldLevelNumber - 1].mapStorage[i][j].rememberedItemKind = pmap[i][j].rememberedItemKind;
            levels[oldLevelNumber - 1].mapStorage[i][j].rememberedItemQuantity = pmap[i][j].rememberedItemQuantity;
            levels[oldLevelNumber - 1].mapStorage[i][j].rememberedItemOriginDepth = pmap[i][j].rememberedItemOriginDepth;
            levels[oldLevelNumber - 1].mapStorage[i][j].rememberedTerrain = pmap[i][j].rememberedTerrain;
            levels[oldLevelNumber - 1].mapStorage[i][j].rememberedCellFlags = pmap[i][j].rememberedCellFlags;
            levels[oldLevelNumber - 1].mapStorage[i][j].rememberedTerrainFlags = pmap[i][j].rememberedTerrainFlags;
            levels[oldLevelNumber - 1].mapStorage[i][j].rememberedTMFlags = pmap[i][j].rememberedTMFlags;
        }
    }

    levels[oldLevelNumber - 1].awaySince = rogue.absoluteTurnNumber;

    //  Prepare the new level
    rogue.minersLightRadius = (DCOLS - 1) * FP_FACTOR;
    for (i = 0; i < rogue.depthLevel * gameConst->depthAccelerator; i++) {
        rogue.minersLightRadius = rogue.minersLightRadius * 85 / 100;
    }
    rogue.minersLightRadius += FP_FACTOR * 225 / 100;
    updateColors();
    updateRingBonuses(); // also updates miner's light

    if (!levels[rogue.depthLevel - 1].visited) { // level has not already been visited
        levels[rogue.depthLevel - 1].scentMap = allocGrid();
        scentMap = levels[rogue.depthLevel - 1].scentMap;
        fillGrid(levels[rogue.depthLevel - 1].scentMap, 0);

        // generate a seed from the current RNG state
        do {
            oldSeed = rand_64bits();
        } while (oldSeed == 0);

        // generate new level
        seedRandomGenerator(levels[rogue.depthLevel - 1].levelSeed);

        // Load up next level's monsters and items, since one might have fallen from above.
        monsters             = &levels[rogue.depthLevel-1].monsters;
        dormantMonsters      = &levels[rogue.depthLevel-1].dormantMonsters;
        floorItems->nextItem = levels[rogue.depthLevel-1].items;

        levels[rogue.depthLevel-1].items = NULL;

        pos upStairLocation;
        int failsafe;
        for (failsafe = 50; failsafe; failsafe--) {
            digDungeon();
            if (placeStairs(&upStairLocation)) {
                break;
            }
        }
        if (!failsafe) {
            printf("\nFailed to place stairs for level %d! Please report this error\n", rogue.depthLevel);
            exit(1);
        }
        initializeLevel(upStairLocation);
        setUpWaypoints();

        shuffleTerrainColors(100, false);

        // If we somehow failed to generate the amulet altar,
        // just toss an amulet in there somewhere.
        // It'll be fiiine!
        if (rogue.depthLevel == gameConst->amuletLevel
            && !numberOfMatchingPackItems(AMULET, 0, 0, false)
            && levels[rogue.depthLevel-1].visited == false) {

            for (theItem = floorItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
                if (theItem->category & AMULET) {
                    break;
                }
            }
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *monst = nextCreature(&it);
                if (monst->carriedItem
                    && (monst->carriedItem->category & AMULET)) {

                    theItem = monst->carriedItem;
                    break;
                }
            }
            if (!theItem) {
                placeItemAt(generateItem(AMULET, 0), INVALID_POS);
            }
        }

        // re-seed the RNG
        seedRandomGenerator(oldSeed);

        //logLevel();

        // Simulate 50 turns so the level is broken in (swamp gas accumulating, brimstone percolating, etc.).
        timeAway = 50;

    } else { // level has already been visited

        // restore level
        scentMap = levels[rogue.depthLevel - 1].scentMap;
        timeAway = clamp(0, rogue.absoluteTurnNumber - levels[rogue.depthLevel - 1].awaySince, 30000);

        for (i=0; i<DCOLS; i++) {
            for (j=0; j<DROWS; j++) {
                for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                    pmap[i][j].layers[layer] = levels[rogue.depthLevel - 1].mapStorage[i][j].layers[layer];
                }
                pmap[i][j].volume = levels[rogue.depthLevel - 1].mapStorage[i][j].volume;
                pmap[i][j].flags = (levels[rogue.depthLevel - 1].mapStorage[i][j].flags & PERMANENT_TILE_FLAGS);
                pmap[i][j].machineNumber = levels[rogue.depthLevel - 1].mapStorage[i][j].machineNumber;
                pmap[i][j].rememberedAppearance = levels[rogue.depthLevel - 1].mapStorage[i][j].rememberedAppearance;
                pmap[i][j].rememberedItemCategory = levels[rogue.depthLevel - 1].mapStorage[i][j].rememberedItemCategory;
                pmap[i][j].rememberedItemKind = levels[rogue.depthLevel - 1].mapStorage[i][j].rememberedItemKind;
                pmap[i][j].rememberedItemQuantity = levels[rogue.depthLevel - 1].mapStorage[i][j].rememberedItemQuantity;
                pmap[i][j].rememberedItemOriginDepth = levels[rogue.depthLevel - 1].mapStorage[i][j].rememberedItemOriginDepth;
                pmap[i][j].rememberedTerrain = levels[rogue.depthLevel - 1].mapStorage[i][j].rememberedTerrain;
                pmap[i][j].rememberedCellFlags = levels[rogue.depthLevel - 1].mapStorage[i][j].rememberedCellFlags;
                pmap[i][j].rememberedTerrainFlags = levels[rogue.depthLevel - 1].mapStorage[i][j].rememberedTerrainFlags;
                pmap[i][j].rememberedTMFlags = levels[rogue.depthLevel - 1].mapStorage[i][j].rememberedTMFlags;
            }
        }

        setUpWaypoints();

        rogue.downLoc = levels[rogue.depthLevel - 1].downStairsLoc;
        rogue.upLoc   = levels[rogue.depthLevel - 1].upStairsLoc;

        monsters             = &levels[rogue.depthLevel - 1].monsters;
        dormantMonsters      = &levels[rogue.depthLevel - 1].dormantMonsters;
        floorItems->nextItem = levels[rogue.depthLevel - 1].items;

        levels[rogue.depthLevel-1].items           = NULL;

        restoreItems();

    }

    // Simulate the environment!
    // First bury the player in limbo while we run the simulation,
    // so that any harmful terrain doesn't affect her during the process.
    px = player.loc.x;
    py = player.loc.y;
    player.loc.x = player.loc.y = 0;
    unsigned long currentTurnNumber = rogue.absoluteTurnNumber;
    timeAway = min(timeAway, 100);
    while (timeAway--) {
        rogue.absoluteTurnNumber = max(currentTurnNumber, timeAway) - timeAway;
        updateEnvironment();
    }
    rogue.absoluteTurnNumber = currentTurnNumber;
    player.loc.x = px;
    player.loc.y = py;

    // This level is now up-to-date as of the current turn.
    // Get the ticker ready for the *next* environment update.
    if (rogue.ticksTillUpdateEnvironment <= 0) {
        rogue.ticksTillUpdateEnvironment += 100;
    }

    if (!levels[rogue.depthLevel-1].visited) {
        levels[rogue.depthLevel-1].visited = true;
        if (rogue.depthLevel == gameConst->amuletLevel) {
            messageWithColor(levelFeelings[0].message, levelFeelings[0].color, 0);
        } else if (rogue.depthLevel == gameConst->deepestLevel) {
            messageWithColor(levelFeelings[1].message, levelFeelings[1].color, 0);
        }
    }

    // Position the player.
    pos loc;
    if (stairDirection == 0) { // fell into the level
        getQualifyingLocNear(&loc, player.loc, true, 0,
                             (T_PATHING_BLOCKER & ~T_IS_DEEP_WATER),
                             (HAS_MONSTER | HAS_ITEM | HAS_STAIRS | IS_IN_MACHINE), false, false);

        if (cellHasTerrainFlag(loc, T_IS_DEEP_WATER)) {
            // Fell into deep water... can we swim out of it?
            pos dryLoc;
            getQualifyingLocNear(&dryLoc, player.loc, true, 0,
                                (T_PATHING_BLOCKER),
                                (HAS_MONSTER | HAS_ITEM | HAS_STAIRS | IS_IN_MACHINE), false, false);

            short swimDistance = pathingDistance(loc.x, loc.y, dryLoc.x, dryLoc.y, T_PATHING_BLOCKER & ~T_IS_DEEP_WATER);
            if (swimDistance == 30000) {
                // Cannot swim out! This is an enclosed lake.
                loc = dryLoc;
            }
        }

    } else {
        if (stairDirection == 1) { // heading downward
            player.loc = rogue.upLoc;
        } else if (stairDirection == -1) { // heading upward
            player.loc = rogue.downLoc;
        }

        placedPlayer = false;
        for (dir=0; dir<4 && !placedPlayer; dir++) {
            loc = posNeighborInDirection(player.loc, dir);
            if (!cellHasTerrainFlag(loc, T_PATHING_BLOCKER)
                && !(pmapAt(loc)->flags & (HAS_MONSTER | HAS_STAIRS | IS_IN_MACHINE))) {
                placedPlayer = true;
            }
        }
        if (!placedPlayer) {
            getQualifyingPathLocNear(&loc.x, &loc.y,
                                     player.loc.x, player.loc.y,
                                     true,
                                     T_DIVIDES_LEVEL, 0,
                                     T_PATHING_BLOCKER, (HAS_MONSTER | HAS_STAIRS | IS_IN_MACHINE),
                                     false);
        }
    }
    player.loc = loc;

    pmapAt(player.loc)->flags |= HAS_PLAYER;

    // Notify the player if they arrive standing on (or otherwise in the same location as) an item
    if (itemAtLoc(player.loc)) {
        item *theItem = itemAtLoc(player.loc);
        char msg[COLS * 3], itemDescription[COLS * 3] = "";
        
        // the message pane wraps so we don't need to limit the description
        describedItemName(theItem, itemDescription, COLS * 3);
        sprintf(msg, "Below you lies %s.", itemDescription);
        messageWithColor(msg, &itemMessageColor, 0);
    }

    if (connectingStairsDiscovered) {
        for (i = rogue.upLoc.x-1; i <= rogue.upLoc.x + 1; i++) {
            for (j = rogue.upLoc.y-1; j <= rogue.upLoc.y + 1; j++) {
                if (coordinatesAreInMap(i, j)) {
                    discoverCell(i, j);
                }
            }
        }
    }
    if (cellHasTerrainFlag(player.loc, T_IS_DEEP_WATER) && !player.status[STATUS_LEVITATING]
        && !cellHasTerrainFlag(player.loc, (T_ENTANGLES | T_OBSTRUCTS_PASSABILITY))) {
        rogue.inWater = true;
    }

    if (levels[rogue.depthLevel - 1].visited) {
        mapToStairs = allocGrid();
        mapToPit = allocGrid();
        fillGrid(mapToStairs, 0);
        fillGrid(mapToPit, 0);
        calculateDistances(mapToStairs, player.loc.x, player.loc.y, T_PATHING_BLOCKER, NULL, true, true);
        calculateDistances(mapToPit, levels[rogue.depthLevel-1].playerExitedVia.x,
                           levels[rogue.depthLevel-1].playerExitedVia.y, T_PATHING_BLOCKER, NULL, true, true);
        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *monst = nextCreature(&it);
            restoreMonster(monst, mapToStairs, mapToPit);
        }
        freeGrid(mapToStairs);
        freeGrid(mapToPit);
    }

    updateMapToShore();
    updateVision(true);
    rogue.stealthRange = currentStealthRange();

    // update monster states so none are hunting if there is no scent and they can't see the player
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        updateMonsterState(monst);
    }

    rogue.playbackBetweenTurns = true;
    displayLevel();
    refreshSideBar(-1, -1, false);

    if (rogue.playerTurnNumber) {
        rogue.playerTurnNumber++; // Increment even though no time has passed.
    }
    RNGCheck();
    flushBufferToFile();
    deleteAllFlares(); // So discovering something on the same turn that you fall down a level doesn't flash stuff on the previous level.
    hideCursor();
}

static void freeGlobalDynamicGrid(short ***grid) {
    if (*grid) {
        freeGrid(*grid);
        *grid = NULL;
    }
}

void freeCreature(creature *monst) {
    freeGlobalDynamicGrid(&(monst->mapToMe));
    freeGlobalDynamicGrid(&(monst->safetyMap));
    if (monst->carriedItem) {
        free(monst->carriedItem);
        monst->carriedItem = NULL;
    }
    if (monst->carriedMonster) {
        freeCreature(monst->carriedMonster);
        monst->carriedMonster = NULL;
    }
    free(monst);
}

static void removeDeadMonstersFromList(creatureList *list) {
    // This needs to be able to access creatures that are dying, but `creatureIterator` skips
    // dying monsters so it can't be used here.
    creatureListNode *next = list->head;
    while (next != NULL) {
        creature *decedent = next->creature;
        next = next->nextCreature;
        if (decedent->bookkeepingFlags & MB_HAS_DIED) {
            removeCreature(list, decedent);
            if (decedent->leader == &player
                && !(decedent->bookkeepingFlags & MB_DOES_NOT_RESURRECT)
                && (!(decedent->info.flags & MONST_INANIMATE) 
                    || (monsterCatalog[decedent->info.monsterID].abilityFlags & MA_ENTER_SUMMONS))
                && (decedent->bookkeepingFlags & MB_WEAPON_AUTO_ID)
                && !(decedent->bookkeepingFlags & MB_ADMINISTRATIVE_DEATH)) {

                // Unset flag, since the purgatory list should be iterable.
                decedent->bookkeepingFlags &= ~MB_HAS_DIED;
                prependCreature(&purgatory, decedent); // add for possible future resurrection
            } else {
                freeCreature(decedent);
            }
        }
    }
}

// Removes dead monsters from `monsters`/`dormantMonsters`, and inserts them into `purgatory` if
// the decedent is a player ally at the moment of death, for possible future resurrection.
void removeDeadMonsters() {
    removeDeadMonstersFromList(monsters);
    removeDeadMonstersFromList(dormantMonsters);
}

void freeEverything() {
    short i;
    item *theItem, *theItem2;

#ifdef AUDIT_RNG
    fclose(RNGLogFile);
#endif

    freeGlobalDynamicGrid(&safetyMap);
    freeGlobalDynamicGrid(&allySafetyMap);
    freeGlobalDynamicGrid(&chokeMap);
    freeGlobalDynamicGrid(&rogue.mapToShore);
    freeGlobalDynamicGrid(&rogue.mapToSafeTerrain);

    for (i=0; i<gameConst->deepestLevel+1; i++) {
        freeCreatureList(&levels[i].monsters);
        freeCreatureList(&levels[i].dormantMonsters);

        for (theItem = levels[i].items; theItem != NULL; theItem = theItem2) {
            theItem2 = theItem->nextItem;
            deleteItem(theItem);
        }
        levels[i].items = NULL;
        if (levels[i].scentMap) {
            freeGrid(levels[i].scentMap);
            levels[i].scentMap = NULL;
        }
    }
    scentMap = NULL;
    freeCreatureList(&purgatory);

    for (theItem = floorItems; theItem != NULL; theItem = theItem2) {
        theItem2 = theItem->nextItem;
        deleteItem(theItem);
    }
    floorItems = NULL;
    for (theItem = packItems; theItem != NULL; theItem = theItem2) {
        theItem2 = theItem->nextItem;
        deleteItem(theItem);
    }
    packItems = NULL;
    for (theItem = monsterItemsHopper; theItem != NULL; theItem = theItem2) {
        theItem2 = theItem->nextItem;
        deleteItem(theItem);
    }
    monsterItemsHopper = NULL;
    for (i=0; i<MAX_WAYPOINT_COUNT; i++) {
        freeGrid(rogue.wpDistance[i]);
    }

    deleteAllFlares();
    if (rogue.flares) {
        free(rogue.flares);
        rogue.flares = NULL;
    }

    free(levels);
    levels = NULL;

    free(rogue.featRecord);
}

void gameOver(char *killedBy, boolean useCustomPhrasing) {
    short i, y;
    char buf[200], highScoreText[200], buf2[200];
    rogueHighScoresEntry theEntry;
    boolean playback;
    rogueEvent theEvent;
    item *theItem;
    char recordingFilename[BROGUE_FILENAME_MAX] = {0};

    if (player.bookkeepingFlags & MB_IS_DYING) {
        // we've already been through this once; let's avoid overkill.
        return;
    }

    player.bookkeepingFlags |= MB_IS_DYING;
    rogue.autoPlayingLevel = false;
    rogue.gameInProgress = false;
    flushBufferToFile();

    if (rogue.playbackFastForward) {
        rogue.playbackFastForward = false;
        displayLevel();
    }

    if (rogue.quit) {
        if (rogue.playbackMode) {
            rogue.playbackMode = false;
            message("(The player quit at this point.)", REQUIRE_ACKNOWLEDGMENT);
            rogue.playbackMode = true;
        }
    } else {
        playback = rogue.playbackMode;
        if (!D_IMMORTAL && !nonInteractivePlayback) {
            rogue.playbackMode = false;
        }
        strcpy(buf, "You die...");
        if (KEYBOARD_LABELS) {
            encodeMessageColor(buf, strlen(buf), &gray);
            strcat(buf, " (press 'i' to view your inventory)");
        }
        player.currentHP = 0; // So it shows up empty in the side bar.
        refreshSideBar(-1, -1, false);
        messageWithColor(buf, &badMessageColor, 0);
        displayMoreSignWithoutWaitingForAcknowledgment();

        do {
            if (rogue.playbackMode) break;
            nextBrogueEvent(&theEvent, false, false, false);
            if (theEvent.eventType == KEYSTROKE
                && theEvent.param1 != ACKNOWLEDGE_KEY
                && theEvent.param1 != ESCAPE_KEY
                && theEvent.param1 != INVENTORY_KEY) {

                flashTemporaryAlert(" -- Press space or click to continue, or press 'i' to view inventory -- ", 1500);
            } else if (theEvent.eventType == KEYSTROKE && theEvent.param1 == INVENTORY_KEY) {
                for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
                    identify(theItem);
                    theItem->flags &= ~ITEM_MAGIC_DETECTED;
                }
                displayInventory(ALL_ITEMS, 0, 0, true, false);
            }
        } while (!(theEvent.eventType == KEYSTROKE && (theEvent.param1 == ACKNOWLEDGE_KEY || theEvent.param1 == ESCAPE_KEY)
                   || theEvent.eventType == MOUSE_UP));

        confirmMessages();

        rogue.playbackMode = playback;
    }

    rogue.creaturesWillFlashThisTurn = false;

    if (D_IMMORTAL && !rogue.quit) {
        message("...but then you get better.", 0);
        player.currentHP = player.info.maxHP;
        if (player.status[STATUS_NUTRITION] < 10) {
            player.status[STATUS_NUTRITION] = STOMACH_SIZE;
        }
        player.bookkeepingFlags &= ~MB_IS_DYING;
        rogue.gameInProgress = true;
        return;
    }

    if (rogue.highScoreSaved) {
        return;
    }
    rogue.highScoreSaved = true;

    if (rogue.quit) {
        blackOutScreen();
    } else {
        screenDisplayBuffer dbuf = displayBuffer;
        funkyFade(&dbuf, &black, 0, 120, mapToWindowX(player.loc.x), mapToWindowY(player.loc.y), false);
    }

    if (useCustomPhrasing) {
        sprintf(buf, "%s on depth %i", killedBy, rogue.depthLevel);
    } else {
        sprintf(buf, "Killed by a%s %s on depth %i", (isVowelish(killedBy) ? "n" : ""), killedBy,
                rogue.depthLevel);
    }

    // Count gems as 500 gold each
    short numGems = numberOfMatchingPackItems(GEM, 0, 0, false);
    rogue.gold += 500 * numGems;
    theEntry.score = rogue.gold;

    if (rogue.easyMode) {
        theEntry.score /= 10;
    }
    strcpy(highScoreText, buf);
    if (theEntry.score > 0) {
        sprintf(buf2, (numGems > 0) ? " with treasure worth %li gold" : " with %li gold", theEntry.score);
        strcat(buf, buf2);
    }
    if (numberOfMatchingPackItems(AMULET, 0, 0, false) > 0) {
        strcat(buf, ", amulet in hand");
    }
    strcat(buf, ".");
    strcat(highScoreText, ".");

    strcpy(theEntry.description, highScoreText);

    if (!rogue.quit) {
        printString(buf, (COLS - strLenWithoutEscapes(buf)) / 2, ROWS / 2, &gray, &black, 0);

        y = ROWS / 2 + 3;
        for (i = 0; i < gameConst->numberFeats; i++) {
            if (rogue.featRecord[i]
                && !featTable[i].initialValue) {

                sprintf(buf, "%s: %s", featTable[i].name, featTable[i].description);
                printString(buf, (COLS - strLenWithoutEscapes(buf)) / 2, y, &advancementMessageColor, &black, 0);
                y++;
            }
        }

        displayMoreSign();
    }

    if (serverMode) {
        blackOutScreen();
        saveRecordingNoPrompt(recordingFilename);
    } else {
        if (!rogue.playbackMode && saveHighScore(theEntry)) {
            printHighScores(true);
        }
        blackOutScreen();
        saveRecording(recordingFilename);
    }

    if (rogue.playbackMode && nonInteractivePlayback) {
        printf("Recording: %s ended after %li turns (game over).\n", rogue.currentGamePath, rogue.playerTurnNumber);
    }

    if (!rogue.playbackMode) {
        if (!rogue.quit) {
            notifyEvent(GAMEOVER_DEATH, theEntry.score, 0, theEntry.description, recordingFilename);
        } else {
            notifyEvent(GAMEOVER_QUIT, theEntry.score, 0, theEntry.description, recordingFilename);
        }
    } else {
        notifyEvent(GAMEOVER_RECORDING, 0, 0, "recording ended", "none");
    }

    if (!rogue.playbackMode && !rogue.easyMode && !rogue.wizard) {
        saveRunHistory(rogue.quit ? "Quit" : "Died", rogue.quit ? "-" : killedBy, (int) theEntry.score, numGems);
    }

    rogue.gameHasEnded = true;
    rogue.gameExitStatusCode = EXIT_STATUS_SUCCESS;
}

void victory(boolean superVictory) {
    char buf[COLS*3], victoryVerb[20];
    item *theItem;
    short i, j, gemCount = 0;
    unsigned long totalValue = 0;
    rogueHighScoresEntry theEntry;
    boolean qualified, isPlayback;
    
    char recordingFilename[BROGUE_FILENAME_MAX] = {0};

    rogue.gameInProgress = false;
    flushBufferToFile();

    if (rogue.playbackFastForward) {
        rogue.playbackFastForward = false;
        displayLevel();
    }

    //
    // First screen - Congratulations...
    //
    if (superVictory) {
        message(    "Light streams through the portal, and you are teleported out of the dungeon.", 0);
        screenDisplayBuffer dbuf = displayBuffer;
        funkyFade(&dbuf, &superVictoryColor, 0, 240, mapToWindowX(player.loc.x), mapToWindowY(player.loc.y), false);
        displayMoreSign();
        printString("Congratulations; you have transcended the Dungeons of Doom!                 ", mapToWindowX(0), mapToWindowY(-1), &black, &white, 0);
        displayMoreSign();
        clearDisplayBuffer(&dbuf);
        deleteMessages();
        strcpy(displayedMessage[0], "You retire in splendor, forever renowned for your remarkable triumph.     ");
    } else {
        message(    "You are bathed in sunlight as you throw open the heavy doors.", 0);
        screenDisplayBuffer dbuf = displayBuffer;
        funkyFade(&dbuf, &white, 0, 240, mapToWindowX(player.loc.x), mapToWindowY(player.loc.y), false);
        displayMoreSign();
        printString("Congratulations; you have escaped from the Dungeons of Doom!     ", mapToWindowX(0), mapToWindowY(-1), &black, &white, 0);
        displayMoreSign();
        deleteMessages();
        strcpy(displayedMessage[0], "You sell your treasures and live out your days in fame and glory.");
    }

    screenDisplayBuffer dbuf;
    clearDisplayBuffer(&dbuf);

    //
    // Second screen - Show inventory and item's value
    //
    printString(displayedMessage[0], mapToWindowX(0), mapToWindowY(-1), &white, &black, &dbuf);

    plotCharToBuffer(G_GOLD, mapToWindow((pos){ 2, 1 }), &yellow, &black, &dbuf);
    printString("Gold", mapToWindowX(4), mapToWindowY(1), &white, &black, &dbuf);
    sprintf(buf, "%li", rogue.gold);
    printString(buf, mapToWindowX(60), mapToWindowY(1), &itemMessageColor, &black, &dbuf);
    totalValue += rogue.gold;

    for (i = 4, theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (theItem->category & GEM) {
            gemCount += theItem->quantity;
        }
        if (theItem->category == AMULET && superVictory) {
            plotCharToBuffer(G_AMULET, (windowpos){ mapToWindowX(2), min(ROWS-1, i + 1) }, &yellow, &black, &dbuf);
            printString("The Birthright of Yendor", mapToWindowX(4), min(ROWS-1, i + 1), &itemMessageColor, &black, &dbuf);
            sprintf(buf, "%li", max(0, itemValue(theItem) * 2));
            printString(buf, mapToWindowX(60), min(ROWS-1, i + 1), &itemMessageColor, &black, &dbuf);
            totalValue += max(0, itemValue(theItem) * 2);
            i++;
        } else {
            identify(theItem);
            itemName(theItem, buf, true, true, &white);
            upperCase(buf);

            plotCharToBuffer(theItem->displayChar, (windowpos){ mapToWindowX(2), min(ROWS-1, i + 1) }, &yellow, &black, &dbuf);
            printString(buf, mapToWindowX(4), min(ROWS-1, i + 1), &white, &black, &dbuf);

            if (itemValue(theItem) > 0) {
                sprintf(buf, "%li", max(0, itemValue(theItem)));
                printString(buf, mapToWindowX(60), min(ROWS-1, i + 1), &itemMessageColor, &black, &dbuf);
            }

            totalValue += max(0, itemValue(theItem));
            i++;
        }
    }
    i++;
    printString("TOTAL:", mapToWindowX(2), min(ROWS-1, i + 1), &lightBlue, &black, &dbuf);
    sprintf(buf, "%li", totalValue);
    printString(buf, mapToWindowX(60), min(ROWS-1, i + 1), &lightBlue, &black, &dbuf);

    funkyFade(&dbuf, &white, 0, 120, COLS/2, ROWS/2, true);
    displayMoreSign();

    //
    // Third screen - List of achievements with recording save prompt
    //
    blackOutScreen();

    i = 4;
    printString("Achievements", mapToWindowX(2), i++, &lightBlue, &black, NULL);

    i++;
    for (j = 0; i < ROWS && j < gameConst->numberFeats; j++) {
        if (rogue.featRecord[j]) {
            sprintf(buf, "%s: %s", featTable[j].name, featTable[j].description);
            printString(buf, mapToWindowX(2), i, &advancementMessageColor, &black, NULL);
            i++;
        }
    }

    strcpy(victoryVerb, superVictory ? "Mastered" : "Escaped");
    if (gemCount == 0) {
        sprintf(theEntry.description, "%s the Dungeons of Doom!", victoryVerb);
    } else if (gemCount == 1) {
        sprintf(theEntry.description, "%s the Dungeons of Doom with a lumenstone!", victoryVerb);
    } else {
        sprintf(theEntry.description, "%s the Dungeons of Doom with %i lumenstones!", victoryVerb, gemCount);
    }

    theEntry.score = totalValue;

    if (rogue.easyMode) {
        theEntry.score /= 10;
    }

    if (!rogue.wizard && !rogue.playbackMode) {
        qualified = saveHighScore(theEntry);
    } else {
        qualified = false;
    }

    isPlayback = rogue.playbackMode;
    rogue.playbackMode = false;
    rogue.playbackMode = isPlayback;

    displayMoreSign();

    if (serverMode) {
        saveRecordingNoPrompt(recordingFilename);
    } else {
        blackOutScreen();
        saveRecording(recordingFilename);
        printHighScores(qualified);
    }

    if (rogue.playbackMode && nonInteractivePlayback) {
        printf("Recording: %s ended after %li turns (victory).\n", rogue.currentGamePath, rogue.playerTurnNumber);
    }

    if (!rogue.playbackMode) {
        if (superVictory) {
            notifyEvent(GAMEOVER_SUPERVICTORY, theEntry.score, 0, theEntry.description, recordingFilename);
        } else {
            notifyEvent(GAMEOVER_VICTORY, theEntry.score, 0, theEntry.description, recordingFilename);
        }
    } else {
        notifyEvent(GAMEOVER_RECORDING, 0, 0, "recording ended", "none");
    }

    if (!rogue.playbackMode && !rogue.easyMode && !rogue.wizard) {
        saveRunHistory(victoryVerb, "-", (int) theEntry.score, gemCount);
    }

    rogue.gameHasEnded = true;
    rogue.gameExitStatusCode = EXIT_STATUS_SUCCESS;
}

void enableEasyMode() {
    if (rogue.easyMode) {
        message("Alas, all hope of salvation is lost. You shed scalding tears at your plight.", 0);
        return;
    }
    message("A dark presence surrounds you, whispering promises of stolen power.", REQUIRE_ACKNOWLEDGMENT);
    if (confirm("Succumb to demonic temptation (i.e. enable Easy Mode)?", false)) {
        recordKeystroke(EASY_MODE_KEY, false, true);
        message("An ancient and terrible evil burrows into your willing flesh!", REQUIRE_ACKNOWLEDGMENT);
        rogue.easyMode = true;
        setPlayerDisplayChar();
        refreshDungeonCell(player.loc);
        refreshSideBar(-1, -1, false);
        message("Wracked by spasms, your body contorts into an ALL-POWERFUL AMPERSAND!!!", 0);
        message("You have a feeling that you will take 20% as much damage from now on.", 0);
        message("But great power comes at a great price -- specifically, a 90% income tax rate.", 0);
    } else {
        message("The evil dissipates, hissing, from the air around you.", 0);
    }
}

// takes a flag of the form Fl(n) and returns n
short unflag(unsigned long flag) {
    short i;
    for (i=0; i<32; i++) {
        if (flag >> i == 1) {
            return i;
        }
    }
    return -1;
}
