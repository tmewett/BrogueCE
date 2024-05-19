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
#include "GlobalsBase.h"
#include "Globals.h"

#define  CSV_HEADER_STRING "dungeon_version,seed,depth,quantity,category,kind,enchantment,runic,vault_number,opens_vault_number,carried_by_monster_name,ally_status_name,mutation_name"
#define  NO_ENCHANTMENT_STRING ""
#define  NO_RUNIC_STRING ""
#define  NO_VAULT_STRING ""
#define  NO_OPENS_VAULT_STRING ""
#define  NO_CARRIED_BY_MONSTER_STRING ""
#define  NO_ALLY_STATUS_STRING ""
#define  NO_MUTATION_STRING ""

static void printSeedCatalogCsvLine(uint64_t seed, short depth, short quantity, char categoryName[50], char kindName[50],
                                    char enchantment[50], char runicName[50], char vaultNumber[10], char opensVaultNumber[10],
                                    char carriedByMonsterName[50], char allyStatusName[20], char mutationName[100]){

    printf("%s,%llu,%i,%i,%s,%s,%s,%s,%s,%s,%s,%s,%s\n", gameConst->dungeonVersionString, (unsigned long long) seed, depth, quantity, categoryName,
           kindName, enchantment, runicName, vaultNumber, opensVaultNumber, carriedByMonsterName, allyStatusName,
           mutationName);
}

static void getMonsterDetailedName(creature *theMonster, char *theMonsterName) {
    if (theMonster->mutationIndex >= 0) {
        sprintf(theMonsterName, "%s [%s]", theMonster->info.monsterName, mutationCatalog[theMonster->mutationIndex].title);
    } else {
        strcpy(theMonsterName, theMonster->info.monsterName);
    }
}

static void printSeedCatalogItem(item *theItem, creature *theMonster, boolean isCsvFormat) {
    char inGameItemName[500] = "", carriedByMonsterName[100] = "", vaultNumber[36] = "", opensVaultNumber[36] = "";
    char categoryName[20] = "", kindName[50] = "", enchantment[5] = "", runicName[30] = "", mutationName[100] = "";

    if (isCsvFormat) {     //for csv output we need the item name components: category, kind, enchantment, & runic
        strcpy(categoryName, itemCategoryNames[unflag(theItem->category)]);
        itemKindName(theItem, kindName);
        itemRunicName(theItem, runicName);
        if (theItem->category & (ARMOR | CHARM | RING | STAFF | WAND | WEAPON)) {   //enchantable items
            if (theItem->category == WAND) {
                sprintf(enchantment, "%i", theItem->charges);
            } else {
                sprintf(enchantment, "%i", theItem->enchant1);
            }
        }
    } else {
        itemName(theItem, inGameItemName, true, true, NULL);   //for standard output, use the in-game item name as base
    }

    if (theMonster != NULL) {   //carried by monster
        if (isCsvFormat) {
            sprintf(carriedByMonsterName, "%s", theMonster->info.monsterName);
            strcpy(mutationName, theMonster->mutationIndex >= 0 ? mutationCatalog[theMonster->mutationIndex].title : "");
        } else {
            getMonsterDetailedName(theMonster, carriedByMonsterName);
        }
    }

    // vaultNumber
    if (pmapAt(theItem->loc)->machineNumber > 0) {
        //not all machines are "vaults" so we need to exclude some.
        if (pmapAt(theItem->loc)->layers[0] != ALTAR_SWITCH
            && pmapAt(theItem->loc)->layers[0] != ALTAR_SWITCH_RETRACTING
            && pmapAt(theItem->loc)->layers[0] != ALTAR_CAGE_RETRACTABLE
            && pmapAt(theItem->loc)->layers[0] != ALTAR_INERT
            && pmapAt(theItem->loc)->layers[0] != AMULET_SWITCH
            && pmapAt(theItem->loc)->layers[0] != FLOOR) {

            sprintf(vaultNumber, isCsvFormat ? "%i" : " (vault %i)", pmapAt(theItem->loc)->machineNumber);
        }
    }

    // opensVaultNumber
    if (theItem->category == KEY && theItem->kind == KEY_DOOR) {
        sprintf(opensVaultNumber, isCsvFormat ? "%i" : " (opens vault %i)",
                pmapAt(theItem->keyLoc[0].loc)->machineNumber - 1);
    }

    if (isCsvFormat) {
        printSeedCatalogCsvLine(rogue.seed, rogue.depthLevel, theItem->quantity, categoryName, kindName, enchantment,
                                runicName, vaultNumber, opensVaultNumber, carriedByMonsterName, NO_ALLY_STATUS_STRING,
                                mutationName);
    } else {
        upperCase(inGameItemName);
        if (theMonster != NULL) {
            printf("        %s (%s)%s%s\n", inGameItemName, carriedByMonsterName, vaultNumber, opensVaultNumber);
        } else {
            printf("        %s%s%s\n", inGameItemName, vaultNumber, opensVaultNumber);
        }
    }
}

static void printSeedCatalogMonster(creature *theMonster, boolean isCsvFormat) {
    char categoryName[10] = "", allyStatusName[20] = "", mutationName[100] = "", theMonsterName[100] = "";

    strcpy(mutationName, theMonster->mutationIndex >= 0 ? mutationCatalog[theMonster->mutationIndex].title : "");

    if (theMonster->bookkeepingFlags & MB_CAPTIVE) {
        strcpy(categoryName,"ally");
        if (cellHasTMFlag(theMonster->loc, TM_PROMOTES_WITH_KEY)) {
            strcpy(allyStatusName, isCsvFormat ? "caged" : "A caged ");
        } else {
            strcpy(allyStatusName, isCsvFormat ? "shackled" : "A shackled ");
        }
    } else if (theMonster->creatureState == MONSTER_ALLY) {
        strcpy(categoryName,"ally");
        strcpy(allyStatusName, isCsvFormat ? "allied" : "An allied ");
    } else {
        strcpy(categoryName,"monster");
    }

    if (isCsvFormat) {
        printSeedCatalogCsvLine(rogue.seed, rogue.depthLevel, 1, categoryName, theMonster->info.monsterName,
                                NO_ENCHANTMENT_STRING, NO_RUNIC_STRING, NO_VAULT_STRING, NO_OPENS_VAULT_STRING,
                                NO_CARRIED_BY_MONSTER_STRING, allyStatusName, mutationName);
    } else {
        getMonsterDetailedName(theMonster, theMonsterName);
        printf("        %s%s\n", allyStatusName, theMonsterName);
    }
}

static void printSeedCatalogMonsters(boolean isCsvFormat, boolean includeAll) {
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *theMonster = nextCreature(&it);
        if (theMonster->bookkeepingFlags & MB_CAPTIVE || theMonster->creatureState == MONSTER_ALLY || includeAll) {
            printSeedCatalogMonster(theMonster, isCsvFormat);
        }
    }

    for (creatureIterator it = iterateCreatures(dormantMonsters); hasNextCreature(it);) {
        creature *theMonster = nextCreature(&it);
        if (theMonster->bookkeepingFlags & MB_CAPTIVE || theMonster->creatureState == MONSTER_ALLY || includeAll) {
            printSeedCatalogMonster(theMonster, isCsvFormat);
        }
    }
}

static void printSeedCatalogMonsterItems(boolean isCsvFormat) {
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *theMonster = nextCreature(&it);
        if (theMonster->carriedItem != NULL && theMonster->carriedItem->category != GOLD) {
            printSeedCatalogItem(theMonster->carriedItem, theMonster, isCsvFormat);
        }
    }

    for (creatureIterator it = iterateCreatures(dormantMonsters); hasNextCreature(it);) {
        creature *theMonster = nextCreature(&it);
        if (theMonster->carriedItem != NULL && theMonster->carriedItem->category != GOLD) {
            printSeedCatalogItem(theMonster->carriedItem, theMonster, isCsvFormat);
        }
    }
}

static void printSeedCatalogFloorGold(int gold, short piles, boolean isCsvFormat) {
    char kindName[50] = "";

    if (isCsvFormat) {
        if (piles == 1) {
            strcpy(kindName, "gold pieces");
        } else if (piles > 1) {
            sprintf(kindName, "gold pieces (%i piles)", piles);
        }
        printSeedCatalogCsvLine(rogue.seed, rogue.depthLevel, gold, "gold", kindName, NO_ENCHANTMENT_STRING,
                                NO_RUNIC_STRING, NO_VAULT_STRING, NO_OPENS_VAULT_STRING, NO_CARRIED_BY_MONSTER_STRING,
                                NO_ALLY_STATUS_STRING, NO_MUTATION_STRING);
    } else {
        if (piles == 1) {
            printf("        %i gold pieces\n", gold);
        } else if (piles > 1) {
            printf("        %i gold pieces (%i piles)\n", gold, piles);
        }
    }
}

static void printSeedCatalogFloorItems(boolean isCsvFormat) {
    item *theItem;
    int gold = 0;
    short piles = 0;

    for (theItem = floorItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (theItem->category == GOLD) {
            piles++;
            gold += theItem->quantity;
        } else if (theItem->category == AMULET) {
        } else {
            printSeedCatalogItem(theItem, NULL, isCsvFormat);
        }
    }

    if (gold > 0) {
        printSeedCatalogFloorGold(gold, piles, isCsvFormat);
    }
}

static void printSeedCatalogAltars(boolean isCsvFormat) {
    short i, j;
    boolean c_altars[50] = {0}; //IO.displayMachines uses 50
    char vaultNumber[10] = "";

    for (j = 0; j < DROWS; j++) {
        for (i = 0; i < DCOLS; i++) {
            if (pmap[i][j].layers[0] == RESURRECTION_ALTAR) {
                sprintf(vaultNumber, "%i", pmap[i][j].machineNumber);
                if (isCsvFormat) {
                    printSeedCatalogCsvLine(rogue.seed, rogue.depthLevel, 1, "altar", "resurrection altar",
                            NO_ENCHANTMENT_STRING, NO_RUNIC_STRING, vaultNumber, NO_OPENS_VAULT_STRING,
                            NO_CARRIED_BY_MONSTER_STRING, NO_ALLY_STATUS_STRING, NO_MUTATION_STRING);
                } else {
                    printf("        A resurrection altar (vault %s)\n", vaultNumber);
                }
            }
            // commutation altars come in pairs. we only want to print 1.
            if (pmap[i][j].layers[0] == COMMUTATION_ALTAR) {
                c_altars[pmap[i][j].machineNumber] = true;
            }
        }
    }
    for (i = 0; i < 50; i++) {
        if (c_altars[i]) {
            sprintf(vaultNumber, "%i", i);
            if (isCsvFormat) {
                printSeedCatalogCsvLine(rogue.seed, rogue.depthLevel, 1, "altar", "commutation altar",
                                        NO_ENCHANTMENT_STRING, NO_RUNIC_STRING, vaultNumber, NO_OPENS_VAULT_STRING,
                                        NO_CARRIED_BY_MONSTER_STRING, NO_ALLY_STATUS_STRING, NO_MUTATION_STRING);
            } else {
                printf("        A commutation altar (vault %s)\n",vaultNumber);
            }
        }
    }
}

int printSeedCatalog(uint64_t startingSeed, uint64_t numberOfSeedsToScan, unsigned int scanThroughDepth,
                     boolean isCsvFormat, char *errorMessage) {
    uint64_t theSeed;
    char message[1000] = "";
    rogue.nextGame = NG_NOTHING;

    initializeGameVariant();

    if (startingSeed == 0) {
        strncpy(errorMessage, "Starting seed must be 1+", ERROR_MESSAGE_LENGTH);
        return 1;
    }

    if (scanThroughDepth == 0 || scanThroughDepth > gameConst->deepestLevel) {
        snprintf(errorMessage, ERROR_MESSAGE_LENGTH, "Depth must be between 1 and %d", gameConst->deepestLevel);
        return 1;
    }

    sprintf(message, "Brogue seed catalog, seeds %llu to %llu, through depth %u.\n"
                     "Generated with %s. Dungeons unchanged since %s.\n\n"
                     "To play one of these seeds, select Play>New Seeded Game from the title screen.\n",
            (unsigned long long) startingSeed, (unsigned long long) startingSeed + numberOfSeedsToScan - 1, scanThroughDepth, gameConst->versionString,
            gameConst->dungeonVersionString);

    if (isCsvFormat) {
        fprintf(stderr, "%s", message);
        printf("%s\n",CSV_HEADER_STRING);
    } else {
        printf("%s", message);
    }

    for (theSeed = startingSeed; theSeed < startingSeed + numberOfSeedsToScan; theSeed++) {
        if (!isCsvFormat) {
            printf("Seed %llu:\n", (unsigned long long) theSeed);
        }
        fprintf(stderr, "Scanning seed %llu...\n", (unsigned long long) theSeed);
        rogue.nextGamePath[0] = '\0';
        randomNumbersGenerated = 0;

        rogue.playbackMode = false;
        rogue.playbackFastForward = false;
        rogue.playbackBetweenTurns = false;

        currentFilePath[0] = '\0';
        initializeRogue(theSeed);
        rogue.playbackOmniscience = true;
        for (rogue.depthLevel = 1; rogue.depthLevel <= scanThroughDepth; rogue.depthLevel++) {
            startLevel(rogue.depthLevel == 1 ? 1 : rogue.depthLevel - 1, 1); // descending into level n
            if (!isCsvFormat) {
                printf("    Depth %i:\n", rogue.depthLevel);
            }

            printSeedCatalogFloorItems(isCsvFormat);
            printSeedCatalogMonsterItems(isCsvFormat);
            printSeedCatalogMonsters(isCsvFormat, false); // captives and allies only
            if (rogue.depthLevel >= 13) { // resurrection & commutation altars can spawn starting on 13
                printSeedCatalogAltars(isCsvFormat);
            }
        }

        freeEverything();
    }
    return 0;
}
