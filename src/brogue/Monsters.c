/*
 *  Monsters.c
 *  Brogue
 *
 *  Created by Brian Walker on 1/13/09.
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

void mutateMonster(creature *monst, short mutationIndex) {
    monst->mutationIndex = mutationIndex;
    const mutation *theMut = &(mutationCatalog[mutationIndex]);
    monst->info.flags |= theMut->monsterFlags;
    monst->info.abilityFlags |= theMut->monsterAbilityFlags;
    monst->info.maxHP = monst->info.maxHP * theMut->healthFactor / 100;
    monst->info.movementSpeed = monst->info.movementSpeed * theMut->moveSpeedFactor / 100;
    monst->info.attackSpeed = monst->info.attackSpeed * theMut->attackSpeedFactor / 100;
    monst->info.defense = monst->info.defense * theMut->defenseFactor / 100;
    if (monst->info.damage.lowerBound > 0) {
        monst->info.damage.lowerBound = monst->info.damage.lowerBound * theMut->damageFactor / 100;
        monst->info.damage.lowerBound = max(monst->info.damage.lowerBound, 1);
    }
    if (monst->info.damage.upperBound > 0) {
        monst->info.damage.upperBound = monst->info.damage.upperBound * theMut->damageFactor / 100;
        monst->info.damage.upperBound = max(monst->info.damage.upperBound, (monst->info.abilityFlags & MA_POISONS) ? 2 : 1);
    }
    if (theMut->DFChance >= 0) {
        monst->info.DFChance = theMut->DFChance;
    }
    if (theMut->DFType > 0) {
        monst->info.DFType = theMut->DFType;
    }
}

// Allocates space, generates a creature of the given type,
// prepends it to the list of creatures, and returns a pointer to that creature. Note that the creature
// is not given a map location here!
// TODO: generateMonster is convenient, but probably it should not add the monster to
// any global lists. The caller can do this, to avoid needlessly moving them elsewhere.
creature *generateMonster(short monsterID, boolean itemPossible, boolean mutationPossible) {
    short mutationChance, mutationAttempt;

    // 1.17^x * 10, with x from 1 to 13:
    const int POW_DEEP_MUTATION[] = {11, 13, 16, 18, 21, 25, 30, 35, 41, 48, 56, 65, 76};

    creature *monst = calloc(1, sizeof(creature));
    monst->info = monsterCatalog[monsterID];
    initializeStatus(monst);

    monst->mutationIndex = -1;
    if (mutationPossible
        && !(monst->info.flags & MONST_NEVER_MUTATED)
        && !(monst->info.abilityFlags & MA_NEVER_MUTATED)
        && rogue.depthLevel > gameConst->mutationsOccurAboveLevel) {

        if (rogue.depthLevel <= gameConst->amuletLevel) {
            mutationChance = clamp((rogue.depthLevel - gameConst->mutationsOccurAboveLevel) * gameConst->depthAccelerator, 1, 10);
        } else {
            mutationChance = POW_DEEP_MUTATION[min((rogue.depthLevel - gameConst->amuletLevel) * gameConst->depthAccelerator, 12)];
            mutationChance = min(mutationChance, 75);
        }

        if (rand_percent(mutationChance)) {
            mutationAttempt = rand_range(0, NUMBER_MUTATORS - 1);
            if (!(monst->info.flags & mutationCatalog[mutationAttempt].forbiddenFlags)
                && !(monst->info.abilityFlags & mutationCatalog[mutationAttempt].forbiddenAbilityFlags)) {

                mutateMonster(monst, mutationAttempt);
            }
        }
    }

    prependCreature(monsters, monst);
    initializeMonster(monst, itemPossible);

    return monst;
}

/// @brief Prepares a monster for placement on the current level but does not assign a location. Sets the initial
/// monster properties, many based on the creatureType (monst->info) from the monster catalog. Expects monst->info
/// to be populated and any mutation already applied. Optionally gives the monster an item.
/// @param monst The monster
/// @param itemPossible True if the monster can carry an item. May have no effect if the monster can never carry one.
void initializeMonster(creature *monst, boolean itemPossible) {

    monst->loc.x = monst->loc.y = 0;
    monst->depth = rogue.depthLevel;
    monst->bookkeepingFlags = 0;
    monst->mapToMe = NULL;
    monst->safetyMap = NULL;
    monst->leader = NULL;
    monst->carriedMonster = NULL;
    monst->creatureState = (((monst->info.flags & MONST_NEVER_SLEEPS) || rand_percent(25))
                            ? MONSTER_TRACKING_SCENT : MONSTER_SLEEPING);
    monst->creatureMode = MODE_NORMAL;
    monst->currentHP = monst->info.maxHP;
    monst->spawnDepth = rogue.depthLevel;
    monst->ticksUntilTurn = monst->info.movementSpeed;
    monst->info.turnsBetweenRegen *= 1000; // tracked as thousandths to prevent rounding errors
    monst->turnsUntilRegen = monst->info.turnsBetweenRegen;
    monst->regenPerTurn = 0;
    monst->movementSpeed = monst->info.movementSpeed;
    monst->attackSpeed = monst->info.attackSpeed;
    monst->turnsSpentStationary = 0;
    monst->xpxp = 0;
    monst->machineHome = 0;
    monst->newPowerCount = monst->totalPowerCount = 0;
    monst->targetCorpseLoc = INVALID_POS;
    monst->lastSeenPlayerAt = INVALID_POS;
    monst->targetWaypointIndex = -1;
    for (int i=0; i < MAX_WAYPOINT_COUNT; i++) {
        monst->waypointAlreadyVisited[i] = rand_range(0, 1);
    }

    int itemChance;
    if (monst->info.flags & MONST_CARRY_ITEM_100) {
        itemChance = 100;
    } else if (monst->info.flags & MONST_CARRY_ITEM_25) {
        itemChance = 25;
    } else {
        itemChance = 0;
    }

    if (ITEMS_ENABLED
        && itemPossible
        && (rogue.depthLevel <= gameConst->amuletLevel)
        && monsterItemsHopper->nextItem
        && rand_percent(itemChance)) {

        monst->carriedItem = monsterItemsHopper->nextItem;
        monsterItemsHopper->nextItem = monsterItemsHopper->nextItem->nextItem;
        monst->carriedItem->nextItem = NULL;
        monst->carriedItem->originDepth = rogue.depthLevel;
    } else {
        monst->carriedItem = NULL;
    }

    initializeGender(monst);

    if (!(monst->info.flags & MONST_INANIMATE) || (monst->info.abilityFlags & MA_ENTER_SUMMONS)) {
        monst->bookkeepingFlags |= MB_WEAPON_AUTO_ID;
    }

}

/// @brief Checks if the player knows a monster's location via telepathy or entrancement.
/// @param monst the monster
/// @return true if the monster is either entranced or revealed by telepathy
boolean monsterRevealed(creature *monst) {
    if (monst == &player) {
        return false;
    } else if (monst->bookkeepingFlags & MB_TELEPATHICALLY_REVEALED) {
        return true;
    } else if (monst->status[STATUS_ENTRANCED]) {
        return true;
    } else if (player.status[STATUS_TELEPATHIC] && !(monst->info.flags & MONST_INANIMATE)) {
        return true;
    }
    return false;
}

boolean monsterHiddenBySubmersion(const creature *monst, const creature *observer) {
    if (monst->bookkeepingFlags & MB_SUBMERGED) {
        if (observer
            && (terrainFlags(observer->loc) & T_IS_DEEP_WATER)
            && !observer->status[STATUS_LEVITATING]) {
            // observer is in deep water, so target is not hidden by water
            return false;
        } else {
            // submerged and the observer is not in deep water.
            return true;
        }
    }
    return false;
}

/// @brief Checks if a creature is in a state that hides it from an observer.
/// A creature is hidden if it's dormant, invisible (and not exposed by gas), or submerged (and the observer isn't).
/// However, leader/followers and player/allies are never hidden from each other.
/// Ignores line of sight, stealth, lighting, clairvoyance, telepathy, and terrain (except deep water).
/// Used for bolt targeting/paths (player & monsters), whip/spear attacks (player & monsters).
/// Called by canSeeMonster and canDirectlySeeMonster. A bit of a misnomer since monst can be the player.
/// @param monst the creature
/// @param observer the observer
/// @return true if the creature is hidden from the observer
boolean monsterIsHidden(const creature *monst, const creature *observer) {
    if (monst->bookkeepingFlags & MB_IS_DORMANT) {
        return true;
    }
    if (observer && monstersAreTeammates(monst, observer)) {
        // Teammates can always see each other.
        return false;
    }
    if ((monst->status[STATUS_INVISIBLE] && !pmapAt(monst->loc)->layers[GAS])) {
        // invisible and not in gas
        return true;
    }
    if (monsterHiddenBySubmersion(monst, observer)) {
        return true;
    }
    return false;
}

/// @brief Checks if the player has full knowledge about a creature,
/// i.e. they know where it is, and what kind it is. Ignores hallucination.
/// Equivalent to the monster being not hidden and either on a visible cell or revealed.
/// Some notable uses include: auto-targeting (staffs, wands, etc.), determining which monsters
/// display in the sidebar, auto-id of unidentified items (staffs, wands, runics, etc.), and the
/// verbiage used in combat/dungeon messages (or whether a message appears at all).
/// @param monst the monster
/// @return true if the monster is not hidden and the player knows its location
boolean canSeeMonster(creature *monst) {
    if (monst == &player) {
        return true;
    }
    if (!monsterIsHidden(monst, &player)
        && (playerCanSee(monst->loc.x, monst->loc.y) || monsterRevealed(monst))) {
        return true;
    }
    return false;
}

/// @brief Checks if the player can physically see a monster (i.e. line of sight and adequate lighting).
/// Ignores telepathy, but invisible allies are treated as visible. Clairvoyant lighting is ignored, but
/// darkening is a factor because it affects a cell's VISIBLE flag.
/// @param monst the monster
/// @return true if the player can physically see the monster
boolean canDirectlySeeMonster(creature *monst) {
    if (monst == &player) {
        return true;
    }
    if (playerCanDirectlySee(monst->loc.x, monst->loc.y) && !monsterIsHidden(monst, &player)) {
        return true;
    }
    return false;
}

void monsterName(char *buf, creature *monst, boolean includeArticle) {
    short oldRNG;

    if (monst == &player) {
        strcpy(buf, "you");
        return;
    }
    if (canSeeMonster(monst) || rogue.playbackOmniscience) {
        if (player.status[STATUS_HALLUCINATING] && !rogue.playbackOmniscience && !player.status[STATUS_TELEPATHIC]) {

            oldRNG = rogue.RNG;
            rogue.RNG = RNG_COSMETIC;
            //assureCosmeticRNG;
            sprintf(buf, "%s%s", (includeArticle ? "the " : ""),
                    monsterCatalog[rand_range(1, NUMBER_MONSTER_KINDS - 1)].monsterName);
            restoreRNG;

            return;
        }
        sprintf(buf, "%s%s", (includeArticle ? (monst->creatureState == MONSTER_ALLY ? "your " : "the ") : ""),
                monst->info.monsterName);
        //monsterText[monst->info.monsterID].name);
        return;
    } else {
        strcpy(buf, "something");
        return;
    }
}

boolean monsterIsInClass(const creature *monst, const short monsterClass) {
    short i;
    for (i = 0; monsterClassCatalog[monsterClass].memberList[i] != 0; i++) {
        if (monsterClassCatalog[monsterClass].memberList[i] == monst->info.monsterID) {
            return true;
        }
    }
    return false;
}

// Don't attack a revenant if you're not magical.
// Don't attack a monster embedded in obstruction crystal.
// Etc.
static boolean attackWouldBeFutile(const creature *attacker, const creature *defender) {
    if (cellHasTerrainFlag(defender->loc, T_OBSTRUCTS_PASSABILITY)
        && !(defender->info.flags & MONST_ATTACKABLE_THRU_WALLS)) {
        return true;
    }
    if (attacker == &player) {
        // Let the player do what she wants, if it's possible.
        return false;
    }
    if ((attacker->info.flags & MONST_RESTRICTED_TO_LIQUID)
        && !(attacker->status[STATUS_LEVITATING])
        && defender->status[STATUS_LEVITATING]) {
        return true;
    }
    if (defender->info.flags & MONST_INVULNERABLE) {
        return true;
    }
    if (defender->info.flags & MONST_IMMUNE_TO_WEAPONS
        && !(attacker->info.abilityFlags & MA_POISONS)) {
        return true;
    }
    return false;
}

/// @brief Determines if a creature is willing to attack another. Considers factors like discord,
/// entrancement, confusion, and whether they are enemies. Terrain and location are not considered,
/// except for krakens and eels that attack anything in deep water. Used for player and monster attacks.
/// @param attacker the attacking creature
/// @param defender the defending creature
/// @return true if the attacker is willing to attack the defender
boolean monsterWillAttackTarget(const creature *attacker, const creature *defender) {
    if (attacker == defender || (defender->bookkeepingFlags & MB_IS_DYING)) {
        return false;
    }
    if (attacker == &player
        && defender->creatureState == MONSTER_ALLY) {

        return defender->status[STATUS_DISCORDANT];
    }
    if (attacker->status[STATUS_ENTRANCED]
        && defender->creatureState != MONSTER_ALLY) {

        return true;
    }
    if (attacker->creatureState == MONSTER_ALLY
        && attacker != &player
        && defender->status[STATUS_ENTRANCED]) {

        return false;
    }
    if (defender->bookkeepingFlags & MB_CAPTIVE) {
        return false;
    }
    if (attacker->status[STATUS_DISCORDANT]
        || defender->status[STATUS_DISCORDANT]
        || attacker->status[STATUS_CONFUSED]) {

        return true;
    }
    if (monstersAreEnemies(attacker, defender)
        && !monstersAreTeammates(attacker, defender)) {
        return true;
    }
    return false;
}

boolean monstersAreTeammates(const creature *monst1, const creature *monst2) {
    // if one follows the other, or the other follows the one, or they both follow the same
    return ((((monst1->bookkeepingFlags & MB_FOLLOWER) && monst1->leader == monst2)
             || ((monst2->bookkeepingFlags & MB_FOLLOWER) && monst2->leader == monst1)
             || (monst1->creatureState == MONSTER_ALLY && monst2 == &player)
             || (monst1 == &player && monst2->creatureState == MONSTER_ALLY)
             || (monst1->creatureState == MONSTER_ALLY && monst2->creatureState == MONSTER_ALLY)
             || ((monst1->bookkeepingFlags & MB_FOLLOWER) && (monst2->bookkeepingFlags & MB_FOLLOWER)
                 && monst1->leader == monst2->leader)) ? true : false);
}

boolean monstersAreEnemies(const creature *monst1, const creature *monst2) {
    if ((monst1->bookkeepingFlags | monst2->bookkeepingFlags) & MB_CAPTIVE) {
        return false;
    }
    if (monst1 == monst2) {
        return false; // Can't be enemies with yourself, even if discordant.
    }
    if (monst1->status[STATUS_DISCORDANT] || monst2->status[STATUS_DISCORDANT]) {
        return true;
    }
    // eels and krakens attack anything in deep water
    if (((monst1->info.flags & MONST_RESTRICTED_TO_LIQUID)
         && !(monst2->info.flags & MONST_IMMUNE_TO_WATER)
         && !(monst2->status[STATUS_LEVITATING])
         && cellHasTerrainFlag(monst2->loc, T_IS_DEEP_WATER))

        || ((monst2->info.flags & MONST_RESTRICTED_TO_LIQUID)
            && !(monst1->info.flags & MONST_IMMUNE_TO_WATER)
            && !(monst1->status[STATUS_LEVITATING])
            && cellHasTerrainFlag(monst1->loc, T_IS_DEEP_WATER))) {

            return true;
        }
    return ((monst1->creatureState == MONSTER_ALLY || monst1 == &player)
            != (monst2->creatureState == MONSTER_ALLY || monst2 == &player));
}


void initializeGender(creature *monst) {
    if ((monst->info.flags & MONST_MALE) && (monst->info.flags & MONST_FEMALE)) {
        monst->info.flags &= ~(rand_percent(50) ? MONST_MALE : MONST_FEMALE);
    }
}

/// @brief Sets the character used to represent the player in the game, based on the game mode
void setPlayerDisplayChar() {
    if (rogue.easyMode) {
        player.info.displayChar = G_DEMON;
    } else {
        player.info.displayChar = G_PLAYER;
    }
}

// Returns true if either string has a null terminator before they otherwise disagree.
boolean stringsMatch(const char *str1, const char *str2) {
    short i;

    for (i=0; str1[i] && str2[i]; i++) {
        if (str1[i] != str2[i]) {
            return false;
        }
    }
    return true;
}

// Genders:
//  0 = [character escape sequence]
//  1 = you
//  2 = male
//  3 = female
//  4 = neuter
void resolvePronounEscapes(char *text, creature *monst) {
    short pronounType, gender, i;
    char *insert, *scan;
    boolean capitalize;
    // Note: Escape sequences MUST be longer than EACH of the possible replacements.
    // That way, the string only contracts, and we don't need a buffer.
    const char pronouns[4][5][20] = {
        {"$HESHE", "you", "he", "she", "it"},
        {"$HIMHER", "you", "him", "her", "it"},
        {"$HISHER", "your", "his", "her", "its"},
        {"$HIMSELFHERSELF", "yourself", "himself", "herself", "itself"}};

    if (monst == &player) {
        gender = 1;
    } else if (!canSeeMonster(monst) && !rogue.playbackOmniscience) {
        gender = 4;
    } else if (monst->info.flags & MONST_MALE) {
        gender = 2;
    } else if (monst->info.flags & MONST_FEMALE) {
        gender = 3;
    } else {
        gender = 4;
    }

    capitalize = false;

    for (insert = scan = text; *scan;) {
        if (scan[0] == '$') {
            for (pronounType=0; pronounType<4; pronounType++) {
                if (stringsMatch(pronouns[pronounType][0], scan)) {
                    strcpy(insert, pronouns[pronounType][gender]);
                    if (capitalize) {
                        upperCase(insert);
                        capitalize = false;
                    }
                    scan += strlen(pronouns[pronounType][0]);
                    insert += strlen(pronouns[pronounType][gender]);
                    break;
                }
            }
            if (pronounType == 4) {
                // Started with a '$' but didn't match an escape sequence; just copy the character and move on.
                *(insert++) = *(scan++);
            }
        } else if (scan[0] == COLOR_ESCAPE) {
            for (i=0; i<4; i++) {
                *(insert++) = *(scan++);
            }
        } else { // Didn't match any of the escape sequences; copy the character instead.
            if (*scan == '.') {
                capitalize = true;
            } else if (*scan != ' ') {
                capitalize = false;
            }

            *(insert++) = *(scan++);
        }
    }
    *insert = '\0';
}

/*
Returns a random horde, weighted by spawn frequency, which has all requiredFlags
and does not have any forbiddenFlags. If summonerType is 0, all hordes valid on
the given depth are considered. (Depth 0 means current depth.) Otherwise, all
hordes with summonerType as a leader are considered.
*/
short pickHordeType(short depth, enum monsterTypes summonerType, unsigned long forbiddenFlags, unsigned long requiredFlags) {
    short i, index, possCount = 0;

    if (depth <= 0) {
        depth = rogue.depthLevel;
    }

    for (i=0; i<gameConst->numberHordes; i++) {
        if (!(hordeCatalog[i].flags & forbiddenFlags)
            && !(~(hordeCatalog[i].flags) & requiredFlags)
            && ((!summonerType && hordeCatalog[i].minLevel <= depth && hordeCatalog[i].maxLevel >= depth)
                || (summonerType && (hordeCatalog[i].flags & HORDE_IS_SUMMONED) && hordeCatalog[i].leaderType == summonerType))) {
                possCount += hordeCatalog[i].frequency;
        }
    }

    if (possCount == 0) {
        return -1;
    }

    index = rand_range(1, possCount);

    for (i=0; i<gameConst->numberHordes; i++) {
        if (!(hordeCatalog[i].flags & forbiddenFlags)
            && !(~(hordeCatalog[i].flags) & requiredFlags)
            && ((!summonerType && hordeCatalog[i].minLevel <= depth && hordeCatalog[i].maxLevel >= depth)
                || (summonerType && (hordeCatalog[i].flags & HORDE_IS_SUMMONED) && hordeCatalog[i].leaderType == summonerType))) {
                if (index <= hordeCatalog[i].frequency) {
                    return i;
                }
                index -= hordeCatalog[i].frequency;
            }
    }
    return 0; // should never happen
}

void empowerMonster(creature *monst) {
    char theMonsterName[100], buf[200];
    monst->info.maxHP += 12;
    monst->info.defense += 10;
    monst->info.accuracy += 10;
    monst->info.damage.lowerBound += max(1, monst->info.damage.lowerBound / 10);
    monst->info.damage.upperBound += max(1, monst->info.damage.upperBound / 10);
    monst->newPowerCount++;
    monst->totalPowerCount++;
    heal(monst, 100, true);

    if (canSeeMonster(monst)) {
        monsterName(theMonsterName, monst, true);
        sprintf(buf, "%s looks stronger", theMonsterName);
        combatMessage(buf, &advancementMessageColor);
    }
}

// If placeClone is false, the clone won't get a location
// and won't set any HAS_MONSTER flags or cause any refreshes;
// it's just generated and inserted into the chains.
creature *cloneMonster(creature *monst, boolean announce, boolean placeClone) {
    char buf[DCOLS], monstName[DCOLS];
    short jellyCount;

    creature *newMonst = generateMonster(monst->info.monsterID, false, false);
    *newMonst = *monst; // boink!

    newMonst->carriedMonster = NULL; // Temporarily remove anything it's carrying.

    initializeGender(newMonst);
    newMonst->bookkeepingFlags &= ~(MB_LEADER | MB_CAPTIVE | MB_WEAPON_AUTO_ID);
    newMonst->bookkeepingFlags |= MB_FOLLOWER;
    newMonst->mapToMe = NULL;
    newMonst->safetyMap = NULL;
    newMonst->carriedItem = NULL;
    if (monst->carriedMonster) {
        creature *parentMonst = cloneMonster(monst->carriedMonster, false, false); // Also clone the carriedMonster
        removeCreature(monsters, parentMonst); // The cloned create will be added to the world, which we immediately undo.
        removeCreature(dormantMonsters, parentMonst); // in case it's added as a dormant creature? TODO: is this possible?
    }
    newMonst->ticksUntilTurn = 101;
    if (!(monst->creatureState == MONSTER_ALLY)) {
        newMonst->bookkeepingFlags &= ~MB_TELEPATHICALLY_REVEALED;
    }
    if (monst->leader) {
        newMonst->leader = monst->leader;
    } else {
        newMonst->leader = monst;
        monst->bookkeepingFlags |= MB_LEADER;
    }

    if (monst->bookkeepingFlags & MB_CAPTIVE) {
        // If you clone a captive, the clone will be your ally.
        becomeAllyWith(newMonst);
    }

    if (placeClone) {
//      getQualifyingLocNear(loc, monst->loc.x, monst->loc.y, true, 0, forbiddenFlagsForMonster(&(monst->info)), (HAS_PLAYER | HAS_MONSTER), false, false);
//      newMonst->loc.x = loc[0];
//      newMonst->loc.y = loc[1];
        getQualifyingPathLocNear(&(newMonst->loc.x), &(newMonst->loc.y), monst->loc.x, monst->loc.y, true,
                                 T_DIVIDES_LEVEL & avoidedFlagsForMonster(&(newMonst->info)), HAS_PLAYER,
                                 avoidedFlagsForMonster(&(newMonst->info)), (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS), false);
        pmapAt(newMonst->loc)->flags |= HAS_MONSTER;
        refreshDungeonCell(newMonst->loc);
        if (announce && canSeeMonster(newMonst)) {
            monsterName(monstName, newMonst, false);
            sprintf(buf, "another %s appears!", monstName);
            message(buf, 0);
        }
    }

    if (monst == &player) { // Player managed to clone himself.
        newMonst->info.foreColor = &gray;
        newMonst->info.damage.lowerBound = 1;
        newMonst->info.damage.upperBound = 2;
        newMonst->info.damage.clumpFactor = 1;
        newMonst->info.defense = 0;
        strcpy(newMonst->info.monsterName, "clone");
        newMonst->creatureState = MONSTER_ALLY;
    }

    if (monst->creatureState == MONSTER_ALLY
        && (monst->info.abilityFlags & MA_CLONE_SELF_ON_DEFEND)
        && !rogue.featRecord[FEAT_JELLYMANCER]) {

        jellyCount = 0;
        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *nextMonst = nextCreature(&it);
            if (nextMonst->creatureState == MONSTER_ALLY
                && (nextMonst->info.abilityFlags & MA_CLONE_SELF_ON_DEFEND)) {

                jellyCount++;
            }
        }
        if (jellyCount >= 90) {
            rogue.featRecord[FEAT_JELLYMANCER] = true;
        }
    }
    return newMonst;
}

unsigned long forbiddenFlagsForMonster(creatureType *monsterType) {
    unsigned long flags;

    flags = T_PATHING_BLOCKER;
    if (monsterType->flags & MONST_INVULNERABLE) {
        flags &= ~(T_LAVA_INSTA_DEATH | T_SPONTANEOUSLY_IGNITES | T_IS_FIRE);
    }
    if (monsterType->flags & (MONST_IMMUNE_TO_FIRE | MONST_FLIES)) {
        flags &= ~T_LAVA_INSTA_DEATH;
    }
    if (monsterType->flags & MONST_IMMUNE_TO_FIRE) {
        flags &= ~(T_SPONTANEOUSLY_IGNITES | T_IS_FIRE);
    }
    if (monsterType->flags & (MONST_IMMUNE_TO_WATER | MONST_FLIES)) {
        flags &= ~T_IS_DEEP_WATER;
    }
    if (monsterType->flags & (MONST_FLIES)) {
        flags &= ~(T_AUTO_DESCENT | T_IS_DF_TRAP);
    }
    return flags;
}

unsigned long avoidedFlagsForMonster(creatureType *monsterType) {
    unsigned long flags;

    flags = forbiddenFlagsForMonster(monsterType) | T_HARMFUL_TERRAIN | T_SACRED;

    if (monsterType->flags & MONST_INVULNERABLE) {
        flags &= ~(T_HARMFUL_TERRAIN | T_IS_DF_TRAP);
    }
    if (monsterType->flags & MONST_INANIMATE) {
        flags &= ~(T_CAUSES_POISON | T_CAUSES_DAMAGE | T_CAUSES_PARALYSIS | T_CAUSES_CONFUSION);
    }
    if (monsterType->flags & MONST_IMMUNE_TO_FIRE) {
        flags &= ~T_IS_FIRE;
    }
    if (monsterType->flags & MONST_FLIES) {
        flags &= ~T_CAUSES_POISON;
    }
    return flags;
}

boolean monsterCanSubmergeNow(creature *monst) {
    return ((monst->info.flags & MONST_SUBMERGES)
            && cellHasTMFlag(monst->loc, TM_ALLOWS_SUBMERGING)
            && !cellHasTerrainFlag(monst->loc, T_OBSTRUCTS_PASSABILITY)
            && !(monst->bookkeepingFlags & (MB_SEIZING | MB_SEIZED | MB_CAPTIVE))
            && ((monst->info.flags & (MONST_IMMUNE_TO_FIRE | MONST_INVULNERABLE))
                || monst->status[STATUS_IMMUNE_TO_FIRE]
                || !cellHasTerrainFlag(monst->loc, T_LAVA_INSTA_DEATH)));
}

// Returns true if at least one minion spawned.
static boolean spawnMinions(short hordeID, creature *leader, boolean summoned, boolean itemPossible) {
    short iSpecies, iMember, count;
    unsigned long forbiddenTerrainFlags;
    const hordeType *theHorde;
    creature *monst;
    short x, y;
    short failsafe;
    boolean atLeastOneMinion = false;

    x = leader->loc.x;
    y = leader->loc.y;

    theHorde = &hordeCatalog[hordeID];

    for (iSpecies = 0; iSpecies < theHorde->numberOfMemberTypes; iSpecies++) {
        count = randClump(theHorde->memberCount[iSpecies]);

        forbiddenTerrainFlags = forbiddenFlagsForMonster(&(monsterCatalog[theHorde->memberType[iSpecies]]));
        if (hordeCatalog[hordeID].spawnsIn) {
            forbiddenTerrainFlags &= ~(tileCatalog[hordeCatalog[hordeID].spawnsIn].flags);
        }

        for (iMember = 0; iMember < count; iMember++) {
            monst = generateMonster(theHorde->memberType[iSpecies], itemPossible, !summoned);
            failsafe = 0;
            do {
                getQualifyingPathLocNear(&(monst->loc.x), &(monst->loc.y), x, y, summoned,
                                         T_DIVIDES_LEVEL & forbiddenTerrainFlags, (HAS_PLAYER | HAS_STAIRS),
                                         forbiddenTerrainFlags, HAS_MONSTER, false);
            } while (theHorde->spawnsIn && !cellHasTerrainType(monst->loc, theHorde->spawnsIn) && failsafe++ < 20);
            if (failsafe >= 20) {
                // abort
                killCreature(monst, true);
                break;
            }
            if (monsterCanSubmergeNow(monst)) {
                monst->bookkeepingFlags |= MB_SUBMERGED;
            }
            brogueAssert(!(pmapAt(monst->loc)->flags & HAS_MONSTER));
            pmapAt(monst->loc)->flags |= HAS_MONSTER;
            monst->bookkeepingFlags |= (MB_FOLLOWER | MB_JUST_SUMMONED);
            monst->leader = leader;
            monst->creatureState = leader->creatureState;
            if (monst->creatureState == MONSTER_ALLY) {
                monst->bookkeepingFlags |= MB_DOES_NOT_RESURRECT;
            }
            monst->mapToMe = NULL;
            if (theHorde->flags & HORDE_DIES_ON_LEADER_DEATH) {
                monst->bookkeepingFlags |= MB_BOUND_TO_LEADER;
            }
            if (hordeCatalog[hordeID].flags & HORDE_ALLIED_WITH_PLAYER) {
                becomeAllyWith(monst);
            }
            atLeastOneMinion = true;
        }
    }

    if (atLeastOneMinion && !(theHorde->flags & HORDE_DIES_ON_LEADER_DEATH)) {
        leader->bookkeepingFlags |= MB_LEADER;
    }

    return atLeastOneMinion;
}

static boolean drawManacle(pos loc, enum directions dir) {
    enum tileType manacles[8] = {MANACLE_T, MANACLE_B, MANACLE_L, MANACLE_R, MANACLE_TL, MANACLE_BL, MANACLE_TR, MANACLE_BR};
    pos newLoc = posNeighborInDirection(loc, dir);
    if (isPosInMap(newLoc)
        && pmapAt(newLoc)->layers[DUNGEON] == FLOOR
        && pmapAt(newLoc)->layers[LIQUID] == NOTHING) {

        pmapAt(newLoc)->layers[SURFACE] = manacles[dir];
        return true;
    }
    return false;
}

static void drawManacles(pos loc) {
    enum directions fallback[4][3] = {{UPLEFT, UP, LEFT}, {DOWNLEFT, DOWN, LEFT}, {UPRIGHT, UP, RIGHT}, {DOWNRIGHT, DOWN, RIGHT}};
    short i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3 && !drawManacle(loc, fallback[i][j]); j++);
    }
}

// If hordeID is 0, it's randomly assigned based on the depth, with a 10% chance of an out-of-depth spawn from 1-5 levels deeper.
// If x is negative, location is random.
// Returns a pointer to the leader.
creature *spawnHorde(short hordeID, pos loc, unsigned long forbiddenFlags, unsigned long requiredFlags) {
    short i, failsafe, depth;
    const hordeType *theHorde;
    creature *leader, *preexistingMonst;
    boolean tryAgain;

    if (rogue.depthLevel > 1 && rand_percent(10)) {
        depth = rogue.depthLevel + rand_range(1, min(5, rogue.depthLevel / 2));
        if (depth > gameConst->amuletLevel) {
            depth = max(rogue.depthLevel, gameConst->amuletLevel);
        }
        forbiddenFlags |= HORDE_NEVER_OOD;
    } else {
        depth = rogue.depthLevel;
    }

    if (hordeID <= 0) {
        failsafe = 50;
        do {
            tryAgain = false;
            hordeID = pickHordeType(depth, 0, forbiddenFlags, requiredFlags);
            if (hordeID < 0) {
                return NULL;
            }
            if (isPosInMap(loc)) {
                if (cellHasTerrainFlag(loc, T_PATHING_BLOCKER)
                    && (!hordeCatalog[hordeID].spawnsIn || !cellHasTerrainType(loc, hordeCatalog[hordeID].spawnsIn))) {

                    // don't spawn a horde in special terrain unless it's meant to spawn there
                    tryAgain = true;
                }
                if (hordeCatalog[hordeID].spawnsIn && !cellHasTerrainType(loc, hordeCatalog[hordeID].spawnsIn)) {
                    // don't spawn a horde on normal terrain if it's meant for special terrain
                    tryAgain = true;
                }
            }
        } while (--failsafe && tryAgain);
    }

    failsafe = 50;

    if (!isPosInMap(loc)) {
        i = 0;
        do {
            while (!randomMatchingLocation(&loc, FLOOR, NOTHING, (hordeCatalog[hordeID].spawnsIn ? hordeCatalog[hordeID].spawnsIn : -1))
                   || passableArcCount(loc.x, loc.y) > 1) {
                if (!--failsafe) {
                    return NULL;
                }
                hordeID = pickHordeType(depth, 0, forbiddenFlags, 0);

                if (hordeID < 0) {
                    return NULL;
                }
            }
            i++;

            // This "while" condition should contain IN_FIELD_OF_VIEW, since that is specifically
            // calculated from the entry stairs when the level is generated, and will prevent monsters
            // from spawning within FOV of the entry stairs.
        } while (i < 25 && (pmapAt(loc)->flags & (ANY_KIND_OF_VISIBLE | IN_FIELD_OF_VIEW)));
    }

//  if (hordeCatalog[hordeID].spawnsIn == DEEP_WATER && pmap[x][y].layers[LIQUID] != DEEP_WATER) {
//      message("Waterborne monsters spawned on land!", REQUIRE_ACKNOWLEDGMENT);
//  }

    theHorde = &hordeCatalog[hordeID];

    if (theHorde->machine > 0) {
        // Build the accompanying machine (e.g. a goblin encampment)
        buildAMachine(theHorde->machine, loc.x, loc.y, 0, NULL, NULL, NULL);
    }

    leader = generateMonster(theHorde->leaderType, true, true);
    leader->loc = loc;

    if (hordeCatalog[hordeID].flags & HORDE_LEADER_CAPTIVE) {
        leader->bookkeepingFlags |= MB_CAPTIVE;
        leader->creatureState = MONSTER_WANDERING;
        if (leader->info.turnsBetweenRegen > 0) {
            leader->currentHP = leader->info.maxHP / 4 + 1;
        }

        // Draw the manacles unless the horde spawns in weird terrain (e.g. cages).
        if (!hordeCatalog[hordeID].spawnsIn) {
            drawManacles(loc);
        }
    } else if (hordeCatalog[hordeID].flags & HORDE_ALLIED_WITH_PLAYER) {
        becomeAllyWith(leader);
    }

    if (hordeCatalog[hordeID].flags & HORDE_SACRIFICE_TARGET) {
        leader->bookkeepingFlags |= MB_MARKED_FOR_SACRIFICE;
        leader->info.intrinsicLightType = SACRIFICE_MARK_LIGHT;
    }

    if ((theHorde->flags & HORDE_MACHINE_THIEF)) {
        leader->safetyMap = allocGrid(); // Keep thieves from fleeing before they see the player
        fillGrid(leader->safetyMap, 0);
    }

    preexistingMonst = monsterAtLoc(loc);
    if (preexistingMonst) {
        killCreature(preexistingMonst, true); // If there's already a monster here, quietly bury the body.
    }

    brogueAssert(!(pmapAt(loc)->flags & HAS_MONSTER));

    pmapAt(loc)->flags |= HAS_MONSTER;
    if (playerCanSeeOrSense(loc.x, loc.y)) {
        refreshDungeonCell(loc);
    }
    if (monsterCanSubmergeNow(leader)) {
        leader->bookkeepingFlags |= MB_SUBMERGED;
    }

    spawnMinions(hordeID, leader, false, true);

    return leader;
}

void fadeInMonster(creature *monst) {
    color fColor, bColor;
    enum displayGlyph displayChar;
    getCellAppearance(monst->loc, &displayChar, &fColor, &bColor);
    flashMonster(monst, &bColor, 100);
}

creatureList createCreatureList() {
    creatureList list;
    list.head = NULL;
    return list;
}
creatureIterator iterateCreatures(creatureList *list) {
    creatureIterator iter;
    iter.list = list;
    iter.next = list->head;
    // Skip monsters that have died.
    while (iter.next != NULL && iter.next->creature->bookkeepingFlags & MB_HAS_DIED) {
        iter.next = iter.next->nextCreature;
    }
    return iter;
}
boolean hasNextCreature(creatureIterator iter) {
    return iter.next != NULL;
}
creature *nextCreature(creatureIterator *iter) {
    if (iter->next == NULL) {
        return NULL;
    }
    creature *result = iter->next->creature;
    iter->next = iter->next->nextCreature;
    // Skip monsters that have died.
    while (iter->next != NULL && iter->next->creature->bookkeepingFlags & MB_HAS_DIED) {
        iter->next = iter->next->nextCreature;
    }
    return result;
}
void prependCreature(creatureList *list, creature *add) {
    creatureListNode *node = calloc(1, sizeof(creatureListNode));
    node->creature = add;
    node->nextCreature = list->head;
    list->head = node;
}
boolean removeCreature(creatureList *list, creature *remove) {
    creatureListNode **node = &list->head;
    while (*node != NULL) {
        if ((*node)->creature == remove) {
            creatureListNode *removeNode = *node;
            *node = removeNode->nextCreature;
            free(removeNode);
            return true;
        }
        node = &(*node)->nextCreature;
    }
    return false;
}
creature *firstCreature(creatureList *list) {
    if (list->head == NULL) {
        return NULL;
    }
    return list->head->creature;
}
void freeCreatureList(creatureList *list) {
    creatureListNode *nextMonst;
    for (creatureListNode *monstNode = list->head; monstNode != NULL; monstNode = nextMonst) {
        nextMonst = monstNode->nextCreature;
        freeCreature(monstNode->creature);
        free(monstNode);
    }
    list->head = NULL;
}

static boolean summonMinions(creature *summoner) {
    enum monsterTypes summonerType = summoner->info.monsterID;
    const short hordeID = pickHordeType(0, summonerType, 0, 0);
    short seenMinionCount = 0, x, y;
    boolean atLeastOneMinion = false;
    char buf[DCOLS];
    char monstName[DCOLS];
    short **grid;

    if (hordeID < 0) {
        return false;
    }

    if (summoner->info.abilityFlags & MA_ENTER_SUMMONS) {
        pmapAt(summoner->loc)->flags &= ~HAS_MONSTER;
        removeCreature(monsters, summoner);
    }

    atLeastOneMinion = spawnMinions(hordeID, summoner, true, false);

    if (hordeCatalog[hordeID].flags & HORDE_SUMMONED_AT_DISTANCE) {
        // Create a grid where "1" denotes a valid summoning location: within DCOLS/2 pathing distance,
        // not in harmful terrain, and outside of the player's field of view.
        grid = allocGrid();
        fillGrid(grid, 0);
        calculateDistances(grid, summoner->loc.x, summoner->loc.y, (T_PATHING_BLOCKER | T_SACRED), NULL, true, true);
        findReplaceGrid(grid, 1, DCOLS/2, 1);
        findReplaceGrid(grid, 2, 30000, 0);
        getTerrainGrid(grid, 0, (T_PATHING_BLOCKER | T_HARMFUL_TERRAIN), (IN_FIELD_OF_VIEW | CLAIRVOYANT_VISIBLE | HAS_PLAYER | HAS_MONSTER));
    } else {
        grid = NULL;
    }

    creature *host = NULL;
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (monst != summoner && monstersAreTeammates(monst, summoner)
            && (monst->bookkeepingFlags & MB_JUST_SUMMONED)) {

            if (hordeCatalog[hordeID].flags & HORDE_SUMMONED_AT_DISTANCE) {
                x = y = -1;
                randomLocationInGrid(grid, &x, &y, 1);
                teleport(monst, (pos){ x, y }, true);
                if (x != -1 && y != -1 && grid != NULL) {
                    grid[x][y] = 0;
                }
            }

            monst->bookkeepingFlags &= ~MB_JUST_SUMMONED;
            if (canSeeMonster(monst)) {
                seenMinionCount++;
                refreshDungeonCell(monst->loc);
            }
            monst->ticksUntilTurn = 101;
            monst->leader = summoner;

            fadeInMonster(monst);
            host = monst;
        }
    }

    if (canSeeMonster(summoner)) {
        monsterName(monstName, summoner, true);
        if (monsterText[summoner->info.monsterID].summonMessage[0]) {
            sprintf(buf, "%s %s", monstName, monsterText[summoner->info.monsterID].summonMessage);
        } else {
            sprintf(buf, "%s incants darkly!", monstName);
        }
        message(buf, 0);
    }

    if (summoner->info.abilityFlags & MA_ENTER_SUMMONS) {
        removeCreature(monsters, summoner);
        if (atLeastOneMinion && host) {
            host->carriedMonster = summoner;
            demoteMonsterFromLeadership(summoner);
            refreshDungeonCell(summoner->loc);
        } else {
            pmapAt(summoner->loc)->flags |= HAS_MONSTER;
            // TODO: why move to the beginning?
            prependCreature(monsters, summoner);
        }
    } else if (atLeastOneMinion) {
        summoner->bookkeepingFlags |= MB_LEADER;
    }
    createFlare(summoner->loc.x, summoner->loc.y, SUMMONING_FLASH_LIGHT);

    if (grid) {
        freeGrid(grid);
    }

    return atLeastOneMinion;
}

// Generates and places monsters for the level.
void populateMonsters() {
    if (!MONSTERS_ENABLED) {
        return;
    }

    short i, numberOfMonsters = min(20, 6 + 3 * max(0, rogue.depthLevel - gameConst->amuletLevel)); // almost always 6.

    while (rand_percent(60)) {
        numberOfMonsters++;
    }
    for (i=0; i<numberOfMonsters; i++) {
        spawnHorde(0, INVALID_POS, (HORDE_IS_SUMMONED | HORDE_MACHINE_ONLY), 0); // random horde type, random location
    }
}

boolean getRandomMonsterSpawnLocation(short *x, short *y) {
    short **grid;

    grid = allocGrid();
    fillGrid(grid, 0);
    calculateDistances(grid, player.loc.x, player.loc.y, T_DIVIDES_LEVEL, NULL, true, true);
    getTerrainGrid(grid, 0, (T_PATHING_BLOCKER | T_HARMFUL_TERRAIN), (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS | IN_FIELD_OF_VIEW));
    findReplaceGrid(grid, -30000, DCOLS/2-1, 0);
    findReplaceGrid(grid, 30000, 30000, 0);
    findReplaceGrid(grid, DCOLS/2, 30000-1, 1);
    randomLocationInGrid(grid, x, y, 1);
    if (*x < 0 || *y < 0) {
        fillGrid(grid, 1);
        getTerrainGrid(grid, 0, (T_PATHING_BLOCKER | T_HARMFUL_TERRAIN), (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS | IN_FIELD_OF_VIEW | IS_IN_MACHINE));
        randomLocationInGrid(grid, x, y, 1);
    }
    //    DEBUG {
    //        dumpLevelToScreen();
    //        hiliteGrid(grid, &orange, 50);
    //        plotCharWithColor('X', mapToWindow((pos){ x, y }), &black, &white);
    //        temporaryMessage("Horde spawn location possibilities:", REQUIRE_ACKNOWLEDGMENT);
    //    }
    freeGrid(grid);
    if (*x < 0 || *y < 0) {
        return false;
    }
    return true;
}

void spawnPeriodicHorde() {
    creature *monst;
    short x, y;

    if (!MONSTERS_ENABLED) {
        return;
    }

    if (getRandomMonsterSpawnLocation(&x, &y)) {
        monst = spawnHorde(0, (pos){ x, y }, (HORDE_IS_SUMMONED | HORDE_LEADER_CAPTIVE | HORDE_NO_PERIODIC_SPAWN | HORDE_MACHINE_ONLY), 0);
        if (monst) {
            monst->creatureState = MONSTER_WANDERING;
            for (creatureIterator it2 = iterateCreatures(monsters); hasNextCreature(it2);) {
                creature *monst2 = nextCreature(&it2);
                if (monst2->leader == monst) {
                    monst2->creatureState = MONSTER_WANDERING;
                }
            }
        }
    }
}

// Instantally disentangles the player/creature. Useful for magical displacement like teleport and blink.
void disentangle(creature *monst) {
    if (monst == &player && monst->status[STATUS_STUCK]) {
        message("you break free!", false);
    }
    monst->status[STATUS_STUCK] = 0;
}

// x and y are optional.
void teleport(creature *monst, pos destination, boolean respectTerrainAvoidancePreferences) {
    short **grid, i, j;
    char monstFOV[DCOLS][DROWS];

    if (!isPosInMap(destination)) {
        zeroOutGrid(monstFOV);
        getFOVMask(monstFOV, monst->loc.x, monst->loc.y, DCOLS * FP_FACTOR, T_OBSTRUCTS_VISION, 0, false);
        grid = allocGrid();
        fillGrid(grid, 0);
        calculateDistances(grid, monst->loc.x, monst->loc.y, forbiddenFlagsForMonster(&(monst->info)) & T_DIVIDES_LEVEL, NULL, true, false);
        findReplaceGrid(grid, -30000, DCOLS/2, 0);
        findReplaceGrid(grid, 2, 30000, 1);
        if (validLocationCount(grid, 1) < 1) {
            fillGrid(grid, 1);
        }
        if (respectTerrainAvoidancePreferences) {
            if (monst->info.flags & MONST_RESTRICTED_TO_LIQUID) {
                fillGrid(grid, 0);
                getTMGrid(grid, 1, TM_ALLOWS_SUBMERGING);
            }
            getTerrainGrid(grid, 0, avoidedFlagsForMonster(&(monst->info)), (IS_IN_MACHINE | HAS_PLAYER | HAS_MONSTER | HAS_STAIRS));
        } else {
            getTerrainGrid(grid, 0, forbiddenFlagsForMonster(&(monst->info)), (IS_IN_MACHINE | HAS_PLAYER | HAS_MONSTER | HAS_STAIRS));
        }
        for (i=0; i<DCOLS; i++) {
            for (j=0; j<DROWS; j++) {
                if (monstFOV[i][j]) {
                    grid[i][j] = 0;
                }
            }
        }
        randomLocationInGrid(grid, &destination.x, &destination.y, 1);
//        DEBUG {
//            dumpLevelToScreen();
//            hiliteGrid(grid, &orange, 50);
//            plotCharWithColor('X', mapToWindow((pos){ x, y }), &white, &red);
//            temporaryMessage("Teleport candidate locations:", REQUIRE_ACKNOWLEDGMENT);
//        }
        freeGrid(grid);
        if (!isPosInMap(destination)) {
            return; // Failure!
        }
    }
    // Always break free on teleport
    disentangle(monst);
    setMonsterLocation(monst, destination);
    if (monst != &player) {
        chooseNewWanderDestination(monst);
    }
}

static boolean isValidWanderDestination(creature *monst, short wpIndex) {
    return (wpIndex >= 0
            && wpIndex < rogue.wpCount
            && !monst->waypointAlreadyVisited[wpIndex]
            && rogue.wpDistance[wpIndex][monst->loc.x][monst->loc.y] >= 0
            && nextStep(rogue.wpDistance[wpIndex], monst->loc.x, monst->loc.y, monst, false) != NO_DIRECTION);
}

static short closestWaypointIndex(creature *monst) {
    short i, closestDistance, closestIndex;

    closestDistance = DCOLS/2;
    closestIndex = -1;
    for (i=0; i < rogue.wpCount; i++) {
        if (isValidWanderDestination(monst, i)
            && rogue.wpDistance[i][monst->loc.x][monst->loc.y] < closestDistance) {

            closestDistance = rogue.wpDistance[i][monst->loc.x][monst->loc.y];
            closestIndex = i;
        }
    }
    return closestIndex;
}

void chooseNewWanderDestination(creature *monst) {
    short i;

    brogueAssert(monst->targetWaypointIndex < MAX_WAYPOINT_COUNT);
    brogueAssert(rogue.wpCount > 0 && rogue.wpCount <= MAX_WAYPOINT_COUNT);

    // Set two checkpoints at random to false (which equilibrates to 50% of checkpoints being active).
    monst->waypointAlreadyVisited[rand_range(0, rogue.wpCount - 1)] = false;
    monst->waypointAlreadyVisited[rand_range(0, rogue.wpCount - 1)] = false;
    // Set the targeted checkpoint to true.
    if (monst->targetWaypointIndex >= 0) {
        monst->waypointAlreadyVisited[monst->targetWaypointIndex] = true;
    }

    monst->targetWaypointIndex = closestWaypointIndex(monst); // Will be -1 if no waypoints were available.
    if (monst->targetWaypointIndex == -1) {
        for (i=0; i < rogue.wpCount; i++) {
            monst->waypointAlreadyVisited[i] = 0;
        }
        monst->targetWaypointIndex = closestWaypointIndex(monst);
    }
}

enum subseqDFTypes {
    SUBSEQ_PROMOTE = 0,
    SUBSEQ_BURN,
    SUBSEQ_DISCOVER,
};

// Returns the terrain flags of this tile after it's promoted according to the event corresponding to subseqDFTypes.
static unsigned long successorTerrainFlags(enum tileType tile, enum subseqDFTypes promotionType) {
    enum dungeonFeatureTypes DF = 0;

    switch (promotionType) {
        case SUBSEQ_PROMOTE:
            DF = tileCatalog[tile].promoteType;
            break;
        case SUBSEQ_BURN:
            DF = tileCatalog[tile].fireType;
            break;
        case SUBSEQ_DISCOVER:
            DF = tileCatalog[tile].discoverType;
            break;
        default:
            break;
    }

    if (DF) {
        return tileCatalog[dungeonFeatureCatalog[DF].tile].flags;
    } else {
        return 0;
    }
}

unsigned long burnedTerrainFlagsAtLoc(pos loc) {
    short layer;
    unsigned long flags = 0;

    for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
        if (tileCatalog[pmapAt(loc)->layers[layer]].flags & T_IS_FLAMMABLE) {
            flags |= successorTerrainFlags(pmapAt(loc)->layers[layer], SUBSEQ_BURN);
            if (tileCatalog[pmapAt(loc)->layers[layer]].mechFlags & TM_EXPLOSIVE_PROMOTE) {
                flags |= successorTerrainFlags(pmapAt(loc)->layers[layer], SUBSEQ_PROMOTE);
            }
        }
    }

    return flags;
}

unsigned long discoveredTerrainFlagsAtLoc(pos loc) {
    short layer;
    unsigned long flags = 0;

    for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
        if (tileCatalog[pmapAt(loc)->layers[layer]].mechFlags & TM_IS_SECRET) {
            flags |= successorTerrainFlags(pmapAt(loc)->layers[layer], SUBSEQ_DISCOVER);
        }
    }

    return flags;
}

boolean monsterAvoids(creature *monst, pos p) {
    unsigned long terrainImmunities;
    creature *defender;
    unsigned long tFlags, cFlags;

    getLocationFlags(p.x, p.y, &tFlags, NULL, &cFlags, monst == &player);

    // everyone but the player avoids the stairs
    if ((p.x == rogue.downLoc.x && p.y == rogue.downLoc.y)
        || (p.x == rogue.upLoc.x && p.y == rogue.upLoc.y)) {

        return monst != &player;
    }

    // dry land
    if (monst->info.flags & MONST_RESTRICTED_TO_LIQUID
        && !cellHasTMFlag(p, TM_ALLOWS_SUBMERGING)) {
        return true;
    }

    // non-allied monsters can always attack the player
    if (player.loc.x == p.x && player.loc.y == p.y && monst != &player && monst->creatureState != MONSTER_ALLY) {
        return false;
    }

    // walls
    if (tFlags & T_OBSTRUCTS_PASSABILITY) {
        if (monst != &player
            && cellHasTMFlag(p, TM_IS_SECRET)
            && !(discoveredTerrainFlagsAtLoc(p) & avoidedFlagsForMonster(&(monst->info)))) {
            // This is so monsters can use secret doors but won't embed themselves in secret levers.
            return false;
        }
        if (distanceBetween(monst->loc, p) <= 1) {
            defender = monsterAtLoc(p);
            if (defender
                && (defender->info.flags & MONST_ATTACKABLE_THRU_WALLS)) {
                return false;
            }
        }
        return true;
    }

    // Monsters can always attack unfriendly neighboring monsters,
    // unless it is immune to us for whatever reason.
    if (distanceBetween(monst->loc, p) <= 1) {
        defender = monsterAtLoc(p);
        if (defender
            && !(defender->bookkeepingFlags & MB_IS_DYING)
            && monsterWillAttackTarget(monst, defender)) {

            if (attackWouldBeFutile(monst, defender)) {
                return true;
            } else {
                return false;
            }
        }
    }

    // Monsters always avoid enemy monsters that we can't damage.
    defender = monsterAtLoc(p);
    if (defender
        && !(defender->bookkeepingFlags & MB_IS_DYING)
        && monstersAreEnemies(monst, defender)
        && attackWouldBeFutile(monst, defender)) {

        return true;
    }

    // hidden terrain
    if (cellHasTMFlag(p, TM_IS_SECRET) && monst == &player) {
        return false; // player won't avoid what he doesn't know about
    }

    // Determine invulnerabilities based only on monster characteristics.
    terrainImmunities = 0;
    if (monst->status[STATUS_IMMUNE_TO_FIRE]) {
        terrainImmunities |= (T_IS_FIRE | T_SPONTANEOUSLY_IGNITES | T_LAVA_INSTA_DEATH);
    }
    if (monst->info.flags & MONST_INVULNERABLE) {
        terrainImmunities |= T_HARMFUL_TERRAIN | T_ENTANGLES | T_SPONTANEOUSLY_IGNITES | T_LAVA_INSTA_DEATH;
    }
    if (monst->info.flags & MONST_INANIMATE) {
        terrainImmunities |= (T_CAUSES_DAMAGE | T_CAUSES_PARALYSIS | T_CAUSES_CONFUSION | T_CAUSES_NAUSEA | T_CAUSES_POISON);
    }
    if (monst->status[STATUS_LEVITATING]) {
        terrainImmunities |= (T_AUTO_DESCENT | T_CAUSES_POISON | T_IS_DEEP_WATER | T_IS_DF_TRAP | T_LAVA_INSTA_DEATH);
    }
    if (monst->info.flags & MONST_IMMUNE_TO_WEBS) {
        terrainImmunities |= T_ENTANGLES;
    }
    if (monst->info.flags & MONST_IMMUNE_TO_WATER) {
        terrainImmunities |= T_IS_DEEP_WATER;
    }
    if (monst == &player) {
        terrainImmunities |= T_SACRED;
    }
    if (monst == &player
        && rogue.armor
        && (rogue.armor->flags & ITEM_RUNIC)
        && (rogue.armor->flags & ITEM_RUNIC_IDENTIFIED)
        && rogue.armor->enchant2 == A_RESPIRATION) {

        terrainImmunities |= T_RESPIRATION_IMMUNITIES;
    }

    // sacred ground
    if ((tFlags & T_SACRED & ~terrainImmunities)) {
        return true;
    }

    // brimstone
    if (!(monst->status[STATUS_IMMUNE_TO_FIRE])
        && !(monst->info.flags & MONST_INVULNERABLE)
        && (tFlags & T_SPONTANEOUSLY_IGNITES)
        && !(cFlags & (HAS_MONSTER | HAS_PLAYER))
        && !cellHasTerrainFlag(monst->loc, T_IS_FIRE | T_SPONTANEOUSLY_IGNITES)
        && (monst == &player || (monst->creatureState != MONSTER_TRACKING_SCENT && monst->creatureState != MONSTER_FLEEING))) {
        return true;
    }

    // burning wandering monsters avoid flammable terrain out of common courtesy
    if (monst != &player
        && monst->creatureState == MONSTER_WANDERING
        && (monst->info.flags & MONST_FIERY)
        && (tFlags & T_IS_FLAMMABLE)) {

        return true;
    }

    // burning monsters avoid explosive terrain and steam-emitting terrain
    if (monst != &player
        && monst->status[STATUS_BURNING]
        && (burnedTerrainFlagsAtLoc(p) & (T_CAUSES_EXPLOSIVE_DAMAGE | T_CAUSES_DAMAGE | T_AUTO_DESCENT) & ~terrainImmunities)) {

        return true;
    }

    // fire
    if ((tFlags & T_IS_FIRE & ~terrainImmunities)
        && !cellHasTerrainFlag(monst->loc, T_IS_FIRE)
        && !(cFlags & (HAS_MONSTER | HAS_PLAYER))
        && (monst != &player || rogue.mapToShore[p.x][p.y] >= player.status[STATUS_IMMUNE_TO_FIRE])) {
        return true;
    }

    // non-fire harmful terrain
    if ((tFlags & T_HARMFUL_TERRAIN & ~T_IS_FIRE & ~terrainImmunities)
        && !cellHasTerrainFlag(monst->loc, (T_HARMFUL_TERRAIN & ~T_IS_FIRE))) {
        return true;
    }

    // chasms or trap doors
    if ((tFlags & T_AUTO_DESCENT & ~terrainImmunities)
        && (!(tFlags & T_ENTANGLES) || !(monst->info.flags & MONST_IMMUNE_TO_WEBS))) {
        return true;
    }

    // gas or other environmental traps
    if ((tFlags & T_IS_DF_TRAP & ~terrainImmunities)
        && !(cFlags & PRESSURE_PLATE_DEPRESSED)
        && (monst == &player || monst->creatureState == MONSTER_WANDERING
            || (monst->creatureState == MONSTER_ALLY && !(cellHasTMFlag(p, TM_IS_SECRET))))
        && !(monst->status[STATUS_ENTRANCED])
        && (!(tFlags & T_ENTANGLES) || !(monst->info.flags & MONST_IMMUNE_TO_WEBS))) {
        return true;
    }

    // lava
    if ((tFlags & T_LAVA_INSTA_DEATH & ~terrainImmunities)
        && (!(tFlags & T_ENTANGLES) || !(monst->info.flags & MONST_IMMUNE_TO_WEBS))
        && (monst != &player || rogue.mapToShore[p.x][p.y] >= max(player.status[STATUS_IMMUNE_TO_FIRE], player.status[STATUS_LEVITATING]))) {
        return true;
    }

    // deep water
    if ((tFlags & T_IS_DEEP_WATER & ~terrainImmunities)
        && (!(tFlags & T_ENTANGLES) || !(monst->info.flags & MONST_IMMUNE_TO_WEBS))
        && !cellHasTerrainFlag(monst->loc, T_IS_DEEP_WATER)) {
        return true; // avoid only if not already in it
    }

    // poisonous lichen
    if ((tFlags & T_CAUSES_POISON & ~terrainImmunities)
        && !cellHasTerrainFlag(monst->loc, T_CAUSES_POISON)
        && (monst == &player || monst->creatureState != MONSTER_TRACKING_SCENT || monst->currentHP < 10)) {
        return true;
    }

    // Smart monsters don't attack in corridors if they belong to a group and they can help it.
    if ((monst->info.abilityFlags & MA_AVOID_CORRIDORS)
        && !(monst->status[STATUS_ENRAGED] && monst->currentHP <= monst->info.maxHP / 2)
        && monst->creatureState == MONSTER_TRACKING_SCENT
        && (monst->bookkeepingFlags & (MB_FOLLOWER | MB_LEADER))
        && passableArcCount(p.x, p.y) >= 2
        && passableArcCount(monst->loc.x, monst->loc.y) < 2
        && !cellHasTerrainFlag(monst->loc, (T_HARMFUL_TERRAIN & ~terrainImmunities))) {
        return true;
    }

    return false;
}

/// @brief Attempts to utilize a monster's turn by either initiating movement or launching an attack.
/// Aims to shift the monster one space closer to the destination by evaluating the feasibility
/// of moves in different directions. If the destination is occupied by an accessible enemy within
/// melee range (including whip/spear), the monster will attack instead of moving.
/// @param monst the monster
/// @param targetLoc the destination
/// @param willingToAttackPlayer
/// @return true if a turn-consuming action was performed
static boolean moveMonsterPassivelyTowards(creature *monst, pos targetLoc, boolean willingToAttackPlayer) {
    const int x = monst->loc.x;
    const int y = monst->loc.y;

    const int dx = signum(targetLoc.x - x);
    const int dy = signum(targetLoc.y - y);

    if (dx == 0 && dy == 0) { // already at the destination
        return false;
    }

    const int newX = x + dx;
    const int newY = y + dy;

    if (!coordinatesAreInMap(newX, newY)) {
        return false;
    }

    if (monst->creatureState != MONSTER_TRACKING_SCENT && dx && dy) {
        if (abs(targetLoc.x - x) > abs(targetLoc.y - y) && rand_range(0, abs(targetLoc.x - x)) > abs(targetLoc.y - y)) {
            if (!(monsterAvoids(monst, (pos){newX, y}) || (!willingToAttackPlayer && (pmap[newX][y].flags & HAS_PLAYER)) || !moveMonster(monst, dx, 0))) {
                return true;
            }
        } else if (abs(targetLoc.x - x) < abs(targetLoc.y - y) && rand_range(0, abs(targetLoc.y - y)) > abs(targetLoc.x - x)) {
            if (!(monsterAvoids(monst, (pos){x, newY}) || (!willingToAttackPlayer && (pmap[x][newY].flags & HAS_PLAYER)) || !moveMonster(monst, 0, dy))) {
                return true;
            }
        }
    }

    // Try to move toward the goal diagonally if possible or else straight.
    // If that fails, try both directions for the shorter coordinate.
    // If they all fail, return false.
    if (monsterAvoids(monst, (pos){newX, newY}) || (!willingToAttackPlayer && (pmap[newX][newY].flags & HAS_PLAYER)) || !moveMonster(monst, dx, dy)) {
        if (distanceBetween((pos){x, y}, targetLoc) <= 1 && (dx == 0 || dy == 0)) { // cardinally adjacent
            return false; // destination is blocked
        }
        //abs(targetLoc.x - x) < abs(targetLoc.y - y)
        if ((max(targetLoc.x, x) - min(targetLoc.x, x)) < (max(targetLoc.y, y) - min(targetLoc.y, y))) {
            if (monsterAvoids(monst, (pos){x, newY}) || (!willingToAttackPlayer && pmap[x][newY].flags & HAS_PLAYER) || !moveMonster(monst, 0, dy)) {
                if (monsterAvoids(monst, (pos){newX, y}) || (!willingToAttackPlayer &&  pmap[newX][y].flags & HAS_PLAYER) || !moveMonster(monst, dx, 0)) {
                    if (monsterAvoids(monst, (pos){x-1, newY}) || (!willingToAttackPlayer && pmap[x-1][newY].flags & HAS_PLAYER) || !moveMonster(monst, -1, dy)) {
                        if (monsterAvoids(monst, (pos){x+1, newY}) || (!willingToAttackPlayer && pmap[x+1][newY].flags & HAS_PLAYER) || !moveMonster(monst, 1, dy)) {
                            return false;
                        }
                    }
                }
            }
        } else {
            if (monsterAvoids(monst, (pos){newX, y}) || (!willingToAttackPlayer && pmap[newX][y].flags & HAS_PLAYER) || !moveMonster(monst, dx, 0)) {
                if (monsterAvoids(monst, (pos){x, newY}) || (!willingToAttackPlayer && pmap[x][newY].flags & HAS_PLAYER) || !moveMonster(monst, 0, dy)) {
                    if (monsterAvoids(monst, (pos){newX, y-1}) || (!willingToAttackPlayer && pmap[newX][y-1].flags & HAS_PLAYER) || !moveMonster(monst, dx, -1)) {
                        if (monsterAvoids(monst, (pos){newX, y+1}) || (!willingToAttackPlayer && pmap[newX][y+1].flags & HAS_PLAYER) || !moveMonster(monst, dx, 1)) {
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true;
}

short distanceBetween(pos loc1, pos loc2) {
    return max(abs(loc1.x - loc2.x), abs(loc1.y - loc2.y));
}

void alertMonster(creature *monst) {
    monst->creatureState = (monst->creatureMode == MODE_PERM_FLEEING ? MONSTER_FLEEING : MONSTER_TRACKING_SCENT);
    monst->lastSeenPlayerAt = player.loc;
}

void wakeUp(creature *monst) {
    if (monst->creatureState != MONSTER_ALLY) {
        alertMonster(monst);
    }
    monst->ticksUntilTurn = 100;
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *teammate = nextCreature(&it);
        if (monst != teammate && monstersAreTeammates(monst, teammate) && teammate->creatureMode == MODE_NORMAL) {
            if (teammate->creatureState == MONSTER_SLEEPING
                || teammate->creatureState == MONSTER_WANDERING) {
                teammate->ticksUntilTurn = max(100, teammate->ticksUntilTurn);
            }
            if (monst->creatureState != MONSTER_ALLY) {
                teammate->creatureState =
                (teammate->creatureMode == MODE_PERM_FLEEING ? MONSTER_FLEEING : MONSTER_TRACKING_SCENT);
                updateMonsterState(teammate);
            }
        }
    }
}

static boolean monsterCanShootWebs(creature *monst) {
    short i;
    for (i=0; monst->info.bolts[i] != 0; i++) {
        const bolt *theBolt = &boltCatalog[monst->info.bolts[i]];
        if (theBolt->pathDF && (tileCatalog[dungeonFeatureCatalog[theBolt->pathDF].tile].flags & T_ENTANGLES)) {
            return true;
        }
    }
    return false;
}

// Assumes that observer is not the player.
// Returns approximately double the actual (quasi-euclidian) distance.
static short awarenessDistance(creature *observer, creature *target) {
    long perceivedDistance;

    // When determining distance from the player for purposes of monster state changes
    // (i.e. whether they start or stop hunting), take the scent value of the monster's tile
    // OR, if the monster is in the player's FOV (including across chasms, through green crystal, etc.),
    // the direct distance -- whichever is less.
    // This means that monsters can aggro within stealth range if they're on the other side
    // of a transparent obstruction, and may just stand motionless but hunting if there's no scent map
    // to guide them, but only as long as the player is within FOV. After that, we switch to wandering
    // and wander toward the last location that we saw the player.
    perceivedDistance = (rogue.scentTurnNumber - scentMap[observer->loc.x][observer->loc.y]); // this value is double the apparent distance
    if ((target == &player && (pmapAt(observer->loc)->flags & IN_FIELD_OF_VIEW))
        || (target != &player && openPathBetween(observer->loc, target->loc))) {

        perceivedDistance = min(perceivedDistance, scentDistance(observer->loc.x, observer->loc.y, target->loc.x, target->loc.y));
    }

    perceivedDistance = min(perceivedDistance, 1000);

    if (perceivedDistance < 0) {
        perceivedDistance = 1000;
    }
    return ((short) perceivedDistance);
}

// yes or no -- observer is aware of the target as of this new turn.
// takes into account whether it is ALREADY aware of the target.
static boolean awareOfTarget(creature *observer, creature *target) {
    short perceivedDistance = awarenessDistance(observer, target);
    short awareness = rogue.stealthRange * 2;
    boolean retval;

    brogueAssert(perceivedDistance >= 0 && awareness >= 0);

    if (observer->info.flags & MONST_ALWAYS_HUNTING) {
        retval = true;
    } else if (observer->info.flags & MONST_IMMOBILE) {
        // Turrets and totems are aware of you iff they are within stealth range.
        // The only exception is mirror totems; they're always ready to shoot because they have "always hunting" set.
        retval = perceivedDistance <= awareness;
    } else if (perceivedDistance > awareness * 3) {
        // out of awareness range, even if hunting
        retval = false;
    } else if (observer->creatureState == MONSTER_TRACKING_SCENT) {
        // already aware of the target, lose track 3% of the time if outside of stealth range.
         if (perceivedDistance > awareness) {
             retval = rand_percent(97);
         } else {
            retval = true;
         }
    } else if (target == &player
        && !(pmapAt(observer->loc)->flags & IN_FIELD_OF_VIEW)) {
        // observer not hunting and player-target not in field of view
        retval = false;
    } else if (perceivedDistance <= awareness) {
        // within range but currently unaware
        retval = rand_percent(25);
    } else {
        retval = false;
    }
    return retval;
}

static short closestWaypointIndexTo(pos p) {
    short i, closestDistance, closestIndex;

    closestDistance = 1000;
    closestIndex = -1;
    for (i=0; i < rogue.wpCount; i++) {
        if (rogue.wpDistance[i][p.x][p.y] < closestDistance) {
            closestDistance = rogue.wpDistance[i][p.x][p.y];
            closestIndex = i;
        }
    }
    return closestIndex;
}

static void wanderToward(creature *monst, pos destination) {
    if (isPosInMap(destination)) {
        const short theWaypointIndex = closestWaypointIndexTo(destination);
        if (theWaypointIndex != -1) {
            monst->waypointAlreadyVisited[theWaypointIndex] = false;
            monst->targetWaypointIndex = theWaypointIndex;
        }
    }
}

void updateMonsterState(creature *monst) {
    short x, y, closestFearedEnemy;
    boolean awareOfPlayer;

    x = monst->loc.x;
    y = monst->loc.y;

    if ((monst->info.flags & MONST_ALWAYS_HUNTING)
        && monst->creatureState != MONSTER_ALLY) {

        monst->creatureState = MONSTER_TRACKING_SCENT;
        return;
    }

    awareOfPlayer = awareOfTarget(monst, &player);

    if ((monst->info.flags & MONST_IMMOBILE)
        && monst->creatureState != MONSTER_ALLY) {

        if (awareOfPlayer) {
            monst->creatureState = MONSTER_TRACKING_SCENT;
        } else {
            monst->creatureState = MONSTER_SLEEPING;
        }
        return;
    }

    if (monst->creatureMode == MODE_PERM_FLEEING
        && (monst->creatureState == MONSTER_WANDERING || monst->creatureState == MONSTER_TRACKING_SCENT)) {

        monst->creatureState = MONSTER_FLEEING;
    }

    closestFearedEnemy = DCOLS+DROWS;

    boolean handledPlayer = false;
    for (creatureIterator it = iterateCreatures(monsters); !handledPlayer || hasNextCreature(it);) {
        creature *monst2 = !handledPlayer ? &player : nextCreature(&it);
        handledPlayer = true;
        if (monsterFleesFrom(monst, monst2)
            && distanceBetween((pos){x, y}, monst2->loc) < closestFearedEnemy
            && traversiblePathBetween(monst2, x, y)
            && openPathBetween((pos){x, y}, monst2->loc)) {

            closestFearedEnemy = distanceBetween((pos){x, y}, monst2->loc);
        }
    }

    if ((monst->creatureState == MONSTER_WANDERING)
        && awareOfPlayer
        && (pmapAt(player.loc)->flags & IN_FIELD_OF_VIEW)) {
        // If wandering and you notice the player, start tracking the scent.
        alertMonster(monst);
    } else if (monst->creatureState == MONSTER_SLEEPING) {
        // if sleeping, the monster has a chance to awaken
        if (awareOfPlayer) {
            wakeUp(monst); // wakes up the whole horde if necessary
        }
    } else if (monst->creatureState == MONSTER_TRACKING_SCENT && !awareOfPlayer) {
        // if tracking scent, but the scent is weaker than the scent detection threshold, begin wandering.
        monst->creatureState = MONSTER_WANDERING;
        wanderToward(monst, monst->lastSeenPlayerAt);
    } else if (monst->creatureState == MONSTER_TRACKING_SCENT
               && closestFearedEnemy < 3) {
        monst->creatureState = MONSTER_FLEEING;
    } else if (monst->creatureState != MONSTER_ALLY
               && (monst->info.flags & MONST_FLEES_NEAR_DEATH)
               && monst->currentHP <= 3 * monst->info.maxHP / 4) {

        if (monst->creatureState == MONSTER_FLEEING
            || monst->currentHP <= monst->info.maxHP / 4) {

            monst->creatureState = MONSTER_FLEEING;
        }
    } else if (monst->creatureMode == MODE_NORMAL
               && monst->creatureState == MONSTER_FLEEING
               && !(monst->status[STATUS_MAGICAL_FEAR])
               && closestFearedEnemy >= 3) {

        monst->creatureState = MONSTER_TRACKING_SCENT;
    } else if (monst->creatureMode == MODE_PERM_FLEEING
               && monst->creatureState == MONSTER_FLEEING
               && (monst->info.abilityFlags & MA_HIT_STEAL_FLEE)
               && !(monst->status[STATUS_MAGICAL_FEAR])
               && !(monst->carriedItem)) {

        monst->creatureMode = MODE_NORMAL;

        if (monst->leader == &player) {
            monst->creatureState = MONSTER_ALLY; // Reset state if a discorded ally steals an item and then loses it (probably in deep water)
        } else {
            alertMonster(monst);
        }

    } else if (monst->creatureMode == MODE_NORMAL
               && monst->creatureState == MONSTER_FLEEING
               && (monst->info.flags & MONST_FLEES_NEAR_DEATH)
               && !(monst->status[STATUS_MAGICAL_FEAR])
               && monst->currentHP >= monst->info.maxHP * 3 / 4) {

        if ((monst->bookkeepingFlags & MB_FOLLOWER) && monst->leader == &player) {
            monst->creatureState = MONSTER_ALLY;
        } else {
            alertMonster(monst);
        }
    }

    if (awareOfPlayer) {
        if (monst->creatureState == MONSTER_FLEEING
            || monst->creatureState == MONSTER_TRACKING_SCENT) {

            monst->lastSeenPlayerAt = player.loc;
        }
    }
}

void decrementMonsterStatus(creature *monst) {
    short i, damage;
    char buf[COLS], buf2[COLS];

    monst->bookkeepingFlags &= ~MB_JUST_SUMMONED;

    if (monst->currentHP < monst->info.maxHP
        && monst->info.turnsBetweenRegen > 0
        && !monst->status[STATUS_POISONED]) {

        if ((monst->turnsUntilRegen -= 1000) <= 0) {
            monst->currentHP++;
            monst->previousHealthPoints++;
            monst->turnsUntilRegen += monst->info.turnsBetweenRegen;
        }
    }

    for (i=0; i<NUMBER_OF_STATUS_EFFECTS; i++) {
        switch (i) {
            case STATUS_LEVITATING:
                if (monst->status[i] && !(monst->info.flags & MONST_FLIES)) {
                    monst->status[i]--;
                }
                break;
            case STATUS_SLOWED:
                if (monst->status[i] && !--monst->status[i]) {
                    monst->movementSpeed = monst->info.movementSpeed;
                    monst->attackSpeed = monst->info.attackSpeed;
                }
                break;
            case STATUS_WEAKENED:
                if (monst->status[i] && !--monst->status[i]) {
                    monst->weaknessAmount = 0;
                }
                break;
            case STATUS_HASTED:
                if (monst->status[i]) {
                    if (!--monst->status[i]) {
                        monst->movementSpeed = monst->info.movementSpeed;
                        monst->attackSpeed = monst->info.attackSpeed;
                    }
                }
                break;
            case STATUS_BURNING:
                if (monst->status[i]) {
                    if (!(monst->info.flags & MONST_FIERY)) {
                        monst->status[i]--;
                    }
                    damage = rand_range(1, 3);
                    if (!(monst->status[STATUS_IMMUNE_TO_FIRE])
                        && !(monst->info.flags & MONST_INVULNERABLE)
                        && inflictDamage(NULL, monst, damage, &orange, true)) {

                        if (canSeeMonster(monst)) {
                            monsterName(buf, monst, true);
                            sprintf(buf2, "%s burns %s.",
                                    buf,
                                    (monst->info.flags & MONST_INANIMATE) ? "up" : "to death");
                            messageWithColor(buf2, messageColorFromVictim(monst), 0);
                        }
                        killCreature(monst, false);
                        return;
                    }
                    if (monst->status[i] <= 0) {
                        extinguishFireOnCreature(monst);
                    }
                }
                break;
            case STATUS_LIFESPAN_REMAINING:
                if (monst->status[i]) {
                    monst->status[i]--;
                    if (monst->status[i] <= 0) {
                        killCreature(monst, false);
                        if (canSeeMonster(monst)) {
                            monsterName(buf, monst, true);
                            sprintf(buf2, "%s dissipates into thin air.", buf);
                            messageWithColor(buf2, &white, 0);
                        }
                        return;
                    }
                }
                break;
            case STATUS_POISONED:
                if (monst->status[i]) {
                    monst->status[i]--;
                    if (inflictDamage(NULL, monst, monst->poisonAmount, &green, true)) {
                        if (canSeeMonster(monst)) {
                            monsterName(buf, monst, true);
                            sprintf(buf2, "%s dies of poison.", buf);
                            messageWithColor(buf2, messageColorFromVictim(monst), 0);
                        }
                        killCreature(monst, false);
                        return;
                    }
                    if (!monst->status[i]) {
                        monst->poisonAmount = 0;
                    }
                }
                break;
            case STATUS_STUCK:
                if (monst->status[i] && !cellHasTerrainFlag(monst->loc, T_ENTANGLES)) {
                    monst->status[i] = 0;
                }
                break;
            case STATUS_DISCORDANT:
                if (monst->status[i] && !--monst->status[i]) {
                    if (monst->creatureState == MONSTER_FLEEING
                        && !monst->status[STATUS_MAGICAL_FEAR]
                        && monst->leader == &player) {

                        monst->creatureState = MONSTER_ALLY;
                        if (monst->carriedItem) {
                            makeMonsterDropItem(monst);
                        }
                    }
                }
                break;
            case STATUS_MAGICAL_FEAR:
                if (monst->status[i]) {
                    if (!--monst->status[i]) {
                        monst->creatureState = (monst->leader == &player ? MONSTER_ALLY : MONSTER_TRACKING_SCENT);
                    }
                }
                break;
            case STATUS_SHIELDED:
                monst->status[i] -= monst->maxStatus[i] / 20;
                if (monst->status[i] <= 0) {
                    monst->status[i] = monst->maxStatus[i] = 0;
                }
                break;
            case STATUS_IMMUNE_TO_FIRE:
                if (monst->status[i] && !(monst->info.flags & MONST_IMMUNE_TO_FIRE)) {
                    monst->status[i]--;
                }
                break;
            case STATUS_INVISIBLE:
                if (monst->status[i]
                    && !(monst->info.flags & MONST_INVISIBLE)
                    && !--monst->status[i]
                    && playerCanSee(monst->loc.x, monst->loc.y)) {

                    refreshDungeonCell(monst->loc);
                }
                break;
            default:
                if (monst->status[i]) {
                    monst->status[i]--;
                }
                break;
        }
    }

    if (monsterCanSubmergeNow(monst) && !(monst->bookkeepingFlags & MB_SUBMERGED)) {
        if (rand_percent(20)) {
            monst->bookkeepingFlags |= MB_SUBMERGED;
            if (!monst->status[STATUS_MAGICAL_FEAR]
                && monst->creatureState == MONSTER_FLEEING
                && (!(monst->info.flags & MONST_FLEES_NEAR_DEATH) || monst->currentHP >= monst->info.maxHP * 3 / 4)) {

                monst->creatureState = MONSTER_TRACKING_SCENT;
            }
            refreshDungeonCell(monst->loc);
        } else if (monst->info.flags & (MONST_RESTRICTED_TO_LIQUID)
                   && monst->creatureState != MONSTER_ALLY) {
            monst->creatureState = MONSTER_FLEEING;
        }
    }
}

boolean traversiblePathBetween(creature *monst, short x2, short y2) {
    pos originLoc = monst->loc;
    pos targetLoc = (pos){ .x = x2, .y = y2 };

    // Using BOLT_NONE here to favor a path that avoids obstacles to one that hits them
    pos coords[DCOLS];
    int n = getLineCoordinates(coords, originLoc, targetLoc, &boltCatalog[BOLT_NONE]);

    for (int i=0; i<n; i++) {
        if (posEq(coords[i], targetLoc)) {
            return true;
        }
        if (monsterAvoids(monst, coords[i])) {
            return false;
        }
    }
    brogueAssert(false);
    return true; // should never get here
}

boolean specifiedPathBetween(short x1, short y1, short x2, short y2,
                             unsigned long blockingTerrain, unsigned long blockingFlags) {
    pos originLoc = (pos){ .x = x1, .y = y1 };
    pos targetLoc = (pos){ .x = x2, .y = y2 };
    pos coords[DCOLS];
    int n = getLineCoordinates(coords, originLoc, targetLoc, &boltCatalog[BOLT_NONE]);

    for (int i=0; i<n; i++) {
        short x = coords[i].x;
        short y = coords[i].y;
        if (cellHasTerrainFlag((pos){ x, y }, blockingTerrain) || (pmap[x][y].flags & blockingFlags)) {
            return false;
        }
        if (x == x2 && y == y2) {
            return true;
        }
    }
    brogueAssert(false);
    return true; // should never get here
}

boolean openPathBetween(const pos startLoc, const pos targetLoc) {

    pos returnLoc;
    getImpactLoc(&returnLoc, startLoc, targetLoc, DCOLS, false, &boltCatalog[BOLT_NONE]);
    return posEq(returnLoc,targetLoc);
}

// will return the player if the player is at (p.x, p.y).
creature *monsterAtLoc(pos p) {
    if (!(pmapAt(p)->flags & (HAS_MONSTER | HAS_PLAYER))) {
        return NULL;
    }
    if (posEq(player.loc, p)) {
        return &player;
    }
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (posEq(monst->loc, p)) {
            return monst;
        }
    }
    // This should be unreachable, since the HAS_MONSTER
    // flag was true at (x, y).
    brogueAssert(0);
    return NULL;
}

creature *dormantMonsterAtLoc(pos p) {
    if (!(pmapAt(p)->flags & HAS_DORMANT_MONSTER)) {
        return NULL;
    }

    for (creatureIterator it = iterateCreatures(dormantMonsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (posEq(monst->loc, p)) {
            return monst;
        }
    }
    // This should be unreachable, since the HAS_DORMANT_MONSTER
    // flag was true at (x, y).
    brogueAssert(0);
    return NULL;
}

static enum boltType monsterHasBoltEffect(creature *monst, enum boltEffects boltEffectIndex) {
    short i;
    for (i=0; monst->info.bolts[i] != 0; i++) {
        if (boltCatalog[monst->info.bolts[i]].boltEffect == boltEffectIndex) {
            return monst->info.bolts[i];
        }
    }
    return BOLT_NONE;
}

static void pathTowardCreature(creature *monst, creature *target) {
    if (traversiblePathBetween(monst, target->loc.x, target->loc.y)) {
        if (distanceBetween(monst->loc, target->loc) <= 2) {
            monst->bookkeepingFlags &= ~MB_GIVEN_UP_ON_SCENT;
        }
        moveMonsterPassivelyTowards(monst, target->loc, (monst->creatureState != MONSTER_ALLY));
        return;
    }

    // is the target missing his map altogether?
    if (!target->mapToMe) {
        target->mapToMe = allocGrid();
        fillGrid(target->mapToMe, 0);
        calculateDistances(target->mapToMe, target->loc.x, target->loc.y, 0, monst, true, false);
    }

    // is the target map out of date?
    if (target->mapToMe[target->loc.x][target->loc.y] > 3) {
        // it is. recalculate the map.
        calculateDistances(target->mapToMe, target->loc.x, target->loc.y, 0, monst, true, false);
    }

    // blink to the target?
    if (distanceBetween(monst->loc, target->loc) > 10
        || monstersAreEnemies(monst, target)) {

        if (monsterBlinkToPreferenceMap(monst, target->mapToMe, false)) { // if it blinked
            monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
            return;
        }
    }

    // follow the map.
    short dir = nextStep(target->mapToMe, monst->loc.x, monst->loc.y, monst, true);
    if (dir == NO_DIRECTION) {
        dir = randValidDirectionFrom(monst, monst->loc.x, monst->loc.y, true);
    }
    if (dir == NO_DIRECTION) {
        return; // monster is blocked
    }
    pos targetLoc = posNeighborInDirection(monst->loc, dir);

    moveMonsterPassivelyTowards(monst, targetLoc, (monst->creatureState != MONSTER_ALLY));
}

static boolean creatureEligibleForSwarming(creature *monst) {
    if ((monst->info.flags & (MONST_IMMOBILE | MONST_GETS_TURN_ON_ACTIVATION | MONST_MAINTAINS_DISTANCE))
        || monst->status[STATUS_ENTRANCED]
        || monst->status[STATUS_CONFUSED]
        || monst->status[STATUS_STUCK]
        || monst->status[STATUS_PARALYZED]
        || monst->status[STATUS_MAGICAL_FEAR]
        || monst->status[STATUS_LIFESPAN_REMAINING] == 1
        || (monst->bookkeepingFlags & (MB_SEIZED | MB_SEIZING))) {

        return false;
    }
    if (monst != &player
        && monst->creatureState != MONSTER_ALLY
        && monst->creatureState != MONSTER_TRACKING_SCENT) {

        return false;
    }
    return true;
}

// Swarming behavior.
// If you’re adjacent to an enemy and about to strike it, and you’re adjacent to a hunting-mode tribemate
// who is not adjacent to another enemy, and there is no empty space adjacent to the tribemate AND the enemy,
// and there is an empty space adjacent to you AND the enemy, then move into that last space.
// (In each case, "adjacent" excludes diagonal tiles obstructed by corner walls.)
static enum directions monsterSwarmDirection(creature *monst, creature *enemy) {
    enum directions dir, targetDir;
    short dirList[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    boolean alternateDirectionExists;

    if (monst == &player || !creatureEligibleForSwarming(monst)) {
        return NO_DIRECTION;
    }

    if (distanceBetween(monst->loc, enemy->loc) != 1
        || (diagonalBlocked(monst->loc.x, monst->loc.y, enemy->loc.x, enemy->loc.y, false) || (enemy->info.flags & MONST_ATTACKABLE_THRU_WALLS))
        || !monstersAreEnemies(monst, enemy)) {

        return NO_DIRECTION; // Too far from the enemy, diagonally blocked, or not enemies with it.
    }

    // Find a location that is adjacent to you and to the enemy.
    targetDir = NO_DIRECTION;
    shuffleList(dirList, 4);
    shuffleList(&(dirList[4]), 4);
    for (int i=0; i<8 && targetDir == NO_DIRECTION; i++) {
        dir = dirList[i];
        const pos newLoc = posNeighborInDirection(monst->loc, dir);
        if (isPosInMap(newLoc)
            && distanceBetween(enemy->loc, newLoc) == 1
            && !(pmapAt(newLoc)->flags & (HAS_PLAYER | HAS_MONSTER))
            && !diagonalBlocked(monst->loc.x, monst->loc.y, newLoc.x, newLoc.y, false)
            && (!diagonalBlocked(enemy->loc.x, enemy->loc.y, newLoc.x, newLoc.y, false) || (enemy->info.flags & MONST_ATTACKABLE_THRU_WALLS))
            && !monsterAvoids(monst, newLoc)) {

            targetDir = dir;
        }
    }
    if (targetDir == NO_DIRECTION) {
        return NO_DIRECTION; // No open location next to both you and the enemy.
    }

    // OK, now we have a place to move toward. Let's analyze the teammates around us to make sure that
    // one of them could take advantage of the space we open.
    boolean handledPlayer = false;
    for (creatureIterator it = iterateCreatures(monsters); !handledPlayer || hasNextCreature(it);) {
        creature *ally = !handledPlayer ? &player : nextCreature(&it);
        handledPlayer = true;
        if (ally != monst
            && ally != enemy
            && monstersAreTeammates(monst, ally)
            && monstersAreEnemies(ally, enemy)
            && creatureEligibleForSwarming(ally)
            && distanceBetween(monst->loc, ally->loc) == 1
            && !diagonalBlocked(monst->loc.x, monst->loc.y, ally->loc.x, ally->loc.y, false)
            && !monsterAvoids(ally, monst->loc)
            && (distanceBetween(enemy->loc, ally->loc) > 1 || diagonalBlocked(enemy->loc.x, enemy->loc.y, ally->loc.x, ally->loc.y, false))) {

            // Found a prospective ally.
            // Check that there isn't already an open space from which to attack the enemy that is accessible to the ally.
            alternateDirectionExists = false;
            for (dir=0; dir< DIRECTION_COUNT && !alternateDirectionExists; dir++) {
                const pos newPos = posNeighborInDirection(ally->loc, dir);
                if (isPosInMap(newPos)
                    && !(pmapAt(newPos)->flags & (HAS_PLAYER | HAS_MONSTER))
                    && distanceBetween(enemy->loc, newPos) == 1
                    && !diagonalBlocked(enemy->loc.x, enemy->loc.y, newPos.x, newPos.y, false)
                    && !diagonalBlocked(ally->loc.x, ally->loc.y, newPos.x, newPos.y, false)
                    && !monsterAvoids(ally, newPos)) {

                    alternateDirectionExists = true;
                }
            }
            if (!alternateDirectionExists) {
                // OK, no alternative open spaces exist.
                // Check that the ally isn't already occupied with an enemy of its own.
                boolean foundConflict = false;
                boolean handledPlayer = false;
                for (creatureIterator it2 = iterateCreatures(monsters); !handledPlayer || hasNextCreature(it2);) {
                    creature *otherEnemy = !handledPlayer ? &player : nextCreature(&it2);
                    handledPlayer = true;
                    if (ally != otherEnemy
                        && monst != otherEnemy
                        && enemy != otherEnemy
                        && monstersAreEnemies(ally, otherEnemy)
                        && distanceBetween(ally->loc, otherEnemy->loc) == 1
                        && (!diagonalBlocked(ally->loc.x, ally->loc.y, otherEnemy->loc.x, otherEnemy->loc.y, false) || (otherEnemy->info.flags & MONST_ATTACKABLE_THRU_WALLS))) {

                        foundConflict = true;
                        break; // Ally is already occupied.
                    }
                }
                if (!foundConflict) {
                    // Success!
                    return targetDir;
                }
            }
        }
    }
    return NO_DIRECTION; // Failure!
}

// Isomorphs a number in [0, 39] to coordinates along the square of radius 5 surrounding (0,0).
// This is used as the sample space for bolt target coordinates, e.g. when reflecting or when
// monsters are deciding where to blink.
pos perimeterCoords(short n) {
    if (n <= 10) {          // top edge, left to right
        return (pos){
            .x = n - 5,
            .y = -5
        };
    } else if (n <= 21) {   // bottom edge, left to right
        return (pos){
            .x = (n - 11) - 5,
            .y = 5
        };
    } else if (n <= 30) {   // left edge, top to bottom
        return (pos){
            .x = -5,
            .y = (n - 22) - 4
        };
    } else if (n <= 39) {   // right edge, top to bottom
        return (pos){
            .x = 5,
            .y = (n - 31) - 4
        };
    } else {
        message("ERROR! Bad perimeter coordinate request!", REQUIRE_ACKNOWLEDGMENT);
        return (pos){ .x = 0, .y = 0 }; // garbage in, garbage out
    }
}

// Tries to make the monster blink to the most desirable square it can aim at, according to the
// preferenceMap argument. "blinkUphill" determines whether it's aiming for higher or lower numbers on
// the preference map -- true means higher. Returns true if the monster blinked; false if it didn't.
boolean monsterBlinkToPreferenceMap(creature *monst, short **preferenceMap, boolean blinkUphill) {
    short i, nowPreference, maxDistance;
    boolean gotOne;
    char monstName[DCOLS];
    char buf[DCOLS];
    enum boltType theBoltType;
    bolt theBolt;

    theBoltType = monsterHasBoltEffect(monst, BE_BLINKING);
    if (!theBoltType) {
        return false;
    }

    maxDistance = staffBlinkDistance(5 * FP_FACTOR);
    gotOne = false;

    pos origin = monst->loc;
    pos bestTarget = (pos){ .x = 0, .y = 0 };
    short bestPreference = preferenceMap[monst->loc.x][monst->loc.y];

    // make sure that we beat the four cardinal neighbors
    for (i = 0; i < 4; i++) {
        const pos monstNeighborLoc = posNeighborInDirection(monst->loc, i);
        nowPreference = preferenceMap[monstNeighborLoc.x][monstNeighborLoc.y];

        if (((blinkUphill && nowPreference > bestPreference) || (!blinkUphill && nowPreference < bestPreference))
            && !monsterAvoids(monst, monstNeighborLoc)) {

            bestPreference = nowPreference;
        }
    }

    for (i=0; i<40; i++) {
        pos target = perimeterCoords(i);
        target.x += monst->loc.x;
        target.y += monst->loc.y;

        pos impact;
        getImpactLoc(&impact, origin, target, maxDistance, true, &boltCatalog[BOLT_BLINKING]);
        nowPreference = preferenceMap[impact.x][impact.y];

        if (((blinkUphill && (nowPreference > bestPreference))
             || (!blinkUphill && (nowPreference < bestPreference)))
            && !monsterAvoids(monst, impact)) {

            bestTarget = target;
            bestPreference  = nowPreference;

            if ((abs(impact.x - origin.x) > 1 || abs(impact.y - origin.y) > 1)
                // Note: these are deliberately backwards:
                || (cellHasTerrainFlag((pos){ impact.x, origin.y }, T_OBSTRUCTS_PASSABILITY))
                || (cellHasTerrainFlag((pos){ origin.x, impact.y }, T_OBSTRUCTS_PASSABILITY))) {
                gotOne = true;
            } else {
                gotOne = false;
            }
        }
    }

    if (gotOne) {
        if (canDirectlySeeMonster(monst)) {
            monsterName(monstName, monst, true);
            sprintf(buf, "%s blinks", monstName);
            combatMessage(buf, 0);
        }
        monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
        theBolt = boltCatalog[theBoltType];
        zap(origin, bestTarget, &theBolt, false, false);
        return true;
    }
    return false;
}

static boolean fleeingMonsterAwareOfPlayer(creature *monst) {
    if (player.status[STATUS_INVISIBLE]) {
        return (distanceBetween(monst->loc, player.loc) <= 1);
    } else {
        return (pmapAt(monst->loc)->flags & IN_FIELD_OF_VIEW) ? true : false;
    }
}

static short **getSafetyMap(creature *monst) {
    if (fleeingMonsterAwareOfPlayer(monst)) {
        if (monst->safetyMap) {
            freeGrid(monst->safetyMap);
            monst->safetyMap = NULL;
        }
        if (!rogue.updatedSafetyMapThisTurn) {
            updateSafetyMap();
        }
        return safetyMap;
    } else {
        if (!monst->safetyMap) {
            if (!rogue.updatedSafetyMapThisTurn) {
                updateSafetyMap();
            }
            monst->safetyMap = allocGrid();
            copyGrid(monst->safetyMap, safetyMap);
        }
        return monst->safetyMap;
    }
}

// returns whether the monster did something (and therefore ended its turn)
static boolean monsterBlinkToSafety(creature *monst) {
    short **blinkSafetyMap;

    if (monst->creatureState == MONSTER_ALLY) {
        if (!rogue.updatedAllySafetyMapThisTurn) {
            updateAllySafetyMap();
        }
        blinkSafetyMap = allySafetyMap;
    } else {
        blinkSafetyMap = getSafetyMap(monst);
    }

    return monsterBlinkToPreferenceMap(monst, blinkSafetyMap, false);
}

boolean monsterSummons(creature *monst, boolean alwaysUse) {
    short minionCount = 0;

    if (monst->info.abilityFlags & (MA_CAST_SUMMON)) {
        // Count existing minions.
        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *target = nextCreature(&it);
            if (monst->creatureState == MONSTER_ALLY) {
                if (target->creatureState == MONSTER_ALLY) {
                    minionCount++; // Allied summoners count all allies.
                }
            } else if ((target->bookkeepingFlags & MB_FOLLOWER) && target->leader == monst) {
                minionCount++; // Enemy summoners count only direct followers, not teammates.
            }
        }
        if (monst->creatureState == MONSTER_ALLY) { // Allied summoners also count monsters on the previous and next depths.
            if (rogue.depthLevel > 1) {
                for (creatureIterator it = iterateCreatures(&levels[rogue.depthLevel - 2].monsters); hasNextCreature(it);) {
                    creature *target = nextCreature(&it);
                    if (target->creatureState == MONSTER_ALLY && !(target->info.flags & MONST_WILL_NOT_USE_STAIRS)) {
                        minionCount++;
                    }
                }
            }
            if (rogue.depthLevel < gameConst->deepestLevel) {
                for (creatureIterator it = iterateCreatures(&levels[rogue.depthLevel].monsters); hasNextCreature(it);) {
                    creature *target = nextCreature(&it);
                    if (target->creatureState == MONSTER_ALLY && !(target->info.flags & MONST_WILL_NOT_USE_STAIRS)) {
                        minionCount++;
                    }
                }
            }
        }
        if (alwaysUse && minionCount < 50) {
            summonMinions(monst);
            return true;
        } else if (monst->info.abilityFlags & MA_ENTER_SUMMONS) {
            if (!rand_range(0, 7)) {
                summonMinions(monst);
                return true;
            }
        } else if ((monst->creatureState != MONSTER_ALLY || minionCount < 5)
                   && !rand_range(0, minionCount * minionCount * 3 + 1)) {

            summonMinions(monst);
            return true;
        }
    }
    return false;
}

/// @brief Checks if a creature has any negatable status effects
/// @param monst The creature
/// @return True if the creature has any negatable status effects
boolean canNegateCreatureStatusEffects(creature *monst) {

    if (!monst || (monst->info.flags & MONST_INVULNERABLE)) {
        return false;
    }

    boolean hasNegatableStatusEffect = false;
    for (int i = 0; i < NUMBER_OF_STATUS_EFFECTS; i++) {
        enum statusEffects theStatus = (enum statusEffects) i;
        if (monst->status[theStatus] > 0 && statusEffectCatalog[theStatus].isNegatable) {
            hasNegatableStatusEffect = true;
        }
    }
    return hasNegatableStatusEffect;
}

/// @brief Negates a creature's negatable status effects
/// @param monst The creature
void negateCreatureStatusEffects(creature *monst) {

    if (!monst || (monst->info.flags & MONST_INVULNERABLE)) {
        return;
    }

    for (int i = 0; i < NUMBER_OF_STATUS_EFFECTS; i++) {
        enum statusEffects theStatus = (enum statusEffects) i;
        if (monst->status[theStatus] > 0 && statusEffectCatalog[theStatus].isNegatable) {
            monst->status[theStatus] = (monst == &player) ? statusEffectCatalog[theStatus].playerNegatedValue : 0;
            if (theStatus == STATUS_DARKNESS && monst == &player) {
                updateMinersLightRadius();
                updateVision(true);
            }
        }
    }
}

/// @brief Checks if a monster will be affected by negation
/// @param monst The monster
/// @return True if negation will have an effect
boolean monsterIsNegatable(creature *monst) {

    if (monst->info.flags & MONST_INVULNERABLE) {
        return false;
    }

    if ((monst->info.abilityFlags & ~MA_NON_NEGATABLE_ABILITIES)
        || (monst->bookkeepingFlags & MB_SEIZING)
        || (monst->info.flags & MONST_DIES_IF_NEGATED)
        || (monst->info.flags & NEGATABLE_TRAITS)
        || (monst->info.flags & MONST_IMMUNE_TO_FIRE)
        || ((monst->info.flags & MONST_FIERY) && (monst->status[STATUS_BURNING]))
        || canNegateCreatureStatusEffects(monst)
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

// Some monsters never make good targets irrespective of what bolt we're contemplating.
// Return false for those. Otherwise, return true.
// Used for monster-cast bolts only.
static boolean generallyValidBoltTarget(creature *caster, creature *target) {
    if (caster == target) {
        // Can't target yourself; that's the fundamental theorem of Brogue bolts.
        return false;
    }
    if (caster->status[STATUS_DISCORDANT]
        && caster->creatureState == MONSTER_WANDERING
        && target == &player) {
        // Discordant monsters always try to cast spells regardless of whether
        // they're hunting the player, so that they cast at other monsters. This
        // by bypasses the usual awareness checks, so the player and any allies
        // can be hit when far away. Hence, we don't target the player with
        // bolts if we're discordant and wandering.
        return false;
    }
    if (caster->creatureState == MONSTER_ALLY && !caster->status[STATUS_DISCORDANT]
            && (target->bookkeepingFlags & MB_MARKED_FOR_SACRIFICE)) {
        // Don't let (sane) allies cast at sacrifice targets.
        return false;
    }

    if (monsterIsHidden(target, caster)
        || (target->bookkeepingFlags & MB_SUBMERGED)) {
        // No bolt will affect a submerged creature. Can't shoot at invisible creatures unless it's in gas.
        return false;
    }
    return openPathBetween(caster->loc, target->loc);
}

static boolean targetEligibleForCombatBuff(creature *caster, creature *target) {
    if (caster->creatureState == MONSTER_ALLY) {
        if (canDirectlySeeMonster(caster)) {
            boolean handledPlayer = false;
            for (creatureIterator it = iterateCreatures(monsters); !handledPlayer || hasNextCreature(it);) {
                creature *enemy = !handledPlayer ? &player : nextCreature(&it);
                handledPlayer = true;
                if (monstersAreEnemies(&player, enemy)
                    && canSeeMonster(enemy)
                    && (pmapAt(enemy->loc)->flags & IN_FIELD_OF_VIEW)) {

                    return true;
                }
            }
        }
        return false;
    } else {
        return (target->creatureState == MONSTER_TRACKING_SCENT);
    }
}

// Make a decision as to whether the given caster should fire the given bolt at the given target.
// Assumes that the conditions in generallyValidBoltTarget have already been satisfied.
// Used for monster-cast bolts only.
static boolean specificallyValidBoltTarget(creature *caster, creature *target, enum boltType theBoltType) {

    if ((boltCatalog[theBoltType].flags & BF_TARGET_ALLIES)
        && (!monstersAreTeammates(caster, target) || monstersAreEnemies(caster, target))) {

        return false;
    }
    if ((boltCatalog[theBoltType].flags & BF_TARGET_ENEMIES)
        && (!monstersAreEnemies(caster, target))) {

        return false;
    }
    if ((boltCatalog[theBoltType].flags & BF_TARGET_ENEMIES)
        && (target->info.flags & MONST_INVULNERABLE)) {

        return false;
    }
    if (((target->info.flags & MONST_REFLECT_50) || (target->info.abilityFlags & MA_REFLECT_100))
        && target->creatureState != MONSTER_ALLY
        && !(boltCatalog[theBoltType].flags & (BF_NEVER_REFLECTS | BF_HALTS_BEFORE_OBSTRUCTION))) {
        // Don't fire a reflectable bolt at a reflective target unless it's your ally.
        return false;
    }
    if (boltCatalog[theBoltType].forbiddenMonsterFlags & target->info.flags) {
        // Don't fire a bolt at a creature type that it won't affect.
        return false;
    }
    if ((boltCatalog[theBoltType].flags & BF_FIERY)
        && target->status[STATUS_IMMUNE_TO_FIRE]) {
        // Don't shoot fireballs at fire-immune creatures.
        return false;
    }
    if ((boltCatalog[theBoltType].flags & BF_FIERY)
        && burnedTerrainFlagsAtLoc(caster->loc) & avoidedFlagsForMonster(&(caster->info))) {
        // Don't shoot fireballs if you're standing on a tile that could combust into something that harms you.
        return false;
    }

    // Rules specific to bolt effects:
    switch (boltCatalog[theBoltType].boltEffect) {
        case BE_BECKONING:
            if (distanceBetween(caster->loc, target->loc) <= 1) {
                return false;
            }
            break;
        case BE_ATTACK:
            if (cellHasTerrainFlag(target->loc, T_OBSTRUCTS_PASSABILITY)
                && !(target->info.flags & MONST_ATTACKABLE_THRU_WALLS)) {
                // Don't shoot an arrow at an embedded creature.
                return false;
            }
            // continue to BE_DAMAGE below
        case BE_DAMAGE:
            if (target->status[STATUS_ENTRANCED]
                && monstersAreEnemies(caster, target)) {
                // Don't break your enemies' entrancement.
                return false;
            }
            break;
        case BE_NONE:
            // BE_NONE bolts are always going to be all about the terrain effects,
            // so our logic has to follow from the terrain parameters of the bolt's target DF.
            if (boltCatalog[theBoltType].targetDF) {
                const unsigned long terrainFlags = tileCatalog[dungeonFeatureCatalog[boltCatalog[theBoltType].targetDF].tile].flags;
                if ((terrainFlags & T_ENTANGLES)
                    && target->status[STATUS_STUCK]) {
                    // Don't try to entangle a creature that is already entangled.
                    return false;
                }
                if ((boltCatalog[theBoltType].flags & BF_TARGET_ENEMIES)
                    && !(terrainFlags & avoidedFlagsForMonster(&(target->info)))
                    && (!(terrainFlags & T_ENTANGLES) || (target->info.flags & MONST_IMMUNE_TO_WEBS))) {

                    return false;
                }
            }
            break;
        case BE_DISCORD:
            if (target->status[STATUS_DISCORDANT]
                || target == &player) {
                // Don't cast discord if the target is already discordant, or if it is the player.
                // (Players should never be intentionally targeted by discord. It's just a fact of monster psychology.)
                return false;
            }
            break;
        case BE_NEGATION:
            if (monstersAreEnemies(caster, target)) {
                if (target->status[STATUS_HASTED] || target->status[STATUS_TELEPATHIC] || target->status[STATUS_SHIELDED]) {
                    // Dispel haste, telepathy, protection.
                    return true;
                }
                if (target->info.flags & (MONST_DIES_IF_NEGATED | MONST_IMMUNE_TO_WEAPONS)) {
                    // Dispel magic creatures; strip weapon invulnerability from revenants.
                    return true;
                }
                if ((target->status[STATUS_IMMUNE_TO_FIRE] || target->status[STATUS_LEVITATING])
                    && cellHasTerrainFlag(target->loc, (T_LAVA_INSTA_DEATH | T_IS_DEEP_WATER | T_AUTO_DESCENT))) {
                    // Drop the target into lava or a chasm if opportunity knocks.
                    return true;
                }
                if (monstersAreTeammates(caster, target)
                    && target->status[STATUS_DISCORDANT]
                    && !caster->status[STATUS_DISCORDANT]
                    && !(target->info.flags & MONST_DIES_IF_NEGATED)) {
                    // Dispel discord from allies unless it would destroy them.
                    return true;
                }
            } else if (monstersAreTeammates(caster, target)) {
                if (target == &player && rogue.armor && (rogue.armor->flags & ITEM_RUNIC) && (rogue.armor->flags & ITEM_RUNIC_IDENTIFIED)
                    && rogue.armor->enchant2 == A_REFLECTION && netEnchant(rogue.armor) > 0) {
                    // Allies shouldn't cast negation on the player if she's knowingly wearing armor of reflection.
                    // Too much risk of negating themselves in the process.
                    return false;
                }
                if (target->info.flags & MONST_DIES_IF_NEGATED) {
                    // Never cast negation if it would destroy an allied creature.
                    return false;
                }
                if (target->status[STATUS_ENTRANCED]
                    && caster->creatureState != MONSTER_ALLY) {
                    // Non-allied monsters will dispel entrancement on their own kind.
                    return true;
                }
                if (target->status[STATUS_MAGICAL_FEAR]) {
                    // Dispel magical fear.
                    return true;
                }
            }
            return false; // Don't cast negation unless there's a good reason.
            break;
        case BE_SLOW:
            if (target->status[STATUS_SLOWED]) {
                return false;
            }
            break;
        case BE_HASTE:
            if (target->status[STATUS_HASTED]) {
                return false;
            }
            if (!targetEligibleForCombatBuff(caster, target)) {
                return false;
            }
            break;
        case BE_SHIELDING:
            if (target->status[STATUS_SHIELDED]) {
                return false;
            }
            if (!targetEligibleForCombatBuff(caster, target)) {
                return false;
            }
            break;
        case BE_HEALING:
            if (target->currentHP >= target->info.maxHP) {
                // Don't heal a creature already at full health.
                return false;
            }
            break;
        case BE_TUNNELING:
        case BE_OBSTRUCTION:
            // Monsters will never cast these.
            return false;
            break;
        default:
            break;
    }
    return true;
}

static void monsterCastSpell(creature *caster, creature *target, enum boltType boltIndex) {
    bolt theBolt;
    char buf[200], monstName[100];

    if (canDirectlySeeMonster(caster)) {
        monsterName(monstName, caster, true);
        sprintf(buf, "%s %s", monstName, boltCatalog[boltIndex].description);
        resolvePronounEscapes(buf, caster);
        combatMessage(buf, 0);
    }

    theBolt = boltCatalog[boltIndex];
    pos originLoc = caster->loc;
    pos targetLoc = target->loc;
    zap(originLoc, targetLoc, &theBolt, false, false);

    if (player.currentHP <= 0) {
        gameOver(monsterCatalog[caster->info.monsterID].monsterName, false);
    }
}

// returns whether the monster cast a bolt.
static boolean monstUseBolt(creature *monst) {
    short i;

    if (!monst->info.bolts[0]) {
        return false; // Don't waste time with monsters that can't cast anything.
    }

    boolean handledPlayer = false;
    for (creatureIterator it = iterateCreatures(monsters); !handledPlayer || hasNextCreature(it);) {
        creature *target = !handledPlayer ? &player : nextCreature(&it);
        handledPlayer = true;
        if (generallyValidBoltTarget(monst, target)) {
            for (i = 0; monst->info.bolts[i]; i++) {
                if (boltCatalog[monst->info.bolts[i]].boltEffect == BE_BLINKING) {
                    continue; // Blinking is handled elsewhere.
                }
                if (specificallyValidBoltTarget(monst, target, monst->info.bolts[i])) {
                    if ((monst->info.flags & MONST_ALWAYS_USE_ABILITY)
                        || rand_percent(30)) {

                        monsterCastSpell(monst, target, monst->info.bolts[i]);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// returns whether the monster did something (and therefore ended its turn)
static boolean monstUseMagic(creature *monst) {
    if (monsterSummons(monst, (monst->info.flags & MONST_ALWAYS_USE_ABILITY))) {
        return true;
    } else if (monstUseBolt(monst)) {
        return true;
    }
    return false;
}

static boolean isLocalScentMaximum(pos loc) {
    const short baselineScent = scentMap[loc.x][loc.y];
    for (enum directions dir=0; dir< DIRECTION_COUNT; dir++) {
        pos newLoc = posNeighborInDirection(loc, dir);
        if (isPosInMap(newLoc)
            && (scentMap[newLoc.x][newLoc.y] > baselineScent)
            && !cellHasTerrainFlag(newLoc, T_OBSTRUCTS_PASSABILITY)
            && !diagonalBlocked(loc.x, loc.y, newLoc.x, newLoc.y, false)) {

            return false;
        }
    }
    return true;
}

// Returns the direction the player's scent points to from a given cell. Returns -1 if the nose comes up blank.
static enum directions scentDirection(creature *monst) {
    short newX, newY, x, y, newestX, newestY;
    enum directions bestDirection = NO_DIRECTION, dir, dir2;
    unsigned short bestNearbyScent = 0;
    boolean canTryAgain = true;
    creature *otherMonst;

    x = monst->loc.x;
    y = monst->loc.y;

    for (;;) {

        for (dir=0; dir< DIRECTION_COUNT; dir++) {
            newX = x + nbDirs[dir][0];
            newY = y + nbDirs[dir][1];
            otherMonst = monsterAtLoc((pos){ newX, newY });
            if (coordinatesAreInMap(newX, newY)
                && (scentMap[newX][newY] > bestNearbyScent)
                && (!(pmap[newX][newY].flags & HAS_MONSTER) || (otherMonst && canPass(monst, otherMonst)))
                && !cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)
                && !diagonalBlocked(x, y, newX, newY, false)
                && !monsterAvoids(monst, (pos){newX, newY})) {

                bestNearbyScent = scentMap[newX][newY];
                bestDirection = dir;
            }
        }

        if (bestDirection >= 0 && bestNearbyScent > scentMap[x][y]) {
            return bestDirection;
        }

        if (canTryAgain) {
            // Okay, the monster may be stuck in some irritating diagonal.
            // If so, we can diffuse the scent into the offending kink and solve the problem.
            // There's a possibility he's stuck for some other reason, though, so we'll only
            // try once per his move -- hence the failsafe.
            canTryAgain = false;
            for (dir=0; dir<4; dir++) {
                newX = x + nbDirs[dir][0];
                newY = y + nbDirs[dir][1];
                for (dir2=0; dir2<4; dir2++) {
                    newestX = newX + nbDirs[dir2][0];
                    newestY = newY + nbDirs[dir2][1];
                    if (coordinatesAreInMap(newX, newY) && coordinatesAreInMap(newestX, newestY)) {
                        scentMap[newX][newY] = max(scentMap[newX][newY], scentMap[newestX][newestY] - 1);
                    }
                }
            }
        } else {
            return NO_DIRECTION; // failure!
        }
    }
}

// returns true if the resurrection was successful.
boolean resurrectAlly(const pos loc) {
    creatureIterator allyIterator = iterateCreatures(&purgatory);

    // Prefer most empowered ally.  In case of tie, prefer ally with greatest monsterID (thus
    // preferring allies found deeper in the dungeon over ones found higher up and preferring
    // legendary allies over everyone else).
    creature *monToCheck, *monToRaise = NULL;
    while (monToCheck = nextCreature(&allyIterator)) {
        if (monToRaise == NULL
            || monToCheck->totalPowerCount > monToRaise->totalPowerCount
            || (monToCheck->totalPowerCount == monToRaise->totalPowerCount
                && monToCheck->info.monsterID > monToRaise->info.monsterID)) {

            monToRaise = monToCheck;
        }
    }

    if (monToRaise) {
        // Remove from purgatory and insert into the mortal plane.
        removeCreature(&purgatory, monToRaise);
        prependCreature(monsters, monToRaise);

        getQualifyingPathLocNear(&monToRaise->loc.x, &monToRaise->loc.y, loc.x, loc.y, true,
                                 (T_PATHING_BLOCKER | T_HARMFUL_TERRAIN), 0,
                                 0, (HAS_PLAYER | HAS_MONSTER), false);
        pmapAt(monToRaise->loc)->flags |= HAS_MONSTER;

        // Restore health etc.
        monToRaise->bookkeepingFlags &= ~(MB_IS_DYING | MB_ADMINISTRATIVE_DEATH | MB_HAS_DIED | MB_IS_FALLING);
        if (!(monToRaise->info.flags & MONST_FIERY)
            && monToRaise->status[STATUS_BURNING]) {

            monToRaise->status[STATUS_BURNING] = 0;
        }
        monToRaise->status[STATUS_DISCORDANT] = 0;
        heal(monToRaise, 100, true);

        // put humpty dumpty back together again. special handling for phoenix egg, phylactery, vampire
        if (monsterCatalog[monToRaise->info.monsterID].abilityFlags & MA_ENTER_SUMMONS) {
            monToRaise->info = monsterCatalog[monToRaise->info.monsterID];
            initializeStatus(monToRaise);
            monToRaise->wasNegated = false;
        }

        return true;
    } else {
        return false;
    }
}

void unAlly(creature *monst) {
    if (monst->creatureState == MONSTER_ALLY) {
        monst->creatureState = MONSTER_TRACKING_SCENT;
        monst->bookkeepingFlags &= ~(MB_FOLLOWER | MB_TELEPATHICALLY_REVEALED);
        monst->leader = NULL;
    }
}

boolean monsterFleesFrom(creature *monst, creature *defender) {
    const short x = monst->loc.x;
    const short y = monst->loc.y;

    if (!monsterWillAttackTarget(defender, monst)) {
        return false;
    }

    if (distanceBetween((pos){x, y}, defender->loc) >= 4) {
        return false;
    }

    if ((defender->info.flags & (MONST_IMMUNE_TO_WEAPONS | MONST_INVULNERABLE))
        && !(defender->info.flags & MONST_IMMOBILE)) {
        // Don't charge if the monster is damage-immune and is NOT immobile;
        // i.e., keep distance from revenants and stone guardians but not mirror totems.
        return true;
    }

    if (monst->creatureState == MONSTER_ALLY && !monst->status[STATUS_DISCORDANT]
            && (defender->bookkeepingFlags & MB_MARKED_FOR_SACRIFICE)) {
        // Willing allies shouldn't charge sacrifice targets.
        return true;
    }

    if ((monst->info.flags & MONST_MAINTAINS_DISTANCE)
        || (defender->info.abilityFlags & MA_KAMIKAZE)) {

        // Don't charge if you maintain distance or if it's a kamikaze monster.
        return true;
    }

    if (monst->info.abilityFlags & MA_POISONS
        && defender->status[STATUS_POISONED] * defender->poisonAmount > defender->currentHP) {

        return true;
    }

    return false;
}

static boolean allyFlees(creature *ally, creature *closestEnemy) {
    const short x = ally->loc.x;
    const short y = ally->loc.y;

    if (!closestEnemy) {
        return false; // No one to flee from.
    }

    if (ally->info.maxHP <= 1 || (ally->status[STATUS_LIFESPAN_REMAINING]) > 0) { // Spectral blades and timed allies should never flee.
        return false;
    }

    if (distanceBetween((pos){x, y}, closestEnemy->loc) < 10
        && (100 * ally->currentHP / ally->info.maxHP <= 33)
        && ally->info.turnsBetweenRegen > 0
        && !ally->carriedMonster
        && ((ally->info.flags & MONST_FLEES_NEAR_DEATH) || (100 * ally->currentHP / ally->info.maxHP * 2 < 100 * player.currentHP / player.info.maxHP))) {
        // Flee if you're within 10 spaces, your HP is under 1/3, you're not a phoenix or lich or vampire in bat form,
        // and you either flee near death or your health fraction is less than half of the player's.
        return true;
    }

    // so do allies that keep their distance or while in the presence of damage-immune or kamikaze enemies
    if (monsterFleesFrom(ally, closestEnemy)) {
        // Flee if you're within 3 spaces and you either flee near death or the closest enemy is a bloat, revenant or guardian.
        return true;
    }

    return false;
}

static void monsterMillAbout(creature *monst, short movementChance) {
    enum directions dir;

    const short x = monst->loc.x;
    const short y = monst->loc.y;

    if (rand_percent(movementChance)) {
        dir = randValidDirectionFrom(monst, x, y, true);
        if (dir != -1) {
            pos targetLoc = {
                x + nbDirs[dir][0],
                y + nbDirs[dir][1]
            };
            moveMonsterPassivelyTowards(monst, targetLoc, false);
        }
    }
}

/// @brief Handles the given allied monster's turn under normal circumstances
/// e.g. not discordant, fleeing, paralyzed or entranced
/// @param monst the allied monster
static void moveAlly(creature *monst) {
    creature *closestMonster = NULL;
    short i, j, x, y, dir, shortestDistance, leashLength;
    short **enemyMap, **costMap;
    char buf[DCOLS], monstName[DCOLS];

    x = monst->loc.x;
    y = monst->loc.y;

    pos targetLoc = INVALID_POS;

    if (!(monst->leader)) {
        monst->leader = &player;
        monst->bookkeepingFlags |= MB_FOLLOWER;
    }

    // If we're standing in harmful terrain and there is a way to escape it, spend this turn escaping it.
    if (cellHasTerrainFlag((pos){ x, y }, (T_HARMFUL_TERRAIN & ~(T_IS_FIRE | T_CAUSES_DAMAGE | T_CAUSES_PARALYSIS | T_CAUSES_CONFUSION)))
        || (cellHasTerrainFlag((pos){ x, y }, T_IS_FIRE) && !monst->status[STATUS_IMMUNE_TO_FIRE])
        || (cellHasTerrainFlag((pos){ x, y }, T_CAUSES_DAMAGE | T_CAUSES_PARALYSIS | T_CAUSES_CONFUSION) && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE)))) {

        if (!rogue.updatedMapToSafeTerrainThisTurn) {
            updateSafeTerrainMap();
        }

        if (monsterBlinkToPreferenceMap(monst, rogue.mapToSafeTerrain, false)) {
            monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
            return;
        }

        dir = nextStep(rogue.mapToSafeTerrain, x, y, monst, true);
        if (dir != -1) {
            targetLoc = (pos){
                x + nbDirs[dir][0],
                y + nbDirs[dir][1]
            };
            if (moveMonsterPassivelyTowards(monst, targetLoc, false)) {
                return;
            }
        }
    }

    // Look around for enemies; shortestDistance will be the distance to the nearest.
    shortestDistance = max(DROWS, DCOLS);
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *target = nextCreature(&it);
        if (target != monst
            && (!(target->bookkeepingFlags & MB_SUBMERGED) || (monst->bookkeepingFlags & MB_SUBMERGED))
            && monsterWillAttackTarget(monst, target)
            && distanceBetween((pos){x, y}, target->loc) < shortestDistance
            && traversiblePathBetween(monst, target->loc.x, target->loc.y)
            && (!cellHasTerrainFlag(target->loc, T_OBSTRUCTS_PASSABILITY) || (target->info.flags & MONST_ATTACKABLE_THRU_WALLS))
            && (!target->status[STATUS_INVISIBLE] || rand_percent(33))) {

            shortestDistance = distanceBetween((pos){x, y}, target->loc);
            closestMonster = target;
        }
    }

    // Weak allies in the presence of enemies seek safety;
    if (allyFlees(monst, closestMonster)) {
        if (monsterHasBoltEffect(monst, BE_BLINKING)
            && ((monst->info.flags & MONST_ALWAYS_USE_ABILITY) || rand_percent(30))
            && monsterBlinkToSafety(monst)) {

            return;
        }
        if (monsterSummons(monst, (monst->info.flags & MONST_ALWAYS_USE_ABILITY))) {
            return;
        }
        if (!rogue.updatedAllySafetyMapThisTurn) {
            updateAllySafetyMap();
        }
        dir = nextStep(allySafetyMap, monst->loc.x, monst->loc.y, monst, true);
        if (dir != -1) {
            targetLoc = (pos){
                x + nbDirs[dir][0],
                y + nbDirs[dir][1]
            };
        }
        if (dir == -1
            || (allySafetyMap[targetLoc.x][targetLoc.y] >= allySafetyMap[x][y])
            || (!moveMonster(monst, nbDirs[dir][0], nbDirs[dir][1]) && !moveMonsterPassivelyTowards(monst, targetLoc, true))) {
            // ally can't flee; continue below
        } else {
            return;
        }
    }

    // Magic users sometimes cast spells.
    if (monstUseMagic(monst)) { // if he actually cast a spell
        monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
        return;
    }

    if (monst->bookkeepingFlags & MB_SEIZED) {
        leashLength = max(DCOLS, DROWS); // Ally will never be prevented from attacking while seized.
    } else if (rogue.justRested || rogue.justSearched) {
        leashLength = 10;
    } else {
        leashLength = 4;
    }
    if (shortestDistance == 1) {
        if (closestMonster->movementSpeed < monst->movementSpeed
            && !(closestMonster->info.flags & (MONST_FLITS | MONST_IMMOBILE))
            && closestMonster->creatureState == MONSTER_TRACKING_SCENT) {
            // Never try to flee from combat with a faster enemy.
            leashLength = max(DCOLS, DROWS);
        } else {
            leashLength++; // If the ally is adjacent to a monster at the end of its leash, it shouldn't be prevented from attacking.
        }
    }

    if (closestMonster
        && (distanceBetween((pos){x, y}, player.loc) < leashLength || (monst->bookkeepingFlags & MB_DOES_NOT_TRACK_LEADER))
        && !(monst->info.flags & MONST_MAINTAINS_DISTANCE)
        && !attackWouldBeFutile(monst, closestMonster)) {

        // Blink toward an enemy?
        if (monsterHasBoltEffect(monst, BE_BLINKING)
            && ((monst->info.flags & MONST_ALWAYS_USE_ABILITY) || rand_percent(30))) {

            enemyMap = allocGrid();
            costMap = allocGrid();

            for (i=0; i<DCOLS; i++) {
                for (j=0; j<DROWS; j++) {
                    if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)) {
                        costMap[i][j] = cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
                        enemyMap[i][j] = 0; // safeguard against OOS
                    } else if (monsterAvoids(monst, (pos){i, j})) {
                        costMap[i][j] = PDS_FORBIDDEN;
                        enemyMap[i][j] = 0; // safeguard against OOS
                    } else {
                        costMap[i][j] = 1;
                        enemyMap[i][j] = 10000;
                    }
                }
            }

            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *target = nextCreature(&it);
                if (target != monst
                    && (!(target->bookkeepingFlags & MB_SUBMERGED) || (monst->bookkeepingFlags & MB_SUBMERGED))
                    && monsterWillAttackTarget(monst, target)
                    && distanceBetween((pos){x, y}, target->loc) < shortestDistance
                    && traversiblePathBetween(monst, target->loc.x, target->loc.y)
                    && (!monsterAvoids(monst, target->loc) || (target->info.flags & MONST_ATTACKABLE_THRU_WALLS))
                    && (!target->status[STATUS_INVISIBLE] || ((monst->info.flags & MONST_ALWAYS_USE_ABILITY) || rand_percent(33)))) {

                    enemyMap[target->loc.x][target->loc.y] = 0;
                    costMap[target->loc.x][target->loc.y] = 1;
                }
            }

            dijkstraScan(enemyMap, costMap, true);
            freeGrid(costMap);

            if (monsterBlinkToPreferenceMap(monst, enemyMap, false)) {
                monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
                freeGrid(enemyMap);
                return;
            }
            freeGrid(enemyMap);
        }

        targetLoc = closestMonster->loc;
        moveMonsterPassivelyTowards(monst, targetLoc, true);
    } else if (isPosInMap(monst->targetCorpseLoc)
               && !monst->status[STATUS_POISONED]
               && (!monst->status[STATUS_BURNING] || monst->status[STATUS_IMMUNE_TO_FIRE])) { // Going to start eating a corpse.
        moveMonsterPassivelyTowards(monst, monst->targetCorpseLoc, false);
        if (posEq(monst->loc, monst->targetCorpseLoc)
            && !(monst->bookkeepingFlags & MB_ABSORBING)) {
            if (canSeeMonster(monst)) {
                monsterName(monstName, monst, true);
                sprintf(buf, "%s begins %s the fallen %s.", monstName, monsterText[monst->info.monsterID].absorbing, monst->targetCorpseName);
                messageWithColor(buf, &goodMessageColor, 0);
            }
            monst->corpseAbsorptionCounter = 20;
            monst->bookkeepingFlags |= MB_ABSORBING;
        }
    } else if ((monst->bookkeepingFlags & MB_DOES_NOT_TRACK_LEADER)
               || (distanceBetween((pos){x, y}, player.loc) < 3 && (pmap[x][y].flags & IN_FIELD_OF_VIEW))) {

        monst->bookkeepingFlags &= ~MB_GIVEN_UP_ON_SCENT;
        monsterMillAbout(monst, 30);
    } else {
        if (!(monst->bookkeepingFlags & MB_GIVEN_UP_ON_SCENT)
            && distanceBetween((pos){x, y}, player.loc) > 10
            && monsterBlinkToPreferenceMap(monst, scentMap, true)) {

            monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
            return;
        }
        dir = scentDirection(monst);
        if (dir == -1 || (monst->bookkeepingFlags & MB_GIVEN_UP_ON_SCENT)) {
            monst->bookkeepingFlags |= MB_GIVEN_UP_ON_SCENT;
            pathTowardCreature(monst, monst->leader);
        } else {
            targetLoc = (pos) {
                x + nbDirs[dir][0],
                y + nbDirs[dir][1]
            };
            moveMonsterPassivelyTowards(monst, targetLoc, false);
        }
    }
}

// Returns whether to abort the turn.
static boolean updateMonsterCorpseAbsorption(creature *monst) {
    short i;
    char buf[COLS], buf2[COLS];

    if (posEq(monst->loc, monst->targetCorpseLoc)
        && (monst->bookkeepingFlags & MB_ABSORBING)) {

        if (--monst->corpseAbsorptionCounter <= 0) {
            monst->targetCorpseLoc = INVALID_POS;
            if (monst->absorptionBolt != BOLT_NONE) {
                for (i=0; monst->info.bolts[i] != BOLT_NONE; i++);
                monst->info.bolts[i] = monst->absorptionBolt;
            } else if (monst->absorbBehavior) {
                monst->info.flags |= monst->absorptionFlags;
            } else {
                monst->info.abilityFlags |= monst->absorptionFlags;
            }
            monst->newPowerCount--;
            monst->bookkeepingFlags &= ~MB_ABSORBING;

            if (monst->info.flags & MONST_FIERY) {
                monst->status[STATUS_BURNING] = monst->maxStatus[STATUS_BURNING] = 1000; // won't decrease
            }
            if (monst->info.flags & MONST_FLIES) {
                monst->status[STATUS_LEVITATING] = monst->maxStatus[STATUS_LEVITATING] = 1000; // won't decrease
                monst->info.flags &= ~(MONST_RESTRICTED_TO_LIQUID | MONST_SUBMERGES);
                monst->bookkeepingFlags &= ~(MB_SUBMERGED);
            }
            if (monst->info.flags & MONST_IMMUNE_TO_FIRE) {
                monst->status[STATUS_IMMUNE_TO_FIRE] = monst->maxStatus[STATUS_IMMUNE_TO_FIRE] = 1000; // won't decrease
            }
            if (monst->info.flags & MONST_INVISIBLE) {
                monst->status[STATUS_INVISIBLE] = monst->maxStatus[STATUS_INVISIBLE] = 1000; // won't decrease
            }
            if (canSeeMonster(monst)) {
                monsterName(buf2, monst, true);
                sprintf(buf, "%s finished %s the %s.", buf2, monsterText[monst->info.monsterID].absorbing, monst->targetCorpseName);
                messageWithColor(buf, &goodMessageColor, 0);
                if (monst->absorptionBolt != BOLT_NONE) {
                    sprintf(buf, "%s %s!", buf2, boltCatalog[monst->absorptionBolt].abilityDescription);
                } else if (monst->absorbBehavior) {
                    sprintf(buf, "%s now %s!", buf2, monsterBehaviorCatalog[unflag(monst->absorptionFlags)].description);
                } else {
                    sprintf(buf, "%s now %s!", buf2, monsterAbilityCatalog[unflag(monst->absorptionFlags)].description);
                }
                resolvePronounEscapes(buf, monst);
                messageWithColor(buf, &advancementMessageColor, 0);
            }
            monst->absorptionFlags = 0;
            monst->absorptionBolt = BOLT_NONE;
        }
        monst->ticksUntilTurn = 100;
        return true;
    } else if (--monst->corpseAbsorptionCounter <= 0) {
        monst->targetCorpseLoc = INVALID_POS; // lost its chance
        monst->bookkeepingFlags &= ~MB_ABSORBING;
        monst->absorptionFlags = 0;
        monst->absorptionBolt = BOLT_NONE;
    } else if (monst->bookkeepingFlags & MB_ABSORBING) {
        monst->bookkeepingFlags &= ~MB_ABSORBING; // absorbing but not on the corpse
        if (monst->corpseAbsorptionCounter <= 15) {
            monst->targetCorpseLoc = INVALID_POS; // lost its chance
            monst->absorptionFlags = 0;
            monst->absorptionBolt = BOLT_NONE;
        }
    }
    return false;
}

void monstersTurn(creature *monst) {
    short x, y, dir, shortestDistance;
    boolean alreadyAtBestScent;
    creature *closestMonster;

    pos targetLoc;

    monst->turnsSpentStationary++;

    if (monst->corpseAbsorptionCounter >= 0 && updateMonsterCorpseAbsorption(monst)) {
        return;
    }

    if (monst->info.DFChance
        && (monst->info.flags & MONST_GETS_TURN_ON_ACTIVATION)
        && rand_percent(monst->info.DFChance)) {

        spawnDungeonFeature(monst->loc.x, monst->loc.y, &dungeonFeatureCatalog[monst->info.DFType], true, false);
    }

    applyInstantTileEffectsToCreature(monst); // Paralysis, confusion etc. take effect before the monster can move.

    // if the monster is paralyzed, entranced or chained, this is where its turn ends.
    if (monst->status[STATUS_PARALYZED] || monst->status[STATUS_ENTRANCED] || (monst->bookkeepingFlags & MB_CAPTIVE)) {
        monst->ticksUntilTurn = monst->movementSpeed;
        if ((monst->bookkeepingFlags & MB_CAPTIVE) && monst->carriedItem) {
            makeMonsterDropItem(monst);
        }
        return;
    }

    if (monst->bookkeepingFlags & MB_IS_DYING) {
        return;
    }

    monst->ticksUntilTurn = monst->movementSpeed / 3; // will be later overwritten by movement or attack

    x = monst->loc.x;
    y = monst->loc.y;

    // Sleepers can awaken, but it takes a whole turn.
    if (monst->creatureState == MONSTER_SLEEPING) {
        monst->ticksUntilTurn = monst->movementSpeed;
        updateMonsterState(monst);
        return;
    }

    // Update creature state if appropriate.
    updateMonsterState(monst);

    if (monst->creatureState == MONSTER_SLEEPING) {
        monst->ticksUntilTurn = monst->movementSpeed;
        return;
    }

    // and move the monster.

    // immobile monsters can only use special abilities:
    if (monst->info.flags & MONST_IMMOBILE) {
        if (monstUseMagic(monst)) { // if he actually cast a spell
            monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
            return;
        }
        monst->ticksUntilTurn = monst->attackSpeed;
        return;
    }

    // discordant monsters
    if (monst->status[STATUS_DISCORDANT] && monst->creatureState != MONSTER_FLEEING) {
        shortestDistance = max(DROWS, DCOLS);
        closestMonster = NULL;
        boolean handledPlayer = false;
        for (creatureIterator it = iterateCreatures(monsters); !handledPlayer || hasNextCreature(it);) {
            creature *target = !handledPlayer ? &player : nextCreature(&it);
            handledPlayer = true;
            if (target != monst
                && (!(target->bookkeepingFlags & MB_SUBMERGED) || (monst->bookkeepingFlags & MB_SUBMERGED))
                && monsterWillAttackTarget(monst, target)
                && distanceBetween((pos){x, y}, target->loc) < shortestDistance
                && traversiblePathBetween(monst, target->loc.x, target->loc.y)
                && (!monsterAvoids(monst, target->loc) || (target->info.flags & MONST_ATTACKABLE_THRU_WALLS))
                && (!target->status[STATUS_INVISIBLE] || rand_percent(33))) {

                shortestDistance = distanceBetween((pos){x, y}, target->loc);
                closestMonster = target;
            }
        }
        if (closestMonster && monstUseMagic(monst)) {
            monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
            return;
        }
        if (closestMonster && !(monst->info.flags & MONST_MAINTAINS_DISTANCE)) {
            targetLoc = closestMonster->loc;
            if (moveMonsterPassivelyTowards(monst, targetLoc, monst->creatureState == MONSTER_ALLY)) {
                return;
            }
        }
    }

    // hunting
    if ((monst->creatureState == MONSTER_TRACKING_SCENT
        || (monst->creatureState == MONSTER_ALLY && monst->status[STATUS_DISCORDANT]))
        // eels don't charge if you're not in the water
        && (!(monst->info.flags & MONST_RESTRICTED_TO_LIQUID) || cellHasTMFlag(player.loc, TM_ALLOWS_SUBMERGING))) {

        // magic users sometimes cast spells
        if (monstUseMagic(monst)
            || (monsterHasBoltEffect(monst, BE_BLINKING)
                && ((monst->info.flags & MONST_ALWAYS_USE_ABILITY) || rand_percent(30))
                && monsterBlinkToPreferenceMap(monst, scentMap, true))) { // if he actually cast a spell

                monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
                return;
            }

        // if the monster is adjacent to an ally and not adjacent to the player, attack the ally
        if (distanceBetween((pos){x, y}, player.loc) > 1
            || diagonalBlocked(x, y, player.loc.x, player.loc.y, false)) {
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *ally = nextCreature(&it);
                if (monsterWillAttackTarget(monst, ally)
                    && distanceBetween((pos){x, y}, ally->loc) == 1
                    && (!ally->status[STATUS_INVISIBLE] || rand_percent(33))) {

                    targetLoc = ally->loc;
                    if (moveMonsterPassivelyTowards(monst, targetLoc, true)) { // attack
                        return;
                    }
                }
            }
        }

        if ((monst->status[STATUS_LEVITATING] || (monst->info.flags & MONST_RESTRICTED_TO_LIQUID) || (monst->bookkeepingFlags & MB_SUBMERGED)
             || ((monst->info.flags & (MONST_IMMUNE_TO_WEBS | MONST_INVULNERABLE) && monsterCanShootWebs(monst))))
            && pmap[x][y].flags & IN_FIELD_OF_VIEW) {
            moveMonsterPassivelyTowards(monst, player.loc, true); // attack
            return;
        }
        if ((monst->info.flags & MONST_ALWAYS_HUNTING)
            && (monst->bookkeepingFlags & MB_GIVEN_UP_ON_SCENT)) {

            pathTowardCreature(monst, &player);
            return;
        }

        dir = scentDirection(monst);
        if (dir == NO_DIRECTION) {
            alreadyAtBestScent = isLocalScentMaximum((pos){ x, y });
            if (alreadyAtBestScent && monst->creatureState != MONSTER_ALLY && !(pmap[x][y].flags & IN_FIELD_OF_VIEW)) {
                if (monst->info.flags & MONST_ALWAYS_HUNTING) {
                    pathTowardCreature(monst, &player);
                    monst->bookkeepingFlags |= MB_GIVEN_UP_ON_SCENT;
                    return;
                }
                monst->creatureState = MONSTER_WANDERING;
                // If we're out of the player's FOV and the scent map is a dead end,
                // wander over to near where we last saw the player.
                wanderToward(monst, monst->lastSeenPlayerAt);
            }
        } else {
            moveMonster(monst, nbDirs[dir][0], nbDirs[dir][1]);
        }
    } else if (monst->creatureState == MONSTER_FLEEING) {
        // fleeing
        if (monsterHasBoltEffect(monst, BE_BLINKING)
            && ((monst->info.flags & MONST_ALWAYS_USE_ABILITY) || rand_percent(30))
            && monsterBlinkToSafety(monst)) {

            return;
        }

        if (monsterSummons(monst, (monst->info.flags & MONST_ALWAYS_USE_ABILITY))) {
            return;
        }

        dir = nextStep(getSafetyMap(monst), monst->loc.x, monst->loc.y, NULL, true);
        if (dir != -1) {
            targetLoc = (pos){
                x + nbDirs[dir][0],
                y + nbDirs[dir][1]
            };
        }
        if (dir == -1 || (!moveMonster(monst, nbDirs[dir][0], nbDirs[dir][1]) && !moveMonsterPassivelyTowards(monst, targetLoc, true))) {
            boolean handledPlayer = false;
            for (creatureIterator it = iterateCreatures(monsters); !handledPlayer || hasNextCreature(it);) {
                creature *ally = !handledPlayer ? &player : nextCreature(&it);
                handledPlayer = true;
                if (!monst->status[STATUS_MAGICAL_FEAR] // Fearful monsters will never attack.
                    && monsterWillAttackTarget(monst, ally)
                    && distanceBetween((pos){x, y}, ally->loc) <= 1) {

                    moveMonster(monst, ally->loc.x - x, ally->loc.y - y); // attack the player if cornered
                    return;
                }
            }
        }
        return;
    } else if (monst->creatureState == MONSTER_WANDERING
               // eels wander if you're not in water
               || ((monst->info.flags & MONST_RESTRICTED_TO_LIQUID) && !cellHasTMFlag(player.loc, TM_ALLOWS_SUBMERGING))) {

        // if we're standing in harmful terrain and there is a way to escape it, spend this turn escaping it.
        if (cellHasTerrainFlag((pos){ x, y }, (T_HARMFUL_TERRAIN & ~T_IS_FIRE))
            || (cellHasTerrainFlag((pos){ x, y }, T_IS_FIRE) && !monst->status[STATUS_IMMUNE_TO_FIRE] && !(monst->info.flags & MONST_INVULNERABLE))) {
            if (!rogue.updatedMapToSafeTerrainThisTurn) {
                updateSafeTerrainMap();
            }

            if (monsterBlinkToPreferenceMap(monst, rogue.mapToSafeTerrain, false)) {
                monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
                return;
            }

            dir = nextStep(rogue.mapToSafeTerrain, x, y, monst, true);
            if (dir != -1) {
                targetLoc = (pos) {
                    x + nbDirs[dir][0],
                    y + nbDirs[dir][1]
                };
                if (moveMonsterPassivelyTowards(monst, targetLoc, true)) {
                    return;
                }
            }
        }

        // if a captive leader is captive, regenerative and healthy enough to withstand an attack,
        // and we're not poisonous, then approach or attack him.
        if ((monst->bookkeepingFlags & MB_FOLLOWER)
            && (monst->leader->bookkeepingFlags & MB_CAPTIVE)
            && monst->leader->currentHP > (int) (monst->info.damage.upperBound * monsterDamageAdjustmentAmount(monst) / FP_FACTOR)
            && monst->leader->info.turnsBetweenRegen > 0
            && !(monst->info.abilityFlags & MA_POISONS)
            && !diagonalBlocked(monst->loc.x, monst->loc.y, monst->leader->loc.x, monst->leader->loc.y, false)) {

            if (distanceBetween(monst->loc, monst->leader->loc) == 1) {
                // Attack if adjacent.
                monst->ticksUntilTurn = monst->attackSpeed;
                attack(monst, monst->leader, false);
                return;
            } else {
                // Otherwise, approach.
                pathTowardCreature(monst, monst->leader);
                return;
            }
        }

        // if the monster is adjacent to an ally and not fleeing, attack the ally
        if (monst->creatureState == MONSTER_WANDERING) {
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *ally = nextCreature(&it);
                if (monsterWillAttackTarget(monst, ally)
                    && distanceBetween((pos){x, y}, ally->loc) == 1
                    && (!ally->status[STATUS_INVISIBLE] || rand_percent(33))) {

                    targetLoc = ally->loc;
                    if (moveMonsterPassivelyTowards(monst, targetLoc, true)) {
                        return;
                    }
                }
            }
        }

        // if you're a follower, don't get separated from the pack
        if (monst->bookkeepingFlags & MB_FOLLOWER) {
            if (distanceBetween((pos){x, y}, monst->leader->loc) > 2) {
                pathTowardCreature(monst, monst->leader);
            } else if (monst->leader->info.flags & MONST_IMMOBILE) {
                monsterMillAbout(monst, 100); // Worshipers will pace frenetically.
            } else if (monst->leader->bookkeepingFlags & MB_CAPTIVE) {
                monsterMillAbout(monst, 10); // Captors are languid.
            } else {
                monsterMillAbout(monst, 30); // Other followers mill about like your allies do.
            }
        } else {
            // Step toward the chosen waypoint.
            dir = NO_DIRECTION;
            if (isValidWanderDestination(monst, monst->targetWaypointIndex)) {
                dir = nextStep(rogue.wpDistance[monst->targetWaypointIndex], monst->loc.x, monst->loc.y, monst, false);
            }
            // If there's no path forward, call that waypoint finished and pick a new one.
            if (!isValidWanderDestination(monst, monst->targetWaypointIndex)
                || dir == NO_DIRECTION) {

                chooseNewWanderDestination(monst);
                if (isValidWanderDestination(monst, monst->targetWaypointIndex)) {
                    dir = nextStep(rogue.wpDistance[monst->targetWaypointIndex], monst->loc.x, monst->loc.y, monst, false);
                }
            }
            // If there's still no path forward, step randomly as though flitting.
            // (This is how eels wander in deep water.)
            if (dir == NO_DIRECTION) {
                dir = randValidDirectionFrom(monst, x, y, true);
            }
            if (dir != NO_DIRECTION) {
                targetLoc = (pos) {
                    x + nbDirs[dir][0],
                    y + nbDirs[dir][1]
                };
                if (moveMonsterPassivelyTowards(monst, targetLoc, true)) {
                    return;
                }
            }
        }
    } else if (monst->creatureState == MONSTER_ALLY) {
        moveAlly(monst);
    }
}

boolean canPass(creature *mover, creature *blocker) {

    if (blocker == &player) {
        return false;
    }

    if (blocker->status[STATUS_CONFUSED]
        || blocker->status[STATUS_STUCK]
        || blocker->status[STATUS_PARALYZED]
        || blocker->status[STATUS_ENTRANCED]
        || mover->status[STATUS_ENTRANCED]) {

        return false;
    }

    if ((blocker->bookkeepingFlags & (MB_CAPTIVE | MB_ABSORBING))
        || (blocker->info.flags & MONST_IMMOBILE)) {
        return false;
    }

    if (monstersAreEnemies(mover, blocker)) {
        return false;
    }

    if (blocker->leader == mover) {
        return true;
    }

    if (mover->leader == blocker) {
        return false;
    }

    return (monstersAreTeammates(mover, blocker)
            && blocker->currentHP < mover->currentHP);
}

boolean isPassableOrSecretDoor(pos loc) {
    return (!cellHasTerrainFlag(loc, T_OBSTRUCTS_PASSABILITY)
            || (cellHasTMFlag(loc, TM_IS_SECRET) && !(discoveredTerrainFlagsAtLoc(loc) & T_OBSTRUCTS_PASSABILITY)));
}

boolean knownToPlayerAsPassableOrSecretDoor(pos loc) {
    unsigned long tFlags, TMFlags;
    getLocationFlags(loc.x, loc.y, &tFlags, &TMFlags, NULL, true);
    return (!(tFlags & T_OBSTRUCTS_PASSABILITY)
            || ((TMFlags & TM_IS_SECRET) && !(discoveredTerrainFlagsAtLoc(loc) & T_OBSTRUCTS_PASSABILITY)));
}

void setMonsterLocation(creature *monst, pos newLoc) {
    unsigned long creatureFlag = (monst == &player ? HAS_PLAYER : HAS_MONSTER);
    pmapAt(monst->loc)->flags &= ~creatureFlag;
    refreshDungeonCell(monst->loc);
    monst->turnsSpentStationary = 0;
    monst->loc = newLoc;
    pmapAt(newLoc)->flags |= creatureFlag;
    if ((monst->bookkeepingFlags & MB_SUBMERGED) && !cellHasTMFlag(newLoc, TM_ALLOWS_SUBMERGING)) {
        monst->bookkeepingFlags &= ~MB_SUBMERGED;
    }
    if (playerCanSee(newLoc.x, newLoc.y)
        && cellHasTMFlag(newLoc, TM_IS_SECRET)
        && cellHasTerrainFlag(newLoc, T_OBSTRUCTS_PASSABILITY)) {

        discover(newLoc.x, newLoc.y); // if you see a monster use a secret door, you discover it
    }
    refreshDungeonCell(newLoc);
    applyInstantTileEffectsToCreature(monst);
    if (monst == &player) {
        updateVision(true);
        // get any items at the destination location
        if (pmapAt(player.loc)->flags & HAS_ITEM) {
            pickUpItemAt(player.loc);
        }
    }
}

/// @brief Tries to move a monster one space or perform a melee attack in the given direction.
/// Handles confused movement, turn-consuming non-movement actions like vomiting, and unique
/// attack patterns (axe-like, whip, spear). Fast-moving monsters get 2 turns, moving one
/// space each time.
/// @param monst the monster
/// @param dx the x axis component of the direction [-1, 0, 1]
/// @param dy the y axis component of the direction [-1, 0, 1]
/// @return true if a turn-consuming action was performed. otherwise false (e.g. monster is
/// unwilling to attack or blocked by terrain)
boolean moveMonster(creature *monst, short dx, short dy) {
    short x = monst->loc.x, y = monst->loc.y;
    short newX, newY;
    short i;
    short confusedDirection, swarmDirection;
    creature *defender = NULL;
    const creature *hitList[16] = {NULL};
    enum directions dir;

    if (dx == 0 && dy == 0) {
        return false;
    }

    newX = x + dx;
    newY = y + dy;

    if (!coordinatesAreInMap(newX, newY)) {
        //DEBUG printf("\nProblem! Monster trying to move more than one space at a time.");
        return false;
    }

    // vomiting
    if (monst->status[STATUS_NAUSEOUS] && rand_percent(25)) {
        vomit(monst);
        monst->ticksUntilTurn = monst->movementSpeed;
        return true;
    }

    // move randomly?
    if (!monst->status[STATUS_ENTRANCED]) {
        if (monst->status[STATUS_CONFUSED]) {
            confusedDirection = randValidDirectionFrom(monst, x, y, false);
            if (confusedDirection != -1) {
                dx = nbDirs[confusedDirection][0];
                dy = nbDirs[confusedDirection][1];
            }
        } else if ((monst->info.flags & MONST_FLITS) && !(monst->bookkeepingFlags & MB_SEIZING) && rand_percent(33)) {
            confusedDirection = randValidDirectionFrom(monst, x, y, true);
            if (confusedDirection != -1) {
                dx = nbDirs[confusedDirection][0];
                dy = nbDirs[confusedDirection][1];
            }
        }
    }

    newX = x + dx;
    newY = y + dy;

    // Liquid-based monsters should never move or attack outside of liquid.
    if ((monst->info.flags & MONST_RESTRICTED_TO_LIQUID) && !cellHasTMFlag((pos){ newX, newY }, TM_ALLOWS_SUBMERGING)) {
        return false;
    }

    // Caught in spiderweb?
    if (monst->status[STATUS_STUCK] && !(pmap[newX][newY].flags & (HAS_PLAYER | HAS_MONSTER))
        && cellHasTerrainFlag((pos){ x, y }, T_ENTANGLES) && !(monst->info.flags & MONST_IMMUNE_TO_WEBS)) {
        if (!(monst->info.flags & MONST_INVULNERABLE)
            && --monst->status[STATUS_STUCK]) {

            monst->ticksUntilTurn = monst->movementSpeed;
            return true;
        } else if (tileCatalog[pmap[x][y].layers[SURFACE]].flags & T_ENTANGLES) {
            pmap[x][y].layers[SURFACE] = NOTHING;
        }
    }

    if (pmap[newX][newY].flags & (HAS_MONSTER | HAS_PLAYER)) {
        defender = monsterAtLoc((pos){ newX, newY });
    } else {
        if (monst->bookkeepingFlags & MB_SEIZED) {
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *defender = nextCreature(&it);
                if ((defender->bookkeepingFlags & MB_SEIZING)
                    && monstersAreEnemies(monst, defender)
                    && distanceBetween(monst->loc, defender->loc) == 1
                    && !diagonalBlocked(monst->loc.x, monst->loc.y, defender->loc.x, defender->loc.y, false)) {

                    monst->ticksUntilTurn = monst->movementSpeed;
                    return true;
                }
            }
            monst->bookkeepingFlags &= ~MB_SEIZED; // failsafe
        }
        if (monst->bookkeepingFlags & MB_SEIZING) {
            monst->bookkeepingFlags &= ~MB_SEIZING;
        }
    }


    for (dir = 0; dir < DIRECTION_COUNT; dir++) {
        if (dx == nbDirs[dir][0]
            && dy == nbDirs[dir][1]) {

            break;
        }
    }
    brogueAssert(dir != NO_DIRECTION);
    if (handleWhipAttacks(monst, dir, NULL)
        || handleSpearAttacks(monst, dir, NULL)) {

        monst->ticksUntilTurn = monst->attackSpeed;
        return true;
    }

    if (((defender && (defender->info.flags & MONST_ATTACKABLE_THRU_WALLS))
         || (isPassableOrSecretDoor((pos){ newX, newY })
             && !diagonalBlocked(x, y, newX, newY, false)
             && isPassableOrSecretDoor((pos){ x, y })))
        && (!defender || canPass(monst, defender) || monsterWillAttackTarget(monst, defender))) {
            // if it's a legal move

            if (defender) {
                if (canPass(monst, defender)) {

                    // swap places
                    pmapAt(defender->loc)->flags &= ~HAS_MONSTER;
                    refreshDungeonCell(defender->loc);

                    pmapAt(monst->loc)->flags &= ~HAS_MONSTER;
                    refreshDungeonCell(monst->loc);

                    monst->loc.x = newX;
                    monst->loc.y = newY;
                    pmapAt(monst->loc)->flags |= HAS_MONSTER;

                    if (monsterAvoids(defender, (pos){x, y})) { // don't want a flying monster to swap a non-flying monster into lava!
                        getQualifyingPathLocNear(&(defender->loc.x), &(defender->loc.y), x, y, true,
                                                 forbiddenFlagsForMonster(&(defender->info)), HAS_PLAYER,
                                                 forbiddenFlagsForMonster(&(defender->info)), (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS), false);
                    } else {
                        defender->loc.x = x;
                        defender->loc.y = y;
                    }
                    pmapAt(defender->loc)->flags |= HAS_MONSTER;

                    refreshDungeonCell(monst->loc);
                    refreshDungeonCell(defender->loc);

                    monst->ticksUntilTurn = monst->movementSpeed;
                    return true;
                }

                // Sights are set on an enemy monster. Would we rather swarm than attack?
                swarmDirection = monsterSwarmDirection(monst, defender);
                if (swarmDirection != NO_DIRECTION) {
                    const pos newLoc = posNeighborInDirection(monst->loc, swarmDirection);
                    setMonsterLocation(monst, newLoc);
                    monst->ticksUntilTurn = monst->movementSpeed;
                    return true;
                } else {
                    // attacking another monster!
                    monst->ticksUntilTurn = monst->attackSpeed;
                    if (!((monst->info.abilityFlags & MA_SEIZES) && !(monst->bookkeepingFlags & MB_SEIZING))) {
                        // Bog monsters and krakens won't surface on the turn that they seize their target.
                        monst->bookkeepingFlags &= ~MB_SUBMERGED;
                    }
                    refreshDungeonCell((pos){ x, y });

                    buildHitList(hitList, monst, defender,
                                 (monst->info.abilityFlags & MA_ATTACKS_ALL_ADJACENT) ? true : false);
                    // Attack!
                    for (i=0; i<16; i++) {
                        if (hitList[i]
                            && monsterWillAttackTarget(monst, hitList[i])
                            && !(hitList[i]->bookkeepingFlags & MB_IS_DYING)
                            && !rogue.gameHasEnded) {

                            attack(monst, hitList[i], false);
                        }
                    }
                }
                return true;
            } else {
                // okay we're moving!
                setMonsterLocation(monst, (pos){ newX, newY });
                monst->ticksUntilTurn = monst->movementSpeed;
                return true;
            }
        }
    return false;
}

/// @brief initialize a creature's status effects to the default values
/// @param monst the creature
void initializeStatus(creature *monst) {
    short i;

    for (i=0; i<NUMBER_OF_STATUS_EFFECTS; i++) {
        monst->status[i] = monst->maxStatus[i] = 0;
    }

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
    monst->status[STATUS_NUTRITION] = monst->maxStatus[STATUS_NUTRITION] = (monst == &player ? STOMACH_SIZE : 1000);
}

// Bumps a creature to a random nearby hospitable cell.
void findAlternativeHomeFor(creature *monst, short *x, short *y, boolean chooseRandomly) {
    short sCols[DCOLS], sRows[DROWS], i, j, maxPermissibleDifference, dist;

    fillSequentialList(sCols, DCOLS);
    fillSequentialList(sRows, DROWS);
    if (chooseRandomly) {
        shuffleList(sCols, DCOLS);
        shuffleList(sRows, DROWS);
    }

    for (maxPermissibleDifference = 1; maxPermissibleDifference < max(DCOLS, DROWS); maxPermissibleDifference++) {
        for (i=0; i < DCOLS; i++) {
            for (j=0; j<DROWS; j++) {
                dist = abs(sCols[i] - monst->loc.x) + abs(sRows[j] - monst->loc.y);
                if (dist <= maxPermissibleDifference
                    && dist > 0
                    && !(pmap[sCols[i]][sRows[j]].flags & (HAS_PLAYER | HAS_MONSTER))
                    && !monsterAvoids(monst, (pos){sCols[i], sRows[j]})
                    && !(monst == &player && cellHasTerrainFlag((pos){ sCols[i], sRows[j] }, T_PATHING_BLOCKER))) {

                    // Success!
                    *x = sCols[i];
                    *y = sRows[j];
                    return;
                }
            }
        }
    }
    // Failure!
    *x = *y = -1;
}

// blockingMap is optional
boolean getQualifyingLocNear(pos *loc,
                             pos target,
                             boolean hallwaysAllowed,
                             char blockingMap[DCOLS][DROWS],
                             unsigned long forbiddenTerrainFlags,
                             unsigned long forbiddenMapFlags,
                             boolean forbidLiquid,
                             boolean deterministic) {
    short candidateLocs = 0;

    // count up the number of candidate locations
    for (int k=0; k<max(DROWS, DCOLS) && !candidateLocs; k++) {
        for (int i = target.x-k; i <= target.x+k; i++) {
            for (int j = target.y-k; j <= target.y+k; j++) {
                if (coordinatesAreInMap(i, j)
                    && (i == target.x-k || i == target.x+k || j == target.y-k || j == target.y+k)
                    && (!blockingMap || !blockingMap[i][j])
                    && !cellHasTerrainFlag((pos){ i, j }, forbiddenTerrainFlags)
                    && !(pmap[i][j].flags & forbiddenMapFlags)
                    && (!forbidLiquid || pmap[i][j].layers[LIQUID] == NOTHING)
                    && (hallwaysAllowed || passableArcCount(i, j) < 2)) {
                    candidateLocs++;
                }
            }
        }
    }

    if (candidateLocs == 0) {
        return false;
    }

    // and pick one
    short randIndex;
    if (deterministic) {
        randIndex = 1 + candidateLocs / 2;
    } else {
        randIndex = rand_range(1, candidateLocs);
    }

    for (int k=0; k<max(DROWS, DCOLS); k++) {
        for (int i = target.x-k; i <= target.x+k; i++) {
            for (int j = target.y-k; j <= target.y+k; j++) {
                if (coordinatesAreInMap(i, j)
                    && (i == target.x-k || i == target.x+k || j == target.y-k || j == target.y+k)
                    && (!blockingMap || !blockingMap[i][j])
                    && !cellHasTerrainFlag((pos){ i, j }, forbiddenTerrainFlags)
                    && !(pmap[i][j].flags & forbiddenMapFlags)
                    && (!forbidLiquid || pmap[i][j].layers[LIQUID] == NOTHING)
                    && (hallwaysAllowed || passableArcCount(i, j) < 2)) {
                    if (--randIndex == 0) {
                        *loc = (pos){ .x = i, .y = j };
                        return true;
                    }
                }
            }
        }
    }

    brogueAssert(false);
    return false; // should never reach this point
}

boolean getQualifyingGridLocNear(pos *loc,
                                 pos target,
                                 boolean grid[DCOLS][DROWS],
                                 boolean deterministic) {
    short candidateLocs = 0;

    // count up the number of candidate locations
    for (int k=0; k<max(DROWS, DCOLS) && !candidateLocs; k++) {
        for (int i = target.x-k; i <= target.x+k; i++) {
            for (int j = target.y-k; j <= target.y+k; j++) {
                if (coordinatesAreInMap(i, j)
                    && (i == target.x-k || i == target.x+k || j == target.y-k || j == target.y+k)
                    && grid[i][j]) {

                    candidateLocs++;
                }
            }
        }
    }

    if (candidateLocs == 0) {
        return false;
    }

    // and pick one
    short randIndex;
    if (deterministic) {
        randIndex = 1 + candidateLocs / 2;
    } else {
        randIndex = rand_range(1, candidateLocs);
    }

    for (int k=0; k<max(DROWS, DCOLS); k++) {
        for (int i = target.x-k; i <= target.x+k; i++) {
            for (int j = target.y-k; j <= target.y+k; j++) {
                if (coordinatesAreInMap(i, j)
                    && (i == target.x-k || i == target.x+k || j == target.y-k || j == target.y+k)
                    && grid[i][j]) {

                    if (--randIndex == 0) {
                        *loc = (pos){ .x = i, .y = j };
                        return true;
                    }
                }
            }
        }
    }

    brogueAssert(false);
    return false; // should never reach this point
}

void makeMonsterDropItem(creature *monst) {
    short x, y;
    getQualifyingPathLocNear(&x, &y, monst->loc.x, monst->loc.y, true,
                             (T_DIVIDES_LEVEL), 0,
                             T_OBSTRUCTS_ITEMS, (HAS_PLAYER | HAS_STAIRS | HAS_ITEM), false);
    placeItemAt(monst->carriedItem, (pos){ x, y });
    monst->carriedItem = NULL;
    refreshDungeonCell((pos){ x, y });
}

void checkForContinuedLeadership(creature *monst) {
    boolean maintainLeadership = false;

    if (monst->bookkeepingFlags & MB_LEADER) {
        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *follower = nextCreature(&it);
            if (follower->leader == monst && monst != follower) {
                maintainLeadership = true;
                break;
            }
        }
    }
    if (!maintainLeadership) {
        monst->bookkeepingFlags &= ~MB_LEADER;
    }
}

void demoteMonsterFromLeadership(creature *monst) {
    creature *newLeader = NULL;
    boolean atLeastOneNewFollower = false;

    monst->bookkeepingFlags &= ~MB_LEADER;
    if (monst->mapToMe) {
        freeGrid(monst->mapToMe);
        monst->mapToMe = NULL;
    }

    for (int level = 0; level <= gameConst->deepestLevel; level++) {
        // we'll work on this level's monsters first, so that the new leader is preferably on the same level
        creatureList *nearbyList = (level == 0 ? monsters : &levels[level-1].monsters);
        for (creatureIterator it = iterateCreatures(nearbyList); hasNextCreature(it);) {
            creature *follower = nextCreature(&it);
            if (follower == monst || follower->leader != monst) continue;
            if (follower->bookkeepingFlags & MB_BOUND_TO_LEADER) {
                // gonna die in playerTurnEnded().
                follower->leader = NULL;
                follower->bookkeepingFlags &= ~MB_FOLLOWER;
            } else if (newLeader) {
                follower->leader = newLeader;
                atLeastOneNewFollower = true;
                follower->targetWaypointIndex = monst->targetWaypointIndex;
                if (follower->targetWaypointIndex >= 0) {
                    follower->waypointAlreadyVisited[follower->targetWaypointIndex] = false;
                }
            } else {
                newLeader = follower;
                follower->bookkeepingFlags |= MB_LEADER;
                follower->bookkeepingFlags &= ~MB_FOLLOWER;
                follower->leader = NULL;
            }
        }
    }

    if (newLeader
        && !atLeastOneNewFollower) {
        newLeader->bookkeepingFlags &= ~MB_LEADER;
    }

    for (int level = 0; level <= gameConst->deepestLevel; level++) {
        creatureList *candidateList = (level == 0 ? dormantMonsters : &levels[level-1].dormantMonsters);
        for (creatureIterator it = iterateCreatures(candidateList); hasNextCreature(it);) {
            creature *follower = nextCreature(&it);
            if (follower == monst || follower->leader != monst) continue;
            follower->leader = NULL;
            follower->bookkeepingFlags &= ~MB_FOLLOWER;
        }
    }
}

// Makes a monster dormant, or awakens it from that state
void toggleMonsterDormancy(creature *monst) {

    if (removeCreature(dormantMonsters, monst)) {
        // Found it! It's dormant. Wake it up.
        // It's been removed from the dormant list.

        // Add it to the normal list.
        prependCreature(monsters, monst);

        pmapAt(monst->loc)->flags &= ~HAS_DORMANT_MONSTER;

        // Does it need a new location?
        if (pmapAt(monst->loc)->flags & (HAS_MONSTER | HAS_PLAYER)) { // Occupied!
            getQualifyingPathLocNear(
                &(monst->loc.x),
                &(monst->loc.y),
                monst->loc.x,
                monst->loc.y,
                true,
                T_DIVIDES_LEVEL & avoidedFlagsForMonster(&(monst->info)),
                HAS_PLAYER,
                avoidedFlagsForMonster(&(monst->info)),
                (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS),
                false
            );
            // getQualifyingLocNear(loc, monst->loc.x, monst->loc.y, true, 0, T_PATHING_BLOCKER, (HAS_PLAYER | HAS_MONSTER), false, false);
            // monst->loc.x = loc[0];
            // monst->loc.y = loc[1];
        }

        if (monst->bookkeepingFlags & MB_MARKED_FOR_SACRIFICE) {
            monst->bookkeepingFlags |= MB_TELEPATHICALLY_REVEALED;
            if (monst->carriedItem) {
                makeMonsterDropItem(monst);
            }
        }

        // Miscellaneous transitional tasks.
        // Don't want it to move before the player has a chance to react.
        monst->ticksUntilTurn = 200;

        pmapAt(monst->loc)->flags |= HAS_MONSTER;
        monst->bookkeepingFlags &= ~MB_IS_DORMANT;
        fadeInMonster(monst);
        return;
    }

    if (removeCreature(monsters, monst)) {
        // Found it! It's alive. Put it into dormancy.
        // Add it to the dormant chain.
        prependCreature(dormantMonsters, monst);
        // Miscellaneous transitional tasks.
        pmapAt(monst->loc)->flags &= ~HAS_MONSTER;
        pmapAt(monst->loc)->flags |= HAS_DORMANT_MONSTER;
        monst->bookkeepingFlags |= MB_IS_DORMANT;
        return;
    }
}

/// @brief Gets a description of the effect a wand of domination will have on the given monster.
/// Assumes the wand is known to be domination.
/// @param buf The string to append
/// @param monst The monster
static void getMonsterDominationText(char *buf, const creature *monst) {

    if (!monst || monst->creatureState == MONSTER_ALLY || (monst->bookkeepingFlags & MB_CAPTIVE)) {
        return;
    }

    char monstName[COLS], monstNamePossessive[COLS];
    monsterName(monstName, monst, true);
    strcpy(monstNamePossessive, monstName);
    strcat(monstNamePossessive, endswith(monstName,"s") ? "'" : "'s");

    char newText[20*COLS];
    short successChance = 0;
    if (!(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
      successChance = wandDominate(monst);
    }

    if (monst->info.flags & MONST_INANIMATE) {
        sprintf(newText, "\n     A wand of domination will have no effect on objects like %s.",
                monstName);
    } else if (monst->info.flags & MONST_INVULNERABLE) {
            sprintf(newText, "\n     A wand of domination will not affect %s.",
                    monstName);
    } else if (successChance <= 0) {
        sprintf(newText, "\n     A wand of domination will fail at %s current health level.",
                monstNamePossessive);
    } else if (successChance >= 100) {
        sprintf(newText, "\n     A wand of domination will always succeed at %s current health level.",
                monstNamePossessive);
    } else {
        sprintf(newText, "\n     A wand of domination will have a %i%% chance of success at %s current health level.",
                successChance,
                monstNamePossessive);
    }
    strcat(buf, newText);
}

// Takes a string delimited with '&' and appends the given destination with proper comma usage.
static void buildProperCommaString(char *dest, char *newText) {

    if (newText == NULL || newText[0] == '\0' || (newText[0] == '&' && newText[1] == '\0')) {
        return;
    }

    int start = newText[0] == '&' ? 1 : 0; // ignore leading '&', if any
    int commaCount = 0;
    for (int i = start; newText[i] != '\0'; i++) {
        if (newText[i] == '&') {
            commaCount++;
        }
    }

    if (commaCount == 0) {
        strcat(dest, start == 0 ? newText : newText + 1);
        return;
    }

    // append the text
    int j = strlen(dest);
    for (int i = start; newText[i] != '\0'; i++) {
        if (newText[i] == '&') {
            dest[j] = '\0';
            if (!--commaCount) {
                strcat(dest, " and ");
                j += 5;
            } else {
                strcat(dest, ", ");
                j += 2;
            }
        } else {
            dest[j++] = newText[i];
        }
    }
    dest[j] = '\0';
}

/// @brief Builds a comma separated list of monster abilities and appends it to the given string.
/// @param monst The monster
/// @param abilitiesText The abilities string to append
/// @param includeNegatable True to include negatable abilities
/// @param includeNonNegatable True to include non-negatable abilities
static void getMonsterAbilitiesText(const creature *monst, char *abilitiesText, boolean includeNegatable, boolean includeNonNegatable) {

    char buf[TEXT_MAX_LENGTH] = "";
    if (includeNegatable && (monst->mutationIndex >= 0) && mutationCatalog[monst->mutationIndex].canBeNegated) {
        strcat(buf, "has a rare mutation");
    }

    if ((includeNegatable && monst->attackSpeed != monst->info.attackSpeed)
        || (includeNonNegatable && monst->attackSpeed == monst->info.attackSpeed)) {

        if (monst->attackSpeed < 100) {
            strcat(buf, "&attacks quickly");
        } else if (monst->attackSpeed > 100) {
            strcat(buf, "&attacks slowly");
        }
    }

    if ((includeNegatable && monst->movementSpeed != monst->info.movementSpeed)
        || (includeNonNegatable && monst->movementSpeed == monst->info.movementSpeed)) {

        if (monst->movementSpeed < 100) {
            strcat(buf, "&moves quickly");
        } else if (monst->movementSpeed > 100) {
            strcat(buf, "&moves slowly");
        }
    }

    if (includeNonNegatable) {
        if (monst->info.turnsBetweenRegen == 0) {
            strcat(buf, "&does not regenerate");
        } else if (monst->info.turnsBetweenRegen < 5000) {
            strcat(buf, "&regenerates quickly");
        }
    }

    for (int i = 0; monst->info.bolts[i] != BOLT_NONE; i++) {
        if (boltCatalog[monst->info.bolts[i]].abilityDescription[0]) {
            if ((includeNegatable && !(boltCatalog[monst->info.bolts[i]].flags & BF_NOT_NEGATABLE))
                || (includeNonNegatable && (boltCatalog[monst->info.bolts[i]].flags & BF_NOT_NEGATABLE))) {

                strcat(buf, "&");
                strcat(buf, boltCatalog[monst->info.bolts[i]].abilityDescription);
            }
        }
    }

    for (int i=0; i<32; i++) {
        if ((monst->info.abilityFlags & (Fl(i)))
            && monsterAbilityCatalog[i].description[0]) {
            if ((includeNegatable && monsterAbilityCatalog[i].isNegatable)
                || (includeNonNegatable && !monsterAbilityCatalog[i].isNegatable)) {

                strcat(buf, "&");
                strcat(buf, monsterAbilityCatalog[i].description);
            }
        }
    }

    for (int i=0; i<32; i++) {
        if ((monst->info.flags & (Fl(i)))
            && monsterBehaviorCatalog[i].description[0]) {
            if ((includeNegatable && monsterBehaviorCatalog[i].isNegatable)
                || (includeNonNegatable && !monsterBehaviorCatalog[i].isNegatable)) {

                strcat(buf, "&");
                strcat(buf, monsterBehaviorCatalog[i].description);
            }
        }
    }

    for (int i=0; i<32; i++) {
        if ((monst->bookkeepingFlags & (Fl(i)))
            && monsterBookkeepingFlagDescriptions[i][0]) {
            if ((includeNegatable && (monst->bookkeepingFlags & MB_SEIZING))
                || (includeNonNegatable && !(monst->bookkeepingFlags & MB_SEIZING))) {

                strcat(buf, "&");
                strcat(buf, monsterBookkeepingFlagDescriptions[i]);
            }
        }
    }

    buildProperCommaString(abilitiesText, buf);
}

static boolean staffOrWandEffectOnMonsterDescription(char *newText, item *theItem, creature *monst) {
    char theItemName[COLS], monstName[COLS];
    boolean successfulDescription = false;
    fixpt enchant = netEnchant(theItem);

    if ((theItem->category & (STAFF | WAND))
        && tableForItemCategory(theItem->category)[theItem->kind].identified) {

        monsterName(monstName, monst, true);
        itemName(theItem, theItemName, false, false, NULL);

        switch (boltEffectForItem(theItem)) {
            case BE_DAMAGE:
                if ((boltCatalog[boltForItem(theItem)].flags & BF_FIERY) && (monst->status[STATUS_IMMUNE_TO_FIRE])
                    || (monst->info.flags & MONST_INVULNERABLE)) {

                    sprintf(newText, "\n     Your %s (%c) will not harm %s.",
                            theItemName,
                            theItem->inventoryLetter,
                            monstName);
                    successfulDescription = true;
                } else if (theItem->flags & (ITEM_MAX_CHARGES_KNOWN | ITEM_IDENTIFIED)) {
                    if (staffDamageLow(enchant) >= monst->currentHP) {
                        sprintf(newText, "\n     Your %s (%c) will %s %s in one hit.",
                                theItemName,
                                theItem->inventoryLetter,
                                (monst->info.flags & MONST_INANIMATE) ? "destroy" : "kill",
                                monstName);
                    } else {
                        sprintf(newText, "\n     Your %s (%c) will hit %s for between %i%% and %i%% of $HISHER current health.",
                                theItemName,
                                theItem->inventoryLetter,
                                monstName,
                                100 * staffDamageLow(enchant) / monst->currentHP,
                                100 * staffDamageHigh(enchant) / monst->currentHP);
                    }
                    successfulDescription = true;
                }
                break;
            case BE_POISON:
                if (monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE)) {
                    sprintf(newText, "\n     Your %s (%c) will not affect %s.",
                            theItemName,
                            theItem->inventoryLetter,
                            monstName);
                } else {
                    sprintf(newText, "\n     Your %s (%c) will poison %s for %i%% of $HISHER current health.",
                            theItemName,
                            theItem->inventoryLetter,
                            monstName,
                            100 * staffPoison(enchant) / monst->currentHP);
                }
                successfulDescription = true;
                break;
            default:
                strcpy(newText, "");
                break;
        }
    }
    return successfulDescription;
}

typedef struct packSummary {
    boolean hasNegationWand;
    boolean hasNegationScroll;
    boolean hasNegationCharm;
    boolean hasDominationWand;
    boolean hasShatteringCharm;
    boolean hasShatteringScroll;
    boolean hasTunnelingStaff;
    int wandCount;
    int staffCount;
} packSummary;

static void summarizePack (packSummary *pack) {
    for (item *theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (theItem->category & (CHARM | WAND | SCROLL |STAFF)) {

            if (tableForItemCategory(theItem->category)[theItem->kind].identified) {
                switch (theItem->category) {
                    case WAND:
                        pack->wandCount++;
                        if (theItem->kind == WAND_NEGATION) {
                            pack->hasNegationWand = true;
                        } else if (theItem->kind == WAND_DOMINATION) {
                            pack->hasDominationWand = true;
                        }
                        break;
                    case STAFF:
                        pack->staffCount++;
                        if (theItem->kind == STAFF_TUNNELING) {
                            pack->hasTunnelingStaff = true;
                        }
                        break;
                    case CHARM:
                        if (theItem->kind == CHARM_NEGATION) {
                            pack->hasNegationCharm = true;
                        } else if (theItem->kind == CHARM_SHATTERING) {
                            pack->hasShatteringCharm = true;
                        }
                        break;
                    case SCROLL:
                        if (theItem->kind == SCROLL_NEGATION) {
                            pack->hasNegationScroll = true;
                        } else if (theItem->kind == SCROLL_SHATTERING) {
                            pack->hasShatteringScroll = true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
}


void monsterDetails(char buf[], creature *monst) {
    char monstName[COLS], capMonstName[COLS], theItemName[COLS * 3], newText[20*COLS];
    short i, combatMath, combatMath2, playerKnownAverageDamage, playerKnownMaxDamage, realArmorValue;
    item *theItem;

    buf[0] = '\0';

    monsterName(monstName, monst, true);
    strcpy(capMonstName, monstName);
    upperCase(capMonstName);

    if (!(monst->info.flags & MONST_RESTRICTED_TO_LIQUID)
         || cellHasTMFlag(monst->loc, TM_ALLOWS_SUBMERGING)) {
        // If the monster is not a beached whale, print the ordinary flavor text.
        sprintf(newText, "     %s\n     ", monsterText[monst->info.monsterID].flavorText);
        strcat(buf, newText);
    }

    if (monst->mutationIndex >= 0) {
        i = strlen(buf);
        i = encodeMessageColor(buf, i, mutationCatalog[monst->mutationIndex].textColor);
        strcpy(newText, mutationCatalog[monst->mutationIndex].description);
        resolvePronounEscapes(newText, monst);
        upperCase(newText);
        strcat(newText, "\n     ");
        strcat(buf, newText);
        i = strlen(buf);
        i = encodeMessageColor(buf, i, &white);
    }

    if (!(monst->info.flags & MONST_ATTACKABLE_THRU_WALLS)
        && cellHasTerrainFlag(monst->loc, T_OBSTRUCTS_PASSABILITY)) {
        // If the monster is trapped in impassible terrain, explain as much.
        sprintf(newText, "%s is trapped %s %s.\n     ",
                capMonstName,
                (tileCatalog[pmapAt(monst->loc)->layers[layerWithFlag(monst->loc.x, monst->loc.y, T_OBSTRUCTS_PASSABILITY)]].mechFlags & TM_STAND_IN_TILE) ? "in" : "on",
                tileCatalog[pmapAt(monst->loc)->layers[layerWithFlag(monst->loc.x, monst->loc.y, T_OBSTRUCTS_PASSABILITY)]].description);
        strcat(buf, newText);
    }

    // Allegiance and ability slots
    newText[0] = '\0';
    if (monst->creatureState == MONSTER_ALLY) {
        i = strlen(buf);
        i = encodeMessageColor(buf, i, &goodMessageColor);

        sprintf(newText, "%s is your ally.\n     ", capMonstName);
        strcat(buf, newText);
        if (monst->newPowerCount > 0) {
            i = strlen(buf);
            i = encodeMessageColor(buf, i, &advancementMessageColor);

            if (monst->newPowerCount == 1) {
                sprintf(newText, "$HESHE seems ready to learn something new.\n     ");
            } else {
                sprintf(newText, "$HESHE seems ready to learn %i new talents.\n     ", monst->newPowerCount);
            }
            resolvePronounEscapes(newText, monst); // So that it gets capitalized appropriately.
            upperCase(newText);
            strcat(buf, newText);
        }
    }

    if (!rogue.armor || (rogue.armor->flags & ITEM_IDENTIFIED)) {
        combatMath2 = hitProbability(monst, &player);
    } else {
        realArmorValue = player.info.defense;
        player.info.defense = (armorTable[rogue.armor->kind].range.upperBound + armorTable[rogue.armor->kind].range.lowerBound) / 2;
        player.info.defense += 10 * strengthModifier(rogue.armor) / FP_FACTOR;
        combatMath2 = hitProbability(monst, &player);
        player.info.defense = realArmorValue;
    }

    // Combat info for the monster attacking the player
    if ((monst->info.flags & MONST_RESTRICTED_TO_LIQUID) && !cellHasTMFlag(monst->loc, TM_ALLOWS_SUBMERGING)) {
        sprintf(newText, "     %s writhes helplessly on dry land.\n     ", capMonstName);
    } else if (rogue.armor
               && (rogue.armor->flags & ITEM_RUNIC)
               && (rogue.armor->flags & ITEM_RUNIC_IDENTIFIED)
               && rogue.armor->enchant2 == A_IMMUNITY
               && monsterIsInClass(monst, rogue.armor->vorpalEnemy)) {

        itemName(rogue.armor, theItemName, false, false, NULL);
        sprintf(newText, "Your %s renders you immune to %s.\n     ", theItemName, monstName);
    } else if (monst->info.damage.upperBound * monsterDamageAdjustmentAmount(monst) / FP_FACTOR == 0) {
        sprintf(newText, "%s deals no direct damage.\n     ", capMonstName);
    } else {
        i = strlen(buf);
        i = encodeMessageColor(buf, i, &badMessageColor);
        if (monst->info.abilityFlags & MA_POISONS) {
            combatMath = player.status[STATUS_POISONED]; // combatMath is poison duration
            for (i = 0; combatMath * (player.poisonAmount + i) < player.currentHP; i++) {
                combatMath += monst->info.damage.upperBound * monsterDamageAdjustmentAmount(monst) / FP_FACTOR;
            }
            if (i == 0) {
                // Already fatally poisoned.
                sprintf(newText, "%s has a %i%% chance to poison you and typically poisons for %i turns.\n     ",
                        capMonstName,
                        combatMath2,
                        (int) ((monst->info.damage.lowerBound + monst->info.damage.upperBound) * monsterDamageAdjustmentAmount(monst) / 2 / FP_FACTOR));
            } else {
            sprintf(newText, "%s has a %i%% chance to poison you, typically poisons for %i turns, and at worst, could fatally poison you in %i hit%s.\n     ",
                    capMonstName,
                    combatMath2,
                    (int) ((monst->info.damage.lowerBound + monst->info.damage.upperBound) * monsterDamageAdjustmentAmount(monst) / 2 / FP_FACTOR),
                    i,
                    (i > 1 ? "s" : ""));
            }
        } else {
            combatMath = ((player.currentHP + (monst->info.damage.upperBound * monsterDamageAdjustmentAmount(monst) / FP_FACTOR) - 1) * FP_FACTOR)
                    / (monst->info.damage.upperBound * monsterDamageAdjustmentAmount(monst));
            if (combatMath < 1) {
                combatMath = 1;
            }
            sprintf(newText, "%s has a %i%% chance to hit you, typically hits for %i%% of your current health, and at worst, could defeat you in %i hit%s.\n     ",
                    capMonstName,
                    combatMath2,
                    (int) (100 * (monst->info.damage.lowerBound + monst->info.damage.upperBound) * monsterDamageAdjustmentAmount(monst) / 2 / player.currentHP / FP_FACTOR),
                    combatMath,
                    (combatMath > 1 ? "s" : ""));
        }
    }
    upperCase(newText);
    strcat(buf, newText);

    if (!rogue.weapon || (rogue.weapon->flags & ITEM_IDENTIFIED)) {
        playerKnownAverageDamage = (player.info.damage.upperBound + player.info.damage.lowerBound) / 2;
        playerKnownMaxDamage = player.info.damage.upperBound;
    } else {
        fixpt strengthFactor = damageFraction(strengthModifier(rogue.weapon));
        short tempLow = rogue.weapon->damage.lowerBound * strengthFactor / FP_FACTOR;
        short tempHigh = rogue.weapon->damage.upperBound * strengthFactor / FP_FACTOR;

        playerKnownAverageDamage = max(1, (tempLow + tempHigh) / 2);
        playerKnownMaxDamage = max(1, tempHigh);
    }

    // Combat info for the player attacking the monster (or whether it's captive)
    if (playerKnownMaxDamage == 0) {
        i = strlen(buf);
        i = encodeMessageColor(buf, i, &white);

        sprintf(newText, "You deal no direct damage.");
    } else if (rogue.weapon
               && (rogue.weapon->flags & ITEM_RUNIC)
               && (rogue.weapon->flags & ITEM_RUNIC_IDENTIFIED)
               && rogue.weapon->enchant2 == W_SLAYING
               && monsterIsInClass(monst, rogue.weapon->vorpalEnemy)) {

        i = strlen(buf);
        i = encodeMessageColor(buf, i, &goodMessageColor);
        itemName(rogue.weapon, theItemName, false, false, NULL);
        sprintf(newText, "Your %s will slay %s in one stroke.", theItemName, monstName);
    } else if (monst->info.flags & (MONST_INVULNERABLE | MONST_IMMUNE_TO_WEAPONS)) {
        i = strlen(buf);
        i = encodeMessageColor(buf, i, &white);
        sprintf(newText, "%s is immune to your attacks.", monstName);
    } else if (monst->bookkeepingFlags & MB_CAPTIVE) {
        i = strlen(buf);
        i = encodeMessageColor(buf, i, &goodMessageColor);

        sprintf(newText, "%s is being held captive.", capMonstName);
    } else {
        i = strlen(buf);
        i = encodeMessageColor(buf, i, &goodMessageColor);

        combatMath = (monst->currentHP + playerKnownMaxDamage - 1) / playerKnownMaxDamage;
        if (combatMath < 1) {
            combatMath = 1;
        }
        if (rogue.weapon && !(rogue.weapon->flags & ITEM_IDENTIFIED)) {
            realArmorValue = rogue.weapon->enchant1;
            rogue.weapon->enchant1 = 0;
            combatMath2 = hitProbability(&player, monst);
            rogue.weapon->enchant1 = realArmorValue;
        } else {
            combatMath2 = hitProbability(&player, monst);
        }
        sprintf(newText, "You have a %i%% chance to hit %s, typically hit for %i%% of $HISHER current health, and at best, could defeat $HIMHER in %i hit%s.",
                combatMath2,
                monstName,
                100 * playerKnownAverageDamage / monst->currentHP,
                combatMath,
                (combatMath > 1 ? "s" : ""));
    }
    upperCase(newText);
    strcat(buf, newText);

    packSummary pack = {0};
    summarizePack(&pack);

    char buf2[COLS] = "";
    if (monsterIsNegatable(monst)) {
        if (pack.hasNegationCharm) {
            strcpy(buf2, "negation charm");
        }
        if (pack.hasNegationScroll) {
            strcat(buf2, "&scroll of negation");
        }
        if (pack.hasNegationWand && !(monst->info.abilityFlags & MA_REFLECT_100)) {
            strcat(buf2, "&wand of negation");
        }
    }

    char negationMethodText[COLS] = "";
    buildProperCommaString(negationMethodText, buf2);

    // todo: A wand of polymorph will have no effect on the <monster>

    // begin item-specific effects
    encodeMessageColor(buf, strlen(buf), &itemMessageColor);
    boolean printStaffOrWandEffect = true;

    // Will it die if negated and we have the means to negate?
    if ((monst->info.flags & MONST_DIES_IF_NEGATED) && negationMethodText[0]) {
        sprintf(newText, "\n     Your %s will %s %s.",
            negationMethodText,
            (monst->info.flags & MONST_INANIMATE) ? "destroy" : "kill",
            monstName);
        strcat(buf, newText);
    }

    // Will shattering or tunneling destroy it?
    if (monst->info.flags & MONST_ATTACKABLE_THRU_WALLS) {
        strcpy(buf2, "");
        if (pack.hasShatteringCharm) {
            strcpy(buf2, "shattering charm");
        }
        if (pack.hasShatteringScroll) {
            strcat(buf2, "&scroll of shattering");
        }
        if (pack.hasTunnelingStaff) {
            strcat(buf2, "&staff of tunneling");
        }

        char shatterMethodText[COLS] = "";
        buildProperCommaString(shatterMethodText, buf2);
        if (shatterMethodText[0]) {
            sprintf(newText, "\n     Your %s will destroy %s.", shatterMethodText, monstName);
            strcat(buf, newText);
        }
    }

    // Will it reflect all bolts?
    if ((monst->info.abilityFlags & MA_REFLECT_100) && (pack.staffCount || pack.wandCount)) {

        sprintf(newText, "\n     Bolts from your %s%s%s%s%s that hit %s will be reflected directly back at you.",
            pack.staffCount ? "staff" : "",
            pack.staffCount > 1 ? "s" : "",
            pack.wandCount && pack.staffCount ? " and " : "",
            pack.wandCount ? "wand" : "",
            pack.wandCount > 1 ? "s" : "",
            monstName);
        strcat(buf, newText);
        printStaffOrWandEffect = false;
    }

    // staffs and wands have no direct effect on the warden
    if (monst->info.flags & MONST_INVULNERABLE) {
        printStaffOrWandEffect = false;
    }

    if (printStaffOrWandEffect) {
        for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
            if ((theItem->category == STAFF || theItem->category == WAND)
                && tableForItemCategory(theItem->category)[theItem->kind].identified
                && staffOrWandEffectOnMonsterDescription(newText, theItem, monst)) {

                strcat(buf, newText);
            }
        }

        if (pack.hasDominationWand) {
            strcpy(newText, "");
            getMonsterDominationText(newText, monst);
            strcat(buf, newText);
        }
    }

    // monster has an item?
    if (monst->carriedItem) {
        i = strlen(buf);
        i = encodeMessageColor(buf, i, &itemMessageColor);
        itemName(monst->carriedItem, theItemName, true, true, NULL);
        sprintf(newText, "%s has %s.", capMonstName, theItemName);
        upperCase(newText);
        strcat(buf, "\n     ");
        strcat(buf, newText);
    }

    // was it negated?
    if (monst->wasNegated && monst->newPowerCount == monst->totalPowerCount) {
        i = strlen(buf);
        i = encodeMessageColor(buf, i, &pink);
        sprintf(newText, "%s is stripped of $HISHER special traits.", capMonstName);
        resolvePronounEscapes(newText, monst);
        upperCase(newText);
        strcat(buf, "\n     ");
        strcat(buf, newText);
    }

    // list the monster's abilities
    encodeMessageColor(buf, strlen(buf), &white);
    char abilitiesText[20*COLS] = "";

     // print all abilities if the player has no effective negation source, or they do and the monster dies to negation
    if (((monst->info.flags & MONST_DIES_IF_NEGATED) && negationMethodText[0]) || !negationMethodText[0]) {
        getMonsterAbilitiesText(monst, abilitiesText, true, true);
        if (abilitiesText[0]) {
            sprintf(newText, "\n     %s %s.", capMonstName, abilitiesText);
            strcat(buf, newText);
        }
    } else {
        getMonsterAbilitiesText(monst, abilitiesText, false, true); // print non-negatable abilities, if any
        boolean hasNonNegatableAbilities = false;
        if (abilitiesText[0]) {
            hasNonNegatableAbilities = true;
            sprintf(newText, "\n     %s %s.", capMonstName, abilitiesText);
            strcat(buf, newText);
        }

        strcpy(abilitiesText, "");
        getMonsterAbilitiesText(monst, abilitiesText, true, false); // then print negatable abilities, if any
        if (abilitiesText[0]) {
            sprintf(newText, "\n     %s%s has special traits that can be removed by a ", capMonstName, hasNonNegatableAbilities ? " also" : "");
            encodeMessageColor(newText, strlen(newText), &itemMessageColor);
            strcat(newText, negationMethodText);
            encodeMessageColor(newText, strlen(newText), &white);
            strcat(newText, ": it ");
            strcat(newText, abilitiesText);
            strcat(newText, ".");
            strcat(buf, newText);
        }
    }

    resolvePronounEscapes(buf, monst);
}
