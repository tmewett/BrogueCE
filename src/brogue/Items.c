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
#include "GlobalsBase.h"
#include "Globals.h"

#define MAGIC_POLARITY_BENEVOLENT 1
#define MAGIC_POLARITY_MALEVOLENT -1
#define MAGIC_POLARITY_NEUTRAL 0
#define MAGIC_POLARITY_ANY 0

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
    theItem->lastUsed[0] = 0;
    theItem->lastUsed[1] = 0;
    theItem->lastUsed[2] = 0;
    theItem->nextItem = NULL;

    for (i=0; i < KEY_ID_MAXIMUM; i++) {
        theItem->keyLoc[i].loc = (pos){ 0, 0 };
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

static unsigned long pickItemCategory(unsigned long theCategory) {
    short i, sum, randIndex;
    unsigned short correspondingCategories[13] =    {GOLD,  SCROLL, POTION, STAFF,  WAND,   WEAPON, ARMOR,  FOOD,   RING,   CHARM,    AMULET,   GEM,    KEY};

    sum = 0;

    for (i=0; i<13; i++) {
        if (theCategory <= 0 || theCategory & correspondingCategories[i]) {
            sum += itemGenerationProbabilities[i];
        }
    }

    if (sum == 0) {
        return theCategory; // e.g. when you pass in AMULET or GEM, since they have no frequency
    }

    randIndex = rand_range(1, sum);

    for (i=0; ; i++) {
        if (theCategory <= 0 || theCategory & correspondingCategories[i]) {
            if (randIndex <= itemGenerationProbabilities[i]) {
                return correspondingCategories[i];
            }
            randIndex -= itemGenerationProbabilities[i];
        }
    }
}

/// @brief Pick a random item category for hallucination
/// @return the category
enum itemCategory getHallucinatedItemCategory(void) {
    const enum itemCategory itemCategories[10] = {FOOD, WEAPON, ARMOR, POTION, SCROLL, STAFF, WAND, RING, CHARM, GOLD};
    return itemCategories[rand_range(0, 9)];
}

/// @brief Gets the glyph used to represent an item of the given category
/// @param theCategory The item category
/// @return The glyph
enum displayGlyph getItemCategoryGlyph(const enum itemCategory theCategory) {

    switch (theCategory) {
        case FOOD:
            return G_FOOD;
        case WEAPON:
            return G_WEAPON;
        case ARMOR:
            return G_ARMOR;
        case SCROLL:
            return G_SCROLL;
        case POTION:
            return G_POTION;
        case STAFF:
            return G_STAFF;
        case WAND:
            return G_WAND;
        case GEM:
            return G_GEM;
        case RING:
            return G_RING;
        case CHARM:
            return G_CHARM;
        case KEY:
            return G_KEY;
        case GOLD:
            return G_GOLD;
        case AMULET:
            return G_AMULET;
        default:
            break;
    }
    return 0;
}

/// @brief Checks if an item is a throwing weapon
/// @param theItem the item
/// @return true if the item is a throwing weapon
static boolean itemIsThrowingWeapon(const item *theItem) {
    if (theItem && (theItem->category == WEAPON)
        && ((theItem->kind == DART) || (theItem->kind == JAVELIN) || (theItem->kind == INCENDIARY_DART))) {

        return true;
    }
    return false;
}

// Sets an item to the given type and category (or chooses randomly if -1) with all other stats
item *makeItemInto(item *theItem, unsigned long itemCategory, short itemKind) {
    const itemTable *theEntry = NULL;

    if (itemCategory <= 0) {
        itemCategory = ALL_ITEMS;
    }

    itemCategory = pickItemCategory(itemCategory);

    theItem->category = itemCategory;
    theItem->displayChar = getItemCategoryGlyph(theItem->category);

    switch (itemCategory) {

        case FOOD:
            if (itemKind < 0) {
                itemKind = chooseKind(foodTable, NUMBER_FOOD_KINDS);
            }
            theEntry = &foodTable[itemKind];
            theItem->flags |= ITEM_IDENTIFIED;
            break;

        case WEAPON:
            if (itemKind < 0) {
                itemKind = chooseKind(weaponTable, NUMBER_WEAPON_KINDS);
            }
            theEntry = &weaponTable[itemKind];
            theItem->damage = weaponTable[itemKind].range;
            theItem->strengthRequired = weaponTable[itemKind].strengthRequired;

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
            theItem->charges = gameConst->weaponKillsToAutoID; // kill 20 enemies to auto-identify
            break;

        case ARMOR:
            if (itemKind < 0) {
                itemKind = chooseKind(armorTable, NUMBER_ARMOR_KINDS);
            }
            theEntry = &armorTable[itemKind];
            theItem->armor = randClump(armorTable[itemKind].range);
            theItem->strengthRequired = armorTable[itemKind].strengthRequired;
            theItem->charges = gameConst->armorDelayToAutoID; // this many turns until it reveals its enchants and whether runic
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
                itemKind = chooseKind(scrollTable, gameConst->numberScrollKinds);
            }
            theEntry = &scrollTable[itemKind];
            theItem->flags |= ITEM_FLAMMABLE;
            break;
        case POTION:
            if (itemKind < 0) {
                itemKind = chooseKind(potionTable, gameConst->numberPotionKinds);
            }
            theEntry = &potionTable[itemKind];
            break;
        case STAFF:
            if (itemKind < 0) {
                itemKind = chooseKind(staffTable, NUMBER_STAFF_KINDS);
            }
            theEntry = &staffTable[itemKind];
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
                itemKind = chooseKind(wandTable, gameConst->numberWandKinds);
            }
            theEntry = &(wandTable[itemKind]);
            theItem->charges = randClump(wandTable[itemKind].range);
            break;
        case RING:
            if (itemKind < 0) {
                itemKind = chooseKind(ringTable, NUMBER_RING_KINDS);
            }
            theEntry = &ringTable[itemKind];
            theItem->enchant1 = randClump(ringTable[itemKind].range);
            theItem->charges = gameConst->ringDelayToAutoID; // how many turns of being worn until it auto-identifies
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
                itemKind = chooseKind(charmTable, gameConst->numberCharmKinds);
            }
            theItem->charges = 0; // Charms are initially ready for use.
            theItem->enchant1 = randClump(charmTable[itemKind].range);
            while (rand_percent(7)) {
                theItem->enchant1++;
            }
            theItem->flags |= ITEM_IDENTIFIED;
            break;
        case GOLD:
            theEntry = NULL;
            theItem->quantity = rand_range(50 + rogue.depthLevel * 10 * gameConst->depthAccelerator, 100 + rogue.depthLevel * 15 * gameConst->depthAccelerator);
            break;
        case AMULET:
            theEntry = NULL;
            itemKind = 0;
            theItem->flags |= ITEM_IDENTIFIED;
            break;
        case GEM:
            theEntry = NULL;
            itemKind = 0;
            theItem->flags |= ITEM_IDENTIFIED;
            break;
        case KEY:
            theEntry = NULL;
            theItem->flags |= ITEM_IDENTIFIED;
            break;
        default:
            theEntry = NULL;
            message("something has gone terribly wrong!", REQUIRE_ACKNOWLEDGMENT);
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

short chooseKind(const itemTable *theTable, short numKinds) {
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
item *placeItemAt(item *theItem, pos dest) {
    enum dungeonLayers layer;
    char theItemName[DCOLS], buf[DCOLS];

    // If no valid position is supplied by the caller, choose a random one instead.
    if (!isPosInMap(dest)) {
        randomMatchingLocation(&dest, FLOOR, NOTHING, -1);
    }

    theItem->loc = dest;

    removeItemFromChain(theItem, floorItems); // just in case; double-placing an item will result in game-crashing loops in the item list
    addItemToChain(theItem, floorItems);
    pmapAt(theItem->loc)->flags |= HAS_ITEM;
    if ((theItem->flags & ITEM_MAGIC_DETECTED) && itemMagicPolarity(theItem)) {
        pmapAt(theItem->loc)->flags |= ITEM_DETECTED;
    }
    if (cellHasTerrainFlag(dest, T_IS_DF_TRAP)
        && !cellHasTerrainFlag(dest, T_MOVES_ITEMS)
        && !(pmapAt(dest)->flags & PRESSURE_PLATE_DEPRESSED)) {

        pmapAt(dest)->flags |= PRESSURE_PLATE_DEPRESSED;
        if (playerCanSee(dest.x, dest.y)) {
            if (cellHasTMFlag(dest, TM_IS_SECRET)) {
                discover(dest.x, dest.y);
                refreshDungeonCell(dest);
            }
            itemName(theItem, theItemName, false, false, NULL);
            sprintf(buf, "a pressure plate clicks underneath the %s!", theItemName);
            message(buf, REQUIRE_ACKNOWLEDGMENT);
        }
        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmapAt(dest)->layers[layer]].flags & T_IS_DF_TRAP) {
                spawnDungeonFeature(dest.x, dest.y, &(dungeonFeatureCatalog[tileCatalog[pmapAt(dest)->layers[layer]].fireType]), true, false);
                promoteTile(dest.x, dest.y, layer, false);
            }
        }
    }
    return theItem;
}

static void fillItemSpawnHeatMap(unsigned short heatMap[DCOLS][DROWS], unsigned short heatLevel, pos loc) {
    if (pmapAt(loc)->layers[DUNGEON] == DOOR) {
        heatLevel += 10;
    } else if (pmapAt(loc)->layers[DUNGEON] == SECRET_DOOR) {
        heatLevel += 3000;
    }
    if (heatMap[loc.x][loc.y] > heatLevel) {
        heatMap[loc.x][loc.y] = heatLevel;
    }
    for (enum directions dir = 0; dir < 4; dir++) {
        pos neighbor = posNeighborInDirection(loc, dir);
        if (isPosInMap(neighbor)
            && !cellHasTerrainFlag(neighbor, T_IS_DEEP_WATER | T_LAVA_INSTA_DEATH | T_AUTO_DESCENT)
            && isPassableOrSecretDoor(neighbor)
            && heatLevel < heatMap[neighbor.x][neighbor.y]) {

            fillItemSpawnHeatMap(heatMap, heatLevel, neighbor);
        }
    }
}

static void coolHeatMapAt(unsigned short heatMap[DCOLS][DROWS], pos loc, unsigned long *totalHeat) {
    short k, l;
    unsigned short currentHeat;

    currentHeat = heatMap[loc.x][loc.y];
    if (currentHeat == 0) {
        return;
    }
    *totalHeat -= heatMap[loc.x][loc.y];
    heatMap[loc.x][loc.y] = 0;

    // lower the heat near the chosen location
    for (k = -5; k <= 5; k++) {
        for (l = -5; l <= 5; l++) {
            if (coordinatesAreInMap(loc.x+k, loc.y+l) && heatMap[loc.x+k][loc.y+l] == currentHeat) {
                heatMap[loc.x+k][loc.y+l] = max(1, heatMap[loc.x+k][loc.y+l]/10);
                *totalHeat -= (currentHeat - heatMap[loc.x+k][loc.y+l]);
            }
        }
    }
}

// Returns false if no place could be found.
// That should happen only if the total heat is zero.
static boolean getItemSpawnLoc(unsigned short heatMap[DCOLS][DROWS], short *x, short *y, unsigned long *totalHeat) {
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
void populateItems(pos upstairs) {
    if (!ITEMS_ENABLED) {
        return;
    }
    unsigned short itemSpawnHeatMap[DCOLS][DROWS];
    short numberOfItems, numberOfGoldPiles, goldBonusProbability;
    unsigned long totalHeat;

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

    // Store copy of potion and scroll tables, since they are modified during level item generation.
    itemTable *potionTableCopy = calloc(gameConst->numberPotionKinds, sizeof(itemTable));
    itemTable *scrollTableCopy = calloc(gameConst->numberScrollKinds, sizeof(itemTable));

    memcpy(potionTableCopy, potionTable, gameConst->numberPotionKinds * sizeof(itemTable));
    memcpy(scrollTableCopy, scrollTable, gameConst->numberScrollKinds * sizeof(itemTable));

    if (rogue.depthLevel > gameConst->amuletLevel) {
        numberOfItems = lumenstoneDistribution[rogue.depthLevel - gameConst->amuletLevel - 1];
        numberOfGoldPiles = 0;
    } else {
        // Add frequency to metered items memory
        for (int i = 0; i < gameConst->numberMeteredItems; i++) {
            rogue.meteredItems[i].frequency += meteredItemsGenerationTable[i].incrementFrequency;
        }
        numberOfItems = 3;
        while (rand_percent(60)) {
            numberOfItems++;
        }
        if (rogue.depthLevel <= 2) {
            numberOfItems += 2; // 4 extra items to kickstart your career as a rogue
        } else if (rogue.depthLevel <= 4) {
            numberOfItems++; // and 2 more here
        }

        numberOfItems += gameConst->extraItemsPerLevel;

        numberOfGoldPiles = min(5, rogue.depthLevel * gameConst->depthAccelerator / 4);
        for (goldBonusProbability = 60;
             rand_percent(goldBonusProbability) && numberOfGoldPiles <= 10;
             goldBonusProbability -= 15) {

            numberOfGoldPiles++;
        }
        // Adjust the amount of gold if we're past depth 5 and we were below or above
        // the production schedule as of the previous depth.
        if (rogue.depthLevel >= gameConst->goldAdjustmentStartDepth) {
            if (rogue.goldGenerated < aggregateGoldLowerBound(rogue.depthLevel * gameConst->depthAccelerator - 1)) {
                numberOfGoldPiles += 2;
            } else if (rogue.goldGenerated > aggregateGoldUpperBound(rogue.depthLevel * gameConst->depthAccelerator - 1)) {
                numberOfGoldPiles -= 2;
            }
        }
    }

    // Create an item spawn heat map to bias item generation behind secret doors (and, to a lesser
    // extent, regular doors). This is in terms of the number of secret/regular doors that must be
    // passed to reach the area when pathing to it from the upward staircase.
    // This is why there are often several items in well hidden secret rooms. Otherwise,
    // those rooms are usually empty, which is demoralizing after you take the trouble to find them.
    for (int i=0; i<DCOLS; i++) {
        for (int j=0; j<DROWS; j++) {
            itemSpawnHeatMap[i][j] = 50000;
        }
    }
    fillItemSpawnHeatMap(itemSpawnHeatMap, 5, upstairs);
    totalHeat = 0;

#ifdef AUDIT_RNG
    sprintf(RNGmessage, "\n\nInitial heat map for level %i:\n", rogue.currentTurnNumber);
    RNGLog(RNGmessage);
#endif

    for (int j=0; j<DROWS; j++) {
        for (int i=0; i<DCOLS; i++) {
            if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_ITEMS | T_PATHING_BLOCKER)
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
        for (int i=0; i<DCOLS; i++) {
            for (int j=0; j<DROWS; j++) {
                map[i][j] = itemSpawnHeatMap[i][j] * -1;
            }
        }
        dumpLevelToScreen();
        displayGrid(map);
        freeGrid(map);
        temporaryMessage("Item spawn heat map:", REQUIRE_ACKNOWLEDGMENT);
    }

    int randomDepthOffset = 0;
    if (rogue.depthLevel > 2) {
        // Include a random factor in food and potion of life generation to make things slightly less predictable.
        randomDepthOffset = rand_range(-1, 1);
        randomDepthOffset += rand_range(-1, 1);
    }

    for (int i=0; i<numberOfItems; i++) {
        short theCategory = ALL_ITEMS & ~GOLD; // gold is placed separately, below, so it's not a punishment
        short theKind = -1;

        // Set metered item itemTable frequency to memory.
        for (int j = 0; j < gameConst->numberMeteredItems; j++) {
            if (meteredItemsGenerationTable[j].incrementFrequency != 0) {
                if (j >= gameConst->numberScrollKinds) {
                    potionTable[j - gameConst->numberScrollKinds].frequency = rogue.meteredItems[j].frequency;
                } else {
                    scrollTable[j].frequency = rogue.meteredItems[j].frequency;
                }
            }
        }

        // Adjust the desired item category if necessary.
        if ((rogue.foodSpawned + foodTable[RATION].power / 3) * 4 * FP_FACTOR
            <= (POW_FOOD[rogue.depthLevel-1] + (randomDepthOffset * FP_FACTOR)) * foodTable[RATION].power * 45/100) {
            // Guarantee a certain nutrition minimum of the approximate equivalent of one ration every four levels,
            // with more food on deeper levels since they generally take more turns to complete.
            theCategory = FOOD;
            if (rogue.depthLevel > gameConst->amuletLevel) {
                numberOfItems++; // Food isn't at the expense of lumenstones.
            }
        } else if (rogue.depthLevel > gameConst->amuletLevel) {
            theCategory = GEM;
        } else {

            for (int j = 0; j < gameConst->numberMeteredItems; j++) {
                // Create any metered items that reach generation thresholds
                if (meteredItemsGenerationTable[j].levelScaling != 0 &&
                    rogue.meteredItems[j].numberSpawned * meteredItemsGenerationTable[j].genMultiplier + meteredItemsGenerationTable[j].genIncrement <
                    rogue.depthLevel * meteredItemsGenerationTable[j].levelScaling + randomDepthOffset) {
                        theCategory = meteredItemsGenerationTable[j].category;
                        theKind = meteredItemsGenerationTable[j].kind;
                        break;
                }
                // Create any metered items that reach hard by-level guarantees
                if (rogue.depthLevel == meteredItemsGenerationTable[j].levelGuarantee &&
                    rogue.meteredItems[j].numberSpawned < meteredItemsGenerationTable[j].itemNumberGuarantee) {
                        theCategory = meteredItemsGenerationTable[j].category;
                        theKind = meteredItemsGenerationTable[j].kind;
                        break;
                    }
            }
        }

        // Generate the item.
        item *theItem = generateItem(theCategory, theKind);
        theItem->originDepth = rogue.depthLevel;

        if (theItem->category & FOOD) {
            rogue.foodSpawned += foodTable[theItem->kind].power;
            if (D_MESSAGE_ITEM_GENERATION) printf("\n(:)  Depth %i: generated food", rogue.depthLevel);
        }

        // Choose a placement location.
        pos itemPlacementLoc = INVALID_POS;
        if ((theItem->category & FOOD) || ((theItem->category & POTION) && theItem->kind == POTION_STRENGTH)) {
            do {
                randomMatchingLocation(&itemPlacementLoc, FLOOR, NOTHING, -1); // Food and gain strength don't follow the heat map.
            } while (passableArcCount(itemPlacementLoc.x, itemPlacementLoc.y) > 1); // Not in a hallway.
        } else {
            getItemSpawnLoc(itemSpawnHeatMap, &itemPlacementLoc.x, &itemPlacementLoc.y, &totalHeat);
        }
        brogueAssert(isPosInMap(itemPlacementLoc));
        // Cool off the item spawning heat map at the chosen location:
        coolHeatMapAt(itemSpawnHeatMap, itemPlacementLoc, &totalHeat);

        // Remove frequency from spawned metered items memory.
        for (int j = 0; j < gameConst->numberMeteredItems; j++) {
            if ((theItem->category & meteredItemsGenerationTable[j].category) && theItem->kind == meteredItemsGenerationTable[j].kind) {
                if (j >= gameConst->numberScrollKinds) {
                    if (D_MESSAGE_ITEM_GENERATION) printf("\n(!)  Depth %i: generated an %s potion at %i frequency", rogue.depthLevel, potionTable[j - gameConst->numberScrollKinds].name, rogue.meteredItems[j].frequency);
                } else {
                    if (D_MESSAGE_ITEM_GENERATION) printf("\n(?)  Depth %i: generated an %s scroll at %i frequency", rogue.depthLevel, scrollTable[j].name, rogue.meteredItems[j].frequency);
                }
                rogue.meteredItems[j].frequency -= meteredItemsGenerationTable[j].decrementFrequency;
                rogue.meteredItems[j].numberSpawned++;
            }
        }

        // Place the item.
        placeItemAt(theItem, itemPlacementLoc); // Random valid location already obtained according to heat map.
        brogueAssert(!cellHasTerrainFlag(itemPlacementLoc, T_OBSTRUCTS_PASSABILITY));

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
            plotCharWithColor(theItem->displayChar, mapToWindow(itemPlacementLoc), &black, &purple);
            temporaryMessage("Added an item.", REQUIRE_ACKNOWLEDGMENT);
        }
    }

    // Now generate gold.
    for (int i=0; i<numberOfGoldPiles; i++) {
        item *theItem = generateItem(GOLD, -1);
        pos itemPlacementLoc = INVALID_POS;
        getItemSpawnLoc(itemSpawnHeatMap, &itemPlacementLoc.x, &itemPlacementLoc.y, &totalHeat);
        coolHeatMapAt(itemSpawnHeatMap, itemPlacementLoc, &totalHeat);
        placeItemAt(theItem, itemPlacementLoc);
        rogue.goldGenerated += theItem->quantity;
    }

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Added gold.", REQUIRE_ACKNOWLEDGMENT);
    }

    // No enchant scrolls or strength/life potions can spawn except via initial
    // item population or blueprints that create them specifically.

    // Restore potion and scroll tables which sets the frequency of these items
    // to zero.

    memcpy(potionTable, potionTableCopy, gameConst->numberPotionKinds * sizeof(itemTable));
    memcpy(scrollTable, scrollTableCopy, gameConst->numberScrollKinds * sizeof(itemTable));

    free(potionTableCopy);
    free(scrollTableCopy);

    if (D_MESSAGE_ITEM_GENERATION) printf("\n---- Depth %i: %lu gold generated so far.", rogue.depthLevel, rogue.goldGenerated);
}

// Name of this function is a bit misleading -- basically returns true iff the item will stack without consuming an extra slot
// i.e. if it's a throwing weapon with a sibling already in your pack. False for potions and scrolls.
static boolean itemWillStackWithPack(item *theItem) {
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

void removeItemAt(pos loc) {
    pmapAt(loc)->flags &= ~HAS_ITEM;

    if (cellHasTMFlag(loc, TM_PROMOTES_ON_ITEM_PICKUP)) {
        for (int layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmapAt(loc)->layers[layer]].mechFlags & TM_PROMOTES_ON_ITEM_PICKUP) {
                promoteTile(loc.x, loc.y, layer, false);
            }
        }
    }
}

// adds the item at (x,y) to the pack
void pickUpItemAt(pos loc) {
    item *theItem;
    creature *monst;
    char buf[COLS * 3], buf2[COLS * 3];
    short guardianX, guardianY;

    rogue.disturbed = true;

    // find the item
    theItem = itemAtLoc(loc);

    if (!theItem) {
        message("Error: Expected item; item not found.", REQUIRE_ACKNOWLEDGMENT);
        return;
    }

    if ((theItem->flags & ITEM_KIND_AUTO_ID)
        && tableForItemCategory(theItem->category)
        && !(tableForItemCategory(theItem->category)[theItem->kind].identified)) {

        identifyItemKind(theItem);
    }

    if ((theItem->category & WAND)
        && wandTable[theItem->kind].identified
        && wandTable[theItem->kind].range.lowerBound == wandTable[theItem->kind].range.upperBound) {

        theItem->flags |= ITEM_IDENTIFIED;
    }

    if (numberOfItemsInPack() < MAX_PACK_ITEMS || (theItem->category & GOLD) || itemWillStackWithPack(theItem)) {
        // remove from floor chain
        pmapAt(loc)->flags &= ~ITEM_DETECTED;

        if (!removeItemFromChain(theItem, floorItems)) {
            brogueAssert(false);
        }

        if (theItem->category & GOLD) {
            rogue.gold += theItem->quantity;
            rogue.featRecord[FEAT_TONE] = false;
            sprintf(buf, "you found %i pieces of gold.", theItem->quantity);
            messageWithColor(buf, &itemMessageColor, 0);
            deleteItem(theItem);
            removeItemAt(loc); // triggers tiles with T_PROMOTES_ON_ITEM_PICKUP
            return;
        }

        if ((theItem->category & AMULET) && numberOfMatchingPackItems(AMULET, 0, 0, false)) {
            message("you already have the Amulet of Yendor.", 0);
            deleteItem(theItem);
            return;
        }

        theItem = addItemToPack(theItem);

        itemName(theItem, buf2, true, true, NULL); // include suffix, article

        sprintf(buf, "you now have %s (%c).", buf2, theItem->inventoryLetter);
        messageWithColor(buf, &itemMessageColor, 0);

        removeItemAt(loc); // triggers tiles with T_PROMOTES_ON_ITEM_PICKUP

        if ((theItem->category & AMULET)
            && !(rogue.yendorWarden)) {
            // Identify the amulet guardian, or generate one if there isn't one.
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *monst = nextCreature(&it);
                if (monst->info.monsterID == MK_WARDEN_OF_YENDOR) {
                    rogue.yendorWarden = monst;
                    break;
                }
            }
            if (!rogue.yendorWarden) {
                getRandomMonsterSpawnLocation(&guardianX, &guardianY);
                monst = generateMonster(MK_WARDEN_OF_YENDOR, false, false);
                monst->loc.x = guardianX;
                monst->loc.y = guardianY;
                pmap[guardianX][guardianY].flags |= HAS_MONSTER;
                rogue.yendorWarden = monst;
            }
        }
    } else {
        theItem->flags |= ITEM_PLAYER_AVOIDS; // explore shouldn't try to pick it up more than once.
        itemName(theItem, buf2, false, true, NULL); // include article
        sprintf(buf, "Your pack is too full to pick up %s.", buf2);
        messageWithColor(buf, &badMessageColor, 0);
    }
}

static void conflateItemCharacteristics(item *newItem, item *oldItem) {

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

static void stackItems(item *newItem, item *oldItem) {
    //Increment the quantity of the old item...
    newItem->quantity += oldItem->quantity;

    // ...conflate attributes...
    conflateItemCharacteristics(newItem, oldItem);

    // ...and delete the new item.
    deleteItem(oldItem);
}

static boolean inventoryLetterAvailable(char proposedLetter) {
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
            messageWithColor(buf, &itemMessageColor, 0);
        }
    }
    if (theItem->flags & ITEM_CURSED
        && theItem->enchant1 >= 0) {

        theItem->flags &= ~ITEM_CURSED;
    }
}

static boolean itemIsSwappable(const item *theItem) {
    if ((theItem->category & CAN_BE_SWAPPED)
        && theItem->quiverNumber == 0) {

        return true;
    } else {
        return false;
    }
}

static void swapItemToEnchantLevel(item *theItem, short newEnchant, boolean enchantmentKnown) {
    short x, y, charmPercent;
    char buf1[COLS * 3], buf2[COLS * 3];

    if ((theItem->category & STAFF) && newEnchant < 2
        || (theItem->category & CHARM) && newEnchant < 1
        || (theItem->category & WAND) && newEnchant < 0) {

        itemName(theItem, buf1, false, true, NULL);
        sprintf(buf2, "%s shatter%s from the strain!",
                buf1,
                theItem->quantity == 1 ? "s" : "");
        x = theItem->loc.x;
        y = theItem->loc.y;
        removeItemFromChain(theItem, floorItems);
        pmap[x][y].flags &= ~(HAS_ITEM | ITEM_DETECTED);
        if (pmap[x][y].flags & (ANY_KIND_OF_VISIBLE | DISCOVERED | ITEM_DETECTED)) {
            refreshDungeonCell((pos){ x, y });
        }
        if (playerCanSee(x, y)) {
            messageWithColor(buf2, &itemMessageColor, 0);
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
                theItem->charges = gameConst->weaponKillsToAutoID; // kill this many enemies to auto-identify
            } else if (theItem->category & ARMOR) {
                theItem->charges = gameConst->armorDelayToAutoID; // this many turns until it reveals its enchants and whether runic
            } else if (theItem->category & RING) {
                theItem->charges = gameConst->ringDelayToAutoID; // how many turns of being worn until it auto-identifies
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

static boolean enchantLevelKnown(const item *theItem) {
    if ((theItem->category & STAFF)
        && (theItem->flags & ITEM_MAX_CHARGES_KNOWN)) {

        return true;
    } else {
        return (theItem->flags & ITEM_IDENTIFIED);
    }
}

static short effectiveEnchantLevel(const item *theItem) {
    if (theItem->category & WAND) {
        return theItem->charges;
    } else {
        return theItem->enchant1;
    }
}

static boolean swapItemEnchants(const short machineNumber) {
    item *lockedItem, *tempItem;
    short i, j, oldEnchant;
    boolean enchantmentKnown;

    lockedItem = NULL;
    for (i = 0; i < DCOLS; i++) {
        for (j = 0; j < DROWS; j++) {
            tempItem = itemAtLoc((pos){ i, j });
            if (tempItem
                && pmap[i][j].machineNumber == machineNumber
                && cellHasTMFlag((pos){ i, j }, TM_SWAP_ENCHANTS_ACTIVATION)
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
    short x, y;
    char buf[DCOLS*3], buf2[DCOLS*3];
    enum dungeonLayers layer;
    item *theItem, *nextItem;

    for (theItem=floorItems->nextItem; theItem != NULL; theItem = nextItem) {
        nextItem = theItem->nextItem;
        x = theItem->loc.x;
        y = theItem->loc.y;
        if (rogue.absoluteTurnNumber < theItem->spawnTurnNumber) {
            // we are simulating an earlier turn than when the item fell into this level... let's not touch it yet
            continue;
        }
        if (cellHasTerrainFlag((pos){ x, y }, T_AUTO_DESCENT)) {
            if (playerCanSeeOrSense(x, y)) {
                itemName(theItem, buf, false, false, NULL);
                sprintf(buf2, "The %s plunge%s out of sight!", buf, (theItem->quantity > 1 ? "" : "s"));
                messageWithColor(buf2, &itemMessageColor, 0);
            }
            if (playerCanSee(x, y)) {
                discover(x, y);
            }
            theItem->flags |= ITEM_PREPLACED;

            // Remove from item chain.
            removeItemFromChain(theItem, floorItems);

            pmap[x][y].flags &= ~(HAS_ITEM | ITEM_DETECTED);

            if (theItem->category == POTION || rogue.depthLevel == gameConst->deepestLevel) {
                // Potions don't survive the fall.
                deleteItem(theItem);
            } else {
                // Add to next level's chain.
                theItem->spawnTurnNumber = rogue.absoluteTurnNumber;
                theItem->nextItem = levels[rogue.depthLevel-1 + 1].items;
                levels[rogue.depthLevel-1 + 1].items = theItem;
            }
            refreshDungeonCell((pos){ x, y });
            continue;
        }
        if ((cellHasTerrainFlag((pos){ x, y }, T_IS_FIRE) && (theItem->flags & ITEM_FLAMMABLE))
            || (cellHasTerrainFlag((pos){ x, y }, T_LAVA_INSTA_DEATH) && !(theItem->category & AMULET))) {

            burnItem(theItem);
            continue;
        }
        if (cellHasTerrainFlag((pos){ x, y }, T_MOVES_ITEMS)) {
            pos loc;
            getQualifyingLocNear(&loc, (pos){ x, y }, true, 0, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_PASSABILITY), (HAS_ITEM), false, false);
            // To prevent items drifting through walls, they can only drift to an adjacent cell
            if (distanceBetween((pos){ x, y }, loc) == 1) {
                removeItemAt((pos){ x, y });
                pmapAt(loc)->flags |= HAS_ITEM;
                if (pmap[x][y].flags & ITEM_DETECTED) {
                    pmap[x][y].flags &= ~ITEM_DETECTED;
                    pmapAt(loc)->flags |= ITEM_DETECTED;
                }
                theItem->loc = loc;
                refreshDungeonCell((pos){ x, y });
                refreshDungeonCell(loc);
                continue;
            }
        }
        if (cellHasTMFlag((pos){ x, y }, TM_PROMOTES_ON_ITEM)) {
            for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_PROMOTES_ON_ITEM) {
                    promoteTile(x, y, layer, false);
                }
            }
            continue;
        }
        if (pmap[x][y].machineNumber
            && pmap[x][y].machineNumber == pmapAt(player.loc)->machineNumber
            && (theItem->flags & ITEM_KIND_AUTO_ID)) {

            identifyItemKind(theItem);
        }
        if (cellHasTMFlag((pos){ x, y }, TM_SWAP_ENCHANTS_ACTIVATION)
            && pmap[x][y].machineNumber) {

            while (nextItem != NULL
                   && pmap[x][y].machineNumber == pmapAt(nextItem->loc)->machineNumber
                   && cellHasTMFlag(nextItem->loc, TM_SWAP_ENCHANTS_ACTIVATION)) {

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

static boolean inscribeItem(item *theItem) {
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
        messageWithColor(buf, &itemMessageColor, 0);
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
               && !tableForItemCategory(theItem->category)[theItem->kind].identified) {
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
                && tableForItemCategory(tempItem->category)[tempItem->kind].identified) {

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
            message("you already know what that is.", 0);
        }
        return;
    }

    if (theItem->category & (WEAPON | ARMOR | STAFF | WAND | RING)) {
        if (tableForItemCategory(theItem->category)[theItem->kind].identified) {
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

    if (tableForItemCategory(theItem->category)
        && !(tableForItemCategory(theItem->category)[theItem->kind].identified)) {

        if (getInputTextString(itemText, "call them: \"", 29, "", "\"", TEXT_INPUT_NORMAL, false)) {
            command[c++] = '\0';
            strcat((char *) command, itemText);
            recordKeystrokeSequence(command);
            recordKeystroke(RETURN_KEY, false, false);
            if (itemText[0]) {
                strcpy(tableForItemCategory(theItem->category)[theItem->kind].callTitle, itemText);
                tableForItemCategory(theItem->category)[theItem->kind].called = true;
            } else {
                tableForItemCategory(theItem->category)[theItem->kind].callTitle[0] = '\0';
                tableForItemCategory(theItem->category)[theItem->kind].called = false;
            }
            confirmMessages();
            itemName(theItem, buf, false, true, NULL);
            messageWithColor(buf, &itemMessageColor, 0);
        }
    } else {
        message("you already know what that is.", 0);
    }
}

// Generates the item name and returns it in the "root" string.
// IncludeDetails governs things such as enchantment, charges, strength requirement, times used, etc.
// IncludeArticle governs the article -- e.g. "some" food, "5" darts, "a" pink potion.
// If baseColor is provided, then the suffix will be in gray, flavor portions of the item name (e.g. a "pink" potion,
//  a "sandalwood" staff, a "ruby" ring) will be in dark purple, and the Amulet of Yendor and lumenstones will be in yellow.
//  BaseColor itself will be the color that the name reverts to outside of these colored portions.
void itemName(item *theItem, char *root, boolean includeDetails, boolean includeArticle, const color *baseColor) {
    char buf[DCOLS * 5], pluralization[10], article[10] = "", runicName[30],
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
                        itemRunicName(theItem, runicName);
                        sprintf(buf, "%s of %s%s", root, runicName, grayEscapeSequence);
                        strcpy(root, buf);
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
                    itemRunicName(theItem, runicName);
                    sprintf(buf, "%s of %s%s", root, runicName, grayEscapeSequence);
                    strcpy(root, buf);
                    }

                if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
                    sprintf(buf, "%s%i %s%s [%i]<%i>",
                            (theItem->enchant1 < 0 ? "" : "+"),
                            theItem->enchant1,
                            root,
                            grayEscapeSequence,
                            theItem->armor/10 + theItem->enchant1,
                            theItem->strengthRequired);
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

void itemKindName(item *theItem, char *kindName) {

    // use lookup table for randomly generated items with more than one kind per category
    if (theItem->category & (ARMOR | CHARM | FOOD | POTION | RING | SCROLL | STAFF | WAND | WEAPON)) {
        strcpy(kindName, tableForItemCategory(theItem->category)[theItem->kind].name);
    } else {
        switch (theItem->category) {
            case KEY:
                strcpy(kindName, keyTable[theItem->kind].name); //keys are not randomly generated but have a lookup table
                break;
            // these items have only one kind per category and no lookup table
            case GOLD:
                strcpy(kindName, "gold pieces");
                break;
            case AMULET:
                strcpy(kindName, "amulet of yendor");
                break;
            case GEM:
                strcpy(kindName, "lumenstone");
                break;
            default:
                strcpy(kindName, "unknown");
                break;
        }
    }
}

void itemRunicName(item *theItem, char *runicName) {
    char vorpalEnemyMonsterClass[15] ="";

    if (theItem->flags & ITEM_RUNIC) {
        if ((theItem->category == ARMOR && theItem->enchant2 == A_IMMUNITY)
        || (theItem->category == WEAPON && theItem->enchant2 == W_SLAYING)) {
            sprintf(vorpalEnemyMonsterClass, "%s ", monsterClassCatalog[theItem->vorpalEnemy].name);
        }
        if (theItem->category == WEAPON) {
            sprintf(runicName, "%s%s", vorpalEnemyMonsterClass, weaponRunicNames[theItem->enchant2]);
        } else if (theItem->category == ARMOR) {
            sprintf(runicName, "%s%s", vorpalEnemyMonsterClass, armorRunicNames[theItem->enchant2]);
        }
    }
}

static int enchantMagnitude() {
    return tableForItemCategory(SCROLL)[SCROLL_ENCHANTING].power;
}

itemTable *tableForItemCategory(enum itemCategory theCat) {
    switch (theCat) {
        case FOOD:
            return foodTable;
        case WEAPON:
            return weaponTable;
        case ARMOR:
            return armorTable;
        case POTION:
            return potionTable;
        case SCROLL:
            return scrollTable;
        case RING:
            return ringTable;
        case WAND:
            return wandTable;
        case STAFF:
            return staffTable;
        case CHARM:
            return charmTable;
        default:
            return NULL;
    }
}

boolean isVowelish(char *theChar) {
    short i;

    while (*theChar == COLOR_ESCAPE) {
        theChar += 4;
    }
    char str[30];
    strncpy(str, theChar, 30);
    str[29] = '\0';
    for (i = 0; i < 29; i++) {
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

static fixpt enchantIncrement(item *theItem) {
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

static short effectiveRingEnchant(item *theItem) {
    if (theItem->category != RING) {
        return 0;
    }
    if (theItem->flags & ITEM_IDENTIFIED) {
        return theItem->enchant1;
    } else {
        return min(theItem->enchant1, theItem->timesEnchanted + 1);
    }
}

static short apparentRingBonus(const enum ringKind kind) {
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

/// @brief Checks if there are any acidic monsters in a monster class
/// @param classID The monster class
/// @return True if the monster class has an acidic monster
static boolean monsterClassHasAcidicMonster(const short classID) {

    for (int i = 0; monsterClassCatalog[classID].memberList[i] != 0; i++) {
        if (monsterCatalog[monsterClassCatalog[classID].memberList[i]].flags & MONST_DEFEND_DEGRADE_WEAPON) {
            return true;
        }
    }
    return false;
}

void itemDetails(char *buf, item *theItem) {
    char buf2[1000], buf3[1000], theName[500], goodColorEscape[20], badColorEscape[20], whiteColorEscape[20];
    boolean singular, carried;
    fixpt enchant;
    fixpt currentDamage, newDamage;
    short nextLevelState = 0, new, current, accuracyChange, damageChange;
    unsigned long turnsSinceLatestUse;
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
    if (tableForItemCategory(theItem->category)
        && (tableForItemCategory(theItem->category)[theItem->kind].identified || rogue.playbackOmniscience)) {

        strcat(buf, tableForItemCategory(theItem->category)[theItem->kind].description);

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
                        tableForItemCategory(theItem->category)[theItem->kind].flavor,
                        (singular ? "it" : "they"));
                break;
            case SCROLL:
                sprintf(buf2, "%s parchment%s %s covered with indecipherable writing, and bear%s a title of \"%s.\" Who knows what %s will do when read aloud?",
                        (singular ? "This" : "These"),
                        (singular ? "" : "s"),
                        (singular ? "is" : "are"),
                        (singular ? "s" : ""),
                        tableForItemCategory(theItem->category)[theItem->kind].flavor,
                        (singular ? "it" : "they"));
                break;
            case STAFF:
                sprintf(buf2, "This gnarled %s staff is warm to the touch. Who knows what it will do when used?",
                        tableForItemCategory(theItem->category)[theItem->kind].flavor);
                break;
            case WAND:
                sprintf(buf2, "This thin %s wand is warm to the touch. Who knows what it will do when used?",
                        tableForItemCategory(theItem->category)[theItem->kind].flavor);
                break;
            case RING:
                sprintf(buf2, "This metal band is adorned with a%s %s gem that glitters in the darkness. Who knows what effect it has when worn? ",
                        isVowelish(tableForItemCategory(theItem->category)[theItem->kind].flavor) ? "n" : "",
                        tableForItemCategory(theItem->category)[theItem->kind].flavor);
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
        sprintf(buf2, " (You found %s%s on depth %i.) ",
            singular ? "it" : "them",
            // items in an item-choice vault are "keys", but are not a KEY
            // we ignore any actual KEY, as those might not be on an altar
            theItem->flags & ITEM_IS_KEY && !(theItem->category & KEY)
                ? " in a vault" : "",
            theItem->originDepth
        );
        strcat(buf, buf2);
    }

    // detailed description
    switch (theItem->category) {

        case FOOD:
            sprintf(buf2, "\n\nYou are %shungry enough to fully enjoy a %s.",
                    ((STOMACH_SIZE - player.status[STATUS_NUTRITION]) >= foodTable[theItem->kind].power ? "" : "not yet "),
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
                            (theItem->charges == gameConst->weaponKillsToAutoID ? "" : " more"),
                            (theItem->charges == 1 ? "enemy" : "enemies"));
                } else {
                    sprintf(buf2, "It will reveal its secrets if worn for %i%s turn%s. ",
                            theItem->charges,
                            (theItem->charges == gameConst->armorDelayToAutoID ? "" : " more"),
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
                    new = 0;

                    if ((theItem->flags & ITEM_IDENTIFIED) || rogue.playbackOmniscience) {
                        new = theItem->armor;
                        new += 10 * netEnchant(theItem) / FP_FACTOR;
                        new /= 10;
                    } else {
                        new = armorValueIfUnenchanted(theItem);
                    }

                    new = max(0, new);

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
            current = armorStealthAdjustment(rogue.armor);
            if ((theItem->category & ARMOR)
                && !(theItem->flags & ITEM_EQUIPPED)
                && (current != armorStealthAdjustment(theItem))) {

                new = armorStealthAdjustment(theItem);
                if (rogue.armor) {
                    new -= armorStealthAdjustment(rogue.armor);
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
                            if (monsterClassHasAcidicMonster(theItem->vorpalEnemy)) {
                                sprintf(buf2, "It is impervious to corrosion when attacking a%s %s, and will never fail to slay one in a single stroke. ",
                                        (isVowelish(buf3) ? "n" : ""),
                                        buf3);
                            } else {
                                sprintf(buf2, "It will never fail to slay a%s %s in a single stroke. ",
                                        (isVowelish(buf3) ? "n" : ""),
                                        buf3);
                            }
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
                                        weaponImageCount(enchant + enchantMagnitude() * enchantIncrement(theItem)),
                                        (weaponImageCount(enchant + enchantMagnitude() * enchantIncrement(theItem)) > 1 ? "s" : ""),
                                        runicWeaponChance(theItem, true, enchant + enchantMagnitude() * enchantIncrement(theItem)),
                                        weaponImageDuration(enchant + enchantMagnitude() * enchantIncrement(theItem)));
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
                                        nextLevelState = (int) (weaponParalysisDuration(enchant + enchantMagnitude() * enchantIncrement(theItem)));
                                        break;
                                    case W_SLOWING:
                                        sprintf(buf2, " for %i turns. ",
                                                weaponSlowDuration(enchant));
                                        strcat(buf, buf2);
                                        nextLevelState = weaponSlowDuration(enchant + enchantMagnitude() * enchantIncrement(theItem));
                                        break;
                                    case W_CONFUSION:
                                        sprintf(buf2, " for %i turns. ",
                                                weaponConfusionDuration(enchant));
                                        strcat(buf, buf2);
                                        nextLevelState = weaponConfusionDuration(enchant + enchantMagnitude() * enchantIncrement(theItem));
                                        break;
                                    case W_FORCE:
                                        sprintf(buf2, " up to %i spaces backward. If the enemy hits an obstruction, it (and any monster it hits) will take damage in proportion to the distance it flew. ",
                                                weaponForceDistance(enchant));
                                        strcat(buf, buf2);
                                        nextLevelState = weaponForceDistance(enchant + enchantMagnitude() * enchantIncrement(theItem));
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
                                    && runicWeaponChance(theItem, false, 0) < runicWeaponChance(theItem, true, enchant + enchantMagnitude() * enchantIncrement(theItem))){
                                    sprintf(buf2, "(If the %s is enchanted, the chance will increase to %i%%",
                                            theName,
                                            runicWeaponChance(theItem, true, enchant + enchantMagnitude() * enchantIncrement(theItem)));
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
                                if (armorImageCount(enchant + enchantMagnitude() * enchantIncrement(theItem)) > armorImageCount(enchant)) {
                                    sprintf(buf3, "(If the %s is enchanted, the number of duplicates will increase to %i.) ",
                                            theName,
                                            (armorImageCount(enchant + enchantMagnitude() * enchantIncrement(theItem))));
                                    strcat(buf2, buf3);
                                }
                                break;
                            case A_MUTUALITY:
                                strcpy(buf2, "When worn, the damage that you incur from physical attacks will be split evenly among yourself and all other adjacent enemies. ");
                                break;
                            case A_ABSORPTION:
                                if (theItem->flags & ITEM_IDENTIFIED) {
                                    sprintf(buf2, "It will reduce the damage of inbound attacks by a random amount between 1 and %i, which is %i%% of your current maximum health. (If the %s is enchanted, this maximum amount will %s %i.) ",
                                            (int) armorAbsorptionMax(enchant),
                                            (int) (100 * armorAbsorptionMax(enchant) / player.info.maxHP),
                                            theName,
                                            (armorAbsorptionMax(enchant) == armorAbsorptionMax(enchant + enchantIncrement(theItem)) ? "remain at" : "increase to"),
                                            (int) armorAbsorptionMax(enchant + enchantMagnitude() * enchantIncrement(theItem)));
                                } else {
                                    strcpy(buf2, "It will reduce the damage of inbound attacks by a random amount determined by its enchantment level. ");
                                }
                                break;
                            case A_REPRISAL:
                                if (theItem->flags & ITEM_IDENTIFIED) {
                                    sprintf(buf2, "Any enemy that attacks you will itself be wounded by %i%% of the damage that it inflicts. (If the %s is enchanted, this percentage will increase to %i%%.) ",
                                            armorReprisalPercent(enchant),
                                            theName,
                                            armorReprisalPercent(enchant + enchantMagnitude() * enchantIncrement(theItem)));
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
                                        short reflectChance2 = reflectionChance(enchant + enchantMagnitude() * enchantIncrement(theItem));
                                        sprintf(buf2, "When worn, you will deflect %i%% of incoming spells -- including directly back at their source %i%% of the time. (If the armor is enchanted, these will increase to %i%% and %i%%.) ",
                                                reflectChance,
                                                reflectChance * reflectChance / 100,
                                                reflectChance2,
                                                reflectChance2 * reflectChance2 / 100);
                                    } else if (theItem->enchant1 < 0) {
                                        short reflectChance = reflectionChance(enchant);
                                        short reflectChance2 = reflectionChance(enchant + enchantMagnitude() * enchantIncrement(theItem));
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
                sprintf(buf2, "\n\nThe %s has %i charges remaining out of a maximum of %i charges, and%s recovers a charge in approximately %lli turns. ",
                        theName,
                        theItem->charges,
                        theItem->enchant1,
                        new == 0 ? "" : ", with your current rings,",
                        FP_DIV(staffChargeDuration(theItem), 10 * ringWisdomMultiplier(new * FP_FACTOR)));
                strcat(buf, buf2);
            } else if (theItem->flags & ITEM_MAX_CHARGES_KNOWN) {
                sprintf(buf2, "\n\nThe %s has a maximum of %i charges, and%s recovers a charge in approximately %lli turns. ",
                        theName,
                        theItem->enchant1,
                        new == 0 ? "" : ", with your current rings,",
                        FP_DIV(staffChargeDuration(theItem), 10 * ringWisdomMultiplier(new * FP_FACTOR)));
                strcat(buf, buf2);
            }

            if (theItem->lastUsed[0] > 0 && theItem->lastUsed[1] > 0 && theItem->lastUsed[2] > 0) {
                sprintf(buf2, "You last used it %li, %li and %li turns ago. ",
                        rogue.absoluteTurnNumber - theItem->lastUsed[0],
                        rogue.absoluteTurnNumber - theItem->lastUsed[1],
                        rogue.absoluteTurnNumber - theItem->lastUsed[2]);
                strcat(buf, buf2);
            } else if (theItem->lastUsed[0] > 0 && theItem->lastUsed[1] > 0) {
                sprintf(buf2, "You last used it %li and %li turns ago. ",
                        rogue.absoluteTurnNumber - theItem->lastUsed[0],
                        rogue.absoluteTurnNumber - theItem->lastUsed[1]);
                strcat(buf, buf2);
            } else if (theItem->lastUsed[0] > 0) {
                turnsSinceLatestUse = rogue.absoluteTurnNumber - theItem->lastUsed[0];
                sprintf(buf2, "You last used it %li turn%s ago. ",
                        turnsSinceLatestUse,
                        turnsSinceLatestUse == 1 ? "" : "s");
                strcat(buf, buf2);
            }

            // effect description
            if (((theItem->flags & (ITEM_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN)) && staffTable[theItem->kind].identified)
                || rogue.playbackOmniscience) {
                switch (theItem->kind) {
                    case STAFF_LIGHTNING:
                        sprintf(buf2, "This staff deals damage to every creature in its line of fire; nothing is immune. (If the staff is enchanted, its average damage will increase by %i%%.)",
                                (int) (100 * (staffDamageLow(enchant + enchantMagnitude() * FP_FACTOR) + staffDamageHigh(enchant + enchantMagnitude() * FP_FACTOR)) / (staffDamageLow(enchant) + staffDamageHigh(enchant)) - 100));
                        break;
                    case STAFF_FIRE:
                        sprintf(buf2, "This staff deals damage to any creature that it hits, unless the creature is immune to fire. (If the staff is enchanted, its average damage will increase by %i%%.) It also sets creatures and flammable terrain on fire.",
                                (int) (100 * (staffDamageLow(enchant + enchantMagnitude() * FP_FACTOR) + staffDamageHigh(enchant + enchantMagnitude() * FP_FACTOR)) / (staffDamageLow(enchant) + staffDamageHigh(enchant)) - 100));
                        break;
                    case STAFF_POISON:
                        sprintf(buf2, "The bolt from this staff will poison any creature that it hits for %i turns. (If the staff is enchanted, this will increase to %i turns.)",
                                staffPoison(enchant),
                                staffPoison(enchant + enchantMagnitude() * FP_FACTOR));
                        break;
                    case STAFF_TUNNELING:
                        sprintf(buf2, "The bolt from this staff will dissolve %i layers of obstruction. (If the staff is enchanted, this will increase to %i layers.)",
                                theItem->enchant1,
                                theItem->enchant1 + enchantMagnitude());
                        break;
                    case STAFF_BLINKING:
                        sprintf(buf2, "This staff enables you to teleport up to %i spaces. (If the staff is enchanted, this will increase to %i spaces.)",
                                staffBlinkDistance(enchant),
                                staffBlinkDistance(enchant + enchantMagnitude() * FP_FACTOR));
                        break;
                    case STAFF_ENTRANCEMENT:
                        sprintf(buf2, "This staff will compel its target to mirror your movements for %i turns. (If the staff is enchanted, this will increase to %i turns.)",
                                staffEntrancementDuration(enchant),
                                staffEntrancementDuration(enchant + enchantMagnitude() * FP_FACTOR));
                        break;
                    case STAFF_HEALING:
                        if (enchant / FP_FACTOR < 10) {
                            sprintf(buf2, "This staff will heal its target by %i%% of its maximum health. (If the staff is enchanted, this will increase to %i%%.)",
                                    theItem->enchant1 * 10,
                                    (theItem->enchant1 + enchantMagnitude()) * 10);
                        } else {
                            strcpy(buf2, "This staff will completely heal its target.");
                        }
                        break;
                    case STAFF_HASTE:
                        sprintf(buf2, "This staff will cause its target to move twice as fast for %i turns. (If the staff is enchanted, this will increase to %i turns.)",
                                staffHasteDuration(enchant),
                                staffHasteDuration(enchant + enchantMagnitude() * FP_FACTOR));
                        break;
                    case STAFF_OBSTRUCTION:
                        strcpy(buf2, "");
                        break;
                    case STAFF_DISCORD:
                        sprintf(buf2, "This staff will cause discord for %i turns. (If the staff is enchanted, this will increase to %i turns.)",
                                staffDiscordDuration(enchant),
                                staffDiscordDuration(enchant + enchantMagnitude() * FP_FACTOR));
                        break;
                    case STAFF_CONJURATION:
                        sprintf(buf2, "%i phantom blades will be called into service. (If the staff is enchanted, this will increase to %i blades.)",
                                staffBladeCount(enchant),
                                staffBladeCount(enchant + enchantMagnitude() * FP_FACTOR));
                        break;
                    case STAFF_PROTECTION:
                        sprintf(buf2, "This staff will shield a creature for up to 20 turns against up to %i damage. (If the staff is enchanted, this will increase to %i damage.)",
                                staffProtection(enchant) / 10,
                                staffProtection(enchant + enchantMagnitude() * FP_FACTOR) / 10);
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
                            wandTable[theItem->kind].range.lowerBound * enchantMagnitude(),
                            (wandTable[theItem->kind].range.lowerBound * enchantMagnitude() == 1 ? "" : "s"));
                } else {
                    sprintf(buf2, "No charges remain.  Enchanting this wand will add %i charge%s.",
                            wandTable[theItem->kind].range.lowerBound * enchantMagnitude(),
                            (wandTable[theItem->kind].range.lowerBound * enchantMagnitude() == 1 ? "" : "s"));
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
                            wandTable[theItem->kind].range.lowerBound * enchantMagnitude(),
                            (wandTable[theItem->kind].range.lowerBound * enchantMagnitude() == 1 ? "" : "s"));
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
                                        theItem->enchant1 + 1 + enchantMagnitude());
                            } else {
                                sprintf(buf2, "\n\nThis ring magically blinds you to a radius of %i. (If the ring is enchanted, this will decrease to %i.)",
                                        (theItem->enchant1 * -1) + 1,
                                        (theItem->enchant1 * -1) + 1 - enchantMagnitude());
                            }
                            strcat(buf, buf2);
                            break;
                        case RING_REGENERATION:
                            sprintf(buf2, "\n\nWith this ring equipped, you will regenerate all of your health in %li turns (instead of %li). (If the ring is enchanted, this will decrease to %li turns.)",
                                    (long) (turnsForFullRegenInThousandths(enchant) / 1000),
                                    (long) TURNS_FOR_FULL_REGEN,
                                    (long) (turnsForFullRegenInThousandths(enchant + enchantMagnitude() * FP_FACTOR) / 1000));
                            strcat(buf, buf2);
                            break;
                        case RING_TRANSFERENCE:
                            sprintf(buf2, "\n\nDealing direct damage to a creature (whether in melee or otherwise) will %s you by %i%% of the damage dealt. (If the ring is enchanted, this will %s to %i%%.)",
                                    (theItem->enchant1 >= 0 ? "heal" : "harm"),
                                    abs(theItem->enchant1) * 5,
                                    (theItem->enchant1 >= 0 ? "increase" : "decrease"),
                                    abs(theItem->enchant1 + enchantMagnitude()) * 5);
                            strcat(buf, buf2);
                            break;
                        case RING_WISDOM:
                            sprintf(buf2, "\n\nWhen worn, your staffs will recharge at %i%% of their normal rate. (If the ring is enchanted, the rate will increase to %i%% of the normal rate.)",
                                    (int) (100 * ringWisdomMultiplier(enchant) / FP_FACTOR),
                                    (int) (100 * ringWisdomMultiplier(enchant + enchantMagnitude() * FP_FACTOR) / FP_FACTOR));
                            strcat(buf, buf2);
                            break;
                        case RING_REAPING:
                            sprintf(buf2, "\n\nEach blow that you land with a weapon will %s your staffs and charms by 0-%i turns per point of damage dealt. (If the ring is enchanted, this will %s to 0-%i turns per point of damage.)",
                                    (theItem->enchant1 >= 0 ? "recharge" : "drain"),
                                    abs(theItem->enchant1),
                                    (theItem->enchant1 >= 0 ? "increase" : "decrease"),
                                    abs(theItem->enchant1 + enchantMagnitude()));
                            strcat(buf, buf2);
                            break;
                        default:
                            break;
                    }
                }
            } else {
                sprintf(buf2, "\n\nIt will reveal its secrets if worn for %i%s turn%s",
                        theItem->charges,
                        (theItem->charges == gameConst->ringDelayToAutoID ? "" : " more"),
                        (theItem->charges == 1 ? "" : "s"));
                strcat(buf, buf2);

                if (!(theItem->flags & ITEM_IDENTIFIED) && (theItem->charges < gameConst->ringDelayToAutoID || (theItem->flags & ITEM_MAGIC_DETECTED))) {
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
                            charmHealing(enchant + enchantMagnitude() * FP_FACTOR),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + enchantMagnitude()));
                    break;
                case CHARM_PROTECTION:
                    sprintf(buf2, "\n\nWhen used, the charm will shield you for up to 20 turns for up to %i%% of your total health and recharge in %i turns. (If the charm is enchanted, it will shield up to %i%% of your total health and recharge in %i turns.)",
                            100 * charmProtection(enchant) / 10 / player.info.maxHP,
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            100 * charmProtection(enchant + enchantMagnitude() * FP_FACTOR) / 10 / player.info.maxHP,
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + enchantMagnitude()));
                    break;
                case CHARM_HASTE:
                    sprintf(buf2, "\n\nWhen used, the charm will haste you for %i turns and recharge in %i turns. (If the charm is enchanted, the haste will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmEffectDuration(theItem->kind, theItem->enchant1 + enchantMagnitude()),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + enchantMagnitude()));
                    break;
                case CHARM_FIRE_IMMUNITY:
                    sprintf(buf2, "\n\nWhen used, the charm will grant you immunity to fire for %i turns and recharge in %i turns. (If the charm is enchanted, the immunity will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmEffectDuration(theItem->kind, theItem->enchant1 + enchantMagnitude()),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + enchantMagnitude()));
                    break;
                case CHARM_INVISIBILITY:
                    sprintf(buf2, "\n\nWhen used, the charm will turn you invisible for %i turns and recharge in %i turns. While invisible, monsters more than two spaces away cannot track you. (If the charm is enchanted, the invisibility will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmEffectDuration(theItem->kind, theItem->enchant1 + enchantMagnitude()),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + enchantMagnitude()));
                    break;
                case CHARM_TELEPATHY:
                    sprintf(buf2, "\n\nWhen used, the charm will grant you telepathy for %i turns and recharge in %i turns. (If the charm is enchanted, the telepathy will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmEffectDuration(theItem->kind, theItem->enchant1 + enchantMagnitude()),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + enchantMagnitude()));
                    break;
                case CHARM_LEVITATION:
                    sprintf(buf2, "\n\nWhen used, the charm will lift you off the ground for %i turns and recharge in %i turns. (If the charm is enchanted, the levitation will last %i turns and it will recharge in %i turns.)",
                            charmEffectDuration(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmEffectDuration(theItem->kind, theItem->enchant1 + enchantMagnitude()),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + enchantMagnitude()));
                    break;
                case CHARM_SHATTERING:
                    sprintf(buf2, "\n\nWhen used, the charm will dissolve the nearby walls up to %i spaces away, and recharge in %i turns. (If the charm is enchanted, it will reach up to %i spaces and recharge in %i turns.)",
                            charmShattering(enchant),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmShattering(enchant + enchantMagnitude() * FP_FACTOR),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + enchantMagnitude()));
                    break;
                case CHARM_GUARDIAN:
                    sprintf(buf2, "\n\nWhen used, a guardian will materialize for %i turns, and the charm will recharge in %i turns. (If the charm is enchanted, the guardian will last for %i turns and the charm will recharge in %i turns.)",
                            charmGuardianLifespan(enchant),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmGuardianLifespan(enchant + enchantMagnitude() * FP_FACTOR),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + enchantMagnitude()));
                    break;
                case CHARM_TELEPORTATION:
                    sprintf(buf2, "\n\nWhen used, the charm will teleport you elsewhere in the dungeon and recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + enchantMagnitude()));
                    break;
                case CHARM_RECHARGING:
                    sprintf(buf2, "\n\nWhen used, the charm will recharge your staffs (though not your wands or charms), after which it will recharge in %i turns. (If the charm is enchanted, it will recharge in %i turns.)",
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + enchantMagnitude()));
                    break;
                case CHARM_NEGATION:
                    sprintf(buf2, "\n\nWhen used, the charm will emit a wave of anti-magic up to %i spaces away, negating all magical effects on you and on creatures and dropped items in your field of view. It will recharge in %i turns. (If the charm is enchanted, it will reach up to %i spaces and recharge in %i turns.)",
                            charmNegationRadius(enchant),
                            charmRechargeDelay(theItem->kind, theItem->enchant1),
                            charmNegationRadius(enchant + enchantMagnitude() * FP_FACTOR),
                            charmRechargeDelay(theItem->kind, theItem->enchant1 + enchantMagnitude()));
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

static boolean displayMagicCharForItem(item *theItem) {
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


    screenDisplayBuffer dbuf;
    clearDisplayBuffer(&dbuf);

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
        message("Your pack is empty!", 0);
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
        message("Nothing of that type!", 0);
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
        drawButton(&(buttons[i]), BUTTON_NORMAL, &dbuf);
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

    const SavedDisplayBuffer rbuf = saveDisplayBuffer();
    overlayDisplayBuffer(&dbuf);

    do {
        repeatDisplay = false;

        // Do the button loop.
        highlightItemLine = -1;
        restoreDisplayBuffer(&rbuf);   // Remove the inventory display while the buttons are active,
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

                overlayDisplayBuffer(&dbuf);

                //buttons[highlightItemLine].buttonColor = interfaceBoxColor;
                drawButton(&(buttons[highlightItemLine]), BUTTON_PRESSED, NULL);
                //buttons[highlightItemLine].buttonColor = black;

                if (theEvent.shiftKey || theEvent.controlKey || waitForAcknowledge) {
                    // Display an information window about the item.
                    actionKey = printCarriedItemDetails(theItem, max(2, mapToWindowX(DCOLS - maxLength - 42)), mapToWindowY(2), 40, includeButtons);

                    restoreDisplayBuffer(&rbuf); // remove the item info window

                    if (actionKey == -1) {
                        repeatDisplay = true;
                        overlayDisplayBuffer(&dbuf); // redisplay the inventory
                    } else {
                        restoreRNG;
                        repeatDisplay = false;
                        restoreDisplayBuffer(&rbuf); // restore the original screen
                    }

                    switch (actionKey) {
                        case APPLY_KEY:
                            apply(theItem);
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

    restoreDisplayBuffer(&rbuf); // restore the original screen

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
            message("Your pack is empty!", 0);
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
            message("You have nothing suitable.", 0);
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

// Estimates the armor value of the given item, assuming the item is unenchanted.
short armorValueIfUnenchanted(item *theItem) {
    short averageValue = (armorTable[theItem->kind].range.upperBound + armorTable[theItem->kind].range.lowerBound) / 2;
    short strengthAdjusted = averageValue + 10 * strengthModifier(theItem) / FP_FACTOR;
    return max(0, strengthAdjusted / 10);
}

// Calculates the armor value to display to the player (estimated if the item is unidentified).
short displayedArmorValue() {
    if (!rogue.armor || (rogue.armor->flags & ITEM_IDENTIFIED)) {
        return player.info.defense / 10;
    } else {
        return armorValueIfUnenchanted(rogue.armor);
    }
}

void strengthCheck(item *theItem, boolean noisy) {
    char buf1[COLS], buf2[COLS*2];
    short strengthDeficiency;

    updateEncumbrance();
    if (noisy && theItem) {
        if (theItem->category & WEAPON && theItem->strengthRequired > rogue.strength - player.weaknessAmount) {
            strengthDeficiency = theItem->strengthRequired - max(0, rogue.strength - player.weaknessAmount);
            strcpy(buf1, "");
            itemName(theItem, buf1, false, false, NULL);
            sprintf(buf2, "You can barely lift the %s; %i more strength would be ideal.", buf1, strengthDeficiency);
            message(buf2, 0);
        }

        if (theItem->category & ARMOR && theItem->strengthRequired > rogue.strength - player.weaknessAmount) {
            strengthDeficiency = theItem->strengthRequired - max(0, rogue.strength - player.weaknessAmount);
            strcpy(buf1, "");
            itemName(theItem, buf1, false, false, NULL);
            sprintf(buf2, "You stagger under the weight of the %s; %i more strength would be ideal.",
                    buf1, strengthDeficiency);
            message(buf2, 0);
        }
    }
}

// Will prompt for an item if none is given.
// Equips the item and records input if successful.
// Player's failure to select an item will result in failure.
// Failure does not record input.
void equip(item *theItem) {
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

    theItem2 = NULL;
    command[c++] = theItem->inventoryLetter;

    if (theItem->category & (WEAPON|ARMOR|RING)) {

        if (theItem->category & RING) {
            if (theItem->flags & ITEM_EQUIPPED) {
                confirmMessages();
                message("you are already wearing that ring.", 0);
                return;
            } else if (rogue.ringLeft && rogue.ringRight) {
                confirmMessages();
                theItem2 = promptForItemOfType((RING), ITEM_EQUIPPED, 0,
                                               "You are already wearing two rings; remove which first?", true);
                if (!theItem2 || theItem2->category != RING || !(theItem2->flags & ITEM_EQUIPPED)) {
                    if (theItem2) { // No message if canceled or did an inventory action instead.
                        message("Invalid entry.", 0);
                    }
                    return;
                } else {
                    command[c++] = theItem2->inventoryLetter;
                }
            }
        }

        if (theItem->flags & ITEM_EQUIPPED) {
            confirmMessages();
            message("already equipped.", 0);
            return;
        }

        if (theItem->category & (WEAPON | ARMOR)) {
            // Swapped out rings are handled above
            theItem2 = theItem->category & WEAPON ? rogue.weapon : rogue.armor;
        }

        if (!equipItem(theItem, false, theItem2)) {
            return; // equip failed because current item is cursed
        }

        command[c] = '\0';
        recordKeystrokeSequence(command);

        // Something is only swapped in if something else swapped out
        rogue.swappedOut = theItem2;
        rogue.swappedIn = rogue.swappedOut ? theItem : NULL;

        playerTurnEnded();
    } else {
        confirmMessages();
        message("You can't equip that.", 0);
    }
}

// Returns whether the given item is a key that can unlock the given location.
// An item qualifies if:
// (1) it's a key (has ITEM_IS_KEY flag),
// (2) its originDepth matches the depth, and
// (3) either its key (x, y) location matches (x, y), or its machine number matches the machine number at (x, y).
static boolean keyMatchesLocation(item *theItem, pos loc) {
    if ((theItem->flags & ITEM_IS_KEY)
        && theItem->originDepth == rogue.depthLevel) {

        for (int i=0; i < KEY_ID_MAXIMUM && (theItem->keyLoc[i].loc.x || theItem->keyLoc[i].machine); i++) {
            if (posEq(theItem->keyLoc[i].loc, loc)) {
                return true;
            } else if (theItem->keyLoc[i].machine == pmapAt(loc)->machineNumber) {
                return true;
            }
        }
    }
    return false;
}

item *keyInPackFor(pos loc) {
    item *theItem;

    for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (keyMatchesLocation(theItem, loc)) {
            return theItem;
        }
    }
    return NULL;
}

item *keyOnTileAt(pos loc) {
    item *theItem;
    creature *monst;

    if ((pmapAt(loc)->flags & HAS_PLAYER) && posEq(player.loc, loc) && keyInPackFor(loc)) {

        return keyInPackFor(loc);
    }
    if (pmapAt(loc)->flags & HAS_ITEM) {
        theItem = itemAtLoc(loc);
        if (keyMatchesLocation(theItem, loc)) {
            return theItem;
        }
    }
    if (pmapAt(loc)->flags & HAS_MONSTER) {
        monst = monsterAtLoc(loc);
        if (monst->carriedItem) {
            theItem = monst->carriedItem;
            if (keyMatchesLocation(theItem, loc)) {
                return theItem;
            }
        }
    }
    return NULL;
}

// Aggroes out to the given distance.
void aggravateMonsters(short distance, short x, short y, const color *flashColor) {
    short i, j, **grid;

    rogue.wpCoordinates[0] = (pos) { x, y };
    refreshWaypoint(0);

    grid = allocGrid();
    fillGrid(grid, 0);
    calculateDistances(grid, x, y, T_PATHING_BLOCKER, NULL, true, false);

    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (grid[monst->loc.x][monst->loc.y] <= distance) {
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

    if (player.loc.x == x && player.loc.y == y) {
        player.status[STATUS_AGGRAVATING] = player.maxStatus[STATUS_AGGRAVATING] = distance;
        rogue.stealthRange = currentStealthRange();
    }

    if (grid[player.loc.x][player.loc.y] >= 0 && grid[player.loc.x][player.loc.y] <= distance) {
        discover(x, y);
        discoverCell(x, y);
        colorFlash(flashColor, 0, (DISCOVERED | MAGIC_MAPPED), 10, distance, x, y);
        if (!playerCanSee(x, y)) {
            message("You hear a piercing shriek; something must have triggered a nearby alarm.", 0);
        }
    }

    freeGrid(grid);
}

// Generates a list of coordinates extending in a straight line
// from originLoc (not included in the output), through targetLoc,
// all the way to the edge of the map.
// Any straight line passing through the target cell generates a valid
// path; the function tries several lines and picks the one that works
// best for the specified bolt type.
// The list is terminated by a marker (-1, -1).
// Returns the number of entries in the list (not counting the terminal marker).
short getLineCoordinates(pos listOfCoordinates[], const pos originLoc, const pos targetLoc, const bolt *theBolt) {
    fixpt point[2], step[2];
    short listLength;
    int score, bestScore = 0, offset, bestOffset = 0;

    // Set of candidate waypoints strategically placed within a diamond shape.
    // For why they must be within a diamond, google "diamond-exit rule".
    const int numOffsets = 21;
    const fixpt offsets[][2] = {
        {50, 50}, // center of the square first (coordinates are in %)
        {40, 40}, {60, 40}, {60, 60}, {40, 60},
        {50, 30}, {70, 50}, {50, 70}, {30, 50},
        {50, 20}, {80, 50}, {50, 80}, {20, 50},
        {50, 10}, {90, 50}, {50, 90}, {10, 50},
        {50,  1}, {99, 50}, {50, 99}, { 1, 50} };

    if (originLoc.x == targetLoc.x && originLoc.y == targetLoc.y) {
        listOfCoordinates[0] = INVALID_POS;
        return 0;
    }

    // try all offsets; the last iteration will use the best offset found
    for (offset = 0; offset < numOffsets + 1; offset++) {

        listLength = 0;


            // always shoot from the center of the origin cell
            point[0] = originLoc.x * FP_FACTOR + FP_FACTOR/2;
            point[1] = originLoc.y * FP_FACTOR + FP_FACTOR/2;
            // vector to target
            step[0] = targetLoc.x * FP_FACTOR + offsets[offset < numOffsets ? offset : bestOffset][0] * FP_FACTOR / 100 - point[0];
            step[1] = targetLoc.y * FP_FACTOR + offsets[offset < numOffsets ? offset : bestOffset][1] * FP_FACTOR / 100 - point[1];

        // normalize the step, to move exactly one row or column at a time
        fixpt m = max(llabs(step[0]), llabs(step[1]));
        step[0] = step[0] * FP_FACTOR / m;
        step[1] = step[1] * FP_FACTOR / m;

        // move until we exit the map
        while (true) {
            point[0] += step[0];
            point[1] += step[1];
            listOfCoordinates[listLength] = (pos){
                .x = (point[0] < 0 ? -1 : point[0] / FP_FACTOR),
                .y = (point[1] < 0 ? -1 : point[1] / FP_FACTOR)
            };
            if (!isPosInMap(listOfCoordinates[listLength])) break;
            listLength++;
        };

        // last iteration does not need evaluation, we are returning it anyway
        if (offset == numOffsets) break;

        // No bolt means we don't want any tuning. Returning first path (using center of target)
        if (theBolt == NULL) break;

        // evaluate this path; we will return the path with the highest score
        score = 0;

        boolean passesThroughUnknown = false;
        for (int i = 0; i < listLength; i++) {
            short x = listOfCoordinates[i].x;
            short y = listOfCoordinates[i].y;

            boolean isImpassable = cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY);
            boolean isOpaque = cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_VISION);
            boolean targetsEnemies = theBolt->flags & BF_TARGET_ENEMIES;
            boolean targetsAllies = theBolt->flags & BF_TARGET_ALLIES;
            boolean burningThrough = (theBolt->flags & BF_FIERY) && cellHasTerrainFlag((pos){ x, y }, T_IS_FLAMMABLE);
            boolean isCastByPlayer = (originLoc.x == player.loc.x && originLoc.y == player.loc.y);

            creature *caster = monsterAtLoc(originLoc);
            creature *monst = monsterAtLoc((pos){ x, y });
            boolean isMonster = monst
                && !(monst->bookkeepingFlags & MB_SUBMERGED)
                && !monsterIsHidden(monst, caster);
            boolean isEnemyOfCaster = (monst && caster && monstersAreEnemies(monst, caster));
            boolean isAllyOfCaster = (monst && caster && monstersAreTeammates(monst, caster));

            // small bonus for making it this far
            score += 2;

            // target reached?
            if (x == targetLoc.x && y == targetLoc.y) {

                if ((!targetsEnemies && !targetsAllies) ||
                    (targetsEnemies && isMonster && isEnemyOfCaster) ||
                    (targetsAllies && isMonster && isAllyOfCaster)) {

                    // Big bonus for hitting the target, but that bonus
                    // is lower if this path uses an unknown tile.
                    score += passesThroughUnknown ? 2500 : 5000;
                }

                break; // we don't care about anything beyond the target--if the player did, they would have selected a farther target
            }

            // if the caster is the player, undiscovered cells don't count (lest we reveal something about them)
            if (isCastByPlayer && !(pmap[x][y].flags & (DISCOVERED | MAGIC_MAPPED))) {
                // Remember that this path used an unknown cell, so that a
                // less-risky path can be used instead if one is known.
                passesThroughUnknown = true;
                continue;
            }

            // nothing can get through impregnable obstacles
            if (isImpassable && pmap[x][y].flags & IMPREGNABLE) {
                break;
            }

            // tunneling goes through everything
            if (theBolt->boltEffect == BE_TUNNELING) {
                score += (isImpassable ? 50 : isOpaque ? 10 : 0);
                continue;
            }

            // hitting a creature with a bolt meant for enemies
            if (isMonster && targetsEnemies) {
                score += isEnemyOfCaster ? 50 : -200;
            }

            // hitting a creature with a bolt meant for allies
            if (isMonster && targetsAllies) {
                score += isAllyOfCaster ? 50 : -200;
            }

            // small penalty for setting terrain on fire (to prefer not to)
            if (burningThrough) {
                score -= 1;
            }

            // check for obstruction
            if (isMonster && (theBolt->flags & BF_PASSES_THRU_CREATURES)) continue;
            if (isMonster || isImpassable || (isOpaque && !burningThrough)) break;
        }

        if (score > bestScore) {
            bestScore = score;
            bestOffset = offset;
        }
    }

    // demarcate the end of the list
    listOfCoordinates[listLength] = INVALID_POS;

    return listLength;
}

// If a hypothetical bolt were launched from originLoc toward targetLoc,
// with a given max distance and a toggle as to whether it halts at its impact location
// or one space prior, where would it stop?
// Takes into account the caster's knowledge; i.e. won't be blocked by monsters
// that the caster is not aware of.
void getImpactLoc(pos *returnLoc, const pos originLoc, const pos targetLoc,
                  const short maxDistance, const boolean returnLastEmptySpace, const bolt *theBolt) {
    pos coords[DCOLS + 1];
    short i, n;
    creature *monst;

    n = getLineCoordinates(coords, originLoc, targetLoc, theBolt);
    n = min(n, maxDistance);
    for (i=0; i<n; i++) {
        monst = monsterAtLoc(coords[i]);
        if (monst
            && !monsterIsHidden(monst, monsterAtLoc(originLoc))
            && !(monst->bookkeepingFlags & MB_SUBMERGED)) {
            // Imaginary bolt hit the player or a monster.
            break;
        }
        if (cellHasTerrainFlag(coords[i], (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))) {
            break;
        }
    }
    if (i == maxDistance) {
        *returnLoc = coords[i-1];
    } else if (returnLastEmptySpace) {
        if (i == 0) {
            *returnLoc = originLoc;
        } else {
            *returnLoc = coords[i-1];
        }
    } else {
        *returnLoc = coords[i];
    }
    brogueAssert(coordinatesAreInMap(returnLoc->x, returnLoc->y));
}

// Returns true if the two coordinates are unobstructed and diagonally adjacent,
// but their two common neighbors are obstructed and at least one blocks diagonal movement.
static boolean impermissibleKinkBetween(short x1, short y1, short x2, short y2) {
    brogueAssert(coordinatesAreInMap(x1, y1));
    brogueAssert(coordinatesAreInMap(x2, y2));
    if (cellHasTerrainFlag((pos){ x1, y1 }, T_OBSTRUCTS_PASSABILITY)
        || cellHasTerrainFlag((pos){ x2, y2 }, T_OBSTRUCTS_PASSABILITY)) {
        // One of the two locations is obstructed.
        return false;
    }
    if (abs(x1 - x2) != 1
        || abs(y1 - y2) != 1) {
        // Not diagonally adjacent.
        return false;
    }
    if (!cellHasTerrainFlag((pos){ x2, y1 }, T_OBSTRUCTS_PASSABILITY)
        || !cellHasTerrainFlag((pos){ x1, y2 }, T_OBSTRUCTS_PASSABILITY)) {
        // At least one of the common neighbors isn't obstructed.
        return false;
    }
    if (!cellHasTerrainFlag((pos){ x2, y1 }, T_OBSTRUCTS_DIAGONAL_MOVEMENT)
        && !cellHasTerrainFlag((pos){ x1, y2 }, T_OBSTRUCTS_DIAGONAL_MOVEMENT)) {
        // Neither of the common neighbors obstructs diagonal movement.
        return false;
    }
    return true;
}

static boolean tunnelize(short x, short y) {
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
            monst = monsterAtLoc((pos){ x, y });
            if (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS) {
                inflictLethalDamage(NULL, monst);
                killCreature(monst, false);
            }
        }
    }
    if (!cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_DIAGONAL_MOVEMENT)
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

/// @brief Checks if a monster will be affected by a negation bolt or blast
/// @param monst The monster
/// @param isBolt True to check for a negation bolt. False for negation blast.
/// @return True if negation will have an effect
static boolean negationWillAffectMonster(creature *monst, boolean isBolt) {

    // negation bolts don't affect monsters that always reflect. negation never affects the warden.
    if ((isBolt && (monst->info.abilityFlags & MA_REFLECT_100))
        || (monst->info.flags & MONST_INVULNERABLE)) {
        return false;
    }

    if ((monst->info.abilityFlags & ~MA_NON_NEGATABLE_ABILITIES)
        || (monst->bookkeepingFlags & MB_SEIZING)
        || (monst->info.flags & MONST_DIES_IF_NEGATED)
        || (monst->info.flags & NEGATABLE_TRAITS)
        || (monst->info.flags & MONST_IMMUNE_TO_FIRE)
        || ((monst->info.flags & MONST_FIERY) && (monst->status[STATUS_BURNING]))
        || (monst->status[STATUS_IMMUNE_TO_FIRE])
        || (monst->status[STATUS_SLOWED])
        || (monst->status[STATUS_HASTED])
        || (monst->status[STATUS_CONFUSED])
        || (monst->status[STATUS_ENTRANCED])
        || (monst->status[STATUS_DISCORDANT])
        || (monst->status[STATUS_SHIELDED])
        || (monst->status[STATUS_INVISIBLE])
        || (monst->status[STATUS_MAGICAL_FEAR])
        || (monst->status[STATUS_LEVITATING])
        || (monst->movementSpeed != monst->info.movementSpeed)
        || (monst->attackSpeed != monst->info.attackSpeed)
        || (monst->mutationIndex > -1 && mutationCatalog[monst->mutationIndex].canBeNegated)) {
        return true;
    }

    // any negatable bolts?
    for (int i = 0; i < 20; i++) {
        if (monst->info.bolts[i] && !(boltCatalog[monst->info.bolts[i]].flags & BF_NOT_NEGATABLE)) {
            return true;
        }
    }

    return false;
}

/* Negates the given creature. Returns true if there was an effect for the purpose of identifying a wand of negation.
 * If the creature was stripped of any traits or abilities, the wasNegated property is set, which is used for display in
 * sidebar and the creature's description.
 */
boolean negate(creature *monst) {
    short i, j;
    enum boltType backupBolts[20];
    char buf[DCOLS * 3], monstName[DCOLS];
    boolean negated = false;

    monsterName(monstName, monst, true);

    if (monst->info.abilityFlags & ~MA_NON_NEGATABLE_ABILITIES) {
        monst->info.abilityFlags &= MA_NON_NEGATABLE_ABILITIES; // negated monsters lose all special abilities
        negated = true;
        monst->wasNegated = true;
    }

    if (monst->bookkeepingFlags & MB_SEIZING){
        monst->bookkeepingFlags &= ~MB_SEIZING;
        negated = true;
    }

    if (monst->info.flags & MONST_DIES_IF_NEGATED) {
        if (monst->status[STATUS_LEVITATING]) {
            sprintf(buf, "%s dissipates into thin air", monstName);
        } else if (monst->info.flags & MONST_INANIMATE) {
            sprintf(buf, "%s shatters into tiny pieces", monstName);
        } else {
            sprintf(buf, "%s falls to the ground, lifeless", monstName);
        }
        killCreature(monst, false);
        combatMessage(buf, messageColorFromVictim(monst));
        negated = true;
    } else if (!(monst->info.flags & MONST_INVULNERABLE)) {
        if (canNegateCreatureStatusEffects(monst)) {
            negated = true;
            negateCreatureStatusEffects(monst);
        }
        if (monst->info.flags & MONST_IMMUNE_TO_FIRE) {
            monst->info.flags &= ~MONST_IMMUNE_TO_FIRE;
            monst->wasNegated = true;
            negated = true;
        }
        if (monst->movementSpeed != monst->info.movementSpeed) {
            monst->movementSpeed = monst->info.movementSpeed;
            negated = true;
        }
        if (monst->attackSpeed != monst->info.attackSpeed) {
            monst->attackSpeed = monst->info.attackSpeed;
            negated = true;
        }

        if (monst != &player && monst->mutationIndex > -1 && mutationCatalog[monst->mutationIndex].canBeNegated) {

            monst->mutationIndex = -1;
            negated = true;
            monst->wasNegated = true;
        }
        if (monst != &player && (monst->info.flags & NEGATABLE_TRAITS)) {
            if ((monst->info.flags & MONST_FIERY) && monst->status[STATUS_BURNING]) {
                extinguishFireOnCreature(monst);
            }
            monst->info.flags &= ~NEGATABLE_TRAITS;
            negated = true;
            monst->wasNegated = true;
            refreshDungeonCell(monst->loc);
            refreshSideBar(-1, -1, false);
        }
        for (i = 0; i < 20; i++) {
            backupBolts[i] = monst->info.bolts[i];
            if (monst->info.bolts[i] && !(boltCatalog[monst->info.bolts[i]].flags & BF_NOT_NEGATABLE)) {
                monst->info.bolts[i] = BOLT_NONE;
                negated = true;
                monst->wasNegated = true;
            }
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

    if (negated && monst != &player && !(monst->info.flags & MONST_DIES_IF_NEGATED)) {
        sprintf(buf, "%s is stripped of $HISHER special traits", monstName);
        resolvePronounEscapes(buf, monst);
        combatMessage(buf, messageColorFromVictim(monst));
    }

    return negated;
}

// Adds one to the creature's weakness, sets the weakness status duration to maxDuration.
void weaken(creature *monst, short maxDuration) {
    if (monst->weaknessAmount < 10) {
        monst->weaknessAmount++;
    }
    monst->status[STATUS_WEAKENED] = max(monst->status[STATUS_WEAKENED], maxDuration);
    monst->maxStatus[STATUS_WEAKENED] = max(monst->maxStatus[STATUS_WEAKENED], maxDuration);
    if (monst == &player) {
        messageWithColor("your muscles weaken as an enervating toxin fills your veins.", &badMessageColor, 0);
        strengthCheck(rogue.weapon, true);
        strengthCheck(rogue.armor, true);
    }
}

// True if the creature polymorphed; false if not.
static boolean polymorph(creature *monst) {
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

    // After polymorphing, don't "drop" any creature on death (e.g. phylactery, phoenix egg)
    if (monst->carriedMonster) {
        freeCreature(monst->carriedMonster);
        monst->carriedMonster = NULL;
    }

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
    monst->wasNegated = false;
    initializeStatus(monst);

    if (monst->bookkeepingFlags & MB_CAPTIVE) {
        demoteMonsterFromLeadership(monst);
        monst->creatureState = MONSTER_TRACKING_SCENT;
        monst->bookkeepingFlags &= ~MB_CAPTIVE;
    }
    monst->bookkeepingFlags &= ~(MB_SEIZING | MB_SEIZED);

    monst->ticksUntilTurn = max(monst->ticksUntilTurn, 101);

    refreshDungeonCell(monst->loc);
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
            message("you feel yourself slow down.", 0);
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
            message("you feel yourself speed up.", 0);
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

static void makePlayerTelepathic(short duration) {
    player.status[STATUS_TELEPATHIC] = player.maxStatus[STATUS_TELEPATHIC] = duration;
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        refreshDungeonCell(monst->loc);
    }
    if (!hasNextCreature(iterateCreatures(monsters))) {
        message("you can somehow tell that you are alone on this depth at the moment.", 0);
    } else {
        message("you can somehow feel the presence of other creatures' minds!", 0);
    }
}

static void rechargeItems(unsigned long categories) {
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
        message(buf, 0);
    } else {
        message("a surge of energy courses through your pack, but nothing happens.", 0);
    }
}

//void causeFear(const char *emitterName) {
//    creature *monst;
//    short numberOfMonsters = 0;
//    char buf[DCOLS*3], mName[DCOLS];
//
//    for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
//        if (pmapAt(monst->loc)->flags & IN_FIELD_OF_VIEW
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
//    message(buf, 0);
//    colorFlash(&redFlashColor, 0, IN_FIELD_OF_VIEW, 15, DCOLS, player.loc.x, player.loc.y);
//}

static void negationBlast(const char *emitterName, const short distance) {
    item *theItem;
    char buf[DCOLS];

    sprintf(buf, "%s emits a numbing torrent of anti-magic!", emitterName);
    messageWithColor(buf, &itemMessageColor, 0);
    colorFlash(&pink, 0, IN_FIELD_OF_VIEW, 3 + distance / 5, distance, player.loc.x, player.loc.y);
    negate(&player);
    flashMonster(&player, &pink, 100);
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if ((pmapAt(monst->loc)->flags & IN_FIELD_OF_VIEW)
            && (player.loc.x - monst->loc.x) * (player.loc.x - monst->loc.x) + (player.loc.y - monst->loc.y) * (player.loc.y - monst->loc.y) <= distance * distance) {

            if (canSeeMonster(monst)) {
                flashMonster(monst, &pink, 100);
            }
            negate(monst); // This can be fatal.
        }
    }
    for (theItem = floorItems; theItem != NULL; theItem = theItem->nextItem) {
        if ((pmapAt(theItem->loc)->flags & IN_FIELD_OF_VIEW)
            && (player.loc.x - theItem->loc.x) * (player.loc.x - theItem->loc.x) + (player.loc.y - theItem->loc.y) * (player.loc.y - theItem->loc.y) <= distance * distance) {

            theItem->flags &= ~(ITEM_MAGIC_DETECTED | ITEM_CURSED);
            switch (theItem->category) {
                case WEAPON:
                case ARMOR:
                    theItem->enchant1 = theItem->enchant2 = theItem->charges = 0;
                    theItem->flags &= ~(ITEM_RUNIC | ITEM_RUNIC_HINTED | ITEM_RUNIC_IDENTIFIED | ITEM_PROTECTED);
                    identify(theItem);
                    pmapAt(theItem->loc)->flags &= ~ITEM_DETECTED;
                    refreshDungeonCell(theItem->loc);
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

static void discordBlast(const char *emitterName, const short distance) {
    char buf[DCOLS];

    sprintf(buf, "%s emits a wave of unsettling purple radiation!", emitterName);
    messageWithColor(buf, &itemMessageColor, 0);
    colorFlash(&discordColor, 0, IN_FIELD_OF_VIEW, 3 + distance / 5, distance, player.loc.x, player.loc.y);
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if ((pmapAt(monst->loc)->flags & IN_FIELD_OF_VIEW)
            && (player.loc.x - monst->loc.x) * (player.loc.x - monst->loc.x) + (player.loc.y - monst->loc.y) * (player.loc.y - monst->loc.y) <= distance * distance) {

            if (!(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
                if (canSeeMonster(monst)) {
                    flashMonster(monst, &discordColor, 100);
                }
                monst->status[STATUS_DISCORDANT] = monst->maxStatus[STATUS_DISCORDANT] = 30;
            }
        }
    }
}

static void crystalize(short radius) {
    extern color forceFieldColor;
    short i, j;
    creature *monst;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j < DROWS; j++) {
            if ((player.loc.x - i) * (player.loc.x - i) + (player.loc.y - j) * (player.loc.y - j) <= radius * radius
                && !(pmap[i][j].flags & IMPREGNABLE)) {

                if (tileCatalog[pmap[i][j].layers[DUNGEON]].flags & (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION)) {

                    pmap[i][j].layers[DUNGEON] = FORCEFIELD;
                    spawnDungeonFeature(i, j, &dungeonFeatureCatalog[DF_SHATTERING_SPELL], true, false);

                    if (pmap[i][j].flags & HAS_MONSTER) {
                        monst = monsterAtLoc((pos){ i, j });
                        if (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS) {
                            inflictLethalDamage(NULL, monst);
                            killCreature(monst, false);
                        } else {
                            freeCaptivesEmbeddedAt(i, j);
                        }
                    }
                    if (i == 0 || i == DCOLS - 1 || j == 0 || j == DROWS - 1) {
                        pmap[i][j].layers[DUNGEON] = CRYSTAL_WALL; // boundary walls turn to crystal
                    }
                }
            }
        }
    }
    updateVision(false);
    colorFlash(&forceFieldColor, 0, 0, radius, radius, player.loc.x, player.loc.y);
    displayLevel();
    refreshSideBar(-1, -1, false);
}

static boolean imbueInvisibility(creature *monst, short duration) {
    boolean autoID = false;

    if (monst && !(monst->info.flags & (MONST_INANIMATE | MONST_INVISIBLE | MONST_INVULNERABLE))) {
        if (monst == &player || monst->creatureState == MONSTER_ALLY) {
            autoID = true;
        } else if (canSeeMonster(monst) && monsterRevealed(monst)) {
            autoID = true;
        }
        monst->status[STATUS_INVISIBLE] = monst->maxStatus[STATUS_INVISIBLE] = duration;
        refreshDungeonCell(monst->loc);
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

    if (defender && (defender->info.abilityFlags & MA_REFLECT_100)) {
        return true;
    }

    if (defender && (defender->info.flags & MONST_REFLECT_50)) {
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
short reflectBolt(short targetX, short targetY, pos listOfCoordinates[], short kinkCell, boolean retracePath) {
    boolean needRandomTarget = (targetX < 0 || targetY < 0
                        || (targetX == listOfCoordinates[kinkCell].x && targetY == listOfCoordinates[kinkCell].y));

    short finalLength;
    if (retracePath) {
        // if reflecting back at caster, follow precise trajectory until we reach the caster
        for (int k = 1; k <= kinkCell && kinkCell + k < MAX_BOLT_LENGTH; k++) {
            listOfCoordinates[kinkCell + k] = listOfCoordinates[kinkCell - k];
        }

        // Calculate a new "extension" path, with an origin at the caster, and a destination at
        // the caster's location translated by the vector from the reflection point to the caster.
        //
        // For example, if the player is at (0,0), and the caster is at (2,3), then the newpath
        // is from (2,3) to (4,6):
        // (2,3) + ((2,3) - (0,0)) = (4,6).

        pos origin = listOfCoordinates[2 * kinkCell];
        pos target = (pos){
            .x = targetX + (targetX - listOfCoordinates[kinkCell].x),
            .y = targetY + (targetY - listOfCoordinates[kinkCell].y)
        };

        pos newPath[DCOLS];
        // (NULL because the reflected bolt is not under the caster's control, so its path should not be tuned)
        const short newPathLength = getLineCoordinates(newPath, origin, target, NULL);

        for (int k=0; k<=newPathLength; k++) {
            listOfCoordinates[2 * kinkCell + k + 1] = newPath[k];
        }
        finalLength = 2 * kinkCell + newPathLength + 1;
    } else {
        short failsafe = 50;
        pos newPath[DCOLS];
        int newPathLength;
        do {
            pos target;
            if (needRandomTarget) {
                // pick random target
                target = perimeterCoords(rand_range(0, 39));
                target.x += listOfCoordinates[kinkCell].x;
                target.y += listOfCoordinates[kinkCell].y;
            } else {
                target = (pos){ .x = targetX, .y = targetY };
            }
            newPathLength = getLineCoordinates(newPath, listOfCoordinates[kinkCell], target, NULL);
            if (newPathLength > 0
                && !cellHasTerrainFlag(newPath[0], (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))) {

                needRandomTarget = false;
            }
        } while (needRandomTarget && --failsafe);

        for (int k = 0; k < newPathLength; k++) {
            listOfCoordinates[kinkCell + k + 1] = newPath[k];
        }

        finalLength = kinkCell + newPathLength + 1;
    }

    listOfCoordinates[finalLength] = INVALID_POS;
    return finalLength;
}

// Update stuff that promotes without keys so players can't abuse item libraries with blinking/haste shenanigans
void checkForMissingKeys(short x, short y) {
    short layer;

    if (cellHasTMFlag((pos){ x, y }, TM_PROMOTES_WITHOUT_KEY) && !keyOnTileAt((pos){ x, y })) {
        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_PROMOTES_WITHOUT_KEY) {
                promoteTile(x, y, layer, false);
            }
        }
    }
}

static void beckonMonster(creature *monst, short x, short y) {
    bolt theBolt = boltCatalog[BOLT_BLINKING];

    if (monst->bookkeepingFlags & MB_CAPTIVE) {
        freeCaptive(monst);
    }
    pos from = monst->loc;
    pos to = (pos){ .x = x, .y = y };
    theBolt.magnitude = max(1, (distanceBetween((pos){x, y}, monst->loc) - 2) / 2);
    zap(from, to, &theBolt, false, true);
    if (monst->ticksUntilTurn < player.attackSpeed+1) {
        monst->ticksUntilTurn = player.attackSpeed+1;
    }
}

enum boltEffects boltEffectForItem(item *theItem) {
    if (theItem->category & (STAFF | WAND)) {
        return boltCatalog[tableForItemCategory(theItem->category)[theItem->kind].power].boltEffect;
    } else {
        return BE_NONE;
    }
}

enum boltType boltForItem(item *theItem) {
    if (theItem->category & (STAFF | WAND)) {
        return tableForItemCategory(theItem->category)[theItem->kind].power;
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
static boolean updateBolt(bolt *theBolt, creature *caster, short x, short y,
                   boolean boltInView, boolean alreadyReflected,
                   boolean *autoID, boolean *lightingChanged) {
    char buf[COLS], monstName[COLS];
    creature *monst; // Creature being hit by the bolt, if any.
    creature *newMonst; // Utility variable for plenty
    boolean terminateBolt = false;
    boolean negated = false;

    if (lightingChanged) {
        *lightingChanged = false;
    }

    // Handle collisions with monsters.

    monst = monsterAtLoc((pos){ x, y });
    if (monst && !(monst->bookkeepingFlags & MB_SUBMERGED)) {
        monsterName(monstName, monst, true);

        switch(theBolt->boltEffect) {
            case BE_ATTACK:
                if (!cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)
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
                // Check paladin feat before creatureState is changed in inflictDamage()
                if (monst && caster == &player) {
                    if (((theBolt->flags & BF_FIERY) && !(monst->status[STATUS_IMMUNE_TO_FIRE] > 0 ))
                        || (theBolt->flags & BF_ELECTRIC)) {
                        handlePaladinFeat(monst);
                    }
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
                        killCreature(monst, false);
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
                    killCreature(monst, false);
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
                        splitMonster(monst, caster);
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
                    teleport(monst, INVALID_POS, false);
                }
                break;
            case BE_BECKONING:
                if (!(monst->info.flags & MONST_IMMOBILE)
                    && caster
                    && distanceBetween(caster->loc, monst->loc) > 1) {

                    if (canSeeMonster(monst) && autoID) {
                        *autoID = true;
                    }
                    beckonMonster(monst, caster->loc.x, caster->loc.y);
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
                if (imbueInvisibility(monst, theBolt->magnitude * 15) && autoID) {
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
                        refreshDungeonCell(monst->loc);
                        if (canSeeMonster(monst)) {
                            if (autoID) {
                                *autoID = true;
                            }
                            sprintf(buf, "%s is bound to your will!", monstName);
                            message(buf, 0);
                            if (boltCatalog[BOLT_DOMINATION].backColor) {
                                flashMonster(monst, boltCatalog[BOLT_DOMINATION].backColor, 100);
                            }
                        }
                    } else if (canSeeMonster(monst)) {
                        if (autoID) {
                            *autoID = true;
                        }
                        sprintf(buf, "%s resists the bolt of domination.", monstName);
                        message(buf, 0);
                    }
                }
                break;
            case BE_NEGATION:
                negated = negate(monst);
                if (boltCatalog[BOLT_NEGATION].backColor) {
                    flashMonster(monst, boltCatalog[BOLT_NEGATION].backColor, 100);
                }
                if (negated && autoID && canSeeMonster(monst)) {
                    *autoID = true;
                }

                break;
            case BE_EMPOWERMENT:
                if (monst != &player
                    && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {

                    empowerMonster(monst);
                    createFlare(monst->loc.x, monst->loc.y, EMPOWERMENT_LIGHT);
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
                    message("the bolt hits you and you suddenly feel disoriented.", REQUIRE_ACKNOWLEDGMENT);
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
                        message(buf, 0);
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
                        monst->currentHP = (monst->currentHP + 1) / 2;
                        newMonst->currentHP = (newMonst->currentHP + 1) / 2;
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
                player.loc.x = x;
                player.loc.y = y;
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
static void detonateBolt(bolt *theBolt, creature *caster, short x, short y, boolean *autoID) {
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
                getQualifyingPathLocNear(&(monst->loc.x), &(monst->loc.y), x, y, true,
                                         T_DIVIDES_LEVEL & avoidedFlagsForMonster(&(monst->info)) & ~T_SPONTANEOUSLY_IGNITES, HAS_PLAYER,
                                         avoidedFlagsForMonster(&(monst->info)) & ~T_SPONTANEOUSLY_IGNITES, (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS), false);
                monst->bookkeepingFlags |= (MB_FOLLOWER | MB_BOUND_TO_LEADER | MB_DOES_NOT_TRACK_LEADER);
                monst->bookkeepingFlags &= ~MB_JUST_SUMMONED;
                monst->leader = &player;
                monst->creatureState = MONSTER_ALLY;
                monst->ticksUntilTurn = monst->info.attackSpeed + 1; // So they don't move before the player's next turn.
                pmapAt(monst->loc)->flags |= HAS_MONSTER;
                //refreshDungeonCell(monst->loc);
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
            if (pmap[x][y].flags & HAS_MONSTER) {
                // We're blinking onto an area already occupied by a submerged monster.
                // Make sure we don't get the shooting monster by accident.
                caster->loc = INVALID_POS; // Will be set back to the destination in a moment.
                monst = monsterAtLoc((pos){ x, y });
                findAlternativeHomeFor(monst, &x2, &y2, true);
                if (x2 >= 0) {
                    // Found an alternative location.
                    monst->loc.x = x2;
                    monst->loc.y = y2;
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
            caster->loc.x = x;
            caster->loc.y = y;
            // Always break free on blink
            disentangle(caster);
            applyInstantTileEffectsToCreature(caster);
            if (caster == &player) {
                // increase scent turn number so monsters don't sniff around at the old cell like idiots
                rogue.scentTurnNumber += 30;
                // get any items at the destination location
                if (pmapAt(player.loc)->flags & HAS_ITEM) {
                    pickUpItemAt(player.loc);
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
boolean zap(pos originLoc, pos targetLoc, bolt *theBolt, boolean hideDetails, boolean reverseBoltDir) {
    pos listOfCoordinates[MAX_BOLT_LENGTH];
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

    brogueAssert(originLoc.x != targetLoc.x || originLoc.y != targetLoc.y);
    if (originLoc.x == targetLoc.x && originLoc.y == targetLoc.y) {
        return false;
    }

    x = originLoc.x;
    y = originLoc.y;

    initialBoltLength = boltLength = 5 * theBolt->magnitude;
    if (reverseBoltDir) {
        // Beckoning (from mirrored totems and the wand) is implemented as two bolts, one going from
        // the totem/player to the target, and another from the target back to the source, to blink
        // them adjacent. However, bolt paths are asymmetric; the path from A to B isn't necessarily
        // the same as the path from B to A. If the second bolt (the blink) follows a different
        // path, it's possible for the target not to be blinked all the way back to the source
        // because it hits an obstacle (usually a monster). This results in issue #497, as well as
        // unintuitive behavior for the wand of beckoning. As a workaround, for the second bolt, we
        // compute it as if it went from the source to the target, and then reverse the list of
        // coordinates. This ensures that the two bolts will include exactly the same coordinates,
        // so the target won't get stuck on any obstacles while being beckoned.
        pos listOfCoordinatesTmp[MAX_BOLT_LENGTH];
        short numCellsTmp = getLineCoordinates(listOfCoordinatesTmp, targetLoc, originLoc, (hideDetails ? &boltCatalog[BOLT_NONE] : theBolt));
        numCells = -1;
        for (int i = 0; i < numCellsTmp; i++) {
            if (listOfCoordinatesTmp[i].x == originLoc.x && listOfCoordinatesTmp[i].y == originLoc.y) {
                numCells = i+1;
                break;
            }
        }
        brogueAssert(numCells > -1);
        for (int i = 0; i < numCells-1; i++) {
            listOfCoordinates[i] = listOfCoordinatesTmp[numCells-2-i];
        }
        listOfCoordinates[numCells-1] = targetLoc;
    } else {
        numCells = getLineCoordinates(listOfCoordinates, originLoc, targetLoc, (hideDetails ? &boltCatalog[BOLT_NONE] : theBolt));
    }
    shootingMonst = monsterAtLoc(originLoc);

    if (hideDetails) {
        boltColor = &gray;
    } else {
        boltColor = theBolt->backColor;
    }

    refreshSideBar(-1, -1, false);
    displayCombatText(); // To announce who fired the bolt while the animation plays.

    if (theBolt->boltEffect == BE_BLINKING) {
        if (cellHasTerrainFlag(listOfCoordinates[0], (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))
            || ((pmapAt(listOfCoordinates[0])->flags & (HAS_PLAYER | HAS_MONSTER))
                && !(monsterAtLoc(listOfCoordinates[0])->bookkeepingFlags & MB_SUBMERGED))) {
                // shooting blink point-blank into an obstruction does nothing.
                return false;
            }
        theBolt->foreColor = &black;
        theBolt->theChar = shootingMonst->info.displayChar;
        pmapAt(originLoc)->flags &= ~(HAS_PLAYER | HAS_MONSTER);
        refreshDungeonCell(originLoc);
        blinkDistance = theBolt->magnitude * 2 + 1;
        checkForMissingKeys(originLoc.x, originLoc.y);
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
        tunnelize(originLoc.x, originLoc.y);
    }

    backUpLighting(lights);
    boltInView = true;
    for (i=0; i<numCells; i++) {

        x = listOfCoordinates[i].x;
        y = listOfCoordinates[i].y;

        monst = monsterAtLoc(listOfCoordinates[i]);

        // Handle bolt reflection off of creatures (reflection off of terrain is handled further down).
        if (monst
            && !(theBolt->flags & BF_NEVER_REFLECTS)
            && projectileReflects(shootingMonst, monst)
            && i < MAX_BOLT_LENGTH - max(DCOLS, DROWS)) {

            if (projectileReflects(shootingMonst, monst)) { // if it scores another reflection roll, reflect at caster
                numCells = reflectBolt(originLoc.x, originLoc.y, listOfCoordinates, i, !alreadyReflected);
            } else {
                numCells = reflectBolt(-1, -1, listOfCoordinates, i, false); // otherwise reflect randomly
            }

            alreadyReflected = true;

            if (boltInView) {
                monsterName(monstName, monst, true);
                sprintf(buf, "%s deflect%s the %s",
                        monstName,
                        (monst == &player ? "" : "s"),
                        hideDetails ? "bolt" : theBolt->name);
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
                    paintLight(&boltLights[k], listOfCoordinates[i-k].x, listOfCoordinates[i-k].y, false, false);
                }
            }
        }
        boltInView = false;
        updateFieldOfViewDisplay(false, true);
        // Now draw the bolt itself.
        for (k = min(i, boltLength + 2); k >= 0; k--) {
            x2 = listOfCoordinates[i-k].x;
            y2 = listOfCoordinates[i-k].y;
            if (playerCanSeeOrSense(x2, y2)) {
                if (!fastForward) {
                    getCellAppearance((pos){ x2, y2 }, &theChar, &foreColor, &backColor);
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
                        plotCharWithColor(theChar, mapToWindow((pos){ x2, y2 }), &foreColor, &backColor);
                    } else if (boltColor) {
                        plotCharWithColor(theChar, mapToWindow((pos){ x2, y2 }), &foreColor, &backColor);
                    } else if (k == 1
                               && theBolt->foreColor
                               && theBolt->theChar) {

                        refreshDungeonCell((pos){ x2, y2 }); // Clean up the contrail so it doesn't leave a trail of characters.
                    }
                }
                if (playerCanSee(x2, y2)) {
                    // Don't want to let omniscience mode affect boltInView; causes OOS.
                    boltInView = true;
                }
            }
        }
        if (!fastForward && (boltInView || rogue.playbackOmniscience)) {
            fastForward = rogue.playbackFastForward || pauseAnimation(16, PAUSE_BEHAVIOR_DEFAULT);
        }

        if (theBolt->boltEffect == BE_BLINKING) {
            theBolt->magnitude = (blinkDistance - i) / 2 + 1;
            boltLength = theBolt->magnitude * 5;
            for (j=0; j<i; j++) {
                refreshDungeonCell(listOfCoordinates[j]);
            }
            if (i >= blinkDistance) {
                break;
            }
        }

        // Some bolts halt at the square before they hit something.
        if ((theBolt->flags & BF_HALTS_BEFORE_OBSTRUCTION)
            && i + 1 < numCells) {

            x2 = listOfCoordinates[i+1].x;
            y2 = listOfCoordinates[i+1].y;

            if (cellHasTerrainFlag((pos){ x2, y2 }, (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))) {
                break;
            }

            if (!(theBolt->flags & BF_PASSES_THRU_CREATURES)) {
                monst = monsterAtLoc(listOfCoordinates[i+1]);
                if (monst && !(monst->bookkeepingFlags & MB_SUBMERGED)) {
                    break;
                }
            }
        }

        // Tunnel if we hit a wall.
        if (cellHasTerrainFlag((pos){ x, y }, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))
            && theBolt->boltEffect == BE_TUNNELING
            && tunnelize(x, y)) {

            updateVision(true);
            backUpLighting(lights);
            autoID = true;
            theBolt->magnitude--;
            boltLength = theBolt->magnitude * 5;
            for (j=0; j<i; j++) {
                refreshDungeonCell(listOfCoordinates[j]);
            }
            if (theBolt->magnitude <= 0) {
                refreshDungeonCell(listOfCoordinates[i-1]);
                refreshDungeonCell((pos){ x, y });
                break;
            }
        }

        // Stop when we hit a wall.
        if (cellHasTerrainFlag((pos){ x, y }, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))) {
            break;
        }

        // Does the bolt bounce before hitting a wall?
        // Can happen with a cursed deflection ring or a reflective terrain target, or when shooting a tunneling bolt into an impregnable wall.
        if (i + 1 < numCells
            && !(theBolt->flags & BF_NEVER_REFLECTS)) {

            x2 = listOfCoordinates[i+1].x;
            y2 = listOfCoordinates[i+1].y;
            if (cellHasTerrainFlag((pos){ x2, y2 }, (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))
                && (projectileReflects(shootingMonst, NULL)
                    || cellHasTMFlag((pos){ x2, y2 }, TM_REFLECTS_BOLTS)
                    || (theBolt->boltEffect == BE_TUNNELING && (pmap[x2][y2].flags & IMPREGNABLE)))
                && i < MAX_BOLT_LENGTH - max(DCOLS, DROWS)) {

                sprintf(buf, "the bolt reflects off of %s", tileText(x2, y2));
                if (projectileReflects(shootingMonst, NULL)) {
                    // If it scores another reflection roll, reflect at caster, unless it's already reflected.
                    numCells = reflectBolt(originLoc.x, originLoc.y, listOfCoordinates, i, !alreadyReflected);
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
        refreshDungeonCell((pos){ x, y });
        if (i > 0) {
            refreshDungeonCell(listOfCoordinates[i-1]);
        }
    }

    if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
        monst = monsterAtLoc((pos){ x, y });
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
                            paintLight(&boltLights[k], listOfCoordinates[j-k].x, listOfCoordinates[j-k].y, false, false);
                        }
                    }
                    updateFieldOfViewDisplay(false, true);
                }

                boltInView = false;

                // beam graphic
                // k iterates from the tail tip of the visible portion of the bolt to the head
                for (k = min(j, boltLength + 2); k >= j-i; k--) {
                    if (playerCanSee(listOfCoordinates[j-k].x, listOfCoordinates[j-k].y)) {
                        if (boltColor) {
                            hiliteCell(listOfCoordinates[j-k].x, listOfCoordinates[j-k].y, boltColor, max(0, 100 - k * 100 / (boltLength)), false);
                        }
                        boltInView = true;
                    }
                }

                if (!fastForward && boltInView) {
                    fastForward = rogue.playbackFastForward || pauseAnimation(16, PAUSE_BEHAVIOR_DEFAULT);
                }
            }
        } else if (theBolt->flags & BF_DISPLAY_CHAR_ALONG_LENGTH) {
            for (j = 0; j < i; j++) {
                x2 = listOfCoordinates[j].x;
                y2 = listOfCoordinates[j].y;
                if (playerCanSeeOrSense(x2, y2)) {
                    refreshDungeonCell((pos){ x2, y2 });
                }
            }
        }
    }
    return autoID;
}

/// @brief Checks if an item is known to be of the given magic polarity
/// @param theItem the item to check
/// @param magicPolarity the magic polarity (-1 for bad, 1 for good)
/// @return true if the player knows the item is of the given magic polarity
static boolean itemMagicPolarityIsKnown(const item *theItem, int magicPolarity) {
    itemTable *table = tableForItemCategory(theItem->category);

    if ((theItem && (theItem->flags & (ITEM_MAGIC_DETECTED | ITEM_IDENTIFIED)))
        || (table && (table[theItem->kind].identified || table[theItem->kind].magicPolarityRevealed))) {

            return itemMagicPolarity(theItem) == magicPolarity;
    }
    return false;
}

/// @brief Checks if a monster is a valid auto-target when the player is using a staff/wand or throwing something.
/// @param monst The monster
/// @param theItem The staff, wand, throwing weapon, or bad/unknown potion
/// @param targetingMode The auto-target mode. Are we throwing or using that staff/wand?
/// @return True if the monster can be auto-targeted
static boolean canAutoTargetMonster(const creature *monst, const item *theItem, enum autoTargetMode targetingMode) {

    boolean throw = targetingMode == AUTOTARGET_MODE_THROW;
    boolean use = targetingMode == AUTOTARGET_MODE_USE_STAFF_OR_WAND;

    if (!monst || !theItem
        || (!throw && !use)
        || monst->depth != rogue.depthLevel
        || (monst->bookkeepingFlags & MB_IS_DYING)
        || (use && (theItem->category != STAFF) && (theItem->category != WAND))
        || (throw && (theItem->category != POTION) && (theItem->category != WEAPON))
        || (throw && (theItem->category == WEAPON) && !itemIsThrowingWeapon(theItem))
        || (throw && (theItem->category == WEAPON) && (monst->info.flags & MONST_INVULNERABLE))
        || (throw && (theItem->category == POTION) && itemMagicPolarityIsKnown(theItem, MAGIC_POLARITY_BENEVOLENT))
        || !canSeeMonster(monst)
        || !openPathBetween(player.loc, monst->loc)
        // When hallucinating but not telepathic, don't target creatures whose appearance changes.
        || (player.status[STATUS_HALLUCINATING] && !player.status[STATUS_TELEPATHIC]
            && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE)))) {
        return false;
    }

    boolean isAlly = monstersAreTeammates(&player, monst) || (monst->bookkeepingFlags & MB_CAPTIVE);
    boolean isEnemy = !isAlly;
    boolean itemKindIsKnown = tableForItemCategory(theItem->category)[theItem->kind].identified;

    if (throw) { // we are throwing a throwing weapon or a bad/unknown potion
        if (theItem->category == WEAPON) {
            if ((theItem->kind == INCENDIARY_DART && monst->status[STATUS_IMMUNE_TO_FIRE])
                || (theItem->kind != INCENDIARY_DART && (monst->info.flags & MONST_IMMUNE_TO_WEAPONS))) {
                return false;
            }
        }
        if (itemKindIsKnown && theItem->category == POTION) {
            if ((theItem->kind == POTION_INCINERATION && monst->status[STATUS_IMMUNE_TO_FIRE])
                || ((monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))
                    && (theItem->kind == POTION_CONFUSION || theItem->kind == POTION_POISON))) {
                return false;
            }
        }
        return isEnemy;
    }

    // If we got this far we're using a staff or wand
    // Don't target enemies that always reflect bolts (stone/winged guardian, non-negated mirrored totem)
    if (isEnemy && (monst->info.abilityFlags & MA_REFLECT_100)) {
        return false;
    }

    bolt theBolt = boltCatalog[tableForItemCategory(theItem->category)[theItem->kind].power];
    boolean magicPolarityIsKnownBenevolent = itemMagicPolarityIsKnown(theItem, MAGIC_POLARITY_BENEVOLENT);
    boolean magicPolarityIsKnownMalevolent = itemMagicPolarityIsKnown(theItem, MAGIC_POLARITY_MALEVOLENT);
    boolean magicPolarityIsKnown = magicPolarityIsKnownBenevolent || magicPolarityIsKnownMalevolent;

    // todo: logic consolidation with this function, specificallyValidBoltTarget, and the monster details screen
    // to show what effect (or not) the bolt will have
    if (itemKindIsKnown) {
        if ((theBolt.forbiddenMonsterFlags & monst->info.flags)
            || (isEnemy && theBolt.boltEffect == BE_DOMINATION && (wandDominate(monst) <= 0))
            || (isEnemy && theBolt.boltEffect == BE_BECKONING && (distanceBetween(player.loc, monst->loc) <= 1))
            || (isEnemy && theBolt.boltEffect == BE_DAMAGE && (theBolt.flags & BF_FIERY) && monst->status[STATUS_IMMUNE_TO_FIRE])
            || (isAlly && theBolt.boltEffect == BE_HEALING && !(monst->info.flags & MONST_REFLECT_50)
                && (monst->currentHP >= monst->info.maxHP))) {
            return false;
        } else if (isEnemy && theBolt.boltEffect == BE_NEGATION) {
            return negationWillAffectMonster(monst, true);
        } else if (isEnemy && theBolt.boltEffect == BE_TUNNELING && (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS)) {
            return true;
        } else if (((isAlly && (theBolt.flags & BF_TARGET_ALLIES)) || (isEnemy && (theBolt.flags & BF_TARGET_ENEMIES)))) {
            return true;
        }
    } else if (magicPolarityIsKnown) {
        if ((isEnemy && magicPolarityIsKnownBenevolent)
            || isAlly && magicPolarityIsKnownMalevolent) {
            return true;
        }
    } else if (isEnemy) { // both the kind and magic polarity are unknown
        return true;
    }

    return false;
}

// Relies on the sidebar entity list. If one is already selected, select the next qualifying. Otherwise, target the first qualifying.
boolean nextTargetAfter(const item *theItem,
                        pos *returnLoc,
                        pos targetLoc,
                        enum autoTargetMode targetMode,
                        boolean reverseDirection) {

    if (targetMode == AUTOTARGET_MODE_NONE) {
        return false;
    }
    short selectedIndex = 0;
    pos deduplicatedTargetList[ROWS];

    int targetCount = 0;
    for (int i=0; i<ROWS; i++) {
        if (isPosInMap(rogue.sidebarLocationList[i])) {
            if (targetCount == 0 || !posEq(deduplicatedTargetList[targetCount-1], rogue.sidebarLocationList[i])) {

                deduplicatedTargetList[targetCount] = rogue.sidebarLocationList[i];
                if (posEq(rogue.sidebarLocationList[i], targetLoc)) {
                    selectedIndex = targetCount;
                }
                targetCount++;
            }
        }
    }
    for (int i = reverseDirection ? targetCount - 1 : 0; reverseDirection ? i >= 0 : i < targetCount; reverseDirection ? i-- : i++) {
        const int n = (selectedIndex + i) % targetCount;
        pos newLoc = (pos) {deduplicatedTargetList[n].x, deduplicatedTargetList[n].y};
        if ((!posEq(newLoc, player.loc) || (posEq(newLoc, player.loc) && targetMode == AUTOTARGET_MODE_EXPLORE && itemAtLoc(player.loc)))
            && (!posEq(newLoc, targetLoc))
            && ((targetMode == AUTOTARGET_MODE_EXPLORE) || openPathBetween(player.loc, newLoc))) {

            brogueAssert(coordinatesAreInMap(newLoc.x, newLoc.y));
            brogueAssert(n >= 0 && n < targetCount);
            creature *const monst = monsterAtLoc(newLoc);
            if ((monst && canAutoTargetMonster(monst, theItem, targetMode))
                || (targetMode == AUTOTARGET_MODE_EXPLORE)) {

                *returnLoc = newLoc;
                return true;
            }
        }
    }
    return false;
}

// Returns how far it went before hitting something.
static short hiliteTrajectory(const pos coordinateList[DCOLS], short numCells, boolean eraseHiliting, const bolt *theBolt, const color *hiliteColor) {
    short x, y, i;
    creature *monst;

    boolean isFiery = theBolt && (theBolt->flags & BF_FIERY);
    boolean isTunneling = theBolt && (theBolt->boltEffect == BE_TUNNELING);
    boolean passThroughMonsters = theBolt && (theBolt->flags & BF_PASSES_THRU_CREATURES);

    for (i=0; i<numCells; i++) {
        x = coordinateList[i].x;
        y = coordinateList[i].y;
        if (eraseHiliting) {
            refreshDungeonCell((pos){ x, y });
        } else {
            hiliteCell(x, y, hiliteColor, 20, true);
        }

        if (!(pmap[x][y].flags & DISCOVERED)) {
            if (isTunneling) {
                continue;
            } else {
                break;
            }
        } else if (!passThroughMonsters && pmap[x][y].flags & (HAS_MONSTER)
                   && (playerCanSee(x, y) || player.status[STATUS_TELEPATHIC])) {
            monst = monsterAtLoc((pos){ x, y });
            if (!(monst->bookkeepingFlags & MB_SUBMERGED)
                && !monsterIsHidden(monst, &player)) {

                i++;
                break;
            }
        } else if (cellHasTerrainFlag((pos){ x, y }, T_IS_FLAMMABLE) && isFiery) {
            continue;
        } else if (isTunneling && cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY) && (pmap[x][y].flags & IMPREGNABLE)
                || !isTunneling && cellHasTerrainFlag((pos){ x, y }, (T_OBSTRUCTS_VISION | T_OBSTRUCTS_PASSABILITY))) {
            i++;
            break;
        }
    }
    return i;
}

// Event is optional. Returns true if the event should be executed by the parent function.
boolean moveCursor(boolean *targetConfirmed,
                   boolean *canceled,
                   boolean *tabKey,
                   pos *targetLoc,
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

    rogue.cursorLoc = *targetLoc;

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
            screenDisplayBuffer dbuf;
            clearDisplayBuffer(&dbuf);
            drawButtonsInState(state, &dbuf);

            const SavedDisplayBuffer rbuf = saveDisplayBuffer();
            // Update the display.
            overlayDisplayBuffer(&dbuf);

            // Get input.
            nextBrogueEvent(&theEvent, false, colorsDance, true);

            // Process the input.
            buttonInput = processButtonInput(state, NULL, &theEvent);

            if (buttonInput != -1) {
                state->buttonDepressed = state->buttonFocused = -1;
            }

            // Revert the display.
            restoreDisplayBuffer(&rbuf);

        } else { // No buttons to worry about.
            nextBrogueEvent(&theEvent, false, colorsDance, true);
        }
        restoreRNG;

        if (theEvent.eventType == MOUSE_UP || theEvent.eventType == MOUSE_ENTERED_CELL) {
            if (theEvent.param1 >= 0
                && theEvent.param1 < mapToWindowX(0)
                && theEvent.param2 >= 0
                && theEvent.param2 < ROWS - 1
                && isPosInMap(rogue.sidebarLocationList[theEvent.param2])) {

                // If the cursor is on an entity in the sidebar.
                rogue.cursorLoc = rogue.sidebarLocationList[theEvent.param2];
                sidebarHighlighted = true;
                cursorMovementCommand = true;
                refreshSideBar(rogue.cursorLoc.x, rogue.cursorLoc.y, false);
                if (theEvent.eventType == MOUSE_UP) {
                    *targetConfirmed = true;
                }
            } else if (coordinatesAreInMap(windowToMapX(theEvent.param1), windowToMapY(theEvent.param2))
                       || targetCanLeaveMap && theEvent.eventType != MOUSE_UP) {

                // If the cursor is in the map area, or is allowed to leave the map and it isn't a click.
                if (theEvent.eventType == MOUSE_UP
                    && !theEvent.shiftKey
                    && (theEvent.controlKey || (rogue.cursorLoc.x == windowToMapX(theEvent.param1) && rogue.cursorLoc.y == windowToMapY(theEvent.param2)))) {

                    *targetConfirmed = true;
                }
                rogue.cursorLoc.x = windowToMapX(theEvent.param1);
                rogue.cursorLoc.y = windowToMapY(theEvent.param2);
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
                    if (keysMoveCursor && rogue.cursorLoc.x > 0) {
                        rogue.cursorLoc.x -= moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case RIGHT_ARROW:
                case RIGHT_KEY:
                case NUMPAD_6:
                    if (keysMoveCursor && rogue.cursorLoc.x < DCOLS - 1) {
                        rogue.cursorLoc.x += moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case UP_ARROW:
                case UP_KEY:
                case NUMPAD_8:
                    if (keysMoveCursor && rogue.cursorLoc.y > 0) {
                        rogue.cursorLoc.y -= moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case DOWN_ARROW:
                case DOWN_KEY:
                case NUMPAD_2:
                    if (keysMoveCursor && rogue.cursorLoc.y < DROWS - 1) {
                        rogue.cursorLoc.y += moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case UPLEFT_KEY:
                case NUMPAD_7:
                    if (keysMoveCursor && rogue.cursorLoc.x > 0 && rogue.cursorLoc.y > 0) {
                        rogue.cursorLoc.x -= moveIncrement;
                        rogue.cursorLoc.y -= moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case UPRIGHT_KEY:
                case NUMPAD_9:
                    if (keysMoveCursor && rogue.cursorLoc.x < DCOLS - 1 && rogue.cursorLoc.y > 0) {
                        rogue.cursorLoc.x += moveIncrement;
                        rogue.cursorLoc.y -= moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case DOWNLEFT_KEY:
                case NUMPAD_1:
                    if (keysMoveCursor && rogue.cursorLoc.x > 0 && rogue.cursorLoc.y < DROWS - 1) {
                        rogue.cursorLoc.x -= moveIncrement;
                        rogue.cursorLoc.y += moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case DOWNRIGHT_KEY:
                case NUMPAD_3:
                    if (keysMoveCursor && rogue.cursorLoc.x < DCOLS - 1 && rogue.cursorLoc.y < DROWS - 1) {
                        rogue.cursorLoc.x += moveIncrement;
                        rogue.cursorLoc.y += moveIncrement;
                    }
                    cursorMovementCommand = movementKeystroke = keysMoveCursor;
                    break;
                case TAB_KEY:
                case SHIFT_TAB_KEY:
                case NUMPAD_0:
                    *tabKey = true;
                    break;
                case RETURN_KEY:
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
            && (!(pmapAt(rogue.cursorLoc)->flags & (HAS_PLAYER | HAS_MONSTER))
                || !canSeeMonster(monsterAtLoc(rogue.cursorLoc)))
            && (!(pmapAt(rogue.cursorLoc)->flags & HAS_ITEM) || !playerCanSeeOrSense(rogue.cursorLoc.x, rogue.cursorLoc.y))
            && (!cellHasTMFlag(rogue.cursorLoc, TM_LIST_IN_SIDEBAR) || !playerCanSeeOrSense(rogue.cursorLoc.x, rogue.cursorLoc.y))) {

            // The sidebar is highlighted but the cursor is not on a visible item, monster or terrain. Un-highlight the sidebar.
            refreshSideBar(-1, -1, false);
            sidebarHighlighted = false;
        }

        if (targetCanLeaveMap && !movementKeystroke) {
            // permit it to leave the map by up to 1 space in any direction if mouse controlled.
            rogue.cursorLoc.x = clamp(rogue.cursorLoc.x, -1, DCOLS);
            rogue.cursorLoc.y = clamp(rogue.cursorLoc.y, -1, DROWS);
        } else {
            rogue.cursorLoc.x = clamp(rogue.cursorLoc.x, 0, DCOLS - 1);
            rogue.cursorLoc.y = clamp(rogue.cursorLoc.y, 0, DROWS - 1);
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

    *targetLoc = rogue.cursorLoc;

    return !cursorMovementCommand;
}

static pos pullMouseClickDuringPlayback(void) {
    rogueEvent theEvent;

    brogueAssert(rogue.playbackMode);
    nextBrogueEvent(&theEvent, false, false, false);
    return (pos){
        .x = windowToMapX(theEvent.param1),
        .y = windowToMapY(theEvent.param2)
    };
}

/// @brief Allows the player to interactively choose a target location for actions that require a straight line from
/// the player to the target (e.g. throwing an item or using a staff/wand). The best path to the target is determined
/// and highlighted. If possible, an inital target is chosen automatically. The player can cycle through additional
/// targets (if any) with the tab key. Alternatively, the player can manually choose a target.
/// @param returnLoc The location of the chosen target, if any.
/// @param maxDistance The maximum throwing/blinking distance. Used to provide a visual cue to the player.
/// @param targetMode Determines how targets are chosen based on the action (e.g. throw, use staff/wand).
/// @param theItem The staff, wand, or thrown item, if any.
/// @return True if the player selected a target
boolean chooseTarget(pos *returnLoc,
                     short maxDistance,
                     enum autoTargetMode targetMode,
                     const item *theItem) {
    short numCells, i, distance;
    pos coordinates[DCOLS];
    creature *monst;
    boolean canceled, targetConfirmed, tabKey, cursorInTrajectory, focusedOnSomething = false;
    rogueEvent event = {0};
    short oldRNG;
    boolean stopAtTarget = (targetMode == AUTOTARGET_MODE_THROW);
    color trajColor;
    bolt theBolt;

    // choose the bolt and color to use for highlighting the path to the target
    if (theItem && (targetMode == AUTOTARGET_MODE_USE_STAFF_OR_WAND)
        && ((theItem->category == STAFF) || (theItem->category == WAND))) {

        if (tableForItemCategory(theItem->category)[theItem->kind].identified) {
            theBolt = boltCatalog[boltForItem(theItem)];
            trajColor = (theBolt.backColor == NULL) ? red : *theBolt.backColor;
        } else {
            trajColor = gray;
        }
    } else if (theItem && (targetMode == AUTOTARGET_MODE_THROW)) {
        trajColor = red;
    } else {
        trajColor = white;
        theBolt = boltCatalog[BOLT_NONE];
    }

    normColor(&trajColor, 100, 10);

    if (rogue.playbackMode) {
        // In playback, pull the next event (a mouseclick) and use that location as the target.
        *returnLoc = pullMouseClickDuringPlayback();
        rogue.cursorLoc = INVALID_POS;
        return true;
    }

    oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;

    pos originLoc = player.loc;
    pos oldTargetLoc = player.loc;
    pos targetLoc = player.loc;
    pos newLoc;

    if (theItem && ((targetMode == AUTOTARGET_MODE_USE_STAFF_OR_WAND) || (targetMode == AUTOTARGET_MODE_THROW))) {
        if (canAutoTargetMonster(rogue.lastTarget, theItem, targetMode)) {
            monst = rogue.lastTarget;
        } else {
            if (nextTargetAfter(theItem, &newLoc, targetLoc, targetMode, false)) {
                targetLoc = newLoc;
            }
            monst = monsterAtLoc(targetLoc);
        }
        if (monst) {
            targetLoc = monst->loc;
            refreshSideBar(monst->loc.x, monst->loc.y, false);
            focusedOnSomething = true;
        }
    }

    numCells = getLineCoordinates(coordinates, originLoc, targetLoc, &theBolt);
    if (maxDistance > 0) {
        numCells = min(numCells, maxDistance);
    }
    if (stopAtTarget) {
        numCells = min(numCells, distanceBetween(player.loc, targetLoc));
    }

    targetConfirmed = canceled = tabKey = false;

    do {
        printLocationDescription(targetLoc.x, targetLoc.y);

        if (canceled) {
            refreshDungeonCell(oldTargetLoc);
            hiliteTrajectory(coordinates, numCells, true, &theBolt, &trajColor);
            confirmMessages();
            rogue.cursorLoc = INVALID_POS;
            restoreRNG;
            return false;
        }

        if (tabKey) {
            if (nextTargetAfter(theItem, &newLoc, targetLoc, targetMode, event.shiftKey)) {
                targetLoc = newLoc;
            }
        }

        monst = monsterAtLoc(targetLoc);
        if (monst != NULL && monst != &player && canSeeMonster(monst)) {
            focusedOnSomething = true;
        } else if (playerCanSeeOrSense(targetLoc.x, targetLoc.y)
                   && (pmapAt(targetLoc)->flags & HAS_ITEM) || cellHasTMFlag(targetLoc, TM_LIST_IN_SIDEBAR)) {
            focusedOnSomething = true;
        } else if (focusedOnSomething) {
            refreshSideBar(-1, -1, false);
            focusedOnSomething = false;
        }
        if (focusedOnSomething) {
            refreshSideBar(targetLoc.x, targetLoc.y, false);
        }

        refreshDungeonCell(oldTargetLoc);
        hiliteTrajectory(coordinates, numCells, true, &theBolt, &trajColor);

        if (!targetConfirmed) {
            numCells = getLineCoordinates(coordinates, originLoc, targetLoc, &theBolt);
            if (maxDistance > 0) {
                numCells = min(numCells, maxDistance);
            }

            if (stopAtTarget) {
                numCells = min(numCells, distanceBetween(player.loc, targetLoc));
            }
            distance = hiliteTrajectory(coordinates, numCells, false, &theBolt, &trajColor);
            cursorInTrajectory = false;
            for (i=0; i<distance; i++) {
                if (coordinates[i].x == targetLoc.x && coordinates[i].y == targetLoc.y) {
                    cursorInTrajectory = true;
                    break;
                }
            }
            hiliteCell(targetLoc.x, targetLoc.y, &white, (cursorInTrajectory ? 100 : 35), true);
        }

        oldTargetLoc = targetLoc;
        moveCursor(&targetConfirmed, &canceled, &tabKey, &targetLoc, &event, NULL, false, true, false);
        if (event.eventType == RIGHT_MOUSE_UP) { // Right mouse cancels.
            canceled = true;
        }
    } while (!targetConfirmed);
    if (maxDistance > 0) {
        numCells = min(numCells, maxDistance);
    }
    hiliteTrajectory(coordinates, numCells, true, &theBolt, &trajColor);
    refreshDungeonCell(oldTargetLoc);

    if (posEq(originLoc, targetLoc)) {
        confirmMessages();
        restoreRNG;
        rogue.cursorLoc = INVALID_POS;
        return false;
    }

    monst = monsterAtLoc(targetLoc);
    if (monst && monst != &player && canSeeMonster(monst)) {
        rogue.lastTarget = monst;
    }

    *returnLoc = targetLoc;
    restoreRNG;
    rogue.cursorLoc = INVALID_POS;
    return true;
}

// Returns the number of total, good, or bad item kinds for the given item category. To ignore
// polarity and get the total kinds, pass polarityConstraint = 0.
int itemKindCount(enum itemCategory category, int polarityConstraint) {
    int kindCount = -1, totalKinds, goodKinds;

    switch (category) {
        case SCROLL:
            totalKinds = gameConst->numberScrollKinds;
            goodKinds = gameConst->numberGoodScrollKinds;
            break;
        case POTION:
            totalKinds = gameConst->numberPotionKinds;
            goodKinds = gameConst->numberGoodPotionKinds;
            break;
        case WAND:
            totalKinds = gameConst->numberWandKinds;
            goodKinds = gameConst->numberGoodWandKinds;
            break;
        case STAFF:
            totalKinds = NUMBER_STAFF_KINDS;
            goodKinds = NUMBER_GOOD_STAFF_KINDS;
            break;
        case FOOD:
            totalKinds = NUMBER_FOOD_KINDS;
            goodKinds = 0;
            break;
        case WEAPON:
            totalKinds = NUMBER_WEAPON_KINDS;
            goodKinds = 0;
            break;
        case ARMOR:
            totalKinds = NUMBER_ARMOR_KINDS;
            goodKinds = 0;
            break;
        case RING:
            totalKinds = NUMBER_RING_KINDS;
            goodKinds = NUMBER_RING_KINDS;
            break;
        case CHARM:
            totalKinds = gameConst->numberCharmKinds;
            goodKinds = gameConst->numberCharmKinds;
            break;
        default:
            totalKinds = 0;
            goodKinds = 0;
            break;
    }

    switch (polarityConstraint) {
        case MAGIC_POLARITY_MALEVOLENT:
            if (goodKinds == 0) {
                kindCount = 0;
            } else {
                kindCount = totalKinds - goodKinds;
            }
            break;
        case MAGIC_POLARITY_BENEVOLENT:
            kindCount = goodKinds;
            break;
        default:
            kindCount = totalKinds;
            break;
    }
    return kindCount;
}

// Gets the final unidentified item kind for the given category and magic polarity, if possible. Returns -1 if
// the final item kind cannot be determined.
static int tryGetLastUnidentifiedItemKind(enum itemCategory category, int polarityConstraint) {
    int lastItemKind = -1;
    int totalItemKinds = itemKindCount(category, 0);
    itemTable *theItemTable = tableForItemCategory(category);

    if (theItemTable && (totalItemKinds > 0)) {
        for (int i = 0; i < totalItemKinds; i++) {
            if (!(theItemTable[i].identified)
                && (theItemTable[i].magicPolarity == polarityConstraint || polarityConstraint == MAGIC_POLARITY_ANY)) {
                if (lastItemKind != -1) {
                    return -1; // At least two unidentified items remain.
                }
                lastItemKind = i;
            }
        }
    }
    return lastItemKind;
}

// Counts the number of items where the magic polarity has been revealed for the given item category
// and magic polarity.
static int magicPolarityRevealedItemKindCount(enum itemCategory category, int polarityConstraint) {
    int kindCount = -1;
    int totalItemKinds = itemKindCount(category, 0);
    itemTable *theItemTable = tableForItemCategory(category);

    if (theItemTable && (totalItemKinds > 0) && polarityConstraint) {
        kindCount = 0;
        for (int i = 0; i < totalItemKinds; i++) {
            if (theItemTable[i].magicPolarity == polarityConstraint &&
                    (theItemTable[i].identified || theItemTable[i].magicPolarityRevealed)) {
                kindCount += 1;
            }
        }
    }
    return kindCount;
}

// Try to identify the last item kind in a given category.
// The category must be in HAS_INTRINSIC_POLARITY.
// polarityConstraint is either 0 (ignore polarity) or +1/-1.
//
// We can identify the last unidentified item kind...
// 1. Of a category
// 2. Of a given polarity within a category if either...
//  A. Its polarity is known
//  B. All items of the opposite polarity are either identified or their polarity is known
static void tryIdentifyLastItemKind(enum itemCategory category, int polarityConstraint) {
    itemTable *theItemTable = tableForItemCategory(category);
    int lastItemKind = tryGetLastUnidentifiedItemKind(category, polarityConstraint);
    int oppositeCount, oppositeRevealedCount;

    if (lastItemKind >= 0) {

        if (polarityConstraint == MAGIC_POLARITY_ANY) {
            theItemTable[lastItemKind].identified = true;
        } else {
            int oppositeMagicPolarity = polarityConstraint * -1;
            oppositeRevealedCount = magicPolarityRevealedItemKindCount(category, oppositeMagicPolarity);
            oppositeCount = itemKindCount(category, oppositeMagicPolarity);
            if (theItemTable[lastItemKind].magicPolarityRevealed || oppositeRevealedCount == oppositeCount) {
                theItemTable[lastItemKind].identified = true;
            }
        }
    }
}

// Try to identify the last item of the given category or all categories. This function
// operates on flavored categories only. The base type of non-flavored categories are
// already identified (e.g. weapons, armor, charms, etc.)
static void tryIdentifyLastItemKinds(enum itemCategory category) {
    enum itemCategory loopCategory;
    int categoryCount = 1;

    if (category == HAS_INTRINSIC_POLARITY) {
        categoryCount = NUMBER_ITEM_CATEGORIES;
    }

    for (int i=0; i<categoryCount; i++) {
        loopCategory = categoryCount == 1 ? category : Fl(i);
        if (category & HAS_INTRINSIC_POLARITY & loopCategory) {
            tryIdentifyLastItemKind(loopCategory, MAGIC_POLARITY_BENEVOLENT);
            tryIdentifyLastItemKind(loopCategory, MAGIC_POLARITY_MALEVOLENT);
        }
    }
}

void identifyItemKind(item *theItem) {
    itemTable *theTable;
    short tableCount;

    theTable = tableForItemCategory(theItem->category);
    if (theTable) {
        theItem->flags &= ~ITEM_KIND_AUTO_ID;

        tableCount = 0;

        switch (theItem->category) {
            case SCROLL:
                tableCount = gameConst->numberScrollKinds;
                break;
            case POTION:
                tableCount = gameConst->numberPotionKinds;
                break;
            case WAND:
                tableCount = gameConst->numberWandKinds;
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
            tryIdentifyLastItemKinds(theItem->category);
        }
    }
}

void autoIdentify(item *theItem) {
    short quantityBackup;
    char buf[COLS * 3], oldName[COLS * 3], newName[COLS * 3];

    if (tableForItemCategory(theItem->category)
        && !tableForItemCategory(theItem->category)[theItem->kind].identified) {

        identifyItemKind(theItem);
        quantityBackup = theItem->quantity;
        theItem->quantity = 1;
        itemName(theItem, newName, false, true, NULL);
        theItem->quantity = quantityBackup;
        sprintf(buf, "(It must %s %s.)",
                ((theItem->category & (POTION | SCROLL)) ? "have been" : "be"),
                newName);
        messageWithColor(buf, &itemMessageColor, 0);
    }

    if ((theItem->category & (WEAPON | ARMOR))
        && (theItem->flags & ITEM_RUNIC)
        && !(theItem->flags & ITEM_RUNIC_IDENTIFIED)) {

        itemName(theItem, oldName, false, false, NULL);
        theItem->flags |= (ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
        itemName(theItem, newName, true, true, NULL);
        sprintf(buf, "(Your %s must be %s.)", oldName, newName);
        messageWithColor(buf, &itemMessageColor, 0);
    }
}

// returns whether the item disappeared
static boolean hitMonsterWithProjectileWeapon(creature *thrower, creature *monst, item *theItem) {
    char buf[DCOLS], theItemName[DCOLS], targetName[DCOLS], armorRunicString[DCOLS];
    boolean thrownWeaponHit;
    item *equippedWeapon;
    short damage;

    if (!(theItem->category & WEAPON)) {
        return false;
    }
    // Check paladin feat before creatureState is changed
    if (thrower == &player && !(monst->info.flags & (MONST_IMMUNE_TO_WEAPONS))) {
        handlePaladinFeat(monst);
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
        equipItem(theItem, true, NULL);
        thrownWeaponHit = attackHit(&player, monst);
        if (equippedWeapon) {
            equipItem(equippedWeapon, true, NULL);
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
            messageWithColor(buf, messageColorFromVictim(monst), 0);
            killCreature(monst, false);
        } else {
            sprintf(buf, "the %s hit %s.", theItemName, targetName);
            if (theItem->flags & ITEM_RUNIC) {
                magicWeaponHit(monst, theItem, false);
            }
            messageWithColor(buf, messageColorFromVictim(monst), 0);
        }
        moralAttack(thrower, monst);
        splitMonster(monst, thrower);
        if (armorRunicString[0]) {
            message(armorRunicString, 0);
        }
        return true;
    } else {
        theItem->flags &= ~ITEM_PLAYER_AVOIDS; // Don't avoid thrown weapons that missed.
        sprintf(buf, "the %s missed %s.", theItemName, targetName);
        message(buf, 0);
        return false;
    }
}

static void throwItem(item *theItem, creature *thrower, pos targetLoc, short maxDistance) {
    short i, numCells;
    creature *monst = NULL;
    char buf[COLS*3], buf2[COLS*3], buf3[COLS*3];
    enum displayGlyph displayChar;
    color foreColor, backColor, multColor;
    boolean hitSomethingSolid = false, fastForward = false;
    enum dungeonLayers layer;

    theItem->flags |= ITEM_PLAYER_AVOIDS; // Avoid thrown items, unless it's a weapon that misses a monster.

    pos originLoc = thrower->loc;
    short x = originLoc.x;
    short y = originLoc.y;

    // Using BOLT_NONE for throws because all flags are off, which means we'll try to avoid all obstacles in front of the target
    pos listOfCoordinates[MAX_BOLT_LENGTH];
    numCells = getLineCoordinates(listOfCoordinates, originLoc, targetLoc, &boltCatalog[BOLT_NONE]);

    thrower->ticksUntilTurn = thrower->attackSpeed;

    if (thrower != &player
        && (pmapAt(originLoc)->flags & IN_FIELD_OF_VIEW)) {

        monsterName(buf2, thrower, true);
        itemName(theItem, buf3, false, true, NULL);
        sprintf(buf, "%s hurls %s.", buf2, buf3);
        message(buf, 0);
    }

    for (i=0; i<numCells && i < maxDistance; i++) {
        x = listOfCoordinates[i].x;
        y = listOfCoordinates[i].y;

        if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
            monst = monsterAtLoc((pos){ x, y });
            if (!(monst->bookkeepingFlags & MB_SUBMERGED)) {
//          if (projectileReflects(thrower, monst) && i < DCOLS*2) {
//              if (projectileReflects(thrower, monst)) { // if it scores another reflection roll, reflect at caster
//                  numCells = reflectBolt(originLoc[0], originLoc.y, listOfCoordinates, i, true);
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
        if (cellHasTerrainFlag((pos){ x, y }, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))) {
            if ((theItem->category & WEAPON)
                && (theItem->kind == INCENDIARY_DART)
                && (cellHasTerrainFlag((pos){ x, y }, T_IS_FLAMMABLE) || (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)))) {
                // Incendiary darts thrown at flammable obstructions (foliage, wooden barricades, doors) will hit the obstruction
                // instead of bursting a cell earlier.
            } else if (cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)
                       && cellHasTMFlag((pos){ x, y }, TM_PROMOTES_ON_PLAYER_ENTRY)
                       && tileCatalog[pmap[x][y].layers[layerWithTMFlag(x, y, TM_PROMOTES_ON_PLAYER_ENTRY)]].flags & T_OBSTRUCTS_PASSABILITY) {
                layer = layerWithTMFlag(x, y, TM_PROMOTES_ON_PLAYER_ENTRY);
                if (tileCatalog[pmap[x][y].layers[layer]].flags & T_OBSTRUCTS_PASSABILITY) {
                    message(tileCatalog[pmap[x][y].layers[layer]].flavorText, 0);
                    promoteTile(x, y, layer, false);
                }
            } else {
                i--;
                if (i >= 0) {
                    x = listOfCoordinates[i].x;
                    y = listOfCoordinates[i].y;
                } else { // it was aimed point-blank into an obstruction
                    x = thrower->loc.x;
                    y = thrower->loc.y;
                }
            }
            hitSomethingSolid = true;
            break;
        }

        if (playerCanSee(x, y)) { // show the graphic
            getCellAppearance((pos){ x, y }, &displayChar, &foreColor, &backColor);
            foreColor = *(theItem->foreColor);
            if (playerCanDirectlySee(x, y)) {
                colorMultiplierFromDungeonLight(x, y, &multColor);
                applyColorMultiplier(&foreColor, &multColor);
            } else { // clairvoyant visible
                applyColorMultiplier(&foreColor, &clairvoyanceColor);
            }
            plotCharWithColor(theItem->displayChar, mapToWindow((pos){ x, y }), &foreColor, &backColor);

            if (!fastForward) {
                fastForward = rogue.playbackFastForward || pauseAnimation(25, PAUSE_BEHAVIOR_DEFAULT);
            }

            refreshDungeonCell((pos){ x, y });
        }

        if (x == targetLoc.x && y == targetLoc.y) { // reached its target
            break;
        }
    }

    if ((theItem->category & POTION) && (hitSomethingSolid || !cellHasTerrainFlag((pos){ x, y }, T_AUTO_DESCENT))) {
        if (theItem->kind == POTION_CONFUSION || theItem->kind == POTION_POISON
            || theItem->kind == POTION_PARALYSIS || theItem->kind == POTION_INCINERATION
            || theItem->kind == POTION_DARKNESS || theItem->kind == POTION_LICHEN
            || theItem->kind == POTION_DESCENT) {
            switch (theItem->kind) {
                case POTION_POISON:
                    strcpy(buf, "the flask shatters and a deadly purple cloud billows out!");
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_POISON_GAS_CLOUD_POTION], true, false);
                    message(buf, 0);
                    break;
                case POTION_CONFUSION:
                    strcpy(buf, "the flask shatters and a multi-hued cloud billows out!");
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_CONFUSION_GAS_CLOUD_POTION], true, false);
                    message(buf, 0);
                    break;
                case POTION_PARALYSIS:
                    strcpy(buf, "the flask shatters and a cloud of pink gas billows out!");
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_PARALYSIS_GAS_CLOUD_POTION], true, false);
                    message(buf, 0);
                    break;
                case POTION_INCINERATION:
                    strcpy(buf, "the flask shatters and its contents burst violently into flame!");
                    message(buf, 0);
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_INCINERATION_POTION], true, false);
                    break;
                case POTION_DARKNESS:
                    strcpy(buf, "the flask shatters and the lights in the area start fading.");
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_DARKNESS_POTION], true, false);
                    message(buf, 0);
                    break;
                case POTION_DESCENT:
                    strcpy(buf, "as the flask shatters, the ground vanishes!");
                    message(buf, 0);
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_HOLE_POTION], true, false);
                    break;
                case POTION_LICHEN:
                    strcpy(buf, "the flask shatters and deadly spores spill out!");
                    message(buf, 0);
                    spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_LICHEN_PLANTED], true, false);
                    break;
            }

            autoIdentify(theItem);

            refreshDungeonCell((pos){ x, y });

            //if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
            //  monst = monsterAtLoc((pos){ x, y });
            //  applyInstantTileEffectsToCreature(monst);
            //}
        } else {
            if (cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)) {
                strcpy(buf2, "against");
            } else if (tileCatalog[pmap[x][y].layers[highestPriorityLayer(x, y, false)]].mechFlags & TM_STAND_IN_TILE) {
                strcpy(buf2, "into");
            } else {
                strcpy(buf2, "on");
            }
            sprintf(buf, "the flask shatters and %s liquid splashes harmlessly %s %s.",
                    potionTable[theItem->kind].flavor, buf2, tileText(x, y));
            message(buf, 0);
            // hallucination is the only malevolent potion that splashes harmlessly when thrown
            if (theItem->kind == POTION_HALLUCINATION) {
                if (theItem->flags & ITEM_MAGIC_DETECTED
                    || (magicPolarityRevealedItemKindCount(theItem->category, 1) == gameConst->numberGoodPotionKinds)) {
                    autoIdentify(theItem);
                }
            }
        }
        deleteItem(theItem);
        return; // potions disappear when they break
    }
    if ((theItem->category & WEAPON) && theItem->kind == INCENDIARY_DART) {
        spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DF_DART_EXPLOSION], true, false);
        if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
            exposeCreatureToFire(monsterAtLoc((pos){ x, y }));
        }
        deleteItem(theItem);
        return;
    }
    pos dropLoc;
    getQualifyingLocNear(&dropLoc, (pos){ x, y }, true, 0, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_PASSABILITY), (HAS_ITEM), false, false);
    placeItemAt(theItem, dropLoc);
    refreshDungeonCell(dropLoc);
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
    short maxDistance, quantity;

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
            messageWithColor(buf, &itemMessageColor, 0);
            return;
        }
    }

    //
    // Ask location to throw
    //
    sprintf(buf, "Throw %s %s where? (<hjklyubn>, mouse, or <tab>)",
            (theItem->quantity > 1 ? "a" : "your"),
            theName);
    temporaryMessage(buf, REFRESH_SIDEBAR);
    maxDistance = (12 + 2 * max(rogue.strength - player.weaknessAmount - 12, 2));

    pos zapTarget;
    if (autoThrow && canAutoTargetMonster(rogue.lastTarget, theItem, AUTOTARGET_MODE_THROW)) {
        zapTarget = rogue.lastTarget->loc;
    } else if (!chooseTarget(&zapTarget, maxDistance, AUTOTARGET_MODE_THROW, theItem)) {
        // player doesn't choose a target? return
        return;
    }

    if ((theItem->flags & ITEM_EQUIPPED) && theItem->quantity <= 1) {
        unequipItem(theItem, false);
    }
    command[2] = '\0';
    recordKeystrokeSequence(command);
    recordMouseClick(mapToWindowX(zapTarget.x), mapToWindowY(zapTarget.y), true, false);

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
        if (rogue.swappedIn == theItem || rogue.swappedOut == theItem) {
            rogue.swappedIn = NULL;
            rogue.swappedOut = NULL;
        }
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
    temporaryMessage("New letter? (a-z)", 0);
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
                messageWithColor(buf, &itemMessageColor, 0);
            }
            theItem->inventoryLetter = newLabel;
            itemName(theItem, theName, true, true, NULL);
            sprintf(buf, "%selabeled %s as (%c).", oldItem ? " r" : "R", theName, newLabel);
            messageWithColor(buf, &itemMessageColor, 0);
        } else {
            itemName(theItem, theName, true, true, NULL);
            sprintf(buf, "%s %s already labeled (%c).",
                    theName,
                    theItem->quantity == 1 ? "is" : "are",
                    theItem->inventoryLetter);
            messageWithColor(buf, &itemMessageColor, 0);
        }
    }
}

// If the most recently equipped item caused another item to be unequiped, is
// uncursed, and both haven't left the inventory since, swap them back.
void swapLastEquipment() {
    item *theItem;
    unsigned char command[10];

    if (rogue.swappedIn == NULL || rogue.swappedOut == NULL) {
        confirmMessages();
        message("You have nothing to swap.", 0);
        return;
    }

    if (!equipItem(rogue.swappedOut, false, rogue.swappedIn)) {
        // Cursed
        return;
    }

    command[0] = SWAP_KEY;
    command[1] = '\0';
    recordKeystrokeSequence(command);

    theItem = rogue.swappedIn;
    rogue.swappedIn = rogue.swappedOut;
    rogue.swappedOut = theItem;

    playerTurnEnded();
}

// If the blink trajectory lands in lava based on the player's knowledge, abort.
// If the blink trajectory might land in lava based on the player's knowledge,
// prompt for confirmation.
static boolean playerCancelsBlinking(const pos originLoc, const pos targetLoc, const short maxDistance) {
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

    pos impactLoc;
    getImpactLoc(&impactLoc, originLoc, targetLoc, maxDistance > 0 ? maxDistance : DCOLS, true, &boltCatalog[BOLT_BLINKING]);
    getLocationFlags(impactLoc.x, impactLoc.y, &tFlags, &tmFlags, NULL, true);
    if (maxDistance > 0) {
        if ((pmapAt(impactLoc)->flags & DISCOVERED)
            && (tFlags & T_LAVA_INSTA_DEATH)
            && !(tFlags & (T_ENTANGLES | T_AUTO_DESCENT))
            && !(tmFlags & TM_EXTINGUISHES_FIRE)) {

            certainDeath = possibleDeath = true;
        }
    } else {
        certainDeath = true;
        pos coordinates[DCOLS];
        numCells = getLineCoordinates(coordinates, originLoc, targetLoc, &boltCatalog[BOLT_BLINKING]);
        for (i = 0; i < numCells; i++) {
            x = coordinates[i].x;
            y = coordinates[i].y;
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
            if (x == impactLoc.x && y == impactLoc.y) {
                break;
            }
        }
    }
    if (possibleDeath && certainDeath) {
        message("that would be certain death!", 0);
        return true;
    }
    if (possibleDeath
        && !confirm("Blink across lava with unknown range?", false)) {
        return true;
    }
    return false;
}

/// @brief Records the keystroke sequence when a player applies an item
/// @param theItem the item that was used
static void recordApplyItemCommand(item *theItem) {
    if (!theItem) {
        return;
    }

    unsigned char command[3] = {'\0'};

    command[0] = APPLY_KEY;
    command[1] = theItem->inventoryLetter;
    recordKeystrokeSequence(command);
}

static boolean useStaffOrWand(item *theItem) {
    char buf[COLS], buf2[COLS];
    short maxDistance;
    boolean autoID, confirmedTarget;
    bolt theBolt;

    if (theItem->charges <= 0 && (theItem->flags & ITEM_IDENTIFIED)) {
        itemName(theItem, buf2, false, false, NULL);
        sprintf(buf, "Your %s has no charges.", buf2);
        messageWithColor(buf, &itemMessageColor, 0);
        return false;
    }
    temporaryMessage("Direction? (<hjklyubn>, mouse, or <tab>; <return> to confirm)", REFRESH_SIDEBAR);
    itemName(theItem, buf2, false, false, NULL);
    sprintf(buf, "Zapping your %s:", buf2);
    printString(buf, mapToWindowX(0), 1, &itemMessageColor, &black, NULL);

    theBolt = boltCatalog[tableForItemCategory(theItem->category)[theItem->kind].power];
    if (theItem->category == STAFF) {
        theBolt.magnitude = theItem->enchant1;
    }

    if ((theItem->category & STAFF) && theItem->kind == STAFF_BLINKING
        && theItem->flags & (ITEM_IDENTIFIED | ITEM_MAX_CHARGES_KNOWN)) {

        maxDistance = staffBlinkDistance(netEnchant(theItem));
    } else {
        maxDistance = -1;
    }

    boolean boltKnown = tableForItemCategory(theItem->category)[theItem->kind].identified;
    pos originLoc = player.loc;
    pos zapTarget;
    confirmedTarget = chooseTarget(&zapTarget, maxDistance, AUTOTARGET_MODE_USE_STAFF_OR_WAND, theItem);
    if (confirmedTarget
        && boltKnown
        && theBolt.boltEffect == BE_BLINKING
        && playerCancelsBlinking(originLoc, zapTarget, maxDistance)) {

        confirmedTarget = false;
    }
    if (confirmedTarget) {

        recordApplyItemCommand(theItem);
        recordMouseClick(mapToWindowX(zapTarget.x), mapToWindowY(zapTarget.y), true, false);
        confirmMessages();

        rogue.featRecord[FEAT_PURE_WARRIOR] = false;

        if (theItem->charges > 0) {
            autoID = zap(originLoc, zapTarget,
                         &theBolt,
                         !boltKnown,   // hide bolt details
                         false);
            if (autoID) {
                if (!tableForItemCategory(theItem->category)[theItem->kind].identified) {
                    itemName(theItem, buf2, false, false, NULL);
                    sprintf(buf, "(Your %s must be ", buf2);
                    identifyItemKind(theItem);
                    itemName(theItem, buf2, false, true, NULL);
                    strcat(buf, buf2);
                    strcat(buf, ".)");
                    messageWithColor(buf, &itemMessageColor, 0);
                }
            }
        } else {
            itemName(theItem, buf2, false, false, NULL);
            if (theItem->category == STAFF) {
                sprintf(buf, "Your %s fizzles; it must be out of charges for now.", buf2);
            } else {
                sprintf(buf, "Your %s fizzles; it must be depleted.", buf2);
            }
            messageWithColor(buf, &itemMessageColor, 0);
            theItem->flags |= ITEM_MAX_CHARGES_KNOWN;
            playerTurnEnded();
            return false;
        }
    } else {
        return false;
    }

    if (theItem->category & STAFF) {
        theItem->lastUsed[2] = theItem->lastUsed[1];
        theItem->lastUsed[1] = theItem->lastUsed[0];
        theItem->lastUsed[0] = rogue.absoluteTurnNumber;
    }

    if (theItem->charges > 0) {
        theItem->charges--;
        if (theItem->category == WAND) {
            theItem->enchant2++; // keeps track of how many times the wand has been discharged for the player's convenience
        }
    }

    return true;
}

static void summonGuardian(item *theItem) {
    short x = player.loc.x, y = player.loc.y;
    creature *monst;

    monst = generateMonster(MK_CHARM_GUARDIAN, false, false);
    getQualifyingPathLocNear(&(monst->loc.x), &(monst->loc.y), x, y, true,
                             T_DIVIDES_LEVEL & avoidedFlagsForMonster(&(monst->info)) & ~T_SPONTANEOUSLY_IGNITES, HAS_PLAYER,
                             avoidedFlagsForMonster(&(monst->info)) & ~T_SPONTANEOUSLY_IGNITES, (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS), false);
    monst->bookkeepingFlags |= (MB_FOLLOWER | MB_BOUND_TO_LEADER | MB_DOES_NOT_TRACK_LEADER);
    monst->bookkeepingFlags &= ~MB_JUST_SUMMONED;
    monst->leader = &player;
    monst->creatureState = MONSTER_ALLY;
    monst->ticksUntilTurn = monst->info.attackSpeed + 1; // So they don't move before the player's next turn.
    monst->status[STATUS_LIFESPAN_REMAINING] = monst->maxStatus[STATUS_LIFESPAN_REMAINING] = charmGuardianLifespan(netEnchant(theItem));
    pmapAt(monst->loc)->flags |= HAS_MONSTER;
    fadeInMonster(monst);
}

/// @brief Decrements item quantity or removes an item from inventory if it's the last one
/// @param theItem the item to consume
static void consumePackItem(item *theItem) {

    if (theItem->quantity > 1) {
        theItem->quantity--;
    } else {
        removeItemFromChain(theItem, packItems);
        deleteItem(theItem);
    }
}

/// @brief The player eats the given item, nutrition is increased, the item is removed from inventory, and the
/// action is optionally recorded. If the player isn't hungry enough they can cancel the action.
/// @param theItem the item to eat
/// @param recordCommand set to true if called from the apply command and we need to record the command sequence
/// @return true if the player ate the item
boolean eat(item *theItem, boolean recordCommands) {
    if (!(theItem->category & FOOD)) {
        return false;
    }

    if (STOMACH_SIZE - player.status[STATUS_NUTRITION] < foodTable[theItem->kind].power) { // Not hungry enough.
        char buf[COLS * 3];
        sprintf(buf, "You're not hungry enough to fully enjoy the %s. Eat it anyway?",
                (theItem->kind == RATION ? "food" : "mango"));
        if (!confirm(buf, false)) {
            return false;
        }
    }

    player.status[STATUS_NUTRITION] = min(foodTable[theItem->kind].power + player.status[STATUS_NUTRITION], STOMACH_SIZE);
    if (theItem->kind == RATION) {
        messageWithColor("That food tasted delicious!", &itemMessageColor, 0);
    } else {
        messageWithColor("My, what a yummy mango!", &itemMessageColor, 0);
    }

    if (recordCommands) {
        recordApplyItemCommand(theItem);
    }

    consumePackItem(theItem);

    return true;
}

static boolean useCharm(item *theItem) {

    if (theItem->charges > 0) {
        char buf[COLS * 3], buf2[COLS * 3];
        itemName(theItem, buf2, false, false, NULL);
        sprintf(buf, "Your %s hasn't finished recharging.", buf2);
        messageWithColor(buf, &itemMessageColor, 0);
        return false;
    }

    fixpt enchant = netEnchant(theItem);

    rogue.featRecord[FEAT_PURE_WARRIOR] = false;

    switch (theItem->kind) {
        case CHARM_HEALTH:
            heal(&player, charmHealing(enchant), false);
            message("You feel much healthier.", 0);
            break;
        case CHARM_PROTECTION:
            if (charmProtection(enchant) > player.status[STATUS_SHIELDED]) {
                player.status[STATUS_SHIELDED] = charmProtection(enchant);
            }
            player.maxStatus[STATUS_SHIELDED] = player.status[STATUS_SHIELDED];
            if (boltCatalog[BOLT_SHIELDING].backColor) {
                flashMonster(&player, boltCatalog[BOLT_SHIELDING].backColor, 100);
            }
            message("A shimmering shield coalesces around you.", 0);
            break;
        case CHARM_HASTE:
            haste(&player, charmEffectDuration(theItem->kind, theItem->enchant1));
            break;
        case CHARM_FIRE_IMMUNITY:
            player.status[STATUS_IMMUNE_TO_FIRE] = player.maxStatus[STATUS_IMMUNE_TO_FIRE] = charmEffectDuration(theItem->kind, theItem->enchant1);
            if (player.status[STATUS_BURNING]) {
                extinguishFireOnCreature(&player);
            }
            message("you no longer fear fire.", 0);
            break;
        case CHARM_INVISIBILITY:
            imbueInvisibility(&player, charmEffectDuration(theItem->kind, theItem->enchant1));
            message("You shiver as a chill runs up your spine.", 0);
            break;
        case CHARM_TELEPATHY:
            makePlayerTelepathic(charmEffectDuration(theItem->kind, theItem->enchant1));
            break;
        case CHARM_LEVITATION:
            player.status[STATUS_LEVITATING] = player.maxStatus[STATUS_LEVITATING] = charmEffectDuration(theItem->kind, theItem->enchant1);
            player.bookkeepingFlags &= ~MB_SEIZED; // break free of holding monsters
            message("you float into the air!", 0);
            break;
        case CHARM_SHATTERING:
            messageWithColor("your charm emits a wave of turquoise light that pierces the nearby walls!", &itemMessageColor, 0);
            crystalize(charmShattering(enchant));
            break;
        case CHARM_GUARDIAN:
            messageWithColor("your charm flashes and the form of a mythical guardian coalesces!", &itemMessageColor, 0);
            summonGuardian(theItem);
            break;
        case CHARM_TELEPORTATION:
            teleport(&player, INVALID_POS, true);
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

    theItem->charges = charmRechargeDelay(theItem->kind, theItem->enchant1);
    recordApplyItemCommand(theItem);
    return true;
}

void apply(item *theItem) {
    char buf[COLS * 3], buf2[COLS * 3];

    if (!theItem) {
        theItem = promptForItemOfType((SCROLL|FOOD|POTION|STAFF|WAND|CHARM), 0, 0,
                                      KEYBOARD_LABELS ? "Apply what? (a-z, shift for more info; or <esc> to cancel)" : "Apply what?",
                                      true);
    }

    if (theItem == NULL) {
        return;
    }

    confirmMessages();
    switch (theItem->category) {
        case FOOD:
            if (eat(theItem, true)) {
                break;
            }
            return;
        case POTION:
            if (drinkPotion(theItem)) {
                break;
            }
            return;
        case SCROLL:
            if (readScroll(theItem)) {
                consumePackItem(theItem);
                break;
            }
            return;
        case STAFF:
        case WAND:
            if (useStaffOrWand(theItem)) {
                break;
            }
            return;
        case CHARM:
            if (useCharm(theItem)) {
                break;
            }
            return;
        default:
            itemName(theItem, buf2, false, true, NULL);
            sprintf(buf, "you can't apply %s.", buf2);
            message(buf, 0);
            return;
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

static short lotteryDraw(short *frequencies, short itemCount) {
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
               && tableForItemCategory(theItem->category)[theItem->kind].identified) {

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

static void magicMapCell(short x, short y) {
    pmap[x][y].flags |= MAGIC_MAPPED;
    pmap[x][y].rememberedTerrainFlags = tileCatalog[pmap[x][y].layers[DUNGEON]].flags | tileCatalog[pmap[x][y].layers[LIQUID]].flags;
    pmap[x][y].rememberedTMFlags = tileCatalog[pmap[x][y].layers[DUNGEON]].mechFlags | tileCatalog[pmap[x][y].layers[LIQUID]].mechFlags;
    if (pmap[x][y].layers[LIQUID] && tileCatalog[pmap[x][y].layers[LIQUID]].drawPriority < tileCatalog[pmap[x][y].layers[DUNGEON]].drawPriority) {
        pmap[x][y].rememberedTerrain = pmap[x][y].layers[LIQUID];
    } else {
        pmap[x][y].rememberedTerrain = pmap[x][y].layers[DUNGEON];
    }
}

static boolean uncurse( item *theItem ) {
    if (theItem->flags & ITEM_CURSED) {
        theItem->flags &= ~ITEM_CURSED;
        return true;
    }
    return false;
}

boolean readScroll(item *theItem) {
    short i, j, x, y, numberOfMonsters = 0;
    item *tempItem;
    creature *monst;
    boolean hadEffect = false;
    char buf[COLS * 3], buf2[COLS * 3];

    itemTable scrollKind = tableForItemCategory(theItem->category)[theItem->kind];

    if (magicCharDiscoverySuffix(theItem->category, theItem->kind) == -1
        && ((theItem->flags & ITEM_MAGIC_DETECTED) || scrollKind.identified)) {

        if (scrollKind.identified) {
            sprintf(buf, "Really read a scroll of %s?", scrollKind.name);
        } else {
            sprintf(buf, "Really read a cursed scroll?");
        }
        if (!confirm(buf, false)) {
            return false;
        }
    }

    // Past the point of no return.
    recordApplyItemCommand(theItem);

    switch (theItem->kind) {
        case SCROLL_IDENTIFY:
            identify(theItem);
            updateIdentifiableItems();
            messageWithColor("this is a scroll of identify.", &itemMessageColor, REQUIRE_ACKNOWLEDGMENT);
            if (numberOfMatchingPackItems(ALL_ITEMS, ITEM_CAN_BE_IDENTIFIED, 0, false) == 0) {
                message("everything in your pack is already identified.", 0);
                break; // regardless, the scroll is consumed
            }
            do {
                theItem = promptForItemOfType((ALL_ITEMS), ITEM_CAN_BE_IDENTIFIED, 0,
                                              KEYBOARD_LABELS ? "Identify what? (a-z; shift for more info)" : "Identify what?",
                                              false);
                if (rogue.gameHasEnded) {
                    return false;
                }
                if (theItem && !(theItem->flags & ITEM_CAN_BE_IDENTIFIED)) {
                    confirmMessages();
                    itemName(theItem, buf2, true, true, NULL);
                    sprintf(buf, "you already know %s %s.", (theItem->quantity > 1 ? "they're" : "it's"), buf2);
                    messageWithColor(buf, &itemMessageColor, 0);
                }
            } while (theItem == NULL || !(theItem->flags & ITEM_CAN_BE_IDENTIFIED));
            recordKeystroke(theItem->inventoryLetter, false, false);
            confirmMessages();
            identify(theItem);
            itemName(theItem, buf, true, true, NULL);
            sprintf(buf2, "%s %s.", (theItem->quantity == 1 ? "this is" : "these are"), buf);
            messageWithColor(buf2, &itemMessageColor, 0);
            break;
        case SCROLL_TELEPORT:
            teleport(&player, INVALID_POS, true);
            break;
        case SCROLL_REMOVE_CURSE:
            for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
                hadEffect |= uncurse(tempItem);
            }
            if (hadEffect) {
                message("your pack glows with a cleansing light, and a malevolent energy disperses.", 0);
            } else {
                message("your pack glows with a cleansing light, but nothing happens.", 0);
            }
            break;
        case SCROLL_ENCHANTING:
            identify(theItem);
            messageWithColor("this is a scroll of enchanting.", &itemMessageColor, REQUIRE_ACKNOWLEDGMENT);
            if (!numberOfMatchingPackItems((WEAPON | ARMOR | RING | STAFF | WAND | CHARM), 0, 0, false)) {
                confirmMessages();
                message("you have nothing that can be enchanted.", 0);
                break; // regardless, the scroll is consumed
            }
            do {
                theItem = promptForItemOfType((WEAPON | ARMOR | RING | STAFF | WAND | CHARM), 0, 0,
                                              KEYBOARD_LABELS ? "Enchant what? (a-z; shift for more info)" : "Enchant what?",
                                              false);
                confirmMessages();
                if (theItem == NULL || !(theItem->category & (WEAPON | ARMOR | RING | STAFF | WAND | CHARM))) {
                    message("Can't enchant that.", REQUIRE_ACKNOWLEDGMENT);
                }
                if (rogue.gameHasEnded) {
                    return false;
                }
            } while (theItem == NULL || !(theItem->category & (WEAPON | ARMOR | RING | STAFF | WAND | CHARM)));
            recordKeystroke(theItem->inventoryLetter, false, false);
            confirmMessages();

            theItem->timesEnchanted += enchantMagnitude();
            switch (theItem->category) {
                case WEAPON:
                    theItem->strengthRequired = max(0, theItem->strengthRequired - enchantMagnitude());
                    theItem->enchant1 += enchantMagnitude();
                    if (theItem->quiverNumber) {
                        theItem->quiverNumber = rand_range(1, 60000);
                    }
                    break;
                case ARMOR:
                    theItem->strengthRequired = max(0, theItem->strengthRequired - enchantMagnitude());
                    theItem->enchant1 += enchantMagnitude();
                    break;
                case RING:
                    theItem->enchant1 += enchantMagnitude();
                    updateRingBonuses();
                    if (theItem->kind == RING_CLAIRVOYANCE) {
                        updateClairvoyance();
                        displayLevel();
                    }
                    break;
                case STAFF:
                    theItem->enchant1 += enchantMagnitude();
                    theItem->charges += enchantMagnitude();
                    theItem->enchant2 = 500 / theItem->enchant1;
                    break;
                case WAND:
                    theItem->charges += wandTable[theItem->kind].range.lowerBound * enchantMagnitude();
                    break;
                case CHARM:
                    theItem->enchant1 += enchantMagnitude();
                    theItem->charges = min(0, theItem->charges); // Enchanting instantly recharges charms.
                    break;
                default:
                    break;
            }
            if ((theItem->category & (WEAPON | ARMOR | STAFF | RING | CHARM))
                && theItem->enchant1 >= 16) {

                rogue.featRecord[FEAT_SPECIALIST] = true;
            }
            if (theItem->flags & ITEM_EQUIPPED) {
                equipItem(theItem, true, NULL);
            }
            itemName(theItem, buf, false, false, NULL);
            sprintf(buf2, "your %s gleam%s briefly in the darkness.", buf, (theItem->quantity == 1 ? "s" : ""));
            messageWithColor(buf2, &itemMessageColor, 0);
            if (uncurse(theItem)) {
                sprintf(buf2, "a malevolent force leaves your %s.", buf);
                messageWithColor(buf2, &itemMessageColor, 0);
            }
            createFlare(player.loc.x, player.loc.y, SCROLL_ENCHANTMENT_LIGHT);
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
                messageWithColor(buf, &itemMessageColor, 0);
                if (uncurse(tempItem)) {
                    sprintf(buf, "a malevolent force leaves your %s.", buf2);
                    messageWithColor(buf, &itemMessageColor, 0);
                }
            } else {
                message("a protective golden light surrounds you, but it quickly disperses.", 0);
            }
            createFlare(player.loc.x, player.loc.y, SCROLL_PROTECTION_LIGHT);
            break;
        case SCROLL_PROTECT_WEAPON:
            if (rogue.weapon) {
                tempItem = rogue.weapon;
                tempItem->flags |= ITEM_PROTECTED;
                itemName(tempItem, buf2, false, false, NULL);
                sprintf(buf, "a protective golden light covers your %s.", buf2);
                messageWithColor(buf, &itemMessageColor, 0);
                if (uncurse(tempItem)) {
                    sprintf(buf, "a malevolent force leaves your %s.", buf2);
                    messageWithColor(buf, &itemMessageColor, 0);
                }
                if (rogue.weapon->quiverNumber) {
                    rogue.weapon->quiverNumber = rand_range(1, 60000);
                }
            } else {
                message("a protective golden light covers your empty hands, but it quickly disperses.", 0);
            }
            createFlare(player.loc.x, player.loc.y, SCROLL_PROTECTION_LIGHT);
            break;
        case SCROLL_SANCTUARY:
            spawnDungeonFeature(player.loc.x, player.loc.y, &dungeonFeatureCatalog[DF_SACRED_GLYPHS], true, false);
            messageWithColor("sprays of color arc to the ground, forming glyphs where they alight.", &itemMessageColor, 0);
            break;
        case SCROLL_MAGIC_MAPPING:
            confirmMessages();
            messageWithColor("this scroll has a map on it!", &itemMessageColor, 0);
            for (i=0; i<DCOLS; i++) {
                for (j=0; j<DROWS; j++) {
                    if (cellHasTMFlag((pos){ i, j }, TM_IS_SECRET)) {
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
            for (i=0; i<DCOLS; i++) {
                for (j=0; j<DROWS; j++) {
                    if (!(cellHasTerrainFlag((pos){ i, j }, T_IS_DF_TRAP))) {
                        pmap[i][j].flags |= KNOWN_TO_BE_TRAP_FREE;
                    }
                }
            }
            colorFlash(&magicMapFlashColor, 0, MAGIC_MAPPED, 15, DCOLS + DROWS, player.loc.x, player.loc.y);
            break;
        case SCROLL_AGGRAVATE_MONSTER:
            aggravateMonsters(DCOLS + DROWS, player.loc.x, player.loc.y, &gray);
            message("the scroll emits a piercing shriek that echoes throughout the dungeon!", 0);
            break;
        case SCROLL_SUMMON_MONSTER:
            for (j=0; j<25 && numberOfMonsters < 3; j++) {
                for (i=0; i<8; i++) {
                    x = player.loc.x + nbDirs[i][0];
                    y = player.loc.y + nbDirs[i][1];
                    if (!cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY) && !(pmap[x][y].flags & HAS_MONSTER)
                        && rand_percent(10) && (numberOfMonsters < 3)) {
                        monst = spawnHorde(0, (pos){ x, y }, (HORDE_LEADER_CAPTIVE | HORDE_NO_PERIODIC_SPAWN | HORDE_IS_SUMMONED | HORDE_MACHINE_ONLY), 0);
                        if (monst) {
                            // refreshDungeonCell((pos){ x, y });
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
                message("the fabric of space ripples, and monsters appear!", 0);
            } else if (numberOfMonsters == 1) {
                message("the fabric of space ripples, and a monster appears!", 0);
            } else {
                message("the fabric of space boils violently around you, but nothing happens.", 0);
            }
            break;
        case SCROLL_NEGATION:
            negationBlast("the scroll", DCOLS);
            break;
        case SCROLL_SHATTERING:
            messageWithColor("the scroll emits a wave of turquoise light that pierces the nearby walls!", &itemMessageColor, 0);
            crystalize(9);
            break;
        case SCROLL_DISCORD:
            discordBlast("the scroll", DCOLS);
            break;
    }

    // all scrolls auto-identify on use
    if (!scrollKind.identified
        && (theItem->kind != SCROLL_ENCHANTING)
        && (theItem->kind != SCROLL_IDENTIFY)) {

        autoIdentify(theItem);
    }

    return true;
}

static void detectMagicOnItem(item *theItem) {
    if (theItem->category & HAS_INTRINSIC_POLARITY) {
        itemTable *theItemTable = tableForItemCategory(theItem->category);
        theItemTable[theItem->kind].magicPolarityRevealed = true;
    }
    theItem->flags |= ITEM_MAGIC_DETECTED;
    if ((theItem->category & (WEAPON | ARMOR))
        && theItem->enchant1 == 0
        && !(theItem->flags & ITEM_RUNIC)) {

        identify(theItem);
    }
}

boolean drinkPotion(item *theItem) {
    item *tempItem = NULL;
    char buf[1000] = "";
    int magnitude;

    brogueAssert(rogue.RNG == RNG_SUBSTANTIVE);

    itemTable potionKind = tableForItemCategory(theItem->category)[theItem->kind];

    if (magicCharDiscoverySuffix(theItem->category, theItem->kind) == -1
        && ((theItem->flags & ITEM_MAGIC_DETECTED) || potionKind.identified)) {

        if (potionKind.identified) {
            sprintf(buf,"Really drink a potion of %s?", potionKind.name);
        } else {
            sprintf(buf,"Really drink a cursed potion?");
        }
        if (!confirm(buf, false)) {
            return false;
        }
    }

    confirmMessages();
    magnitude = randClump(potionKind.range);

    switch (theItem->kind) {
        case POTION_LIFE:
            sprintf(buf, "%syour maximum health increases by %i%%.",
                    ((player.currentHP < player.info.maxHP) ? "you heal completely and " : ""),
                    (player.info.maxHP + magnitude) * 100 / player.info.maxHP - 100);

            player.info.maxHP += magnitude;
            heal(&player, 100, true);
            updatePlayerRegenerationDelay();
            messageWithColor(buf, &advancementMessageColor, 0);
            break;
        case POTION_HALLUCINATION:
            player.status[STATUS_HALLUCINATING] = player.maxStatus[STATUS_HALLUCINATING] = magnitude;
            message("colors are everywhere! The walls are singing!", 0);
            break;
        case POTION_INCINERATION:
            //colorFlash(&darkOrange, 0, IN_FIELD_OF_VIEW, 4, 4, player.loc.x, player.loc.y);
            message("as you uncork the flask, it explodes in flame!", 0);
            spawnDungeonFeature(player.loc.x, player.loc.y, &dungeonFeatureCatalog[DF_INCINERATION_POTION], true, false);
            exposeCreatureToFire(&player);
            break;
        case POTION_DARKNESS:
            player.status[STATUS_DARKNESS] = max(magnitude, player.status[STATUS_DARKNESS]);
            player.maxStatus[STATUS_DARKNESS] = max(magnitude, player.maxStatus[STATUS_DARKNESS]);
            updateMinersLightRadius();
            updateVision(true);
            message("your vision flickers as a cloak of darkness settles around you!", 0);
            break;
        case POTION_DESCENT:
            colorFlash(&darkBlue, 0, IN_FIELD_OF_VIEW, 3, 3, player.loc.x, player.loc.y);
            message("vapor pours out of the flask and causes the floor to disappear!", 0);
            spawnDungeonFeature(player.loc.x, player.loc.y, &dungeonFeatureCatalog[DF_HOLE_POTION], true, false);
            if (!player.status[STATUS_LEVITATING]) {
                player.bookkeepingFlags |= MB_IS_FALLING;
            }
            break;
        case POTION_STRENGTH:
            rogue.strength += magnitude;
            if (player.status[STATUS_WEAKENED]) {
                player.status[STATUS_WEAKENED] = 1;
            }
            //Needed to make potion have immediate effect
            player.weaknessAmount = 0;
            updateEncumbrance();
            messageWithColor("newfound strength surges through your body.", &advancementMessageColor, 0);
            createFlare(player.loc.x, player.loc.y, POTION_STRENGTH_LIGHT);
            break;
        case POTION_POISON:
            spawnDungeonFeature(player.loc.x, player.loc.y, &dungeonFeatureCatalog[DF_POISON_GAS_CLOUD_POTION], true, false);
            message("caustic gas billows out of the open flask!", 0);
            break;
        case POTION_PARALYSIS:
            spawnDungeonFeature(player.loc.x, player.loc.y, &dungeonFeatureCatalog[DF_PARALYSIS_GAS_CLOUD_POTION], true, false);
            message("your muscles stiffen as a cloud of pink gas bursts from the open flask!", 0);
            break;
        case POTION_TELEPATHY:
            makePlayerTelepathic(magnitude);
            break;
        case POTION_LEVITATION:
            player.status[STATUS_LEVITATING] = player.maxStatus[STATUS_LEVITATING] = magnitude;
            player.bookkeepingFlags &= ~MB_SEIZED; // break free of holding monsters
            message("you float into the air!", 0);
            break;
        case POTION_CONFUSION:
            spawnDungeonFeature(player.loc.x, player.loc.y, &dungeonFeatureCatalog[DF_CONFUSION_GAS_CLOUD_POTION], true, false);
            message("a shimmering cloud of rainbow-colored gas billows out of the open flask!", 0);
            break;
        case POTION_LICHEN:
            message("a handful of tiny spores burst out of the open flask!", 0);
            spawnDungeonFeature(player.loc.x, player.loc.y, &dungeonFeatureCatalog[DF_LICHEN_PLANTED], true, false);
            break;
        case POTION_DETECT_MAGIC: {
            boolean hadEffectOnLevel = false;
            boolean hadEffectOnPack = false;
            for (tempItem = floorItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
                if (tempItem->category & CAN_BE_DETECTED) {
                    detectMagicOnItem(tempItem);
                    if (itemMagicPolarity(tempItem)) {
                        pmapAt(tempItem->loc)->flags |= ITEM_DETECTED;
                        hadEffectOnLevel = true;
                        refreshDungeonCell(tempItem->loc);
                    }
                }
            }
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *monst = nextCreature(&it);
                if (monst->carriedItem && (monst->carriedItem->category & CAN_BE_DETECTED)) {
                    detectMagicOnItem(monst->carriedItem);
                    if (itemMagicPolarity(monst->carriedItem)) {
                        hadEffectOnLevel = true;
                        refreshDungeonCell(monst->loc);
                    }
                }
            }
            for (tempItem = packItems->nextItem; tempItem != NULL; tempItem = tempItem->nextItem) {
                if (tempItem->category & CAN_BE_DETECTED) {
                    detectMagicOnItem(tempItem);
                    if (itemMagicPolarity(tempItem)) {
                        if (tempItem != theItem && (tempItem->flags & ITEM_MAGIC_DETECTED)) {
                            // Don't allow the potion of detect magic to detect itself.
                            hadEffectOnPack = true;
                        }
                    }
                }
            }
            if (hadEffectOnLevel || hadEffectOnPack) {
                tryIdentifyLastItemKinds(HAS_INTRINSIC_POLARITY);
                if (hadEffectOnLevel && hadEffectOnPack) {
                    message("you can somehow feel the presence of magic on the level and in your pack.", 0);
                } else if (hadEffectOnLevel) {
                    message("you can somehow feel the presence of magic on the level.", 0);
                } else {
                    message("you can somehow feel the presence of magic in your pack.", 0);
                }
            } else {
                message("you can somehow feel the absence of magic on the level and in your pack.", 0);
            }
            break;
        }
        case POTION_HASTE_SELF:
            haste(&player, magnitude);
            break;
        case POTION_FIRE_IMMUNITY:
            player.status[STATUS_IMMUNE_TO_FIRE] = player.maxStatus[STATUS_IMMUNE_TO_FIRE] = magnitude;
            if (player.status[STATUS_BURNING]) {
                extinguishFireOnCreature(&player);
            }
            message("a comforting breeze envelops you, and you no longer fear fire.", 0);
            break;
        case POTION_INVISIBILITY:
            player.status[STATUS_INVISIBLE] = player.maxStatus[STATUS_INVISIBLE] = magnitude;
            message("you shiver as a chill runs up your spine.", 0);
            break;
        default:
            message("you feel very strange, as though your body doesn't know how to react!", REQUIRE_ACKNOWLEDGMENT);
    }

    if (!potionKind.identified) {
        autoIdentify(theItem);
    }

    recordApplyItemCommand(theItem);
    consumePackItem(theItem);
    return true;
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
            if (boltCatalog[tableForItemCategory(category)[kind].power].flags & (BF_TARGET_ALLIES)) {
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
    itemTable *theItemTable = tableForItemCategory(theItem->category);
    switch (theItem->category) {
        case WEAPON:
        case ARMOR:
            if ((theItem->flags & ITEM_CURSED) || theItem->enchant1 < 0) {
                return MAGIC_POLARITY_MALEVOLENT;
            } else if (theItem->enchant1 > 0) {
                return MAGIC_POLARITY_BENEVOLENT;
            }
            return MAGIC_POLARITY_NEUTRAL;
        case WAND:
            if (theItem->charges == 0) {
                return MAGIC_POLARITY_NEUTRAL;
            }
        case SCROLL:
        case POTION:
        case CHARM:
        case STAFF:
            return theItemTable[theItem->kind].magicPolarity;
        case RING:
            if (theItem->flags & ITEM_CURSED || theItem->enchant1 < 0) {
                return MAGIC_POLARITY_MALEVOLENT;
            } else if (theItem->enchant1 > 0) {
                return MAGIC_POLARITY_BENEVOLENT;
            } else {
                return MAGIC_POLARITY_NEUTRAL;
            }
        case AMULET:
            return MAGIC_POLARITY_BENEVOLENT;
        default:
            return MAGIC_POLARITY_NEUTRAL;
    }
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
        messageWithColor(buf, &itemMessageColor, 0);
        return;
    } else {
        if (!unequipItem(theItem, false)) {
            return; // cursed
        }
        recordKeystrokeSequence(command);
        itemName(theItem, buf2, true, true, NULL);
        if (strLenWithoutEscapes(buf2) > 52) {
            itemName(theItem, buf2, false, true, NULL);
        }
        confirmMessages();
        sprintf(buf, "you are no longer %s %s.", (theItem->category & WEAPON ? "wielding" : "wearing"), buf2);
        messageWithColor(buf, &itemMessageColor, 0);
    }
    playerTurnEnded();
}

static boolean canDrop() {
    if (cellHasTerrainFlag(player.loc, T_OBSTRUCTS_ITEMS)) {
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
        messageWithColor(buf, &itemMessageColor, 0);
    } else if (canDrop()) {
        recordKeystrokeSequence(command);
        if (theItem->flags & ITEM_EQUIPPED) {
            unequipItem(theItem, false);
        }
        theItem = dropItem(theItem); // This is where it gets dropped.
        theItem->flags |= ITEM_PLAYER_AVOIDS; // Try not to pick up stuff you've already dropped.
        itemName(theItem, buf2, true, true, NULL);
        sprintf(buf, "You dropped %s.", buf2);
        messageWithColor(buf, &itemMessageColor, 0);
        playerTurnEnded();
    } else {
        confirmMessages();
        message("There is already something there.", 0);
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

    temporaryMessage(prompt, 0);

    keystroke = displayInventory(category, requiredFlags, forbiddenFlags, false, allowInventoryActions);

    if (!keystroke) {
        // This can happen if the player does an action with an item directly from the inventory screen via a button.
        return NULL;
    }

    if (keystroke < 'a' || keystroke > 'z') {
        confirmMessages();
        if (keystroke != ESCAPE_KEY && keystroke != ACKNOWLEDGE_KEY) {
            message("Invalid entry.", 0);
        }
        return NULL;
    }

    theItem = itemOfPackLetter(keystroke);
    if (theItem == NULL) {
        confirmMessages();
        message("No such item.", 0);
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

item *itemAtLoc(pos loc) {
    item *theItem;

    if (!(pmapAt(loc)->flags & HAS_ITEM)) {
        return NULL; // easy optimization
    }
    for (theItem = floorItems->nextItem; theItem != NULL && !posEq(theItem->loc, loc); theItem = theItem->nextItem);
    if (theItem == NULL) {
        pmapAt(loc)->flags &= ~HAS_ITEM;
        hiliteCell(loc.x, loc.y, &white, 75, true);
        rogue.automationActive = false;
        message("ERROR: An item was supposed to be here, but I couldn't find it.", REQUIRE_ACKNOWLEDGMENT);
        refreshDungeonCell(loc);
    }
    return theItem;
}

item *dropItem(item *theItem) {
    item *itemFromTopOfStack, *itemOnFloor;

    if (cellHasTerrainFlag(player.loc, T_OBSTRUCTS_ITEMS)) {
        return NULL;
    }

    itemOnFloor = itemAtLoc(player.loc);

    if (theItem->quantity > 1 && !(theItem->category & (WEAPON | GEM))) { // peel off the top item and drop it
        itemFromTopOfStack = generateItem(ALL_ITEMS, -1);
        *itemFromTopOfStack = *theItem; // clone the item
        theItem->quantity--;
        itemFromTopOfStack->quantity = 1;
        if (itemOnFloor) {
            itemOnFloor->inventoryLetter = theItem->inventoryLetter; // just in case all letters are taken
            pickUpItemAt(player.loc);
        }
        placeItemAt(itemFromTopOfStack, player.loc);
        return itemFromTopOfStack;
    } else { // drop the entire item
        if (rogue.swappedIn == theItem || rogue.swappedOut == theItem) {
            rogue.swappedIn = NULL;
            rogue.swappedOut = NULL;
        }
        removeItemFromChain(theItem, packItems);
        if (itemOnFloor) {
            itemOnFloor->inventoryLetter = theItem->inventoryLetter;
            pickUpItemAt(player.loc);
        }
        placeItemAt(theItem, player.loc);
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

// Returns true on success, false otherwise (for example, if failing to remove
// a cursed item) If something must be first unequipped and it is not clear
// what, unequipHint will be used if passed.
boolean equipItem(item *theItem, boolean force, item *unequipHint) {
    char buf1[COLS * 3], buf2[COLS * 3], buf3[COLS * 3];
    item *previouslyEquippedItem = NULL;

    if ((theItem->category & RING) && (theItem->flags & ITEM_EQUIPPED)) {
        return false;
    }

    if (theItem->category & WEAPON) {
        previouslyEquippedItem = rogue.weapon;
    } else if (theItem->category & ARMOR) {
        previouslyEquippedItem = rogue.armor;
    } else if (theItem->category & RING
              && unequipHint && rogue.ringLeft && rogue.ringRight
              && (unequipHint == rogue.ringLeft || unequipHint == rogue.ringRight)) {
        previouslyEquippedItem = unequipHint;
    }

    if (previouslyEquippedItem && !unequipItem(previouslyEquippedItem, force)) {
        return false; // already using a cursed item
    }
    if (theItem->category & WEAPON) {
        rogue.weapon = theItem;
        strengthCheck(theItem, !force);
    } else if (theItem->category & ARMOR) {
        if (!force) {
            player.status[STATUS_DONNING] = player.maxStatus[STATUS_DONNING] = theItem->armor / 10;
        }
        rogue.armor = theItem;
        strengthCheck(theItem, !force);
    } else if (theItem->category & RING) {
        if (rogue.ringLeft && rogue.ringRight) {
            return false; // no available ring slot and no hint, see equip()
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
        updateEncumbrance();
    }
    theItem->flags |= ITEM_EQUIPPED;

    itemName(theItem, buf2, true, true, NULL);

    if (!force) {
        if (previouslyEquippedItem) {
            itemName(previouslyEquippedItem, buf3, false, false, NULL);
            sprintf(buf1, "Now %s %s instead of your %s.", (theItem->category & WEAPON ? "wielding" : "wearing"), buf2, buf3);
        } else {
            sprintf(buf1, "Now %s %s.", (theItem->category & WEAPON ? "wielding" : "wearing"), buf2);
        }

        confirmMessages();
        messageWithColor(buf1, &itemMessageColor, false);

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
            messageWithColor(buf1, &itemMessageColor, 0);
        }
    }

    return true;
}

// Returns true on success, false otherwise (for example, if cursed and not forced)
boolean unequipItem(item *theItem, boolean force) {
    char buf[COLS * 3], buf2[COLS * 3];

    if (theItem == NULL || !(theItem->flags & ITEM_EQUIPPED)) {
        return false;
    }
    if ((theItem->flags & ITEM_CURSED) && !force) {
        itemName(theItem, buf2, false, false, NULL);
        sprintf(buf, "you can't; your %s appear%s to be cursed.",
                buf2,
                theItem->quantity == 1 ? "s" : "");
        confirmMessages();
        messageWithColor(buf, &itemMessageColor, 0);
        return false;
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
    return true;
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

static void resetItemTableEntry(itemTable *theEntry) {
    theEntry->identified = false;
    theEntry->magicPolarityRevealed = false;
    theEntry->called = false;
    theEntry->callTitle[0] = '\0';
}

void shuffleFlavors() {
    short i, j, randIndex, randNumber;
    char buf[COLS];

    for (i=0; i<gameConst->numberPotionKinds; i++) {
        resetItemTableEntry(potionTable + i);
    }
    for (i=0; i<NUMBER_STAFF_KINDS; i++) {
        resetItemTableEntry(staffTable+ i);
    }
    for (i=0; i<gameConst->numberWandKinds; i++) {
        resetItemTableEntry(wandTable + i);
    }
    for (i=0; i<gameConst->numberScrollKinds; i++) {
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

    for (i=0; i<NUMBER_ITEM_TITLES; i++) {
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
