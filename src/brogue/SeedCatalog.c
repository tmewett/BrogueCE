/*
 *  SeedCatalog.c
 *  Brogue
 *
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
#include "IncludeGlobals.h"

static void printSeedCatalogItem(item *theItem, creature *theMonster) {
    char buf[500] = "", monster[128] = "", location[36] = "", usageLocation[36] = "";

    itemName(theItem, buf, true, true, NULL);

    //monster
    if (theMonster != NULL) {
        sprintf(monster, " (%s)", theMonster->info.monsterName);
    }

    //location
    if (pmap[theItem->xLoc][theItem->yLoc].machineNumber > 0) {
        //not all machines are "vaults" so we need to exclude some.
        if (pmap[theItem->xLoc][theItem->yLoc].layers[0] != ALTAR_SWITCH
            && pmap[theItem->xLoc][theItem->yLoc].layers[0] != ALTAR_SWITCH_RETRACTING
            && pmap[theItem->xLoc][theItem->yLoc].layers[0] != ALTAR_CAGE_RETRACTABLE
            && pmap[theItem->xLoc][theItem->yLoc].layers[0] != ALTAR_INERT
            && pmap[theItem->xLoc][theItem->yLoc].layers[0] != AMULET_SWITCH
            && pmap[theItem->xLoc][theItem->yLoc].layers[0] != FLOOR) {

            sprintf(location, " (vault %i)", pmap[theItem->xLoc][theItem->yLoc].machineNumber);
        }
    }

    //usage location
    if (theItem->category == KEY && theItem->kind == KEY_DOOR) {
        sprintf(usageLocation, " (opens vault %i)", pmap[theItem->keyLoc[0].x][theItem->keyLoc[0].y].machineNumber - 1);
    }

    upperCase(buf);
    printf("        %s%s%s%s\n", buf, monster, location, usageLocation);
    return;
}

static void printSeedCatalogMonster(creature *theMonster) {
    char descriptor[16] = "";

    if (theMonster->bookkeepingFlags & MB_CAPTIVE) {
        if (cellHasTMFlag(theMonster->xLoc, theMonster->yLoc, TM_PROMOTES_WITH_KEY)) {
            strcpy(descriptor,"A caged ");
        } else {
            strcpy(descriptor,"A shackled ");
        }
    } else if (theMonster->creatureState == MONSTER_ALLY) {
        strcpy(descriptor, "An allied ");
    }
    printf("        %s%s\n", descriptor, theMonster->info.monsterName);
}

static void printSeedCatalogMonsters(boolean includeAll) {
    creature *theMonster;

    for (theMonster = monsters->nextCreature; theMonster != NULL; theMonster = theMonster->nextCreature) {
        if (theMonster->bookkeepingFlags & MB_CAPTIVE || theMonster->creatureState == MONSTER_ALLY || includeAll) {
            printSeedCatalogMonster(theMonster);
        }
    }

    for (theMonster = dormantMonsters->nextCreature; theMonster != NULL; theMonster = theMonster->nextCreature) {
        if (theMonster->bookkeepingFlags & MB_CAPTIVE || theMonster->creatureState == MONSTER_ALLY || includeAll) {
            printSeedCatalogMonster(theMonster);
        }
    }
}

static void printSeedCatalogMonsterItems() {
    creature *theMonster;

    for (theMonster = monsters->nextCreature; theMonster != NULL; theMonster = theMonster->nextCreature) {
        if (theMonster->carriedItem != NULL && theMonster->carriedItem->category != GOLD) {
            printSeedCatalogItem(theMonster->carriedItem, theMonster);
        }
    }

    for (theMonster = dormantMonsters->nextCreature; theMonster != NULL; theMonster = theMonster->nextCreature) {
        if (theMonster->carriedItem != NULL && theMonster->carriedItem->category != GOLD) {
            printSeedCatalogItem(theMonster->carriedItem, theMonster);
        }
    }
}

static void printSeedCatalogFloorGold(int gold, short piles) {

    if (piles == 1) {
        printf("        %i gold pieces\n", gold);
    } else if (piles > 1) {
        printf("        %i gold pieces (%i piles)\n", gold, piles);
    }
}

static void printSeedCatalogFloorItems() {
    item *theItem;
    int gold = 0;
    short piles = 0;

    for (theItem = floorItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (theItem->category == GOLD) {
            piles++;
            gold += theItem->quantity;
        } else if (theItem->category == AMULET) {
        } else {
            printSeedCatalogItem(theItem, NULL);
        }
    }

    if (gold > 0) {
        printSeedCatalogFloorGold(gold, piles);
    }
}

static void printSeedCatalogAltars() {
    short i, j;
    boolean c_altars[100] = {0};

    for (j = 0; j < DROWS; j++) {
        for (i = 0; i < DCOLS; i++) {
            if (pmap[i][j].layers[0] == RESURRECTION_ALTAR) {
                printf("        A resurrection altar (vault %i)\n", pmap[i][j].machineNumber);
            }
            // commutation altars come in pairs. we only want to print 1.
            if (pmap[i][j].layers[0] == COMMUTATION_ALTAR) {
                c_altars[pmap[i][j].machineNumber] = true;
            }
        }
    }
    for (i = 0; i < 100; i++){
        if (c_altars[i]) {
            printf("        A commutation altar (vault %i)\n",i);
        }
    }
}

void printSeedCatalog(unsigned long startingSeed, unsigned long numberOfSeedsToScan, unsigned int scanThroughDepth) {
    unsigned long theSeed;
    char path[BROGUE_FILENAME_MAX];

    rogue.nextGame = NG_NOTHING;

    getAvailableFilePath(path, LAST_GAME_NAME, GAME_SUFFIX);
    strcat(path, GAME_SUFFIX);

    printf("Brogue seed catalog, seeds %lu to %lu, through depth %u.\n"
                     "Generated with %s. Dungeons unchanged since %s.\n\n"
                     "To play one of these seeds, press control-N from the title screen"
                     "and enter the seed number. Knowing which items will appear on"
                     "the first %u depths will, of course, make the game significantly easier.\n",
            startingSeed, startingSeed + numberOfSeedsToScan - 1, scanThroughDepth, BROGUE_VERSION_STRING, BROGUE_DUNGEON_VERSION_STRING, scanThroughDepth);

    for (theSeed = startingSeed; theSeed < startingSeed + numberOfSeedsToScan; theSeed++) {
        printf("Seed %lu:\n", theSeed);
        fprintf(stderr, "Scanning seed %lu...\n", theSeed);
        rogue.nextGamePath[0] = '\0';
        randomNumbersGenerated = 0;

        rogue.playbackMode = false;
        rogue.playbackFastForward = false;
        rogue.playbackBetweenTurns = false;

        strcpy(currentFilePath, path);
        initializeRogue(theSeed);
        rogue.playbackOmniscience = true;
        for (rogue.depthLevel = 1; rogue.depthLevel <= scanThroughDepth; rogue.depthLevel++) {
            startLevel(rogue.depthLevel == 1 ? 1 : rogue.depthLevel - 1, 1); // descending into level n
            printf("    Depth %i:\n", rogue.depthLevel);

            printSeedCatalogFloorItems();
            printSeedCatalogMonsterItems();
            printSeedCatalogMonsters(false); // captives and allies only
            if (rogue.depthLevel >= 13) { // resurrection & commutation altars can spawn starting on 13
                printSeedCatalogAltars();
            }
        }

        freeEverything();
        remove(currentFilePath); // Don't add a spurious LastGame file to the brogue folder.
    }

}
