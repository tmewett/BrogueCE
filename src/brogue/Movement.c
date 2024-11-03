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
#include "GlobalsBase.h"
#include "Globals.h"

void playerRuns(short direction) {
    boolean cardinalPassability[4];

    rogue.disturbed = (player.status[STATUS_CONFUSED] ? true : false);

    for (int dir = 0; dir < 4; dir++) {
        cardinalPassability[dir] = monsterAvoids(&player, posNeighborInDirection(player.loc, dir));
    }

    while (!rogue.disturbed) {
        if (!playerMoves(direction)) {
            rogue.disturbed = true;
            break;
        }

        const pos newLoc = posNeighborInDirection(player.loc, direction);
        if (!isPosInMap(newLoc) || monsterAvoids(&player, newLoc)) {
            rogue.disturbed = true;
        }
        if (isDisturbed(player.loc.x, player.loc.y)) {
            rogue.disturbed = true;
        } else if (direction < 4) {
            for (int dir = 0; dir < 4; dir++) {
                const pos newLoc = posNeighborInDirection(player.loc, dir);
                if (cardinalPassability[dir] != monsterAvoids(&player, newLoc)
                    && !posEq(player.loc, posNeighborInDirection(newLoc, direction))) {
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
const char *tileFlavor(short x, short y) {
    return tileCatalog[pmap[x][y].layers[highestPriorityLayer(x, y, false)]].flavorText;
}

// Retrieves a pointer to the description text of the highest-priority terrain at the given location
const char *tileText(short x, short y) {
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

/// @brief Describes an item, primarily as a component of the location description that displays just above the
/// menu bar at the bottom of the screen. The complete location description needs to fit on a single line the width
/// of the dungeon, so an attempt is made to fit as much detail as possible within a loose length constraint. If
/// there is enough room, item details are included. If not, a terse description is used. Note that the final
/// description may exceed the max length.
/// @param theItem The item
/// @param buf The item description
/// @param maxLength The maximum length of the description
void describedItemName(const item *theItem, char *description, int maxLength) {
    if (rogue.playbackOmniscience || (!player.status[STATUS_HALLUCINATING])) {
        itemName(theItem, description, true, true, NULL);
        if (strlen(description) > maxLength) {
            itemName(theItem, description, false, true, NULL);
        }
    } else {
        describeHallucinatedItem(description);
    }
}

void describeLocation(char *buf, short x, short y) {
    creature *monst;
    item *theItem, *magicItem;
    boolean standsInTerrain;
    boolean subjectMoving;
    boolean prepositionLocked = false;
    boolean monsterDormant;
    boolean monsterIsPlayer = false;

    char subject[COLS * 3];
    char verb[COLS * 3];
    char preposition[COLS * 3];
    char object[COLS * 3];
    char itemLocation[COLS * 3] = "";
    char adjective[COLS * 3];

    assureCosmeticRNG;

    theItem = itemAtLoc((pos){ x, y });
    // describe the player's location when there is no item there
    if ((x == player.loc.x) && (y == player.loc.y) && !theItem) {
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
    monsterDormant = false;
    if (pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER)) {
        monst = monsterAtLoc((pos){ x, y });
        monsterIsPlayer = (monst == &player);

    } else if (pmap[x][y].flags & HAS_DORMANT_MONSTER) {
        monst = dormantMonsterAtLoc((pos){ x, y });
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
        && !monsterIsPlayer
        && !canSeeMonster(monst)
        && monsterRevealed(monst)) {

        strcpy(adjective, (((!player.status[STATUS_HALLUCINATING] || rogue.playbackOmniscience) && !monst->info.isLarge)
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
        strcpy(verb, monsterIsPlayer ? "are" : "is");

        if (pmap[x][y].layers[GAS] && monst->status[STATUS_INVISIBLE] && !monsterIsPlayer) { // phantoms in gas
            sprintf(buf, "you can perceive the faint outline of %s in %s.", subject, tileCatalog[pmap[x][y].layers[GAS]].description);
            restoreRNG;
            return;
        }

        subjectMoving = (monst->turnsSpentStationary == 0
                         && !(monst->info.flags & (MONST_GETS_TURN_ON_ACTIVATION | MONST_IMMOBILE))
                         && monst->creatureState != MONSTER_SLEEPING
                         && !(monst->bookkeepingFlags & (MB_SEIZED | MB_CAPTIVE)));
        if ((monst->info.flags & MONST_ATTACKABLE_THRU_WALLS)
            && cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)) {
            strcat(verb, " embedded");
        } else if (cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)) {
            strcat(verb, " trapped");
            subjectMoving = false;
        } else if (monst->bookkeepingFlags & MB_CAPTIVE) {
            strcat(verb, " shackled in place");
            subjectMoving = false;
        } else if (monst->status[STATUS_PARALYZED]) {
            strcat(verb, " frozen in place");
            subjectMoving = false;
        } else if (monst->status[STATUS_STUCK]) {
            strcat(verb, " entangled");
            subjectMoving = false;
        } else if (monst->status[STATUS_LEVITATING]) {
            strcat(verb, (monsterIsPlayer ? " hovering" : (subjectMoving ? " flying" : " hovering")));
            strcat(preposition, "over");
            prepositionLocked = true;
        } else if (monsterCanSubmergeNow(monst)) {
            strcat(verb, (subjectMoving ? " gliding" : " drifting"));
        } else if (cellHasTerrainFlag((pos){ x, y }, T_MOVES_ITEMS) && !(monst->info.flags & MONST_SUBMERGES)) {
            strcat(verb, (subjectMoving ? " swimming" : " struggling"));
        } else if (cellHasTerrainFlag((pos){ x, y }, T_AUTO_DESCENT)) {
            strcat(verb, " suspended in mid-air");
            strcat(preposition, "over");
            prepositionLocked = true;
            subjectMoving = false;
        } else if (monst->status[STATUS_CONFUSED]) {
            strcat(verb, " staggering");
        } else if ((monst->info.flags & MONST_RESTRICTED_TO_LIQUID)
                   && !cellHasTMFlag(monst->loc, TM_ALLOWS_SUBMERGING)) {
            strcat(verb, " lying");
            subjectMoving = false;
        } else if (monst->info.flags & MONST_IMMOBILE) {
            strcat(verb, " resting");
        } else {
            switch (monst->creatureState) {
                case MONSTER_SLEEPING:
                    strcat(verb, " sleeping");
                    subjectMoving = false;
                    break;
                case MONSTER_WANDERING:
                    strcat(verb, subjectMoving ? " wandering" : " standing");
                    break;
                case MONSTER_FLEEING:
                    strcat(verb, subjectMoving ? " fleeing" : " standing");
                    break;
                case MONSTER_TRACKING_SCENT:
                    strcat(verb, subjectMoving ? " charging" : " standing");
                    break;
                case MONSTER_ALLY:
                    strcat(verb, monsterIsPlayer ? " standing" : (subjectMoving ? " following you" : " standing"));
                    break;
                default:
                    strcat(verb, " standing");
                    break;
            }
        }
        if (monst->status[STATUS_BURNING] && !(monst->info.flags & MONST_FIERY)) {
            strcat(verb, ", burning,");
        }

        if (theItem) {
            if (verb[strlen(verb) - 1] != ' ') {
                strcat(verb, " ");
            }
            strcpy(preposition, "over");
            if (monsterIsPlayer) {
                strcat(itemLocation, standsInTerrain ? " in " : " on ");
                strcat(itemLocation, tileText(x, y));
            }
            sprintf(buf, "%s %s %s %s%s.", subject, verb, preposition, object, itemLocation);
            describedItemName(theItem, object, DCOLS - strlen(buf));

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
            subjectMoving = cellHasTerrainFlag((pos){ x, y }, T_MOVES_ITEMS);
            if (player.status[STATUS_HALLUCINATING] && !rogue.playbackOmniscience) {
                strcpy(verb, "is");
            } else {
                strcpy(verb, (theItem->quantity > 1 || (theItem->category & GOLD)) ? "are" : "is");
            }
            if (cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)) {
                strcat(verb, " enclosed");
            } else {
                strcat(verb, subjectMoving ? " drifting" : " lying");
            }
            strcpy(preposition, standsInTerrain ? (subjectMoving ? "through" : "in")
                   : (subjectMoving ? "across" : "on"));

            sprintf(buf, "%s %s %s %s%s.", subject, verb, preposition, object, itemLocation);
            describedItemName(theItem, subject, DCOLS - strlen(buf));

        } else { // no item
            sprintf(buf, "you %s %s.", (playerCanDirectlySee(x, y) ? "see" : "sense"), object);
            restoreRNG;
            return;
        }
    }

    sprintf(buf, "%s %s %s %s%s.", subject, verb, preposition, object, itemLocation);
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
    for (i=0; i < KEY_ID_MAXIMUM && (theItem->keyLoc[i].loc.x || theItem->keyLoc[i].machine); i++) {
        if (posEq(theItem->keyLoc[i].loc, (pos){ x, y }) && theItem->keyLoc[i].disposableHere) {
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
            messageWithColor(buf, &itemMessageColor, 0);
            deleteItem(theItem);
        } else if (removeItemFromChain(theItem, floorItems)) {
            deleteItem(theItem);
            pmap[x][y].flags &= ~HAS_ITEM;
        } else if (pmap[x][y].flags & HAS_MONSTER) {
            monst = monsterAtLoc((pos){ x, y });
            if (monst->carriedItem && monst->carriedItem == theItem) {
                monst->carriedItem = NULL;
                deleteItem(theItem);
            }
        }
    }
}

short randValidDirectionFrom(creature *monst, short x, short y, boolean respectAvoidancePreferences) {
    short i, newX, newY, validDirections[8], count = 0;

    brogueAssert(rogue.RNG == RNG_SUBSTANTIVE);
    for (i=0; i<8; i++) {
        newX = x + nbDirs[i][0];
        newY = y + nbDirs[i][1];
        if (coordinatesAreInMap(newX, newY)
            && !cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)
            && !diagonalBlocked(x, y, newX, newY, false)
            && (!respectAvoidancePreferences
                || (!monsterAvoids(monst, (pos){newX, newY}))
                || ((pmap[newX][newY].flags & HAS_PLAYER) && monst->creatureState != MONSTER_ALLY))) {
            validDirections[count++] = i;
        }
    }
    if (count == 0) {
        // Rare, and important in this case that the function returns BEFORE a random roll is made to avoid OOS.
        return NO_DIRECTION;
    }
    return validDirections[rand_range(0, count - 1)];
}

void vomit(creature *monst) {
    char buf[COLS], monstName[COLS];
    spawnDungeonFeature(monst->loc.x, monst->loc.y, &dungeonFeatureCatalog[DF_VOMIT], true, false);

    if (canDirectlySeeMonster(monst)
        && !rogue.automationActive) {

        monsterName(monstName, monst, true);
        sprintf(buf, "%s vomit%s profusely", monstName, (monst == &player ? "" : "s"));
        combatMessage(buf, NULL);
    }
}

static void moveEntrancedMonsters(enum directions dir) {
    dir = oppositeDirection(dir);

    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
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
    refreshDungeonCell(monst->loc);
}

void freeCaptive(creature *monst) {
    char buf[COLS * 3], monstName[COLS];

    becomeAllyWith(monst);
    monsterName(monstName, monst, false);
    sprintf(buf, "you free the grateful %s and gain a faithful ally.", monstName);
    message(buf, 0);
}

boolean freeCaptivesEmbeddedAt(short x, short y) {
    creature *monst;

    if (pmap[x][y].flags & HAS_MONSTER) {
        // Free any captives trapped in the tunnelized terrain.
        monst = monsterAtLoc((pos){ x, y });
        if ((monst->bookkeepingFlags & MB_CAPTIVE)
            && !(monst->info.flags & MONST_ATTACKABLE_THRU_WALLS)
            && (cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY))) {
            freeCaptive(monst);
            return true;
        }
    }
    return false;
}

/// @brief Ask the player for confirmation before attacking an acidic monster
/// @param hitList the creature(s) getting attacked
/// @return true to abort the attack
static boolean abortAttackAgainstAcidicTarget(const creature *hitList[8]) {
    short i;
    char monstName[COLS], weaponName[COLS];
    char buf[COLS*3];

    if (rogue.weapon && !(rogue.weapon->flags & ITEM_PROTECTED)) {

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

/// @brief Ask the player for confirmation before attacking a discordant ally
/// @param hitList the creature(s) getting attacked
/// @return true to abort the attack
static boolean abortAttackAgainstDiscordantAlly(const creature *hitList[8]) {

    for (int i=0; i<8; i++) {
        if (hitList[i]
            && hitList[i]->creatureState == MONSTER_ALLY
            && hitList[i]->status[STATUS_DISCORDANT]
            && canSeeMonster(hitList[i])) {

            char monstName[COLS], buf[COLS*3];
            monsterName(monstName, hitList[i], true);
            sprintf(buf, "Are you sure you want to attack %s?", monstName);
            if (confirm(buf, false)) {
                return false; // Don't abort. Attack the ally.
            } else {
                return true; // Abort!
            }
        }
    }
    return false; // the confirmation dialog was not shown
}

/// @brief Determines if a player attack against the given creature(s) should be aborted. A confirmation
/// dialog is shown when attempting to attack an acidic monster or discordant ally, unless confused or
/// hallucinating (but not telepathic).
/// @param hitList the creature(s) getting attacked
/// @return true to abort the attack
static boolean abortAttack(const creature *hitList[8]) {

    // too bad so sad if you're confused or hallucinating (but not telepathic)
    if (player.status[STATUS_CONFUSED]
        || (player.status[STATUS_HALLUCINATING] && !player.status[STATUS_TELEPATHIC])) {
        return false;
    }

    if (abortAttackAgainstAcidicTarget(hitList)
        || abortAttackAgainstDiscordantAlly(hitList)) {
        return true;
    }

    return false; // either the player confirmed the attack or the confirmation dialog was not shown
}

// Returns true if a whip attack was launched.
// If "aborted" pointer is provided, sets it to true if it was aborted because
// the player opted not to attack (in which case the whole turn
// should be aborted), as opposed to there being no valid whip attack available
// (in which case the player/monster should move instead).
boolean handleWhipAttacks(creature *attacker, enum directions dir, boolean *aborted) {
    bolt theBolt;
    creature *defender;
    const creature *hitList[8] = {0};

    const char boltChar[DIRECTION_COUNT] = "||~~\\//\\";

    brogueAssert(dir > NO_DIRECTION && dir < DIRECTION_COUNT);

    if (attacker == &player) {
        if (!rogue.weapon || !(rogue.weapon->flags & ITEM_ATTACKS_EXTEND)) {
            return false;
        }
    } else if (!(attacker->info.abilityFlags & MA_ATTACKS_EXTEND)) {
        return false;
    }
    pos originLoc = attacker->loc;
    pos targetLoc = posNeighborInDirection(attacker->loc, dir);

    // The neighboring position in the attack direction must not be diagonally blocked.
    // Generally speaking, the attacker must be able to move one tile in the attack direction.
    if (diagonalBlocked(originLoc.x, originLoc.y, targetLoc.x, targetLoc.y, attacker == &player)) {
        return false;
    }

    pos strikeLoc;
    getImpactLoc(&strikeLoc, originLoc, targetLoc, 5, false, &boltCatalog[BOLT_WHIP]);

    defender = monsterAtLoc(strikeLoc);
    if (defender
        && (attacker != &player || canSeeMonster(defender))
        && !monsterIsHidden(defender, attacker)
        && monsterWillAttackTarget(attacker, defender)) {

        if (attacker == &player) {
            hitList[0] = defender;
            if (abortAttack(hitList)) {
                if (aborted) {
                    *aborted = true;
                }
                return false;
            }
        }
        attacker->bookkeepingFlags &= ~MB_SUBMERGED;
        theBolt = boltCatalog[BOLT_WHIP];
        theBolt.theChar = boltChar[dir];
        zap(originLoc, targetLoc, &theBolt, false, false);
        return true;
    }
    return false;
}

// Returns true if a spear attack was launched.
// If "aborted" pointer is provided, sets it to true if it was aborted because
// the player opted not to attack (in which case the whole turn
// should be aborted), as opposed to there being no valid spear attack available
// (in which case the player/monster should move instead).
boolean handleSpearAttacks(creature *attacker, enum directions dir, boolean *aborted) {
    creature *defender;
    const creature *hitList[8] = {0};
    short range = 2, i = 0, h = 0;
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

    // The neighboring position in the attack direction must not be diagonally blocked
    // Generally speaking, the attacker must be able to move one tile in the attack direction.
    pos neighborLoc = posNeighborInDirection(attacker->loc, dir);
    if (diagonalBlocked(attacker->loc.x, attacker->loc.y, neighborLoc.x, neighborLoc.y, attacker == &player)) {
        return false;
    }

    for (i = 0; i < range; i++) {
        const pos targetLoc = (pos) {
            attacker->loc.x + (1 + i) * nbDirs[dir][0],
            attacker->loc.y + (1 + i) * nbDirs[dir][1]
        };
        if (!isPosInMap(targetLoc)) {
            break;
        }

        /* Add creatures that we are willing to attack to the potential
        hitlist. Any of those that are either right by us or visible will
        trigger the attack. */
        defender = monsterAtLoc(targetLoc);
        if (defender
            && (!cellHasTerrainFlag(targetLoc, T_OBSTRUCTS_PASSABILITY)
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

        if (cellHasTerrainFlag(targetLoc, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION))) {
            break;
        }
    }
    range = i;
    if (proceed) {
        if (attacker == &player) {
            if (abortAttack(hitList)) {
                if (aborted) {
                    *aborted = true;
                }
                return false;
            }
        }
        if (!rogue.playbackFastForward) {
            for (i = 0; i < range; i++) {
                const pos targetLoc = (pos) {
                    attacker->loc.x + (1 + i) * nbDirs[dir][0],
                    attacker->loc.y + (1 + i) * nbDirs[dir][1]
                };
                if (isPosInMap(targetLoc)
                    && playerCanSeeOrSense(targetLoc.x, targetLoc.y)) {

                    visualEffect = true;
                    plotForegroundChar(boltChar[dir], targetLoc.x, targetLoc.y, &lightBlue, true);
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
            pauseAnimation(16, PAUSE_BEHAVIOR_DEFAULT);
            for (i = 0; i < range; i++) {
                const pos targetLoc = (pos) {
                    attacker->loc.x + (1 + i) * nbDirs[dir][0],
                    attacker->loc.y + (1 + i) * nbDirs[dir][1]
                };
                if (isPosInMap(targetLoc)) {
                    refreshDungeonCell(targetLoc);
                }
            }
        }
        return true;
    }
    return false;
}

static void buildFlailHitList(const short x, const short y, const short newX, const short newY, const creature *hitList[16]) {
    short mx, my;
    short i = 0;

    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        mx = monst->loc.x;
        my = monst->loc.y;
        if (distanceBetween((pos){x, y}, (pos){mx, my}) == 1
            && distanceBetween((pos){newX, newY}, (pos){mx, my}) == 1
            && canSeeMonster(monst)
            && monstersAreEnemies(&player, monst)
            && monst->creatureState != MONSTER_ALLY
            && !(monst->bookkeepingFlags & MB_IS_DYING)
            && (!cellHasTerrainFlag(monst->loc, T_OBSTRUCTS_PASSABILITY) || (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS))) {

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
    short x = player.loc.x, y = player.loc.y;
    short newX, newY, newestX, newestY;
    boolean playerMoved = false, specialAttackAborted = false, anyAttackHit = false;
    creature *defender = NULL, *tempMonst = NULL;
    const creature *hitList[16] = {NULL};
    char monstName[COLS];
    char buf[COLS*3];
    const int directionKeys[8] = {UP_KEY, DOWN_KEY, LEFT_KEY, RIGHT_KEY, UPLEFT_KEY, DOWNLEFT_KEY, UPRIGHT_KEY, DOWNRIGHT_KEY};

    brogueAssert(direction >= 0 && direction < DIRECTION_COUNT);

    newX = x + nbDirs[direction][0];
    newY = y + nbDirs[direction][1];

    if (!coordinatesAreInMap(newX, newY)) {
        return false;
    }

    // Save thet keystroke up-front; we'll revert if the player cancels.
    recordKeystroke(directionKeys[initialDirection], false, false);
    boolean committed = false; // as long as this is false, the keystroke can be cancelled

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
                    && cellHasTerrainFlag((pos){ newestX, newestY }, T_LAVA_INSTA_DEATH)
                    && !cellHasTerrainFlag((pos){ newestX, newestY }, T_OBSTRUCTS_PASSABILITY | T_ENTANGLES)
                    && !((pmap[newestX][newestY].flags & HAS_MONSTER)
                         && canSeeMonster(monsterAtLoc((pos){ newestX, newestY }))
                         && monsterAtLoc((pos){ newestX, newestY })->creatureState != MONSTER_ALLY)) {

                    if (!confirm("Risk stumbling into lava?", false)) {
                        cancelKeystroke();
                        return false;
                    } else {
                        break;
                    }
                }
            }
        }

        direction = randValidDirectionFrom(&player, x, y, false);
        if (direction == -1) {
            cancelKeystroke();
            return false;
        } else {
            newX = x + nbDirs[direction][0];
            newY = y + nbDirs[direction][1];
            if (!coordinatesAreInMap(newX, newY)) {
                cancelKeystroke();
                return false;
            }
            committed = true;
        }
    }

    if (pmap[newX][newY].flags & HAS_MONSTER) {
        defender = monsterAtLoc((pos){ newX, newY });
    }

    // If there's no enemy at the movement location that the player is aware of, consider terrain promotions.
    if (!defender
        || (!canSeeMonster(defender) && !monsterRevealed(defender))
        || !monstersAreEnemies(&player, defender)) {

        if (cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY) && cellHasTMFlag((pos){ newX, newY }, TM_PROMOTES_ON_PLAYER_ENTRY)) {
            layer = layerWithTMFlag(newX, newY, TM_PROMOTES_ON_PLAYER_ENTRY);
            if (tileCatalog[pmap[newX][newY].layers[layer]].flags & T_OBSTRUCTS_PASSABILITY) {
                committed = true;
                message(tileCatalog[pmap[newX][newY].layers[layer]].flavorText, 0);
                promoteTile(newX, newY, layer, false);
                playerTurnEnded();
                return true;
            }
        }

    }

    if (((!cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY) || (cellHasTMFlag((pos){ newX, newY }, TM_PROMOTES_WITH_KEY) && keyInPackFor((pos){ newX, newY })))
         && !diagonalBlocked(x, y, newX, newY, false)
         && (!cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY) || (cellHasTMFlag((pos){ x, y }, TM_PROMOTES_WITH_KEY) && keyInPackFor((pos){ x, y }))))
        || (defender && defender->info.flags & MONST_ATTACKABLE_THRU_WALLS)) {
        // if the move is not blocked

        if (handleWhipAttacks(&player, direction, &specialAttackAborted)
            || handleSpearAttacks(&player, direction, &specialAttackAborted)) {

            committed = true;
            playerRecoversFromAttacking(true);
            moveEntrancedMonsters(direction);
            playerTurnEnded();
            return true;
        } else if (specialAttackAborted) { // Canceled an attack against an acidic monster or discordant ally
            brogueAssert(!committed);
            cancelKeystroke();
            rogue.disturbed = true;
            return false;
        }

        if (defender) {
            // if there is a monster there

            if (defender->bookkeepingFlags & MB_CAPTIVE) {
                monsterName(monstName, defender, false);
                sprintf(buf, "Free the captive %s?", monstName);
                if (committed || confirm(buf, false)) {
                    committed = true;
                    if (cellHasTMFlag((pos){ newX, newY }, TM_PROMOTES_WITH_KEY) && keyInPackFor((pos){ newX, newY })) {
                        useKeyAt(keyInPackFor((pos){ newX, newY }), newX, newY);
                    }
                    freeCaptive(defender);
                    player.ticksUntilTurn += player.attackSpeed;
                    playerTurnEnded();
                    return true;
                } else {
                    cancelKeystroke();
                    return false;
                }
            }

            if (defender->creatureState != MONSTER_ALLY || defender->status[STATUS_DISCORDANT]) {
                // Make a hit list of monsters the player is attacking this turn.
                // We separate this tallying phase from the actual attacking phase because sometimes the attacks themselves
                // create more monsters, and those shouldn't be attacked in the same turn.

                buildHitList(hitList, &player, defender,
                             rogue.weapon && (rogue.weapon->flags & ITEM_ATTACKS_ALL_ADJACENT));

                if (abortAttack(hitList)) {
                    brogueAssert(!committed);
                    cancelKeystroke();
                    rogue.disturbed = true;
                    return false;
                }

                if (player.status[STATUS_NAUSEOUS]) {
                    committed = true;
                    if (rand_percent(25)) {
                        vomit(&player);
                        playerTurnEnded();
                        return false;
                    }
                }

                // Proceeding with the attack.
                committed = true;

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
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *tempMonst = nextCreature(&it);
                if ((tempMonst->bookkeepingFlags & MB_SEIZING)
                    && monstersAreEnemies(&player, tempMonst)
                    && distanceBetween(player.loc, tempMonst->loc) == 1
                    && !diagonalBlocked(player.loc.x, player.loc.y, tempMonst->loc.x, tempMonst->loc.y, false)
                    && !tempMonst->status[STATUS_ENTRANCED]) {

                    monsterName(monstName, tempMonst, true);
                    if (committed || !canSeeMonster(tempMonst)) {
                        committed = true;
                        sprintf(buf, "you struggle but %s is holding your legs!", monstName);
                        moveEntrancedMonsters(direction);
                        message(buf, 0);
                        playerTurnEnded();
                        return true;
                    } else {
                        sprintf(buf, "you cannot move; %s is holding your legs!", monstName);
                        message(buf, 0);
                        cancelKeystroke();
                        return false;
                    }
                }
            }
            player.bookkeepingFlags &= ~MB_SEIZED; // failsafe
        }

        if (pmap[newX][newY].flags & (DISCOVERED | MAGIC_MAPPED)
            && player.status[STATUS_LEVITATING] <= 1
            && !player.status[STATUS_CONFUSED]
            && cellHasTerrainFlag((pos){ newX, newY }, T_LAVA_INSTA_DEATH)
            && player.status[STATUS_IMMUNE_TO_FIRE] <= 1
            && !cellHasTerrainFlag((pos){ newX, newY }, T_ENTANGLES)
            && !cellHasTMFlag((pos){ newX, newY }, TM_IS_SECRET)) {
            message("that would be certain death!", 0);
            brogueAssert(!committed);
            cancelKeystroke();
            return false; // player won't willingly step into lava

        } else if (pmap[newX][newY].flags & (DISCOVERED | MAGIC_MAPPED)
                   && player.status[STATUS_LEVITATING] <= 1
                   && !player.status[STATUS_CONFUSED]
                   && cellHasTerrainFlag((pos){ newX, newY }, T_AUTO_DESCENT)
                   && (!cellHasTerrainFlag((pos){ newX, newY }, T_ENTANGLES) || cellHasTMFlag((pos){ newX, newY }, TM_PROMOTES_ON_PLAYER_ENTRY))
                   && !cellHasTMFlag((pos){ newX, newY }, TM_IS_SECRET)
                   && !confirm("Dive into the depths?", false)) {

            brogueAssert(!committed);
            cancelKeystroke();
            return false;

        } else if (playerCanSee(newX, newY)
                   && !player.status[STATUS_CONFUSED]
                   && !player.status[STATUS_BURNING]
                   && player.status[STATUS_IMMUNE_TO_FIRE] <= 1
                   && cellHasTerrainFlag((pos){ newX, newY }, T_IS_FIRE)
                   && !cellHasTMFlag((pos){ newX, newY }, TM_EXTINGUISHES_FIRE)
                   && !confirm("Venture into flame?", false)) {

            brogueAssert(!committed);
            cancelKeystroke();
            return false;

        } else if (playerCanSee(newX, newY)
                   && !player.status[STATUS_CONFUSED]
                   && !player.status[STATUS_BURNING]
                   && cellHasTerrainFlag((pos){ newX, newY }, T_CAUSES_CONFUSION | T_CAUSES_PARALYSIS)
                   && (!rogue.armor || !(rogue.armor->flags & ITEM_RUNIC) || !(rogue.armor->flags & ITEM_RUNIC_IDENTIFIED) || rogue.armor->enchant2 != A_RESPIRATION)
                   && !confirm("Venture into dangerous gas?", false)) {

            brogueAssert(!committed);
            cancelKeystroke();
            return false;

        } else if (pmap[newX][newY].flags & (ANY_KIND_OF_VISIBLE | MAGIC_MAPPED)
                   && player.status[STATUS_LEVITATING] <= 1
                   && !player.status[STATUS_CONFUSED]
                   && cellHasTerrainFlag((pos){ newX, newY }, T_IS_DF_TRAP)
                   && !(pmap[newX][newY].flags & PRESSURE_PLATE_DEPRESSED)
                   && !cellHasTMFlag((pos){ newX, newY }, TM_IS_SECRET)
                   && (!rogue.armor || !(rogue.armor->flags & ITEM_RUNIC) || !(rogue.armor->flags & ITEM_RUNIC_IDENTIFIED) || rogue.armor->enchant2 != A_RESPIRATION ||
                        (!cellHasTerrainType((pos){ newX, newY }, GAS_TRAP_POISON)
                         && !cellHasTerrainType((pos){ newX, newY }, GAS_TRAP_PARALYSIS)
                         && !cellHasTerrainType((pos){ newX, newY }, GAS_TRAP_CONFUSION)))
                   && !confirm("Step onto the pressure plate?", false)) {

            brogueAssert(!committed);
            cancelKeystroke();
            return false;
        }

        if (rogue.weapon && (rogue.weapon->flags & ITEM_LUNGE_ATTACKS)) {
            newestX = player.loc.x + 2*nbDirs[direction][0];
            newestY = player.loc.y + 2*nbDirs[direction][1];
            if (coordinatesAreInMap(newestX, newestY) && (pmap[newestX][newestY].flags & HAS_MONSTER)) {
                tempMonst = monsterAtLoc((pos){ newestX, newestY });
                if (tempMonst
                    && (canSeeMonster(tempMonst) || monsterRevealed(tempMonst))
                    && monstersAreEnemies(&player, tempMonst)
                    && tempMonst->creatureState != MONSTER_ALLY
                    && !(tempMonst->bookkeepingFlags & MB_IS_DYING)
                    && (!cellHasTerrainFlag(tempMonst->loc, T_OBSTRUCTS_PASSABILITY) || (tempMonst->info.flags & MONST_ATTACKABLE_THRU_WALLS))) {

                    hitList[0] = tempMonst;
                    if (abortAttack(hitList)) {
                        brogueAssert(!committed);
                        cancelKeystroke();
                        rogue.disturbed = true;
                        return false;
                    }
                }
            }
        }
        if (rogue.weapon && (rogue.weapon->flags & ITEM_PASS_ATTACKS)) {
            buildFlailHitList(x, y, newX, newY, hitList);
            if (abortAttack(hitList)) {
                brogueAssert(!committed);
                cancelKeystroke();
                rogue.disturbed = true;
                return false;
            }
        }

        if (player.status[STATUS_STUCK] && cellHasTerrainFlag((pos){ x, y }, T_ENTANGLES)) {
                // Don't interrupt exploration with this message.
            if (--player.status[STATUS_STUCK]) {
                if (!rogue.automationActive) {
                    message("you struggle but cannot free yourself.", 0);
                }
                moveEntrancedMonsters(direction);
                committed = true;
                playerTurnEnded();
                return true;
            } else {
                if (!rogue.automationActive) {
                    message("you break free!", 0);
                }
                if (tileCatalog[pmap[x][y].layers[SURFACE]].flags & T_ENTANGLES) {
                    pmap[x][y].layers[SURFACE] = NOTHING;
                }
            }
        }

        if (player.status[STATUS_NAUSEOUS]) {
            committed = true;
            if (rand_percent(25)) {
                vomit(&player);
                playerTurnEnded();
                return true;
            }
        }

        // Are we taking the stairs?
        if (rogue.downLoc.x == newX && rogue.downLoc.y == newY) {
            committed = true;
            useStairs(1);
        } else if (rogue.upLoc.x == newX && rogue.upLoc.y == newY) {
            committed = true;
            useStairs(-1);
        } else {
            // Okay, we're finally moving!
            committed = true;

            player.loc.x += nbDirs[direction][0];
            player.loc.y += nbDirs[direction][1];
            pmap[x][y].flags &= ~HAS_PLAYER;
            pmapAt(player.loc)->flags |= HAS_PLAYER;
            pmapAt(player.loc)->flags &= ~IS_IN_PATH;
            if (defender && defender->creatureState == MONSTER_ALLY) { // Swap places with ally.
                pmapAt(defender->loc)->flags &= ~HAS_MONSTER;
                defender->loc.x = x;
                defender->loc.y = y;
                if (monsterAvoids(defender, (pos){x, y})) {
                    getQualifyingPathLocNear(&(defender->loc.x), &(defender->loc.y), player.loc.x, player.loc.y, true, forbiddenFlagsForMonster(&(defender->info)), 0, 0, (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS), false);
                }
                //getQualifyingLocNear(loc, player.loc.x, player.loc.y, true, NULL, forbiddenFlagsForMonster(&(defender->info)) & ~(T_IS_DF_TRAP | T_IS_DEEP_WATER | T_SPONTANEOUSLY_IGNITES), HAS_MONSTER, false, false);
                //defender->loc.x = loc[0];
                //defender->loc.y = loc[1];
                pmapAt(defender->loc)->flags |= HAS_MONSTER;
            }

            if (pmapAt(player.loc)->flags & HAS_ITEM) {
                pickUpItemAt(player.loc);
                rogue.disturbed = true;
            }
            refreshDungeonCell((pos){ x, y });
            refreshDungeonCell(player.loc);
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
    } else if (cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)) {
        i = pmap[newX][newY].layers[layerWithFlag(newX, newY, T_OBSTRUCTS_PASSABILITY)];
        if ((tileCatalog[i].flags & T_OBSTRUCTS_PASSABILITY)
            && (!diagonalBlocked(x, y, newX, newY, false) || !cellHasTMFlag((pos){ newX, newY }, TM_PROMOTES_WITH_KEY))) {

            if (!(pmap[newX][newY].flags & DISCOVERED)) {
                committed = true;
                discoverCell(newX, newY);
                refreshDungeonCell((pos){ newX, newY });
            }

            messageWithColor(tileCatalog[i].flavorText, &backgroundMessageColor, 0);
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
                                 || ((traveler && monsterAvoids(traveler, (pos){i, j}))
                                     || cellHasTerrainFlag((pos){ i, j }, blockingTerrainFlags))) ? -1 : 30000;
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
            blocker = monsterAtLoc((pos){ newX, newY });
            if (monst
                && monsterAvoids(monst, (pos){newX, newY})) {

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
                && knownToPlayerAsPassableOrSecretDoor((pos){ newX, newY })
                && !blocked) {

                bestDir = dir;
                bestScore = distanceMap[x][y] - distanceMap[newX][newY];
            }
        }
    }
    return bestDir;
}

static void displayRoute(short **distanceMap, boolean removeRoute) {
    short currentX = player.loc.x, currentY = player.loc.y, dir, newX, newY;
    boolean advanced;

    if (distanceMap[player.loc.x][player.loc.y] < 0 || distanceMap[player.loc.x][player.loc.y] == 30000) {
        return;
    }
    do {
        if (removeRoute) {
            refreshDungeonCell((pos){ currentX, currentY });
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

void travelRoute(pos path[1000], short steps) {
    short i, j;
    short dir;

    brogueAssert(!rogue.playbackMode);

    rogue.disturbed = false;
    rogue.automationActive = true;

    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (canSeeMonster(monst)) {
            monst->bookkeepingFlags |= MB_ALREADY_SEEN;
        } else {
            monst->bookkeepingFlags &= ~MB_ALREADY_SEEN;
        }
    }

    for (i=0; i < steps && !rogue.disturbed; i++) {
        for (j = i + 1; j < steps - 1; j++) {
            // Check to see if the path has become obstructed or avoided since the last time we saw it.
            if (diagonalBlocked(path[j-1].x, path[j-1].y, path[j].x, path[j].y, true)
                || monsterAvoids(&player, path[j])) {

                rogue.disturbed = true;
                break;
            }
        }
        for (dir = 0; dir < DIRECTION_COUNT && !rogue.disturbed; dir++) {
            if (posEq(posNeighborInDirection(player.loc, dir), path[i])) {
                if (!playerMoves(dir)) {
                    rogue.disturbed = true;
                }
                if (pauseAnimation(25, PAUSE_BEHAVIOR_DEFAULT)) {
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

static void travelMap(short **distanceMap) {
    short currentX = player.loc.x, currentY = player.loc.y, dir, newX, newY;
    boolean advanced;

    rogue.disturbed = false;
    rogue.automationActive = true;

    if (distanceMap[player.loc.x][player.loc.y] < 0 || distanceMap[player.loc.x][player.loc.y] == 30000) {
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
                if (pauseAnimation(500, PAUSE_BEHAVIOR_DEFAULT)) {
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
        pmapAt(player.loc)->flags &= ~HAS_PLAYER;
        refreshDungeonCell(player.loc);
        player.loc.x = x;
        player.loc.y = y;
        pmap[x][y].flags |= HAS_PLAYER;
        updatePlayerUnderwaterness();
        refreshDungeonCell((pos){ x, y });
        updateVision(true);
        return;
    }

    if (abs(player.loc.x - x) + abs(player.loc.y - y) == 1) {
        // targeting a cardinal neighbor
        for (i=0; i<4; i++) {
            if (nbDirs[i][0] == (x - player.loc.x) && nbDirs[i][1] == (y - player.loc.y)) {
                playerMoves(i);
                break;
            }
        }
        return;
    }

    if (!(pmap[x][y].flags & (DISCOVERED | MAGIC_MAPPED))) {
        message("You have not explored that location.", 0);
        return;
    }

    distanceMap = allocGrid();

    calculateDistances(distanceMap, x, y, 0, &player, false, false);
    if (distanceMap[player.loc.x][player.loc.y] < 30000) {
        if (autoConfirm) {
            travelMap(distanceMap);
            //refreshSideBar(-1, -1, false);
        } else {
            if (rogue.upLoc.x == x && rogue.upLoc.y == y) {
                staircaseConfirmKey = ASCEND_KEY;
            } else if (rogue.downLoc.x == x && rogue.downLoc.y == y) {
                staircaseConfirmKey = DESCEND_KEY;
            } else {
                staircaseConfirmKey = 0;
            }
            displayRoute(distanceMap, false);
            message("Travel this route? (y/n)", 0);

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
//      if (player.loc.x == x && player.loc.y == y) {
//          rogue.cursorLoc.x = rogue.cursorLoc.y = 0;
//      } else {
//          rogue.cursorLoc.x = x;
//          rogue.cursorLoc.y = y;
//      }
    } else {
        rogue.cursorLoc = INVALID_POS;
        message("No path is available.", 0);
    }
    freeGrid(distanceMap);
}

void populateGenericCostMap(short **costMap) {
    short i, j;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)
                && (!cellHasTMFlag((pos){ i, j }, TM_IS_SECRET) || (discoveredTerrainFlagsAtLoc((pos){ i, j }) & T_OBSTRUCTS_PASSABILITY))) {

                costMap[i][j] = cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
            } else if (cellHasTerrainFlag((pos){ i, j }, T_PATHING_BLOCKER & ~T_OBSTRUCTS_PASSABILITY)) {
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
            *tFlags = terrainFlags((pos){ x, y });
        }
        if (TMFlags) {
            *TMFlags = terrainMechFlags((pos){ x, y });
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
                 && (!cellHasTMFlag((pos){ i, j }, TM_IS_SECRET) || (discoveredTerrainFlagsAtLoc((pos){ i, j }) & T_OBSTRUCTS_PASSABILITY) || monst == &player)) {

                costMap[i][j] = (tFlags & T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
                continue;
            }

            if ((tFlags & T_LAVA_INSTA_DEATH)
                && !(monst->info.flags & (MONST_IMMUNE_TO_FIRE | MONST_FLIES | MONST_INVULNERABLE))
                && (monst->status[STATUS_LEVITATING] || monst->status[STATUS_IMMUNE_TO_FIRE])
                && max(monst->status[STATUS_LEVITATING], monst->status[STATUS_IMMUNE_TO_FIRE]) < (rogue.mapToShore[i][j] + distanceBetween((pos){i, j}, monst->loc) * monst->movementSpeed / 100)) {
                // Only a temporary effect will permit the monster to survive the lava, and the remaining duration either isn't
                // enough to get it to the spot, or it won't suffice to let it return to shore if it does get there.
                // Treat these locations as obstacles.
                costMap[i][j] = PDS_FORBIDDEN;
                continue;
            }

            if (((tFlags & T_AUTO_DESCENT) || (tFlags & T_IS_DEEP_WATER) && !(monst->info.flags & MONST_IMMUNE_TO_WATER))
                && !(monst->info.flags & MONST_FLIES)
                && (monst->status[STATUS_LEVITATING])
                && monst->status[STATUS_LEVITATING] < (rogue.mapToShore[i][j] + distanceBetween((pos){i, j}, monst->loc) * monst->movementSpeed / 100)) {
                // Only a temporary effect will permit the monster to levitate over the chasm/water, and the remaining duration either isn't
                // enough to get it to the spot, or it won't suffice to let it return to shore if it does get there.
                // Treat these locations as obstacles.
                costMap[i][j] = PDS_FORBIDDEN;
                continue;
            }

            if (monsterAvoids(monst, (pos){i, j})) {
                costMap[i][j] = PDS_FORBIDDEN;
                continue;
            }

            if (cFlags & HAS_MONSTER) {
                currentTenant = monsterAtLoc((pos){ i, j });
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
                    || cellHasTMFlag((pos){ i, j }, TM_PROMOTES_ON_ITEM_PICKUP)
                    || (tFlags & T_ENTANGLES) && !(monst->info.flags & MONST_IMMUNE_TO_WEBS)) {

                    costMap[i][j] += 20;
                }
            }

            if (monst == &player) {
                theItem = itemAtLoc((pos){ i, j });
                if (theItem && (theItem->flags & ITEM_PLAYER_AVOIDS)) {
                    costMap[i][j] += 10;
                }
            }
        }
    }
}

enum directions adjacentFightingDir() {
    if (cellHasTerrainFlag(player.loc, T_OBSTRUCTS_PASSABILITY)) {
        return NO_DIRECTION;
    }
    for (enum directions dir = 0; dir < DIRECTION_COUNT; dir++) {
        const pos newLoc = posNeighborInDirection(player.loc, dir);
        creature *const monst = monsterAtLoc(newLoc);
        if (monst
            && canSeeMonster(monst)
            && (!diagonalBlocked(player.loc.x, player.loc.y, newLoc.x, newLoc.y, false) || (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS))
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
            theItem = itemAtLoc((pos){ i, j });
            if (!(pmap[i][j].flags & DISCOVERED)) {
                if ((pmap[i][j].flags & MAGIC_MAPPED)
                    && (tileCatalog[pmap[i][j].layers[DUNGEON]].flags | tileCatalog[pmap[i][j].layers[LIQUID]].flags) & T_PATHING_BLOCKER) {
                    // Magic-mapped cells revealed as obstructions should be treated as such even though they're not discovered.
                    costMap[i][j] = cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
                } else {
                    costMap[i][j] = 1;
                    map[i][j] = exploreGoalValue(i, j);
                }
            } else if (theItem
                       && !monsterAvoids(&player, (pos){i, j})) {
                if (theItem->flags & ITEM_PLAYER_AVOIDS) {
                    costMap[i][j] = 20;
                } else {
                    costMap[i][j] = 1;
                    map[i][j] = exploreGoalValue(i, j) - 10;
                }
            }
        }
    }

    costMap[rogue.downLoc.x][rogue.downLoc.y] = 100;
    costMap[rogue.upLoc.x][rogue.upLoc.y]     = 100;

    if (headingToStairs) {
        map[rogue.downLoc.x][rogue.downLoc.y] = 0; // head to the stairs
    }

    dijkstraScan(map, costMap, true);

    //displayGrid(costMap);
    freeGrid(costMap);
}

boolean explore(short frameDelay) {
    // Explore commands should never be written to a recording.
    // Instead, the elemental movement commands that compose it
    // should be written individually.
    brogueAssert(!rogue.playbackMode);

    clearCursorPath();

    boolean madeProgress    = false;
    boolean headingToStairs = false;

    if (player.status[STATUS_CONFUSED]) {
        message("Not while you're confused.", 0);
        return false;
    }
    if (cellHasTerrainFlag(player.loc, T_OBSTRUCTS_PASSABILITY)) {
        message("Not while you're trapped.", 0);
        return false;
    }

    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (canSeeMonster(monst)) {
            monst->bookkeepingFlags |= MB_ALREADY_SEEN;
        } else {
            monst->bookkeepingFlags &= ~MB_ALREADY_SEEN;
        }
    }

    // fight any adjacent enemies
    enum directions dir = adjacentFightingDir();
    if (dir != NO_DIRECTION
        && startFighting(dir, (player.status[STATUS_HALLUCINATING] ? true : false))) {

        return true;
    }

    if (!rogue.autoPlayingLevel) {
        message(KEYBOARD_LABELS ? "Exploring... press any key to stop." : "Exploring... touch anywhere to stop.",
                0);
        // A little hack so the exploring message remains bright while exploring and then auto-dims when
        // another message is displayed:
        confirmMessages();
        printString(KEYBOARD_LABELS ? "Exploring... press any key to stop." : "Exploring... touch anywhere to stop.",
                    mapToWindowX(0), mapToWindowY(-1), &white, &black, NULL);
    }
    rogue.disturbed = false;
    rogue.automationActive = true;

    short** distanceMap = allocGrid();
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
        pos path[1000];
        const int steps = getPlayerPathOnMap(path, distanceMap, player.loc);
        hilitePath(path, steps, false);

        // take a step
        dir = nextStep(distanceMap, player.loc.x, player.loc.y, NULL, false);

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
            if (pauseAnimation(frameDelay, PAUSE_BEHAVIOR_DEFAULT)) {

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
    message(KEYBOARD_LABELS ? "Playing... press any key to stop." : "Playing... touch anywhere to stop.", 0);

    // explore until we are not making progress
    do {
        madeProgress = explore(fastForward ? 1 : 50);
        //refreshSideBar(-1, -1, false);

        if (!madeProgress && rogue.downLoc.x == player.loc.x && rogue.downLoc.y == player.loc.y) {
            useStairs(1);
            madeProgress = true;
        }
    } while (madeProgress && rogue.autoPlayingLevel);

    confirmMessages();

    rogue.autoPlayingLevel = false;
}

boolean startFighting(enum directions dir, boolean tillDeath) {
    const pos neighborLoc = posNeighborInDirection(player.loc, dir);
    creature * const monst = monsterAtLoc(neighborLoc);
    if (monst->info.flags & (MONST_IMMUNE_TO_WEAPONS | MONST_INVULNERABLE)) {
        return false;
    }
    int expectedDamage = monst->info.damage.upperBound * monsterDamageAdjustmentAmount(monst) / FP_FACTOR;
    if (rogue.easyMode) {
        expectedDamage /= 5;
    }
    rogue.blockCombatText = true;
    rogue.disturbed = false;
    do {
        if (!playerMoves(dir)) {
            break;
        }
        if (pauseAnimation(1, PAUSE_BEHAVIOR_DEFAULT)) {
            break;
        }
    } while (!rogue.disturbed && !rogue.gameHasEnded && (tillDeath || player.currentHP > expectedDamage)
             && (pmapAt(neighborLoc)->flags & HAS_MONSTER) && monsterAtLoc(neighborLoc) == monst);

    rogue.blockCombatText = false;
    return rogue.disturbed;
}

boolean isDisturbed(short x, short y) {
    pos p = (pos){ x, y };
    for (int i=0; i< DIRECTION_COUNT; i++) {
        const pos neighborPos = posNeighborInDirection(p, i);
        creature *const monst = monsterAtLoc(neighborPos);
        if (pmapAt(neighborPos)->flags & (HAS_ITEM)) {
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
    if (cellHasTMFlag((pos){ x, y }, TM_IS_SECRET)) {

        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_IS_SECRET) {
                feat = &dungeonFeatureCatalog[tileCatalog[pmap[x][y].layers[layer]].discoverType];
                pmap[x][y].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                spawnDungeonFeature(x, y, feat, true, false);
            }
        }
        refreshDungeonCell((pos){ x, y });

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
    x = player.loc.x;
    y = player.loc.y;

    for (i = x - radius; i <= x + radius; i++) {
        for (j = y - radius; j <= y + radius; j++) {
            if (coordinatesAreInMap(i, j)
                && playerCanDirectlySee(i, j)) {

                percent = searchStrength - distanceBetween((pos){x, y}, (pos){i, j}) * 10;
                if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)) {
                    percent = percent * 2/3;
                }
                if (percent >= 100) {
                    pmap[i][j].flags |= KNOWN_TO_BE_TRAP_FREE;
                }
                percent = min(percent, 100);
                if (cellHasTMFlag((pos){ i, j }, TM_IS_SECRET)) {
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
    if (player.loc.x == x && player.loc.y == y) {
        message("you are already there.", 0);
    } else if (pmap[x][y].flags & (DISCOVERED | MAGIC_MAPPED)) {
        if (rogue.cursorLoc.x == x && rogue.cursorLoc.y == y) {
            retval = true;
        } else {
            rogue.cursorLoc.x = x;
            rogue.cursorLoc.y = y;
        }
    } else {
        message(failureMessage, 0);
    }
    return retval;
}

boolean useStairs(short stairDirection) {
    boolean succeeded = false;
    //screenDisplayBuffer fromBuf, toBuf;

    if (stairDirection == 1) {
        if (rogue.depthLevel < gameConst->deepestLevel) {
            rogue.cursorLoc = INVALID_POS;
            rogue.depthLevel++;
            message("You descend.", 0);
            startLevel(rogue.depthLevel - 1, stairDirection);
            if (rogue.depthLevel > rogue.deepestLevel) {
                rogue.deepestLevel = rogue.depthLevel;
            }
            //overlayDisplayBuffer(NULL, toBuf);
            //irisFadeBetweenBuffers(fromBuf, toBuf, mapToWindowX(player.loc.x), mapToWindowY(player.loc.y), 20, false);
        } else if (numberOfMatchingPackItems(AMULET, 0, 0, false)) {
            victory(true);
        } else {
            confirmMessages();
            messageWithColor("the crystal archway repels you with a mysterious force!", &lightBlue, 0);
            messageWithColor("(Only the bearer of the Amulet of Yendor may pass.)", &backgroundMessageColor, 0);
        }
        succeeded = true;
    } else {
        if (rogue.depthLevel > 1 || numberOfMatchingPackItems(AMULET, 0, 0, false)) {
            rogue.cursorLoc = INVALID_POS;
            rogue.depthLevel--;
            if (rogue.depthLevel == 0) {
                victory(false);
            } else {
                message("You ascend.", 0);
                startLevel(rogue.depthLevel + 1, stairDirection);
                //irisFadeBetweenBuffers(fromBuf, toBuf, mapToWindowX(player.loc.x), mapToWindowY(player.loc.y), 20, true);
            }
            succeeded = true;
        } else {
            confirmMessages();
            messageWithColor("The dungeon exit is magically sealed!", &lightBlue, 0);
            messageWithColor("(Only the bearer of the Amulet of Yendor may pass.)", &backgroundMessageColor, 0);
        }
    }

    if (succeeded) {
        updatePlayerUnderwaterness();
    }

    return succeeded;
}

void storeMemories(const short x, const short y) {
    pmap[x][y].rememberedTerrainFlags = terrainFlags((pos){ x, y });
    pmap[x][y].rememberedTMFlags = terrainMechFlags((pos){ x, y });
    pmap[x][y].rememberedCellFlags = pmap[x][y].flags;
    pmap[x][y].rememberedTerrain = pmap[x][y].layers[highestPriorityLayer(x, y, false)];
}

void updateFieldOfViewDisplay(boolean updateDancingTerrain, boolean refreshDisplay) {
    short i, j;
    item *theItem;
    char buf[COLS*3], name[COLS*3];

    assureCosmeticRNG;

    for (i=0; i<DCOLS; i++) {
        for (j = DROWS-1; j >= 0; j--) {
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
                        theItem = itemAtLoc((pos){ i, j });
                        if (theItem && (theItem->category & KEY)) {
                            itemName(theItem, name, false, true, NULL);
                            sprintf(buf, "you see %s.", name);
                            messageWithColor(buf, &itemMessageColor, 0);
                        }
                    }
                    if (!(pmap[i][j].flags & MAGIC_MAPPED)
                        && cellHasTMFlag((pos){ i, j }, TM_INTERRUPT_EXPLORATION_WHEN_SEEN)) {

                        strcpy(name, tileCatalog[pmap[i][j].layers[layerWithTMFlag(i, j, TM_INTERRUPT_EXPLORATION_WHEN_SEEN)]].description);
                        sprintf(buf, "you see %s.", name);
                        messageWithColor(buf, &backgroundMessageColor, 0);
                    }
                }
                discoverCell(i, j);
                if (refreshDisplay) {
                    refreshDungeonCell((pos){ i, j });
                }
            } else if (!(pmap[i][j].flags & VISIBLE) && (pmap[i][j].flags & WAS_VISIBLE)) { // if the cell ceased being visible this move
                storeMemories(i, j);
                if (refreshDisplay) {
                    refreshDungeonCell((pos){ i, j });
                }
            } else if (!(pmap[i][j].flags & CLAIRVOYANT_VISIBLE) && (pmap[i][j].flags & WAS_CLAIRVOYANT_VISIBLE)) { // ceased being clairvoyantly visible
                storeMemories(i, j);
                if (refreshDisplay) {
                    refreshDungeonCell((pos){ i, j });
                }
            } else if (!(pmap[i][j].flags & WAS_CLAIRVOYANT_VISIBLE) && (pmap[i][j].flags & CLAIRVOYANT_VISIBLE)) { // became clairvoyantly visible
                pmap[i][j].flags &= ~STABLE_MEMORY;
                if (refreshDisplay) {
                    refreshDungeonCell((pos){ i, j });
                }
            } else if (!(pmap[i][j].flags & TELEPATHIC_VISIBLE) && (pmap[i][j].flags & WAS_TELEPATHIC_VISIBLE)) { // ceased being telepathically visible
                storeMemories(i, j);
                if (refreshDisplay) {
                    refreshDungeonCell((pos){ i, j });
                }
            } else if (!(pmap[i][j].flags & WAS_TELEPATHIC_VISIBLE) && (pmap[i][j].flags & TELEPATHIC_VISIBLE)) { // became telepathically visible
                if (!(pmap[i][j].flags & DISCOVERED)
                    && !cellHasTerrainFlag((pos){ i, j }, T_PATHING_BLOCKER)) {
                    rogue.xpxpThisTurn++;
                }

                pmap[i][j].flags &= ~STABLE_MEMORY;
                if (refreshDisplay) {
                    refreshDungeonCell((pos){ i, j });
                }
            } else if (playerCanSeeOrSense(i, j)
                       && (tmap[i][j].light[0] != tmap[i][j].oldLight[0] ||
                           tmap[i][j].light[1] != tmap[i][j].oldLight[1] ||
                           tmap[i][j].light[2] != tmap[i][j].oldLight[2])) { // if the cell's light color changed this move

                           if (refreshDisplay) {
                               refreshDungeonCell((pos){ i, j });
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
                                          refreshDungeonCell((pos){ i, j });
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

// Returns a boolean grid indicating whether each square is in the field of view of (loc.x, loc.y).
// forbiddenTerrain is the set of terrain flags that will block vision (but the blocking cell itself is
// illuminated); forbiddenFlags is the set of map flags that will block vision.
// If cautiousOnWalls is set, we will not illuminate blocking tiles unless the tile one space closer to the origin
// is visible to the player; this is to prevent lights from illuminating a wall when the player is on the other
// side of the wall.
void getFOVMask(char grid[DCOLS][DROWS], short xLoc, short yLoc, fixpt maxRadius,
                unsigned long forbiddenTerrain, unsigned long forbiddenFlags, boolean cautiousOnWalls) {
    pos loc = { xLoc, yLoc };

    for (int i=1; i<=8; i++) {
        scanOctantFOV(grid, loc.x, loc.y, i, maxRadius, 1, LOS_SLOPE_GRANULARITY * -1, 0,
                      forbiddenTerrain, forbiddenFlags, cautiousOnWalls);
    }
}

// This is a custom implementation of recursive shadowcasting.
void scanOctantFOV(char grid[DCOLS][DROWS], short xLoc, short yLoc, short octant, fixpt maxRadius,
                   short columnsRightFromOrigin, long startSlope, long endSlope, unsigned long forbiddenTerrain,
                   unsigned long forbiddenFlags, boolean cautiousOnWalls) {
    const pos loc = { xLoc, yLoc };

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

    x = loc.x + columnsRightFromOrigin;
    y = loc.y + iStart;
    betweenOctant1andN(&x, &y, loc.x, loc.y, octant);
    boolean currentlyLit = coordinatesAreInMap(x, y) && !(cellHasTerrainFlag((pos){ x, y }, forbiddenTerrain) ||
                                                          (pmap[x][y].flags & forbiddenFlags));
    for (i = iStart; i <= iEnd; i++) {
        x = loc.x + columnsRightFromOrigin;
        y = loc.y + i;
        betweenOctant1andN(&x, &y, loc.x, loc.y, octant);
        if (!coordinatesAreInMap(x, y)) {
            // We're off the map -- here there be memory corruption.
            continue;
        }
        cellObstructed = (cellHasTerrainFlag((pos){ x, y }, forbiddenTerrain) || (pmap[x][y].flags & forbiddenFlags));
        // if we're cautious on walls and this is a wall:
        if (cautiousOnWalls && cellObstructed) {
            // (x2, y2) is the tile one space closer to the origin from the tile we're on:
            x2 = loc.x + columnsRightFromOrigin - 1;
            y2 = loc.y + i;
            if (i < 0) {
                y2++;
            } else if (i > 0) {
                y2--;
            }
            betweenOctant1andN(&x2, &y2, loc.x, loc.y, octant);

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
                scanOctantFOV(grid, loc.x, loc.y, octant, maxRadius, columnsRightFromOrigin + 1, newStartSlope, newEndSlope,
                              forbiddenTerrain, forbiddenFlags, cautiousOnWalls);
            }
            currentlyLit = false;
        }
    }
    if (currentlyLit) { // got to the bottom of the scan while lit
        newEndSlope = endSlope;
        if (newStartSlope <= newEndSlope) {
            // run next column
            scanOctantFOV(grid, loc.x, loc.y, octant, maxRadius, columnsRightFromOrigin + 1, newStartSlope, newEndSlope,
                          forbiddenTerrain, forbiddenFlags, cautiousOnWalls);
        }
    }
}

void addScentToCell(short x, short y, short distance) {
    unsigned short value;
    if (!cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_SCENT) || !cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)) {
        value = rogue.scentTurnNumber - distance;
        scentMap[x][y] = max(value, (unsigned short) scentMap[x][y]);
    }
}
