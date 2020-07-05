/*
 *  Movement.c
 *  Brogue
 *
 *  Created by Brian Walker on 1/10/09.
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

void playerRuns(short direction) {
    short newX, newY, dir;
    boolean cardinalPassability[4];

    rogue.disturbed = (player.status[STATUS_CONFUSED] ? true : false);

    for (dir = 0; dir < 4; dir++) {
        newX = player.xLoc + nbDirs[dir][0];
        newY = player.yLoc + nbDirs[dir][1];
        cardinalPassability[dir] = monsterAvoids(&player, newX, newY);
    }

    while (!rogue.disturbed) {
        if (!playerMoves(direction)) {
            rogue.disturbed = true;
            break;
        }

        newX = player.xLoc + nbDirs[direction][0];
        newY = player.yLoc + nbDirs[direction][1];
        if (!coordinatesAreInMap(newX, newY)
            || monsterAvoids(&player, newX, newY)) {

            rogue.disturbed = true;
        }
        if (isDisturbed(player.xLoc, player.yLoc)) {
            rogue.disturbed = true;
        } else if (direction < 4) {
            for (dir = 0; dir < 4; dir++) {
                newX = player.xLoc + nbDirs[dir][0];
                newY = player.yLoc + nbDirs[dir][1];
                if (cardinalPassability[dir] != monsterAvoids(&player, newX, newY)
                    && !(nbDirs[dir][0] + nbDirs[direction][0] == 0 &&
                         nbDirs[dir][1] + nbDirs[direction][1] == 0)) {
                        // dir is not the x-opposite or y-opposite of direction
                    rogue.disturbed = true;
                }
            }
        }
    }
    updateFlavorText();
}

enum dungeonLayers highestPriorityLayer(short x, short y, boolean skipGas) {
    short bestPriority = 10000;
    enum dungeonLayers tt, best = 0;

    for (tt = 0; tt < NUMBER_TERRAIN_LAYERS; tt++) {
        if (tt == GAS && skipGas) {
            continue;
        }
        if (pmap[x][y].layers[tt] && tileCatalog[pmap[x][y].layers[tt]].drawPriority < bestPriority) {
            bestPriority = tileCatalog[pmap[x][y].layers[tt]].drawPriority;
            best = tt;
        }
    }
    return best;
}

enum dungeonLayers layerWithTMFlag(short x, short y, unsigned long flag) {
    enum dungeonLayers layer;

    for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
        if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & flag) {
            return layer;
        }
    }
    return NO_LAYER;
}

enum dungeonLayers layerWithFlag(short x, short y, unsigned long flag) {
    enum dungeonLayers layer;

    for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
        if (tileCatalog[pmap[x][y].layers[layer]].flags & flag) {
            return layer;
        }
    }
    return NO_LAYER;
}

// Retrieves a pointer to the flavor text of the highest-priority terrain at the given location
char *tileFlavor(short x, short y) {
    return tileCatalog[pmap[x][y].layers[highestPriorityLayer(x, y, false)]].flavorText;
}

// Retrieves a pointer to the description text of the highest-priority terrain at the given location
char *tileText(short x, short y) {
    return tileCatalog[pmap[x][y].layers[highestPriorityLayer(x, y, false)]].description;
}

void describedItemBasedOnParameters(short theCategory, short theKind, short theQuantity, short theOriginDepth, char *buf) {
    item *tempItem = initializeItem();
    tempItem->category = theCategory;
    tempItem->kind = theKind;
    tempItem->quantity = theQuantity;
    tempItem->originDepth = theOriginDepth;
    itemName(tempItem, buf, false, true, NULL);
    free(tempItem);
    return;
}

// Describes the item in question either by naming it if the player has already seen its name,
// or by tersely identifying its category otherwise.
void describedItemName(item *theItem, char *buf) {
    if (rogue.playbackOmniscience || (!player.status[STATUS_HALLUCINATING])) {
        itemName(theItem, buf, (theItem->category & (WEAPON | ARMOR) ? false : true), true, NULL);
    } else {
        describeHallucinatedItem(buf);
    }
}

void describeLocation(char *buf, short x, short y) {
    creature *monst;
    item *theItem, *magicItem;
    boolean standsInTerrain;
    boolean subjectMoving;
    boolean prepositionLocked = false;
    boolean monsterDormant;

    char subject[COLS * 3];
    char verb[COLS * 3];
    char preposition[COLS * 3];
    char object[COLS * 3];
    char adjective[COLS * 3];

    assureCosmeticRNG;

    if (x == player.xLoc && y == player.yLoc) {
        if (player.status[STATUS_LEVITATING]) {
            sprintf(buf, "you are hovering above %s.", tileText(x, y));
        } else {
            strcpy(buf, tileFlavor(x, y));
        }
        restoreRNG;
        return;
    }

    monst = NULL;
    standsInTerrain = ((tileCatalog[pmap[x][y].layers[highestPriorityLayer(x, y, false)]].mechFlags & TM_STAND_IN_TILE) ? true : false);
    theItem = itemAtLoc(x, y);
    monsterDormant = false;
    if (pmap[x][y].flags & HAS_MONSTER) {
        monst = monsterAtLoc(x, y);
    } else if (pmap[x][y].flags & HAS_DORMANT_MONSTER) {
        monst = dormantMonsterAtLoc(x, y);
        monsterDormant = true;
    }

    // detecting magical items
    magicItem = NULL;
    if (theItem && !playerCanSeeOrSense(x, y)
        && (theItem->flags & ITEM_MAGIC_DETECTED)
        && itemMagicPolarity(theItem)) {
        magicItem = theItem;
    } else if (monst && !canSeeMonster(monst)
               && monst->carriedItem
               && (monst->carriedItem->flags & ITEM_MAGIC_DETECTED)
               && itemMagicPolarity(monst->carriedItem)) {
        magicItem = monst->carriedItem;
    }
    if (magicItem && !(pmap[x][y].flags & DISCOVERED)) {
        switch (itemMagicPolarity(magicItem)) {
            case 1:
                strcpy(object, magicItem->category == AMULET ? "the Amulet of Yendor" : "benevolent magic");
                break;
            case -1:
                strcpy(object, "malevolent magic");
                break;
            default:
                strcpy(object, "mysterious magic");
                break;
        }
        sprintf(buf, "you can detect the aura of %s here.", object);
        restoreRNG;
        return;
    }

    // telepathy
    if (monst
        && !canSeeMonster(monst)
        && monsterRevealed(monst)) {

        strcpy(adjective, (((!player.status[STATUS_HALLUCINATING] || rogue.playbackOmniscience) && monst->info.displayChar >= 'a' && monst->info.displayChar <= 'z')
                           || (player.status[STATUS_HALLUCINATING] && !rogue.playbackOmniscience && rand_range(0, 1)) ? "small" : "large"));
        if (pmap[x][y].flags & DISCOVERED) {
            strcpy(object, tileText(x, y));
            if (monst->bookkeepingFlags & MB_SUBMERGED) {
                strcpy(preposition, "under ");
            } else if (monsterDormant) {
                strcpy(preposition, "coming from within ");
            } else if (standsInTerrain) {
                strcpy(preposition, "in ");
            } else {
                strcpy(preposition, "over ");
            }
        } else {
            strcpy(object, "here");
            strcpy(preposition, "");
        }

        sprintf(buf, "you can sense a %s psychic emanation %s%s.", adjective, preposition, object);
        restoreRNG;
        return;
    }

    if (monst && !canSeeMonster(monst) && !rogue.playbackOmniscience) {
        // Monster is not visible.
        monst = NULL;
    }

    if (!playerCanSeeOrSense(x, y)) {
        if (pmap[x][y].flags & DISCOVERED) { // memory
            if (pmap[x][y].rememberedItemCategory) {
                if (player.status[STATUS_HALLUCINATING] && !rogue.playbackOmniscience) {
                    describeHallucinatedItem(object);
                } else {
                    describedItemBasedOnParameters(pmap[x][y].rememberedItemCategory, pmap[x][y].rememberedItemKind,
                                                   pmap[x][y].rememberedItemQuantity, pmap[x][y].rememberedItemOriginDepth, object);
                }
            } else {
                strcpy(object, tileCatalog[pmap[x][y].rememberedTerrain].description);
            }
            sprintf(buf, "you remember seeing %s here.", object);
            restoreRNG;
            return;
        } else if (pmap[x][y].flags & MAGIC_MAPPED) { // magic mapped
            sprintf(buf, "you expect %s to be here.", tileCatalog[pmap[x][y].rememberedTerrain].description);
            restoreRNG;
            return;
        }
        strcpy(buf, "");
        restoreRNG;
        return;
    }

    if (monst) {

        monsterName(subject, monst, true);

        if (pmap[x][y].layers[GAS] && monst->status[STATUS_INVISIBLE]) { // phantoms in gas
            sprintf(buf, "you can perceive the faint outline of %s in %s.", subject, tileCatalog[pmap[x][y].layers[GAS]].description);
            restoreRNG;
            return;
        }

        subjectMoving = (monst->turnsSpentStationary == 0
                         && !(monst->info.flags & (MONST_GETS_TURN_ON_ACTIVATION | MONST_IMMOBILE))
                         && monst->creatureState != MONSTER_SLEEPING
                         && !(monst->bookkeepingFlags & (MB_SEIZED | MB_CAPTIVE)));
        if ((monst->info.flags & MONST_ATTACKABLE_THRU_WALLS)
            && cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY)) {
            strcpy(verb, "is embedded");
        } else if (cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY)) {
            strcpy(verb, "is trapped");
            subjectMoving = false;
        } else if (monst->bookkeepingFlags & MB_CAPTIVE) {
            strcpy(verb, "is shackled in place");
            subjectMoving = false;
        } else if (monst->status[STATUS_PARALYZED]) {
            strcpy(verb, "is frozen in place");
            subjectMoving = false;
        } else if (monst->status[STATUS_STUCK]) {
            strcpy(verb, "is entangled");
            subjectMoving = false;
        } else if (monst->status[STATUS_LEVITATING]) {
            strcpy(verb, (subjectMoving ? "is flying" : "is hovering"));
            strcpy(preposition, "over");
            prepositionLocked = true;
        } else if (monsterCanSubmergeNow(monst)) {
            strcpy(verb, (subjectMoving ? "is gliding" : "is drifting"));
        } else if (cellHasTerrainFlag(x, y, T_MOVES_ITEMS) && !(monst->info.flags & MONST_SUBMERGES)) {
            strcpy(verb, (subjectMoving ? "is swimming" : "is struggling"));
        } else if (cellHasTerrainFlag(x, y, T_AUTO_DESCENT)) {
            strcpy(verb, "is suspended in mid-air");
            strcpy(preposition, "over");
            prepositionLocked = true;
            subjectMoving = false;
        } else if (monst->status[STATUS_CONFUSED]) {
            strcpy(verb, "is staggering");
        } else if ((monst->info.flags & MONST_RESTRICTED_TO_LIQUID)
                   && !cellHasTMFlag(monst->xLoc, monst->yLoc, TM_ALLOWS_SUBMERGING)) {
            strcpy(verb, "is lying");
            subjectMoving = false;
        } else if (monst->info.flags & MONST_IMMOBILE) {
            strcpy(verb, "is resting");
        } else {
            switch (monst->creatureState) {
                case MONSTER_SLEEPING:
                    strcpy(verb, "is sleeping");
                    subjectMoving = false;
                    break;
                case MONSTER_WANDERING:
                    strcpy(verb, subjectMoving ? "is wandering" : "is standing");
                    break;
                case MONSTER_FLEEING:
                    strcpy(verb, subjectMoving ? "is fleeing" : "is standing");
                    break;
                case MONSTER_TRACKING_SCENT:
                    strcpy(verb, subjectMoving ? "is charging" : "is standing");
                    break;
                case MONSTER_ALLY:
                    strcpy(verb, subjectMoving ? "is following you" : "is standing");
                    break;
                default:
                    strcpy(verb, "is standing");
                    break;
            }
        }
        if (monst->status[STATUS_BURNING] && !(monst->info.flags & MONST_FIERY)) {
            strcat(verb, ", burning,");
        }

        if (theItem) {
            strcpy(preposition, "over");
            describedItemName(theItem, object);
        } else {
            if (!prepositionLocked) {
                strcpy(preposition, subjectMoving ? (standsInTerrain ? "through" : "across")
                       : (standsInTerrain ? "in" : "on"));
            }

            strcpy(object, tileText(x, y));

        }
    } else { // no monster
        strcpy(object, tileText(x, y));
        if (theItem) {
            describedItemName(theItem, subject);
            subjectMoving = cellHasTerrainFlag(x, y, T_MOVES_ITEMS);
            if (player.status[STATUS_HALLUCINATING] && !rogue.playbackOmniscience) {
                strcpy(verb, "is");
            } else {
                strcpy(verb, (theItem->quantity > 1 || (theItem->category & GOLD)) ? "are" : "is");
            }
            if (cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY)) {
                strcat(verb, " enclosed");
            } else {
                strcat(verb, subjectMoving ? " drifting" : " lying");
            }
            strcpy(preposition, standsInTerrain ? (subjectMoving ? "through" : "in")
                   : (subjectMoving ? "across" : "on"));


        } else { // no item
            sprintf(buf, "you %s %s.", (playerCanDirectlySee(x, y) ? "see" : "sense"), object);
            restoreRNG;
            return;
        }
    }

    sprintf(buf, "%s %s %s %s.", subject, verb, preposition, object);
    restoreRNG;
}

void printLocationDescription(short x, short y) {
    char buf[DCOLS*3];
    describeLocation(buf, x, y);
    flavorMessage(buf);
}

void useKeyAt(item *theItem, short x, short y) {
    short layer, i;
    creature *monst;
    char buf[COLS], buf2[COLS], terrainName[COLS], preposition[10];
    boolean disposable;

    strcpy(terrainName, "unknown terrain"); // redundant failsafe
    for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
        if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_PROMOTES_WITH_KEY) {
            if (tileCatalog[pmap[x][y].layers[layer]].description[0] == 'a'
                && tileCatalog[pmap[x][y].layers[layer]].description[1] == ' ') {
                sprintf(terrainName, "the %s", &(tileCatalog[pmap[x][y].layers[layer]].description[2]));
            } else {
                strcpy(terrainName, tileCatalog[pmap[x][y].layers[layer]].description);
            }
            if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_STAND_IN_TILE) {
                strcpy(preposition, "in");
            } else {
                strcpy(preposition, "on");
            }
            promoteTile(x, y, layer, false);
        }
    }

    disposable = false;
    for (i=0; i < KEY_ID_MAXIMUM && (theItem->keyLoc[i].x || theItem->keyLoc[i].machine); i++) {
        if (theItem->keyLoc[i].x == x && theItem->keyLoc[i].y == y && theItem->keyLoc[i].disposableHere) {
            disposable = true;
        } else if (theItem->keyLoc[i].machine == pmap[x][y].machineNumber && theItem->keyLoc[i].disposableHere) {
            disposable = true;
        }
    }

    if (disposable) {
        if (removeItemFromChain(theItem, packItems)) {
            itemName(theItem, buf2, true, false, NULL);
            sprintf(buf, "you use your %s %s %s.",
                    buf2,
                    preposition,
                    terrainName);
            messageWithColor(buf, &itemMessageColor, false);
            deleteItem(theItem);
        } else if (removeItemFromChain(theItem, floorItems)) {
            deleteItem(theItem);
            pmap[x][y].flags &= ~HAS_ITEM;
        } else if (pmap[x][y].flags & HAS_MONSTER) {
            monst = monsterAtLoc(x, y);
            if (monst->carriedItem && monst->carriedItem == theItem) {
                monst->carriedItem = NULL;
                deleteItem(theItem);
            }
        }
    }
}

short randValidDirectionFrom(creature *monst, short x, short y, boolean respectAvoidancePreferences) {
    short i, newX, newY, validDirectionCount = 0, randIndex;

    brogueAssert(rogue.RNG == RNG_SUBSTANTIVE);
    for (i=0; i<8; i++) {
        newX = x + nbDirs[i][0];
        newY = y + nbDirs[i][1];
        if (coordinatesAreInMap(newX, newY)
            && !cellHasTerrainFlag(newX, newY, T_OBSTRUCTS_PASSABILITY)
            && !diagonalBlocked(x, y, newX, newY, false)
            && (!respectAvoidancePreferences
                || (!monsterAvoids(monst, newX, newY))
                || ((pmap[newX][newY].flags & HAS_PLAYER) && monst->creatureState != MONSTER_ALLY))) {
            validDirectionCount++;
        }
    }
    if (validDirectionCount == 0) {
        // Rare, and important in this case that the function returns BEFORE a random roll is made to avoid OOS.
        return NO_DIRECTION;
    }
    randIndex = rand_range(1, validDirectionCount);
    validDirectionCount = 0;
    for (i=0; i<8; i++) {
        newX = x + nbDirs[i][0];
        newY = y + nbDirs[i][1];
        if (coordinatesAreInMap(newX, newY)
            && !cellHasTerrainFlag(newX, newY, T_OBSTRUCTS_PASSABILITY)
            && !diagonalBlocked(x, y, newX, newY, false)
            && (!respectAvoidancePreferences
                || (!monsterAvoids(monst, newX, newY))
                || ((pmap[newX][newY].flags & HAS_PLAYER) && monst->creatureState != MONSTER_ALLY))) {
            validDirectionCount++;
            if (validDirectionCount == randIndex) {
                return i;
            }
        }
    }
    return NO_DIRECTION; // should rarely get here
}

void vomit(creature *monst) {
    char buf[COLS], monstName[COLS];
    spawnDungeonFeature(monst->xLoc, monst->yLoc, &dungeonFeatureCatalog[DF_VOMIT], true, false);

    if (canDirectlySeeMonster(monst)
        && !rogue.automationActive) {

        monsterName(monstName, monst, true);
        sprintf(buf, "%s vomit%s profusely", monstName, (monst == &player ? "" : "s"));
        combatMessage(buf, NULL);
    }
}

void moveEntrancedMonsters(enum directions dir) {
    creature *monst, *nextMonst;

    dir = oppositeDirection(dir);

    for (monst = monsters->nextCreature; monst != NULL; monst = nextMonst) {
        nextMonst = monst->nextCreature;
        if (monst->status[STATUS_ENTRANCED]
            && !monst->status[STATUS_STUCK]
            && !monst->status[STATUS_PARALYZED]
            && !(monst->bookkeepingFlags & MB_CAPTIVE)) {

            moveMonster(monst, nbDirs[dir][0], nbDirs[dir][1]);
        }
    }
}

void becomeAllyWith(creature *monst) {
    demoteMonsterFromLeadership(monst);
    // Drop your item.
    if (monst->carriedItem) {
        makeMonsterDropItem(monst);
    }
    // If you're going to change into something, it should be friendly.
    if (monst->carriedMonster) {
        becomeAllyWith(monst->carriedMonster);
    }
    monst->creatureState = MONSTER_ALLY;
    monst->bookkeepingFlags |= MB_FOLLOWER;
    monst->leader = &player;
    monst->bookkeepingFlags &= ~(MB_CAPTIVE | MB_SEIZED);
    refreshDungeonCell(monst->xLoc, monst->yLoc);
}

void freeCaptive(creature *monst) {
    char buf[COLS * 3], monstName[COLS];

    becomeAllyWith(monst);
    monsterName(monstName, monst, false);
    sprintf(buf, "you free the grateful %s and gain a faithful ally.", monstName);
    message(buf, false);
}

boolean freeCaptivesEmbeddedAt(short x, short y) {
    creature *monst;

    if (pmap[x][y].flags & HAS_MONSTER) {
        // Free any captives trapped in the tunnelized terrain.
        monst = monsterAtLoc(x, y);
        if ((monst->bookkeepingFlags & MB_CAPTIVE)
            && !(monst->info.flags & MONST_ATTACKABLE_THRU_WALLS)
            && (cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY))) {
            freeCaptive(monst);
            return true;
        }
    }
    return false;
}

// Do we need confirmation so we don't accidently hit an acid mound?
boolean abortAttackAgainstAcidicTarget(creature *hitList[8]) {
    short i;
    char monstName[COLS], weaponName[COLS];
    char buf[COLS*3];

    if (rogue.weapon
        && !(rogue.weapon->flags & ITEM_PROTECTED)
        && !player.status[STATUS_HALLUCINATING]
        && !player.status[STATUS_CONFUSED]) {

        for (i=0; i<8; i++) {
            if (hitList[i]
                && (hitList[i]->info.flags & MONST_DEFEND_DEGRADE_WEAPON)
                && canSeeMonster(hitList[i])
                && (!(rogue.weapon->flags & ITEM_RUNIC)
                    || !(rogue.weapon->flags & ITEM_RUNIC_IDENTIFIED)
                    || rogue.weapon->enchant2 != W_SLAYING
                    || !monsterIsInClass(hitList[i], rogue.weapon->vorpalEnemy))) {

                monsterName(monstName, hitList[i], true);
                itemName(rogue.weapon, weaponName, false, false, NULL);
                sprintf(buf, "Degrade your %s by attacking %s?", weaponName, monstName);
                if (confirm(buf, false)) {
                    return false; // Fire when ready!
                } else {
                    return true; // Abort!
                }
            }
        }
    }
    return false;
}

// Returns true if a whip attack was launched.
// If "aborted" pointer is provided, sets it to true if it was aborted because
// the player opted not to attack an acid mound (in which case the whole turn
// should be aborted), as opposed to there being no valid whip attack available
// (in which case the player/monster should move instead).
boolean handleWhipAttacks(creature *attacker, enum directions dir, boolean *aborted) {
    bolt theBolt;
    creature *defender, *hitList[8] = {0};
    short strikeLoc[2], originLoc[2], targetLoc[2];

    const char boltChar[DIRECTION_COUNT] = "||~~\\//\\";

    brogueAssert(dir > NO_DIRECTION && dir < DIRECTION_COUNT);

    if (attacker == &player) {
        if (!rogue.weapon || !(rogue.weapon->flags & ITEM_ATTACKS_EXTEND)) {
            return false;
        }
    } else if (!(attacker->info.abilityFlags & MA_ATTACKS_EXTEND)) {
        return false;
    }
    originLoc[0] = attacker->xLoc;
    originLoc[1] = attacker->yLoc;
    targetLoc[0] = attacker->xLoc + nbDirs[dir][0];
    targetLoc[1] = attacker->yLoc + nbDirs[dir][1];
    getImpactLoc(strikeLoc, originLoc, targetLoc, 5, false);

    defender = monsterAtLoc(strikeLoc[0], strikeLoc[1]);
    if (defender
        && (attacker != &player || canSeeMonster(defender))
        && !monsterIsHidden(defender, attacker)
        && monsterWillAttackTarget(attacker, defender)) {

        if (attacker == &player) {
            hitList[0] = defender;
            if (abortAttackAgainstAcidicTarget(hitList)) {
                if (aborted) {
                    *aborted = true;
                }
                return false;
            }
        }
        attacker->bookkeepingFlags &= ~MB_SUBMERGED;
        theBolt = boltCatalog[BOLT_WHIP];
        theBolt.theChar = boltChar[dir];
        zap(originLoc, targetLoc, &theBolt, false);
        return true;
    }
    return false;
}

// Returns true if a spear attack was launched.
// If "aborted" pointer is provided, sets it to true if it was aborted because
// the player opted not to attack an acid mound (in which case the whole turn
// should be aborted), as opposed to there being no valid spear attack available
// (in which case the player/monster should move instead).
boolean handleSpearAttacks(creature *attacker, enum directions dir, boolean *aborted) {
    creature *defender, *hitList[8] = {0};
    short targetLoc[2], range = 2, i = 0, h = 0;
    boolean proceed = false, visualEffect = false;

    const char boltChar[DIRECTION_COUNT] = "||--\\//\\";

    brogueAssert(dir > NO_DIRECTION && dir < DIRECTION_COUNT);

    if (attacker == &player) {
        if (!rogue.weapon || !(rogue.weapon->flags & ITEM_ATTACKS_PENETRATE)) {
            return false;
        }
    } else if (!(attacker->info.abilityFlags & MA_ATTACKS_PENETRATE)) {
        return false;
    }

    for (i = 0; i < range; i++) {
        targetLoc[0] = attacker->xLoc + (1 + i) * nbDirs[dir][0];
        targetLoc[1] = attacker->yLoc + (1 + i) * nbDirs[dir][1];
        if (!coordinatesAreInMap(targetLoc[0], targetLoc[1])) {
            break;
        }

        /* Add creatures that we are willing to attack to the potential
        hitlist. Any of those that are either right by us or visible will
        trigger the attack. */
        defender = monsterAtLoc(targetLoc[0], targetLoc[1]);
        if (defender
            && (!cellHasTerrainFlag(targetLoc[0], targetLoc[1], T_OBSTRUCTS_PASSABILITY)
                || (defender->info.flags & MONST_ATTACKABLE_THRU_WALLS))
            && monsterWillAttackTarget(attacker, defender)) {

            hitList[h++] = defender;

            /* We check if i=0, i.e. the defender is right next to us, because
            we have to do "normal" attacking here. We can't just return
            false and leave to playerMoves/moveMonster due to the collateral hitlist. */
            if (i == 0 || !monsterIsHidden(defender, attacker)
                && (attacker != &player || canSeeMonster(defender))) {
                // We'll attack.
                proceed = true;
            }
        }

        if (cellHasTerrainFlag(targetLoc[0], targetLoc[1], (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))) {
            break;
        }
    }
    range = i;
    if (proceed) {
        if (attacker == &player) {
            if (abortAttackAgainstAcidicTarget(hitList)) {
                if (aborted) {
                    *aborted = true;
                }
                return false;
            }
        }
        if (!rogue.playbackFastForward) {
            for (i = 0; i < range; i++) {
                targetLoc[0] = attacker->xLoc + (1 + i) * nbDirs[dir][0];
                targetLoc[1] = attacker->yLoc + (1 + i) * nbDirs[dir][1];
                if (coordinatesAreInMap(targetLoc[0], targetLoc[1])
                    && playerCanSeeOrSense(targetLoc[0], targetLoc[1])) {

                    visualEffect = true;
                    plotForegroundChar(boltChar[dir], targetLoc[0], targetLoc[1], &lightBlue, true);
                }
            }
        }
        attacker->bookkeepingFlags &= ~MB_SUBMERGED;
        // Artificially reverse the order of the attacks,
        // so that spears of force can send both monsters flying.
        for (i = h - 1; i >= 0; i--) {
            attack(attacker, hitList[i], false);
        }
        if (visualEffect) {
            pauseBrogue(16);
            for (i = 0; i < range; i++) {
                targetLoc[0] = attacker->xLoc + (1 + i) * nbDirs[dir][0];
                targetLoc[1] = attacker->yLoc + (1 + i) * nbDirs[dir][1];
                if (coordinatesAreInMap(targetLoc[0], targetLoc[1])) {
                    refreshDungeonCell(targetLoc[0], targetLoc[1]);
                }
            }
        }
        return true;
    }
    return false;
}

void buildFlailHitList(const short x, const short y, const short newX, const short newY, creature *hitList[16]) {
    creature *monst;
    short mx, my;
    short i = 0;

    for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
        mx = monst->xLoc;
        my = monst->yLoc;
        if (distanceBetween(x, y, mx, my) == 1
            && distanceBetween(newX, newY, mx, my) == 1
            && canSeeMonster(monst)
            && monstersAreEnemies(&player, monst)
            && monst->creatureState != MONSTER_ALLY
            && !(monst->bookkeepingFlags & MB_IS_DYING)
            && (!cellHasTerrainFlag(monst->xLoc, monst->yLoc, T_OBSTRUCTS_PASSABILITY) || (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS))) {

            while (hitList[i]) {
                i++;
            }
            hitList[i] = monst;
        }
    }
}

boolean diagonalBlocked(const short x1, const short y1, const short x2, const short y2, const boolean limitToPlayerKnowledge) {
    unsigned long tFlags;
    if (x1 == x2 || y1 == y2) {
        return false; // If it's not a diagonal, it's not diagonally blocked.
    }
    getLocationFlags(x1, y2, &tFlags, NULL, NULL, limitToPlayerKnowledge);
    if (tFlags & T_OBSTRUCTS_DIAGONAL_MOVEMENT) {
        return true;
    }
    getLocationFlags(x2, y1, &tFlags, NULL, NULL, limitToPlayerKnowledge);
    if (tFlags & T_OBSTRUCTS_DIAGONAL_MOVEMENT) {
        return true;
    }
    return false;
}

// Called whenever the player voluntarily tries to move in a given direction.
// Can be called from movement keys, exploration, or auto-travel.
boolean playerMoves(short direction) {
    short initialDirection = direction, i, layer;
    short x = player.xLoc, y = player.yLoc;
    short newX, newY, newestX, newestY;
    boolean playerMoved = false, alreadyRecorded = false, specialAttackAborted = false, anyAttackHit = false;
    creature *defender = NULL, *tempMonst = NULL, *hitList[16] = {NULL};
    char monstName[COLS];
    char buf[COLS*3];
    const int directionKeys[8] = {UP_KEY, DOWN_KEY, LEFT_KEY, RIGHT_KEY, UPLEFT_KEY, DOWNLEFT_KEY, UPRIGHT_KEY, DOWNRIGHT_KEY};

    brogueAssert(direction >= 0 && direction < DIRECTION_COUNT);

    newX = x + nbDirs[direction][0];
    newY = y + nbDirs[direction][1];

    if (!coordinatesAreInMap(newX, newY)) {
        return false;
    }

    if (player.status[STATUS_CONFUSED]) {
        // Confirmation dialog if you're moving while confused and you're next to lava and not levitating or immune to fire.
        if (player.status[STATUS_LEVITATING] <= 1
            && player.status[STATUS_IMMUNE_TO_FIRE] <= 1) {

            for (i=0; i<8; i++) {
                newestX = x + nbDirs[i][0];
                newestY = y + nbDirs[i][1];
                if (coordinatesAreInMap(newestX, newestY)
                    && (pmap[newestX][newestY].flags & (DISCOVERED | MAGIC_MAPPED))
                    && !diagonalBlocked(x, y, newestX, newestY, false)
                    && cellHasTerrainFlag(newestX, newestY, T_LAVA_INSTA_DEATH)
                    && !cellHasTerrainFlag(newestX, newestY, T_OBSTRUCTS_PASSABILITY | T_ENTANGLES)
                    && !((pmap[newestX][newestY].flags & HAS_MONSTER)
                         && canSeeMonster(monsterAtLoc(newestX, newestY))
                         && monsterAtLoc(newestX, newestY)->creatureState != MONSTER_ALLY)) {

                    if (!confirm("Risk stumbling into lava?", false)) {
                        return false;
                    } else {
                        break;
                    }
                }
            }
        }

        direction = randValidDirectionFrom(&player, x, y, false);
        if (direction == -1) {
            return false;
        } else {
            newX = x + nbDirs[direction][0];
            newY = y + nbDirs[direction][1];
            if (!coordinatesAreInMap(newX, newY)) {
                return false;
            }
            if (!alreadyRecorded) {
                recordKeystroke(directionKeys[initialDirection], false, false);
                alreadyRecorded = true;
            }
        }
    }

    if (pmap[newX][newY].flags & HAS_MONSTER) {
        defender = monsterAtLoc(newX, newY);
    }

    // If there's no enemy at the movement location that the player is aware of, consider terrain promotions.
    if (!defender
        || (!canSeeMonster(defender) && !monsterRevealed(defender))
        || !monstersAreEnemies(&player, defender)) {

        if (cellHasTerrainFlag(newX, newY, T_OBSTRUCTS_PASSABILITY) && cellHasTMFlag(newX, newY, TM_PROMOTES_ON_PLAYER_ENTRY)) {
            layer = layerWithTMFlag(newX, newY, TM_PROMOTES_ON_PLAYER_ENTRY);
            if (tileCatalog[pmap[newX][newY].layers[layer]].flags & T_OBSTRUCTS_PASSABILITY) {
                if (!alreadyRecorded) {
                    recordKeystroke(directionKeys[initialDirection], false, false);
                    alreadyRecorded = true;
                }
                message(tileCatalog[pmap[newX][newY].layers[layer]].flavorText, false);
                promoteTile(newX, newY, layer, false);
                playerTurnEnded();
                return true;
            }
        }

        if (player.status[STATUS_STUCK] && cellHasTerrainFlag(x, y, T_ENTANGLES)) {
                // Don't interrupt exploration with this message.
            if (--player.status[STATUS_STUCK]) {
                if (!rogue.automationActive) {
                    message("you struggle but cannot free yourself.", false);
                }
            } else {
                if (!rogue.automationActive) {
                    message("you break free!", false);
                }
                if (tileCatalog[pmap[x][y].layers[SURFACE]].flags & T_ENTANGLES) {
                    pmap[x][y].layers[SURFACE] = NOTHING;
                }
            }
            moveEntrancedMonsters(direction);
            if (!alreadyRecorded) {
                recordKeystroke(directionKeys[initialDirection], false, false);
                alreadyRecorded = true;
            }
            if (player.status[STATUS_STUCK]) {
                playerTurnEnded();
                return true;
            }
        }
    }

    if (((!cellHasTerrainFlag(newX, newY, T_OBSTRUCTS_PASSABILITY) || (cellHasTMFlag(newX, newY, TM_PROMOTES_WITH_KEY) && keyInPackFor(newX, newY)))
         && !diagonalBlocked(x, y, newX, newY, false)
         && (!cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY) || (cellHasTMFlag(x, y, TM_PROMOTES_WITH_KEY) && keyInPackFor(x, y))))
        || (defender && defender->info.flags & MONST_ATTACKABLE_THRU_WALLS)) {
        // if the move is not blocked

        if (handleWhipAttacks(&player, direction, &specialAttackAborted)
            || handleSpearAttacks(&player, direction, &specialAttackAborted)) {

            if (!alreadyRecorded) {
                recordKeystroke(directionKeys[initialDirection], false, false);
                alreadyRecorded = true;
            }
            playerRecoversFromAttacking(true);
            moveEntrancedMonsters(direction);
            playerTurnEnded();
            return true;
        } else if (specialAttackAborted) { // Canceled an attack against an acid mound.
            brogueAssert(!alreadyRecorded);
            rogue.disturbed = true;
            return false;
        }

        if (defender) {
            // if there is a monster there

            if (defender->bookkeepingFlags & MB_CAPTIVE) {
                monsterName(monstName, defender, false);
                sprintf(buf, "Free the captive %s?", monstName);
                if (alreadyRecorded || confirm(buf, false)) {
                    if (!alreadyRecorded) {
                        recordKeystroke(directionKeys[initialDirection], false, false);
                        alreadyRecorded = true;
                    }
                    if (cellHasTMFlag(newX, newY, TM_PROMOTES_WITH_KEY) && keyInPackFor(newX, newY)) {
                        useKeyAt(keyInPackFor(newX, newY), newX, newY);
                    }
                    freeCaptive(defender);
                    player.ticksUntilTurn += player.attackSpeed;
                    playerTurnEnded();
                    return true;
                } else {
                    return false;
                }
            }

            if (defender->creatureState != MONSTER_ALLY) {
                // Make a hit list of monsters the player is attacking this turn.
                // We separate this tallying phase from the actual attacking phase because sometimes the attacks themselves
                // create more monsters, and those shouldn't be attacked in the same turn.

                buildHitList(hitList, &player, defender,
                             rogue.weapon && (rogue.weapon->flags & ITEM_ATTACKS_ALL_ADJACENT));

                if (abortAttackAgainstAcidicTarget(hitList)) { // Acid mound attack confirmation.
                    brogueAssert(!alreadyRecorded);
                    rogue.disturbed = true;
                    return false;
                }

                if (player.status[STATUS_NAUSEOUS]) {
                    if (!alreadyRecorded) {
                        recordKeystroke(directionKeys[initialDirection], false, false);
                        alreadyRecorded = true;
                    }
                    if (rand_percent(25)) {
                        vomit(&player);
                        playerTurnEnded();
                        return false;
                    }
                }

                // Proceeding with the attack.

                if (!alreadyRecorded) {
                    recordKeystroke(directionKeys[initialDirection], false, false);
                    alreadyRecorded = true;
                }

                // Attack!
                for (i=0; i<16; i++) {
                    if (hitList[i]
                        && monsterWillAttackTarget(&player, hitList[i])
                        && !(hitList[i]->bookkeepingFlags & MB_IS_DYING)
                        && !rogue.gameHasEnded) {

                        if (attack(&player, hitList[i], false)) {
                            anyAttackHit = true;
                        }
                    }
                }

                playerRecoversFromAttacking(anyAttackHit);
                moveEntrancedMonsters(direction);
                playerTurnEnded();
                return true;
            }
        }

        if (player.bookkeepingFlags & MB_SEIZED) {
            for (tempMonst = monsters->nextCreature; tempMonst != NULL; tempMonst = tempMonst->nextCreature) {
                if ((tempMonst->bookkeepingFlags & MB_SEIZING)
                    && monstersAreEnemies(&player, tempMonst)
                    && distanceBetween(player.xLoc, player.yLoc, tempMonst->xLoc, tempMonst->yLoc) == 1
                    && !diagonalBlocked(player.xLoc, player.yLoc, tempMonst->xLoc, tempMonst->yLoc, false)
                    && !tempMonst->status[STATUS_ENTRANCED]) {

                    monsterName(monstName, tempMonst, true);
                    if (alreadyRecorded || !canSeeMonster(tempMonst)) {
                        if (!alreadyRecorded) {
                            recordKeystroke(directionKeys[initialDirection], false, false);
                            alreadyRecorded = true;
                        }
                        sprintf(buf, "you struggle but %s is holding your legs!", monstName);
                        moveEntrancedMonsters(direction);
                        message(buf, false);
                        playerTurnEnded();
                        return true;
                    } else {
                        sprintf(buf, "you cannot move; %s is holding your legs!", monstName);
                        message(buf, false);
                        return false;
                    }
                }
            }
            player.bookkeepingFlags &= ~MB_SEIZED; // failsafe
        }

        if (pmap[newX][newY].flags & (DISCOVERED | MAGIC_MAPPED)
            && player.status[STATUS_LEVITATING] <= 1
            && !player.status[STATUS_CONFUSED]
            && cellHasTerrainFlag(newX, newY, T_LAVA_INSTA_DEATH)
            && player.status[STATUS_IMMUNE_TO_FIRE] <= 1
            && !cellHasTerrainFlag(newX, newY, T_ENTANGLES)
            && !cellHasTMFlag(newX, newY, TM_IS_SECRET)) {
            message("that would be certain death!", false);
            return false; // player won't willingly step into lava
        } else if (pmap[newX][newY].flags & (DISCOVERED | MAGIC_MAPPED)
                   && player.status[STATUS_LEVITATING] <= 1
                   && !player.status[STATUS_CONFUSED]
                   && cellHasTerrainFlag(newX, newY, T_AUTO_DESCENT)
                   && !cellHasTerrainFlag(newX, newY, T_ENTANGLES)
                   && !cellHasTMFlag(newX, newY, TM_IS_SECRET)
                   && !confirm("Dive into the depths?", false)) {
            return false;
        } else if (playerCanSee(newX, newY)
                   && !player.status[STATUS_CONFUSED]
                   && !player.status[STATUS_BURNING]
                   && player.status[STATUS_IMMUNE_TO_FIRE] <= 1
                   && cellHasTerrainFlag(newX, newY, T_IS_FIRE)
                   && !cellHasTMFlag(newX, newY, TM_EXTINGUISHES_FIRE)
                   && !confirm("Venture into flame?", false)) {
            return false;
        } else if (playerCanSee(newX, newY)
                   && !player.status[STATUS_CONFUSED]
                   && !player.status[STATUS_BURNING]
                   && cellHasTerrainFlag(newX, newY, T_CAUSES_CONFUSION | T_CAUSES_PARALYSIS)
                   && (!rogue.armor || !(rogue.armor->flags & ITEM_RUNIC) || !(rogue.armor->flags & ITEM_RUNIC_IDENTIFIED) || rogue.armor->enchant2 != A_RESPIRATION)
                   && !confirm("Venture into dangerous gas?", false)) {
            return false;
        } else if (pmap[newX][newY].flags & (ANY_KIND_OF_VISIBLE | MAGIC_MAPPED)
                   && player.status[STATUS_LEVITATING] <= 1
                   && !player.status[STATUS_CONFUSED]
                   && cellHasTerrainFlag(newX, newY, T_IS_DF_TRAP)
                   && !(pmap[newX][newY].flags & PRESSURE_PLATE_DEPRESSED)
                   && !cellHasTMFlag(newX, newY, TM_IS_SECRET)
                   && !confirm("Step onto the pressure plate?", false)) {
            return false;
        }

        if (rogue.weapon && (rogue.weapon->flags & ITEM_LUNGE_ATTACKS)) {
            newestX = player.xLoc + 2*nbDirs[direction][0];
            newestY = player.yLoc + 2*nbDirs[direction][1];
            if (coordinatesAreInMap(newestX, newestY) && (pmap[newestX][newestY].flags & HAS_MONSTER)) {
                tempMonst = monsterAtLoc(newestX, newestY);
                if (tempMonst
                    && canSeeMonster(tempMonst)
                    && monstersAreEnemies(&player, tempMonst)
                    && tempMonst->creatureState != MONSTER_ALLY
                    && !(tempMonst->bookkeepingFlags & MB_IS_DYING)
                    && (!cellHasTerrainFlag(tempMonst->xLoc, tempMonst->yLoc, T_OBSTRUCTS_PASSABILITY) || (tempMonst->info.flags & MONST_ATTACKABLE_THRU_WALLS))) {

                    hitList[0] = tempMonst;
                    if (abortAttackAgainstAcidicTarget(hitList)) { // Acid mound attack confirmation.
                        brogueAssert(!alreadyRecorded);
                        rogue.disturbed = true;
                        return false;
                    }
                }
            }
        }
        if (rogue.weapon && (rogue.weapon->flags & ITEM_PASS_ATTACKS)) {
            buildFlailHitList(x, y, newX, newY, hitList);
            if (abortAttackAgainstAcidicTarget(hitList)) { // Acid mound attack confirmation.
                brogueAssert(!alreadyRecorded);
                rogue.disturbed = true;
                return false;
            }
        }

        if (player.status[STATUS_NAUSEOUS]) {
            if (!alreadyRecorded) {
                recordKeystroke(directionKeys[initialDirection], false, false);
                alreadyRecorded = true;
            }
            if (rand_percent(25)) {
                vomit(&player);
                playerTurnEnded();
                return true;
            }
        }

        // Are we taking the stairs?
        if (rogue.downLoc[0] == newX && rogue.downLoc[1] == newY) {
            if (!alreadyRecorded) {
                recordKeystroke(directionKeys[initialDirection], false, false);
                alreadyRecorded = true;
            }
            useStairs(1);
        } else if (rogue.upLoc[0] == newX && rogue.upLoc[1] == newY) {
            if (!alreadyRecorded) {
                recordKeystroke(directionKeys[initialDirection], false, false);
                alreadyRecorded = true;
            }
            useStairs(-1);
        } else {
            // Okay, we're finally moving!
            if (!alreadyRecorded) {
                recordKeystroke(directionKeys[initialDirection], false, false);
                alreadyRecorded = true;
            }

            player.xLoc += nbDirs[direction][0];
            player.yLoc += nbDirs[direction][1];
            pmap[x][y].flags &= ~HAS_PLAYER;
            pmap[player.xLoc][player.yLoc].flags |= HAS_PLAYER;
            pmap[player.xLoc][player.yLoc].flags &= ~IS_IN_PATH;
            if (defender && defender->creatureState == MONSTER_ALLY) { // Swap places with ally.
                pmap[defender->xLoc][defender->yLoc].flags &= ~HAS_MONSTER;
                defender->xLoc = x;
                defender->yLoc = y;
                if (monsterAvoids(defender, x, y)) {
                    getQualifyingPathLocNear(&(defender->xLoc), &(defender->yLoc), player.xLoc, player.yLoc, true, forbiddenFlagsForMonster(&(defender->info)), 0, 0, (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS), false);
                }
                //getQualifyingLocNear(loc, player.xLoc, player.yLoc, true, NULL, forbiddenFlagsForMonster(&(defender->info)) & ~(T_IS_DF_TRAP | T_IS_DEEP_WATER | T_SPONTANEOUSLY_IGNITES), HAS_MONSTER, false, false);
                //defender->xLoc = loc[0];
                //defender->yLoc = loc[1];
                pmap[defender->xLoc][defender->yLoc].flags |= HAS_MONSTER;
            }

            if (pmap[player.xLoc][player.yLoc].flags & HAS_ITEM) {
                pickUpItemAt(player.xLoc, player.yLoc);
                rogue.disturbed = true;
            }
            refreshDungeonCell(x, y);
            refreshDungeonCell(player.xLoc, player.yLoc);
            playerMoved = true;

            checkForMissingKeys(x, y);
            if (monsterShouldFall(&player)) {
                player.bookkeepingFlags |= MB_IS_FALLING;
            }
            moveEntrancedMonsters(direction);

            // Perform a lunge or flail attack if appropriate.
            for (i=0; i<16; i++) {
                if (hitList[i]) {
                    if (attack(&player, hitList[i], (rogue.weapon && (rogue.weapon->flags & ITEM_LUNGE_ATTACKS)))) {
                        anyAttackHit = true;
                    }
                }
            }
            if (hitList[0]) {
                playerRecoversFromAttacking(anyAttackHit);
            }

            playerTurnEnded();
        }
    } else if (cellHasTerrainFlag(newX, newY, T_OBSTRUCTS_PASSABILITY)) {
        i = pmap[newX][newY].layers[layerWithFlag(newX, newY, T_OBSTRUCTS_PASSABILITY)];
        if ((tileCatalog[i].flags & T_OBSTRUCTS_PASSABILITY)
            && (!diagonalBlocked(x, y, newX, newY, false) || !cellHasTMFlag(newX, newY, TM_PROMOTES_WITH_KEY))) {

            if (!(pmap[newX][newY].flags & DISCOVERED)) {
                if (!alreadyRecorded) {
                    recordKeystroke(directionKeys[initialDirection], false, false);
                    alreadyRecorded = true;
                }
                discoverCell(newX, newY);
                refreshDungeonCell(newX, newY);
            }

            messageWithColor(tileCatalog[i].flavorText, &backgroundMessageColor, false);
        }
    }
    return playerMoved;
}

// replaced in Dijkstra.c:
/*
// returns true if the cell value changed
boolean updateDistanceCell(short **distanceMap, short x, short y) {
    short dir, newX, newY;
    boolean somethingChanged = false;

    if (distanceMap[x][y] >= 0 && distanceMap[x][y] < 30000) {
        for (dir=0; dir< DIRECTION_COUNT; dir++) {
            newX = x + nbDirs[dir][0];
            newY = y + nbDirs[dir][1];
            if (coordinatesAreInMap(newX, newY)
                && distanceMap[newX][newY] >= distanceMap[x][y] + 2
                && !diagonalBlocked(x, y, newX, newY)) {
                distanceMap[newX][newY] = distanceMap[x][y] + 1;
                somethingChanged = true;
            }
        }
    }
    return somethingChanged;
}

void dijkstraScan(short **distanceMap, char passMap[DCOLS][DROWS], boolean allowDiagonals) {
    short i, j, maxDir;
    enum directions dir;
    boolean somethingChanged;

    maxDir = (allowDiagonals ? 8 : 4);

    do {
        somethingChanged = false;
        for (i=1; i<DCOLS-1; i++) {
            for (j=1; j<DROWS-1; j++) {
                if (!passMap || passMap[i][j]) {
                    for (dir = 0; dir < maxDir; dir++) {
                        if (coordinatesAreInMap(i + nbDirs[dir][0], j + nbDirs[dir][1])
                            && (!passMap || passMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]])
                            && distanceMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]] >= distanceMap[i][j] + 2) {
                            distanceMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]] = distanceMap[i][j] + 1;
                            somethingChanged = true;
                        }
                    }
                }
            }
        }


        for (i = DCOLS - 1; i >= 0; i--) {
            for (j = DROWS - 1; j >= 0; j--) {
                if (!passMap || passMap[i][j]) {
                    for (dir = 0; dir < maxDir; dir++) {
                        if (coordinatesAreInMap(i + nbDirs[dir][0], j + nbDirs[dir][1])
                            && (!passMap || passMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]])
                            && distanceMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]] >= distanceMap[i][j] + 2) {
                            distanceMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]] = distanceMap[i][j] + 1;
                            somethingChanged = true;
                        }
                    }
                }
            }
        }
    } while (somethingChanged);
}*/

/*void enqueue(short x, short y, short val, distanceQueue *dQ) {
    short *qX2, *qY2, *qVal2;

    // if we need to allocate more memory:
    if (dQ->qLen + 1 > dQ->qMaxLen) {
        dQ->qMaxLen *= 2;
        qX2 = realloc(dQ->qX, dQ->qMaxLen);
        if (qX2) {
            free(dQ->qX);
            dQ->qX = qX2;
        } else {
            // out of memory
        }
        qY2 = realloc(dQ->qY, dQ->qMaxLen);
        if (qY2) {
            free(dQ->qY);
            dQ->qY = qY2;
        } else {
            // out of memory
        }
        qVal2 = realloc(dQ->qVal, dQ->qMaxLen);
        if (qVal2) {
            free(dQ->qVal);
            dQ->qVal = qVal2;
        } else {
            // out of memory
        }
    }

    dQ->qX[dQ->qLen] = x;
    dQ->qY[dQ->qLen] = y;
    (dQ->qVal)[dQ->qLen] = val;

    dQ->qLen++;

    if (val < dQ->qMinVal) {
        dQ->qMinVal = val;
        dQ->qMinCount = 1;
    } else if (val == dQ->qMinVal) {
        dQ->qMinCount++;
    }
}

void updateQueueMinCache(distanceQueue *dQ) {
    short i;
    dQ->qMinCount = 0;
    dQ->qMinVal = 30001;
    for (i = 0; i < dQ->qLen; i++) {
        if (dQ->qVal[i] < dQ->qMinVal) {
            dQ->qMinVal = dQ->qVal[i];
            dQ->qMinCount = 1;
        } else if (dQ->qVal[i] == dQ->qMinVal) {
            dQ->qMinCount++;
        }
    }
}

// removes the lowest value from the queue, populates x/y/value variables and updates min caching
void dequeue(short *x, short *y, short *val, distanceQueue *dQ) {
    short i, minIndex;

    if (dQ->qMinCount <= 0) {
        updateQueueMinCache(dQ);
    }

    *val = dQ->qMinVal;

    // find the last instance of the minVal
    for (minIndex = dQ->qLen - 1; minIndex >= 0 && dQ->qVal[minIndex] != *val; minIndex--);

    // populate the return variables
    *x = dQ->qX[minIndex];
    *y = dQ->qY[minIndex];

    dQ->qLen--;

    // delete the minValue queue entry
    for (i = minIndex; i < dQ->qLen; i++) {
        dQ->qX[i] = dQ->qX[i+1];
        dQ->qY[i] = dQ->qY[i+1];
        dQ->qVal[i] = dQ->qVal[i+1];
    }

    // update min values
    dQ->qMinCount--;
    if (!dQ->qMinCount && dQ->qLen) {
        updateQueueMinCache(dQ);
    }

}

void dijkstraScan(short **distanceMap, char passMap[DCOLS][DROWS], boolean allowDiagonals) {
    short i, j, maxDir, val;
    enum directions dir;
    distanceQueue dQ;

    dQ.qMaxLen = DCOLS * DROWS * 1.5;
    dQ.qX = (short *) malloc(dQ.qMaxLen * sizeof(short));
    dQ.qY = (short *) malloc(dQ.qMaxLen * sizeof(short));
    dQ.qVal = (short *) malloc(dQ.qMaxLen * sizeof(short));
    dQ.qLen = 0;
    dQ.qMinVal = 30000;
    dQ.qMinCount = 0;

    maxDir = (allowDiagonals ? 8 : 4);

    // seed the queue with the entire map
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (!passMap || passMap[i][j]) {
                enqueue(i, j, distanceMap[i][j], &dQ);
            }
        }
    }

    // iterate through queue updating lowest entries until the queue is empty
    while (dQ.qLen) {
        dequeue(&i, &j, &val, &dQ);
        if (distanceMap[i][j] == val) { // if it hasn't been improved since joining the queue
            for (dir = 0; dir < maxDir; dir++) {
                if (coordinatesAreInMap(i + nbDirs[dir][0], j + nbDirs[dir][1])
                    && (!passMap || passMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]])
                    && distanceMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]] >= distanceMap[i][j] + 2) {

                    distanceMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]] = distanceMap[i][j] + 1;

                    enqueue(i + nbDirs[dir][0], j + nbDirs[dir][1], distanceMap[i][j] + 1, &dQ);
                }
            }
        }
    }

    free(dQ.qX);
    free(dQ.qY);
    free(dQ.qVal);
}*/

/*
void calculateDistances(short **distanceMap, short destinationX, short destinationY, unsigned long blockingTerrainFlags, creature *traveler) {
    short i, j;
    boolean somethingChanged;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            distanceMap[i][j] = ((traveler && traveler == &player && !(pmap[i][j].flags & (DISCOVERED | MAGIC_MAPPED)))
                                 || ((traveler && monsterAvoids(traveler, i, j))
                                     || cellHasTerrainFlag(i, j, blockingTerrainFlags))) ? -1 : 30000;
        }
    }

    distanceMap[destinationX][destinationY] = 0;

//  dijkstraScan(distanceMap);
    do {
        somethingChanged = false;
        for (i=0; i<DCOLS; i++) {
            for (j=0; j<DROWS; j++) {
                if (updateDistanceCell(distanceMap, i, j)) {
                    somethingChanged = true;
                }
            }
        }


        for (i = DCOLS - 1; i >= 0; i--) {
            for (j = DROWS - 1; j >= 0; j--) {
                if (updateDistanceCell(distanceMap, i, j)) {
                    somethingChanged = true;
                }
            }
        }
    } while (somethingChanged);
}*/

// Returns -1 if there are no beneficial moves.
// If preferDiagonals is true, we will prefer diagonal moves.
// Always rolls downhill on the distance map.
// If monst is provided, do not return a direction pointing to
// a cell that the monster avoids.
short nextStep(short **distanceMap, short x, short y, creature *monst, boolean preferDiagonals) {
    short newX, newY, bestScore;
    enum directions dir, bestDir;
    creature *blocker;
    boolean blocked;

    brogueAssert(coordinatesAreInMap(x, y));

    bestScore = 0;
    bestDir = NO_DIRECTION;

    for (dir = (preferDiagonals ? 7 : 0);
         (preferDiagonals ? dir >= 0 : dir < DIRECTION_COUNT);
         (preferDiagonals ? dir-- : dir++)) {

        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];

        brogueAssert(coordinatesAreInMap(newX, newY));
        if (coordinatesAreInMap(newX, newY)) {
            blocked = false;
            blocker = monsterAtLoc(newX, newY);
            if (monst
                && monsterAvoids(monst, newX, newY)) {

                blocked = true;
            } else if (monst
                       && blocker
                       && !canPass(monst, blocker)
                       && !monstersAreTeammates(monst, blocker)
                       && !monstersAreEnemies(monst, blocker)) {
                blocked = true;
            }
            if ((distanceMap[x][y] - distanceMap[newX][newY]) > bestScore
                && !diagonalBlocked(x, y, newX, newY, monst == &player)
                && knownToPlayerAsPassableOrSecretDoor(newX, newY)
                && !blocked) {

                bestDir = dir;
                bestScore = distanceMap[x][y] - distanceMap[newX][newY];
            }
        }
    }
    return bestDir;
}

void displayRoute(short **distanceMap, boolean removeRoute) {
    short currentX = player.xLoc, currentY = player.yLoc, dir, newX, newY;
    boolean advanced;

    if (distanceMap[player.xLoc][player.yLoc] < 0 || distanceMap[player.xLoc][player.yLoc] == 30000) {
        return;
    }
    do {
        if (removeRoute) {
            refreshDungeonCell(currentX, currentY);
        } else {
            hiliteCell(currentX, currentY, &hiliteColor, 50, true);
        }
        advanced = false;
        for (dir = 7; dir >= 0; dir--) {
            newX = currentX + nbDirs[dir][0];
            newY = currentY + nbDirs[dir][1];
            if (coordinatesAreInMap(newX, newY)
                && distanceMap[newX][newY] >= 0 && distanceMap[newX][newY] < distanceMap[currentX][currentY]
                && !diagonalBlocked(currentX, currentY, newX, newY, true)) {

                currentX = newX;
                currentY = newY;
                advanced = true;
                break;
            }
        }
    } while (advanced);
}

void travelRoute(short path[1000][2], short steps) {
    short i, j;
    short dir;
    creature *monst;

    brogueAssert(!rogue.playbackMode);

    rogue.disturbed = false;
    rogue.automationActive = true;

    for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
        if (canSeeMonster(monst)) {
            monst->bookkeepingFlags |= MB_ALREADY_SEEN;
        } else {
            monst->bookkeepingFlags &= ~MB_ALREADY_SEEN;
        }
    }

    for (i=0; i < steps && !rogue.disturbed; i++) {
        for (j = i + 1; j < steps - 1; j++) {
            // Check to see if the path has become obstructed or avoided since the last time we saw it.
            if (diagonalBlocked(path[j-1][0], path[j-1][1], path[j][0], path[j][1], true)
                || monsterAvoids(&player, path[j][0], path[j][1])) {

                rogue.disturbed = true;
                break;
            }
        }
        for (dir = 0; dir < DIRECTION_COUNT && !rogue.disturbed; dir++) {
            if (player.xLoc + nbDirs[dir][0] == path[i][0]
                && player.yLoc + nbDirs[dir][1] == path[i][1]) {

                if (!playerMoves(dir)) {
                    rogue.disturbed = true;
                }
                if (pauseBrogue(25)) {
                    rogue.disturbed = true;
                }
                break;
            }
        }
    }
    rogue.disturbed = true;
    rogue.automationActive = false;
    updateFlavorText();
}

void travelMap(short **distanceMap) {
    short currentX = player.xLoc, currentY = player.yLoc, dir, newX, newY;
    boolean advanced;

    rogue.disturbed = false;
    rogue.automationActive = true;

    if (distanceMap[player.xLoc][player.yLoc] < 0 || distanceMap[player.xLoc][player.yLoc] == 30000) {
        return;
    }
    do {
        advanced = false;
        for (dir = 7; dir >= 0; dir--) {
            newX = currentX + nbDirs[dir][0];
            newY = currentY + nbDirs[dir][1];
            if (coordinatesAreInMap(newX, newY)
                && distanceMap[newX][newY] >= 0
                && distanceMap[newX][newY] < distanceMap[currentX][currentY]
                && !diagonalBlocked(currentX, currentY, newX, newY, true)) {

                if (!playerMoves(dir)) {
                    rogue.disturbed = true;
                }
                if (pauseBrogue(500)) {
                    rogue.disturbed = true;
                }
                currentX = newX;
                currentY = newY;
                advanced = true;
                break;
            }
        }
    } while (advanced && !rogue.disturbed);
    rogue.disturbed = true;
    rogue.automationActive = false;
    updateFlavorText();
}

void travel(short x, short y, boolean autoConfirm) {
    short **distanceMap, i;
    rogueEvent theEvent;
    unsigned short staircaseConfirmKey;

    confirmMessages();

    if (D_WORMHOLING) {
        recordMouseClick(mapToWindowX(x), mapToWindowY(y), true, false);
        pmap[player.xLoc][player.yLoc].flags &= ~HAS_PLAYER;
        refreshDungeonCell(player.xLoc, player.yLoc);
        player.xLoc = x;
        player.yLoc = y;
        pmap[x][y].flags |= HAS_PLAYER;
        updatePlayerUnderwaterness();
        refreshDungeonCell(x, y);
        updateVision(true);
        return;
    }

    if (abs(player.xLoc - x) + abs(player.yLoc - y) == 1) {
        // targeting a cardinal neighbor
        for (i=0; i<4; i++) {
            if (nbDirs[i][0] == (x - player.xLoc) && nbDirs[i][1] == (y - player.yLoc)) {
                playerMoves(i);
                break;
            }
        }
        return;
    }

    if (!(pmap[x][y].flags & (DISCOVERED | MAGIC_MAPPED))) {
        message("You have not explored that location.", false);
        return;
    }

    distanceMap = allocGrid();

    calculateDistances(distanceMap, x, y, 0, &player, false, false);
    if (distanceMap[player.xLoc][player.yLoc] < 30000) {
        if (autoConfirm) {
            travelMap(distanceMap);
            //refreshSideBar(-1, -1, false);
        } else {
            if (rogue.upLoc[0] == x && rogue.upLoc[1] == y) {
                staircaseConfirmKey = ASCEND_KEY;
            } else if (rogue.downLoc[0] == x && rogue.downLoc[1] == y) {
                staircaseConfirmKey = DESCEND_KEY;
            } else {
                staircaseConfirmKey = 0;
            }
            displayRoute(distanceMap, false);
            message("Travel this route? (y/n)", false);

            do {
                nextBrogueEvent(&theEvent, true, false, false);
            } while (theEvent.eventType != MOUSE_UP && theEvent.eventType != KEYSTROKE);

            displayRoute(distanceMap, true); // clear route display
            confirmMessages();

            if ((theEvent.eventType == MOUSE_UP && windowToMapX(theEvent.param1) == x && windowToMapY(theEvent.param2) == y)
                || (theEvent.eventType == KEYSTROKE && (theEvent.param1 == 'Y' || theEvent.param1 == 'y'
                                                        || theEvent.param1 == RETURN_KEY
                                                        || (theEvent.param1 == staircaseConfirmKey
                                                            && theEvent.param1 != 0)))) {
                travelMap(distanceMap);
                //refreshSideBar(-1, -1, false);
                commitDraws();
            } else if (theEvent.eventType == MOUSE_UP) {
                executeMouseClick(&theEvent);
            }
        }
//      if (player.xLoc == x && player.yLoc == y) {
//          rogue.cursorLoc[0] = rogue.cursorLoc[1] = 0;
//      } else {
//          rogue.cursorLoc[0] = x;
//          rogue.cursorLoc[1] = y;
//      }
    } else {
        rogue.cursorLoc[0] = rogue.cursorLoc[1] = -1;
        message("No path is available.", false);
    }
    freeGrid(distanceMap);
}

void populateGenericCostMap(short **costMap) {
    short i, j;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (cellHasTerrainFlag(i, j, T_OBSTRUCTS_PASSABILITY)
                && (!cellHasTMFlag(i, j, TM_IS_SECRET) || (discoveredTerrainFlagsAtLoc(i, j) & T_OBSTRUCTS_PASSABILITY))) {

                costMap[i][j] = cellHasTerrainFlag(i, j, T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
            } else if (cellHasTerrainFlag(i, j, T_PATHING_BLOCKER & ~T_OBSTRUCTS_PASSABILITY)) {
                costMap[i][j] = PDS_FORBIDDEN;
            } else {
                costMap[i][j] = 1;
            }
        }
    }
}

void getLocationFlags(const short x, const short y,
                      unsigned long *tFlags, unsigned long *TMFlags, unsigned long *cellFlags,
                      const boolean limitToPlayerKnowledge) {
    if (limitToPlayerKnowledge
        && (pmap[x][y].flags & (DISCOVERED | MAGIC_MAPPED))
        && !playerCanSee(x, y)) {

        if (tFlags) {
            *tFlags = pmap[x][y].rememberedTerrainFlags;
        }
        if (TMFlags) {
            *TMFlags = pmap[x][y].rememberedTMFlags;
        }
        if (cellFlags) {
            *cellFlags = pmap[x][y].rememberedCellFlags;
        }
    } else {
        if (tFlags) {
            *tFlags = terrainFlags(x, y);
        }
        if (TMFlags) {
            *TMFlags = terrainMechFlags(x, y);
        }
        if (cellFlags) {
            *cellFlags = pmap[x][y].flags;
        }
    }
}

void populateCreatureCostMap(short **costMap, creature *monst) {
    short i, j, unexploredCellCost;
    creature *currentTenant;
    item *theItem;
    unsigned long tFlags, cFlags;

    unexploredCellCost = 10 + (clamp(rogue.depthLevel, 5, 15) - 5) * 2;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (monst == &player && !(pmap[i][j].flags & (DISCOVERED | MAGIC_MAPPED))) {
                costMap[i][j] = PDS_OBSTRUCTION;
                continue;
            }

            getLocationFlags(i, j, &tFlags, NULL, &cFlags, monst == &player);

            if ((tFlags & T_OBSTRUCTS_PASSABILITY)
                 && (!cellHasTMFlag(i, j, TM_IS_SECRET) || (discoveredTerrainFlagsAtLoc(i, j) & T_OBSTRUCTS_PASSABILITY) || monst == &player)) {

                costMap[i][j] = (tFlags & T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
                continue;
            }

            if ((tFlags & T_LAVA_INSTA_DEATH)
                && !(monst->info.flags & (MONST_IMMUNE_TO_FIRE | MONST_FLIES | MONST_INVULNERABLE))
                && (monst->status[STATUS_LEVITATING] || monst->status[STATUS_IMMUNE_TO_FIRE])
                && max(monst->status[STATUS_LEVITATING], monst->status[STATUS_IMMUNE_TO_FIRE]) < (rogue.mapToShore[i][j] + distanceBetween(i, j, monst->xLoc, monst->yLoc) * monst->movementSpeed / 100)) {
                // Only a temporary effect will permit the monster to survive the lava, and the remaining duration either isn't
                // enough to get it to the spot, or it won't suffice to let it return to shore if it does get there.
                // Treat these locations as obstacles.
                costMap[i][j] = PDS_FORBIDDEN;
                continue;
            }

            if (((tFlags & T_AUTO_DESCENT) || (tFlags & T_IS_DEEP_WATER) && !(monst->info.flags & MONST_IMMUNE_TO_WATER))
                && !(monst->info.flags & MONST_FLIES)
                && (monst->status[STATUS_LEVITATING])
                && monst->status[STATUS_LEVITATING] < (rogue.mapToShore[i][j] + distanceBetween(i, j, monst->xLoc, monst->yLoc) * monst->movementSpeed / 100)) {
                // Only a temporary effect will permit the monster to levitate over the chasm/water, and the remaining duration either isn't
                // enough to get it to the spot, or it won't suffice to let it return to shore if it does get there.
                // Treat these locations as obstacles.
                costMap[i][j] = PDS_FORBIDDEN;
                continue;
            }

            if (monsterAvoids(monst, i, j)) {
                costMap[i][j] = PDS_FORBIDDEN;
                continue;
            }

            if (cFlags & HAS_MONSTER) {
                currentTenant = monsterAtLoc(i, j);
                if (currentTenant
                    && (currentTenant->info.flags & (MONST_IMMUNE_TO_WEAPONS | MONST_INVULNERABLE))
                    && !canPass(monst, currentTenant)) {

                    costMap[i][j] = PDS_FORBIDDEN;
                    continue;
                }
            }

            if ((cFlags & KNOWN_TO_BE_TRAP_FREE)
                || (monst != &player && monst->creatureState != MONSTER_ALLY)) {

                costMap[i][j] = 10;
            } else {
                // Player and allies give locations that are known to be free of traps
                // an advantage that increases with depth level, based on the depths
                // at which traps are generated.
                costMap[i][j] = unexploredCellCost;
            }

            if (!(monst->info.flags & MONST_INVULNERABLE)) {
                if ((tFlags & T_CAUSES_NAUSEA)
                    || cellHasTMFlag(i, j, TM_PROMOTES_ON_ITEM_PICKUP)
                    || (tFlags & T_ENTANGLES) && !(monst->info.flags & MONST_IMMUNE_TO_WEBS)) {

                    costMap[i][j] += 20;
                }
            }

            if (monst == &player) {
                theItem = itemAtLoc(i, j);
                if (theItem && (theItem->flags & ITEM_PLAYER_AVOIDS)) {
                    costMap[i][j] += 10;
                }
            }
        }
    }
}

enum directions adjacentFightingDir() {
    short newX, newY;
    enum directions dir;
    creature *monst;

    if (cellHasTerrainFlag(player.xLoc, player.yLoc, T_OBSTRUCTS_PASSABILITY)) {
        return NO_DIRECTION;
    }
    for (dir = 0; dir < DIRECTION_COUNT; dir++) {
        newX = player.xLoc + nbDirs[dir][0];
        newY = player.yLoc + nbDirs[dir][1];
        monst = monsterAtLoc(newX, newY);
        if (monst
            && canSeeMonster(monst)
            && (!diagonalBlocked(player.xLoc, player.yLoc, newX, newY, false) || (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS))
            && monstersAreEnemies(&player, monst)
            && !(monst->info.flags & (MONST_IMMUNE_TO_WEAPONS | MONST_INVULNERABLE))) {

            return dir;
        }
    }
    return NO_DIRECTION;
}

#define exploreGoalValue(x, y)  (0 - abs((x) - DCOLS / 2) / 3 - abs((x) - DCOLS / 2) / 4)

void getExploreMap(short **map, boolean headingToStairs) {// calculate explore map
    short i, j;
    short **costMap;
    item *theItem;

    costMap = allocGrid();
    populateCreatureCostMap(costMap, &player);

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            map[i][j] = 30000; // Can be overridden later.
            theItem = itemAtLoc(i, j);
            if (!(pmap[i][j].flags & DISCOVERED)) {
                if ((pmap[i][j].flags & MAGIC_MAPPED)
                    && (tileCatalog[pmap[i][j].layers[DUNGEON]].flags | tileCatalog[pmap[i][j].layers[LIQUID]].flags) & T_PATHING_BLOCKER) {
                    // Magic-mapped cells revealed as obstructions should be treated as such even though they're not discovered.
                    costMap[i][j] = cellHasTerrainFlag(i, j, T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
                } else {
                    costMap[i][j] = 1;
                    map[i][j] = exploreGoalValue(i, j);
                }
            } else if (theItem
                       && !monsterAvoids(&player, i, j)) {
                if (theItem->flags & ITEM_PLAYER_AVOIDS) {
                    costMap[i][j] = 20;
                } else {
                    costMap[i][j] = 1;
                    map[i][j] = exploreGoalValue(i, j) - 10;
                }
            }
        }
    }

    costMap[rogue.downLoc[0]][rogue.downLoc[1]] = 100;
    costMap[rogue.upLoc[0]][rogue.upLoc[1]]     = 100;

    if (headingToStairs) {
        map[rogue.downLoc[0]][rogue.downLoc[1]] = 0; // head to the stairs
    }

    dijkstraScan(map, costMap, true);

    //displayGrid(costMap);
    freeGrid(costMap);
}

boolean explore(short frameDelay) {
    short **distanceMap;
    short path[1000][2], steps;
    boolean madeProgress, headingToStairs;
    enum directions dir;
    creature *monst;

    // Explore commands should never be written to a recording.
    // Instead, the elemental movement commands that compose it
    // should be written individually.
    brogueAssert(!rogue.playbackMode);

    clearCursorPath();

    madeProgress    = false;
    headingToStairs = false;

    if (player.status[STATUS_CONFUSED]) {
        message("Not while you're confused.", false);
        return false;
    }
    if (cellHasTerrainFlag(player.xLoc, player.yLoc, T_OBSTRUCTS_PASSABILITY)) {
        message("Not while you're trapped.", false);
        return false;
    }

    for (monst = monsters->nextCreature; monst != NULL; monst = monst->nextCreature) {
        if (canSeeMonster(monst)) {
            monst->bookkeepingFlags |= MB_ALREADY_SEEN;
        } else {
            monst->bookkeepingFlags &= ~MB_ALREADY_SEEN;
        }
    }

    // fight any adjacent enemies
    dir = adjacentFightingDir();
    if (dir != NO_DIRECTION
        && startFighting(dir, (player.status[STATUS_HALLUCINATING] ? true : false))) {

        return true;
    }

    if (!rogue.autoPlayingLevel) {
        message(KEYBOARD_LABELS ? "Exploring... press any key to stop." : "Exploring... touch anywhere to stop.",
                false);
        // A little hack so the exploring message remains bright while exploring and then auto-dims when
        // another message is displayed:
        confirmMessages();
        printString(KEYBOARD_LABELS ? "Exploring... press any key to stop." : "Exploring... touch anywhere to stop.",
                    mapToWindowX(0), mapToWindowY(-1), &white, &black, NULL);
    }
    rogue.disturbed = false;
    rogue.automationActive = true;

    distanceMap = allocGrid();
    do {
        // fight any adjacent enemies
        dir = adjacentFightingDir();
        if (dir != NO_DIRECTION) {
            startFighting(dir, (player.status[STATUS_HALLUCINATING] ? true : false));
            if (rogue.disturbed) {
                madeProgress = true;
                continue;
            }
        }
        if (rogue.disturbed) {
            continue;
        }

        getExploreMap(distanceMap, headingToStairs);

        // hilite path
        steps = getPlayerPathOnMap(path, distanceMap, player.xLoc, player.yLoc);
        hilitePath(path, steps, false);

        // take a step
        dir = nextStep(distanceMap, player.xLoc, player.yLoc, NULL, false);

        if (!headingToStairs && rogue.autoPlayingLevel && dir == NO_DIRECTION) {
            headingToStairs = true;
            continue;
        }

        refreshSideBar(-1, -1, false);

        if (dir == NO_DIRECTION) {
            rogue.disturbed = true;
        } else if (!playerMoves(dir)) {
            rogue.disturbed = true;
        } else {
            madeProgress = true;
            if (pauseBrogue(frameDelay)) {
                rogue.disturbed = true;
                rogue.autoPlayingLevel = false;
            }
        }
        hilitePath(path, steps, true);
    } while (!rogue.disturbed);
    //clearCursorPath();
    rogue.automationActive = false;
    refreshSideBar(-1, -1, false);
    freeGrid(distanceMap);
    return madeProgress;
}

void autoPlayLevel(boolean fastForward) {
    boolean madeProgress;

    rogue.autoPlayingLevel = true;

    confirmMessages();
    message(KEYBOARD_LABELS ? "Playing... press any key to stop." : "Playing... touch anywhere to stop.", false);

    // explore until we are not making progress
    do {
        madeProgress = explore(fastForward ? 1 : 50);
        //refreshSideBar(-1, -1, false);

        if (!madeProgress && rogue.downLoc[0] == player.xLoc && rogue.downLoc[1] == player.yLoc) {
            useStairs(1);
            madeProgress = true;
        }
    } while (madeProgress && rogue.autoPlayingLevel);

    confirmMessages();

    rogue.autoPlayingLevel = false;
}

short directionOfKeypress(unsigned short ch) {
    switch (ch) {
        case LEFT_KEY:
        case LEFT_ARROW:
        case NUMPAD_4:
            return LEFT;
        case RIGHT_KEY:
        case RIGHT_ARROW:
        case NUMPAD_6:
            return RIGHT;
        case UP_KEY:
        case UP_ARROW:
        case NUMPAD_8:
            return UP;
        case DOWN_KEY:
        case DOWN_ARROW:
        case NUMPAD_2:
            return DOWN;
        case UPLEFT_KEY:
        case NUMPAD_7:
            return UPLEFT;
        case UPRIGHT_KEY:
        case NUMPAD_9:
            return UPRIGHT;
        case DOWNLEFT_KEY:
        case NUMPAD_1:
            return DOWNLEFT;
        case DOWNRIGHT_KEY:
        case NUMPAD_3:
            return DOWNRIGHT;
        default:
            return -1;
    }
}

boolean startFighting(enum directions dir, boolean tillDeath) {
    short x, y, expectedDamage;
    creature *monst;

    x = player.xLoc + nbDirs[dir][0];
    y = player.yLoc + nbDirs[dir][1];
    monst = monsterAtLoc(x, y);
    if (monst->info.flags & (MONST_IMMUNE_TO_WEAPONS | MONST_INVULNERABLE)) {
        return false;
    }
    expectedDamage = monst->info.damage.upperBound * monsterDamageAdjustmentAmount(monst) / FP_FACTOR;
    if (rogue.easyMode) {
        expectedDamage /= 5;
    }
    rogue.blockCombatText = true;
    rogue.disturbed = false;
    do {
        if (!playerMoves(dir)) {
            break;
        }
        if (pauseBrogue(1)) {
            break;
        }
    } while (!rogue.disturbed && !rogue.gameHasEnded && (tillDeath || player.currentHP > expectedDamage)
             && (pmap[x][y].flags & HAS_MONSTER) && monsterAtLoc(x, y) == monst);

    rogue.blockCombatText = false;
    return rogue.disturbed;
}

boolean isDisturbed(short x, short y) {
    short i;
    creature *monst;
    for (i=0; i< DIRECTION_COUNT; i++) {
        monst = monsterAtLoc(x + nbDirs[i][0], y + nbDirs[i][1]);
        if (pmap[x + nbDirs[i][0]][y + nbDirs[i][1]].flags & (HAS_ITEM)) {
            // Do not trigger for submerged or invisible or unseen monsters.
            return true;
        }
        if (monst
            && !(monst->creatureState == MONSTER_ALLY)
            && (canSeeMonster(monst) || monsterRevealed(monst))) {
            // Do not trigger for submerged or invisible or unseen monsters.
            return true;
        }
    }
    return false;
}

void discover(short x, short y) {
    enum dungeonLayers layer;
    dungeonFeature *feat;
    if (cellHasTMFlag(x, y, TM_IS_SECRET)) {

        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_IS_SECRET) {
                feat = &dungeonFeatureCatalog[tileCatalog[pmap[x][y].layers[layer]].discoverType];
                pmap[x][y].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                spawnDungeonFeature(x, y, feat, true, false);
            }
        }
        refreshDungeonCell(x, y);

        if (playerCanSee(x, y)) {
            rogue.disturbed = true;
        }
    }
}

// returns true if found anything
boolean search(short searchStrength) {
    short i, j, radius, x, y, percent;
    boolean foundSomething = false;

    radius = searchStrength / 10;
    x = player.xLoc;
    y = player.yLoc;

    for (i = x - radius; i <= x + radius; i++) {
        for (j = y - radius; j <= y + radius; j++) {
            if (coordinatesAreInMap(i, j)
                && playerCanDirectlySee(i, j)) {

                percent = searchStrength - distanceBetween(x, y, i, j) * 10;
                if (cellHasTerrainFlag(i, j, T_OBSTRUCTS_PASSABILITY)) {
                    percent = percent * 2/3;
                }
                if (percent >= 100) {
                    pmap[i][j].flags |= KNOWN_TO_BE_TRAP_FREE;
                }
                percent = min(percent, 100);
                if (cellHasTMFlag(i, j, TM_IS_SECRET)) {
                    if (rand_percent(percent)) {
                        discover(i, j);
                        foundSomething = true;
                    }
                }
            }
        }
    }
    return foundSomething;
}

boolean proposeOrConfirmLocation(short x, short y, char *failureMessage) {
    boolean retval = false;
    if (player.xLoc == x && player.yLoc == y) {
        message("you are already there.", false);
    } else if (pmap[x][y].flags & (DISCOVERED | MAGIC_MAPPED)) {
        if (rogue.cursorLoc[0] == x && rogue.cursorLoc[1] == y) {
            retval = true;
        } else {
            rogue.cursorLoc[0] = x;
            rogue.cursorLoc[1] = y;
        }
    } else {
        message(failureMessage, false);
    }
    return retval;
}

boolean useStairs(short stairDirection) {
    boolean succeeded = false;
    //cellDisplayBuffer fromBuf[COLS][ROWS], toBuf[COLS][ROWS];

    if (stairDirection == 1) {
        if (rogue.depthLevel < DEEPEST_LEVEL) {
            //copyDisplayBuffer(fromBuf, displayBuffer);
            rogue.cursorLoc[0] = rogue.cursorLoc[1] = -1;
            rogue.depthLevel++;
            message("You descend.", false);
            startLevel(rogue.depthLevel - 1, stairDirection);
            if (rogue.depthLevel > rogue.deepestLevel) {
                rogue.deepestLevel = rogue.depthLevel;
            }
            //copyDisplayBuffer(toBuf, displayBuffer);
            //irisFadeBetweenBuffers(fromBuf, toBuf, mapToWindowX(player.xLoc), mapToWindowY(player.yLoc), 20, false);
        } else if (numberOfMatchingPackItems(AMULET, 0, 0, false)) {
            victory(true);
        } else {
            confirmMessages();
            messageWithColor("the crystal archway repels you with a mysterious force!", &lightBlue, false);
            messageWithColor("(Only the bearer of the Amulet of Yendor may pass.)", &backgroundMessageColor, false);
        }
        succeeded = true;
    } else {
        if (rogue.depthLevel > 1 || numberOfMatchingPackItems(AMULET, 0, 0, false)) {
            rogue.cursorLoc[0] = rogue.cursorLoc[1] = -1;
            rogue.depthLevel--;
            if (rogue.depthLevel == 0) {
                victory(false);
            } else {
                //copyDisplayBuffer(fromBuf, displayBuffer);
                message("You ascend.", false);
                startLevel(rogue.depthLevel + 1, stairDirection);
                //copyDisplayBuffer(toBuf, displayBuffer);
                //irisFadeBetweenBuffers(fromBuf, toBuf, mapToWindowX(player.xLoc), mapToWindowY(player.yLoc), 20, true);
            }
            succeeded = true;
        } else {
            confirmMessages();
            messageWithColor("The dungeon exit is magically sealed!", &lightBlue, false);
            messageWithColor("(Only the bearer of the Amulet of Yendor may pass.)", &backgroundMessageColor, false);
        }
    }

    if (succeeded) {
        updatePlayerUnderwaterness();
    }

    return succeeded;
}

void storeMemories(const short x, const short y) {
    pmap[x][y].rememberedTerrainFlags = terrainFlags(x, y);
    pmap[x][y].rememberedTMFlags = terrainMechFlags(x, y);
    pmap[x][y].rememberedCellFlags = pmap[x][y].flags;
    pmap[x][y].rememberedTerrain = pmap[x][y].layers[highestPriorityLayer(x, y, false)];
}

void updateFieldOfViewDisplay(boolean updateDancingTerrain, boolean refreshDisplay) {
    short i, j;
    item *theItem;
    char buf[COLS*3], name[COLS*3];

    assureCosmeticRNG;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (pmap[i][j].flags & IN_FIELD_OF_VIEW
                && (max(0, tmap[i][j].light[0])
                    + max(0, tmap[i][j].light[1])
                    + max(0, tmap[i][j].light[2]) > VISIBILITY_THRESHOLD)
                && !(pmap[i][j].flags & CLAIRVOYANT_DARKENED)) {

                pmap[i][j].flags |= VISIBLE;
            }

            if ((pmap[i][j].flags & VISIBLE) && !(pmap[i][j].flags & WAS_VISIBLE)) { // if the cell became visible this move
                if (!(pmap[i][j].flags & DISCOVERED) && rogue.automationActive) {
                    if (pmap[i][j].flags & HAS_ITEM) {
                        theItem = itemAtLoc(i, j);
                        if (theItem && (theItem->category & KEY)) {
                            itemName(theItem, name, false, true, NULL);
                            sprintf(buf, "you see %s.", name);
                            messageWithColor(buf, &itemMessageColor, false);
                        }
                    }
                    if (!(pmap[i][j].flags & MAGIC_MAPPED)
                        && cellHasTMFlag(i, j, TM_INTERRUPT_EXPLORATION_WHEN_SEEN)) {

                        strcpy(name, tileCatalog[pmap[i][j].layers[layerWithTMFlag(i, j, TM_INTERRUPT_EXPLORATION_WHEN_SEEN)]].description);
                        sprintf(buf, "you see %s.", name);
                        messageWithColor(buf, &backgroundMessageColor, false);
                    }
                }
                discoverCell(i, j);
                if (refreshDisplay) {
                    refreshDungeonCell(i, j);
                }
            } else if (!(pmap[i][j].flags & VISIBLE) && (pmap[i][j].flags & WAS_VISIBLE)) { // if the cell ceased being visible this move
                storeMemories(i, j);
                if (refreshDisplay) {
                    refreshDungeonCell(i, j);
                }
            } else if (!(pmap[i][j].flags & CLAIRVOYANT_VISIBLE) && (pmap[i][j].flags & WAS_CLAIRVOYANT_VISIBLE)) { // ceased being clairvoyantly visible
                storeMemories(i, j);
                if (refreshDisplay) {
                    refreshDungeonCell(i, j);
                }
            } else if (!(pmap[i][j].flags & WAS_CLAIRVOYANT_VISIBLE) && (pmap[i][j].flags & CLAIRVOYANT_VISIBLE)) { // became clairvoyantly visible
                pmap[i][j].flags &= ~STABLE_MEMORY;
                if (refreshDisplay) {
                    refreshDungeonCell(i, j);
                }
            } else if (!(pmap[i][j].flags & TELEPATHIC_VISIBLE) && (pmap[i][j].flags & WAS_TELEPATHIC_VISIBLE)) { // ceased being telepathically visible
                storeMemories(i, j);
                if (refreshDisplay) {
                    refreshDungeonCell(i, j);
                }
            } else if (!(pmap[i][j].flags & WAS_TELEPATHIC_VISIBLE) && (pmap[i][j].flags & TELEPATHIC_VISIBLE)) { // became telepathically visible
                if (!(pmap[i][j].flags & DISCOVERED)
                    && !cellHasTerrainFlag(i, j, T_PATHING_BLOCKER)) {
                    rogue.xpxpThisTurn++;
                }

                pmap[i][j].flags &= ~STABLE_MEMORY;
                if (refreshDisplay) {
                    refreshDungeonCell(i, j);
                }
            } else if (playerCanSeeOrSense(i, j)
                       && (tmap[i][j].light[0] != tmap[i][j].oldLight[0] ||
                           tmap[i][j].light[1] != tmap[i][j].oldLight[1] ||
                           tmap[i][j].light[2] != tmap[i][j].oldLight[2])) { // if the cell's light color changed this move

                           if (refreshDisplay) {
                               refreshDungeonCell(i, j);
                           }
                       } else if (updateDancingTerrain
                                  && playerCanSee(i, j)
                                  && (!rogue.automationActive || !(rogue.playerTurnNumber % 5))
                                  && ((tileCatalog[pmap[i][j].layers[DUNGEON]].backColor)       && tileCatalog[pmap[i][j].layers[DUNGEON]].backColor->colorDances
                                      || (tileCatalog[pmap[i][j].layers[DUNGEON]].foreColor)    && tileCatalog[pmap[i][j].layers[DUNGEON]].foreColor->colorDances
                                      || (tileCatalog[pmap[i][j].layers[LIQUID]].backColor)     && tileCatalog[pmap[i][j].layers[LIQUID]].backColor->colorDances
                                      || (tileCatalog[pmap[i][j].layers[LIQUID]].foreColor)     && tileCatalog[pmap[i][j].layers[LIQUID]].foreColor->colorDances
                                      || (tileCatalog[pmap[i][j].layers[SURFACE]].backColor)    && tileCatalog[pmap[i][j].layers[SURFACE]].backColor->colorDances
                                      || (tileCatalog[pmap[i][j].layers[SURFACE]].foreColor)    && tileCatalog[pmap[i][j].layers[SURFACE]].foreColor->colorDances
                                      || (tileCatalog[pmap[i][j].layers[GAS]].backColor)        && tileCatalog[pmap[i][j].layers[GAS]].backColor->colorDances
                                      || (tileCatalog[pmap[i][j].layers[GAS]].foreColor)        && tileCatalog[pmap[i][j].layers[GAS]].foreColor->colorDances
                                      || player.status[STATUS_HALLUCINATING])) {

                                      pmap[i][j].flags &= ~STABLE_MEMORY;
                                      if (refreshDisplay) {
                                          refreshDungeonCell(i, j);
                                      }
                                  }
        }
    }
    restoreRNG;
}

//         Octants:      //
//          \7|8/        //
//          6\|/1        //
//          --@--        //
//          5/|\2        //
//          /4|3\        //

void betweenOctant1andN(short *x, short *y, short x0, short y0, short n) {
    short x1 = *x, y1 = *y;
    short dx = x1 - x0, dy = y1 - y0;
    switch (n) {
        case 1:
            return;
        case 2:
            *y = y0 - dy;
            return;
        case 5:
            *x = x0 - dx;
            *y = y0 - dy;
            return;
        case 6:
            *x = x0 - dx;
            return;
        case 8:
            *x = x0 - dy;
            *y = y0 - dx;
            return;
        case 3:
            *x = x0 - dy;
            *y = y0 + dx;
            return;
        case 7:
            *x = x0 + dy;
            *y = y0 - dx;
            return;
        case 4:
            *x = x0 + dy;
            *y = y0 + dx;
            return;
    }
}

// Returns a boolean grid indicating whether each square is in the field of view of (xLoc, yLoc).
// forbiddenTerrain is the set of terrain flags that will block vision (but the blocking cell itself is
// illuminated); forbiddenFlags is the set of map flags that will block vision.
// If cautiousOnWalls is set, we will not illuminate blocking tiles unless the tile one space closer to the origin
// is visible to the player; this is to prevent lights from illuminating a wall when the player is on the other
// side of the wall.
void getFOVMask(char grid[DCOLS][DROWS], short xLoc, short yLoc, fixpt maxRadius,
                unsigned long forbiddenTerrain, unsigned long forbiddenFlags, boolean cautiousOnWalls) {
    short i;

    for (i=1; i<=8; i++) {
        scanOctantFOV(grid, xLoc, yLoc, i, maxRadius, 1, LOS_SLOPE_GRANULARITY * -1, 0,
                      forbiddenTerrain, forbiddenFlags, cautiousOnWalls);
    }
}

// This is a custom implementation of recursive shadowcasting.
void scanOctantFOV(char grid[DCOLS][DROWS], short xLoc, short yLoc, short octant, fixpt maxRadius,
                   short columnsRightFromOrigin, long startSlope, long endSlope, unsigned long forbiddenTerrain,
                   unsigned long forbiddenFlags, boolean cautiousOnWalls) {

    if (columnsRightFromOrigin * FP_FACTOR >= maxRadius) return;

    short i, a, b, iStart, iEnd, x, y, x2, y2; // x and y are temporary variables on which we do the octant transform
    long newStartSlope, newEndSlope;
    boolean cellObstructed;

    newStartSlope = startSlope;

    a = ((LOS_SLOPE_GRANULARITY / -2 + 1) + startSlope * columnsRightFromOrigin) / LOS_SLOPE_GRANULARITY;
    b = ((LOS_SLOPE_GRANULARITY / -2 + 1) + endSlope * columnsRightFromOrigin) / LOS_SLOPE_GRANULARITY;

    iStart = min(a, b);
    iEnd = max(a, b);

    // restrict vision to a circle of radius maxRadius
    if ((columnsRightFromOrigin*columnsRightFromOrigin + iEnd*iEnd) >= maxRadius*maxRadius / FP_FACTOR / FP_FACTOR) {
        return;
    }
    if ((columnsRightFromOrigin*columnsRightFromOrigin + iStart*iStart) >= maxRadius*maxRadius / FP_FACTOR / FP_FACTOR) {
        iStart = (int) (-1 * fp_sqrt((maxRadius*maxRadius / FP_FACTOR) - (columnsRightFromOrigin*columnsRightFromOrigin * FP_FACTOR)) / FP_FACTOR);
    }

    x = xLoc + columnsRightFromOrigin;
    y = yLoc + iStart;
    betweenOctant1andN(&x, &y, xLoc, yLoc, octant);
    boolean currentlyLit = coordinatesAreInMap(x, y) && !(cellHasTerrainFlag(x, y, forbiddenTerrain) ||
                                                          (pmap[x][y].flags & forbiddenFlags));
    for (i = iStart; i <= iEnd; i++) {
        x = xLoc + columnsRightFromOrigin;
        y = yLoc + i;
        betweenOctant1andN(&x, &y, xLoc, yLoc, octant);
        if (!coordinatesAreInMap(x, y)) {
            // We're off the map -- here there be memory corruption.
            continue;
        }
        cellObstructed = (cellHasTerrainFlag(x, y, forbiddenTerrain) || (pmap[x][y].flags & forbiddenFlags));
        // if we're cautious on walls and this is a wall:
        if (cautiousOnWalls && cellObstructed) {
            // (x2, y2) is the tile one space closer to the origin from the tile we're on:
            x2 = xLoc + columnsRightFromOrigin - 1;
            y2 = yLoc + i;
            if (i < 0) {
                y2++;
            } else if (i > 0) {
                y2--;
            }
            betweenOctant1andN(&x2, &y2, xLoc, yLoc, octant);

            if (pmap[x2][y2].flags & IN_FIELD_OF_VIEW) {
                // previous tile is visible, so illuminate
                grid[x][y] = 1;
            }
        } else {
            // illuminate
            grid[x][y] = 1;
        }
        if (!cellObstructed && !currentlyLit) { // next column slope starts here
            newStartSlope = (long int) ((LOS_SLOPE_GRANULARITY * (i) - LOS_SLOPE_GRANULARITY / 2) / (columnsRightFromOrigin * 2 + 1) * 2);
            currentlyLit = true;
        } else if (cellObstructed && currentlyLit) { // next column slope ends here
            newEndSlope = (long int) ((LOS_SLOPE_GRANULARITY * (i) - LOS_SLOPE_GRANULARITY / 2)
                            / (columnsRightFromOrigin * 2 - 1) * 2);
            if (newStartSlope <= newEndSlope) {
                // run next column
                scanOctantFOV(grid, xLoc, yLoc, octant, maxRadius, columnsRightFromOrigin + 1, newStartSlope, newEndSlope,
                              forbiddenTerrain, forbiddenFlags, cautiousOnWalls);
            }
            currentlyLit = false;
        }
    }
    if (currentlyLit) { // got to the bottom of the scan while lit
        newEndSlope = endSlope;
        if (newStartSlope <= newEndSlope) {
            // run next column
            scanOctantFOV(grid, xLoc, yLoc, octant, maxRadius, columnsRightFromOrigin + 1, newStartSlope, newEndSlope,
                          forbiddenTerrain, forbiddenFlags, cautiousOnWalls);
        }
    }
}

void addScentToCell(short x, short y, short distance) {
    unsigned short value;
    if (!cellHasTerrainFlag(x, y, T_OBSTRUCTS_SCENT) || !cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY)) {
        value = rogue.scentTurnNumber - distance;
        scentMap[x][y] = max(value, (unsigned short) scentMap[x][y]);
    }
}
