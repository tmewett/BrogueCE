/*
 *  Items.c
 *  Brogue
 *
 *  Created by Brian Walker on 1/17/09.
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

item *initializeItem() {
    short i;
    item *theItem;

    theItem = (item *) malloc(sizeof(item));
    memset(theItem, '\0', sizeof(item) );

    theItem->category = 0;
    theItem->kind = 0;
    theItem->flags = 0;
    theItem->displayChar = '&';
    theItem->foreColor = &itemColor;
    theItem->inventoryColor = &white;
    theItem->inventoryLetter = '\0';
    theItem->armor = 0;
    theItem->strengthRequired = 0;
    theItem->enchant1 = 0;
    theItem->enchant2 = 0;
    theItem->timesEnchanted = 0;
    theItem->vorpalEnemy = 0;
    theItem->charges = 0;
    theItem->quantity = 1;
    theItem->quiverNumber = 0;
    theItem->originDepth = 0;
    theItem->inscription[0] = '\0';
    theItem->nextItem = NULL;

    for (i=0; i < KEY_ID_MAXIMUM; i++) {
        theItem->keyLoc[i].x = 0;
        theItem->keyLoc[i].y = 0;
        theItem->keyLoc[i].machine = 0;
        theItem->keyLoc[i].disposableHere = false;
    }
    return theItem;
}

// Allocates space, generates a specified item (or random category/kind if -1)
// and returns a pointer to that item. The item is not given a location here
// and is not inserted into the item chain!
item *generateItem(unsigned short theCategory, short theKind) {
    item *theItem = initializeItem();
    makeItemInto(theItem, theCategory, theKind);
    return theItem;
}

unsigned long pickItemCategory(unsigned long theCategory) {
    short i, sum, randIndex;
    short probabilities[13] =                       {50,    42,     52,     3,      3,      10,     8,      2,      3,      2,        0,        0,      0};
    unsigned short correspondingCategories[13] =    {GOLD,  SCROLL, POTION, STAFF,  WAND,   WEAPON, ARMOR,  FOOD,   RING,   CHARM,    AMULET,   GEM,    KEY};

    sum = 0;

    for (i=0; i<13; i++) {
        if (theCategory <= 0 || theCategory & correspondingCategories[i]) {
            sum += probabilities[i];
        }
    }

    if (sum == 0) {
        return theCategory; // e.g. when you pass in AMULET or GEM, since they have no frequency
    }

    randIndex = rand_range(1, sum);

    for (i=0; ; i++) {
        if (theCategory <= 0 || theCategory & correspondingCategories[i]) {
            if (randIndex <= probabilities[i]) {
                return correspondingCategories[i];
            }
            randIndex -= probabilities[i];
        }
    }
}

// Sets an item to the given type and category (or chooses randomly if -1) with all other stats
item *makeItemInto(item *theItem, unsigned long itemCategory, short itemKind) {
    itemTable *theEntry = NULL;

    if (itemCategory <= 0) {
        itemCategory = ALL_ITEMS;
    }

    itemCategory = pickItemCategory(itemCategory);

    theItem->category = itemCategory;

    switch (itemCategory) {

        case FOOD:
            if (itemKind < 0) {
                itemKind = chooseKind(foodTable, NUMBER_FOOD_KINDS);
            }
            theEntry = &foodTable[itemKind];
            theItem->displayChar = G_FOOD;
            theItem->flags |= ITEM_IDENTIFIED;
            break;

        case WEAPON:
            if (itemKind < 0) {
                itemKind = chooseKind(weaponTable, NUMBER_WEAPON_KINDS);
            }
            theEntry = &weaponTable[itemKind];
            theItem->damage = weaponTable[itemKind].range;
            theItem->strengthRequired = weaponTable[itemKind].strengthRequired;
            theItem->displayChar = G_WEAPON;

            switch (itemKind) {
                case DAGGER:
                    theItem->flags |= ITEM_SNEAK_ATTACK_BONUS;
                    break;
                case MACE:
                case HAMMER:
                    theItem->flags |= ITEM_ATTACKS_STAGGER;
                    break;
                case WHIP:
                    theItem->flags |= ITEM_ATTACKS_EXTEND;
                    break;
                case RAPIER:
                    theItem->flags |= (ITEM_ATTACKS_QUICKLY | ITEM_LUNGE_ATTACKS);
                    break;
                case FLAIL:
                    theItem->flags |= ITEM_PASS_ATTACKS;
                    break;
                case SPEAR:
                case PIKE:
                    theItem->flags |= ITEM_ATTACKS_PENETRATE;
                    break;
                case AXE:
                case WAR_AXE:
                    theItem->flags |= ITEM_ATTACKS_ALL_ADJACENT;
                    break;
                default:
                    break;
            }

            if (rand_percent(40)) {
                theItem->enchant1 += rand_range(1, 3);
                if (rand_percent(50)) {
                    // cursed
                    theItem->enchant1 *= -1;
                    theItem->flags |= ITEM_CURSED;
                    if (rand_percent(33)) { // give it a bad runic
                        theItem->enchant2 = rand_range(NUMBER_GOOD_WEAPON_ENCHANT_KINDS, NUMBER_WEAPON_RUNIC_KINDS - 1);
                        theItem->flags |= ITEM_RUNIC;
                    }
                } else if (rand_range(3, 10)
                           * ((theItem->flags & ITEM_ATTACKS_STAGGER) ? 2 : 1)
                           / ((theItem->flags & ITEM_ATTACKS_QUICKLY) ? 2 : 1)
                           / ((theItem->flags & ITEM_ATTACKS_EXTEND) ? 2 : 1)
                           > theItem->damage.lowerBound) {
                    // give it a good runic; lower damage items are more likely to be runic
                    theItem->enchant2 = rand_range(0, NUMBER_GOOD_WEAPON_ENCHANT_KINDS - 1);
                    theItem->flags |= ITEM_RUNIC;
                    if (theItem->enchant2 == W_SLAYING) {
                        theItem->vorpalEnemy = chooseVorpalEnemy();
                    }
                } else {
                    while (rand_percent(10)) {
                        theItem->enchant1++;
                    }
                }
            }
            if (itemKind == DART || itemKind == INCENDIARY_DART || itemKind == JAVELIN) {
                if (itemKind == INCENDIARY_DART) {
                    theItem->quantity = rand_range(3, 6);
                } else {
                    theItem->quantity = rand_range(5, 18);
                }
                theItem->quiverNumber = rand_range(1, 60000);
                theItem->flags &= ~(ITEM_CURSED | ITEM_RUNIC); // throwing weapons can't be cursed or runic
                theItem->enchant1 = 0; // throwing weapons can't be magical
            }
            theItem->charges = WEAPON_KILLS_TO_AUTO_ID; // kill 20 enemies to auto-identify
            break;

        case ARMOR:
            if (itemKind < 0) {
                itemKind = chooseKind(armorTable, NUMBER_ARMOR_KINDS);
            }
            theEntry = &armorTable[itemKind];
            theItem->armor = randClump(armorTable[itemKind].range);
            theItem->strengthRequired = armorTable[itemKind].strengthRequired;
            theItem->displayChar = G_ARMOR;
            theItem->charges = ARMOR_DELAY_TO_AUTO_ID; // this many turns until it reveals its enchants and whether runic
            if (rand_percent(40)) {
                theItem->enchant1 += rand_range(1, 3);
                if (rand_percent(50)) {
                    // cursed
                    theItem->enchant1 *= -1;
                    theItem->flags |= ITEM_CURSED;
                    if (rand_percent(33)) { // give it a bad runic
                        theItem->enchant2 = rand_range(NUMBER_GOOD_ARMOR_ENCHANT_KINDS, NUMBER_ARMOR_ENCHANT_KINDS - 1);
                        theItem->flags |= ITEM_RUNIC;
                    }
                } else if (rand_range(0, 95) > theItem->armor) { // give it a good runic
                    theItem->enchant2 = rand_range(0, NUMBER_GOOD_ARMOR_ENCHANT_KINDS - 1);
                    theItem->flags |= ITEM_RUNIC;
                    if (theItem->enchant2 == A_IMMUNITY) {
                        theItem->vorpalEnemy = chooseVorpalEnemy();
                    }
                } else {
                    while (rand_percent(10)) {
                        theItem->enchant1++;
                    }
                }
            }
            break;
        case SCROLL:
            if (itemKind < 0) {
                itemKind = chooseKind(scrollTable, NUMBER_SCROLL_KINDS);
            }
            theEntry = &scrollTable[itemKind];
            theItem->displayChar = G_SCROLL;
            theItem->flags |= ITEM_FLAMMABLE;
            break;
        case POTION:
            if (itemKind < 0) {
                itemKind = chooseKind(potionTable, NUMBER_POTION_KINDS);
            }
            theEntry = &potionTable[itemKind];
            theItem->displayChar = G_POTION;
            break;
        case STAFF:
            if (itemKind < 0) {
                itemKind = chooseKind(staffTable, NUMBER_STAFF_KINDS);
            }
            theEntry = &staffTable[itemKind];
            theItem->displayChar = G_STAFF;
            theItem->charges = 2;
            if (rand_percent(50)) {
                theItem->charges++;
                if (rand_percent(15)) {
                    theItem->charges++;
                    while (rand_percent(10)) {
                        theItem->charges++;
                    }
                }
            }
            theItem->enchant1 = theItem->charges;
            theItem->enchant2 = (itemKind == STAFF_BLINKING || itemKind == STAFF_OBSTRUCTION ? 1000 : 500); // start with no recharging mojo
            break;
        case WAND:
            if (itemKind < 0) {
                itemKind = chooseKind(wandTable, NUMBER_WAND_KINDS);
            }
            theEntry = &wandTable[itemKind];
            theItem->displayChar = G_WAND;
            theItem->charges = randClump(wandTable[itemKind].range);
            break;
        case RING:
            if (itemKind < 0) {
                itemKind = chooseKind(ringTable, NUMBER_RING_KINDS);
            }
            theEntry = &ringTable[itemKind];
            theItem->displayChar = G_RING;
            theItem->enchant1 = randClump(ringTable[itemKind].range);
            theItem->charges = RING_DELAY_TO_AUTO_ID; // how many turns of being worn until it auto-identifies
            if (rand_percent(16)) {
                // cursed
                theItem->enchant1 *= -1;
                theItem->flags |= ITEM_CURSED;
            } else {
                while (rand_percent(10)) {
                    theItem->enchant1++;
                }
            }
            break;
        case CHARM:
            if (itemKind < 0) {
                itemKind = chooseKind(charmTable, NUMBER_CHARM_KINDS);
            }
            theItem->displayChar = G_CHARM;
            theItem->charges = 0; // Charms are initially ready for use.
            theItem->enchant1 = randClump(charmTable[itemKind].range);
            while (rand_percent(7)) {
                theItem->enchant1++;
            }
            theItem->flags |= ITEM_IDENTIFIED;
            break;
        case GOLD:
            theEntry = NULL;
            theItem->displayChar = G_GOLD;
            theItem->quantity = rand_range(50 + rogue.depthLevel * 10, 100 + rogue.depthLevel * 15);
            break;
        case AMULET:
            theEntry = NULL;
            theItem->displayChar = G_AMULET;
            itemKind = 0;
            theItem->flags |= ITEM_IDENTIFIED;
            break;
        case GEM:
            theEntry = NULL;
            theItem->displayChar = G_GEM;
            itemKind = 0;
            theItem->flags |= ITEM_IDENTIFIED;
            break;
        case KEY:
            theEntry = NULL;
            theItem->displayChar = G_KEY;
            theItem->flags |= ITEM_IDENTIFIED;
            break;
        default:
            theEntry = NULL;
            message("something has gone terribly wrong!", true);
            break;
    }
    if (theItem
        && !(theItem->flags & ITEM_IDENTIFIED)
        && (!(theItem->category & (POTION | SCROLL) ) || (theEntry && !theEntry->identified))) {

        theItem->flags |= ITEM_CAN_BE_IDENTIFIED;
    }
    theItem->kind = itemKind;

    return theItem;
}

short chooseKind(itemTable *theTable, short numKinds) {
    short i, totalFrequencies = 0, randomFrequency;
    for (i=0; i<numKinds; i++) {
        totalFrequencies += max(0, theTable[i].frequency);
    }
    randomFrequency = rand_range(1, totalFrequencies);
    for (i=0; randomFrequency > theTable[i].frequency; i++) {
        randomFrequency -= max(0, theTable[i].frequency);
    }
    return i;
}

// Places an item at (x,y) if provided or else a random location if they're 0. Inserts item into the floor list.
item *placeItem(item *theItem, short x, short y) {
    short loc[2];
    enum dungeonLayers layer;
    char theItemName[DCOLS], buf[DCOLS];
    if (x <= 0 || y <= 0) {
        randomMatchingLocation(&(loc[0]), &(loc[1]), FLOOR, NOTHING, -1);
        theItem->xLoc = loc[0];
        theItem->yLoc = loc[1];
    } else {
        theItem->xLoc = x;
        theItem->yLoc = y;
    }

    removeItemFromChain(theItem, floorItems); // just in case; double-placing an item will result in game-crashing loops in the item list
    addItemToChain(theItem, floorItems);
    pmap[theItem->xLoc][theItem->yLoc].flags |= HAS_ITEM;
    if ((theItem->flags & ITEM_MAGIC_DETECTED) && itemMagicPolarity(theItem)) {
        pmap[theItem->xLoc][theItem->yLoc].flags |= ITEM_DETECTED;
    }
    if (cellHasTerrainFlag(x, y, T_IS_DF_TRAP)
        && !cellHasTerrainFlag(x, y, T_MOVES_ITEMS)
        && !(pmap[x][y].flags & PRESSURE_PLATE_DEPRESSED)) {

        pmap[x][y].flags |= PRESSURE_PLATE_DEPRESSED;
        if (playerCanSee(x, y)) {
            if (cellHasTMFlag(x, y, TM_IS_SECRET)) {
                discover(x, y);
                refreshDungeonCell(x, y);
            }
            itemName(theItem, theItemName, false, false, NULL);
            sprintf(buf, "a pressure plate clicks underneath the %s!", theItemName);
            message(buf, true);
        }
        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmap[x][y].layers[layer]].flags & T_IS_DF_TRAP) {
                spawnDungeonFeature(x, y, &(dungeonFeatureCatalog[tileCatalog[pmap[x][y].layers[layer]].fireType]), true, false);
                promoteTile(x, y, layer, false);
            }
        }
    }
    return theItem;
}

void fillItemSpawnHeatMap(unsigned short heatMap[DCOLS][DROWS], unsigned short heatLevel, short x, short y) {
    enum directions dir;
    short newX, newY;

    if (pmap[x][y].layers[DUNGEON] == DOOR) {
        heatLevel += 10;
    } else if (pmap[x][y].layers[DUNGEON] == SECRET_DOOR) {
        heatLevel += 3000;
    }
    if (heatMap[x][y] > heatLevel) {
        heatMap[x][y] = heatLevel;
    }
    for (dir = 0; dir < 4; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];
        if (coordinatesAreInMap(newX, newY)
            && !cellHasTerrainFlag(newX, newY, T_IS_DEEP_WATER | T_LAVA_INSTA_DEATH | T_AUTO_DESCENT)
            && isPassableOrSecretDoor(newX, newY)
            && heatLevel < heatMap[newX][newY]) {

            fillItemSpawnHeatMap(heatMap, heatLevel, newX, newY);
        }
    }
}

void coolHeatMapAt(unsigned short heatMap[DCOLS][DROWS], short x, short y, unsigned long *totalHeat) {
    short k, l;
    unsigned short currentHeat;

    currentHeat = heatMap[x][y];
    if (currentHeat == 0) {
        return;
    }
    *totalHeat -= heatMap[x][y];
    heatMap[x][y] = 0;

    // lower the heat near the chosen location
    for (k = -5; k <= 5; k++) {
        for (l = -5; l <= 5; l++) {
            if (coordinatesAreInMap(x+k, y+l) && heatMap[x+k][y+l] == currentHeat) {
                heatMap[x+k][y+l] = max(1, heatMap[x+k][y+l]/10);
                *totalHeat -= (currentHeat - heatMap[x+k][y+l]);
            }
        }
    }
}

// Returns false if no place could be found.
// That should happen only if the total heat is zero.
boolean getItemSpawnLoc(unsigned short heatMap[DCOLS][DROWS], short *x, short *y, unsigned long *totalHeat) {
    unsigned long randIndex;
    unsigned short currentHeat;
    short i, j;

    if (*totalHeat <= 0) {
        return false;
    }

    randIndex = rand_range(1, *totalHeat);

    //printf("\nrandIndex: %i", randIndex);

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            currentHeat = heatMap[i][j];
            if (randIndex <= currentHeat) { // this is the spot!
                *x = i;
                *y = j;
                return true;
            }
            randIndex -= currentHeat;
        }
    }
    brogueAssert(0); // should never get here!
    return false;
}

// Generates and places items for the level. Must pass the location of the up-stairway on the level.
void populateItems(short upstairsX, short upstairsY) {
    if (!ITEMS_ENABLED) {
        return;
    }
    item *theItem;
    unsigned short itemSpawnHeatMap[DCOLS][DROWS];
    short i, j, numberOfItems, numberOfGoldPiles, goldBonusProbability, x = 0, y = 0;
    unsigned long totalHeat;
    short theCategory, theKind, randomDepthOffset = 0;

    const int POW_GOLD[] = {
        // b^3.05, with b from 0 to 25:
        0, 1, 8, 28, 68, 135, 236, 378, 568, 813, 1122, 1500, 1956, 2497, 3131,
        3864, 4705, 5660, 6738, 7946, 9292, 10783, 12427, 14232, 16204, 18353};
#define aggregateGoldLowerBound(d)  (POW_GOLD[d] + 320 * (d))
#define aggregateGoldUpperBound(d)  (POW_GOLD[d] + 420 * (d))
    const fixpt POW_FOOD[] = {
        // b^1.35 fixed point, with b from 1 to 50 (for future-proofing):
        65536, 167059, 288797, 425854, 575558, 736180, 906488, 1085553, 1272645,
        1467168, 1668630, 1876612, 2090756, 2310749, 2536314, 2767208, 3003211,
        3244126, 3489773, 3739989, 3994624, 4253540, 4516609, 4783712, 5054741,
        5329591, 5608167, 5890379, 6176141, 6465373, 6758000, 7053950, 7353155,
        7655551, 7961076, 8269672, 8581283, 8895856, 9213341, 9533687, 9856849,
        10182782, 10511443, 10842789, 11176783, 11513384, 11852556, 12194264,
        12538472, 12885148};

#ifdef AUDIT_RNG
    char RNGmessage[100];
#endif

    if (rogue.depthLevel > AMULET_LEVEL) {
        if (rogue.depthLevel - AMULET_LEVEL - 1 >= 8) {
            numberOfItems = 1;
        } else {
            const short lumenstoneDistribution[8] = {3, 3, 3, 2, 2, 2, 2, 2};
            numberOfItems = lumenstoneDistribution[rogue.depthLevel - AMULET_LEVEL - 1];
        }
        numberOfGoldPiles = 0;
    } else {
        rogue.lifePotionFrequency += 34;
        rogue.strengthPotionFrequency += 17;
        rogue.enchantScrollFrequency += 30;
        numberOfItems = 3;
        while (rand_percent(60)) {
            numberOfItems++;
        }
        if (rogue.depthLevel <= 2) {
            numberOfItems += 2; // 4 extra items to kickstart your career as a rogue
        } else if (rogue.depthLevel <= 4) {
            numberOfItems++; // and 2 more here
        }

        numberOfGoldPiles = min(5, rogue.depthLevel / 4);
        for (goldBonusProbability = 60;
             rand_percent(goldBonusProbability) && numberOfGoldPiles <= 10;
             goldBonusProbability -= 15) {

            numberOfGoldPiles++;
        }
        // Adjust the amount of gold if we're past depth 5 and we were below or above
        // the production schedule as of the previous depth.
        if (rogue.depthLevel > 5) {
            if (rogue.goldGenerated < aggregateGoldLowerBound(rogue.depthLevel - 1)) {
                numberOfGoldPiles += 2;
            } else if (rogue.goldGenerated > aggregateGoldUpperBound(rogue.depthLevel - 1)) {
                numberOfGoldPiles -= 2;
            }
        }
    }

    // Create an item spawn heat map to bias item generation behind secret doors (and, to a lesser
    // extent, regular doors). This is in terms of the number of secret/regular doors that must be
    // passed to reach the area when pathing to it from the upward staircase.
    // This is why there are often several items in well hidden secret rooms. Otherwise,
    // those rooms are usually empty, which is demoralizing after you take the trouble to find them.
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            itemSpawnHeatMap[i][j] = 50000;
        }
    }
    fillItemSpawnHeatMap(itemSpawnHeatMap, 5, upstairsX, upstairsY);
    totalHeat = 0;

#ifdef AUDIT_RNG
    sprintf(RNGmessage, "\n\nInitial heat map for level %i:\n", rogue.currentTurnNumber);
    RNGLog(RNGmessage);
#endif

    for (j=0; j<DROWS; j++) {
        for (i=0; i<DCOLS; i++) {
            if (cellHasTerrainFlag(i, j, T_OBSTRUCTS_ITEMS | T_PATHING_BLOCKER)
                || (pmap[i][j].flags & (IS_CHOKEPOINT | IN_LOOP | IS_IN_MACHINE))
                || passableArcCount(i, j) > 1) { // Not in walls, hallways, quest rooms, loops or chokepoints, please.

                itemSpawnHeatMap[i][j] = 0;
            } else if (itemSpawnHeatMap[i][j] == 50000) {
                itemSpawnHeatMap[i][j] = 0;
                pmap[i][j].layers[DUNGEON] = WALL; // due to a bug that created occasional isolated one-cell islands;
                                                   // not sure if it's still around, but this is a good-enough failsafe
            }
#ifdef AUDIT_RNG
            sprintf(RNGmessage, "%u%s%s\t%s",
                    itemSpawnHeatMap[i][j],
                    ((pmap[i][j].flags & IS_CHOKEPOINT) ? " (C)": ""), // chokepoint
                    ((pmap[i][j].flags & IN_LOOP) ? " (L)": ""), // loop
                    (i == DCOLS-1 ? "\n" : ""));
            RNGLog(RNGmessage);
#endif
            totalHeat += itemSpawnHeatMap[i][j];
        }
    }

    if (D_INSPECT_LEVELGEN) {
        short **map = allocGrid();
        for (i=0; i<DCOLS; i++) {
            for (j=0; j<DROWS; j++) {
                map[i][j] = itemSpawnHeatMap[i][j] * -1;
            }
        }
        dumpLevelToScreen();
        displayGrid(map);
        freeGrid(map);
        temporaryMessage("Item spawn heat map:", true);
    }

    if (rogue.depthLevel > 2) {
        // Include a random factor in food and potion of life generation to make things slightly less predictable.
        randomDepthOffset = rand_range(-1, 1);
        randomDepthOffset += rand_range(-1, 1);
    }

    for (i=0; i<numberOfItems; i++) {
        theCategory = ALL_ITEMS & ~GOLD; // gold is placed separately, below, so it's not a punishment
        theKind = -1;

        scrollTable[SCROLL_ENCHANTING].frequency = rogue.enchantScrollFrequency;
        potionTable[POTION_STRENGTH].frequency = rogue.strengthPotionFrequency;
        potionTable[POTION_LIFE].frequency = rogue.lifePotionFrequency;

        // Adjust the desired item category if necessary.
        if ((rogue.foodSpawned + foodTable[RATION].strengthRequired / 3) * 4 * FP_FACTOR
            <= (POW_FOOD[rogue.depthLevel-1] + (randomDepthOffset * FP_FACTOR)) * foodTable[RATION].strengthRequired * 45/100) {
            // Guarantee a certain nutrition minimum of the approximate equivalent of one ration every four levels,
            // with more food on deeper levels since they generally take more turns to complete.
            theCategory = FOOD;
            if (rogue.depthLevel > AMULET_LEVEL) {
                numberOfItems++; // Food isn't at the expense of lumenstones.
            }
        } else if (rogue.depthLevel > AMULET_LEVEL) {
            theCategory = GEM;
        } else if (rogue.lifePotionsSpawned * 4 + 3 < rogue.depthLevel + randomDepthOffset) {
            theCategory = POTION;
            theKind = POTION_LIFE;
        }

        // Generate the item.
        theItem = generateItem(theCategory, theKind);
        theItem->originDepth = rogue.depthLevel;

        if (theItem->category & FOOD) {
            rogue.foodSpawned += foodTable[theItem->kind].strengthRequired;
            if (D_MESSAGE_ITEM_GENERATION) printf("\n(:)  Depth %i: generated food", rogue.depthLevel);
        }

        // Choose a placement location.
        if ((theItem->category & FOOD) || ((theItem->category & POTION) && theItem->kind == POTION_STRENGTH)) {
            do {
                randomMatchingLocation(&x, &y, FLOOR, NOTHING, -1); // Food and gain strength don't follow the heat map.
            } while (passableArcCount(x, y) > 1); // Not in a hallway.
        } else {
            getItemSpawnLoc(itemSpawnHeatMap, &x, &y, &totalHeat);
        }
        brogueAssert(coordinatesAreInMap(x, y));
        // Cool off the item spawning heat map at the chosen location:
        coolHeatMapAt(itemSpawnHeatMap, x, y, &totalHeat);

        // Regulate the frequency of enchantment scrolls and strength/life potions.
        if ((theItem->category & SCROLL) && theItem->kind == SCROLL_ENCHANTING) {
            rogue.enchantScrollFrequency -= 50;
            if (D_MESSAGE_ITEM_GENERATION) printf("\n(?)  Depth %i: generated an enchant scroll at %i frequency", rogue.depthLevel, rogue.enchantScrollFrequency);
        } else if (theItem->category & POTION && theItem->kind == POTION_LIFE) {
            if (D_MESSAGE_ITEM_GENERATION) printf("\n(!l) Depth %i: generated a life potion at %i frequency", rogue.depthLevel, rogue.lifePotionFrequency);
            rogue.lifePotionFrequency -= 150;
            rogue.lifePotionsSpawned++;
        } else if (theItem->category & POTION && theItem->kind == POTION_STRENGTH) {
            if (D_MESSAGE_ITEM_GENERATION) printf("\n(!s) Depth %i: generated a strength potion at %i frequency", rogue.depthLevel, rogue.strengthPotionFrequency);
            rogue.strengthPotionFrequency -= 50;
        }

        // Place the item.
        placeItem(theItem, x, y); // Random valid location already obtained according to heat map.
        brogueAssert(!cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY));

        if (D_INSPECT_LEVELGEN) {
            short **map = allocGrid();
            short i2, j2;
            for (i2=0; i2<DCOLS; i2++) {
                for (j2=0; j2<DROWS; j2++) {
                    map[i2][j2] = itemSpawnHeatMap[i2][j2] * -1;
                }
            }
            dumpLevelToScreen();
            displayGrid(map);
            freeGrid(map);
            plotCharWithColor(theItem->displayChar, mapToWindowX(x), mapToWindowY(y), &black, &purple);
            temporaryMessage("Added an item.", true);
        }
    }

    // Now generate gold.
    for (i=0; i<numberOfGoldPiles; i++) {
        theItem = generateItem(GOLD, -1);
        getItemSpawnLoc(itemSpawnHeatMap, &x, &y, &totalHeat);
        coolHeatMapAt(itemSpawnHeatMap, x, y, &totalHeat);
        placeItem(theItem, x, y);
        rogue.goldGenerated += theItem->quantity;
    }

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Added gold.", true);
    }

    scrollTable[SCROLL_ENCHANTING].frequency    = 0;    // No enchant scrolls or strength/life potions can spawn except via initial
    potionTable[POTION_STRENGTH].frequency      = 0;    // item population or blueprints that create them specifically.
    potionTable[POTION_LIFE].frequency          = 0;

    if (D_MESSAGE_ITEM_GENERATION) printf("\n---- Depth %i: %lu gold generated so far.", rogue.depthLevel, rogue.goldGenerated);
}

// Name of this function is a bit misleading -- basically returns true iff the item will stack without consuming an extra slot
// i.e. if it's a throwing weapon with a sibling already in your pack. False for potions and scrolls.
boolean itemWillStackWithPack(item *theItem) {
    item *tempItem;
    if (theItem->category & GEM) {
        for (tempItem = packItems->nextItem;
             tempItem != NULL && !((tempItem->category & GEM) && theItem->originDepth == tempItem->originDepth);
             tempItem = tempItem->nextItem);
        return (tempItem ? true : false);
    } else if (!(theItem->quiverNumber)) {
        return false;
    } else {
        for (tempItem = packItems->nextItem;
             tempItem != NULL && tempItem->quiverNumber != theItem->quiverNumber;
             tempItem = tempItem->nextItem);
        return (tempItem ? true : false);
    }
}

void removeItemFrom(short x, short y) {
    short layer;

    pmap[x][y].flags &= ~HAS_ITEM;

    if (cellHasTMFlag(x, y, TM_PROMOTES_ON_ITEM_PICKUP)) {
        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_PROMOTES_ON_ITEM_PICKUP) {
                promoteTile(x, y, layer, false);
            }
        }
    }
}

// adds the item at (x,y) to the pack
void pickUpItemAt(short x, short y) {
    item *theItem;
    creature *monst;
    char buf[COLS * 3], buf2[COLS * 3];
    short guardianX, guardianY;

    rogue.disturbed = true;

    // find the item
    theItem = itemAtLoc(x, y);

    if (!theItem) {
        message("Error: Expected item; item not found.", true);
        return;
    }

    if ((theItem->flags & ITEM_KIND_AUTO_ID)
        && tableForItemCategory(theItem->category, NULL)
        && !(tableForItemCategory(theItem->category, NULL)[theItem->kind].identified)) {

        identifyItemKind(theItem);
    }

    if ((theItem->category & WAND)
        && wandTable[theItem->kind].identified
        && wandTable[theItem->kind].range.lowerBound == wandTable[theItem->kind].range.upperBound) {

        theItem->flags |= ITEM_IDENTIFIED;
    }

    if (numberOfItemsInPack() < MAX_PACK_ITEMS || (theItem->category & GOLD) || itemWillStackWithPack(theItem)) {
        // remove from floor chain
        pmap[x][y].flags &= ~ITEM_DETECTED;

        if (!removeItemFromChain(theItem, floorItems)) {
            brogueAssert(false);
        }

        if (theItem->category & GOLD) {
            rogue.gold += theItem->quantity;
            sprintf(buf, "you found %i pieces of gold.", theItem->quantity);
            messageWithColor(buf, &itemMessageColor, false);
            deleteItem(theItem);
            removeItemFrom(x, y); // triggers tiles with T_PROMOTES_ON_ITEM_PICKUP
            return;
        }

        if ((theItem->category & AMULET) && numberOfMatchingPackItems(AMULET, 0, 0, false)) {
            message("you already have the Amulet of Yendor.", false);
            deleteItem(theItem);
            return;
        }

        theItem = addItemToPack(theItem);

        itemName(theItem, buf2, true, true, NULL); // include suffix, article

        sprintf(buf, "you now have %s (%c).", buf2, theItem->inventoryLetter);
        messageWithColor(buf, &itemMessageColor, false);

        removeItemFrom(x, y); // triggers tiles with T_PROMOTES_ON_ITEM_PICKUP

        if ((theItem->category & AMULET)
            && !(rogue.yendorWarden)) {
            // Identify the amulet guardian, or generate one if there isn't one.
            for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
                if (monst->info.monsterID == MK_WARDEN_OF_YENDOR) {
                    rogue.yendorWarden = monst;
                    break;
                }
            }
            if (!rogue.yendorWarden) {
                getRandomMonsterSpawnLocation(&guardianX, &guardianY);
                monst = generateMonster(MK_WARDEN_OF_YENDOR, false, false);
                monst->xLoc = guardianX;
                monst->yLoc = guardianY;
                pmap[guardianX][guardianY].flags |= HAS_MONSTER;
                rogue.yendorWarden = monst;
            }
        }
    } else {
        theItem->flags |= ITEM_PLAYER_AVOIDS; // explore shouldn't try to pick it up more than once.
        itemName(theItem, buf2, false, true, NULL); // include article
        sprintf(buf, "Your pack is too full to pick up %s.", buf2);
        message(buf, false);
    }
}

void conflateItemCharacteristics(item *newItem, item *oldItem) {

    // let magic detection and other flags propagate to the new stack...
    newItem->flags |= (oldItem->flags & (ITEM_MAGIC_DETECTED | ITEM_IDENTIFIED | ITEM_PROTECTED | ITEM_RUNIC
                                         | ITEM_RUNIC_HINTED | ITEM_CAN_BE_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN));

    // keep the higher enchantment and lower strength requirement...
    if (oldItem->enchant1 > newItem->enchant1) {
        newItem->enchant1 = oldItem->enchant1;
    }
    if (oldItem->strengthRequired < newItem->strengthRequired) {
        newItem->strengthRequired = oldItem->strengthRequired;
    }
    // Keep track of origin depth only if every item in the stack has the same origin depth.
    if (oldItem->originDepth <= 0 || newItem->originDepth != oldItem->originDepth) {
        newItem->originDepth = 0;
    }
}

void stackItems(item *newItem, item *oldItem) {
    //Increment the quantity of the old item...
    newItem->quantity += oldItem->quantity;

    // ...conflate attributes...
    conflateItemCharacteristics(newItem, oldItem);

    // ...and delete the new item.
    deleteItem(oldItem);
}

boolean inventoryLetterAvailable(char proposedLetter) {
    item *theItem;
    if (proposedLetter >= 'a'
        && proposedLetter <= 'z') {

        for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
            if (theItem->inventoryLetter == proposedLetter) {
                return false;
            }
        }
        return true;
    }
    return false;
}

item *addItemToPack(item *theItem) {
    item *previousItem, *tempItem;
    char itemLetter;

    // Can the item stack with another in the inventory?
    if (theItem->category & (FOOD|POTION|SCROLL|GEM)) {
        for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
            if (theItem->category == tempItem->category
                && theItem->kind == tempItem->kind
                && (!(theItem->category & GEM) || theItem->originDepth == tempItem->originDepth)) {

                // We found a match!
                stackItems(tempItem, theItem);

                // Pass back the incremented (old) item. No need to add it to the pack since it's already there.
                return tempItem;
            }
        }
    } else if (theItem->category & WEAPON && theItem->quiverNumber > 0) {
        for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
            if (theItem->category == tempItem->category && theItem->kind == tempItem->kind
                && theItem->quiverNumber == tempItem->quiverNumber) {
                // We found a match!
                stackItems(tempItem, theItem);

                // Pass back the incremented (old) item. No need to add it to the pack since it's already there.
                return tempItem;
            }
        }
    }

    // assign a reference letter to the item
    if (!inventoryLetterAvailable(theItem->inventoryLetter)) {
        itemLetter = nextAvailableInventoryCharacter();
        if (itemLetter) {
            theItem->inventoryLetter = itemLetter;
        }
    }

    // insert at proper place in pack chain
    for (previousItem = packItems;
         previousItem->nextItem != NULL && previousItem->nextItem->category <= theItem->category;
         previousItem = previousItem->nextItem);
    theItem->nextItem = previousItem->nextItem;
    previousItem->nextItem = theItem;

    return theItem;
}

short numberOfItemsInPack() {
    short theCount = 0;
    item *theItem;
    for(theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        theCount += (theItem->category & (WEAPON | GEM) ? 1 : theItem->quantity);
    }
    return theCount;
}

char nextAvailableInventoryCharacter() {
    boolean charTaken[26];
    short i;
    item *theItem;
    char c;
    for(i=0; i<26; i++) {
        charTaken[i] = false;
    }
    for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        c = theItem->inventoryLetter;
        if (c >= 'a' && c <= 'z') {
            charTaken[c - 'a'] = true;
        }
    }
    for(i=0; i<26; i++) {
        if (!charTaken[i]) {
            return ('a' + i);
        }
    }
    return 0;
}

void checkForDisenchantment(item *theItem) {
    char buf[COLS], buf2[COLS];

    if ((theItem->flags & ITEM_RUNIC)
        && (((theItem->category & WEAPON) && theItem->enchant2 < NUMBER_GOOD_WEAPON_ENCHANT_KINDS) || ((theItem->category & ARMOR) && theItem->enchant2 < NUMBER_GOOD_ARMOR_ENCHANT_KINDS))
        && theItem->enchant1 <= 0) {

        theItem->enchant2 = 0;
        theItem->flags &= ~(ITEM_RUNIC | ITEM_RUNIC_HINTED | ITEM_RUNIC_IDENTIFIED);

        if (theItem->flags & ITEM_IDENTIFIED) {
            identify(theItem);
            itemName(theItem, buf2, false, false, NULL);
            sprintf(buf, "the runes fade from your %s.", buf2);
            messageWithColor(buf, &itemMessageColor, false);
        }
    }
    if (theItem->flags & ITEM_CURSED
        && theItem->enchant1 >= 0) {

        theItem->flags &= ~ITEM_CURSED;
    }
}

boolean itemIsSwappable(const item *theItem) {
    if ((theItem->category & CAN_BE_SWAPPED)
        && theItem->quiverNumber == 0) {

        return true;
    } else {
        return false;
    }
}

void swapItemToEnchantLevel(item *theItem, short newEnchant, boolean enchantmentKnown) {
    short x, y, charmPercent;
    char buf1[COLS * 3], buf2[COLS * 3];

    if ((theItem->category & STAFF) && newEnchant < 2
        || (theItem->category & CHARM) && newEnchant < 1
        || (theItem->category & WAND) && newEnchant < 0) {

        itemName(theItem, buf1, false, true, NULL);
        sprintf(buf2, "%s shatter%s from the strain!",
                buf1,
                theItem->quantity == 1 ? "s" : "");
        x = theItem->xLoc;
        y = theItem->yLoc;
        removeItemFromChain(theItem, floorItems);
        pmap[x][y].flags &= ~(HAS_ITEM | ITEM_DETECTED);
        if (pmap[x][y].flags & (ANY_KIND_OF_VISIBLE | DISCOVERED | ITEM_DETECTED)) {
            refreshDungeonCell(x, y);
        }
        if (playerCanSee(x, y)) {
            messageWithColor(buf2, &itemMessageColor, false);
        }
    } else {
        if ((theItem->category & STAFF)
            && theItem->charges > newEnchant) {

            theItem->charges = newEnchant;
        }
        if (theItem->category & CHARM) {
            charmPercent = theItem->charges * 100 / charmRechargeDelay(theItem->kind, theItem->enchant1);
            theItem->charges = charmPercent * charmRechargeDelay(theItem->kind, newEnchant) / 100;
        }
        if (enchantmentKnown) {
            if (theItem->category & STAFF) {
                theItem->flags |= ITEM_MAX_CHARGES_KNOWN;
            }
            theItem->flags |= ITEM_IDENTIFIED;
        } else {
            theItem->flags &= ~(ITEM_MAX_CHARGES_KNOWN | ITEM_IDENTIFIED);
            theItem->flags |= ITEM_CAN_BE_IDENTIFIED;
            if (theItem->category & WEAPON) {
                theItem->charges = WEAPON_KILLS_TO_AUTO_ID; // kill this many enemies to auto-identify
            } else if (theItem->category & ARMOR) {
                theItem->charges = ARMOR_DELAY_TO_AUTO_ID; // this many turns until it reveals its enchants and whether runic
            } else if (theItem->category & RING) {
                theItem->charges = RING_DELAY_TO_AUTO_ID; // how many turns of being worn until it auto-identifies
            }
        }
        if (theItem->category & WAND) {
            theItem->charges = newEnchant;
        } else {
            theItem->enchant1 = newEnchant;
        }
        checkForDisenchantment(theItem);
    }
}

boolean enchantLevelKnown(const item *theItem) {
    if ((theItem->category & STAFF)
        && (theItem->flags & ITEM_MAX_CHARGES_KNOWN)) {

        return true;
    } else {
        return (theItem->flags & ITEM_IDENTIFIED);
    }
}

short effectiveEnchantLevel(const item *theItem) {
    if (theItem->category & WAND) {
        return theItem->charges;
    } else {
        return theItem->enchant1;
    }
}

boolean swapItemEnchants(const short machineNumber) {
    item *lockedItem, *tempItem;
    short i, j, oldEnchant;
    boolean enchantmentKnown;

    lockedItem = NULL;
    for (i = 0; i < DCOLS; i++) {
        for (j = 0; j < DROWS; j++) {
            tempItem = itemAtLoc(i, j);
            if (tempItem
                && pmap[i][j].machineNumber == machineNumber
                && cellHasTMFlag(i, j, TM_SWAP_ENCHANTS_ACTIVATION)
                && itemIsSwappable(tempItem)) {

                if (lockedItem) {
                    if (effectiveEnchantLevel(lockedItem) != effectiveEnchantLevel(tempItem)) {
                        // Presto change-o!
                        oldEnchant = effectiveEnchantLevel(lockedItem);
                        enchantmentKnown = enchantLevelKnown(lockedItem);
                        swapItemToEnchantLevel(lockedItem, effectiveEnchantLevel(tempItem), enchantLevelKnown(tempItem));
                        swapItemToEnchantLevel(tempItem, oldEnchant, enchantmentKnown);
                        return true;
                    }
                } else {
                    lockedItem = tempItem;
                }
            }
        }
    }
    return false;
}

void updateFloorItems() {
    short x, y, loc[2];
    char buf[DCOLS*3], buf2[DCOLS*3];
    enum dungeonLayers layer;
    item *theItem, *nextItem;

    for (theItem=floorItems->nextItem; theItem != NULL; theItem = nextItem) {
        nextItem = theItem->nextItem;
        x = theItem->xLoc;
        y = theItem->yLoc;
        if (cellHasTerrainFlag(x, y, T_AUTO_DESCENT)) {
            if (playerCanSeeOrSense(x, y)) {
                itemName(theItem, buf, false, false, NULL);
                sprintf(buf2, "The %s plunge%s out of sight!", buf, (theItem->quantity > 1 ? "" : "s"));
                messageWithColor(buf2, &itemMessageColor, false);
            }
            if (playerCanSee(x, y)) {
                discover(x, y);
            }
            theItem->flags |= ITEM_PREPLACED;

            // Remove from item chain.
            removeItemFromChain(theItem, floorItems);

            pmap[x][y].flags &= ~(HAS_ITEM | ITEM_DETECTED);

            if (theItem->category == POTION || rogue.depthLevel == DEEPEST_LEVEL) {
                // Potions don't survive the fall.
                deleteItem(theItem);
            } else {
                // Add to next level's chain.
                theItem->nextItem = levels[rogue.depthLevel-1 + 1].items;
                levels[rogue.depthLevel-1 + 1].items = theItem;
            }
            refreshDungeonCell(x, y);
            continue;
        }
        if ((cellHasTerrainFlag(x, y, T_IS_FIRE) && (theItem->flags & ITEM_FLAMMABLE))
            || (cellHasTerrainFlag(x, y, T_LAVA_INSTA_DEATH) && !(theItem->category & AMULET))) {

            burnItem(theItem);
            continue;
        }
        if (cellHasTerrainFlag(x, y, T_MOVES_ITEMS)) {
            getQualifyingLocNear(loc, x, y, true, 0, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_PASSABILITY), (HAS_ITEM), false, false);
            removeItemFrom(x, y);
            pmap[loc[0]][loc[1]].flags |= HAS_ITEM;
            if (pmap[x][y].flags & ITEM_DETECTED) {
                pmap[x][y].flags &= ~ITEM_DETECTED;
                pmap[loc[0]][loc[1]].flags |= ITEM_DETECTED;
            }
            theItem->xLoc = loc[0];
            theItem->yLoc = loc[1];
            refreshDungeonCell(x, y);
            refreshDungeonCell(loc[0], loc[1]);
            continue;
        }
        if (cellHasTMFlag(x, y, TM_PROMOTES_ON_STEP)) {
            for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_PROMOTES_ON_STEP) {
                    promoteTile(x, y, layer, false);
                }
            }
            continue;
        }
        if (pmap[x][y].machineNumber
            && pmap[x][y].machineNumber == pmap[player.xLoc][player.yLoc].machineNumber
            && (theItem->flags & ITEM_KIND_AUTO_ID)) {

            identifyItemKind(theItem);
        }
        if (cellHasTMFlag(x, y, TM_SWAP_ENCHANTS_ACTIVATION)
            && pmap[x][y].machineNumber) {

            while (nextItem != NULL
                   && pmap[x][y].machineNumber == pmap[nextItem->xLoc][nextItem->yLoc].machineNumber
                   && cellHasTMFlag(nextItem->xLoc, nextItem->yLoc, TM_SWAP_ENCHANTS_ACTIVATION)) {

                // Skip future items that are also swappable, so that we don't inadvertently
                // destroy the next item and then try to update it.
                nextItem = nextItem->nextItem;
            }

            if (!circuitBreakersPreventActivation(pmap[x][y].machineNumber)
                && swapItemEnchants(pmap[x][y].machineNumber)) {

                activateMachine(pmap[x][y].machineNumber);
            }
        }
    }
}

boolean inscribeItem(item *theItem) {
    char itemText[30], buf[COLS * 3], nameOfItem[COLS * 3], oldInscription[COLS];

    strcpy(oldInscription, theItem->inscription);
    theItem->inscription[0] = '\0';
    itemName(theItem, nameOfItem, true, true, NULL);
    strcpy(theItem->inscription, oldInscription);

    sprintf(buf, "inscribe: %s \"", nameOfItem);
    if (getInputTextString(itemText, buf, min(29, DCOLS - strLenWithoutEscapes(buf) - 1), "", "\"", TEXT_INPUT_NORMAL, false)) {
        strcpy(theItem->inscription, itemText);
        confirmMessages();
        itemName(theItem, nameOfItem, true, true, NULL);
        sprintf(buf, "%s %s.", (theItem->quantity > 1 ? "they're" : "it's"), nameOfItem);
        messageWithColor(buf, &itemMessageColor, false);
        return true;
    } else {
        confirmMessages();
        return false;
    }
}

boolean itemCanBeCalled(item *theItem) {
    if (theItem->category & (WEAPON|ARMOR|SCROLL|RING|POTION|STAFF|WAND|CHARM)) {
        return true;
    } else if ((theItem->category & (POTION | SCROLL))
               && !tableForItemCategory(theItem->category, NULL)[theItem->kind].identified) {
        return true;
    } else {
        return false;
    }
}

void call(item *theItem) {
    char itemText[30], buf[COLS * 3];
    short c;
    unsigned char command[100];
    item *tempItem;

    c = 0;
    command[c++] = CALL_KEY;
    if (theItem == NULL) {
        // Need to gray out known potions and scrolls from inventory selection.
        // Hijack the "item can be identified" flag for this purpose,
        // and then reset it immediately afterward.
        for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
            if ((tempItem->category & (POTION | SCROLL))
                && tableForItemCategory(tempItem->category, NULL)[tempItem->kind].identified) {

                tempItem->flags &= ~ITEM_CAN_BE_IDENTIFIED;
            } else {
                tempItem->flags |= ITEM_CAN_BE_IDENTIFIED;
            }
        }
        theItem = promptForItemOfType((WEAPON|ARMOR|SCROLL|RING|POTION|STAFF|WAND|CHARM), ITEM_CAN_BE_IDENTIFIED, 0,
                                      KEYBOARD_LABELS ? "Call what? (a-z, shift for more info; or <esc> to cancel)" : "Call what?",
                                      true);
        updateIdentifiableItems(); // Reset the flags.
    }
    if (theItem == NULL) {
        return;
    }

    command[c++] = theItem->inventoryLetter;

    confirmMessages();

    if ((theItem->flags & ITEM_IDENTIFIED) || theItem->category & (WEAPON|ARMOR|CHARM|FOOD|GOLD|AMULET|GEM)) {
        if (theItem->category & (WEAPON | ARMOR | CHARM | STAFF | WAND | RING)) {
            if (inscribeItem(theItem)) {
                command[c++] = '\0';
                strcat((char *) command, theItem->inscription);
                recordKeystrokeSequence(command);
                recordKeystroke(RETURN_KEY, false, false);
            }
        } else {
            message("you already know what that is.", false);
        }
        return;
    }

    if (theItem->category & (WEAPON | ARMOR | STAFF | WAND | RING)) {
        if (tableForItemCategory(theItem->category, NULL)[theItem->kind].identified) {
            if (inscribeItem(theItem)) {
                command[c++] = '\0';
                strcat((char *) command, theItem->inscription);
                recordKeystrokeSequence(command);
                recordKeystroke(RETURN_KEY, false, false);
            }
            return;
        } else if (confirm("Inscribe this particular item instead of all similar items?", true)) {
            command[c++] = 'y'; // y means yes, since the recording also needs to negotiate the above confirmation prompt.
            if (inscribeItem(theItem)) {
                command[c++] = '\0';
                strcat((char *) command, theItem->inscription);
                recordKeystrokeSequence(command);
                recordKeystroke(RETURN_KEY, false, false);
            }
            return;
        } else {
            command[c++] = 'n'; // n means no
        }
    }

    if (tableForItemCategory(theItem->category, NULL)
        && !(tableForItemCategory(theItem->category, NULL)[theItem->kind].identified)) {

        if (getInputTextString(itemText, "call them: \"", 29, "", "\"", TEXT_INPUT_NORMAL, false)) {
            command[c++] = '\0';
            strcat((char *) command, itemText);
            recordKeystrokeSequence(command);
            recordKeystroke(RETURN_KEY, false, false);
            if (itemText[0]) {
                strcpy(tableForItemCategory(theItem->category, NULL)[theItem->kind].callTitle, itemText);
                tableForItemCategory(theItem->category, NULL)[theItem->kind].called = true;
            } else {
                tableForItemCategory(theItem->category, NULL)[theItem->kind].callTitle[0] = '\0';
                tableForItemCategory(theItem->category, NULL)[theItem->kind].called = false;
            }
            confirmMessages();
            itemName(theItem, buf, false, true, NULL);
            messageWithColor(buf, &itemMessageColor, false);
        }
    } else {
        message("you already know what that is.", false);
    }
}

// Generates the item name and returns it in the "root" string.
// IncludeDetails governs things such as enchantment, charges, strength requirement, times used, etc.
// IncludeArticle governs the article -- e.g. "some" food, "5" darts, "a" pink potion.
// If baseColor is provided, then the suffix will be in gray, flavor portions of the item name (e.g. a "pink" potion,
//  a "sandalwood" staff, a "ruby" ring) will be in dark purple, and the Amulet of Yendor and lumenstones will be in yellow.
//  BaseColor itself will be the color that the name reverts to outside of these colored portions.
void itemName(item *theItem, char *root, boolean includeDetails, boolean includeArticle, color *baseColor) {
    char buf[DCOLS * 5], pluralization[10], article[10] = "",
    grayEscapeSequence[5], purpleEscapeSequence[5], yellowEscapeSequence[5], baseEscapeSequence[5];
    color tempColor;

    strcpy(pluralization, (theItem->quantity > 1 ? "s" : ""));

    grayEscapeSequence[0] = '\0';
    purpleEscapeSequence[0] = '\0';
    yellowEscapeSequence[0] = '\0';
    baseEscapeSequence[0] = '\0';
    if (baseColor) {
        tempColor = backgroundMessageColor;
        applyColorMultiplier(&tempColor, baseColor); // To gray out the purple if necessary.
        encodeMessageColor(purpleEscapeSequence, 0, &tempColor);

        tempColor = gray;
        //applyColorMultiplier(&tempColor, baseColor);
        encodeMessageColor(grayEscapeSequence, 0, &tempColor);

        tempColor = itemMessageColor;
        applyColorMultiplier(&tempColor, baseColor);
        encodeMessageColor(yellowEscapeSequence, 0, &tempColor);

        encodeMessageColor(baseEscapeSequence, 0, baseColor);
    }

    switch (theItem -> category) {
        case FOOD:
            if (theItem -> kind == FRUIT) {
                sprintf(root, "mango%s", pluralization);
            } else {
                if (theItem->quantity == 1) {
                    sprintf(article, "some ");
                    sprintf(root, "food");
                } else {
                    sprintf(root, "ration%s of food", pluralization);
                }
            }
            break;
        case WEAPON:
            sprintf(root, "%s%s", weaponTable[theItem->kind].name, pluralization);
            if (includeDetails) {
                if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
                    sprintf(buf, "%s%i %s", (theItem->enchant1 < 0 ? "" : "+"), theItem->enchant1, root);
                    strcpy(root, buf);
                }

                if (theItem->flags & ITEM_RUNIC) {
                    if ((theItem->flags & ITEM_RUNIC_IDENTIFIED) || rogue.playbackOmniscience) {
                        if (theItem->enchant2 == W_SLAYING) {
                            sprintf(buf, "%s of %s slaying%s",
                                    root,
                                    monsterClassCatalog[theItem->vorpalEnemy].name,
                                    grayEscapeSequence);
                            strcpy(root, buf);
                        } else {
                            sprintf(buf, "%s of %s%s",
                                    root,
                                    weaponRunicNames[theItem->enchant2],
                                    grayEscapeSequence);
                            strcpy(root, buf);
                        }
                    } else if (theItem->flags & (ITEM_IDENTIFIED | ITEM_RUNIC_HINTED)) {
                        if (grayEscapeSequence[0]) {
                            strcat(root, grayEscapeSequence);
                        }
                        strcat(root, " (unknown runic)");
                    }
                }
                sprintf(buf, "%s%s <%i>", root, grayEscapeSequence, theItem->strengthRequired);
                strcpy(root, buf);
            }
            break;
        case ARMOR:
            sprintf(root, "%s", armorTable[theItem->kind].name);
            if (includeDetails) {

                if ((theItem->flags & ITEM_RUNIC)
                    && ((theItem->flags & ITEM_RUNIC_IDENTIFIED)
                        || rogue.playbackOmniscience)) {
                        if (theItem->enchant2 == A_IMMUNITY) {
                            sprintf(buf, "%s of %s immunity", root, monsterClassCatalog[theItem->vorpalEnemy].name);
                            strcpy(root, buf);
                        } else {
                            sprintf(buf, "%s of %s", root, armorRunicNames[theItem->enchant2]);
                            strcpy(root, buf);
                        }
                    }

                if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
                    if (theItem->enchant1 == 0) {
                        sprintf(buf, "%s%s [%i]<%i>", root, grayEscapeSequence, theItem->armor/10, theItem->strengthRequired);
                    } else {
                        sprintf(buf, "%s%i %s%s [%i]<%i>",
                                (theItem->enchant1 < 0 ? "" : "+"),
                                theItem->enchant1,
                                root,
                                grayEscapeSequence,
                                theItem->armor/10 + theItem->enchant1,
                                theItem->strengthRequired);
                    }
                    strcpy(root, buf);
                } else {
                    sprintf(buf, "%s%s <%i>", root, grayEscapeSequence, theItem->strengthRequired);
                    strcpy(root, buf);
                }

                if ((theItem->flags & ITEM_RUNIC)
                    && (theItem->flags & (ITEM_IDENTIFIED | ITEM_RUNIC_HINTED))
                    && !(theItem->flags & ITEM_RUNIC_IDENTIFIED)
                    && !rogue.playbackOmniscience) {
                    strcat(root, " (unknown runic)");
                }
            }
            break;
        case SCROLL:
            if (scrollTable[theItem->kind].identified || rogue.playbackOmniscience) {
                sprintf(root, "scroll%s of %s", pluralization, scrollTable[theItem->kind].name);
            } else if (scrollTable[theItem->kind].called) {
                sprintf(root, "scroll%s called %s%s%s",
                        pluralization,
                        purpleEscapeSequence,
                        scrollTable[theItem->kind].callTitle,
                        baseEscapeSequence);
            } else {
                sprintf(root, "scroll%s entitled %s\"%s\"%s",
                        pluralization,
                        purpleEscapeSequence,
                        scrollTable[theItem->kind].flavor,
                        baseEscapeSequence);
            }
            break;
        case POTION:
            if (potionTable[theItem->kind].identified || rogue.playbackOmniscience) {
                sprintf(root, "potion%s of %s", pluralization, potionTable[theItem->kind].name);
            } else if (potionTable[theItem->kind].called) {
                sprintf(root, "potion%s called %s%s%s",
                        pluralization,
                        purpleEscapeSequence,
                        potionTable[theItem->kind].callTitle,
                        baseEscapeSequence);
            } else {
                sprintf(root, "%s%s%s potion%s",
                        purpleEscapeSequence,
                        potionTable[theItem->kind].flavor,
                        baseEscapeSequence,
                        pluralization);
            }
            break;
        case WAND:
            if (wandTable[theItem->kind].identified || rogue.playbackOmniscience) {
                sprintf(root, "wand%s of %s",
                        pluralization,
                        wandTable[theItem->kind].name);
            } else if (wandTable[theItem->kind].called) {
                sprintf(root, "wand%s called %s%s%s",
                        pluralization,
                        purpleEscapeSequence,
                        wandTable[theItem->kind].callTitle,
                        baseEscapeSequence);
            } else {
                sprintf(root, "%s%s%s wand%s",
                        purpleEscapeSequence,
                        wandTable[theItem->kind].flavor,
                        baseEscapeSequence,
                        pluralization);
            }
            if (includeDetails) {
                if (theItem->flags & (ITEM_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN) || rogue.playbackOmniscience) {
                    sprintf(buf, "%s%s [%i]",
                            root,
                            grayEscapeSequence,
                            theItem->charges);
                    strcpy(root, buf);
                } else if (theItem->enchant2 > 2) {
                    sprintf(buf, "%s%s (used %i times)",
                            root,
                            grayEscapeSequence,
                            theItem->enchant2);
                    strcpy(root, buf);
                } else if (theItem->enchant2) {
                    sprintf(buf, "%s%s (used %s)",
                            root,
                            grayEscapeSequence,
                            (theItem->enchant2 == 2 ? "twice" : "once"));
                    strcpy(root, buf);
                }
            }
            break;
        case STAFF:
            if (staffTable[theItem->kind].identified || rogue.playbackOmniscience) {
                sprintf(root, "staff%s of %s", pluralization, staffTable[theItem->kind].name);
            } else if (staffTable[theItem->kind].called) {
                sprintf(root, "staff%s called %s%s%s",
                        pluralization,
                        purpleEscapeSequence,
                        staffTable[theItem->kind].callTitle,
                        baseEscapeSequence);
            } else {
                sprintf(root, "%s%s%s staff%s",
                        purpleEscapeSequence,
                        staffTable[theItem->kind].flavor,
                        baseEscapeSequence,
                        pluralization);
            }
            if (includeDetails) {
                if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
                    sprintf(buf, "%s%s [%i/%i]", root, grayEscapeSequence, theItem->charges, theItem->enchant1);
                    strcpy(root, buf);
                } else if (theItem->flags & ITEM_MAX_CHARGES_KNOWN) {
                    sprintf(buf, "%s%s [?/%i]", root, grayEscapeSequence, theItem->enchant1);
                    strcpy(root, buf);
                }
            }
            break;
        case RING:
            if (ringTable[theItem->kind].identified || rogue.playbackOmniscience) {
                sprintf(root, "ring%s of %s", pluralization, ringTable[theItem->kind].name);
            } else if (ringTable[theItem->kind].called) {
                sprintf(root, "ring%s called %s%s%s",
                        pluralization,
                        purpleEscapeSequence,
                        ringTable[theItem->kind].callTitle,
                        baseEscapeSequence);
            } else {
                sprintf(root, "%s%s%s ring%s",
                        purpleEscapeSequence,
                        ringTable[theItem->kind].flavor,
                        baseEscapeSequence,
                        pluralization);
            }
            if (includeDetails && ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience)) {
                sprintf(buf, "%s%i %s", (theItem->enchant1 < 0 ? "" : "+"), theItem->enchant1, root);
                strcpy(root, buf);
            }
            break;
        case CHARM:
            sprintf(root, "%s charm%s", charmTable[theItem->kind].name, pluralization);

            if (includeDetails) {
                sprintf(buf, "%s%i %s", (theItem->enchant1 < 0 ? "" : "+"), theItem->enchant1, root);
                strcpy(root, buf);

                if (theItem->charges) {
                    sprintf(buf, "%s %s(%i%%)",
                            root,
                            grayEscapeSequence,
                            (charmRechargeDelay(theItem->kind, theItem->enchant1) - theItem->charges) * 100 / charmRechargeDelay(theItem->kind, theItem->enchant1));
                    strcpy(root, buf);
                } else {
                    strcat(root, grayEscapeSequence);
                    strcat(root, " (ready)");
                }
            }
            break;
        case GOLD:
            sprintf(root, "gold piece%s", pluralization);
            break;
        case AMULET:
            sprintf(root, "%sAmulet%s of Yendor%s", yellowEscapeSequence, pluralization, baseEscapeSequence);
            break;
        case GEM:
            sprintf(root, "%slumenstone%s%s from depth %i", yellowEscapeSequence, pluralization, baseEscapeSequence, theItem->originDepth);
            break;
        case KEY:
            if (includeDetails && theItem->originDepth > 0 && theItem->originDepth != rogue.depthLevel) {
                sprintf(root, "%s%s%s from depth %i",
                        keyTable[theItem->kind].name,
                        pluralization,
                        grayEscapeSequence,
                        theItem->originDepth);
            } else {
                sprintf(root,
                        keyTable[theItem->kind].name,
                        "%s%s",
                        pluralization);
            }
            break;
        default:
            sprintf(root, "unknown item%s", pluralization);
            break;
    }

    if (includeArticle) {
        // prepend number if quantity is over 1
        if (theItem->quantity > 1) {
            sprintf(article, "%i ", theItem->quantity);
        } else if (theItem->category & AMULET) {
            sprintf(article, "the ");
        } else if (!(theItem->category & ARMOR) && !(theItem->category & FOOD && theItem->kind == RATION)) {
            // otherwise prepend a/an if the item is not armor and not a ration of food;
            // armor gets no article, and "some food" was taken care of above.
            sprintf(article, "a%s ", (isVowelish(root) ? "n" : ""));
        }
    }
    // strcat(buf, suffixID);
    if (includeArticle) {
        sprintf(buf, "%s%s", article, root);
        strcpy(root, buf);
    }

    if (includeDetails && theItem->inscription[0]) {
        sprintf(buf, "%s \"%s\"", root, theItem->inscription);
        strcpy(root, buf);
    }
    return;
}

// kindCount is optional
itemTable *tableForItemCategory(enum itemCategory theCat, short *kindCount) {
    itemTable *returnedTable;
    short returnedCount;
    switch (theCat) {
        case FOOD:
            returnedTable = foodTable;
            returnedCount = NUMBER_FOOD_KINDS;
            break;
        case WEAPON:
            returnedTable = weaponTable;
            returnedCount = NUMBER_WEAPON_KINDS;
            break;
        case ARMOR:
            returnedTable = armorTable;
            returnedCount = NUMBER_ARMOR_KINDS;
            break;
        case POTION:
            returnedTable = potionTable;
            returnedCount = NUMBER_POTION_KINDS;
            break;
        case SCROLL:
            returnedTable = scrollTable;
            returnedCount = NUMBER_SCROLL_KINDS;
            break;
        case RING:
            returnedTable = ringTable;
            returnedCount = NUMBER_RING_KINDS;
            break;
        case WAND:
            returnedTable = wandTable;
            returnedCount = NUMBER_WAND_KINDS;
            break;
        case STAFF:
            returnedTable = staffTable;
            returnedCount = NUMBER_STAFF_KINDS;
            break;
        case CHARM:
            returnedTable = charmTable;
            returnedCount = NUMBER_CHARM_KINDS;
            break;
        default:
            returnedTable = NULL;
            returnedCount = 0;
            break;
    }
    if (kindCount) {
        *kindCount = returnedCount;
    }
    return returnedTable;
}

boolean isVowelish(char *theChar) {
    short i;

    while (*theChar == COLOR_ESCAPE) {
        theChar += 4;
    }
    char str[30];
    strncpy(str, theChar, 29);
    for (i = 0; i < 30; i++) {
        upperCase(&(str[i]));
    }
    if (stringsMatch(str, "UNI")        // Words that start with "uni" aren't treated like vowels; e.g., "a" unicorn.
        || stringsMatch(str, "EU")) {   // Words that start with "eu" aren't treated like vowels; e.g., "a" eucalpytus staff.

        return false;
    } else {
        return (str[0] == 'A'
                || str[0] == 'E'
                || str[0] == 'I'
                || str[0] == 'O'
                || str[0] == 'U');
    }
}

fixpt enchantIncrement(item *theItem) {
    if (theItem->category & (WEAPON | ARMOR)) {
        if (theItem->strengthRequired == 0) {
            return FP_FACTOR;
        } else if (rogue.strength - player.weaknessAmount < theItem->strengthRequired) {
            return FP_FACTOR * 35 / 10;
        } else {
            return FP_FACTOR * 125 / 100;
        }
    } else {
        return FP_FACTOR;
    }
}

boolean itemIsCarried(item *theItem) {
    item *tempItem;

    for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
        if (tempItem == theItem) {
            return true;
        }
    }
    return false;
}

short effectiveRingEnchant(item *theItem) {
    if (theItem->category != RING) {
        return 0;
    }
    if (!(theItem->flags & ITEM_IDENTIFIED)
        && theItem->enchant1 > 0) {

        return theItem->timesEnchanted + 1; // Unidentified positive rings act as +1 until identified.
    }
    return theItem->enchant1;
}

short apparentRingBonus(const enum ringKind kind) {
    item *rings[2] = {rogue.ringLeft, rogue.ringRight}, *ring;
    short retval = 0;
    short i;

    if (ringTable[kind].identified) {
        for (i = 0; i < 2; i++) {
            ring = rings[i];
            if (ring && ring->kind == kind) {
                retval += effectiveRingEnchant(ring);
            }
        }
    }
    return retval;
}

void itemDetails(char *buf, item *theItem) {
    char buf2[1000], buf3[1000], theName[500], goodColorEscape[20], badColorEscape[20], whiteColorEscape[20];
    boolean singular, carried;
    fixpt enchant;
    fixpt currentDamage, newDamage;
    short nextLevelState = 0, new, current, accuracyChange, damageChange;
    const char weaponRunicEffectDescriptions[NUMBER_WEAPON_RUNIC_KINDS][DCOLS] = {
        "time will stop while you take an extra turn",
        "the enemy will die instantly",
        "the enemy will be paralyzed",
        "[multiplicity]", // never used
        "the enemy will be slowed",
        "the enemy will be confused",
        "the enemy will be flung",
        "[slaying]", // never used
        "the enemy will be healed",
        "the enemy will be cloned"
    };

    goodColorEscape[0] = badColorEscape[0] = whiteColorEscape[0] = '\0';
    encodeMessageColor(goodColorEscape, 0, &goodMessageColor);
    encodeMessageColor(badColorEscape, 0, &badMessageColor);
    encodeMessageColor(whiteColorEscape, 0, &white);

    singular = (theItem->quantity == 1 ? true : false);
    carried = itemIsCarried(theItem);

    // Name
    itemName(theItem, theName, true, true, NULL);
    buf[0] = '\0';
    encodeMessageColor(buf, 0, &itemMessageColor);
    upperCase(theName);
    strcat(buf, theName);
    if (carried) {
        sprintf(buf2, " (%c)", theItem->inventoryLetter);
        strcat(buf, buf2);
    }
    encodeMessageColor(buf, strlen(buf), &white);
    strcat(buf, "\n\n");

    enchant = netEnchant(theItem);

    itemName(theItem, theName, false, false, NULL);

    // introductory text
    if (tableForItemCategory(theItem->category, NULL)
        && (tableForItemCategory(theItem->category, NULL)[theItem->kind].identified || rogue.playbackOmniscience)) {

        strcat(buf, tableForItemCategory(theItem->category, NULL)[theItem->kind].description);

        if (theItem->category == POTION && theItem->kind == POTION_LIFE) {
            sprintf(buf2, "\n\nIt will increase your maximum health by %s%i%%%s.",
                    goodColorEscape,
                    (player.info.maxHP + 10) * 100 / player.info.maxHP - 100,
                    whiteColorEscape);
            strcat(buf, buf2);
        }
    } else {
        switch (theItem->category) {
            case POTION:
                sprintf(buf2, "%s flask%s contain%s a swirling %s liquid. Who knows what %s will do when drunk or thrown?",
                        (singular ? "This" : "These"),
                        (singular ? "" : "s"),
                        (singular ? "s" : ""),
                        tableForItemCategory(theItem->category, NULL)[theItem->kind].flavor,
                        (singular ? "it" : "they"));
                break;
            case SCROLL:
                sprintf(buf2, "%s parchment%s %s covered with indecipherable writing, and bear%s a title of \"%s.\" Who knows what %s will do when read aloud?",
                        (singular ? "This" : "These"),
                        (singular ? "" : "s"),
                        (singular ? "is" : "are"),
                        (singular ? "s" : ""),
                        tableForItemCategory(theItem->category, NULL)[theItem->kind].flavor,
                        (singular ? "it" : "they"));
                break;
            case STAFF:
                sprintf(buf2, "This gnarled %s staff is warm to the touch. Who knows what it will do when used?",
                        tableForItemCategory(theItem->category, NULL)[theItem->kind].flavor);
                break;
            case WAND:
                sprintf(buf2, "This thin %s wand is warm to the touch. Who knows what it will do when used?",
                        tableForItemCategory(theItem->category, NULL)[theItem->kind].flavor);
                break;
            case RING:
                sprintf(buf2, "This metal band is adorned with a%s %s gem that glitters in the darkness. Who knows what effect it has when worn? ",
                        isVowelish(tableForItemCategory(theItem->category, NULL)[theItem->kind].flavor) ? "n" : "",
                        tableForItemCategory(theItem->category, NULL)[theItem->kind].flavor);
                break;
            case CHARM: // Should never be displayed.
                strcat(buf2, "What a perplexing charm!");
                break;
            case AMULET:
                strcpy(buf2, "Legends are told about this mysterious golden amulet, and legions of adventurers have perished in its pursuit. Unfathomable riches await anyone with the skill and ambition to carry it into the light of day.");
                break;
            case GEM:
                sprintf(buf2, "Faint golden lights swirl and fluoresce beneath the stone%s surface. Lumenstones are said to contain mysterious properties of untold power, but for you, they mean one thing: riches.",
                        (singular ? "'s" : "s'"));
                break;
            case KEY:
                strcpy(buf2, keyTable[theItem->kind].description);
                break;
            case GOLD:
                sprintf(buf2, "A pile of %i shining gold coins.", theItem->quantity);
                break;
            default:
                break;
        }
        strcat(buf, buf2);
    }

    if (carried && theItem->originDepth > 0) {
        sprintf(buf2, " (You found %s on depth %i.) ",
                singular ? "it" : "them",
                theItem->originDepth);
        strcat(buf, buf2);
    }

    // detailed description
    switch (theItem->category) {

        case FOOD:
            sprintf(buf2, "\n\nYou are %shungry enough to fully enjoy a %s.",
                    ((STOMACH_SIZE - player.status[STATUS_NUTRITION]) >= foodTable[theItem->kind].strengthRequired ? "" : "not yet "),
                    foodTable[theItem->kind].name);
            strcat(buf, buf2);
            break;

        case WEAPON:
        case ARMOR:
            // enchanted? strength modifier?
            if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
                if (theItem->enchant1) {
                    if (theItem->enchant1 > 0) {
                        sprintf(buf2, "\n\nThe %s bear%s an intrinsic enchantment of %s+%i%s",
                                theName,
                                (singular ? "s" : ""),
                                goodColorEscape,
                                theItem->enchant1,
                                whiteColorEscape);
                    } else {
                        sprintf(buf2, "\n\nThe %s bear%s an intrinsic penalty of %s%i%s",
                                theName,
                                (singular ? "s" : ""),
                                badColorEscape,
                                theItem->enchant1,
                                whiteColorEscape);
                    }
                } else {
                    sprintf(buf2, "\n\nThe %s bear%s no intrinsic enchantment",
                            theName,
                            (singular ? "s" : ""));
                }
                strcat(buf, buf2);
                if (strengthModifier(theItem)) {
                    sprintf(buf2, ", %s %s %s %s%s%+.2f%s because of your %s strength. ",
                            (theItem->enchant1 ? "and" : "but"),
                            (singular ? "carries" : "carry"),
                            (theItem->enchant1 && (theItem->enchant1 > 0) == (strengthModifier(theItem) > 0) ? "an additional" : "a"),
                            (strengthModifier(theItem) > 0 ? "bonus of " : "penalty of "),
                            (strengthModifier(theItem) > 0 ? goodColorEscape : badColorEscape),
                            strengthModifier(theItem) / (double) FP_FACTOR,
                            whiteColorEscape,
                            (strengthModifier(theItem) > 0 ? "excess" : "inadequate"));
                    strcat(buf, buf2);
                } else {
                    strcat(buf, ". ");
                }
            } else {
                if ((theItem->enchant1 > 0) && (theItem->flags & ITEM_MAGIC_DETECTED)) {
                    sprintf(buf2, "\n\nYou can feel an %saura of benevolent magic%s radiating from the %s. ",
                            goodColorEscape,
                            whiteColorEscape,
                            theName);
                    strcat(buf, buf2);
                }
                if (strengthModifier(theItem)) {
                    sprintf(buf2, "\n\nThe %s %s%s a %s%s%+.2f%s because of your %s strength. ",
                            theName,
                            ((theItem->enchant1 > 0) && (theItem->flags & ITEM_MAGIC_DETECTED) ? "also " : ""),
                            (singular ? "carries" : "carry"),
                            (strengthModifier(theItem) > 0 ? "bonus of " : "penalty of "),
                            (strengthModifier(theItem) > 0 ? goodColorEscape : badColorEscape),
                            strengthModifier(theItem) / (double) FP_FACTOR,
                            whiteColorEscape,
                            (strengthModifier(theItem) > 0 ? "excess" : "inadequate"));
                    strcat(buf, buf2);
                }

                if (theItem->category & WEAPON) {
                    sprintf(buf2, "It will reveal its secrets if you defeat %i%s %s with it. ",
                            theItem->charges,
                            (theItem->charges == WEAPON_KILLS_TO_AUTO_ID ? "" : " more"),
                            (theItem->charges == 1 ? "enemy" : "enemies"));
                } else {
                    sprintf(buf2, "It will reveal its secrets if worn for %i%s turn%s. ",
                            theItem->charges,
                            (theItem->charges == ARMOR_DELAY_TO_AUTO_ID ? "" : " more"),
                            (theItem->charges == 1 ? "" : "s"));
                }
                strcat(buf, buf2);
            }

            // Display the known percentage by which the armor/weapon will increase/decrease accuracy/damage/defense if not already equipped.
            if (!(theItem->flags & ITEM_EQUIPPED)) {
                if (theItem->category & WEAPON) {
                    current = player.info.accuracy;
                    if (rogue.weapon) {
                        currentDamage = (rogue.weapon->damage.lowerBound + rogue.weapon->damage.upperBound) * FP_FACTOR / 2;
                        if ((rogue.weapon->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
                            current = current * accuracyFraction(netEnchant(rogue.weapon)) / FP_FACTOR;
                            currentDamage = currentDamage * damageFraction(netEnchant(rogue.weapon)) / FP_FACTOR;
                        } else {
                            current = current * accuracyFraction(strengthModifier(rogue.weapon)) / FP_FACTOR;
                            currentDamage = currentDamage * damageFraction(strengthModifier(rogue.weapon)) / FP_FACTOR;
                        }
                    } else {
                        currentDamage = (player.info.damage.lowerBound + player.info.damage.upperBound) * FP_FACTOR / 2;
                    }

                    new = player.info.accuracy;
                    newDamage = (theItem->damage.lowerBound + theItem->damage.upperBound) * FP_FACTOR / 2;
                    if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
                        new = new * accuracyFraction(netEnchant(theItem)) / FP_FACTOR;
                        newDamage = newDamage * damageFraction(netEnchant(theItem)) / FP_FACTOR;
                    } else {
                        new = new * accuracyFraction(strengthModifier(theItem)) / FP_FACTOR;
                        newDamage = newDamage * damageFraction(strengthModifier(theItem)) / FP_FACTOR;
                    }
                    accuracyChange  = (new * 100 / current) - 100;
                    damageChange    = (newDamage * 100 / currentDamage) - 100;
                    sprintf(buf2, "Wielding the %s%s will %s your current accuracy by %s%i%%%s, and will %s your current damage by %s%i%%%s. ",
                            theName,
                            ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) ? "" : ", assuming it has no hidden properties,",
                            (((short) accuracyChange) < 0) ? "decrease" : "increase",
                            (((short) accuracyChange) < 0) ? badColorEscape : (accuracyChange > 0 ? goodColorEscape : ""),
                            abs((short) accuracyChange),
                            whiteColorEscape,
                            (((short) damageChange) < 0) ? "decrease" : "increase",
                            (((short) damageChange) < 0) ? badColorEscape : (damageChange > 0 ? goodColorEscape : ""),
                            abs((short) damageChange),
                            whiteColorEscape);
                } else {
                    new = theItem->armor;
                    if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
                        new += 10 * netEnchant(theItem) / FP_FACTOR;
                    } else {
                        new += 10 * strengthModifier(theItem) / FP_FACTOR;
                    }
                    new = max(0, new);
                    new /= 10;
                    sprintf(buf2, "Wearing the %s%s will result in an armor rating of %s%i%s. ",
                            theName,
                            ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) ? "" : ", assuming it has no hidden properties,",
                            (new > displayedArmorValue() ? goodColorEscape : (new < displayedArmorValue() ? badColorEscape : whiteColorEscape)),
                            new, whiteColorEscape);
                }
                strcat(buf, buf2);
            }

            // protected?
            if (theItem->flags & ITEM_PROTECTED) {
                sprintf(buf2, "%sThe %s cannot be corroded by acid.%s ",
                        goodColorEscape,
                        theName,
                        whiteColorEscape);
                strcat(buf, buf2);
            }

            // heavy armor?
            current = armorAggroAdjustment(rogue.armor);
            if ((theItem->category & ARMOR)
                && !(theItem->flags & ITEM_EQUIPPED)
                && (current != armorAggroAdjustment(theItem))) {

                new = armorAggroAdjustment(theItem);
                if (rogue.armor) {
                    new -= armorAggroAdjustment(rogue.armor);
                }
                sprintf(buf2, "Equipping the %s will %s%s your stealth range by %i%s. ",
                        theName,
                        new > 0 ? badColorEscape : goodColorEscape,
                        new > 0 ? "increase" : "decrease",
                        abs(new),
                        whiteColorEscape);
                strcat(buf, buf2);
            }

            if (theItem->category & WEAPON) {

                // runic?
                if (theItem->flags & ITEM_RUNIC) {
                    if ((theItem->flags & ITEM_RUNIC_IDENTIFIED) || rogue.playbackOmniscience) {
                        sprintf(buf2, "\n\nGlowing runes of %s adorn the %s. ",
                                weaponRunicNames[theItem->enchant2],
                                theName);
                        strcat(buf, buf2);
                        if (theItem->enchant2 == W_SLAYING) {
                            describeMonsterClass(buf3, theItem->vorpalEnemy, false);
                            sprintf(buf2, "It will never fail to slay a%s %s in a single stroke. ",
                                    (isVowelish(buf3) ? "n" : ""),
                                    buf3);
                            strcat(buf, buf2);
                        } else if (theItem->enchant2 == W_MULTIPLICITY) {
                            if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
                                sprintf(buf2, "%i%% of the time that it hits an enemy, %i spectral %s%s will spring into being with accuracy and attack power equal to your own, and will dissipate %i turns later. (If the %s is enchanted, %i image%s will appear %i%% of the time, and will last %i turns.)",
                                        runicWeaponChance(theItem, false, 0),
                                        weaponImageCount(enchant),
                                        theName,
                                        (weaponImageCount(enchant) > 1 ? "s" : ""),
                                        weaponImageDuration(enchant),
                                        theName,
                                        weaponImageCount(enchant + enchantIncrement(theItem)),
                                        (weaponImageCount(enchant + enchantIncrement(theItem)) > 1 ? "s" : ""),
                                        runicWeaponChance(theItem, true, enchant + enchantIncrement(theItem)),
                                        weaponImageDuration(enchant + enchantIncrement(theItem)));
                            } else {
                                sprintf(buf2, "Sometimes, when it hits an enemy, spectral %ss will spring into being with accuracy and attack power equal to your own, and will dissipate shortly thereafter.",
                                        theName);
                            }
                            strcat(buf, buf2);
                        } else {
                            if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
                                if (runicWeaponChance(theItem, false, 0) < 2
                                    && rogue.strength - player.weaknessAmount < theItem->strengthRequired) {

                                    strcpy(buf2, "Its runic effect will almost never activate because of your inadequate strength, but sometimes, when");
                                } else {
                                    sprintf(buf2, "%i%% of the time that",
                                            runicWeaponChance(theItem, false, 0));
                                }
                                strcat(buf, buf2);
                            } else {
                                strcat(buf, "Sometimes, when");
                            }
                            sprintf(buf2, " it hits an enemy, %s",
                                    weaponRunicEffectDescriptions[theItem->enchant2]);
                            strcat(buf, buf2);

                            if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
                                switch (theItem->enchant2) {
                                    case W_SPEED:
                                        strcat(buf, ". ");
                                        break;
                                    case W_PARALYSIS:
                                        sprintf(buf2, " for %i turns. ",
                                                (int) (weaponParalysisDuration(enchant)));
                                        strcat(buf, buf2);
                                        nextLevelState = (int) (weaponParalysisDuration(enchant + enchantIncrement(theItem)));
                                        break;
                                    case W_SLOWING:
                                        sprintf(buf2, " for %i turns. ",
                                                weaponSlowDuration(enchant));
                                        strcat(buf, buf2);
                                        nextLevelState = weaponSlowDuration(enchant + enchantIncrement(theItem));
                                        break;
                                    case W_CONFUSION:
                                        sprintf(buf2, " for %i turns. ",
                                                weaponConfusionDuration(enchant));
                                        strcat(buf, buf2);
                                        nextLevelState = weaponConfusionDuration(enchant + enchantIncrement(theItem));
                                        break;
                                    case W_FORCE:
                                        sprintf(buf2, " up to %i spaces backward. If the enemy hits an obstruction, it (and any monster it hits) will take damage in proportion to the distance it flew. ",
                                                weaponForceDistance(enchant));
                                        strcat(buf, buf2);
                                        nextLevelState = weaponForceDistance(enchant + enchantIncrement(theItem));
                                        break;
                                    case W_MERCY:
                                        strcpy(buf2, " by 50% of its maximum health. ");
                                        strcat(buf, buf2);
                                        break;
                                    default:
                                        strcpy(buf2, ". ");
                                        strcat(buf, buf2);
                                        break;
                                }

                                if (((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience)
                                    && runicWeaponChance(theItem, false, 0) < runicWeaponChance(theItem, true, enchant + enchantIncrement(theItem))){
                                    sprintf(buf2, "(If the %s is enchanted, the chance will increase to %i%%",
                                            theName,
                                            runicWeaponChance(theItem, true, (float) (enchant + enchantIncrement(theItem))));
                                    strcat(buf, buf2);
                                    if (nextLevelState) {
                                        if (theItem->enchant2 == W_FORCE) {
                                            sprintf(buf2, " and the distance will increase to %i.)",
                                                    nextLevelState);
                                        } else {
                                            sprintf(buf2, " and the duration will increase to %i turns.)",
                                                    nextLevelState);
                                        }
                                    } else {
                                        strcpy(buf2, ".)");
                                    }
                                    strcat(buf, buf2);
                                }
                            } else {
                                strcat(buf, ". ");
                            }
                        }

                    } else if (theItem->flags & ITEM_IDENTIFIED) {
                        sprintf(buf2, "\n\nGlowing runes of an indecipherable language run down the length of the %s. ",
                                theName);
                        strcat(buf, buf2);
                    }
                }

                // equipped? cursed?
                if (theItem->flags & ITEM_EQUIPPED) {
                    sprintf(buf2, "\n\nYou hold the %s at the ready%s. ",
                            theName,
                            ((theItem->flags & ITEM_CURSED) ? ", and because it is cursed, you are powerless to let go" : ""));
                    strcat(buf, buf2);
                } else if (((theItem->flags & (ITEM_IDENTIFIED | ITEM_MAGIC_DETECTED)) || rogue.playbackOmniscience)
                           && (theItem->flags & ITEM_CURSED)) {
                    sprintf(buf2, "\n\n%sYou can feel a malevolent magic lurking within the %s.%s ",
                            badColorEscape,
                            theName,
                            whiteColorEscape);
                    strcat(buf, buf2);
                }

            } else if (theItem->category & ARMOR) {

                // runic?
                if (theItem->flags & ITEM_RUNIC) {
                    if ((theItem->flags & ITEM_RUNIC_IDENTIFIED) || rogue.playbackOmniscience) {
                        sprintf(buf2, "\n\nGlowing runes of %s adorn the %s. ",
                                armorRunicNames[theItem->enchant2],
                                theName);
                        strcat(buf, buf2);

                        // A_MULTIPLICITY, A_MUTUALITY, A_ABSORPTION, A_REPRISAL, A_IMMUNITY, A_REFLECTION, A_BURDEN, A_VULNERABILITY, A_IMMOLATION
                        switch (theItem->enchant2) {
                            case A_MULTIPLICITY:
                                sprintf(buf2, "When worn, 33%% of the time that an enemy's attack connects, %i allied spectral duplicate%s of your attacker will appear for 3 turns. ",
                                        armorImageCount(enchant),
                                        (armorImageCount(enchant) == 1 ? "" : "s"));
                                if (armorImageCount(enchant + enchantIncrement(theItem)) > armorImageCount(enchant)) {
                                    sprintf(buf3, "(If the %s is enchanted, the number of duplicates will increase to %i.) ",
                                            theName,
                                            (armorImageCount(enchant + enchantIncrement(theItem))));
                                    strcat(buf2, buf3);
                                }
                                break;
                            case A_MUTUALITY:
                                strcpy(buf2, "When worn, the damage that you incur from physical attacks will be split evenly among yourself and all other adjacent enemies. ");
                                break;
                            case A_ABSORPTION:
                                if (theItem->flags & ITEM_IDENTIFIED) {
                                    sprintf(buf2, "It will reduce the damage of inbound attacks by a random amount between 0 and %i, which is %i%% of your current maximum health. (If the %s is enchanted, this maximum amount will %s %i.) ",
                                            (int) armorAbsorptionMax(enchant),
                                            (int) (100 * armorAbsorptionMax(enchant) / player.info.maxHP),
                                            theName,
                                            (armorAbsorptionMax(enchant) == armorAbsorptionMax(enchant + enchantIncrement(theItem)) ? "remain at" : "increase to"),
                                            (int) armorAbsorptionMax(enchant + enchantIncrement(theItem)));
                                } else {
                                    strcpy(buf2, "It will reduce the damage of inbound attacks by a random amount determined by its enchantment level. ");
                                }
                                break;
                            case A_REPRISAL:
                                if (theItem->flags & ITEM_IDENTIFIED) {
                                    sprintf(buf2, "Any enemy that attacks you will itself be wounded by %i%% of the damage that it inflicts. (If the %s is enchanted, this percentage will increase to %i%%.) ",
                                            armorReprisalPercent(enchant),
                                            theName,
                                            armorReprisalPercent(enchant + enchantIncrement(theItem)));
                                } else {
                                    strcpy(buf2, "Any enemy that attacks you will itself be wounded by a percentage (determined by enchantment level) of the damage that it inflicts. ");
                                }
                                break;
                            case A_IMMUNITY:
                                describeMonsterClass(buf3, theItem->vorpalEnemy, false);
                                sprintf(buf2, "It offers complete protection from any attacking %s. ",
                                        buf3);
                                break;
                            case A_REFLECTION:
                                if (theItem->flags & ITEM_IDENTIFIED) {
                                    if (theItem->enchant1 > 0) {
                                        short reflectChance = reflectionChance(enchant);
                                        short reflectChance2 = reflectionChance(enchant + enchantIncrement(theItem));
                                        sprintf(buf2, "When worn, you will deflect %i%% of incoming spells -- including directly back at their source %i%% of the time. (If the armor is enchanted, these will increase to %i%% and %i%%.) ",
                                                reflectChance,
                                                reflectChance * reflectChance / 100,
                                                reflectChance2,
                                                reflectChance2 * reflectChance2 / 100);
                                    } else if (theItem->enchant1 < 0) {
                                        short reflectChance = reflectionChance(enchant);
                                        short reflectChance2 = reflectionChance(enchant + enchantIncrement(theItem));
                                        sprintf(buf2, "When worn, %i%% of your own spells will deflect from their target -- including directly back at you %i%% of the time. (If the armor is enchanted, these will decrease to %i%% and %i%%.) ",
                                                reflectChance,
                                                reflectChance * reflectChance / 100,
                                                reflectChance2,
                                                reflectChance2 * reflectChance2 / 100);
                                    }
                                } else {
                                    strcpy(buf2, "When worn, you will deflect some percentage of incoming spells, determined by enchantment level. ");
                                }
                                break;
                            case A_RESPIRATION:
                                strcpy(buf2, "When worn, it will maintain a pocket of fresh air around you, rendering you immune to the effects of steam and all toxic gases. ");
                                break;
                            case A_DAMPENING:
                                strcpy(buf2, "When worn, it will safely absorb the concussive impact of any explosions (though you may still be burned). ");
                                break;
                            case A_BURDEN:
                                strcpy(buf2, "10% of the time it absorbs a blow, its strength requirement will permanently increase. ");
                                break;
                            case A_VULNERABILITY:
                                strcpy(buf2, "While it is worn, inbound attacks will inflict twice as much damage. ");
                                break;
                            case A_IMMOLATION:
                                strcpy(buf2, "10% of the time it absorbs a blow, it will explode in flames. ");
                                break;
                            default:
                                break;
                        }
                        strcat(buf, buf2);
                    } else if (theItem->flags & ITEM_IDENTIFIED) {
                        sprintf(buf2, "\n\nGlowing runes of an indecipherable language spiral around the %s. ",
                                theName);
                        strcat(buf, buf2);
                    }
                }

                // equipped? cursed?
                if (theItem->flags & ITEM_EQUIPPED) {
                    sprintf(buf2, "\n\nYou are wearing the %s%s. ",
                            theName,
                            ((theItem->flags & ITEM_CURSED) ? ", and because it is cursed, you are powerless to remove it" : ""));
                    strcat(buf, buf2);
                } else if (((theItem->flags & (ITEM_IDENTIFIED | ITEM_MAGIC_DETECTED)) || rogue.playbackOmniscience)
                           && (theItem->flags & ITEM_CURSED)) {
                    sprintf(buf2, "\n\n%sYou can feel a malevolent magic lurking within the %s.%s ",
                            badColorEscape,
                            theName,
                            whiteColorEscape);
                    strcat(buf, buf2);
                }

            }
            break;

        case STAFF:

            // charges
            new = apparentRingBonus(RING_WISDOM);
            if ((theItem->flags & ITEM_IDENTIFIED)  || rogue.playbackOmniscience) {
                sprintf(buf2, "\n\nThe %s has %i charges remaining out of a maximum of %i charges, and%s recovers a charge in approximately %i turns. ",
                        theName,
                        theItem->charges,
                        theItem->enchant1,
                        new == 0 ? "" : ", with your current rings,",
                        FP_DIV(staffChargeDuration(theItem), 10 * ringWisdomMultiplier(new * FP_FACTOR)));
                strcat(buf, buf2);
            } else if (theItem->flags & ITEM_MAX_CHARGES_KNOWN) {
                sprintf(buf2, "\n\nThe %s has a maximum of %i charges, and%s recovers a charge in approximately %i turns. ",
                        theName,
                        theItem->enchant1,
                        new == 0 ? "" : ", with your current rings,",
                        FP_DIV(staffChargeDuration(theItem), 10 * ringWisdomMultiplier(new * FP_FACTOR)));
                strcat(buf, buf2);
            }

            // effect description
            if (((theItem->flags & (ITEM_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN)) && staffTable[theItem->kind].identified)
                || rogue.playbackOmniscience) {
                switch (theItem->kind) {
                    case STAFF_LIGHTNING:
                        sprintf(buf2, "This staff deals damage to every creature in its line of fire; nothing is immune. (If the staff is enchanted, its average damage will increase by %i%%.)",
                                (int) (100 * (staffDamageLow(enchant + FP_FACTOR) + staffDamageHigh(enchant + FP_FACTOR)) / (staffDamageLow(enchant) + staffDamageHigh(enchant)) - 100));
                        break;
                    case STAFF_FIRE:
                        sprintf(buf2, "This staff deals damage to any creature that it hits, unless the creature is immune to fire. (If the staff is enchanted, its average damage will increase by %i%%.) It also sets creatures and flammable terrain on fire.",
                                (int) (100 * (staffDamageLow(enchant + FP_FACTOR) + staffDamageHigh(enchant + FP_FACTOR)) / (staffDamageLow(enchant) + staffDamageHigh(enchant)) - 100));
                        break;
                    case STAFF_POISON:
                        sprintf(buf2, "The bolt from this staff will poison any creature that it hits for %i turns. (If the staff is enchanted, this will increase to %i turns.)",
                                staffPoison(enchant),
                                staffPoison(enchant + FP_FACTOR));
                        break;
                    case STAFF_TUNNELING:
                        sprintf(buf2, "The bolt from this staff will dissolve %i layers of obstruction. (If the staff is enchanted, this will increase to %i layers.)",
                                theItem->enchant1,
                                theItem->enchant1 + 1);
                        break;
                    case STAFF_BLINKING:
                        sprintf(buf2, "This staff enables you to teleport up to %i spaces. (If the staff is enchanted, this will increase to %i spaces.)",
                                staffBlinkDistance(enchant),
                                staffBlinkDistance(enchant + FP_FACTOR));
                        break;
                    case STAFF_ENTRANCEMENT:
                        sprintf(buf2, "This staff will compel its target to mirror your movements for %i turns. (If the staff is enchanted, this will increase to %i turns.)",
                                staffEntrancementDuration(enchant),
                                staffEntrancementDuration(enchant + FP_FACTOR));
                        break;
                    case STAFF_HEALING:
                        if (enchant / FP_FACTOR < 10) {
                            sprintf(buf2, "This staff will heal its target by %i%% of its maximum health. (If the staff is enchanted, this will increase to %i%%.)",
                                    theItem->enchant1 * 10,
                                    (theItem->enchant1 + 1) * 10);
                        } else {
                            strcpy(buf2, "This staff will completely heal its target.");
                        }
                        break;
                    case STAFF_HASTE:
                        sprintf(buf2, "This staff will cause its target to move twice as fast for %i turns. (If the staff is enchanted, this will increase to %i turns.)",
                                staffHasteDuration(enchant),
                                staffHasteDuration(enchant + FP_FACTOR));
                        break;
                    case STAFF_OBSTRUCTION:
                        strcpy(buf2, "");
                        break;
                    case STAFF_DISCORD:
                        sprintf(buf2, "This staff will cause discord for %i turns. (If the staff is enchanted, this will increase to %i turns.)",
                                staffDiscordDuration(enchant),
                                staffDiscordDuration(enchant + FP_FACTOR));
                        break;
                    case STAFF_CONJURATION:
                        sprintf(buf2, "%i phantom blades will be called into service. (If the staff is enchanted, this will increase to %i blades.)",
                                staffBladeCount(enchant),
                                staffBladeCount(enchant + FP_FACTOR));
                        break;
                    case STAFF_PROTECTION:
                        sprintf(buf2, "This staff will shield a creature for up to 20 turns against up to %i damage. (If the staff is enchanted, this will increase to %i damage.)",
                                staffProtection(enchant) / 10,
                                staffProtection(enchant + FP_FACTOR) / 10);
                        break;
                    default:
                        strcpy(buf2, "No one knows what this staff does.");
                        break;
                }
                if (buf2[0]) {
                    strcat(buf, "\n\n");
                    strcat(buf, buf2);
                }
            }
            break;

        case WAND:
            strcat(buf, "\n\n");
            if ((theItem->flags & (ITEM_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN)) || rogue.playbackOmniscience) {
                if (theItem->charges) {
                    sprintf(buf2, "%i charge%s remain%s. Enchanting this wand will add %i charge%s.",
                            theItem->charges,
                            (theItem->charges == 1 ? "" : "s"),
                            (theItem->charges == 1 ? "s" : ""),
                            wandTable[theItem->kind].range.lowerBound,
                            (wandTable[theItem->kind].range.lowerBound == 1 ? "" : "s"));
                } else {
                    sprintf(buf2, "No charges remain.  Enchanting this wand will add %i charge%s.",
                            wandTable[theItem->kind].range.lowerBound,
                            (wandTable[theItem->kind].range.lowerBound == 1 ? "" : "s"));
                }
            } else {
                if (theItem->enchant2) {
                    sprintf(buf2, "You have used this wand %i time%s, but do not know how many charges, if any, remain.",
                            theItem->enchant2,
                            (theItem->enchant2 == 1 ? "" : "s"));
                } else {
                    strcpy(buf2, "You have not yet used this wand.");
                }

                if (wandTable[theItem->kind].identified) {
                    strcat(buf, buf2);
                    sprintf(buf2, " Wands of this type can be found with %i to %i charges. Enchanting this wand will add %i charge%s.",
                            wandTable[theItem->kind].range.lowerBound,
                            wandTable[theItem->kind].range.upperBound,
                            wandTable[theItem->kind].range.lowerBound,
                            (wandTable[theItem->kind].range.lowerBound == 1 ? "" : "s"));
                }
            }
            strcat(buf, buf2);
            break;

        case RING:
            if (((theItem->flags & ITEM_IDENTIFIED) && ringTable[theItem->kind].identified) || rogue.playbackOmniscience) {
                if (theItem->enchant1) {
                    switch (theItem->kind) {
                        case RING_CLAIRVOYANCE:
                            if (theItem->enchant1 > 0) {
                                sprintf(buf2, "\n\nThis ring provides magical sight with a radius of %i. (If the ring is enchanted, this will increase to %i.)",
                                        theItem->enchant1 + 1,
                                        theItem->enchant1 + 2);
                            } else {
                                sprintf(buf2, "\n\nThis ring magically blinds you to a radius of %i. (If the ring is enchanted, this will decrease to %i.)",
                                        (theItem->enchant1 * -1) + 1,
                                        (theItem->enchant1 * -1));
                            }
                            strcat(buf, buf2);
                            break;
                        case RING_REGENERATION:
                            sprintf(buf2, "\n\nWith this ring equipped, you will regenerate all of your health in %li turns (instead of %li). (If the ring is enchanted, this will decrease to %li turns.)",
                                    (long) (turnsForFullRegenInThousandths(enchant) / 1000),
                                    (long) TURNS_FOR_FULL_REGEN,
                                    (long) (turnsForFullRegenInThousandths(enchant + FP_FACTOR) / 1000));
                            strcat(buf, buf2);
                            break;
                        case RING_TRANSFERENCE:
                            sprintf(buf2, "\n\nDealing direct damage to a creature (whether in melee or otherwise) will %s you by %i%% of the damage dealt. (If the ring is enchanted, this will %s to %i%%.)",
                                    (theItem->enchant1 >= 0 ? "heal" : "harm"),
                                    abs(theItem->enchant1) * 5,
                                    (theItem->enchant1 >= 0 ? "increase" : "decrease"),
                                    abs(theItem->enchant1 + 1) * 5);
                            strcat(buf, buf2);
                            break;
                        case RING_WISDOM:
                            sprintf(buf2, "\n\nWhen worn, your staffs will recharge at %i%% of their normal rate. (If the ring is enchanted, the rate will increase to %i%% of the normal rate.)",
                                    (int) (100 * ringWisdomMultiplier(enchant) / FP_FACTOR),
                                    (int) (100 * ringWisdomMultiplier(enchant + FP_FACTOR) / FP_FACTOR));
                            strcat(buf, buf2);
                            break;
                        case RING_REAPING:
                            sprintf(buf2, "\n\nEach blow that you land with a weapon will %s your staffs and charms by 0-%i turns per point of damage dealt. (If the ring is enchanted, this will %s to 0-%i turns per point of damage.)",
                                    (theItem->enchant1 >= 0 ? "recharge" : "drain"),
                                    abs(theItem->enchant1),
                                    (theItem->enchant1 >= 0 ? "increase" : "decrease"),
                                    abs(theItem->enchant1 + 1));
                            strcat(buf, buf2);
                            break;
                        default:
                            break;
                    }
                }
            } else {
                sprintf(buf2, "\n\nIt will reveal its secrets if worn for %i%s turn%s",
                        theItem->charges,
                        (theItem->charges == RING_DELAY_TO_AUTO_ID ? "" : " more"),
                        (theItem->charges == 1 ? "" : "s"));
                strcat(buf, buf2);

                if ((theItem->charges < RING_DELAY_TO_AUTO_ID || (theItem->flags & (ITEM_MAGIC_DETECTED | ITEM_IDENTIFIED)))) {
                    sprintf(buf2, ", and until then it will function, at best, as a +%i ring.", theItem->timesEnchanted + 1);
                    strcat(buf, buf2);
                } else {
                    strcat(buf, ".");
                }
            }

            // equipped? cursed?
            if (theItem->flags & ITEM_EQUIPPED) {
                sprintf(buf2, "\n\nThe %s is on your finger%s. ",
                        theName,
                        ((theItem->flags & ITEM_CURSED) ? ", and because it is cursed, you are powerless to remove it" : ""));
                strcat(buf, buf2);
            } else if (((theItem->flags & (ITEM_IDENTIFIED | ITEM_MAGIC_DETECTED)) || rogue.playbackOmniscience)
                       && (theItem->flags & ITEM_CURSED)) {
                sprintf(buf2, "\n\n%sYou can feel a malevolent magic lurking within the %s.%s ",
                        badColorEscape,
                        theName,
                        whiteColorEscape);
                strcat(buf, buf2);
            }
            break;
        case CHARM:
            switch (theItem->kind) {
                case CHARM_HEALTH:
                    sprintf(buf2, "\n\nWhen used, the charm will heal %i%% of your health and recharge in %i turns. (If the charm is enchanted, it will heal %i%% of your health and recharge in %i turns.)",
                            charmHealing(enchant),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmHealing(enchant + FP_FACTOR),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + 1));
                    break;
                case CHARM_PROTECTION:
                    sprintf(buf2, "\n\nWhen used, the charm will shield you for up to 20 turns for up to %i%% of your total health and recharge in %i turns. (If the charm is enchanted, it will shield up to %i%% of your total health and recharge in %i turns.)",
                            100 * charmProtection(enchant) / 10 / player.info.maxHP,
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            100 * charmProtection(enchant + FP_FACTOR) / 10 / player.info.maxHP,
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + 1));
                    break;
                case CHARM_HASTE:
                    sprintf(buf2, "\n\nWhen used, the charm will haste you for %i turns and recharge in %i turns. (If the charm is enchanted, the haste will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmEffectDuration(theItem->kind, theItem->enchant1 + 1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + 1));
                    break;
                case CHARM_FIRE_IMMUNITY:
                    sprintf(buf2, "\n\nWhen used, the charm will grant you immunity to fire for %i turns and recharge in %i turns. (If the charm is enchanted, the immunity will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmEffectDuration(theItem->kind, theItem->enchant1 + 1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + 1));
                    break;
                case CHARM_INVISIBILITY:
                    sprintf(buf2, "\n\nWhen used, the charm will turn you invisible for %i turns and recharge in %i turns. While invisible, monsters more than two spaces away cannot track you. (If the charm is enchanted, the invisibility will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmEffectDuration(theItem->kind, theItem->enchant1 + 1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + 1));
                    break;
                case CHARM_TELEPATHY:
                    sprintf(buf2, "\n\nWhen used, the charm will grant you telepathy for %i turns and recharge in %i turns. (If the charm is enchanted, the telepathy will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmEffectDuration(theItem->kind, theItem->enchant1 + 1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + 1));
                    break;
                case CHARM_LEVITATION:
                    sprintf(buf2, "\n\nWhen used, the charm will lift you off the ground for %i turns and recharge in %i turns. (If the charm is enchanted, the levitation will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmEffectDuration(theItem->kind, theItem->enchant1 + 1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + 1));
                    break;
                case CHARM_SHATTERING:
                    sprintf(buf2, "\n\nWhen used, the charm will dissolve the nearby walls up to %i spaces away, and recharge in %i turns. (If the charm is enchanted, it will reach up to %i spaces and recharge in %i turns.)",
                            charmShattering(enchant),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmShattering(enchant + FP_FACTOR),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + 1));
                    break;
                case CHARM_GUARDIAN:
                    sprintf(buf2, "\n\nWhen used, a guardian will materialize for %i turns, and the charm will recharge in %i turns. (If the charm is enchanted, the guardian will last for %i turns and the charm will recharge in %i turns.)",
                            charmGuardianLifespan(enchant),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmGuardianLifespan(enchant + FP_FACTOR),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + 1));
                    break;
                case CHARM_TELEPORTATION:
                    sprintf(buf2, "\n\nWhen used, the charm will teleport you elsewhere in the dungeon and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + 1));
                    break;
                case CHARM_RECHARGING:
                    sprintf(buf2, "\n\nWhen used, the charm will recharge your staffs (though not your wands or charms), after which it will recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + 1));
                    break;
                case CHARM_NEGATION:
                    sprintf(buf2, "\n\nWhen used, the charm will negate all magical effects on the creatures in your field of view and the items on the ground up to %i spaces away, and recharge in %i turns. (If the charm is enchanted, it will reach up to %i spaces and recharge in %i turns.)",
                            charmNegationRadius(enchant),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmNegationRadius(enchant + FP_FACTOR),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + 1));
                    break;
                default:
                    break;
            }
            strcat(buf, buf2);
            break;
        default:
            break;
    }
}

boolean displayMagicCharForItem(item *theItem) {
    if (!(theItem->flags & ITEM_MAGIC_DETECTED)
        || (theItem->category & PRENAMED_CATEGORY)) {
        return false;
    } else {
        return true;
    }
}

char displayInventory(unsigned short categoryMask,
                      unsigned long requiredFlags,
                      unsigned long forbiddenFlags,
                      boolean waitForAcknowledge,
                      boolean includeButtons) {
    item *theItem;
    short i, j, m, maxLength = 0, itemNumber, itemCount, equippedItemCount;
    short extraLineCount = 0;
    item *itemList[DROWS];
    char buf[COLS*3];
    char theKey;
    rogueEvent theEvent;
    boolean magicDetected, repeatDisplay;
    short highlightItemLine, itemSpaceRemaining;
    cellDisplayBuffer dbuf[COLS][ROWS];
    cellDisplayBuffer rbuf[COLS][ROWS];
    brogueButton buttons[50] = {{{0}}};
    short actionKey = -1;
    color darkItemColor;

    char whiteColorEscapeSequence[20],
    grayColorEscapeSequence[20],
    yellowColorEscapeSequence[20],
    darkYellowColorEscapeSequence[20],
    goodColorEscapeSequence[20],
    badColorEscapeSequence[20];
    char *magicEscapePtr;

    assureCosmeticRNG;

    clearCursorPath();
    clearDisplayBuffer(dbuf);

    whiteColorEscapeSequence[0] = '\0';
    encodeMessageColor(whiteColorEscapeSequence, 0, &white);
    grayColorEscapeSequence[0] = '\0';
    encodeMessageColor(grayColorEscapeSequence, 0, &gray);
    yellowColorEscapeSequence[0] = '\0';
    encodeMessageColor(yellowColorEscapeSequence, 0, &itemColor);
    darkItemColor = itemColor;
    applyColorAverage(&darkItemColor, &black, 50);
    darkYellowColorEscapeSequence[0] = '\0';
    encodeMessageColor(darkYellowColorEscapeSequence, 0, &darkItemColor);
    goodColorEscapeSequence[0] = '\0';
    encodeMessageColor(goodColorEscapeSequence, 0, &goodMessageColor);
    badColorEscapeSequence[0] = '\0';
    encodeMessageColor(badColorEscapeSequence, 0, &badMessageColor);

    if (packItems->nextItem == NULL) {
        confirmMessages();
        message("Your pack is empty!", false);
        restoreRNG;
        return 0;
    }

    magicDetected = false;
    for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (displayMagicCharForItem(theItem) && (theItem->flags & ITEM_MAGIC_DETECTED)) {
            magicDetected = true;
        }
    }

    // List the items in the order we want to display them, with equipped items at the top.
    itemNumber = 0;
    equippedItemCount = 0;
    // First, the equipped weapon if any.
    if (rogue.weapon) {
        itemList[itemNumber] = rogue.weapon;
        itemNumber++;
        equippedItemCount++;
    }
    // Now, the equipped armor if any.
    if (rogue.armor) {
        itemList[itemNumber] = rogue.armor;
        itemNumber++;
        equippedItemCount++;
    }
    // Now, the equipped rings, if any.
    if (rogue.ringLeft) {
        itemList[itemNumber] = rogue.ringLeft;
        itemNumber++;
        equippedItemCount++;
    }
    if (rogue.ringRight) {
        itemList[itemNumber] = rogue.ringRight;
        itemNumber++;
        equippedItemCount++;
    }
    // Now all of the non-equipped items.
    for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (!(theItem->flags & ITEM_EQUIPPED)) {
            itemList[itemNumber] = theItem;
            itemNumber++;
        }
    }

    // Initialize the buttons:
    for (i=0; i < max(MAX_PACK_ITEMS, ROWS); i++) {
        buttons[i].y = mapToWindowY(i + (equippedItemCount && i >= equippedItemCount ? 1 : 0));
        buttons[i].buttonColor = black;
        buttons[i].opacity = INTERFACE_OPACITY;
        buttons[i].flags |= B_DRAW;
    }
    // Now prepare the buttons.
    const char closeParen = KEYBOARD_LABELS ? ')' : ' ';
    for (i=0; i<itemNumber; i++) {
        theItem = itemList[i];
        // Set button parameters for the item:
        buttons[i].flags |= (B_DRAW | B_GRADIENT | B_ENABLED);
        if (!waitForAcknowledge) {
            buttons[i].flags |= B_KEYPRESS_HIGHLIGHT;
        }
        buttons[i].hotkey[0] = theItem->inventoryLetter;
        buttons[i].hotkey[1] = theItem->inventoryLetter + 'A' - 'a';

        if ((theItem->category & categoryMask) &&
            !(~(theItem->flags) & requiredFlags) &&
            !(theItem->flags & forbiddenFlags)) {

            buttons[i].flags |= (B_HOVER_ENABLED);
        }

        // Set the text for the button:
        itemName(theItem, buf, true, true, (buttons[i].flags & B_HOVER_ENABLED) ? &white : &gray);
        upperCase(buf);

        if ((theItem->flags & ITEM_MAGIC_DETECTED)
            && !(theItem->category & AMULET)) { // Won't include food, keys, lumenstones or amulet.

            int polarity = itemMagicPolarity(theItem);
            if (polarity == 0) {
                buttons[i].symbol[0] = '-';
                magicEscapePtr = yellowColorEscapeSequence;
            } else if (polarity == 1) {
                buttons[i].symbol[0] = G_GOOD_MAGIC;
                magicEscapePtr = goodColorEscapeSequence;
            } else {
                buttons[i].symbol[0] = G_BAD_MAGIC;
                magicEscapePtr = badColorEscapeSequence;
            }

            // The first '*' is the magic detection symbol, e.g. '-' for non-magical.
            // The second '*' is the item character, e.g. ':' for food.
            sprintf(buttons[i].text, " %c%c %s* %s* %s%s%s%s",
                    KEYBOARD_LABELS ? theItem->inventoryLetter : ' ',
                    (theItem->flags & ITEM_PROTECTED ? '}' : closeParen),
                    magicEscapePtr,
                    (buttons[i].flags & B_HOVER_ENABLED) ? yellowColorEscapeSequence : darkYellowColorEscapeSequence,
                    (buttons[i].flags & B_HOVER_ENABLED) ? whiteColorEscapeSequence : grayColorEscapeSequence,
                    buf,
                    grayColorEscapeSequence,
                    (theItem->flags & ITEM_EQUIPPED ? ((theItem->category & WEAPON) ? " (in hand) " : " (worn) ") : ""));
            buttons[i].symbol[1] = theItem->displayChar;
        } else {
            sprintf(buttons[i].text, " %c%c %s%s* %s%s%s%s", // The '*' is the item character, e.g. ':' for food.
                    KEYBOARD_LABELS ? theItem->inventoryLetter : ' ',
                    (theItem->flags & ITEM_PROTECTED ? '}' : closeParen),
                    (magicDetected ? "  " : ""), // For proper spacing when this item is not detected but another is.
                    (buttons[i].flags & B_HOVER_ENABLED) ? yellowColorEscapeSequence : darkYellowColorEscapeSequence,
                    (buttons[i].flags & B_HOVER_ENABLED) ? whiteColorEscapeSequence : grayColorEscapeSequence,
                    buf,
                    grayColorEscapeSequence,
                    (theItem->flags & ITEM_EQUIPPED ? ((theItem->category & WEAPON) ? " (in hand) " : " (worn) ") : ""));
            buttons[i].symbol[0] = theItem->displayChar;
        }

        // Keep track of the maximum width needed:
        maxLength = max(maxLength, strLenWithoutEscapes(buttons[i].text));

        //      itemList[itemNumber] = theItem;
        //
        //      itemNumber++;
    }
    //printf("\nMaxlength: %i", maxLength);
    itemCount = itemNumber;
    if (!itemNumber) {
        confirmMessages();
        message("Nothing of that type!", false);
        restoreRNG;
        return 0;
    }
    if (waitForAcknowledge) {
        // Add the two extra lines as disabled buttons.
        itemSpaceRemaining = MAX_PACK_ITEMS - numberOfItemsInPack();
        if (itemSpaceRemaining) {
            sprintf(buttons[itemNumber + extraLineCount].text, "%s%s    You have room for %i more item%s.",
                    grayColorEscapeSequence,
                    (magicDetected ? "  " : ""),
                    itemSpaceRemaining,
                    (itemSpaceRemaining == 1 ? "" : "s"));
        } else {
            sprintf(buttons[itemNumber + extraLineCount].text, "%s%s    Your pack is full.",
                    grayColorEscapeSequence,
                    (magicDetected ? "  " : ""));
        }
        maxLength = max(maxLength, (strLenWithoutEscapes(buttons[itemNumber + extraLineCount].text)));
        extraLineCount++;

        sprintf(buttons[itemNumber + extraLineCount].text,
                KEYBOARD_LABELS ? "%s%s -- press (a-z) for more info -- " : "%s%s -- touch an item for more info -- ",
                grayColorEscapeSequence,
                (magicDetected ? "  " : ""));
        maxLength = max(maxLength, (strLenWithoutEscapes(buttons[itemNumber + extraLineCount].text)));
        extraLineCount++;
    }
    if (equippedItemCount) {
        // Add a separator button to fill in the blank line between equipped and unequipped items.
        sprintf(buttons[itemNumber + extraLineCount].text, "      %s%s---",
                (magicDetected ? "  " : ""),
                grayColorEscapeSequence);
        buttons[itemNumber + extraLineCount].y = mapToWindowY(equippedItemCount);
        extraLineCount++;
    }

    for (i=0; i < itemNumber + extraLineCount; i++) {

        // Position the button.
        buttons[i].x = COLS - maxLength;

        // Pad the button label with space, so the button reaches to the right edge of the screen.
        m = strlen(buttons[i].text);
        for (j=buttons[i].x + strLenWithoutEscapes(buttons[i].text); j < COLS; j++) {
            buttons[i].text[m] = ' ';
            m++;
        }
        buttons[i].text[m] = '\0';

        // Display the button. This would be redundant with the button loop,
        // except that we want the display to stick around until we get rid of it.
        drawButton(&(buttons[i]), BUTTON_NORMAL, dbuf);
    }

    // Add invisible previous and next buttons, so up and down arrows can select items.
    // Previous
    buttons[itemNumber + extraLineCount + 0].flags = B_ENABLED; // clear everything else
    buttons[itemNumber + extraLineCount + 0].hotkey[0] = NUMPAD_8;
    buttons[itemNumber + extraLineCount + 0].hotkey[1] = UP_ARROW;
    // Next
    buttons[itemNumber + extraLineCount + 1].flags = B_ENABLED; // clear everything else
    buttons[itemNumber + extraLineCount + 1].hotkey[0] = NUMPAD_2;
    buttons[itemNumber + extraLineCount + 1].hotkey[1] = DOWN_ARROW;

    overlayDisplayBuffer(dbuf, rbuf);

    do {
        repeatDisplay = false;

        // Do the button loop.
        highlightItemLine = -1;
        overlayDisplayBuffer(rbuf, NULL);   // Remove the inventory display while the buttons are active,
                                            // since they look the same and we don't want their opacities to stack.

        highlightItemLine = buttonInputLoop(buttons,
                                            itemCount + extraLineCount + 2, // the 2 is for up/down hotkeys
                                            COLS - maxLength,
                                            mapToWindowY(0),
                                            maxLength,
                                            itemNumber + extraLineCount,
                                            &theEvent);
        if (highlightItemLine == itemNumber + extraLineCount + 0) {
            // Up key
            highlightItemLine = itemNumber - 1;
            theEvent.shiftKey = true;
        } else if (highlightItemLine == itemNumber + extraLineCount + 1) {
            // Down key
            highlightItemLine = 0;
            theEvent.shiftKey = true;
        }

        if (highlightItemLine >= 0) {
            theKey = itemList[highlightItemLine]->inventoryLetter;
            theItem = itemList[highlightItemLine];
        } else {
            theKey = ESCAPE_KEY;
        }

        // Was an item selected?
        if (highlightItemLine > -1 && (waitForAcknowledge || theEvent.shiftKey || theEvent.controlKey)) {

            do {
                // Yes. Highlight the selected item. Do this by changing the button color and re-displaying it.

                overlayDisplayBuffer(dbuf, NULL);

                //buttons[highlightItemLine].buttonColor = interfaceBoxColor;
                drawButton(&(buttons[highlightItemLine]), BUTTON_PRESSED, NULL);
                //buttons[highlightItemLine].buttonColor = black;

                if (theEvent.shiftKey || theEvent.controlKey || waitForAcknowledge) {
                    // Display an information window about the item.
                    actionKey = printCarriedItemDetails(theItem, max(2, mapToWindowX(DCOLS - maxLength - 42)), mapToWindowY(2), 40, includeButtons, NULL);

                    overlayDisplayBuffer(rbuf, NULL); // remove the item info window

                    if (actionKey == -1) {
                        repeatDisplay = true;
                        overlayDisplayBuffer(dbuf, NULL); // redisplay the inventory
                    } else {
                        restoreRNG;
                        repeatDisplay = false;
                        overlayDisplayBuffer(rbuf, NULL); // restore the original screen
                    }

                    switch (actionKey) {
                        case APPLY_KEY:
                            apply(theItem, true);
                            break;
                        case EQUIP_KEY:
                            equip(theItem);
                            break;
                        case UNEQUIP_KEY:
                            unequip(theItem);
                            break;
                        case DROP_KEY:
                            drop(theItem);
                            break;
                        case THROW_KEY:
                            throwCommand(theItem, false);
                            break;
                        case RELABEL_KEY:
                            relabel(theItem);
                            break;
                        case CALL_KEY:
                            call(theItem);
                            break;
                        case UP_KEY:
                            highlightItemLine = highlightItemLine - 1;
                            if (highlightItemLine < 0) {
                                highlightItemLine = itemNumber - 1;
                            }
                            break;
                        case DOWN_KEY:
                            highlightItemLine = highlightItemLine + 1;
                            if (highlightItemLine >= itemNumber) {
                                highlightItemLine = 0;
                            }
                            break;
                        default:
                            break;
                    }

                    if (actionKey == UP_KEY || actionKey == DOWN_KEY) {
                        theKey = itemList[highlightItemLine]->inventoryLetter;
                        theItem = itemList[highlightItemLine];
                    } else if (actionKey > -1) {
                        // Player took an action directly from the item screen; we're done here.
                        restoreRNG;
                        return 0;
                    }
                }
            } while (actionKey == UP_KEY || actionKey == DOWN_KEY);
        }
    } while (repeatDisplay); // so you can get info on multiple items sequentially

    overlayDisplayBuffer(rbuf, NULL); // restore the original screen

    restoreRNG;
    return theKey;
}

short numberOfMatchingPackItems(unsigned short categoryMask,
                                unsigned long requiredFlags, unsigned long forbiddenFlags,
                                boolean displayErrors) {
    item *theItem;
    short matchingItemCount = 0;

    if (packItems->nextItem == NULL) {
        if (displayErrors) {
            confirmMessages();
            message("Your pack is empty!", false);
        }
        return 0;
    }

    for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {

        if (theItem->category & categoryMask &&
            !(~(theItem->flags) & requiredFlags) &&
            !(theItem->flags & forbiddenFlags)) {

            matchingItemCount++;
        }
    }

    if (matchingItemCount == 0) {
        if (displayErrors) {
            confirmMessages();
            message("You have nothing suitable.", false);
        }
        return 0;
    }

    return matchingItemCount;
}

void updateEncumbrance() {
    short moveSpeed, attackSpeed;

    moveSpeed = player.info.movementSpeed;
    attackSpeed = player.info.attackSpeed;

    if (player.status[STATUS_HASTED]) {
        moveSpeed /= 2;
        attackSpeed /= 2;
    } else if (player.status[STATUS_SLOWED]) {
        moveSpeed *= 2;
        attackSpeed *= 2;
    }

    player.movementSpeed = moveSpeed;
    player.attackSpeed = attackSpeed;

    recalculateEquipmentBonuses();
}

short displayedArmorValue() {
    if (!rogue.armor || (rogue.armor->flags & ITEM_IDENTIFIED)) {
        return player.info.defense / 10;
    } else {
        return ((armorTable[rogue.armor->kind].range.upperBound + armorTable[rogue.armor->kind].range.lowerBound) * FP_FACTOR / 2 / 10
                + strengthModifier(rogue.armor)) / FP_FACTOR;
    }
}

void strengthCheck(item *theItem) {
    char buf1[COLS], buf2[COLS*2];
    short strengthDeficiency;

    updateEncumbrance();
    if (theItem) {
        if (theItem->category & WEAPON && theItem->strengthRequired > rogue.strength - player.weaknessAmount) {
            strengthDeficiency = theItem->strengthRequired - max(0, rogue.strength - player.weaknessAmount);
            strcpy(buf1, "");
            itemName(theItem, buf1, false, false, NULL);
            sprintf(buf2, "You can barely lift the %s; %i more strength would be ideal.", buf1, strengthDeficiency);
            message(buf2, false);
        }

        if (theItem->category & ARMOR && theItem->strengthRequired > rogue.strength - player.weaknessAmount) {
            strengthDeficiency = theItem->strengthRequired - max(0, rogue.strength - player.weaknessAmount);
            strcpy(buf1, "");
            itemName(theItem, buf1, false, false, NULL);
            sprintf(buf2, "You stagger under the weight of the %s; %i more strength would be ideal.",
                    buf1, strengthDeficiency);
            message(buf2, false);
        }
    }
}

boolean canEquip(item *theItem) {
    item *previouslyEquippedItem = NULL;

    if (theItem->category & WEAPON) {
        previouslyEquippedItem = rogue.weapon;
    } else if (theItem->category & ARMOR) {
        previouslyEquippedItem = rogue.armor;
    }
    if (previouslyEquippedItem && (previouslyEquippedItem->flags & ITEM_CURSED)) {
        return false; // already using a cursed item
    }

    if ((theItem->category & RING) && rogue.ringLeft && rogue.ringRight) {
        return false;
    }
    return true;
}

// Will prompt for an item if none is given.
// Equips the item and records input if successful.
// Player's failure to select an item will result in failure.
// Failure does not record input.
void equip(item *theItem) {
    char buf1[COLS * 3], buf2[COLS * 3];
    unsigned char command[10];
    short c = 0;
    item *theItem2;

    command[c++] = EQUIP_KEY;
    if (!theItem) {
        theItem = promptForItemOfType((WEAPON|ARMOR|RING), 0, ITEM_EQUIPPED,
                                      KEYBOARD_LABELS ? "Equip what? (a-z, shift for more info; or <esc> to cancel)" : "Equip what?", true);
    }
    if (theItem == NULL) {
        return;
    }

    command[c++] = theItem->inventoryLetter;

    if (theItem->category & (WEAPON|ARMOR|RING)) {

        if (theItem->category & RING) {
            if (theItem->flags & ITEM_EQUIPPED) {
                confirmMessages();
                message("you are already wearing that ring.", false);
                return;
            } else if (rogue.ringLeft && rogue.ringRight) {
                confirmMessages();
                theItem2 = promptForItemOfType((RING), ITEM_EQUIPPED, 0,
                                               "You are already wearing two rings; remove which first?", true);
                if (!theItem2 || theItem2->category != RING || !(theItem2->flags & ITEM_EQUIPPED)) {
                    if (theItem2) { // No message if canceled or did an inventory action instead.
                        message("Invalid entry.", false);
                    }
                    return;
                } else {
                    if (theItem2->flags & ITEM_CURSED) {
                        itemName(theItem2, buf1, false, false, NULL);
                        sprintf(buf2, "You can't remove your %s: it appears to be cursed.", buf1);
                        confirmMessages();
                        messageWithColor(buf2, &itemMessageColor, false);
                        return;
                    }
                    unequipItem(theItem2, false);
                    command[c++] = theItem2->inventoryLetter;
                }
            }
        }

        if (theItem->flags & ITEM_EQUIPPED) {
            confirmMessages();
            message("already equipped.", false);
            return;
        }

        if (!canEquip(theItem)) {
            // equip failed because current item is cursed
            if (theItem->category & WEAPON) {
                itemName(rogue.weapon, buf1, false, false, NULL);
            } else if (theItem->category & ARMOR) {
                itemName(rogue.armor, buf1, false, false, NULL);
            } else {
                sprintf(buf1, "one");
            }
            sprintf(buf2, "You can't; the %s you are using appears to be cursed.", buf1);
            confirmMessages();
            messageWithColor(buf2, &itemMessageColor, false);
            return;
        }
        command[c] = '\0';
        recordKeystrokeSequence(command);


        equipItem(theItem, false);

        itemName(theItem, buf2, true, true, NULL);
        sprintf(buf1, "Now %s %s.", (theItem->category & WEAPON ? "wielding" : "wearing"), buf2);
        confirmMessages();
        messageWithColor(buf1, &itemMessageColor, false);

        strengthCheck(theItem);

        if (theItem->flags & ITEM_CURSED) {
            itemName(theItem, buf2, false, false, NULL);
            switch(theItem->category) {
                case WEAPON:
                    sprintf(buf1, "you wince as your grip involuntarily tightens around your %s.", buf2);
                    break;
                case ARMOR:
                    sprintf(buf1, "your %s constricts around you painfully.", buf2);
                    break;
                case RING:
                    sprintf(buf1, "your %s tightens around your finger painfully.", buf2);
                    break;
                default:
                    sprintf(buf1, "your %s seizes you with a malevolent force.", buf2);
                    break;
            }
            messageWithColor(buf1, &itemMessageColor, false);
        }
        playerTurnEnded();
    } else {
        confirmMessages();
        message("You can't equip that.", false);
    }
}

// Returns whether the given item is a key that can unlock the given location.
// An item qualifies if:
// (1) it's a key (has ITEM_IS_KEY flag),
// (2) its originDepth matches the depth, and
// (3) either its key (x, y) location matches (x, y), or its machine number matches the machine number at (x, y).
boolean keyMatchesLocation(item *theItem, short x, short y) {
    short i;

    if ((theItem->flags & ITEM_IS_KEY)
        && theItem->originDepth == rogue.depthLevel) {

        for (i=0; i < KEY_ID_MAXIMUM && (theItem->keyLoc[i].x || theItem->keyLoc[i].machine); i++) {
            if (theItem->keyLoc[i].x == x && theItem->keyLoc[i].y == y) {
                return true;
            } else if (theItem->keyLoc[i].machine == pmap[x][y].machineNumber) {
                return true;
            }
        }
    }
    return false;
}

item *keyInPackFor(short x, short y) {
    item *theItem;

    for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (keyMatchesLocation(theItem, x, y)) {
            return theItem;
        }
    }
    return NULL;
}

item *keyOnTileAt(short x, short y) {
    item *theItem;
    creature *monst;

    if ((pmap[x][y].flags & HAS_PLAYER)
        && player.xLoc == x
        && player.yLoc == y
        && keyInPackFor(x, y)) {

        return keyInPackFor(x, y);
    }
    if (pmap[x][y].flags & HAS_ITEM) {
        theItem = itemAtLoc(x, y);
        if (keyMatchesLocation(theItem, x, y)) {
            return theItem;
        }
    }
    if (pmap[x][y].flags & HAS_MONSTER) {
        monst = monsterAtLoc(x, y);
        if (monst->carriedItem) {
            theItem = monst->carriedItem;
            if (keyMatchesLocation(theItem, x, y)) {
                return theItem;
            }
        }
    }
    return NULL;
}

// Aggroes out to the given distance.
void aggravateMonsters(short distance, short x, short y, const color *flashColor) {
    creature *monst;
    short i, j, **grid;

    rogue.wpCoordinates[0][0] = x;
    rogue.wpCoordinates[0][1] = y;
    refreshWaypoint(0);

    grid = allocGrid();
    fillGrid(grid, 0);
    calculateDistances(grid, x, y, T_PATHING_BLOCKER, NULL, true, false);

    for (monst=monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
        if (grid[monst->xLoc][monst->yLoc] <= distance) {
            if (monst->creatureState == MONSTER_SLEEPING) {
                wakeUp(monst);
            }
            if (monst->creatureState != MONSTER_ALLY && monst->leader != &player) {
                alertMonster(monst);
                monst->info.flags &= ~MONST_MAINTAINS_DISTANCE;
                monst->info.abilityFlags &= ~MA_AVOID_CORRIDORS;
            }
        }
    }
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (grid[i][j] >= 0 && grid[i][j] <= distance) {
                scentMap[i][j] = 0;
                addScentToCell(i, j, 2 * grid[i][j]);
            }
        }
    }

    if (player.xLoc == x && player.yLoc == y) {
        player.status[STATUS_AGGRAVATING] = player.maxStatus[STATUS_AGGRAVATING] = distance;
        rogue.aggroRange = currentAggroValue();
    }

    if (grid[player.xLoc][player.yLoc] >= 0 && grid[player.xLoc][player.yLoc] <= distance) {
        discover(x, y);
        discoverCell(x, y);
        colorFlash(flashColor, 0, (DISCOVERED | MAGIC_MAPPED), 10, distance, x, y);
        if (!playerCanSee(x, y)) {
            message("You hear a piercing shriek; something must have triggered a nearby alarm.", false);
        }
    }

    freeGrid(grid);
}

// Simple line algorithm (maybe this is Bresenham?) that returns a list of coordinates
// that extends all the way to the edge of the map based on an originLoc (which is not included
// in the list of coordinates) and a targetLoc.
// Returns the number of entries in the list, and includes (-1, -1) as an additional
// terminus indicator after the end of the list.
short getLineCoordinates(short listOfCoordinates[][2], const short originLoc[2], const short targetLoc[2]) {
    fixpt targetVector[2], error[2], largerTargetComponent;
    short currentVector[2], previousVector[2], quadrantTransform[2], i;
    short currentLoc[2];
    short cellNumber = 0;

    if (originLoc[0] == targetLoc[0] && originLoc[1] == targetLoc[1]) {
        return 0;
    }

    // Neither vector is negative. We keep track of negatives with quadrantTransform.
    for (i=0; i<= 1; i++) {
        targetVector[i] = (targetLoc[i] - originLoc[i]) * FP_FACTOR;
        if (targetVector[i] < 0) {
            targetVector[i] *= -1;
            quadrantTransform[i] = -1;
        } else {
            quadrantTransform[i] = 1;
        }
        currentVector[i] = previousVector[i] = error[i] = 0;
        currentLoc[i] = originLoc[i];
    }

    // normalize target vector such that one dimension equals 1 and the other is in [0, 1].
    largerTargetComponent = max(targetVector[0], targetVector[1]);
    targetVector[0] = (targetVector[0] * FP_FACTOR) / largerTargetComponent;
    targetVector[1] = (targetVector[1] * FP_FACTOR) / largerTargetComponent;

    do {
        for (i=0; i<= 1; i++) {

            currentVector[i] += targetVector[i] / FP_FACTOR;
            error[i] += (targetVector[i] == FP_FACTOR ? 0 : targetVector[i]);

            if (error[i] >= FP_FACTOR / 2) {
                currentVector[i]++;
                error[i] -= FP_FACTOR;
            }

            currentLoc[i] = quadrantTransform[i]*currentVector[i] + originLoc[i];

            listOfCoordinates[cellNumber][i] = currentLoc[i];
        }

        //DEBUG printf("\ncell %i: (%i, %i)", cellNumber, listOfCoordinates[cellNumber][0], listOfCoordinates[cellNumber][1]);
        cellNumber++;

    } while (coordinatesAreInMap(currentLoc[0], currentLoc[1]));

    cellNumber--;

    listOfCoordinates[cellNumber][0] = listOfCoordinates[cellNumber][1] = -1; // demarcates the end of the list
    return cellNumber;
}

// If a hypothetical bolt were launched from originLoc toward targetLoc,
// with a given max distance and a toggle as to whether it halts at its impact location
// or one space prior, where would it stop?
// Takes into account the caster's knowledge; i.e. won't be blocked by monsters
// that the caster is not aware of.
void getImpactLoc(short returnLoc[2], const short originLoc[2], const short targetLoc[2],
                  const short maxDistance, const boolean returnLastEmptySpace) {
    short coords[DCOLS + 1][2];
    short i, n;
    creature *monst;

    n = getLineCoordinates(coords, originLoc, targetLoc);
    n = min(n, maxDistance);
    for (i=0; i<n; i++) {
        monst = monsterAtLoc(coords[i][0], coords[i][1]);
        if (monst
            && !monsterIsHidden(monst, monsterAtLoc(originLoc[0], originLoc[1]))
            && !(monst->bookkeepingFlags & MB_SUBMERGED)) {
            // Imaginary bolt hit the player or a monster.
            break;
        }
        if (cellHasTerrainFlag(coords[i][0], coords[i][1], (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))) {
            break;
        }
    }
    if (i == maxDistance) {
        returnLoc[0] = coords[i-1][0];
        returnLoc[1] = coords[i-1][1];
    } else if (returnLastEmptySpace) {
        if (i == 0) {
            returnLoc[0] = originLoc[0];
            returnLoc[1] = originLoc[1];
        } else {
            returnLoc[0] = coords[i-1][0];
            returnLoc[1] = coords[i-1][1];
        }
    } else {
        returnLoc[0] = coords[i][0];
        returnLoc[1] = coords[i][1];
    }
    brogueAssert(coordinatesAreInMap(returnLoc[0], returnLoc[1]));
}

// Returns true if the two coordinates are unobstructed and diagonally adjacent,
// but their two common neighbors are obstructed and at least one blocks diagonal movement.
boolean impermissibleKinkBetween(short x1, short y1, short x2, short y2) {
    brogueAssert(coordinatesAreInMap(x1, y1));
    brogueAssert(coordinatesAreInMap(x2, y2));
    if (cellHasTerrainFlag(x1, y1, T_OBSTRUCTS_PASSABILITY)
        || cellHasTerrainFlag(x2, y2, T_OBSTRUCTS_PASSABILITY)) {
        // One of the two locations is obstructed.
        return false;
    }
    if (abs(x1 - x2) != 1
        || abs(y1 - y2) != 1) {
        // Not diagonally adjacent.
        return false;
    }
    if (!cellHasTerrainFlag(x2, y1, T_OBSTRUCTS_PASSABILITY)
        || !cellHasTerrainFlag(x1, y2, T_OBSTRUCTS_PASSABILITY)) {
        // At least one of the common neighbors isn't obstructed.
        return false;
    }
    if (!cellHasTerrainFlag(x2, y1, T_OBSTRUCTS_DIAGONAL_MOVEMENT)
        && !cellHasTerrainFlag(x1, y2, T_OBSTRUCTS_DIAGONAL_MOVEMENT)) {
        // Neither of the common neighbors obstructs diagonal movement.
        return false;
    }
    return true;
}

boolean tunnelize(short x, short y) {
    enum dungeonLayers layer;
    boolean didSomething = false;
    creature *monst;
    short x2, y2;
    enum directions dir;

    if (pmap[x][y].flags & IMPREGNABLE) {
        return false;
    }
    freeCaptivesEmbeddedAt(x, y);
    if (x == 0 || x == DCOLS - 1 || y == 0 || y == DROWS - 1) {
        pmap[x][y].layers[DUNGEON] = CRYSTAL_WALL; // don't dissolve the boundary walls
        didSomething = true;
    } else {
        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmap[x][y].layers[layer]].flags & (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION)) {
                pmap[x][y].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                didSomething = true;
            }
        }
    }
    if (didSomething) {
        spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_TUNNELIZE], true, false);
        if (pmap[x][y].flags & HAS_MONSTER) {
            // Kill turrets and sentinels if you tunnelize them.
            monst = monsterAtLoc(x, y);
            if (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS) {
                inflictLethalDamage(NULL, monst);
            }
        }
    }
    if (!cellHasTerrainFlag(x, y, T_OBSTRUCTS_DIAGONAL_MOVEMENT)
        && didSomething) {
        // Tunnel out any diagonal kinks between walls.
        for (dir = 0; dir < DIRECTION_COUNT; dir++) {
            x2 = x + nbDirs[dir][0];
            y2 = y + nbDirs[dir][1];
            if (coordinatesAreInMap(x2, y2)
                && impermissibleKinkBetween(x, y, x2, y2)) {

                if ((pmap[x][y2].flags & IMPREGNABLE)
                    || (!(pmap[x2][y].flags & IMPREGNABLE) && rand_percent(50))) {

                    tunnelize(x2, y);
                } else {
                    tunnelize(x, y2);
                }
            }
        }
    }
    return didSomething;
}

void negate(creature *monst) {
    short i, j;
    enum boltType backupBolts[20];
    monst->info.abilityFlags &= MA_NON_NEGATABLE_ABILITIES; // negated monsters lose all special abilities
    monst->bookkeepingFlags &= ~MB_SEIZING;

    if (monst->info.flags & MONST_DIES_IF_NEGATED) {
        char buf[DCOLS * 3], monstName[DCOLS];
        monsterName(monstName, monst, true);
        if (monst->status[STATUS_LEVITATING]) {
            sprintf(buf, "%s dissipates into thin air", monstName);
        } else if (monst->info.flags & MONST_INANIMATE) {
            sprintf(buf, "%s shatters into tiny pieces", monstName);
        } else {
            sprintf(buf, "%s falls to the ground, lifeless", monstName);
        }
        killCreature(monst, false);
        combatMessage(buf, messageColorFromVictim(monst));
    } else if (!(monst->info.flags & MONST_INVULNERABLE)) {
        // works on inanimates
        monst->status[STATUS_IMMUNE_TO_FIRE] = 0;
        monst->status[STATUS_SLOWED] = 0;
        monst->status[STATUS_HASTED] = 0;
        monst->status[STATUS_CONFUSED] = 0;
        monst->status[STATUS_ENTRANCED] = 0;
        monst->status[STATUS_DISCORDANT] = 0;
        monst->status[STATUS_SHIELDED] = 0;
        monst->status[STATUS_INVISIBLE] = 0;
        if (monst == &player) {
            monst->status[STATUS_TELEPATHIC] = min(monst->status[STATUS_TELEPATHIC], 1);
            monst->status[STATUS_MAGICAL_FEAR] = min(monst->status[STATUS_MAGICAL_FEAR], 1);
            monst->status[STATUS_LEVITATING] = min(monst->status[STATUS_LEVITATING], 1);
            if (monst->status[STATUS_DARKNESS]) {
                monst->status[STATUS_DARKNESS] = 0;
                updateMinersLightRadius();
                updateVision(true);
            }
        } else {
            monst->status[STATUS_TELEPATHIC] = 0;
            monst->status[STATUS_MAGICAL_FEAR] = 0;
            monst->status[STATUS_LEVITATING] = 0;
        }
        monst->info.flags &= ~MONST_IMMUNE_TO_FIRE;
        monst->movementSpeed = monst->info.movementSpeed;
        monst->attackSpeed = monst->info.attackSpeed;
        if (monst != &player && (monst->info.flags & NEGATABLE_TRAITS)) {
            if ((monst->info.flags & MONST_FIERY) && monst->status[STATUS_BURNING]) {
                extinguishFireOnCreature(monst);
            }
            monst->info.flags &= ~NEGATABLE_TRAITS;
            refreshDungeonCell(monst->xLoc, monst->yLoc);
            refreshSideBar(-1, -1, false);
        }
        for (i = 0; i < 20; i++) {
            backupBolts[i] = monst->info.bolts[i];
            monst->info.bolts[i] = BOLT_NONE;
        }
        for (i = 0, j = 0; i < 20 && backupBolts[i]; i++) {
            if (boltCatalog[backupBolts[i]].flags & BF_NOT_NEGATABLE) {
                monst->info.bolts[j] = backupBolts[i];
                j++;
            }
        }
        monst->newPowerCount = monst->totalPowerCount; // Allies can re-learn lost ability slots.
        applyInstantTileEffectsToCreature(monst); // in case it should immediately die or fall into a chasm
    }
}

// Adds one to the creature's weakness, sets the weakness status duration to maxDuration.
void weaken(creature *monst, short maxDuration) {
    if (monst->weaknessAmount < 10) {
        monst->weaknessAmount++;
    }
    monst->status[STATUS_WEAKENED] = max(monst->status[STATUS_WEAKENED], maxDuration);
    monst->maxStatus[STATUS_WEAKENED] = max(monst->maxStatus[STATUS_WEAKENED], maxDuration);
    if (monst == &player) {
        messageWithColor("your muscles weaken as an enervating toxin fills your veins.", &badMessageColor, false);
        strengthCheck(rogue.weapon);
        strengthCheck(rogue.armor);
    }
}

// True if the creature polymorphed; false if not.
boolean polymorph(creature *monst) {
    short previousDamageTaken, healthFraction, newMonsterIndex;

    if (monst == &player || (monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
        return false; // Sorry, this is not Nethack.
    }

    if (monst->creatureState == MONSTER_FLEEING
        && (monst->info.flags & (MONST_MAINTAINS_DISTANCE | MONST_FLEES_NEAR_DEATH)) || (monst->info.abilityFlags & MA_HIT_STEAL_FLEE)) {

        monst->creatureState = MONSTER_TRACKING_SCENT;
        monst->creatureMode = MODE_NORMAL;
    }

    unAlly(monst); // Sorry, no cheap dragon allies.
    monst->mutationIndex = -1; // Polymorph cures mutation -- basic science.
    healthFraction = monst->currentHP * 1000 / monst->info.maxHP;
    previousDamageTaken = monst->info.maxHP - monst->currentHP;

    do {
        newMonsterIndex = rand_range(1, NUMBER_MONSTER_KINDS - 1);
    } while (monsterCatalog[newMonsterIndex].flags & (MONST_INANIMATE | MONST_NO_POLYMORPH) // Can't turn something into an inanimate object or lich/phoenix/warden.
             || newMonsterIndex == monst->info.monsterID); // Can't stay the same monster.
    monst->info = monsterCatalog[newMonsterIndex]; // Presto change-o!

    monst->info.turnsBetweenRegen *= 1000;
    monst->currentHP = max(1, max(healthFraction * monst->info.maxHP / 1000, monst->info.maxHP - previousDamageTaken));

    monst->movementSpeed = monst->info.movementSpeed;
    monst->attackSpeed = monst->info.attackSpeed;
    if (monst->status[STATUS_HASTED]) {
        monst->movementSpeed /= 2;
        monst->attackSpeed /= 2;
    }
    if (monst->status[STATUS_SLOWED]) {
        monst->movementSpeed *= 2;
        monst->attackSpeed *= 2;
    }

    clearStatus(monst);

    if (monst->info.flags & MONST_FIERY) {
        monst->status[STATUS_BURNING] = monst->maxStatus[STATUS_BURNING] = 1000; // won't decrease
    }
    if (monst->info.flags & MONST_FLIES) {
        monst->status[STATUS_LEVITATING] = monst->maxStatus[STATUS_LEVITATING] = 1000; // won't decrease
    }
    if (monst->info.flags & MONST_IMMUNE_TO_FIRE) {
        monst->status[STATUS_IMMUNE_TO_FIRE] = monst->maxStatus[STATUS_IMMUNE_TO_FIRE] = 1000; // won't decrease
    }
    if (monst->info.flags & MONST_INVISIBLE) {
        monst->status[STATUS_INVISIBLE] = monst->maxStatus[STATUS_INVISIBLE] = 1000; // won't decrease
    }
    monst->status[STATUS_NUTRITION] = monst->maxStatus[STATUS_NUTRITION] = 1000;

    if (monst->bookkeepingFlags & MB_CAPTIVE) {
        demoteMonsterFromLeadership(monst);
        monst->creatureState = MONSTER_TRACKING_SCENT;
        monst->bookkeepingFlags &= ~MB_CAPTIVE;
    }
    monst->bookkeepingFlags &= ~(MB_SEIZING | MB_SEIZED);

    monst->ticksUntilTurn = max(monst->ticksUntilTurn, 101);

    refreshDungeonCell(monst->xLoc, monst->yLoc);
    if (boltCatalog[BOLT_POLYMORPH].backColor) {
        flashMonster(monst, boltCatalog[BOLT_POLYMORPH].backColor, 100);
    }
    return true;
}

void slow(creature *monst, short turns) {
    if (!(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
        monst->status[STATUS_SLOWED] = monst->maxStatus[STATUS_SLOWED] = turns;
        monst->status[STATUS_HASTED] = 0;
        if (monst == &player) {
            updateEncumbrance();
            message("you feel yourself slow down.", false);
        } else {
            monst->movementSpeed = monst->info.movementSpeed * 2;
            monst->attackSpeed = monst->info.attackSpeed * 2;
        }
    }
}

void haste(creature *monst, short turns) {
    if (monst && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
        monst->status[STATUS_SLOWED] = 0;
        monst->status[STATUS_HASTED] = monst->maxStatus[STATUS_HASTED] = turns;
        if (monst == &player) {
            updateEncumbrance();
            message("you feel yourself speed up.", false);
        } else {
            monst->movementSpeed = monst->info.movementSpeed / 2;
            monst->attackSpeed = monst->info.attackSpeed / 2;
        }
    }
}

void heal(creature *monst, short percent, boolean panacea) {
    char buf[COLS], monstName[COLS];
    monst->currentHP = min(monst->info.maxHP, monst->currentHP + percent * monst->info.maxHP / 100);
    if (panacea) {
        if (monst->status[STATUS_HALLUCINATING] > 1) {
            monst->status[STATUS_HALLUCINATING] = 1;
        }
        if (monst->status[STATUS_CONFUSED] > 1) {
            monst->status[STATUS_CONFUSED] = 1;
        }
        if (monst->status[STATUS_NAUSEOUS] > 1) {
            monst->status[STATUS_NAUSEOUS] = 1;
        }
        if (monst->status[STATUS_SLOWED] > 1) {
            monst->status[STATUS_SLOWED] = 1;
        }
        if (monst->status[STATUS_WEAKENED] > 1) {
            monst->weaknessAmount = 0;
            monst->status[STATUS_WEAKENED] = 0;
            updateEncumbrance();
        }
        if (monst->status[STATUS_POISONED]) {
            monst->poisonAmount = 0;
            monst->status[STATUS_POISONED] = 0;
        }
        if (monst->status[STATUS_DARKNESS] > 0) {
            monst->status[STATUS_DARKNESS] = 0;
            if (monst == &player) {
                updateMinersLightRadius();
                updateVision(true);
            }
        }
    }
    if (canDirectlySeeMonster(monst)
        && monst != &player
        && !panacea) {

        monsterName(monstName, monst, true);
        sprintf(buf, "%s looks healthier", monstName);
        combatMessage(buf, NULL);
    }
}

void makePlayerTelepathic(short duration) {
    creature *monst;

    player.status[STATUS_TELEPATHIC] = player.maxStatus[STATUS_TELEPATHIC] = duration;
    for (monst=monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
        refreshDungeonCell(monst->xLoc, monst->yLoc);
    }
    if (monsters->nextCreature == NULL) {
        message("you can somehow tell that you are alone on this depth at the moment.", false);
    } else {
        message("you can somehow feel the presence of other creatures' minds!", false);
    }
}

void rechargeItems(unsigned long categories) {
    item *tempItem;
    short x, y, z, i, categoryCount;
    char buf[DCOLS * 3];

    x = y = z = 0; // x counts staffs, y counts wands, z counts charms
    for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
        if (tempItem->category & categories & STAFF) {
            x++;
            tempItem->charges = tempItem->enchant1;
            tempItem->enchant2 = (tempItem->kind == STAFF_BLINKING || tempItem->kind == STAFF_OBSTRUCTION ? 10000 : 5000) / tempItem->enchant1;
        }
        if (tempItem->category & categories & WAND) {
            y++;
            tempItem->charges++;
        }
        if (tempItem->category & categories & CHARM) {
            z++;
            tempItem->charges = 0;
        }
    }

    categoryCount = (x ? 1 : 0) + (y ? 1 : 0) + (z ? 1 : 0);

    if (categoryCount) {
        i = 0;
        strcpy(buf, "a surge of energy courses through your pack, recharging your ");
        if (x) {
            i++;
            strcat(buf, x == 1 ? "staff" : "staffs");
            if (i == categoryCount - 1) {
                strcat(buf, " and ");
            } else if (i <= categoryCount - 2) {
                strcat(buf, ", ");
            }
        }
        if (y) {
            i++;
            strcat(buf, y == 1 ? "wand" : "wands");
            if (i == categoryCount - 1) {
                strcat(buf, " and ");
            } else if (i <= categoryCount - 2) {
                strcat(buf, ", ");
            }
        }
        if (z) {
            strcat(buf, z == 1 ? "charm" : "charms");
        }
        strcat(buf, ".");
        message(buf, false);
    } else {
        message("a surge of energy courses through your pack, but nothing happens.", false);
    }
}

//void causeFear(const char *emitterName) {
//    creature *monst;
//    short numberOfMonsters = 0;
//    char buf[DCOLS*3], mName[DCOLS];
//
//    for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
//        if (pmap[monst->xLoc][monst->yLoc].flags & IN_FIELD_OF_VIEW
//            && monst->creatureState != MONSTER_FLEEING
//            && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
//
//            monst->status[STATUS_MAGICAL_FEAR] = monst->maxStatus[STATUS_MAGICAL_FEAR] = rand_range(150, 225);
//            monst->creatureState = MONSTER_FLEEING;
//            if (canSeeMonster(monst)) {
//                numberOfMonsters++;
//                monsterName(mName, monst, true);
//            }
//        }
//    }
//    if (numberOfMonsters > 1) {
//        sprintf(buf, "%s emits a brilliant flash of red light, and the monsters flee!", emitterName);
//    } else if (numberOfMonsters == 1) {
//        sprintf(buf, "%s emits a brilliant flash of red light, and %s flees!", emitterName, mName);
//    } else {
//        sprintf(buf, "%s emits a brilliant flash of red light!", emitterName);
//    }
//    message(buf, false);
//    colorFlash(&redFlashColor, 0, IN_FIELD_OF_VIEW, 15, DCOLS, player.xLoc, player.yLoc);
//}

void negationBlast(const char *emitterName, const short distance) {
    creature *monst, *nextMonst;
    item *theItem;
    char buf[DCOLS];

    sprintf(buf, "%s emits a numbing torrent of anti-magic!", emitterName);
    messageWithColor(buf, &itemMessageColor, false);
    colorFlash(&pink, 0, IN_FIELD_OF_VIEW, 3 + distance / 5, distance, player.xLoc, player.yLoc);
    negate(&player);
    flashMonster(&player, &pink, 100);
    for (monst = monsters->nextCreature; monst != NULL;) {
        nextMonst = monst->nextCreature;
        if ((pmap[monst->xLoc][monst->yLoc].flags & IN_FIELD_OF_VIEW)
            && (player.xLoc - monst->xLoc) * (player.xLoc - monst->xLoc) + (player.yLoc - monst->yLoc) * (player.yLoc - monst->yLoc) <= distance * distance) {

            if (canSeeMonster(monst)) {
                flashMonster(monst, &pink, 100);
            }
            negate(monst); // This can be fatal.
        }
        monst = nextMonst;
    }
    for (theItem = floorItems; theItem != NULL; theItem = theItem->nextItem) {
        if ((pmap[theItem->xLoc][theItem->yLoc].flags & IN_FIELD_OF_VIEW)
            && (player.xLoc - theItem->xLoc) * (player.xLoc - theItem->xLoc) + (player.yLoc - theItem->yLoc) * (player.yLoc - theItem->yLoc) <= distance * distance) {

            theItem->flags &= ~(ITEM_MAGIC_DETECTED | ITEM_CURSED);
            switch (theItem->category) {
                case WEAPON:
                case ARMOR:
                    theItem->enchant1 = theItem->enchant2 = theItem->charges = 0;
                    theItem->flags &= ~(ITEM_RUNIC | ITEM_RUNIC_HINTED | ITEM_RUNIC_IDENTIFIED | ITEM_PROTECTED);
                    identify(theItem);
                    break;
                case STAFF:
                    theItem->charges = 0;
                    break;
                case WAND:
                    theItem->charges = 0;
                    theItem->flags |= ITEM_MAX_CHARGES_KNOWN;
                    break;
                case RING:
                    theItem->enchant1 = 0;
                    theItem->flags |= ITEM_IDENTIFIED; // Reveal that it is (now) +0, but not necessarily which kind of ring it is.
                    updateIdentifiableItems();
                    break;
                case CHARM:
                    theItem->charges = charmRechargeDelay(theItem->kind, theItem->enchant1);
                    break;
                default:
                    break;
            }
        }
    }
}

void discordBlast(const char *emitterName, const short distance) {
    creature *monst, *nextMonst;
    char buf[DCOLS];

    sprintf(buf, "%s emits a wave of unsettling purple radiation!", emitterName);
    messageWithColor(buf, &itemMessageColor, false);
    colorFlash(&discordColor, 0, IN_FIELD_OF_VIEW, 3 + distance / 5, distance, player.xLoc, player.yLoc);
    for (monst = monsters->nextCreature; monst != NULL;) {
        nextMonst = monst->nextCreature;
        if ((pmap[monst->xLoc][monst->yLoc].flags & IN_FIELD_OF_VIEW)
            && (player.xLoc - monst->xLoc) * (player.xLoc - monst->xLoc) + (player.yLoc - monst->yLoc) * (player.yLoc - monst->yLoc) <= distance * distance) {

            if (!(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
                if (canSeeMonster(monst)) {
                    flashMonster(monst, &discordColor, 100);
                }
                monst->status[STATUS_DISCORDANT] = monst->maxStatus[STATUS_DISCORDANT] = 30;
            }
        }
        monst = nextMonst;
    }
}

void crystalize(short radius) {
    extern color forceFieldColor;
    short i, j;
    creature *monst;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j < DROWS; j++) {
            if ((player.xLoc - i) * (player.xLoc - i) + (player.yLoc - j) * (player.yLoc - j) <= radius * radius
                && !(pmap[i][j].flags & IMPREGNABLE)) {

                if (i == 0 || i == DCOLS - 1 || j == 0 || j == DROWS - 1) {
                    pmap[i][j].layers[DUNGEON] = CRYSTAL_WALL; // don't dissolve the boundary walls
                } else if (tileCatalog[pmap[i][j].layers[DUNGEON]].flags & (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION)) {

                    pmap[i][j].layers[DUNGEON] = FORCEFIELD;
                    spawnDungeonFeature(i, j, &dungeonFeatureCatalog[DF_SHATTERING_SPELL], true, false);

                    if (pmap[i][j].flags & HAS_MONSTER) {
                        monst = monsterAtLoc(i, j);
                        if (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS) {
                            inflictLethalDamage(NULL, monst);
                        } else {
                            freeCaptivesEmbeddedAt(i, j);
                        }
                    }
                }
            }
        }
    }
    updateVision(false);
    colorFlash(&forceFieldColor, 0, 0, radius, radius, player.xLoc, player.yLoc);
    displayLevel();
    refreshSideBar(-1, -1, false);
}

boolean imbueInvisibility(creature *monst, short duration) {
    boolean autoID = false;

    if (monst && !(monst->info.flags & (MONST_INANIMATE | MONST_INVISIBLE | MONST_INVULNERABLE))) {
        if (monst == &player || monst->creatureState == MONSTER_ALLY) {
            autoID = true;
        }
        monst->status[STATUS_INVISIBLE] = monst->maxStatus[STATUS_INVISIBLE] = duration;
        refreshDungeonCell(monst->xLoc, monst->yLoc);
        refreshSideBar(-1, -1, false);
        if (boltCatalog[BOLT_POLYMORPH].backColor) {
            flashMonster(monst, boltCatalog[BOLT_INVISIBILITY].backColor, 100);
        }
    }
    return autoID;
}

boolean projectileReflects(creature *attacker, creature *defender) {
    short prob;
    fixpt netReflectionLevel;

    // immunity armor always reflects its vorpal enemy's projectiles
    if (defender == &player && rogue.armor && (rogue.armor->flags & ITEM_RUNIC) && rogue.armor->enchant2 == A_IMMUNITY
        && monsterIsInClass(attacker, rogue.armor->vorpalEnemy)
        && monstersAreEnemies(attacker, defender)) {

        return true;
    }

    if (defender == &player && rogue.armor && (rogue.armor->flags & ITEM_RUNIC) && rogue.armor->enchant2 == A_REFLECTION) {
        netReflectionLevel = netEnchant(rogue.armor);
    } else {
        netReflectionLevel = 0;
    }

    if (defender && (defender->info.flags & MONST_REFLECT_4)) {
        if (defender->info.flags & MONST_ALWAYS_USE_ABILITY) {
            return true;
        }
        netReflectionLevel += 4 * FP_FACTOR;
    }

    if (netReflectionLevel <= 0) {
        return false;
    }

    prob = reflectionChance(netReflectionLevel);

    return rand_percent(prob);
}

// Alters listOfCoordinates to describe reflected path,
// which diverges from the existing path at kinkCell,
// and then returns the path length of the reflected path.
short reflectBolt(short targetX, short targetY, short listOfCoordinates[][2], short kinkCell, boolean retracePath) {
    short k, target[2], origin[2], newPath[DCOLS][2], newPathLength, failsafe, finalLength;
    boolean needRandomTarget;

    needRandomTarget = (targetX < 0 || targetY < 0
                        || (targetX == listOfCoordinates[kinkCell][0] && targetY == listOfCoordinates[kinkCell][1]));

    if (retracePath) {
        // if reflecting back at caster, follow precise trajectory until we reach the caster
        for (k = 1; k <= kinkCell && kinkCell + k < MAX_BOLT_LENGTH; k++) {
            listOfCoordinates[kinkCell + k][0] = listOfCoordinates[kinkCell - k][0];
            listOfCoordinates[kinkCell + k][1] = listOfCoordinates[kinkCell - k][1];
        }

        // Calculate a new "extension" path, with an origin at the caster, and a destination at
        // the caster's location translated by the vector from the reflection point to the caster.
        //
        // For example, if the player is at (0,0), and the caster is at (2,3), then the newpath
        // is from (2,3) to (4,6):
        // (2,3) + ((2,3) - (0,0)) = (4,6).

        origin[0] = listOfCoordinates[2 * kinkCell][0];
        origin[1] = listOfCoordinates[2 * kinkCell][1];
        target[0] = targetX + (targetX - listOfCoordinates[kinkCell][0]);
        target[1] = targetY + (targetY - listOfCoordinates[kinkCell][1]);
        newPathLength = getLineCoordinates(newPath, origin, target);
        for (k=0; k<=newPathLength; k++) {
            listOfCoordinates[2 * kinkCell + k + 1][0] = newPath[k][0];
            listOfCoordinates[2 * kinkCell + k + 1][1] = newPath[k][1];
        }
        finalLength = 2 * kinkCell + newPathLength + 1;
    } else {
        failsafe = 50;
        do {
            if (needRandomTarget) {
                // pick random target
                perimeterCoords(target, rand_range(0, 39));
                target[0] += listOfCoordinates[kinkCell][0];
                target[1] += listOfCoordinates[kinkCell][1];
            } else {
                target[0] = targetX;
                target[1] = targetY;
            }
            newPathLength = getLineCoordinates(newPath, listOfCoordinates[kinkCell], target);
            if (newPathLength > 0
                && !cellHasTerrainFlag(newPath[0][0], newPath[0][1], (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))) {

                needRandomTarget = false;
            }
        } while (needRandomTarget && --failsafe);

        for (k = 0; k < newPathLength; k++) {
            listOfCoordinates[kinkCell + k + 1][0] = newPath[k][0];
            listOfCoordinates[kinkCell + k + 1][1] = newPath[k][1];
        }

        finalLength = kinkCell + newPathLength + 1;
    }

    listOfCoordinates[finalLength][0] = -1;
    listOfCoordinates[finalLength][1] = -1;
    return finalLength;
}

// Update stuff that promotes without keys so players can't abuse item libraries with blinking/haste shenanigans
void checkForMissingKeys(short x, short y) {
    short layer;

    if (cellHasTMFlag(x, y, TM_PROMOTES_WITHOUT_KEY) && !keyOnTileAt(x, y)) {
        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_PROMOTES_WITHOUT_KEY) {
                promoteTile(x, y, layer, false);
            }
        }
    }
}

void beckonMonster(creature *monst, short x, short y) {
    short from[2], to[2];
    bolt theBolt = boltCatalog[BOLT_BLINKING];

    if (monst->bookkeepingFlags & MB_CAPTIVE) {
        freeCaptive(monst);
    }
    from[0] = monst->xLoc;
    from[1] = monst->yLoc;
    to[0] = x;
    to[1] = y;
    theBolt.magnitude = max(1, (distanceBetween(x, y, monst->xLoc, monst->yLoc) - 2) / 2);
    zap(from, to, &theBolt, false);
    if (monst->ticksUntilTurn < player.attackSpeed+1) {
        monst->ticksUntilTurn = player.attackSpeed+1;
    }
}

enum boltEffects boltEffectForItem(item *theItem) {
    if (theItem->category & (STAFF | WAND)) {
        return boltCatalog[tableForItemCategory(theItem->category, NULL)[theItem->kind].strengthRequired].boltEffect;
    } else {
        return BE_NONE;
    }
}

enum boltType boltForItem(item *theItem) {
    if (theItem->category & (STAFF | WAND)) {
        return tableForItemCategory(theItem->category, NULL)[theItem->kind].strengthRequired;
    } else {
        return 0;
    }
}

// Called on each space of the bolt's flight.
// Returns true if the bolt terminates here.
// Caster can be null.
// Pass in true for boltInView if any part of the bolt is currently visible to the player.
// Pass in true for alreadyReflected if the bolt has already reflected off of something.
// If the effect is visible enough for the player to identify the shooting item,
// *autoID will be set to true. (AutoID can be null.)
// If the effect causes the level's lighting or vision to change, *lightingChanged
// will be set to true. (LightingChanged can be null.)
boolean updateBolt(bolt *theBolt, creature *caster, short x, short y,
                   boolean boltInView, boolean alreadyReflected,
                   boolean *autoID, boolean *lightingChanged) {
    char buf[COLS], monstName[COLS];
    creature *monst; // Creature being hit by the bolt, if any.
    creature *newMonst; // Utility variable for plenty
    boolean terminateBolt = false;

    if (lightingChanged) {
        *lightingChanged = false;
    }

    // Handle collisions with monsters.

    monst = monsterAtLoc(x, y);
    if (monst && !(monst->bookkeepingFlags & MB_SUBMERGED)) {
        monsterName(monstName, monst, true);

        switch(theBolt->boltEffect) {
            case BE_ATTACK:
                if (!cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY)
                    || (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS)) {

                    attack(caster, monst, false);
                    if (autoID) {
                        *autoID = true;
                    }
                }
                break;
            case BE_DAMAGE:
                if (autoID) {
                    *autoID = true;
                }
                if (((theBolt->flags & BF_FIERY) && monst->status[STATUS_IMMUNE_TO_FIRE] > 0)
                    || (monst->info.flags & MONST_INVULNERABLE)) {

                    if (canSeeMonster(monst)) {
                        sprintf(buf, "%s ignore%s %s %s",
                                monstName,
                                (monst == &player ? "" : "s"),
                                canSeeMonster(caster) ? "the" : "a",
                                theBolt->name);
                        combatMessage(buf, 0);
                    }
                } else if (inflictDamage(caster, monst, staffDamage(theBolt->magnitude * FP_FACTOR), theBolt->backColor, false)) {
                    // killed monster
                    if (player.currentHP <= 0) {
                        if (caster == &player) {
                            sprintf(buf, "Killed by a reflected %s", theBolt->name);
                            gameOver(buf, true);
                        }
                        terminateBolt = true;
                        return true;
                    }
                    if (boltInView || canSeeMonster(monst)) {
                        sprintf(buf, "%s %s %s %s",
                                canSeeMonster(caster) ? "the" : "a",
                                theBolt->name,
                                ((monst->info.flags & MONST_INANIMATE) ? "destroys" : "kills"),
                                monstName);
                        combatMessage(buf, messageColorFromVictim(monst));
                    } else {
                        sprintf(buf, "you hear %s %s", monstName, ((monst->info.flags & MONST_INANIMATE) ? "get destroyed" : "die"));
                        combatMessage(buf, 0);
                    }
                } else {
                    // monster lives
                    if (monst->creatureMode != MODE_PERM_FLEEING
                        && monst->creatureState != MONSTER_ALLY
                        && (monst->creatureState != MONSTER_FLEEING || monst->status[STATUS_MAGICAL_FEAR])) {

                        monst->creatureState = MONSTER_TRACKING_SCENT;
                        monst->status[STATUS_MAGICAL_FEAR] = 0;
                    }
                    if (boltInView) {
                        sprintf(buf, "%s %s hits %s",
                                canSeeMonster(caster) ? "the" : "a",
                                theBolt->name,
                                monstName);
                        combatMessage(buf, messageColorFromVictim(monst));
                    }
                    if (theBolt->flags & BF_FIERY) {
                        exposeCreatureToFire(monst);
                    }
                    if (!alreadyReflected
                        || caster != &player) {
                        moralAttack(caster, monst);
                    }
                }
                if (theBolt->flags & BF_FIERY) {
                    exposeTileToFire(x, y, true); // burninate
                }
                break;
            case BE_TELEPORT:
                if (!(monst->info.flags & MONST_IMMOBILE)) {
                    if (monst->bookkeepingFlags & MB_CAPTIVE) {
                        freeCaptive(monst);
                    }
                    teleport(monst, -1, -1, false);
                }
                break;
            case BE_BECKONING:
                if (!(monst->info.flags & MONST_IMMOBILE)
                    && caster
                    && distanceBetween(caster->xLoc, caster->yLoc, monst->xLoc, monst->yLoc) > 1) {

                    if (canSeeMonster(monst) && autoID) {
                        *autoID = true;
                    }
                    beckonMonster(monst, caster->xLoc, caster->yLoc);
                    if (canSeeMonster(monst) && autoID) {
                        *autoID = true;
                    }
                }
                break;
            case BE_SLOW:
                slow(monst, theBolt->magnitude * 5);
                if (boltCatalog[BOLT_SLOW].backColor) {
                    flashMonster(monst, boltCatalog[BOLT_SLOW].backColor, 100);
                }
                if (autoID) {
                    *autoID = true;
                }
                break;
            case BE_HASTE:
                haste(monst, staffHasteDuration(theBolt->magnitude * FP_FACTOR));
                if (boltCatalog[BOLT_HASTE].backColor) {
                    flashMonster(monst, boltCatalog[BOLT_HASTE].backColor, 100);
                }
                if (autoID) {
                    *autoID = true;
                }
                break;
            case BE_POLYMORPH:
                if (polymorph(monst)) {
                    if (!monst->status[STATUS_INVISIBLE]) {
                        if (autoID) {
                            *autoID = true;
                        }
                    }
                }
                break;
            case BE_INVISIBILITY:
                if (imbueInvisibility(monst, 150) && autoID) {
                    *autoID = true;
                }
                break;
            case BE_DOMINATION:
                if (monst != &player && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
                    if (rand_percent(wandDominate(monst))) {
                        // domination succeeded
                        monst->status[STATUS_DISCORDANT] = 0;
                        becomeAllyWith(monst);
                        //refreshSideBar(-1, -1, false);
                        refreshDungeonCell(monst->xLoc, monst->yLoc);
                        if (canSeeMonster(monst)) {
                            if (autoID) {
                                *autoID = true;
                            }
                            sprintf(buf, "%s is bound to your will!", monstName);
                            message(buf, false);
                            if (boltCatalog[BOLT_DOMINATION].backColor) {
                                flashMonster(monst, boltCatalog[BOLT_DOMINATION].backColor, 100);
                            }
                        }
                    } else if (canSeeMonster(monst)) {
                        if (autoID) {
                            *autoID = true;
                        }
                        sprintf(buf, "%s resists the bolt of domination.", monstName);
                        message(buf, false);
                    }
                }
                break;
            case BE_NEGATION:
                negate(monst);
                if (boltCatalog[BOLT_NEGATION].backColor) {
                    flashMonster(monst, boltCatalog[BOLT_NEGATION].backColor, 100);
                }
                break;
            case BE_EMPOWERMENT:
                if (monst != &player
                    && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {

                    empowerMonster(monst);
                    createFlare(monst->xLoc, monst->yLoc, EMPOWERMENT_LIGHT);
                    if (canSeeMonster(monst) && autoID) {
                        *autoID = true;
                    }
                }
                break;
            case BE_POISON:
                if (!(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
                    addPoison(monst, staffPoison(theBolt->magnitude * FP_FACTOR), 1);
                    if (canSeeMonster(monst)) {
                        if (boltCatalog[BOLT_POISON].backColor) {
                            flashMonster(monst, boltCatalog[BOLT_POISON].backColor, 100);
                        }
                        if (autoID) {
                            *autoID = true;
                        }
                        if (monst != &player) {
                            sprintf(buf, "%s %s %s sick",
                                    monstName,
                                    (monst == &player ? "feel" : "looks"),
                                    (monst->status[STATUS_POISONED] * monst->poisonAmount >= monst->currentHP && !player.status[STATUS_HALLUCINATING] ? "fatally" : "very"));
                            combatMessage(buf, messageColorFromVictim(monst));
                        }
                    }
                }
                break;
            case BE_ENTRANCEMENT:
                if (monst == &player) {
                    flashMonster(monst, &confusionGasColor, 100);
                    monst->status[STATUS_CONFUSED] = staffEntrancementDuration(theBolt->magnitude * FP_FACTOR);
                    monst->maxStatus[STATUS_CONFUSED] = max(monst->status[STATUS_CONFUSED], monst->maxStatus[STATUS_CONFUSED]);
                    message("the bolt hits you and you suddently feel disoriented.", true);
                    if (autoID) {
                        *autoID = true;
                    }
                } else if (!(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
                    monst->status[STATUS_ENTRANCED] = monst->maxStatus[STATUS_ENTRANCED] = staffEntrancementDuration(theBolt->magnitude * FP_FACTOR);
                    wakeUp(monst);
                    if (canSeeMonster(monst)) {
                        if (boltCatalog[BOLT_ENTRANCEMENT].backColor) {
                            flashMonster(monst, boltCatalog[BOLT_ENTRANCEMENT].backColor, 100);
                        }
                        if (autoID) {
                            *autoID = true;
                        }
                        sprintf(buf, "%s is entranced!", monstName);
                        message(buf, false);
                    }
                }
                break;
            case BE_HEALING:
                heal(monst, theBolt->magnitude * 10, false);
                if (canSeeMonster(monst)) {
                    if (autoID) {
                        *autoID = true;
                    }
                }
                break;
            case BE_PLENTY:
                if (!(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
                    newMonst = cloneMonster(monst, true, true);
                    if (newMonst) {
                        newMonst->currentHP = newMonst->info.maxHP = (newMonst->currentHP + 1) / 2;
                        monst->currentHP = monst->info.maxHP = (monst->currentHP + 1) / 2;
                        if (boltCatalog[BOLT_PLENTY].backColor) {
                            flashMonster(monst, boltCatalog[BOLT_PLENTY].backColor, 100);
                            flashMonster(newMonst, boltCatalog[BOLT_PLENTY].backColor, 100);
                        }
                        if (autoID) {
                            *autoID = true;
                        }
                    }
                }
                break;
            case BE_DISCORD:
                if (!(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
                    monst->status[STATUS_DISCORDANT] = monst->maxStatus[STATUS_DISCORDANT] = max(staffDiscordDuration(theBolt->magnitude * FP_FACTOR),
                                                                                                 monst->status[STATUS_DISCORDANT]);
                    if (canSeeMonster(monst)) {
                        if (boltCatalog[BOLT_DISCORD].backColor) {
                            flashMonster(monst, boltCatalog[BOLT_DISCORD].backColor, 100);
                        }
                        if (autoID) {
                            *autoID = true;
                        }
                    }
                }
                break;
            case BE_SHIELDING:
                if (staffProtection(theBolt->magnitude * FP_FACTOR) > monst->status[STATUS_SHIELDED]) {
                    monst->status[STATUS_SHIELDED] = staffProtection(theBolt->magnitude * FP_FACTOR);
                }
                monst->maxStatus[STATUS_SHIELDED] = monst->status[STATUS_SHIELDED];
                if (boltCatalog[BOLT_SHIELDING].backColor) {
                    flashMonster(monst, boltCatalog[BOLT_SHIELDING].backColor, 100);
                }
                if (autoID) {
                    *autoID = true;
                }
                break;
            default:
                break;
        }

        if (!(theBolt->flags & BF_PASSES_THRU_CREATURES)) {
            terminateBolt = true;
        }
    }

    // Handle ordinary bolt updates that aren't dependent on hitting a creature.
    switch (theBolt->boltEffect) {
        case BE_BLINKING:
            if (caster == &player) {
                player.xLoc = x;
                player.yLoc = y;
                if (lightingChanged) {
                    *lightingChanged = true;
                }
            }
            break;
        default:
            break;
    }

    if (theBolt->pathDF) {
        spawnDungeonFeature(x, y, &dungeonFeatureCatalog[theBolt->pathDF], true, false);
    }

    if ((theBolt->flags & BF_FIERY)
        && exposeTileToFire(x, y, true)) {

        if (lightingChanged) {
            *lightingChanged = true;
        }
        if (autoID) {
            *autoID = true;
        }
    }

    if ((theBolt->flags & BF_ELECTRIC)
        && exposeTileToElectricity(x, y)) {

        if (lightingChanged) {
            *lightingChanged = true;
        }
        if (autoID) {
            *autoID = true;
        }
    }

    return terminateBolt;
}

// Called when the bolt hits something.
// Caster can be null.
// Pass in true for alreadyReflected if the bolt has already reflected off of something.
// If the effect is visible enough for the player to identify the shooting item,
// *autoID will be set to true. (AutoID can be null.)
void detonateBolt(bolt *theBolt, creature *caster, short x, short y, boolean *autoID) {
    dungeonFeature feat;
    short i, x2, y2;
    creature *monst;

    const fixpt POW_OBSTRUCTION[] = {
        // 0.8^x, with x from 2 to 40:
        41943, 33554, 26843, 21474, 17179, 13743, 10995, 8796, 7036, 5629, 4503, 3602,
        2882, 2305, 1844, 1475, 1180, 944, 755, 604, 483, 386, 309, 247, 198, 158, 126,
        101, 81, 64, 51, 41, 33, 26, 21, 17, 13, 10, 8, 6, 5};

    switch(theBolt->boltEffect) {
        case BE_OBSTRUCTION:
            feat = dungeonFeatureCatalog[DF_FORCEFIELD];
            feat.probabilityDecrement = max(1, 75 * POW_OBSTRUCTION[min(40, theBolt->magnitude) - 2] / FP_FACTOR);
            spawnDungeonFeature(x, y, &feat, true, false);
            if (autoID) {
                *autoID = true;
            }
            break;
        case BE_CONJURATION:
            for (i = 0; i < (staffBladeCount(theBolt->magnitude * FP_FACTOR)); i++) {
                monst = generateMonster(MK_SPECTRAL_BLADE, true, false);
                getQualifyingPathLocNear(&(monst->xLoc), &(monst->yLoc), x, y, true,
                                         T_DIVIDES_LEVEL & avoidedFlagsForMonster(&(monst->info)) & ~T_SPONTANEOUSLY_IGNITES, HAS_PLAYER,
                                         avoidedFlagsForMonster(&(monst->info)) & ~T_SPONTANEOUSLY_IGNITES, (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS), false);
                monst->bookkeepingFlags |= (MB_FOLLOWER | MB_BOUND_TO_LEADER | MB_DOES_NOT_TRACK_LEADER);
                monst->bookkeepingFlags &= ~MB_JUST_SUMMONED;
                monst->leader = &player;
                monst->creatureState = MONSTER_ALLY;
                monst->ticksUntilTurn = monst->info.attackSpeed + 1; // So they don't move before the player's next turn.
                pmap[monst->xLoc][monst->yLoc].flags |= HAS_MONSTER;
                //refreshDungeonCell(monst->xLoc, monst->yLoc);
                fadeInMonster(monst);
            }
            updateVision(true);
            //refreshSideBar(-1, -1, false);
            monst = NULL;
            if (autoID) {
                *autoID = true;
            }
            break;
        case BE_BLINKING:
            if (pmap[x][y].flags & HAS_MONSTER) { // We're blinking onto an area already occupied by a submerged monster.
                                                  // Make sure we don't get the shooting monster by accident.
                caster->xLoc = caster->yLoc = -1; // Will be set back to the destination in a moment.
                monst = monsterAtLoc(x, y);
                findAlternativeHomeFor(monst, &x2, &y2, true);
                if (x2 >= 0) {
                    // Found an alternative location.
                    monst->xLoc = x2;
                    monst->yLoc = y2;
                    pmap[x][y].flags &= ~HAS_MONSTER;
                    pmap[x2][y2].flags |= HAS_MONSTER;
                } else {
                    // No alternative location?? Hard to imagine how this could happen.
                    // Just bury the monster and never speak of this incident again.
                    killCreature(monst, true);
                    pmap[x][y].flags &= ~HAS_MONSTER;
                    monst = NULL;
                }
            }
            caster->bookkeepingFlags &= ~MB_SUBMERGED;
            pmap[x][y].flags |= (caster == &player ? HAS_PLAYER : HAS_MONSTER);
            caster->xLoc = x;
            caster->yLoc = y;
            applyInstantTileEffectsToCreature(caster);
            if (caster == &player) {
                // increase scent turn number so monsters don't sniff around at the old cell like idiots
                rogue.scentTurnNumber += 30;
                // get any items at the destination location
                if (pmap[player.xLoc][player.yLoc].flags & HAS_ITEM) {
                    pickUpItemAt(player.xLoc, player.yLoc);
                }
                updateVision(true);
            }
            if (autoID) {
                *autoID = true;
            }
            break;
        case BE_TUNNELING:
            setUpWaypoints(); // Recompute waypoints based on the new situation.
            break;
    }

    if (theBolt->targetDF) {
        spawnDungeonFeature(x, y, &dungeonFeatureCatalog[theBolt->targetDF], true, false);
    }
}

// returns whether the bolt effect should autoID any staff or wand it came from, if it came from a staff or wand
boolean zap(short originLoc[2], short targetLoc[2], bolt *theBolt, boolean hideDetails) {
    short listOfCoordinates[MAX_BOLT_LENGTH][2];
    short i, j, k, x, y, x2, y2, numCells, blinkDistance = 0, boltLength, initialBoltLength, lights[DCOLS][DROWS][3];
    creature *monst = NULL, *shootingMonst;
    char buf[COLS], monstName[COLS];
    boolean autoID = false;
    boolean lightingChanged = false;
    boolean fastForward = false;
    boolean alreadyReflected = false;
    boolean boltInView;
    const color *boltColor;
    fixpt boltLightRadius;

    enum displayGlyph theChar;
    color foreColor, backColor, multColor;

    lightSource boltLights[500];
    color boltLightColors[500];

    brogueAssert(originLoc[0] != targetLoc[0] || originLoc[1] != targetLoc[1]);
    if (originLoc[0] == targetLoc[0] && originLoc[1] == targetLoc[1]) {
        return false;
    }

    x = originLoc[0];
    y = originLoc[1];

    initialBoltLength = boltLength = 5 * theBolt->magnitude;
    numCells = getLineCoordinates(listOfCoordinates, originLoc, targetLoc);
    shootingMonst = monsterAtLoc(originLoc[0], originLoc[1]);

    if (hideDetails) {
        boltColor = &gray;
    } else {
        boltColor = theBolt->backColor;
    }

    refreshSideBar(-1, -1, false);
    displayCombatText(); // To announce who fired the bolt while the animation plays.

    if (theBolt->boltEffect == BE_BLINKING) {
        if (cellHasTerrainFlag(listOfCoordinates[0][0], listOfCoordinates[0][1], (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))
            || ((pmap[listOfCoordinates[0][0]][listOfCoordinates[0][1]].flags & (HAS_PLAYER | HAS_MONSTER))
                && !(monsterAtLoc(listOfCoordinates[0][0], listOfCoordinates[0][1])->bookkeepingFlags & MB_SUBMERGED))) {
                // shooting blink point-blank into an obstruction does nothing.
                return false;
            }
        theBolt->foreColor = &black;
        theBolt->theChar = shootingMonst->info.displayChar;
        pmap[originLoc[0]][originLoc[1]].flags &= ~(HAS_PLAYER | HAS_MONSTER);
        refreshDungeonCell(originLoc[0], originLoc[1]);
        blinkDistance = theBolt->magnitude * 2 + 1;
        checkForMissingKeys(originLoc[0], originLoc[1]);
    }

    if (boltColor) {
        for (i=0; i<initialBoltLength; i++) {
            boltLightColors[i] = *boltColor;
            boltLights[i] = lightCatalog[BOLT_LIGHT_SOURCE];
            boltLights[i].lightColor = &boltLightColors[i];
            boltLightRadius = 50LL * ((3 * FP_FACTOR) + (theBolt->magnitude * FP_FACTOR) * 4/3) * (initialBoltLength - i) / initialBoltLength / FP_FACTOR;
            boltLights[i].lightRadius.lowerBound = boltLights[i].lightRadius.upperBound = boltLightRadius;
            //boltLights[i].lightRadius.lowerBound = boltLights[i].lightRadius.upperBound = 50 * (3 + theBolt->magnitude * 1.33) * (initialBoltLength - i) / initialBoltLength;
            //printf("\nStandard: %i, attempted new: %lli", boltLights[i].lightRadius.lowerBound, boltLightRadius);
        }
    }

    if (theBolt->boltEffect == BE_TUNNELING) {
        tunnelize(originLoc[0], originLoc[1]);
    }

    backUpLighting(lights);
    boltInView = true;
    for (i=0; i<numCells; i++) {

        x = listOfCoordinates[i][0];
        y = listOfCoordinates[i][1];

        monst = monsterAtLoc(x, y);

        // Handle bolt reflection off of creatures (reflection off of terrain is handled further down).
        if (monst
            && !(theBolt->flags & BF_NEVER_REFLECTS)
            && projectileReflects(shootingMonst, monst)
            && i < MAX_BOLT_LENGTH - max(DCOLS, DROWS)) {

            if (projectileReflects(shootingMonst, monst)) { // if it scores another reflection roll, reflect at caster
                numCells = reflectBolt(originLoc[0], originLoc[1], listOfCoordinates, i, !alreadyReflected);
            } else {
                numCells = reflectBolt(-1, -1, listOfCoordinates, i, false); // otherwise reflect randomly
            }

            alreadyReflected = true;

            if (boltInView) {
                monsterName(monstName, monst, true);
                sprintf(buf, "%s deflect%s the %s",
                        monstName,
                        (monst == &player ? "" : "s"),
                        theBolt->name);
                combatMessage(buf, 0);
            }
            if (monst == &player
                && rogue.armor
                && rogue.armor->enchant2 == A_REFLECTION
                && !(rogue.armor->flags & ITEM_RUNIC_IDENTIFIED)) {

                autoIdentify(rogue.armor);
            }
            continue;
        }

        if (updateBolt(theBolt, shootingMonst, x, y, boltInView, alreadyReflected, &autoID, &lightingChanged)) {
            break;
        }

        if (lightingChanged) {
            updateVision(true);
            backUpLighting(lights);
        }

        // Update the visual effect of the bolt.
        // First do lighting. This lighting effect is expensive; do it only if the player can see the bolt.
        if (boltInView && boltColor) {
            demoteVisibility();
            restoreLighting(lights);
            for (k = min(i, boltLength + 2); k >= 0; k--) {
                if (k < initialBoltLength) {
                    paintLight(&boltLights[k], listOfCoordinates[i-k][0], listOfCoordinates[i-k][1], false, false);
                }
            }
        }
        boltInView = false;
        updateFieldOfViewDisplay(false, true);
        // Now draw the bolt itself.
        for (k = min(i, boltLength + 2); k >= 0; k--) {
            x2 = listOfCoordinates[i-k][0];
            y2 = listOfCoordinates[i-k][1];
            if (playerCanSeeOrSense(x2, y2)) {
                if (!fastForward) {
                    getCellAppearance(x2, y2, &theChar, &foreColor, &backColor);
                    if (boltColor) {
                        applyColorAugment(&foreColor, boltColor, max(0, 100 - k * 100 / (boltLength)));
                        applyColorAugment(&backColor, boltColor, max(0, 100 - k * 100 / (boltLength)));
                    }
                    const boolean displayChar = (k == 0 || (theBolt->flags & BF_DISPLAY_CHAR_ALONG_LENGTH));
                    if (displayChar) {
                        if (theBolt->foreColor) {
                            foreColor = *(theBolt->foreColor);
                        }
                        if (theBolt->theChar) {
                            theChar = theBolt->theChar;
                        }
                    }
                    if (displayChar
                        && theBolt->foreColor
                        && theBolt->theChar) {

                        colorMultiplierFromDungeonLight(x2, y2, &multColor);
                        applyColorMultiplier(&foreColor, &multColor);
                        plotCharWithColor(theChar, mapToWindowX(x2), mapToWindowY(y2), &foreColor, &backColor);
                    } else if (boltColor) {
                        plotCharWithColor(theChar, mapToWindowX(x2), mapToWindowY(y2), &foreColor, &backColor);
                    } else if (k == 1
                               && theBolt->foreColor
                               && theBolt->theChar) {

                        refreshDungeonCell(x2, y2); // Clean up the contrail so it doesn't leave a trail of characters.
                    }
                }
                if (playerCanSee(x2, y2)) {
                    // Don't want to let omniscience mode affect boltInView; causes OOS.
                    boltInView = true;
                }
            }
        }
        if (!fastForward && (boltInView || rogue.playbackOmniscience)) {
            fastForward = rogue.playbackFastForward || pauseBrogue(16);
        }

        if (theBolt->boltEffect == BE_BLINKING) {
            theBolt->magnitude = (blinkDistance - i) / 2 + 1;
            boltLength = theBolt->magnitude * 5;
            for (j=0; j<i; j++) {
                refreshDungeonCell(listOfCoordinates[j][0], listOfCoordinates[j][1]);
            }
            if (i >= blinkDistance) {
                break;
            }
        }

        // Some bolts halt at the square before they hit something.
        if ((theBolt->flags & BF_HALTS_BEFORE_OBSTRUCTION)
            && i + 1 < numCells) {

            x2 = listOfCoordinates[i+1][0];
            y2 = listOfCoordinates[i+1][1];

            if (cellHasTerrainFlag(x2, y2, (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))) {
                break;
            }

            if (!(theBolt->flags & BF_PASSES_THRU_CREATURES)) {
                monst = monsterAtLoc(listOfCoordinates[i+1][0], listOfCoordinates[i+1][1]);
                if (monst && !(monst->bookkeepingFlags & MB_SUBMERGED)) {
                    break;
                }
            }
        }

        // Tunnel if we hit a wall.
        if (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))
            && theBolt->boltEffect == BE_TUNNELING
            && tunnelize(x, y)) {

            updateVision(true);
            backUpLighting(lights);
            autoID = true;
            theBolt->magnitude--;
            boltLength = theBolt->magnitude * 5;
            for (j=0; j<i; j++) {
                refreshDungeonCell(listOfCoordinates[j][0], listOfCoordinates[j][1]);
            }
            if (theBolt->magnitude <= 0) {
                refreshDungeonCell(listOfCoordinates[i-1][0], listOfCoordinates[i-1][1]);
                refreshDungeonCell(x, y);
                break;
            }
        }

        // Stop when we hit a wall.
        if (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))) {
            break;
        }

        // Does the bolt bounce before hitting a wall?
        // Can happen with a cursed deflection ring or a reflective terrain target, or when shooting a tunneling bolt into an impregnable wall.
        if (i + 1 < numCells
            && !(theBolt->flags & BF_NEVER_REFLECTS)) {

            x2 = listOfCoordinates[i+1][0];
            y2 = listOfCoordinates[i+1][1];
            if (cellHasTerrainFlag(x2, y2, (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))
                && (projectileReflects(shootingMonst, NULL)
                    || cellHasTMFlag(x2, y2, TM_REFLECTS_BOLTS)
                    || (theBolt->boltEffect == BE_TUNNELING && (pmap[x2][y2].flags & IMPREGNABLE)))
                && i < MAX_BOLT_LENGTH - max(DCOLS, DROWS)) {

                sprintf(buf, "the bolt reflects off of %s", tileText(x2, y2));
                if (projectileReflects(shootingMonst, NULL)) {
                    // If it scores another reflection roll, reflect at caster, unless it's already reflected.
                    numCells = reflectBolt(originLoc[0], originLoc[1], listOfCoordinates, i, !alreadyReflected);
                } else {
                    numCells = reflectBolt(-1, -1, listOfCoordinates, i, false); // Otherwise reflect randomly.
                }
                alreadyReflected = true;
                if (boltInView) {
                    combatMessage(buf, 0);
                }
            }
        }
    }

    if (!fastForward) {
        refreshDungeonCell(x, y);
        if (i > 0) {
            refreshDungeonCell(listOfCoordinates[i-1][0], listOfCoordinates[i-1][1]);
        }
    }

    if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
        monst = monsterAtLoc(x, y);
        monsterName(monstName, monst, true);
    } else {
        monst = NULL;
    }

    detonateBolt(theBolt, shootingMonst, x, y, &autoID);

    updateLighting();
    backUpLighting(lights);
    boltInView = true;
    refreshSideBar(-1, -1, false);
    if (boltLength > 0) {
        if (boltColor) {
            // j is where the front tip of the bolt would be if it hadn't collided at i
            for (j=i; j < i + boltLength + 2; j++) { // j can imply a bolt tip position that is off the map

                // dynamic lighting
                if (boltInView) {
                    demoteVisibility();
                    restoreLighting(lights);

                    // k = j-i;
                    // boltLights[k].lightRadius.lowerBound *= 2;
                    // boltLights[k].lightRadius.upperBound *= 2;
                    // boltLights[k].lightColor = &boltImpactColor;

                    for (k = min(j, boltLength + 2); k >= j-i; k--) {
                        if (k < initialBoltLength) {
                            paintLight(&boltLights[k], listOfCoordinates[j-k][0], listOfCoordinates[j-k][1], false, false);
                        }
                    }
                    updateFieldOfViewDisplay(false, true);
                }

                boltInView = false;

                // beam graphic
                // k iterates from the tail tip of the visible portion of the bolt to the head
                for (k = min(j, boltLength + 2); k >= j-i; k--) {
                    if (playerCanSee(listOfCoordinates[j-k][0], listOfCoordinates[j-k][1])) {
                        if (boltColor) {
                            hiliteCell(listOfCoordinates[j-k][0], listOfCoordinates[j-k][1], boltColor, max(0, 100 - k * 100 / (boltLength)), false);
                        }
                        boltInView = true;
                    }
                }

                if (!fastForward && boltInView) {
                    fastForward = rogue.playbackFastForward || pauseBrogue(16);
                }
            }
        } else if (theBolt->flags & BF_DISPLAY_CHAR_ALONG_LENGTH) {
            for (j = 0; j < i; j++) {
                x2 = listOfCoordinates[j][0];
                y2 = listOfCoordinates[j][1];
                if (playerCanSeeOrSense(x2, y2)) {
                    refreshDungeonCell(x2, y2);
                }
            }
        }
    }
    return autoID;
}

// Relies on the sidebar entity list. If one is already selected, select the next qualifying. Otherwise, target the first qualifying.
boolean nextTargetAfter(short *returnX,
                        short *returnY,
                        short targetX,
                        short targetY,
                        boolean targetEnemies,
                        boolean targetAllies,
                        boolean targetItems,
                        boolean targetTerrain,
                        boolean requireOpenPath,
                        boolean reverseDirection) {
    short i, n, targetCount, newX, newY;
    short selectedIndex = 0;
    creature *monst;
    item *theItem;
    short deduplicatedTargetList[ROWS][2];

    targetCount = 0;
    for (i=0; i<ROWS; i++) {
        if (rogue.sidebarLocationList[i][0] != -1) {
            if (targetCount == 0
                || deduplicatedTargetList[targetCount-1][0] != rogue.sidebarLocationList[i][0]
                || deduplicatedTargetList[targetCount-1][1] != rogue.sidebarLocationList[i][1]) {

                deduplicatedTargetList[targetCount][0] = rogue.sidebarLocationList[i][0];
                deduplicatedTargetList[targetCount][1] = rogue.sidebarLocationList[i][1];
                if (rogue.sidebarLocationList[i][0] == targetX
                    && rogue.sidebarLocationList[i][1] == targetY) {
                    selectedIndex = targetCount;
                }
                targetCount++;
            }
        }
    }
    for (i = reverseDirection ? targetCount - 1 : 0; reverseDirection ? i >= 0 : i < targetCount; reverseDirection ? i-- : i++) {
        n = (selectedIndex + i) % targetCount;
        newX = deduplicatedTargetList[n][0];
        newY = deduplicatedTargetList[n][1];
        if ((newX != player.xLoc || newY != player.yLoc)
            && (newX != targetX || newY != targetY)
            && (!requireOpenPath || openPathBetween(player.xLoc, player.yLoc, newX, newY))) {

            brogueAssert(coordinatesAreInMap(newX, newY));
            brogueAssert(n >= 0 && n < targetCount);
            monst = monsterAtLoc(newX, newY);
            if (monst) {
                if (monstersAreEnemies(&player, monst)) {
                    if (targetEnemies) {
                        *returnX = newX;
                        *returnY = newY;
                        return true;
                    }
                } else {
                    if (targetAllies) {
                        *returnX = newX;
                        *returnY = newY;
                        return true;
                    }
                }
            }
            theItem = itemAtLoc(newX, newY);
            if (!monst && theItem && targetItems) {
                *returnX = newX;
                *returnY = newY;
                return true;
            }
            if (!monst && !theItem && targetTerrain) {
                *returnX = newX;
                *returnY = newY;
                return true;
            }
        }
    }
    return false;
}

// Returns how far it went before hitting something.
short hiliteTrajectory(short coordinateList[DCOLS][2], short numCells, boolean eraseHiliting, boolean passThroughMonsters, const color *hiliteColor) {
    short x, y, i;
    creature *monst;

    for (i=0; i<numCells; i++) {
        x = coordinateList[i][0];
        y = coordinateList[i][1];
        if (eraseHiliting) {
            refreshDungeonCell(x, y);
        } else {
            hiliteCell(x, y, hiliteColor, 20, true);
        }

        if (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))
            || pmap[x][y].flags & (HAS_PLAYER)) {
            i++;
            break;
        } else if (!(pmap[x][y].flags & DISCOVERED)) {
            break;
        } else if (!passThroughMonsters && pmap[x][y].flags & (HAS_MONSTER)
                   && (playerCanSee(x, y) || player.status[STATUS_TELEPATHIC])) {
            monst = monsterAtLoc(x, y);
            if (!(monst->bookkeepingFlags & MB_SUBMERGED)
                && !monsterIsHidden(monst, &player)) {

                i++;
                break;
            }
        }
    }
    return i;
}

// Event is optional. Returns true if the event should be executed by the parent function.
boolean moveCursor(boolean *targetConfirmed,
                   boolean *canceled,
                   boolean *tabKey,
                   short targetLoc[2],
                   rogueEvent *event,
                   buttonState *state,
                   boolean colorsDance,
                   boolean keysMoveCursor,
                   boolean targetCanLeaveMap) {
    signed long keystroke;
    short moveIncrement;
    short buttonInput;
    boolean cursorMovementCommand, again, movementKeystroke, sidebarHighlighted;
    rogueEvent theEvent;
    short oldRNG;

    short *cursor = rogue.cursorLoc; // shorthand

    cursor[0] = targetLoc[0];
    cursor[1] = targetLoc[1];

    *targetConfirmed = *canceled = *tabKey = false;
    sidebarHighlighted = false;

    do {
        again = false;
        cursorMovementCommand = false;
        movementKeystroke = false;

        oldRNG = rogue.RNG;
        rogue.RNG = RNG_COSMETIC;
        //assureCosmeticRNG;

        if (state) { // Also running a button loop.

            // Update the display.
            overlayDisplayBuffer(state->dbuf, NULL);

            // Get input.
            nextBrogueEvent(&theEvent, false, colorsDance, true);

            // Process the input.
            buttonInput = processButtonInput(state, NULL, &theEvent);

            if (buttonInput != -1) {
                state->buttonDepressed = state->buttonFocused = -1;
                drawButtonsInState(state);
            }

            // Revert the display.
            overlayDisplayBuffer(state->rbuf, NULL);

        } else { // No buttons to worry about.
            nextBrogueEvent(&theEvent, false, colorsDance, true);
        }
        restoreRNG;

        if (theEvent.eventType == MOUSE_UP || theEvent.eventType == MOUSE_ENTERED_CELL) {
            if (theEvent.param1 >= 0
                && theEvent.param1 < mapToWindowX(0)
                && theEvent.param2 >= 0
                && theEvent.param2 < ROWS - 1
                && rogue.sidebarLocationList[theEvent.param2][0] > -1) {

                // If the cursor is on an entity in the sidebar.
                cursor[0] = rogue.sidebarLocationList[theEvent.param2][0];
                cursor[1] = rogue.sidebarLocationList[theEvent.param2][1];
                sidebarHighlighted = true;
                cursorMovementCommand = true;
                refreshSideBar(cursor[0], cursor[1], false);
                if (theEvent.eventType == MOUSE_UP) {
                    *targetConfirmed = true;
                }
            } else if (coordinatesAreInMap(windowToMapX(theEvent.param1), windowToMapY(theEvent.param2))
                       || targetCanLeaveMap && theEvent.eventType != MOUSE_UP) {

                // If the cursor is in the map area, or is allowed to leave the map and it isn't a click.
                if (theEvent.eventType == MOUSE_UP
                    && !theEvent.shiftKey
                    && (theEvent.controlKey || (cursor[0] == windowToMapX(theEvent.param1) && cursor[1] == windowToMapY(theEvent.param2)))) {

                    *targetConfirmed = true;
                }
                cursor[0] = windowToMapX(theEvent.param1);
                cursor[1] = windowToMapY(theEvent.param2);
                cursorMovementCommand = true;
            } else {
                cursorMovementCommand = false;
                again = theEvent.eventType != MOUSE_UP;
            }
        } else if (theEvent.eventType == KEYSTROKE) {
            keystroke = theEvent.param1;
            moveIncrement = ( (theEvent.controlKey || theEvent.shiftKey) ? 5 : 1 );
            stripShiftFromMovementKeystroke(&keystroke);
            switch(keystroke) {
                case LEFT_ARROW:
                case LEFT_KEY:
                case NUMPAD_4:
                    if (keysMoveCursor && cursor[0] > 0) {
                        cursor[0] -= moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case RIGHT_ARROW:
                case RIGHT_KEY:
                case NUMPAD_6:
                    if (keysMoveCursor && cursor[0] < DCOLS - 1) {
                        cursor[0] += moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case UP_ARROW:
                case UP_KEY:
                case NUMPAD_8:
                    if (keysMoveCursor && cursor[1] > 0) {
                        cursor[1] -= moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case DOWN_ARROW:
                case DOWN_KEY:
                case NUMPAD_2:
                    if (keysMoveCursor && cursor[1] < DROWS - 1) {
                        cursor[1] += moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case UPLEFT_KEY:
                case NUMPAD_7:
                    if (keysMoveCursor && cursor[0] > 0 && cursor[1] > 0) {
                        cursor[0] -= moveIncrement;
                        cursor[1] -= moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case UPRIGHT_KEY:
                case NUMPAD_9:
                    if (keysMoveCursor && cursor[0] < DCOLS - 1 && cursor[1] > 0) {
                        cursor[0] += moveIncrement;
                        cursor[1] -= moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case DOWNLEFT_KEY:
                case NUMPAD_1:
                    if (keysMoveCursor && cursor[0] > 0 && cursor[1] < DROWS - 1) {
                        cursor[0] -= moveIncrement;
                        cursor[1] += moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case DOWNRIGHT_KEY:
                case NUMPAD_3:
                    if (keysMoveCursor && cursor[0] < DCOLS - 1 && cursor[1] < DROWS - 1) {
                        cursor[0] += moveIncrement;
                        cursor[1] += moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case TAB_KEY:
                case SHIFT_TAB_KEY:
                case NUMPAD_0:
                    *tabKey = true;
                    break;
                case RETURN_KEY:
                case NUMPAD_5:
                    *targetConfirmed = true;
                    break;
                case ESCAPE_KEY:
                case ACKNOWLEDGE_KEY:
                    *canceled = true;
                    break;
                default:
                    break;
            }
        } else if (theEvent.eventType == RIGHT_MOUSE_UP) {
            // do nothing
        } else {
            again = true;
        }

        if (sidebarHighlighted
            && (!(pmap[cursor[0]][cursor[1]].flags & (HAS_PLAYER | HAS_MONSTER))
                || !canSeeMonster(monsterAtLoc(cursor[0], cursor[1])))
            && (!(pmap[cursor[0]][cursor[1]].flags & HAS_ITEM) || !playerCanSeeOrSense(cursor[0], cursor[1]))
            && (!cellHasTMFlag(cursor[0], cursor[1], TM_LIST_IN_SIDEBAR) || !playerCanSeeOrSense(cursor[0], cursor[1]))) {

            // The sidebar is highlighted but the cursor is not on a visible item, monster or terrain. Un-highlight the sidebar.
            refreshSideBar(-1, -1, false);
            sidebarHighlighted = false;
        }

        if (targetCanLeaveMap && !movementKeystroke) {
            // permit it to leave the map by up to 1 space in any direction if mouse controlled.
            cursor[0] = clamp(cursor[0], -1, DCOLS);
            cursor[1] = clamp(cursor[1], -1, DROWS);
        } else {
            cursor[0] = clamp(cursor[0], 0, DCOLS - 1);
            cursor[1] = clamp(cursor[1], 0, DROWS - 1);
        }
    } while (again && (!event || !cursorMovementCommand));

    if (event) {
        *event = theEvent;
    }

    if (sidebarHighlighted) {
        // Don't leave the sidebar highlighted when we exit.
        refreshSideBar(-1, -1, false);
        sidebarHighlighted = false;
    }

    targetLoc[0] = cursor[0];
    targetLoc[1] = cursor[1];

    return !cursorMovementCommand;
}

void pullMouseClickDuringPlayback(short loc[2]) {
    rogueEvent theEvent;

    brogueAssert(rogue.playbackMode);
    nextBrogueEvent(&theEvent, false, false, false);
    loc[0] = windowToMapX(theEvent.param1);
    loc[1] = windowToMapY(theEvent.param2);
}

// Returns whether monst is targetable with thrown items, staves, wands, etc.
// i.e. would the player ever select it?
static boolean creatureIsTargetable(creature *monst) {
    return monst != NULL
        && canSeeMonster(monst)
        && monst->depth == rogue.depthLevel
        && !(monst->bookkeepingFlags & MB_IS_DYING)
        && openPathBetween(player.xLoc, player.yLoc, monst->xLoc, monst->yLoc);
}

// Return true if a target is chosen, or false if canceled.
boolean chooseTarget(short returnLoc[2],
                     short maxDistance,
                     boolean stopAtTarget,
                     boolean autoTarget,
                     boolean targetAllies,
                     boolean passThroughCreatures,
                     const color *trajectoryColor) {
    short originLoc[2], targetLoc[2], oldTargetLoc[2], coordinates[DCOLS][2], numCells, i, distance, newX, newY;
    creature *monst;
    boolean canceled, targetConfirmed, tabKey, cursorInTrajectory, focusedOnSomething = false;
    rogueEvent event = {0};
    short oldRNG;
    color trajColor = *trajectoryColor;

    normColor(&trajColor, 100, 10);

    if (rogue.playbackMode) {
        // In playback, pull the next event (a mouseclick) and use that location as the target.
        pullMouseClickDuringPlayback(returnLoc);
        rogue.cursorLoc[0] = rogue.cursorLoc[1] = -1;
        return true;
    }

    oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;
    //assureCosmeticRNG;

    originLoc[0] = player.xLoc;
    originLoc[1] = player.yLoc;

    targetLoc[0] = oldTargetLoc[0] = player.xLoc;
    targetLoc[1] = oldTargetLoc[1] = player.yLoc;

    if (autoTarget) {
        if (creatureIsTargetable(rogue.lastTarget) && (targetAllies == (rogue.lastTarget->creatureState == MONSTER_ALLY))) {
            monst = rogue.lastTarget;
        } else {
            //rogue.lastTarget = NULL;
            if (nextTargetAfter(&newX, &newY, targetLoc[0], targetLoc[1], !targetAllies, targetAllies, false, false, true, false)) {
                targetLoc[0] = newX;
                targetLoc[1] = newY;
            }
            monst = monsterAtLoc(targetLoc[0], targetLoc[1]);
        }
        if (monst) {
            targetLoc[0] = monst->xLoc;
            targetLoc[1] = monst->yLoc;
            refreshSideBar(monst->xLoc, monst->yLoc, false);
            focusedOnSomething = true;
        }
    }

    numCells = getLineCoordinates(coordinates, originLoc, targetLoc);
    if (maxDistance > 0) {
        numCells = min(numCells, maxDistance);
    }
    if (stopAtTarget) {
        numCells = min(numCells, distanceBetween(player.xLoc, player.yLoc, targetLoc[0], targetLoc[1]));
    }

    targetConfirmed = canceled = tabKey = false;

    do {
        printLocationDescription(targetLoc[0], targetLoc[1]);

        if (canceled) {
            refreshDungeonCell(oldTargetLoc[0], oldTargetLoc[1]);
            hiliteTrajectory(coordinates, numCells, true, passThroughCreatures, trajectoryColor);
            confirmMessages();
            rogue.cursorLoc[0] = rogue.cursorLoc[1] = -1;
            restoreRNG;
            return false;
        }

        if (tabKey) {
            if (nextTargetAfter(&newX, &newY, targetLoc[0], targetLoc[1], !targetAllies, targetAllies, false, false, true, event.shiftKey)) {
                targetLoc[0] = newX;
                targetLoc[1] = newY;
            }
        }

        monst = monsterAtLoc(targetLoc[0], targetLoc[1]);
        if (monst != NULL && monst != &player && canSeeMonster(monst)) {
            focusedOnSomething = true;
        } else if (playerCanSeeOrSense(targetLoc[0], targetLoc[1])
                   && (pmap[targetLoc[0]][targetLoc[1]].flags & HAS_ITEM) || cellHasTMFlag(targetLoc[0], targetLoc[1], TM_LIST_IN_SIDEBAR)) {
            focusedOnSomething = true;
        } else if (focusedOnSomething) {
            refreshSideBar(-1, -1, false);
            focusedOnSomething = false;
        }
        if (focusedOnSomething) {
            refreshSideBar(targetLoc[0], targetLoc[1], false);
        }

        refreshDungeonCell(oldTargetLoc[0], oldTargetLoc[1]);
        hiliteTrajectory(coordinates, numCells, true, passThroughCreatures, &trajColor);

        if (!targetConfirmed) {
            numCells = getLineCoordinates(coordinates, originLoc, targetLoc);
            if (maxDistance > 0) {
                numCells = min(numCells, maxDistance);
            }

            if (stopAtTarget) {
                numCells = min(numCells, distanceBetween(player.xLoc, player.yLoc, targetLoc[0], targetLoc[1]));
            }
            distance = hiliteTrajectory(coordinates, numCells, false, passThroughCreatures, &trajColor);
            cursorInTrajectory = false;
            for (i=0; i<distance; i++) {
                if (coordinates[i][0] == targetLoc[0] && coordinates[i][1] == targetLoc[1]) {
                    cursorInTrajectory = true;
                    break;
                }
            }
            hiliteCell(targetLoc[0], targetLoc[1], &white, (cursorInTrajectory ? 100 : 35), true);
        }

        oldTargetLoc[0] = targetLoc[0];
        oldTargetLoc[1] = targetLoc[1];
        moveCursor(&targetConfirmed, &canceled, &tabKey, targetLoc, &event, NULL, false, true, false);
        if (event.eventType == RIGHT_MOUSE_UP) { // Right mouse cancels.
            canceled = true;
        }
    } while (!targetConfirmed);
    if (maxDistance > 0) {
        numCells = min(numCells, maxDistance);
    }
    hiliteTrajectory(coordinates, numCells, true, passThroughCreatures, trajectoryColor);
    refreshDungeonCell(oldTargetLoc[0], oldTargetLoc[1]);

    if (originLoc[0] == targetLoc[0] && originLoc[1] == targetLoc[1]) {
        confirmMessages();
        restoreRNG;
        rogue.cursorLoc[0] = rogue.cursorLoc[1] = -1;
        return false;
    }

    monst = monsterAtLoc(targetLoc[0], targetLoc[1]);
    if (monst && monst != &player && canSeeMonster(monst)) {
        rogue.lastTarget = monst;
    }

    returnLoc[0] = targetLoc[0];
    returnLoc[1] = targetLoc[1];
    restoreRNG;
    rogue.cursorLoc[0] = rogue.cursorLoc[1] = -1;
    return true;
}

void identifyItemKind(item *theItem) {
    itemTable *theTable;
    short tableCount, i, lastItem;

    theTable = tableForItemCategory(theItem->category, NULL);
    if (theTable) {
        theItem->flags &= ~ITEM_KIND_AUTO_ID;

        tableCount = 0;
        lastItem = -1;

        switch (theItem->category) {
            case SCROLL:
                tableCount = NUMBER_SCROLL_KINDS;
                break;
            case POTION:
                tableCount = NUMBER_POTION_KINDS;
                break;
            case WAND:
                tableCount = NUMBER_WAND_KINDS;
                break;
            case STAFF:
                tableCount = NUMBER_STAFF_KINDS;
                break;
            case RING:
                tableCount = NUMBER_RING_KINDS;
                break;
            default:
                break;
        }
        if ((theItem->category & RING)
            && theItem->enchant1 <= 0) {

            theItem->flags |= ITEM_IDENTIFIED;
        }

        if ((theItem->category & WAND)
            && theTable[theItem->kind].range.lowerBound == theTable[theItem->kind].range.upperBound) {

            theItem->flags |= ITEM_IDENTIFIED;
        }
        if (tableCount) {
            theTable[theItem->kind].identified = true;
            for (i=0; i<tableCount; i++) {
                if (!(theTable[i].identified)) {
                    if (lastItem != -1) {
                        return; // At least two unidentified items remain.
                    }
                    lastItem = i;
                }
            }
            if (lastItem != -1) {
                // Exactly one unidentified item remains; identify it.
                theTable[lastItem].identified = true;
            }
        }
    }
}

void autoIdentify(item *theItem) {
    short quantityBackup;
    char buf[COLS * 3], oldName[COLS * 3], newName[COLS * 3];

    if (tableForItemCategory(theItem->category, NULL)
        && !tableForItemCategory(theItem->category, NULL)[theItem->kind].identified) {

        identifyItemKind(theItem);
        quantityBackup = theItem->quantity;
        theItem->quantity = 1;
        itemName(theItem, newName, false, true, NULL);
        theItem->quantity = quantityBackup;
        sprintf(buf, "(It must %s %s.)",
                ((theItem->category & (POTION | SCROLL)) ? "have been" : "be"),
                newName);
        messageWithColor(buf, &itemMessageColor, false);
    }

    if ((theItem->category & (WEAPON | ARMOR))
        && (theItem->flags & ITEM_RUNIC)
        && !(theItem->flags & ITEM_RUNIC_IDENTIFIED)) {

        itemName(theItem, oldName, false, false, NULL);
        theItem->flags |= (ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
        itemName(theItem, newName, true, true, NULL);
        sprintf(buf, "(Your %s must be %s.)", oldName, newName);
        messageWithColor(buf, &itemMessageColor, false);
    }
}

// returns whether the item disappeared
boolean hitMonsterWithProjectileWeapon(creature *thrower, creature *monst, item *theItem) {
    char buf[DCOLS], theItemName[DCOLS], targetName[DCOLS], armorRunicString[DCOLS];
    boolean thrownWeaponHit;
    item *equippedWeapon;
    short damage;

    if (!(theItem->category & WEAPON)) {
        return false;
    }

    armorRunicString[0] = '\0';

    itemName(theItem, theItemName, false, false, NULL);
    monsterName(targetName, monst, true);

    monst->status[STATUS_ENTRANCED] = 0;

    if (monst != &player
        && monst->creatureMode != MODE_PERM_FLEEING
        && (monst->creatureState != MONSTER_FLEEING || monst->status[STATUS_MAGICAL_FEAR])
        && !(monst->bookkeepingFlags & MB_CAPTIVE)
        && monst->creatureState != MONSTER_ALLY) {

        monst->creatureState = MONSTER_TRACKING_SCENT;
        if (monst->status[STATUS_MAGICAL_FEAR]) {
            monst->status[STATUS_MAGICAL_FEAR] = 1;
        }
    }

    if (thrower == &player) {
        equippedWeapon = rogue.weapon;
        equipItem(theItem, true);
        thrownWeaponHit = attackHit(&player, monst);
        if (equippedWeapon) {
            equipItem(equippedWeapon, true);
        } else {
            unequipItem(theItem, true);
        }
    } else {
        thrownWeaponHit = attackHit(thrower, monst);
    }

    if (thrownWeaponHit) {
        damage = monst->info.flags & (MONST_IMMUNE_TO_WEAPONS | MONST_INVULNERABLE) ? 0 :
                  (randClump(theItem->damage) * damageFraction(netEnchant(theItem)) / FP_FACTOR);

        if (monst == &player) {
            applyArmorRunicEffect(armorRunicString, thrower, &damage, false);
        }

        if (inflictDamage(thrower, monst, damage, &red, false)) { // monster killed
            sprintf(buf, "the %s %s %s.",
                    theItemName,
                    (monst->info.flags & MONST_INANIMATE) ? "destroyed" : "killed",
                    targetName);
            messageWithColor(buf, messageColorFromVictim(monst), false);
        } else {
            sprintf(buf, "the %s hit %s.", theItemName, targetName);
            if (theItem->flags & ITEM_RUNIC) {
                magicWeaponHit(monst, theItem, false);
            }
            messageWithColor(buf, messageColorFromVictim(monst), false);
        }
        moralAttack(thrower, monst);
        if (armorRunicString[0]) {
            message(armorRunicString, false);
        }
        return true;
    } else {
        theItem->flags &= ~ITEM_PLAYER_AVOIDS; // Don't avoid thrown weapons that missed.
        sprintf(buf, "the %s missed %s.", theItemName, targetName);
        message(buf, false);
        return false;
    }
}

void throwItem(item *theItem, creature *thrower, short targetLoc[2], short maxDistance) {
    short listOfCoordinates[MAX_BOLT_LENGTH][2], originLoc[2];
    short i, x, y, numCells;
    creature *monst = NULL;
    char buf[COLS*3], buf2[COLS*3], buf3[COLS*3];
    enum displayGlyph displayChar;
    color foreColor, backColor, multColor;
    short dropLoc[2];
    boolean hitSomethingSolid = false, fastForward = false;
    enum dungeonLayers layer;

    theItem->flags |= ITEM_PLAYER_AVOIDS; // Avoid thrown items, unless it's a weapon that misses a monster.

    x = originLoc[0] = thrower->xLoc;
    y = originLoc[1] = thrower->yLoc;

    numCells = getLineCoordinates(listOfCoordinates, originLoc, targetLoc);

    thrower->ticksUntilTurn = thrower->attackSpeed;

    if (thrower != &player
        && (pmap[originLoc[0]][originLoc[1]].flags & IN_FIELD_OF_VIEW)) {

        monsterName(buf2, thrower, true);
        itemName(theItem, buf3, false, true, NULL);
        sprintf(buf, "%s hurls %s.", buf2, buf3);
        message(buf, false);
    }

    for (i=0; i<numCells && i < maxDistance; i++) {
        x = listOfCoordinates[i][0];
        y = listOfCoordinates[i][1];

        if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
            monst = monsterAtLoc(x, y);
            if (!(monst->bookkeepingFlags & MB_SUBMERGED)) {
//          if (projectileReflects(thrower, monst) && i < DCOLS*2) {
//              if (projectileReflects(thrower, monst)) { // if it scores another reflection roll, reflect at caster
//                  numCells = reflectBolt(originLoc[0], originLoc[1], listOfCoordinates, i, true);
//              } else {
//                  numCells = reflectBolt(-1, -1, listOfCoordinates, i, false); // otherwise reflect randomly
//              }
//
//              monsterName(buf2, monst, true);
//              itemName(theItem, buf3, false, false, NULL);
//              sprintf(buf, "%s deflect%s the %s", buf2, (monst == &player ? "" : "s"), buf3);
//              combatMessage(buf, 0);
//              continue;
//          }
                if ((theItem->category & WEAPON)
                    && theItem->kind != INCENDIARY_DART
                    && hitMonsterWithProjectileWeapon(thrower, monst, theItem)) {
                    deleteItem(theItem);
                    return;
                }
                break;
            }
        }

        // We hit something!
        if (cellHasTerrainFlag(x, y, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))) {
            if ((theItem->category & WEAPON)
                && (theItem->kind == INCENDIARY_DART)
                && (cellHasTerrainFlag(x, y, T_IS_FLAMMABLE) || (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)))) {
                // Incendiary darts thrown at flammable obstructions (foliage, wooden barricades, doors) will hit the obstruction
                // instead of bursting a cell earlier.
            } else if (cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY)
                       && cellHasTMFlag(x, y, TM_PROMOTES_ON_PLAYER_ENTRY)
                       && tileCatalog[pmap[x][y].layers[layerWithTMFlag(x, y, TM_PROMOTES_ON_PLAYER_ENTRY)]].flags & T_OBSTRUCTS_PASSABILITY) {
                layer = layerWithTMFlag(x, y, TM_PROMOTES_ON_PLAYER_ENTRY);
                if (tileCatalog[pmap[x][y].layers[layer]].flags & T_OBSTRUCTS_PASSABILITY) {
                    message(tileCatalog[pmap[x][y].layers[layer]].flavorText, false);
                    promoteTile(x, y, layer, false);
                }
            } else {
                i--;
                if (i >= 0) {
                    x = listOfCoordinates[i][0];
                    y = listOfCoordinates[i][1];
                } else { // it was aimed point-blank into an obstruction
                    x = thrower->xLoc;
                    y = thrower->yLoc;
                }
            }
            hitSomethingSolid = true;
            break;
        }

        if (playerCanSee(x, y)) { // show the graphic
            getCellAppearance(x, y, &displayChar, &foreColor, &backColor);
            foreColor = *(theItem->foreColor);
            if (playerCanDirectlySee(x, y)) {
                colorMultiplierFromDungeonLight(x, y, &multColor);
                applyColorMultiplier(&foreColor, &multColor);
            } else { // clairvoyant visible
                applyColorMultiplier(&foreColor, &clairvoyanceColor);
            }
            plotCharWithColor(theItem->displayChar, mapToWindowX(x), mapToWindowY(y), &foreColor, &backColor);

            if (!fastForward) {
                fastForward = rogue.playbackFastForward || pauseBrogue(25);
            }

            refreshDungeonCell(x, y);
        }

        if (x == targetLoc[0] && y == targetLoc[1]) { // reached its target
            break;
        }
    }

    if ((theItem->category & POTION) && (hitSomethingSolid || !cellHasTerrainFlag(x, y, T_AUTO_DESCENT))) {
        if (theItem->kind == POTION_CONFUSION || theItem->kind == POTION_POISON
            || theItem->kind == POTION_PARALYSIS || theItem->kind == POTION_INCINERATION
            || theItem->kind == POTION_DARKNESS || theItem->kind == POTION_LICHEN
            || theItem->kind == POTION_DESCENT) {
            switch (theItem->kind) {
                case POTION_POISON:
                    strcpy(buf, "the flask shatters and a deadly purple cloud billows out!");
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_POISON_GAS_CLOUD_POTION], true, false);
                    message(buf, false);
                    break;
                case POTION_CONFUSION:
                    strcpy(buf, "the flask shatters and a multi-hued cloud billows out!");
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_CONFUSION_GAS_CLOUD_POTION], true, false);
                    message(buf, false);
                    break;
                case POTION_PARALYSIS:
                    strcpy(buf, "the flask shatters and a cloud of pink gas billows out!");
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_PARALYSIS_GAS_CLOUD_POTION], true, false);
                    message(buf, false);
                    break;
                case POTION_INCINERATION:
                    strcpy(buf, "the flask shatters and its contents burst violently into flame!");
                    message(buf, false);
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_INCINERATION_POTION], true, false);
                    break;
                case POTION_DARKNESS:
                    strcpy(buf, "the flask shatters and the lights in the area start fading.");
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_DARKNESS_POTION], true, false);
                    message(buf, false);
                    break;
                case POTION_DESCENT:
                    strcpy(buf, "as the flask shatters, the ground vanishes!");
                    message(buf, false);
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_HOLE_POTION], true, false);
                    break;
                case POTION_LICHEN:
                    strcpy(buf, "the flask shatters and deadly spores spill out!");
                    message(buf, false);
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_LICHEN_PLANTED], true, false);
                    break;
            }

            autoIdentify(theItem);

            refreshDungeonCell(x, y);

            //if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
            //  monst = monsterAtLoc(x, y);
            //  applyInstantTileEffectsToCreature(monst);
            //}
        } else {
            if (cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY)) {
                strcpy(buf2, "against");
            } else if (tileCatalog[pmap[x][y].layers[highestPriorityLayer(x, y, false)]].mechFlags & TM_STAND_IN_TILE) {
                strcpy(buf2, "into");
            } else {
                strcpy(buf2, "on");
            }
            sprintf(buf, "the flask shatters and %s liquid splashes harmlessly %s %s.",
                    potionTable[theItem->kind].flavor, buf2, tileText(x, y));
            message(buf, false);
            if (theItem->kind == POTION_HALLUCINATION && (theItem->flags & ITEM_MAGIC_DETECTED)) {
                autoIdentify(theItem);
            }
        }
        deleteItem(theItem);
        return; // potions disappear when they break
    }
    if ((theItem->category & WEAPON) && theItem->kind == INCENDIARY_DART) {
        spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_DART_EXPLOSION], true, false);
        if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
            exposeCreatureToFire(monsterAtLoc(x, y));
        }
        deleteItem(theItem);
        return;
    }
    getQualifyingLocNear(dropLoc, x, y, true, 0, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_PASSABILITY), (HAS_ITEM), false, false);
    placeItem(theItem, dropLoc[0], dropLoc[1]);
    refreshDungeonCell(dropLoc[0], dropLoc[1]);
}

/*
Called when the player chooses to throw an item. theItem is optional; if it is
NULL, the player is prompted to choose one. If autoThrow is true and the last
targeted creature is still targetable, the item is thrown at it without prompting.
*/
void throwCommand(item *theItem, boolean autoThrow) {
    item *thrownItem;
    char buf[COLS], theName[COLS];
    unsigned char command[10];
    short maxDistance, zapTarget[2], quantity;
    boolean autoTarget;

    command[0] = THROW_KEY;

    //
    // From inventory, we know item
    // Else ask ITEM
    //
    if (theItem == NULL) {
        theItem = promptForItemOfType((ALL_ITEMS), 0, 0,
                                      KEYBOARD_LABELS ? "Throw what? (a-z, shift for more info; or <esc> to cancel)" : "Throw what?", true);
    }
    if (theItem == NULL) {
        return;
    }

    //
    // Change quantity to 1 to generate name of item ("a" and not "some, the, etc")
    //
    quantity = theItem->quantity;
    theItem->quantity = 1;
    itemName(theItem, theName, false, false, NULL);
    theItem->quantity = quantity;

    command[1] = theItem->inventoryLetter;
    confirmMessages();

    //
    // If special item (not throw item)
    // -> Confirm before throw
    if (((theItem->flags & ITEM_EQUIPPED) || theItem->timesEnchanted > 0)
        && theItem->quantity <= 1) {

        sprintf(buf, "Are you sure you want to throw your %s?", theName);
        if (!confirm(buf, false)) {
            return;
        }
        if (theItem->flags & ITEM_CURSED) {
            sprintf(buf, "You cannot unequip your %s; it appears to be cursed.", theName);
            messageWithColor(buf, &itemMessageColor, false);
            return;
        }
    }

    //
    // Ask location to throw
    //
    sprintf(buf, "Throw %s %s where? (<hjklyubn>, mouse, or <tab>)",
            (theItem->quantity > 1 ? "a" : "your"),
            theName);
    temporaryMessage(buf, false);
    maxDistance = (12 + 2 * max(rogue.strength - player.weaknessAmount - 12, 2));
    autoTarget = (theItem->category & (WEAPON | POTION)) ? true : false;

    if (autoThrow && creatureIsTargetable(rogue.lastTarget)) {
        zapTarget[0] = rogue.lastTarget->xLoc;
        zapTarget[1] = rogue.lastTarget->yLoc;
    } else if (!chooseTarget(zapTarget, maxDistance, true, autoTarget, false, false, &red)) {
        // player doesn't choose a target? return
        return;
    }

    if ((theItem->flags & ITEM_EQUIPPED) && theItem->quantity <= 1) {
        unequipItem(theItem, false);
    }
    command[2] = '\0';
    recordKeystrokeSequence(command);
    recordMouseClick(mapToWindowX(zapTarget[0]), mapToWindowY(zapTarget[1]), true, false);

    confirmMessages();

    thrownItem = generateItem(ALL_ITEMS, -1);   // generate item object in memory
    *thrownItem = *theItem;                     // clone the item
    thrownItem->flags &= ~ITEM_EQUIPPED;        // item not equiped
    thrownItem->quantity = 1;                   // item thrown, so quantity == 1

    itemName(thrownItem, theName, false, false, NULL); // update name of the thrown item

    throwItem(thrownItem, &player, zapTarget, maxDistance);

    // Update inventory
    // -> Now decrement or delete the thrown item out of the inventory.
    // -> Save last item thrown
    if (theItem->quantity > 1) {
        theItem->quantity--;
        rogue.lastItemThrown = theItem;
    } else {
        rogue.lastItemThrown = NULL;
        removeItemFromChain(theItem, packItems);
        deleteItem(theItem);
    }
    playerTurnEnded();
}

void relabel(item *theItem) {
    item *oldItem;
    char buf[COLS * 3], theName[COLS], newLabel;
    unsigned char command[10];

    if (!KEYBOARD_LABELS && !rogue.playbackMode) {
        return;
    }
    if (theItem == NULL) {
        theItem = promptForItemOfType((ALL_ITEMS), 0, 0,
                                      KEYBOARD_LABELS ? "Relabel what? (a-z, shift for more info; or <esc> to cancel)" : "Relabel what?", true);
    }
    if (theItem == NULL) {
        return;
    }
    temporaryMessage("New letter? (a-z)", false);
    newLabel = '\0';
    do {
        newLabel = nextKeyPress(true);
    } while (!newLabel);

    if (newLabel >= 'A' && newLabel <= 'Z') {
        newLabel += 'a' - 'A'; // lower-case.
    }
    if (newLabel >= 'a' && newLabel <= 'z') {
        if (newLabel != theItem->inventoryLetter) {
            command[0] = RELABEL_KEY;
            command[1] = theItem->inventoryLetter;
            command[2] = newLabel;
            command[3] = '\0';
            recordKeystrokeSequence(command);

            oldItem = itemOfPackLetter(newLabel);
            if (oldItem) {
                oldItem->inventoryLetter = theItem->inventoryLetter;
                itemName(oldItem, theName, true, true, NULL);
                sprintf(buf, "Relabeled %s as (%c);", theName, oldItem->inventoryLetter);
                messageWithColor(buf, &itemMessageColor, false);
            }
            theItem->inventoryLetter = newLabel;
            itemName(theItem, theName, true, true, NULL);
            sprintf(buf, "%selabeled %s as (%c).", oldItem ? " r" : "R", theName, newLabel);
            messageWithColor(buf, &itemMessageColor, false);
        } else {
            itemName(theItem, theName, true, true, NULL);
            sprintf(buf, "%s %s already labeled (%c).",
                    theName,
                    theItem->quantity == 1 ? "is" : "are",
                    theItem->inventoryLetter);
            messageWithColor(buf, &itemMessageColor, false);
        }
    }
}

// If the blink trajectory lands in lava based on the player's knowledge, abort.
// If the blink trajectory might land in lava based on the player's knowledge,
// prompt for confirmation.
boolean playerCancelsBlinking(const short originLoc[2], const short targetLoc[2], const short maxDistance) {
    short coordinates[DCOLS][2], impactLoc[2];
    short numCells, i, x, y;
    boolean certainDeath = false;
    boolean possibleDeath = false;
    unsigned long tFlags, tmFlags;

    if (rogue.playbackMode) {
        return false;
    }

    if (player.status[STATUS_IMMUNE_TO_FIRE]
        || player.status[STATUS_LEVITATING]) {
        return false;
    }

    getImpactLoc(impactLoc, originLoc, targetLoc, maxDistance > 0 ? maxDistance : DCOLS, true);
    getLocationFlags(impactLoc[0], impactLoc[1], &tFlags, &tmFlags, NULL, true);
    if (maxDistance > 0) {
        if ((pmap[impactLoc[0]][impactLoc[1]].flags & DISCOVERED)
            && (tFlags & T_LAVA_INSTA_DEATH)
            && !(tFlags & (T_ENTANGLES | T_AUTO_DESCENT))
            && !(tmFlags & TM_EXTINGUISHES_FIRE)) {

            certainDeath = possibleDeath = true;
        }
    } else {
        certainDeath = true;
        numCells = getLineCoordinates(coordinates, originLoc, targetLoc);
        for (i = 0; i < numCells; i++) {
            x = coordinates[i][0];
            y = coordinates[i][1];
            if (pmap[x][y].flags & DISCOVERED) {
                getLocationFlags(x, y, &tFlags, NULL, NULL, true);
                if ((tFlags & T_LAVA_INSTA_DEATH)
                    && !(tFlags & (T_ENTANGLES | T_AUTO_DESCENT))
                    && !(tmFlags & TM_EXTINGUISHES_FIRE)) {

                    possibleDeath = true;
                } else if (i >= staffBlinkDistance(2 * FP_FACTOR) - 1) {
                    // Found at least one possible safe landing spot.
                    certainDeath = false;
                }
            }
            if (x == impactLoc[0]
                && y == impactLoc[1]) {

                break;
            }
        }
    }
    if (possibleDeath && certainDeath) {
        message("that would be certain death!", false);
        return true;
    }
    if (possibleDeath
        && !confirm("Blink across lava with unknown range?", false)) {
        return true;
    }
    return false;
}

boolean useStaffOrWand(item *theItem, boolean *commandsRecorded) {
    char buf[COLS], buf2[COLS];
    unsigned char command[10];
    short zapTarget[2], originLoc[2], maxDistance, c;
    boolean autoTarget, targetAllies, autoID, boltKnown, passThroughCreatures, confirmedTarget;
    bolt theBolt;
    color trajectoryHiliteColor;

    c = 0;
    command[c++] = APPLY_KEY;
    command[c++] = theItem->inventoryLetter;

    if (theItem->charges <= 0 && (theItem->flags & ITEM_IDENTIFIED)) {
        itemName(theItem, buf2, false, false, NULL);
        sprintf(buf, "Your %s has no charges.", buf2);
        messageWithColor(buf, &itemMessageColor, false);
        return false;
    }
    temporaryMessage("Direction? (<hjklyubn>, mouse, or <tab>; <return> to confirm)", false);
    itemName(theItem, buf2, false, false, NULL);
    sprintf(buf, "Zapping your %s:", buf2);
    printString(buf, mapToWindowX(0), 1, &itemMessageColor, &black, NULL);

    theBolt = boltCatalog[tableForItemCategory(theItem->category, NULL)[theItem->kind].strengthRequired];
    if (theItem->category == STAFF) {
        theBolt.magnitude = theItem->enchant1;
    }

    if ((theItem->category & STAFF) && theItem->kind == STAFF_BLINKING
        && theItem->flags & (ITEM_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN)) {

        maxDistance = staffBlinkDistance(netEnchant(theItem));
    } else {
        maxDistance = -1;
    }
    if (tableForItemCategory(theItem->category, NULL)[theItem->kind].identified) {
        autoTarget = targetAllies = passThroughCreatures = false;
        if (!player.status[STATUS_HALLUCINATING]) {
            if (theBolt.flags & (BF_TARGET_ALLIES | BF_TARGET_ENEMIES)) {
                autoTarget = true;
            }
            if (theBolt.flags & BF_TARGET_ALLIES) {
                targetAllies = true;
            }
        }
        if (theBolt.flags & BF_PASSES_THRU_CREATURES) {
            passThroughCreatures = true;
        }
    } else {
        autoTarget = true;
        targetAllies = false;
        passThroughCreatures = false;
    }
    boltKnown = (((theItem->category & WAND) && wandTable[theItem->kind].identified)
                 || ((theItem->category & STAFF) && staffTable[theItem->kind].identified));
    if (!boltKnown) {
        trajectoryHiliteColor = gray;
    } else if (theBolt.backColor == NULL) {
        trajectoryHiliteColor = red;
    } else {
        trajectoryHiliteColor = *theBolt.backColor;
    }

    originLoc[0] = player.xLoc;
    originLoc[1] = player.yLoc;
    confirmedTarget = chooseTarget(zapTarget, maxDistance, false, autoTarget, targetAllies, passThroughCreatures, &trajectoryHiliteColor);
    if (confirmedTarget
        && boltKnown
        && theBolt.boltEffect == BE_BLINKING
        && playerCancelsBlinking(originLoc, zapTarget, maxDistance)) {

        confirmedTarget = false;
    }
    if (confirmedTarget) {

        command[c] = '\0';
        if (!(*commandsRecorded)) {
            recordKeystrokeSequence(command);
            recordMouseClick(mapToWindowX(zapTarget[0]), mapToWindowY(zapTarget[1]), true, false);
            *commandsRecorded = true;
        }
        confirmMessages();

        rogue.featRecord[FEAT_PURE_WARRIOR] = false;

        if (theItem->charges > 0) {
            autoID = zap(originLoc, zapTarget,
                         &theBolt,
                         !boltKnown);   // hide bolt details
            if (autoID) {
                if (!tableForItemCategory(theItem->category, NULL)[theItem->kind].identified) {
                    itemName(theItem, buf2, false, false, NULL);
                    sprintf(buf, "(Your %s must be ", buf2);
                    identifyItemKind(theItem);
                    itemName(theItem, buf2, false, true, NULL);
                    strcat(buf, buf2);
                    strcat(buf, ".)");
                    messageWithColor(buf, &itemMessageColor, false);
                }
            }
        } else {
            itemName(theItem, buf2, false, false, NULL);
            if (theItem->category == STAFF) {
                sprintf(buf, "Your %s fizzles; it must be out of charges for now.", buf2);
            } else {
                sprintf(buf, "Your %s fizzles; it must be depleted.", buf2);
            }
            messageWithColor(buf, &itemMessageColor, false);
            theItem->flags |= ITEM_MAX_CHARGES_KNOWN;
            playerTurnEnded();
            return false;
        }
    } else {
        return false;
    }
    return true;
}

void summonGuardian(item *theItem) {
    short x = player.xLoc, y = player.yLoc;
    creature *monst;

    monst = generateMonster(MK_CHARM_GUARDIAN, false, false);
    getQualifyingPathLocNear(&(monst->xLoc), &(monst->yLoc), x, y, true,
                             T_DIVIDES_LEVEL & avoidedFlagsForMonster(&(monst->info)) & ~T_SPONTANEOUSLY_IGNITES, HAS_PLAYER,
                             avoidedFlagsForMonster(&(monst->info)) & ~T_SPONTANEOUSLY_IGNITES, (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS), false);
    monst->bookkeepingFlags |= (MB_FOLLOWER | MB_BOUND_TO_LEADER | MB_DOES_NOT_TRACK_LEADER);
    monst->bookkeepingFlags &= ~MB_JUST_SUMMONED;
    monst->leader = &player;
    monst->creatureState = MONSTER_ALLY;
    monst->ticksUntilTurn = monst->info.attackSpeed + 1; // So they don't move before the player's next turn.
    monst->status[STATUS_LIFESPAN_REMAINING] = monst->maxStatus[STATUS_LIFESPAN_REMAINING] = charmGuardianLifespan(netEnchant(theItem));
    pmap[monst->xLoc][monst->yLoc].flags |= HAS_MONSTER;
    fadeInMonster(monst);
}

void useCharm(item *theItem) {
    fixpt enchant = netEnchant(theItem);

    rogue.featRecord[FEAT_PURE_WARRIOR] = false;

    switch (theItem->kind) {
        case CHARM_HEALTH:
            heal(&player, charmHealing(enchant), false);
            message("You feel much healthier.", false);
            break;
        case CHARM_PROTECTION:
            if (charmProtection(enchant) > player.status[STATUS_SHIELDED]) {
                player.status[STATUS_SHIELDED] = charmProtection(enchant);
            }
            player.maxStatus[STATUS_SHIELDED] = player.status[STATUS_SHIELDED];
            if (boltCatalog[BOLT_SHIELDING].backColor) {
                flashMonster(&player, boltCatalog[BOLT_SHIELDING].backColor, 100);
            }
            message("A shimmering shield coalesces around you.", false);
            break;
        case CHARM_HASTE:
            haste(&player, charmEffectDuration(theItem->kind, theItem->enchant1));
            break;
        case CHARM_FIRE_IMMUNITY:
            player.status[STATUS_IMMUNE_TO_FIRE] = player.maxStatus[STATUS_IMMUNE_TO_FIRE] = charmEffectDuration(theItem->kind, theItem->enchant1);
            if (player.status[STATUS_BURNING]) {
                extinguishFireOnCreature(&player);
            }
            message("you no longer fear fire.", false);
            break;
        case CHARM_INVISIBILITY:
            imbueInvisibility(&player, charmEffectDuration(theItem->kind, theItem->enchant1));
            message("You shiver as a chill runs up your spine.", false);
            break;
        case CHARM_TELEPATHY:
            makePlayerTelepathic(charmEffectDuration(theItem->kind, theItem->enchant1));
            break;
        case CHARM_LEVITATION:
            player.status[STATUS_LEVITATING] = player.maxStatus[STATUS_LEVITATING] = charmEffectDuration(theItem->kind, theItem->enchant1);
            player.bookkeepingFlags &= ~MB_SEIZED; // break free of holding monsters
            message("you float into the air!", false);
            break;
        case CHARM_SHATTERING:
            messageWithColor("your charm emits a wave of turquoise light that pierces the nearby walls!", &itemMessageColor, false);
            crystalize(charmShattering(enchant));
            break;
        case CHARM_GUARDIAN:
            messageWithColor("your charm flashes and the form of a mythical guardian coalesces!", &itemMessageColor, false);
            summonGuardian(theItem);
            break;
        case CHARM_TELEPORTATION:
            teleport(&player, -1, -1, true);
            break;
        case CHARM_RECHARGING:
            rechargeItems(STAFF);
            break;
        case CHARM_NEGATION:
            negationBlast("your charm", charmNegationRadius(enchant) + 1); // Add 1 because otherwise radius 1 would affect only the player.
            break;
        default:
            break;
    }
}

void apply(item *theItem, boolean recordCommands) {
    char buf[COLS * 3], buf2[COLS * 3];
    boolean commandsRecorded, revealItemType;
    unsigned char command[10] = "";
    short c;

    commandsRecorded = !recordCommands;
    c = 0;
    command[c++] = APPLY_KEY;

    revealItemType = false;

    if (!theItem) {
        theItem = promptForItemOfType((SCROLL|FOOD|POTION|STAFF|WAND|CHARM), 0, 0,
                                      KEYBOARD_LABELS ? "Apply what? (a-z, shift for more info; or <esc> to cancel)" : "Apply what?",
                                      true);
    }

    if (theItem == NULL) {
        return;
    }

    if ((theItem->category == SCROLL || theItem->category == POTION)
        && magicCharDiscoverySuffix(theItem->category, theItem->kind) == -1
        && ((theItem->flags & ITEM_MAGIC_DETECTED) || tableForItemCategory(theItem->category, NULL)[theItem->kind].identified)) {

        if (tableForItemCategory(theItem->category, NULL)[theItem->kind].identified) {
            sprintf(buf,
                    "Really %s a %s of %s?",
                    theItem->category == SCROLL ? "read" : "drink",
                    theItem->category == SCROLL ? "scroll" : "potion",
                    tableForItemCategory(theItem->category, NULL)[theItem->kind].name);
        } else {
            sprintf(buf,
                    "Really %s a cursed %s?",
                    theItem->category == SCROLL ? "read" : "drink",
                    theItem->category == SCROLL ? "scroll" : "potion");
        }
        if (!confirm(buf, false)) {
            return;
        }
    }

    command[c++] = theItem->inventoryLetter;
    confirmMessages();
    switch (theItem->category) {
        case FOOD:
            if (STOMACH_SIZE - player.status[STATUS_NUTRITION] < foodTable[theItem->kind].strengthRequired) { // Not hungry enough.
                sprintf(buf, "You're not hungry enough to fully enjoy the %s. Eat it anyway?",
                        (theItem->kind == RATION ? "food" : "mango"));
                if (!confirm(buf, false)) {
                    return;
                }
            }
            player.status[STATUS_NUTRITION] = min(foodTable[theItem->kind].strengthRequired + player.status[STATUS_NUTRITION], STOMACH_SIZE);
            if (theItem->kind == RATION) {
                messageWithColor("That food tasted delicious!", &itemMessageColor, false);
            } else {
                messageWithColor("My, what a yummy mango!", &itemMessageColor, false);
            }
            rogue.featRecord[FEAT_MYSTIC] = false;
            break;
        case POTION:
            command[c] = '\0';
            if (!commandsRecorded) {
                recordKeystrokeSequence(command);
                commandsRecorded = true;
            }
            if (!potionTable[theItem->kind].identified) {
                revealItemType = true;
            }
            drinkPotion(theItem);
            break;
        case SCROLL:
            command[c] = '\0';
            if (!commandsRecorded) {
                recordKeystrokeSequence(command);
                commandsRecorded = true; // have to record in case further keystrokes are necessary (e.g. enchant scroll)
            }
            if (!scrollTable[theItem->kind].identified
                && theItem->kind != SCROLL_ENCHANTING
                && theItem->kind != SCROLL_IDENTIFY) {

                revealItemType = true;
            }
            readScroll(theItem);
            break;
        case STAFF:
        case WAND:
            if (!useStaffOrWand(theItem, &commandsRecorded)) {
                return;
            }
            break;
        case CHARM:
            if (theItem->charges > 0) {
                itemName(theItem, buf2, false, false, NULL);
                sprintf(buf, "Your %s hasn't finished recharging.", buf2);
                messageWithColor(buf, &itemMessageColor, false);
                return;
            }
            if (!commandsRecorded) {
                command[c] = '\0';
                recordKeystrokeSequence(command);
                commandsRecorded = true;
            }
            useCharm(theItem);
            break;
        default:
            itemName(theItem, buf2, false, true, NULL);
            sprintf(buf, "you can't apply %s.", buf2);
            message(buf, false);
            return;
    }

    if (!commandsRecorded) { // to make sure we didn't already record the keystrokes above with staff/wand targeting
        command[c] = '\0';
        recordKeystrokeSequence(command);
        commandsRecorded = true;
    }

    // Reveal the item type if appropriate.
    if (revealItemType) {
        autoIdentify(theItem);
    }

    if (theItem->category & CHARM) {
        theItem->charges = charmRechargeDelay(theItem->kind, theItem->enchant1);
    } else if (theItem->charges > 0) {
        theItem->charges--;
        if (theItem->category == WAND) {
            theItem->enchant2++; // keeps track of how many times the wand has been discharged for the player's convenience
        }
    } else if (theItem->quantity > 1) {
        theItem->quantity--;
    } else {
        removeItemFromChain(theItem, packItems);
        deleteItem(theItem);
    }
    playerTurnEnded();
}

void identify(item *theItem) {
    theItem->flags |= ITEM_IDENTIFIED;
    theItem->flags &= ~ITEM_CAN_BE_IDENTIFIED;
    if (theItem->flags & ITEM_RUNIC) {
        theItem->flags |= (ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    }
    if (theItem->category & RING) {
        updateRingBonuses();
    }
    identifyItemKind(theItem);
}
/*
enum monsterTypes chooseVorpalEnemy() {
    short i, index, possCount = 0, deepestLevel = 0, deepestHorde, chosenHorde, failsafe = 25;
    enum monsterTypes candidate;

    for (i=0; i<NUMBER_HORDES; i++) {
        if (hordeCatalog[i].minLevel >= rogue.depthLevel && !hordeCatalog[i].flags) {
            possCount += hordeCatalog[i].frequency;
        }
        if (hordeCatalog[i].minLevel > deepestLevel) {
            deepestHorde = i;
            deepestLevel = hordeCatalog[i].minLevel;
        }
    }

    do {
        if (possCount == 0) {
            chosenHorde = deepestHorde;
        } else {
            index = rand_range(1, possCount);
            for (i=0; i<NUMBER_HORDES; i++) {
                if (hordeCatalog[i].minLevel >= rogue.depthLevel && !hordeCatalog[i].flags) {
                    if (index <= hordeCatalog[i].frequency) {
                        chosenHorde = i;
                        break;
                    }
                    index -= hordeCatalog[i].frequency;
                }
            }
        }

        index = rand_range(-1, hordeCatalog[chosenHorde].numberOfMemberTypes - 1);
        if (index == -1) {
            candidate = hordeCatalog[chosenHorde].leaderType;
        } else {
            candidate = hordeCatalog[chosenHorde].memberType[index];
        }
    } while (((monsterCatalog[candidate].flags & MONST_NEVER_VORPAL_ENEMY)
              || (monsterCatalog[candidate].abilityFlags & MA_NEVER_VORPAL_ENEMY))
             && --failsafe > 0);
    return candidate;
}*/

short lotteryDraw(short *frequencies, short itemCount) {
    short i, maxFreq, randIndex;
    maxFreq = 0;
    for (i = 0; i < itemCount; i++) {
        maxFreq += frequencies[i];
    }
    brogueAssert(maxFreq > 0);
    randIndex = rand_range(0, maxFreq - 1);
    for (i = 0; i < itemCount; i++) {
        if (frequencies[i] > randIndex) {
            return i;
        } else {
            randIndex -= frequencies[i];
        }
    }
    brogueAssert(false);
    return 0;
}

short chooseVorpalEnemy() {
    short i, frequencies[MONSTER_CLASS_COUNT];
    for (i = 0; i < MONSTER_CLASS_COUNT; i++) {
        if (monsterClassCatalog[i].maxDepth <= 0
            || rogue.depthLevel <= monsterClassCatalog[i].maxDepth) {

            frequencies[i] = monsterClassCatalog[i].frequency;
        } else {
            frequencies[i] = 0;
        }
    }
    return lotteryDraw(frequencies, MONSTER_CLASS_COUNT);
}

void describeMonsterClass(char *buf, const short classID, boolean conjunctionAnd) {
    short i;
    char buf2[50];

    buf[0] = '\0';
    for (i = 0; monsterClassCatalog[classID].memberList[i] != 0; i++) {
        strcpy(buf2, monsterCatalog[monsterClassCatalog[classID].memberList[i]].monsterName);
        if (monsterClassCatalog[classID].memberList[i + 1] != 0) {
            if (monsterClassCatalog[classID].memberList[i + 2] == 0) {
                strcat(buf2, conjunctionAnd ? " and " : " or ");
            } else {
                strcat(buf2, ", ");
            }
        }
        strcat(buf, buf2);
    }
}

void updateIdentifiableItem(item *theItem) {
    if ((theItem->category & SCROLL) && scrollTable[theItem->kind].identified) {
        theItem->flags &= ~ITEM_CAN_BE_IDENTIFIED;
    } else if ((theItem->category & POTION) && potionTable[theItem->kind].identified) {
        theItem->flags &= ~ITEM_CAN_BE_IDENTIFIED;
    } else if ((theItem->category & (RING | STAFF | WAND))
               && (theItem->flags & ITEM_IDENTIFIED)
               && tableForItemCategory(theItem->category, NULL)[theItem->kind].identified) {

        theItem->flags &= ~ITEM_CAN_BE_IDENTIFIED;
    } else if ((theItem->category & (WEAPON | ARMOR))
               && (theItem->flags & ITEM_IDENTIFIED)
               && (!(theItem->flags & ITEM_RUNIC) || (theItem->flags & ITEM_RUNIC_IDENTIFIED))) {

        theItem->flags &= ~ITEM_CAN_BE_IDENTIFIED;
    } else if (theItem->category & NEVER_IDENTIFIABLE) {
        theItem->flags &= ~ITEM_CAN_BE_IDENTIFIED;
    }
}

void updateIdentifiableItems() {
    item *theItem;
    for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        updateIdentifiableItem(theItem);
    }
    for (theItem = floorItems; theItem != NULL; theItem = theItem->nextItem) {
        updateIdentifiableItem(theItem);
    }
}

void magicMapCell(short x, short y) {
    pmap[x][y].flags |= MAGIC_MAPPED;
    pmap[x][y].rememberedTerrainFlags = tileCatalog[pmap[x][y].layers[DUNGEON]].flags | tileCatalog[pmap[x][y].layers[LIQUID]].flags;
    pmap[x][y].rememberedTMFlags = tileCatalog[pmap[x][y].layers[DUNGEON]].mechFlags | tileCatalog[pmap[x][y].layers[LIQUID]].mechFlags;
    if (pmap[x][y].layers[LIQUID] && tileCatalog[pmap[x][y].layers[LIQUID]].drawPriority < tileCatalog[pmap[x][y].layers[DUNGEON]].drawPriority) {
        pmap[x][y].rememberedTerrain = pmap[x][y].layers[LIQUID];
    } else {
        pmap[x][y].rememberedTerrain = pmap[x][y].layers[DUNGEON];
    }
}

void readScroll(item *theItem) {
    short i, j, x, y, numberOfMonsters = 0;
    item *tempItem;
    creature *monst;
    boolean hadEffect = false;
    char buf[COLS * 3], buf2[COLS * 3];

    rogue.featRecord[FEAT_ARCHIVIST] = false;

    switch (theItem->kind) {
        case SCROLL_IDENTIFY:
            identify(theItem);
            updateIdentifiableItems();
            messageWithColor("this is a scroll of identify.", &itemMessageColor, true);
            if (numberOfMatchingPackItems(ALL_ITEMS, ITEM_CAN_BE_IDENTIFIED, 0, false) == 0) {
                message("everything in your pack is already identified.", false);
                break;
            }
            do {
                theItem = promptForItemOfType((ALL_ITEMS), ITEM_CAN_BE_IDENTIFIED, 0,
                                              KEYBOARD_LABELS ? "Identify what? (a-z; shift for more info)" : "Identify what?",
                                              false);
                if (rogue.gameHasEnded) {
                    return;
                }
                if (theItem && !(theItem->flags & ITEM_CAN_BE_IDENTIFIED)) {
                    confirmMessages();
                    itemName(theItem, buf2, true, true, NULL);
                    sprintf(buf, "you already know %s %s.", (theItem->quantity > 1 ? "they're" : "it's"), buf2);
                    messageWithColor(buf, &itemMessageColor, false);
                }
            } while (theItem == NULL || !(theItem->flags & ITEM_CAN_BE_IDENTIFIED));
            recordKeystroke(theItem->inventoryLetter, false, false);
            confirmMessages();
            identify(theItem);
            itemName(theItem, buf, true, true, NULL);
            sprintf(buf2, "%s %s.", (theItem->quantity == 1 ? "this is" : "these are"), buf);
            messageWithColor(buf2, &itemMessageColor, false);
            break;
        case SCROLL_TELEPORT:
            teleport(&player, -1, -1, true);
            break;
        case SCROLL_REMOVE_CURSE:
            for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
                if (tempItem->flags & ITEM_CURSED) {
                    hadEffect = true;
                    tempItem->flags &= ~ITEM_CURSED;
                }
            }
            if (hadEffect) {
                message("your pack glows with a cleansing light, and a malevolent energy disperses.", false);
            } else {
                message("your pack glows with a cleansing light, but nothing happens.", false);
            }
            break;
        case SCROLL_ENCHANTING:
            identify(theItem);
            messageWithColor("this is a scroll of enchanting.", &itemMessageColor, true);
            if (!numberOfMatchingPackItems((WEAPON | ARMOR | RING | STAFF | WAND | CHARM), 0, 0, false)) {
                confirmMessages();
                message("you have nothing that can be enchanted.", false);
                break;
            }
            do {
                theItem = promptForItemOfType((WEAPON | ARMOR | RING | STAFF | WAND | CHARM), 0, 0,
                                              KEYBOARD_LABELS ? "Enchant what? (a-z; shift for more info)" : "Enchant what?",
                                              false);
                confirmMessages();
                if (theItem == NULL || !(theItem->category & (WEAPON | ARMOR | RING | STAFF | WAND | CHARM))) {
                    message("Can't enchant that.", true);
                }
                if (rogue.gameHasEnded) {
                    return;
                }
            } while (theItem == NULL || !(theItem->category & (WEAPON | ARMOR | RING | STAFF | WAND | CHARM)));
            recordKeystroke(theItem->inventoryLetter, false, false);
            confirmMessages();
            switch (theItem->category) {
                case WEAPON:
                    theItem->strengthRequired = max(0, theItem->strengthRequired - 1);
                    theItem->enchant1++;
                    if (theItem->quiverNumber) {
                        theItem->quiverNumber = rand_range(1, 60000);
                    }
                    break;
                case ARMOR:
                    theItem->strengthRequired = max(0, theItem->strengthRequired - 1);
                    theItem->enchant1++;
                    break;
                case RING:
                    theItem->enchant1++;
                    updateRingBonuses();
                    if (theItem->kind == RING_CLAIRVOYANCE) {
                        updateClairvoyance();
                        displayLevel();
                    }
                    break;
                case STAFF:
                    theItem->enchant1++;
                    theItem->charges++;
                    theItem->enchant2 = 500 / theItem->enchant1;
                    break;
                case WAND:
                    theItem->charges += wandTable[theItem->kind].range.lowerBound;
                    break;
                case CHARM:
                    theItem->enchant1++;
                    theItem->charges = min(0, theItem->charges); // Enchanting instantly recharges charms.
                                                                 //                    theItem->charges = theItem->charges
                                                                 //                    * charmRechargeDelay(theItem->kind, theItem->enchant1)
                                                                 //                    / charmRechargeDelay(theItem->kind, theItem->enchant1 - 1);

                    break;
                default:
                    break;
            }
            theItem->timesEnchanted++;
            if ((theItem->category & (WEAPON | ARMOR | STAFF | RING | CHARM))
                && theItem->enchant1 >= 16) {

                rogue.featRecord[FEAT_SPECIALIST] = true;
            }
            if (theItem->flags & ITEM_EQUIPPED) {
                equipItem(theItem, true);
            }
            itemName(theItem, buf, false, false, NULL);
            sprintf(buf2, "your %s gleam%s briefly in the darkness.", buf, (theItem->quantity == 1 ? "s" : ""));
            messageWithColor(buf2, &itemMessageColor, false);
            if (theItem->flags & ITEM_CURSED) {
                sprintf(buf2, "a malevolent force leaves your %s.", buf);
                messageWithColor(buf2, &itemMessageColor, false);
                theItem->flags &= ~ITEM_CURSED;
            }
            createFlare(player.xLoc, player.yLoc, SCROLL_ENCHANTMENT_LIGHT);
            break;
        case SCROLL_RECHARGING:
            rechargeItems(STAFF | CHARM);
            break;
        case SCROLL_PROTECT_ARMOR:
            if (rogue.armor) {
                tempItem = rogue.armor;
                tempItem->flags |= ITEM_PROTECTED;
                itemName(tempItem, buf2, false, false, NULL);
                sprintf(buf, "a protective golden light covers your %s.", buf2);
                messageWithColor(buf, &itemMessageColor, false);
                if (tempItem->flags & ITEM_CURSED) {
                    sprintf(buf, "a malevolent force leaves your %s.", buf2);
                    messageWithColor(buf, &itemMessageColor, false);
                    tempItem->flags &= ~ITEM_CURSED;
                }
            } else {
                message("a protective golden light surrounds you, but it quickly disperses.", false);
            }
            createFlare(player.xLoc, player.yLoc, SCROLL_PROTECTION_LIGHT);
            break;
        case SCROLL_PROTECT_WEAPON:
            if (rogue.weapon) {
                tempItem = rogue.weapon;
                tempItem->flags |= ITEM_PROTECTED;
                itemName(tempItem, buf2, false, false, NULL);
                sprintf(buf, "a protective golden light covers your %s.", buf2);
                messageWithColor(buf, &itemMessageColor, false);
                if (tempItem->flags & ITEM_CURSED) {
                    sprintf(buf, "a malevolent force leaves your %s.", buf2);
                    messageWithColor(buf, &itemMessageColor, false);
                    tempItem->flags &= ~ITEM_CURSED;
                }
                if (rogue.weapon->quiverNumber) {
                    rogue.weapon->quiverNumber = rand_range(1, 60000);
                }
            } else {
                message("a protective golden light covers your empty hands, but it quickly disperses.", false);
            }
            createFlare(player.xLoc, player.yLoc, SCROLL_PROTECTION_LIGHT);
            break;
        case SCROLL_SANCTUARY:
            spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_SACRED_GLYPHS], true, false);
            messageWithColor("sprays of color arc to the ground, forming glyphs where they alight.", &itemMessageColor, false);
            break;
        case SCROLL_MAGIC_MAPPING:
            confirmMessages();
            messageWithColor("this scroll has a map on it!", &itemMessageColor, false);
            for (i=0; i<DCOLS; i++) {
                for (j=0; j<DROWS; j++) {
                    if (cellHasTMFlag(i, j, TM_IS_SECRET)) {
                        discover(i, j);
                        magicMapCell(i, j);
                        pmap[i][j].flags &= ~(STABLE_MEMORY | DISCOVERED);
                    }
                }
            }
            for (i=0; i<DCOLS; i++) {
                for (j=0; j<DROWS; j++) {
                    if (!(pmap[i][j].flags & DISCOVERED) && pmap[i][j].layers[DUNGEON] != GRANITE) {
                        magicMapCell(i, j);
                    }
                }
            }
            colorFlash(&magicMapFlashColor, 0, MAGIC_MAPPED, 15, DCOLS + DROWS, player.xLoc, player.yLoc);
            break;
        case SCROLL_AGGRAVATE_MONSTER:
            aggravateMonsters(DCOLS + DROWS, player.xLoc, player.yLoc, &gray);
            message("the scroll emits a piercing shriek that echoes throughout the dungeon!", false);
            break;
        case SCROLL_SUMMON_MONSTER:
            for (j=0; j<25 && numberOfMonsters < 3; j++) {
                for (i=0; i<8; i++) {
                    x = player.xLoc + nbDirs[i][0];
                    y = player.yLoc + nbDirs[i][1];
                    if (!cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY) && !(pmap[x][y].flags & HAS_MONSTER)
                        && rand_percent(10) && (numberOfMonsters < 3)) {
                        monst = spawnHorde(0, x, y, (HORDE_LEADER_CAPTIVE | HORDE_NO_PERIODIC_SPAWN | HORDE_IS_SUMMONED | HORDE_MACHINE_ONLY), 0);
                        if (monst) {
                            // refreshDungeonCell(x, y);
                            // monst->creatureState = MONSTER_TRACKING_SCENT;
                            // monst->ticksUntilTurn = player.movementSpeed;
                            wakeUp(monst);
                            fadeInMonster(monst);
                            numberOfMonsters++;
                        }
                    }
                }
            }
            if (numberOfMonsters > 1) {
                message("the fabric of space ripples, and monsters appear!", false);
            } else if (numberOfMonsters == 1) {
                message("the fabric of space ripples, and a monster appears!", false);
            } else {
                message("the fabric of space boils violently around you, but nothing happens.", false);
            }
            break;
        case SCROLL_NEGATION:
            negationBlast("the scroll", DCOLS);
            break;
        case SCROLL_SHATTERING:
            messageWithColor("the scroll emits a wave of turquoise light that pierces the nearby walls!", &itemMessageColor, false);
            crystalize(9);
            break;
        case SCROLL_DISCORD:
            discordBlast("the scroll", DCOLS);
            break;
    }
}

void detectMagicOnItem(item *theItem) {
    theItem->flags |= ITEM_MAGIC_DETECTED;
    if ((theItem->category & (WEAPON | ARMOR))
        && theItem->enchant1 == 0
        && !(theItem->flags & ITEM_RUNIC)) {

        identify(theItem);
    }
}

void drinkPotion(item *theItem) {
    item *tempItem = NULL;
    creature *monst = NULL;
    boolean hadEffect = false;
    boolean hadEffect2 = false;
    char buf[1000] = "";

    brogueAssert(rogue.RNG == RNG_SUBSTANTIVE);

    rogue.featRecord[FEAT_ARCHIVIST] = false;

    switch (theItem->kind) {
        case POTION_LIFE:
            sprintf(buf, "%syour maximum health increases by %i%%.",
                    ((player.currentHP < player.info.maxHP) ? "you heal completely and " : ""),
                    (player.info.maxHP + 10) * 100 / player.info.maxHP - 100);

            player.info.maxHP += 10;
            heal(&player, 100, true);
            updatePlayerRegenerationDelay();
            messageWithColor(buf, &advancementMessageColor, false);
            break;
        case POTION_HALLUCINATION:
            player.status[STATUS_HALLUCINATING] = player.maxStatus[STATUS_HALLUCINATING] = 300;
            message("colors are everywhere! The walls are singing!", false);
            break;
        case POTION_INCINERATION:
            //colorFlash(&darkOrange, 0, IN_FIELD_OF_VIEW, 4, 4, player.xLoc, player.yLoc);
            message("as you uncork the flask, it explodes in flame!", false);
            spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_INCINERATION_POTION], true, false);
            exposeCreatureToFire(&player);
            break;
        case POTION_DARKNESS:
            player.status[STATUS_DARKNESS] = max(400, player.status[STATUS_DARKNESS]);
            player.maxStatus[STATUS_DARKNESS] = max(400, player.maxStatus[STATUS_DARKNESS]);
            updateMinersLightRadius();
            updateVision(true);
            message("your vision flickers as a cloak of darkness settles around you!", false);
            break;
        case POTION_DESCENT:
            colorFlash(&darkBlue, 0, IN_FIELD_OF_VIEW, 3, 3, player.xLoc, player.yLoc);
            message("vapor pours out of the flask and causes the floor to disappear!", false);
            spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_HOLE_POTION], true, false);
            if (!player.status[STATUS_LEVITATING]) {
                player.bookkeepingFlags |= MB_IS_FALLING;
            }
            break;
        case POTION_STRENGTH:
            rogue.strength++;
            if (player.status[STATUS_WEAKENED]) {
                player.status[STATUS_WEAKENED] = 1;
            }
            updateEncumbrance();
            messageWithColor("newfound strength surges through your body.", &advancementMessageColor, false);
            createFlare(player.xLoc, player.yLoc, POTION_STRENGTH_LIGHT);
            break;
        case POTION_POISON:
            spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_POISON_GAS_CLOUD_POTION], true, false);
            message("caustic gas billows out of the open flask!", false);
            break;
        case POTION_PARALYSIS:
            spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_PARALYSIS_GAS_CLOUD_POTION], true, false);
            message("your muscles stiffen as a cloud of pink gas bursts from the open flask!", false);
            break;
        case POTION_TELEPATHY:
            makePlayerTelepathic(300);
            break;
        case POTION_LEVITATION:
            player.status[STATUS_LEVITATING] = player.maxStatus[STATUS_LEVITATING] = 100;
            player.bookkeepingFlags &= ~MB_SEIZED; // break free of holding monsters
            message("you float into the air!", false);
            break;
        case POTION_CONFUSION:
            spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_CONFUSION_GAS_CLOUD_POTION], true, false);
            message("a shimmering cloud of rainbow-colored gas billows out of the open flask!", false);
            break;
        case POTION_LICHEN:
            message("a handful of tiny spores burst out of the open flask!", false);
            spawnDungeonFeature(player.xLoc, player.yLoc, &dungeonFeatureCatalog[DF_LICHEN_PLANTED], true, false);
            break;
        case POTION_DETECT_MAGIC:
            hadEffect = false;
            hadEffect2 = false;
            for (tempItem = floorItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
                if (tempItem->category & CAN_BE_DETECTED) {
                    detectMagicOnItem(tempItem);
                    if (itemMagicPolarity(tempItem)) {
                        pmap[tempItem->xLoc][tempItem->yLoc].flags |= ITEM_DETECTED;
                        hadEffect = true;
                        refreshDungeonCell(tempItem->xLoc, tempItem->yLoc);
                    }
                }
            }
            for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
                if (monst->carriedItem && (monst->carriedItem->category & CAN_BE_DETECTED)) {
                    detectMagicOnItem(monst->carriedItem);
                    if (itemMagicPolarity(monst->carriedItem)) {
                        hadEffect = true;
                        refreshDungeonCell(monst->xLoc, monst->yLoc);
                    }
                }
            }
            for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
                if (tempItem->category & CAN_BE_DETECTED) {
                    detectMagicOnItem(tempItem);
                    if (itemMagicPolarity(tempItem)) {
                        if (tempItem->flags & ITEM_MAGIC_DETECTED) {
                            hadEffect2 = true;
                        }
                    }
                }
            }
            if (hadEffect || hadEffect2) {
                if (hadEffect && hadEffect2) {
                    message("you can somehow feel the presence of magic on the level and in your pack.", false);
                } else if (hadEffect) {
                    message("you can somehow feel the presence of magic on the level.", false);
                } else {
                    message("you can somehow feel the presence of magic in your pack.", false);
                }
            } else {
                message("you can somehow feel the absence of magic on the level and in your pack.", false);
            }
            break;
        case POTION_HASTE_SELF:
            haste(&player, 25);
            break;
        case POTION_FIRE_IMMUNITY:
            player.status[STATUS_IMMUNE_TO_FIRE] = player.maxStatus[STATUS_IMMUNE_TO_FIRE] = 150;
            if (player.status[STATUS_BURNING]) {
                extinguishFireOnCreature(&player);
            }
            message("a comforting breeze envelops you, and you no longer fear fire.", false);
            break;
        case POTION_INVISIBILITY:
            player.status[STATUS_INVISIBLE] = player.maxStatus[STATUS_INVISIBLE] = 75;
            message("you shiver as a chill runs up your spine.", false);
            break;
        default:
            message("you feel very strange, as though your body doesn't know how to react!", true);
    }
}

// Used for the Discoveries screen. Returns a number: 1 == good, -1 == bad, 0 == could go either way.
short magicCharDiscoverySuffix(short category, short kind) {
    short result = 0;

    switch (category) {
        case SCROLL:
            switch (kind) {
                case SCROLL_AGGRAVATE_MONSTER:
                case SCROLL_SUMMON_MONSTER:
                    result = -1;
                    break;
                default:
                    result = 1;
                    break;
            }
            break;
        case POTION:
            switch (kind) {
                case POTION_HALLUCINATION:
                case POTION_INCINERATION:
                case POTION_DESCENT:
                case POTION_POISON:
                case POTION_PARALYSIS:
                case POTION_CONFUSION:
                case POTION_LICHEN:
                case POTION_DARKNESS:
                    result = -1;
                    break;
                default:
                    result = 1;
                    break;
            }
            break;
        case WAND:
        case STAFF:
            if (boltCatalog[tableForItemCategory(category, NULL)[kind].strengthRequired].flags & (BF_TARGET_ALLIES)) {
                result = -1;
            } else {
                result = 1;
            }
            break;
        case RING:
            result = 0;
            break;
        case CHARM:
            result = 1;
            break;
    }
    return result;
}

/* Returns
-1 if the item is of bad magic
 0 if it is neutral
 1 if it is of good magic */
int itemMagicPolarity(item *theItem) {
    switch (theItem->category) {
        case WEAPON:
        case ARMOR:
            if ((theItem->flags & ITEM_CURSED) || theItem->enchant1 < 0) {
                return -1;
            } else if (theItem->enchant1 > 0) {
                return 1;
            }
            return 0;
            break;
        case SCROLL:
            switch (theItem->kind) {
                case SCROLL_AGGRAVATE_MONSTER:
                case SCROLL_SUMMON_MONSTER:
                    return -1;
                default:
                    return 1;
            }
        case POTION:
            switch (theItem->kind) {
                case POTION_HALLUCINATION:
                case POTION_INCINERATION:
                case POTION_DESCENT:
                case POTION_POISON:
                case POTION_PARALYSIS:
                case POTION_CONFUSION:
                case POTION_LICHEN:
                case POTION_DARKNESS:
                    return -1;
                default:
                    return 1;
            }
        case WAND:
            if (theItem->charges == 0) {
                return 0;
            }
        case STAFF:
            if (boltCatalog[tableForItemCategory(theItem->category, NULL)[theItem->kind].strengthRequired].flags & (BF_TARGET_ALLIES)) {
                return -1;
            } else {
                return 1;
            }
        case RING:
            if (theItem->flags & ITEM_CURSED || theItem->enchant1 < 0) {
                return -1;
            } else if (theItem->enchant1 > 0) {
                return 1;
            } else {
                return 0;
            }
        case CHARM:
            return 1;
        case AMULET:
            return 1;
    }
    return 0;
}

void unequip(item *theItem) {
    char buf[COLS * 3], buf2[COLS * 3];
    unsigned char command[3];

    command[0] = UNEQUIP_KEY;
    if (theItem == NULL) {
        theItem = promptForItemOfType(ALL_ITEMS, ITEM_EQUIPPED, 0,
                                      KEYBOARD_LABELS ? "Remove (unequip) what? (a-z or <esc> to cancel)" : "Remove (unequip) what?",
                                      true);
    }
    if (theItem == NULL) {
        return;
    }

    command[1] = theItem->inventoryLetter;
    command[2] = '\0';

    if (!(theItem->flags & ITEM_EQUIPPED)) {
        itemName(theItem, buf2, false, false, NULL);
        sprintf(buf, "your %s %s not equipped.",
                buf2,
                theItem->quantity == 1 ? "was" : "were");
        confirmMessages();
        messageWithColor(buf, &itemMessageColor, false);
        return;
    } else if (theItem->flags & ITEM_CURSED) { // this is where the item gets unequipped
        itemName(theItem, buf2, false, false, NULL);
        sprintf(buf, "you can't; your %s appear%s to be cursed.",
                buf2,
                theItem->quantity == 1 ? "s" : "");
        confirmMessages();
        messageWithColor(buf, &itemMessageColor, false);
        return;
    } else {
        recordKeystrokeSequence(command);
        unequipItem(theItem, false);
        if (theItem->category & RING) {
            updateRingBonuses();
        }
        itemName(theItem, buf2, true, true, NULL);
        if (strLenWithoutEscapes(buf2) > 52) {
            itemName(theItem, buf2, false, true, NULL);
        }
        confirmMessages();
        updateEncumbrance();
        sprintf(buf, "you are no longer %s %s.", (theItem->category & WEAPON ? "wielding" : "wearing"), buf2);
        messageWithColor(buf, &itemMessageColor, false);
    }
    playerTurnEnded();
}

boolean canDrop() {
    if (cellHasTerrainFlag(player.xLoc, player.yLoc, T_OBSTRUCTS_ITEMS)) {
        return false;
    }
    return true;
}

void drop(item *theItem) {
    char buf[COLS * 3], buf2[COLS * 3];
    unsigned char command[3];

    command[0] = DROP_KEY;
    if (theItem == NULL) {
        theItem = promptForItemOfType(ALL_ITEMS, 0, 0,
                                      KEYBOARD_LABELS ? "Drop what? (a-z, shift for more info; or <esc> to cancel)" : "Drop what?",
                                      true);
    }
    if (theItem == NULL) {
        return;
    }
    command[1] = theItem->inventoryLetter;
    command[2] = '\0';

    if ((theItem->flags & ITEM_EQUIPPED) && (theItem->flags & ITEM_CURSED)) {
        itemName(theItem, buf2, false, false, NULL);
        sprintf(buf, "you can't; your %s appears to be cursed.", buf2);
        confirmMessages();
        messageWithColor(buf, &itemMessageColor, false);
    } else if (canDrop()) {
        recordKeystrokeSequence(command);
        if (theItem->flags & ITEM_EQUIPPED) {
            unequipItem(theItem, false);
        }
        theItem = dropItem(theItem); // This is where it gets dropped.
        theItem->flags |= ITEM_PLAYER_AVOIDS; // Try not to pick up stuff you've already dropped.
        itemName(theItem, buf2, true, true, NULL);
        sprintf(buf, "You dropped %s.", buf2);
        messageWithColor(buf, &itemMessageColor, false);
        playerTurnEnded();
    } else {
        confirmMessages();
        message("There is already something there.", false);
    }
}

item *promptForItemOfType(unsigned short category,
                          unsigned long requiredFlags,
                          unsigned long forbiddenFlags,
                          char *prompt,
                          boolean allowInventoryActions) {
    char keystroke;
    item *theItem;

    if (!numberOfMatchingPackItems(ALL_ITEMS, requiredFlags, forbiddenFlags, true)) {
        return NULL;
    }

    temporaryMessage(prompt, false);

    keystroke = displayInventory(category, requiredFlags, forbiddenFlags, false, allowInventoryActions);

    if (!keystroke) {
        // This can happen if the player does an action with an item directly from the inventory screen via a button.
        return NULL;
    }

    if (keystroke < 'a' || keystroke > 'z') {
        confirmMessages();
        if (keystroke != ESCAPE_KEY && keystroke != ACKNOWLEDGE_KEY) {
            message("Invalid entry.", false);
        }
        return NULL;
    }

    theItem = itemOfPackLetter(keystroke);
    if (theItem == NULL) {
        confirmMessages();
        message("No such item.", false);
        return NULL;
    }

    return theItem;
}

item *itemOfPackLetter(char letter) {
    item *theItem;
    for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (theItem->inventoryLetter == letter) {
            return theItem;
        }
    }
    return NULL;
}

item *itemAtLoc(short x, short y) {
    item *theItem;

    if (!(pmap[x][y].flags & HAS_ITEM)) {
        return NULL; // easy optimization
    }
    for (theItem = floorItems->nextItem; theItem != NULL && (theItem->xLoc != x || theItem->yLoc != y); theItem = theItem->nextItem);
    if (theItem == NULL) {
        pmap[x][y].flags &= ~HAS_ITEM;
        hiliteCell(x, y, &white, 75, true);
        rogue.automationActive = false;
        message("ERROR: An item was supposed to be here, but I couldn't find it.", true);
        refreshDungeonCell(x, y);
    }
    return theItem;
}

item *dropItem(item *theItem) {
    item *itemFromTopOfStack, *itemOnFloor;

    if (cellHasTerrainFlag(player.xLoc, player.yLoc, T_OBSTRUCTS_ITEMS)) {
        return NULL;
    }

    itemOnFloor = itemAtLoc(player.xLoc, player.yLoc);

    if (theItem->quantity > 1 && !(theItem->category & (WEAPON | GEM))) { // peel off the top item and drop it
        itemFromTopOfStack = generateItem(ALL_ITEMS, -1);
        *itemFromTopOfStack = *theItem; // clone the item
        theItem->quantity--;
        itemFromTopOfStack->quantity = 1;
        if (itemOnFloor) {
            itemOnFloor->inventoryLetter = theItem->inventoryLetter; // just in case all letters are taken
            pickUpItemAt(player.xLoc, player.yLoc);
        }
        placeItem(itemFromTopOfStack, player.xLoc, player.yLoc);
        return itemFromTopOfStack;
    } else { // drop the entire item
        removeItemFromChain(theItem, packItems);
        if (itemOnFloor) {
            itemOnFloor->inventoryLetter = theItem->inventoryLetter;
            pickUpItemAt(player.xLoc, player.yLoc);
        }
        placeItem(theItem, player.xLoc, player.yLoc);
        return theItem;
    }
}

void recalculateEquipmentBonuses() {
    fixpt enchant;
    item *theItem;
    if (rogue.weapon) {
        theItem = rogue.weapon;
        enchant = netEnchant(theItem);
        player.info.damage = theItem->damage;
        player.info.damage.lowerBound = player.info.damage.lowerBound * damageFraction(enchant) / FP_FACTOR;
        player.info.damage.upperBound = player.info.damage.upperBound * damageFraction(enchant) / FP_FACTOR;
        if (player.info.damage.lowerBound < 1) {
            player.info.damage.lowerBound = 1;
        }
        if (player.info.damage.upperBound < 1) {
            player.info.damage.upperBound = 1;
        }
    }

    if (rogue.armor) {
        theItem = rogue.armor;
        enchant = netEnchant(theItem);
        enchant -= player.status[STATUS_DONNING] * FP_FACTOR;
        player.info.defense = (theItem->armor * FP_FACTOR + enchant * 10) / FP_FACTOR;
        if (player.info.defense < 0) {
            player.info.defense = 0;
        }
    }
}

void equipItem(item *theItem, boolean force) {
    item *previouslyEquippedItem = NULL;

    if ((theItem->category & RING) && (theItem->flags & ITEM_EQUIPPED)) {
        return;
    }

    if (theItem->category & WEAPON) {
        previouslyEquippedItem = rogue.weapon;
    } else if (theItem->category & ARMOR) {
        previouslyEquippedItem = rogue.armor;
    }
    if (previouslyEquippedItem) {
        if (!force && (previouslyEquippedItem->flags & ITEM_CURSED)) {
            return; // already using a cursed item
        } else {
            unequipItem(previouslyEquippedItem, force);
        }
    }
    if (theItem->category & WEAPON) {
        rogue.weapon = theItem;
        recalculateEquipmentBonuses();
    } else if (theItem->category & ARMOR) {
        if (!force) {
            player.status[STATUS_DONNING] = player.maxStatus[STATUS_DONNING] = theItem->armor / 10;
        }
        rogue.armor = theItem;
        recalculateEquipmentBonuses();
    } else if (theItem->category & RING) {
        if (rogue.ringLeft && rogue.ringRight) {
            return;
        }
        if (rogue.ringLeft) {
            rogue.ringRight = theItem;
        } else {
            rogue.ringLeft = theItem;
        }
        updateRingBonuses();
        if (theItem->kind == RING_CLAIRVOYANCE) {
            updateClairvoyance();
            displayLevel();
            identifyItemKind(theItem);
        } else if (theItem->kind == RING_LIGHT
                   || theItem->kind == RING_STEALTH) {
            identifyItemKind(theItem);
        }
    }
    theItem->flags |= ITEM_EQUIPPED;
    return;
}

void unequipItem(item *theItem, boolean force) {

    if (theItem == NULL || !(theItem->flags & ITEM_EQUIPPED)) {
        return;
    }
    if ((theItem->flags & ITEM_CURSED) && !force) {
        return;
    }
    theItem->flags &= ~ITEM_EQUIPPED;
    if (theItem->category & WEAPON) {
        player.info.damage.lowerBound = 1;
        player.info.damage.upperBound = 2;
        player.info.damage.clumpFactor = 1;
        rogue.weapon = NULL;
    }
    if (theItem->category & ARMOR) {
        player.info.defense = 0;
        rogue.armor = NULL;
        player.status[STATUS_DONNING] = 0;
    }
    if (theItem->category & RING) {
        if (rogue.ringLeft == theItem) {
            rogue.ringLeft = NULL;
        } else if (rogue.ringRight == theItem) {
            rogue.ringRight = NULL;
        }
        updateRingBonuses();
        if (theItem->kind == RING_CLAIRVOYANCE) {
            updateClairvoyance();
            updateFieldOfViewDisplay(false, false);
            updateClairvoyance(); // Yes, we have to call this a second time.
            displayLevel();
        }
    }
    updateEncumbrance();
    return;
}

void updateRingBonuses() {
    short i;
    item *rings[2] = {rogue.ringLeft, rogue.ringRight};

    rogue.clairvoyance = rogue.stealthBonus = rogue.transference
    = rogue.awarenessBonus = rogue.regenerationBonus = rogue.wisdomBonus = rogue.reaping = 0;
    rogue.lightMultiplier = 1;

    for (i=0; i<= 1; i++) {
        if (rings[i]) {
            switch (rings[i]->kind) {
                case RING_CLAIRVOYANCE:
                    rogue.clairvoyance += effectiveRingEnchant(rings[i]);
                    break;
                case RING_STEALTH:
                    rogue.stealthBonus += effectiveRingEnchant(rings[i]);
                    break;
                case RING_REGENERATION:
                    rogue.regenerationBonus += effectiveRingEnchant(rings[i]);
                    break;
                case RING_TRANSFERENCE:
                    rogue.transference += effectiveRingEnchant(rings[i]);
                    break;
                case RING_LIGHT:
                    rogue.lightMultiplier += effectiveRingEnchant(rings[i]);
                    break;
                case RING_AWARENESS:
                    rogue.awarenessBonus += 20 * effectiveRingEnchant(rings[i]);
                    break;
                case RING_WISDOM:
                    rogue.wisdomBonus += effectiveRingEnchant(rings[i]);
                    break;
                case RING_REAPING:
                    rogue.reaping += effectiveRingEnchant(rings[i]);
                    break;
            }
        }
    }

    if (rogue.lightMultiplier <= 0) {
        rogue.lightMultiplier--; // because it starts at positive 1 instead of 0
    }

    updateMinersLightRadius();
    updatePlayerRegenerationDelay();

    if (rogue.stealthBonus < 0) {
        rogue.stealthBonus *= 4;
    }
}

void updatePlayerRegenerationDelay() {
    short maxHP;
    long turnsForFull; // In thousandths of a turn.
    maxHP = player.info.maxHP;
    turnsForFull = turnsForFullRegenInThousandths(rogue.regenerationBonus * FP_FACTOR);

    player.regenPerTurn = 0;
    while (maxHP > turnsForFull / 1000) {
        player.regenPerTurn++;
        maxHP -= turnsForFull / 1000;
    }

    player.info.turnsBetweenRegen = (turnsForFull / maxHP);
    // DEBUG printf("\nTurnsForFull: %i; regenPerTurn: %i; (thousandths of) turnsBetweenRegen: %i", turnsForFull, player.regenPerTurn, player.info.turnsBetweenRegen);
}

boolean removeItemFromChain(item *theItem, item *theChain) {
    item *previousItem;

    for (previousItem = theChain;
         previousItem->nextItem;
         previousItem = previousItem->nextItem) {
        if (previousItem->nextItem == theItem) {
            previousItem->nextItem = theItem->nextItem;
            return true;
        }
    }
    return false;
}

void addItemToChain(item *theItem, item *theChain) {
    theItem->nextItem = theChain->nextItem;
    theChain->nextItem = theItem;
}

void deleteItem(item *theItem) {
    free(theItem);
}

void resetItemTableEntry(itemTable *theEntry) {
    theEntry->identified = false;
    theEntry->called = false;
    theEntry->callTitle[0] = '\0';
}

void shuffleFlavors() {
    short i, j, randIndex, randNumber;
    char buf[COLS];

    for (i=0; i<NUMBER_POTION_KINDS; i++) {
        resetItemTableEntry(potionTable + i);
    }
    for (i=0; i<NUMBER_STAFF_KINDS; i++) {
        resetItemTableEntry(staffTable+ i);
    }
    for (i=0; i<NUMBER_WAND_KINDS; i++) {
        resetItemTableEntry(wandTable + i);
    }
    for (i=0; i<NUMBER_SCROLL_KINDS; i++) {
        resetItemTableEntry(scrollTable + i);
    }
    for (i=0; i<NUMBER_RING_KINDS; i++) {
        resetItemTableEntry(ringTable + i);
    }

    for (i=0; i<NUMBER_ITEM_COLORS; i++) {
        strcpy(itemColors[i], itemColorsRef[i]);
    }
    for (i=0; i<NUMBER_ITEM_COLORS; i++) {
        randIndex = rand_range(0, NUMBER_ITEM_COLORS - 1);
        if (randIndex != i) {
            strcpy(buf, itemColors[i]);
            strcpy(itemColors[i], itemColors[randIndex]);
            strcpy(itemColors[randIndex], buf);
        }
    }

    for (i=0; i<NUMBER_ITEM_WOODS; i++) {
        strcpy(itemWoods[i], itemWoodsRef[i]);
    }
    for (i=0; i<NUMBER_ITEM_WOODS; i++) {
        randIndex = rand_range(0, NUMBER_ITEM_WOODS - 1);
        if (randIndex != i) {
            strcpy(buf, itemWoods[i]);
            strcpy(itemWoods[i], itemWoods[randIndex]);
            strcpy(itemWoods[randIndex], buf);
        }
    }

    for (i=0; i<NUMBER_ITEM_GEMS; i++) {
        strcpy(itemGems[i], itemGemsRef[i]);
    }
    for (i=0; i<NUMBER_ITEM_GEMS; i++) {
        randIndex = rand_range(0, NUMBER_ITEM_GEMS - 1);
        if (randIndex != i) {
            strcpy(buf, itemGems[i]);
            strcpy(itemGems[i], itemGems[randIndex]);
            strcpy(itemGems[randIndex], buf);
        }
    }

    for (i=0; i<NUMBER_ITEM_METALS; i++) {
        strcpy(itemMetals[i], itemMetalsRef[i]);
    }
    for (i=0; i<NUMBER_ITEM_METALS; i++) {
        randIndex = rand_range(0, NUMBER_ITEM_METALS - 1);
        if (randIndex != i) {
            strcpy(buf, itemMetals[i]);
            strcpy(itemMetals[i], itemMetals[randIndex]);
            strcpy(itemMetals[randIndex], buf);
        }
    }

    for (i=0; i<NUMBER_SCROLL_KINDS; i++) {
        itemTitles[i][0] = '\0';
        randNumber = rand_range(3, 4);
        for (j=0; j<randNumber; j++) {
            randIndex = rand_range(0, NUMBER_TITLE_PHONEMES - 1);
            strcpy(buf, itemTitles[i]);
            sprintf(itemTitles[i], "%s%s%s", buf, ((rand_percent(50) && j>0) ? " " : ""), titlePhonemes[randIndex]);
        }
    }
}

unsigned long itemValue(item *theItem) {
    switch (theItem->category) {
        case AMULET:
            return 35000;
            break;
        case GEM:
            return 5000 * theItem->quantity;
            break;
        default:
            return 0;
            break;
    }
}
