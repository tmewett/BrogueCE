#include "Rogue.h"
#include "IncludeGlobals.h"

static void seedCatalogItemName(item *theItem, char *name, creature *theMonster) {
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
    sprintf(name, "%s%s%s%s", buf, monster, location, usageLocation);
    return;
}

static void seedCatalogMonsterName(char *name, creature *theMonster) {
    char descriptor[16] = "";
    char location[16] = "";

    if (theMonster->bookkeepingFlags & MB_CAPTIVE) {
        if (cellHasTMFlag(theMonster->xLoc, theMonster->yLoc, TM_PROMOTES_WITH_KEY)) {
            strcpy(descriptor,"A caged ");
        } else {
            strcpy(descriptor,"A shackled ");
        }
    } else if (theMonster->creatureState == MONSTER_ALLY) {
        strcpy(descriptor, "An allied ");
    }
    sprintf(name,"%s%s%s", descriptor, theMonster->info.monsterName, location);
}

static void printSeedCatalogMonsters(boolean includeAll) {
    creature *theMonster;
    char name[500];

    for (theMonster = monsters->nextCreature; theMonster != NULL; theMonster = theMonster->nextCreature) {
        seedCatalogMonsterName(name, theMonster);
        upperCase(name);
        if (theMonster->bookkeepingFlags & MB_CAPTIVE || theMonster->creatureState == MONSTER_ALLY || includeAll) {
            printf("        %s\n", name);
        }
    }

    for (theMonster = dormantMonsters->nextCreature; theMonster != NULL; theMonster = theMonster->nextCreature) {
        seedCatalogMonsterName(name, theMonster);
        upperCase(name);
        if (theMonster->bookkeepingFlags & MB_CAPTIVE || theMonster->creatureState == MONSTER_ALLY || includeAll) {
            printf("        %s\n", name);
        }
    }
}

static void printSeedCatalogMonsterItems() {
    creature *theMonster;
    char name[500];

    for (theMonster = monsters->nextCreature; theMonster != NULL; theMonster = theMonster->nextCreature) {
        if (theMonster->carriedItem != NULL && theMonster->carriedItem->category != GOLD) {
                seedCatalogItemName(theMonster->carriedItem, name, theMonster);
                printf("        %s\n", name);
        }
    }

    for (theMonster = dormantMonsters->nextCreature; theMonster != NULL; theMonster = theMonster->nextCreature) {
        if (theMonster->carriedItem != NULL && theMonster->carriedItem->category != GOLD) {
            seedCatalogItemName(theMonster->carriedItem, name, theMonster);
            printf("        %s\n", name);
        }
    }
}

static void printSeedCatalogFloorItems() {
    item *theItem;
    int gold = 0;
    short piles = 0;
    char buf[500];

    for (theItem = floorItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (theItem->category == GOLD) {
            piles++;
            gold += theItem->quantity;
        } else if (theItem->category == AMULET) {
        } else {
            seedCatalogItemName(theItem, buf, NULL);
            printf("        %s\n", buf);
        }
    }
    if (piles == 1) {
        printf("        %i gold pieces\n", gold);
    }
    if (piles > 1) {
        printf("        %i gold pieces (%i piles)\n", gold, piles);
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

    printf("Brogue seed catalog, seeds %lu to %lu, through depth %u.\n\n\
To play one of these seeds, press control-N from the title screen \
and enter the seed number. Knowing which items will appear on \
the first %u depths will, of course, make the game significantly easier.\n\n",
            startingSeed, startingSeed + numberOfSeedsToScan - 1, scanThroughDepth, scanThroughDepth);

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
