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
#include "IncludeGlobals.h"

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
    short itemChance, mutationChance, i, mutationAttempt;

    // 1.17^x * 10, with x from 1 to 13:
    const int POW_DEEP_MUTATION[] = {11, 13, 16, 18, 21, 25, 30, 35, 41, 48, 56, 65, 76};

    creature *monst = calloc(1, sizeof(creature));
    clearStatus(monst);
    monst->info = monsterCatalog[monsterID];

    monst->mutationIndex = -1;
    if (mutationPossible
        && !(monst->info.flags & MONST_NEVER_MUTATED)
        && !(monst->info.abilityFlags & MA_NEVER_MUTATED)
        && rogue.depthLevel > MUTATIONS_OCCUR_ABOVE_LEVEL) {

        if (rogue.depthLevel <= AMULET_LEVEL) {
            mutationChance = clamp((rogue.depthLevel - MUTATIONS_OCCUR_ABOVE_LEVEL) * DEPTH_ACCELERATOR, 1, 10);
        } else {
            mutationChance = POW_DEEP_MUTATION[min((rogue.depthLevel - AMULET_LEVEL) * DEPTH_ACCELERATOR, 12)];
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
    monst->targetCorpseLoc[0] = monst->targetCorpseLoc[1] = 0;
    monst->lastSeenPlayerAt[0] = monst->lastSeenPlayerAt[1] = -1;
    monst->targetWaypointIndex = -1;
    for (i=0; i < MAX_WAYPOINT_COUNT; i++) {
        monst->waypointAlreadyVisited[i] = rand_range(0, 1);
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
    monst->status[STATUS_NUTRITION] = monst->maxStatus[STATUS_NUTRITION] = 1000;

    if (monst->info.flags & MONST_CARRY_ITEM_100) {
        itemChance = 100;
    } else if (monst->info.flags & MONST_CARRY_ITEM_25) {
        itemChance = 25;
    } else {
        itemChance = 0;
    }

    if (ITEMS_ENABLED
        && itemPossible
        && (rogue.depthLevel <= AMULET_LEVEL)
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

    if (!(monst->info.flags & MONST_INANIMATE) && !monst->status[STATUS_LIFESPAN_REMAINING]) {
        monst->bookkeepingFlags |= MB_HAS_SOUL;
    }

    return monst;
}

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
            && (terrainFlags(observer->loc.x, observer->loc.y) & T_IS_DEEP_WATER)
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

boolean monsterIsHidden(const creature *monst, const creature *observer) {
    if (monst->bookkeepingFlags & MB_IS_DORMANT) {
        return true;
    }
    if (observer && monstersAreTeammates(monst, observer)) {
        // Teammates can always see each other.
        return false;
    }
    if ((monst->status[STATUS_INVISIBLE] && !pmap[monst->loc.x][monst->loc.y].layers[GAS])) {
        // invisible and not in gas
        return true;
    }
    if (monsterHiddenBySubmersion(monst, observer)) {
        return true;
    }
    return false;
}

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

// This is different from canSeeMonster() in that it counts only physical sight -- not clairvoyance or telepathy.
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
boolean attackWouldBeFutile(const creature *attacker, const creature *defender) {
    if (cellHasTerrainFlag(defender->loc.x, defender->loc.y, T_OBSTRUCTS_PASSABILITY)
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

// This is a specific kind of willingness, bordering on ability.
// Intuition: if it swung an axe from that position, should it
// hit the defender? Or silently pass through it, as it does for
// allies?
boolean monsterWillAttackTarget(const creature *attacker, const creature *defender) {
    if (attacker == defender || (defender->bookkeepingFlags & MB_IS_DYING)) {
        return false;
    }
    if (attacker == &player
        && defender->creatureState == MONSTER_ALLY) {

        return false;
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
         && cellHasTerrainFlag(monst2->loc.x, monst2->loc.y, T_IS_DEEP_WATER))

        || ((monst2->info.flags & MONST_RESTRICTED_TO_LIQUID)
            && !(monst1->info.flags & MONST_IMMUNE_TO_WATER)
            && !(monst1->status[STATUS_LEVITATING])
            && cellHasTerrainFlag(monst1->loc.x, monst1->loc.y, T_IS_DEEP_WATER))) {

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

    for (i=0; i<NUMBER_HORDES; i++) {
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

    for (i=0; i<NUMBER_HORDES; i++) {
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
    newMonst->bookkeepingFlags &= ~(MB_LEADER | MB_CAPTIVE | MB_HAS_SOUL);
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
        pmap[newMonst->loc.x][newMonst->loc.y].flags |= HAS_MONSTER;
        refreshDungeonCell(newMonst->loc.x, newMonst->loc.y);
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
            && cellHasTMFlag(monst->loc.x, monst->loc.y, TM_ALLOWS_SUBMERGING)
            && !cellHasTerrainFlag(monst->loc.x, monst->loc.y, T_OBSTRUCTS_PASSABILITY)
            && !(monst->bookkeepingFlags & (MB_SEIZING | MB_SEIZED | MB_CAPTIVE))
            && ((monst->info.flags & (MONST_IMMUNE_TO_FIRE | MONST_INVULNERABLE))
                || monst->status[STATUS_IMMUNE_TO_FIRE]
                || !cellHasTerrainFlag(monst->loc.x, monst->loc.y, T_LAVA_INSTA_DEATH)));
}

// Returns true if at least one minion spawned.
boolean spawnMinions(short hordeID, creature *leader, boolean summoned, boolean itemPossible) {
    short iSpecies, iMember, count;
    unsigned long forbiddenTerrainFlags;
    hordeType *theHorde;
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
            } while (theHorde->spawnsIn && !cellHasTerrainType(monst->loc.x, monst->loc.y, theHorde->spawnsIn) && failsafe++ < 20);
            if (failsafe >= 20) {
                // abort
                killCreature(monst, true);
                break;
            }
            if (monsterCanSubmergeNow(monst)) {
                monst->bookkeepingFlags |= MB_SUBMERGED;
            }
            brogueAssert(!(pmap[monst->loc.x][monst->loc.y].flags & HAS_MONSTER));
            pmap[monst->loc.x][monst->loc.y].flags |= HAS_MONSTER;
            monst->bookkeepingFlags |= (MB_FOLLOWER | MB_JUST_SUMMONED);
            monst->leader = leader;
            monst->creatureState = leader->creatureState;
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

boolean drawManacle(short x, short y, enum directions dir) {
    enum tileType manacles[8] = {MANACLE_T, MANACLE_B, MANACLE_L, MANACLE_R, MANACLE_TL, MANACLE_BL, MANACLE_TR, MANACLE_BR};
    short newX = x + nbDirs[dir][0];
    short newY = y + nbDirs[dir][1];
    if (coordinatesAreInMap(newX, newY)
        && pmap[newX][newY].layers[DUNGEON] == FLOOR
        && pmap[newX][newY].layers[LIQUID] == NOTHING) {

        pmap[x + nbDirs[dir][0]][y + nbDirs[dir][1]].layers[SURFACE] = manacles[dir];
        return true;
    }
    return false;
}

void drawManacles(short x, short y) {
    enum directions fallback[4][3] = {{UPLEFT, UP, LEFT}, {DOWNLEFT, DOWN, LEFT}, {UPRIGHT, UP, RIGHT}, {DOWNRIGHT, DOWN, RIGHT}};
    short i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3 && !drawManacle(x, y, fallback[i][j]); j++);
    }
}

// If hordeID is 0, it's randomly assigned based on the depth, with a 10% chance of an out-of-depth spawn from 1-5 levels deeper.
// If x is negative, location is random.
// Returns a pointer to the leader.
creature *spawnHorde(short hordeID, short x, short y, unsigned long forbiddenFlags, unsigned long requiredFlags) {
    short loc[2];
    short i, failsafe, depth;
    hordeType *theHorde;
    creature *leader, *preexistingMonst;
    boolean tryAgain;

    if (rogue.depthLevel > 1 && rand_percent(10)) {
        depth = rogue.depthLevel + rand_range(1, min(5, rogue.depthLevel / 2));
        if (depth > AMULET_LEVEL) {
            depth = max(rogue.depthLevel, AMULET_LEVEL);
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
            if (x >= 0 && y >= 0) {
                if (cellHasTerrainFlag(x, y, T_PATHING_BLOCKER)
                    && (!hordeCatalog[hordeID].spawnsIn || !cellHasTerrainType(x, y, hordeCatalog[hordeID].spawnsIn))) {

                    // don't spawn a horde in special terrain unless it's meant to spawn there
                    tryAgain = true;
                }
                if (hordeCatalog[hordeID].spawnsIn && !cellHasTerrainType(x, y, hordeCatalog[hordeID].spawnsIn)) {
                    // don't spawn a horde on normal terrain if it's meant for special terrain
                    tryAgain = true;
                }
            }
        } while (--failsafe && tryAgain);
    }

    failsafe = 50;

    if (x < 0 || y < 0) {
        i = 0;
        do {
            while (!randomMatchingLocation(&(loc[0]), &(loc[1]), FLOOR, NOTHING, (hordeCatalog[hordeID].spawnsIn ? hordeCatalog[hordeID].spawnsIn : -1))
                   || passableArcCount(loc[0], loc[1]) > 1) {
                if (!--failsafe) {
                    return NULL;
                }
                hordeID = pickHordeType(depth, 0, forbiddenFlags, 0);

                if (hordeID < 0) {
                    return NULL;
                }
            }
            x = loc[0];
            y = loc[1];
            i++;

            // This "while" condition should contain IN_FIELD_OF_VIEW, since that is specifically
            // calculated from the entry stairs when the level is generated, and will prevent monsters
            // from spawning within FOV of the entry stairs.
        } while (i < 25 && (pmap[x][y].flags & (ANY_KIND_OF_VISIBLE | IN_FIELD_OF_VIEW)));
    }

//  if (hordeCatalog[hordeID].spawnsIn == DEEP_WATER && pmap[x][y].layers[LIQUID] != DEEP_WATER) {
//      message("Waterborne monsters spawned on land!", REQUIRE_ACKNOWLEDGMENT);
//  }

    theHorde = &hordeCatalog[hordeID];

    if (theHorde->machine > 0) {
        // Build the accompanying machine (e.g. a goblin encampment)
        buildAMachine(theHorde->machine, x, y, 0, NULL, NULL, NULL);
    }

    leader = generateMonster(theHorde->leaderType, true, true);
    leader->loc.x = x;
    leader->loc.y = y;

    if (hordeCatalog[hordeID].flags & HORDE_LEADER_CAPTIVE) {
        leader->bookkeepingFlags |= MB_CAPTIVE;
        leader->creatureState = MONSTER_WANDERING;
        if (leader->info.turnsBetweenRegen > 0) {
            leader->currentHP = leader->info.maxHP / 4 + 1;
        }

        // Draw the manacles unless the horde spawns in weird terrain (e.g. cages).
        if (!hordeCatalog[hordeID].spawnsIn) {
            drawManacles(x, y);
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

    preexistingMonst = monsterAtLoc(x, y);
    if (preexistingMonst) {
        killCreature(preexistingMonst, true); // If there's already a monster here, quietly bury the body.
    }

    brogueAssert(!(pmap[x][y].flags & HAS_MONSTER));

    pmap[x][y].flags |= HAS_MONSTER;
    if (playerCanSeeOrSense(x, y)) {
        refreshDungeonCell(x, y);
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
    getCellAppearance(monst->loc.x, monst->loc.y, &displayChar, &fColor, &bColor);
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

boolean summonMinions(creature *summoner) {
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
        pmap[summoner->loc.x][summoner->loc.y].flags &= ~HAS_MONSTER;
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
                teleport(monst, x, y, true);
                if (x != -1 && y != -1 && grid != NULL) {
                    grid[x][y] = 0;
                }
            }

            monst->bookkeepingFlags &= ~MB_JUST_SUMMONED;
            if (canSeeMonster(monst)) {
                seenMinionCount++;
                refreshDungeonCell(monst->loc.x, monst->loc.y);
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
            refreshDungeonCell(summoner->loc.x, summoner->loc.y);
        } else {
            pmap[summoner->loc.x][summoner->loc.y].flags |= HAS_MONSTER;
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

    short i, numberOfMonsters = min(20, 6 + 3 * max(0, rogue.depthLevel - AMULET_LEVEL)); // almost always 6.

    while (rand_percent(60)) {
        numberOfMonsters++;
    }
    for (i=0; i<numberOfMonsters; i++) {
        spawnHorde(0, -1, -1, (HORDE_IS_SUMMONED | HORDE_MACHINE_ONLY), 0); // random horde type, random location
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
    //        plotCharWithColor('X', mapToWindowX(x), mapToWindowY(y), &black, &white);
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
        monst = spawnHorde(0, x, y, (HORDE_IS_SUMMONED | HORDE_LEADER_CAPTIVE | HORDE_NO_PERIODIC_SPAWN | HORDE_MACHINE_ONLY), 0);
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
void teleport(creature *monst, short x, short y, boolean respectTerrainAvoidancePreferences) {
    short **grid, i, j;
    char monstFOV[DCOLS][DROWS];

    if (!coordinatesAreInMap(x, y)) {
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
        randomLocationInGrid(grid, &x, &y, 1);
//        DEBUG {
//            dumpLevelToScreen();
//            hiliteGrid(grid, &orange, 50);
//            plotCharWithColor('X', mapToWindowX(x), mapToWindowY(y), &white, &red);
//            temporaryMessage("Teleport candidate locations:", REQUIRE_ACKNOWLEDGMENT);
//        }
        freeGrid(grid);
        if (x < 0 || y < 0) {
            return; // Failure!
        }
    }
    // Always break free on teleport
    disentangle(monst);
    setMonsterLocation(monst, x, y);
    if (monst != &player) {
        chooseNewWanderDestination(monst);
    }
}

boolean isValidWanderDestination(creature *monst, short wpIndex) {
    return (wpIndex >= 0
            && wpIndex < rogue.wpCount
            && !monst->waypointAlreadyVisited[wpIndex]
            && rogue.wpDistance[wpIndex][monst->loc.x][monst->loc.y] >= 0
            && nextStep(rogue.wpDistance[wpIndex], monst->loc.x, monst->loc.y, monst, false) != NO_DIRECTION);
}

short closestWaypointIndex(creature *monst) {
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
unsigned long successorTerrainFlags(enum tileType tile, enum subseqDFTypes promotionType) {
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

unsigned long burnedTerrainFlagsAtLoc(short x, short y) {
    short layer;
    unsigned long flags = 0;

    for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
        if (tileCatalog[pmap[x][y].layers[layer]].flags & T_IS_FLAMMABLE) {
            flags |= successorTerrainFlags(pmap[x][y].layers[layer], SUBSEQ_BURN);
            if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_EXPLOSIVE_PROMOTE) {
                flags |= successorTerrainFlags(pmap[x][y].layers[layer], SUBSEQ_PROMOTE);
            }
        }
    }

    return flags;
}

unsigned long discoveredTerrainFlagsAtLoc(short x, short y) {
    short layer;
    unsigned long flags = 0;

    for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
        if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_IS_SECRET) {
            flags |= successorTerrainFlags(pmap[x][y].layers[layer], SUBSEQ_DISCOVER);
        }
    }

    return flags;
}

boolean monsterAvoids(creature *monst, short x, short y) {
    unsigned long terrainImmunities;
    creature *defender;
    unsigned long tFlags, cFlags;

    getLocationFlags(x, y, &tFlags, NULL, &cFlags, monst == &player);

    // everyone but the player avoids the stairs
    if ((x == rogue.downLoc.x && y == rogue.downLoc.y)
        || (x == rogue.upLoc.x && y == rogue.upLoc.y)) {

        return monst != &player;
    }

    // dry land
    if (monst->info.flags & MONST_RESTRICTED_TO_LIQUID
        && !cellHasTMFlag(x, y, TM_ALLOWS_SUBMERGING)) {
        return true;
    }

    // non-allied monsters can always attack the player
    if (player.loc.x == x && player.loc.y == y && monst != &player && monst->creatureState != MONSTER_ALLY) {
        return false;
    }

    // walls
    if (tFlags & T_OBSTRUCTS_PASSABILITY) {
        if (monst != &player
            && cellHasTMFlag(x, y, TM_IS_SECRET)
            && !(discoveredTerrainFlagsAtLoc(x, y) & avoidedFlagsForMonster(&(monst->info)))) {
            // This is so monsters can use secret doors but won't embed themselves in secret levers.
            return false;
        }
        if (distanceBetween(monst->loc.x, monst->loc.y, x, y) <= 1) {
            defender = monsterAtLoc(x, y);
            if (defender
                && (defender->info.flags & MONST_ATTACKABLE_THRU_WALLS)) {
                return false;
            }
        }
        return true;
    }

    // Monsters can always attack unfriendly neighboring monsters,
    // unless it is immune to us for whatever reason.
    if (distanceBetween(monst->loc.x, monst->loc.y, x, y) <= 1) {
        defender = monsterAtLoc(x, y);
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
    defender = monsterAtLoc(x, y);
    if (defender
        && !(defender->bookkeepingFlags & MB_IS_DYING)
        && monstersAreEnemies(monst, defender)
        && attackWouldBeFutile(monst, defender)) {

        return true;
    }

    // hidden terrain
    if (cellHasTMFlag(x, y, TM_IS_SECRET) && monst == &player) {
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
        && !cellHasTerrainFlag(monst->loc.x, monst->loc.y, T_IS_FIRE | T_SPONTANEOUSLY_IGNITES)
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
        && (burnedTerrainFlagsAtLoc(x, y) & (T_CAUSES_EXPLOSIVE_DAMAGE | T_CAUSES_DAMAGE | T_AUTO_DESCENT) & ~terrainImmunities)) {

        return true;
    }

    // fire
    if ((tFlags & T_IS_FIRE & ~terrainImmunities)
        && !cellHasTerrainFlag(monst->loc.x, monst->loc.y, T_IS_FIRE)
        && !(cFlags & (HAS_MONSTER | HAS_PLAYER))
        && (monst != &player || rogue.mapToShore[x][y] >= player.status[STATUS_IMMUNE_TO_FIRE])) {
        return true;
    }

    // non-fire harmful terrain
    if ((tFlags & T_HARMFUL_TERRAIN & ~T_IS_FIRE & ~terrainImmunities)
        && !cellHasTerrainFlag(monst->loc.x, monst->loc.y, (T_HARMFUL_TERRAIN & ~T_IS_FIRE))) {
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
            || (monst->creatureState == MONSTER_ALLY && !(cellHasTMFlag(x, y, TM_IS_SECRET))))
        && !(monst->status[STATUS_ENTRANCED])
        && (!(tFlags & T_ENTANGLES) || !(monst->info.flags & MONST_IMMUNE_TO_WEBS))) {
        return true;
    }

    // lava
    if ((tFlags & T_LAVA_INSTA_DEATH & ~terrainImmunities)
        && (!(tFlags & T_ENTANGLES) || !(monst->info.flags & MONST_IMMUNE_TO_WEBS))
        && (monst != &player || rogue.mapToShore[x][y] >= max(player.status[STATUS_IMMUNE_TO_FIRE], player.status[STATUS_LEVITATING]))) {
        return true;
    }

    // deep water
    if ((tFlags & T_IS_DEEP_WATER & ~terrainImmunities)
        && (!(tFlags & T_ENTANGLES) || !(monst->info.flags & MONST_IMMUNE_TO_WEBS))
        && !cellHasTerrainFlag(monst->loc.x, monst->loc.y, T_IS_DEEP_WATER)) {
        return true; // avoid only if not already in it
    }

    // poisonous lichen
    if ((tFlags & T_CAUSES_POISON & ~terrainImmunities)
        && !cellHasTerrainFlag(monst->loc.x, monst->loc.y, T_CAUSES_POISON)
        && (monst == &player || monst->creatureState != MONSTER_TRACKING_SCENT || monst->currentHP < 10)) {
        return true;
    }

    // Smart monsters don't attack in corridors if they belong to a group and they can help it.
    if ((monst->info.abilityFlags & MA_AVOID_CORRIDORS)
        && !(monst->status[STATUS_ENRAGED] && monst->currentHP <= monst->info.maxHP / 2)
        && monst->creatureState == MONSTER_TRACKING_SCENT
        && (monst->bookkeepingFlags & (MB_FOLLOWER | MB_LEADER))
        && passableArcCount(x, y) >= 2
        && passableArcCount(monst->loc.x, monst->loc.y) < 2
        && !cellHasTerrainFlag(monst->loc.x, monst->loc.y, (T_HARMFUL_TERRAIN & ~terrainImmunities))) {
        return true;
    }

    return false;
}

boolean moveMonsterPassivelyTowards(creature *monst, short targetLoc[2], boolean willingToAttackPlayer) {
    short x, y, dx, dy, newX, newY;

    x = monst->loc.x;
    y = monst->loc.y;

    if (targetLoc[0] == x) {
        dx = 0;
    } else {
        dx = (targetLoc[0] < x ? -1 : 1);
    }
    if (targetLoc[1] == y) {
        dy = 0;
    } else {
        dy = (targetLoc[1] < y ? -1 : 1);
    }

    if (dx == 0 && dy == 0) { // already at the destination
        return false;
    }

    newX = x + dx;
    newY = y + dy;

    if (!coordinatesAreInMap(newX, newY)) {
        return false;
    }

    if (monst->creatureState != MONSTER_TRACKING_SCENT && dx && dy) {
        if (abs(targetLoc[0] - x) > abs(targetLoc[1] - y) && rand_range(0, abs(targetLoc[0] - x)) > abs(targetLoc[1] - y)) {
            if (!(monsterAvoids(monst, newX, y) || (!willingToAttackPlayer && (pmap[newX][y].flags & HAS_PLAYER)) || !moveMonster(monst, dx, 0))) {
                return true;
            }
        } else if (abs(targetLoc[0] - x) < abs(targetLoc[1] - y) && rand_range(0, abs(targetLoc[1] - y)) > abs(targetLoc[0] - x)) {
            if (!(monsterAvoids(monst, x, newY) || (!willingToAttackPlayer && (pmap[x][newY].flags & HAS_PLAYER)) || !moveMonster(monst, 0, dy))) {
                return true;
            }
        }
    }

    // Try to move toward the goal diagonally if possible or else straight.
    // If that fails, try both directions for the shorter coordinate.
    // If they all fail, return false.
    if (monsterAvoids(monst, newX, newY) || (!willingToAttackPlayer && (pmap[newX][newY].flags & HAS_PLAYER)) || !moveMonster(monst, dx, dy)) {
        if (distanceBetween(x, y, targetLoc[0], targetLoc[1]) <= 1 && (dx == 0 || dy == 0)) { // cardinally adjacent
            return false; // destination is blocked
        }
        //abs(targetLoc[0] - x) < abs(targetLoc[1] - y)
        if ((max(targetLoc[0], x) - min(targetLoc[0], x)) < (max(targetLoc[1], y) - min(targetLoc[1], y))) {
            if (monsterAvoids(monst, x, newY) || (!willingToAttackPlayer && pmap[x][newY].flags & HAS_PLAYER) || !moveMonster(monst, 0, dy)) {
                if (monsterAvoids(monst, newX, y) || (!willingToAttackPlayer &&  pmap[newX][y].flags & HAS_PLAYER) || !moveMonster(monst, dx, 0)) {
                    if (monsterAvoids(monst, x-1, newY) || (!willingToAttackPlayer && pmap[x-1][newY].flags & HAS_PLAYER) || !moveMonster(monst, -1, dy)) {
                        if (monsterAvoids(monst, x+1, newY) || (!willingToAttackPlayer && pmap[x+1][newY].flags & HAS_PLAYER) || !moveMonster(monst, 1, dy)) {
                            return false;
                        }
                    }
                }
            }
        } else {
            if (monsterAvoids(monst, newX, y) || (!willingToAttackPlayer && pmap[newX][y].flags & HAS_PLAYER) || !moveMonster(monst, dx, 0)) {
                if (monsterAvoids(monst, x, newY) || (!willingToAttackPlayer && pmap[x][newY].flags & HAS_PLAYER) || !moveMonster(monst, 0, dy)) {
                    if (monsterAvoids(monst, newX, y-1) || (!willingToAttackPlayer && pmap[newX][y-1].flags & HAS_PLAYER) || !moveMonster(monst, dx, -1)) {
                        if (monsterAvoids(monst, newX, y+1) || (!willingToAttackPlayer && pmap[newX][y+1].flags & HAS_PLAYER) || !moveMonster(monst, dx, 1)) {
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true;
}

short distanceBetween(short x1, short y1, short x2, short y2) {
    return max(abs(x1 - x2), abs(y1 - y2));
}

void alertMonster(creature *monst) {
    monst->creatureState = (monst->creatureMode == MODE_PERM_FLEEING ? MONSTER_FLEEING : MONSTER_TRACKING_SCENT);
    monst->lastSeenPlayerAt[0] = player.loc.x;
    monst->lastSeenPlayerAt[1] = player.loc.y;
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

boolean monsterCanShootWebs(creature *monst) {
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
short awarenessDistance(creature *observer, creature *target) {
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
    if ((target == &player && (pmap[observer->loc.x][observer->loc.y].flags & IN_FIELD_OF_VIEW))
        || (target != &player && openPathBetween(observer->loc.x, observer->loc.y, target->loc.x, target->loc.y))) {

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
boolean awareOfTarget(creature *observer, creature *target) {
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
        && !(pmap[observer->loc.x][observer->loc.y].flags & IN_FIELD_OF_VIEW)) {
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

short closestWaypointIndexTo(const short x, const short y) {
    short i, closestDistance, closestIndex;

    closestDistance = 1000;
    closestIndex = -1;
    for (i=0; i < rogue.wpCount; i++) {
        if (rogue.wpDistance[i][x][y] < closestDistance) {
            closestDistance = rogue.wpDistance[i][x][y];
            closestIndex = i;
        }
    }
    return closestIndex;
}

void wanderToward(creature *monst, const short x, const short y) {
    if (coordinatesAreInMap(x, y)) {
        const short theWaypointIndex = closestWaypointIndexTo(x, y);
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
            && distanceBetween(x, y, monst2->loc.x, monst2->loc.y) < closestFearedEnemy
            && traversiblePathBetween(monst2, x, y)
            && openPathBetween(x, y, monst2->loc.x, monst2->loc.y)) {

            closestFearedEnemy = distanceBetween(x, y, monst2->loc.x, monst2->loc.y);
        }
    }

    if ((monst->creatureState == MONSTER_WANDERING)
        && awareOfPlayer
        && (pmap[player.loc.x][player.loc.y].flags & IN_FIELD_OF_VIEW)) {
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
        wanderToward(monst, monst->lastSeenPlayerAt[0], monst->lastSeenPlayerAt[1]);
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

            monst->lastSeenPlayerAt[0] = player.loc.x;
            monst->lastSeenPlayerAt[1] = player.loc.y;
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
                        return;
                    }
                    if (!monst->status[i]) {
                        monst->poisonAmount = 0;
                    }
                }
                break;
            case STATUS_STUCK:
                if (monst->status[i] && !cellHasTerrainFlag(monst->loc.x, monst->loc.y, T_ENTANGLES)) {
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

                    refreshDungeonCell(monst->loc.x, monst->loc.y);
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
            refreshDungeonCell(monst->loc.x, monst->loc.y);
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
        short x = coords[i].x;
        short y = coords[i].y;
        if (x == x2 && y == y2) {
            return true;
        }
        if (monsterAvoids(monst, x, y)) {
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
        if (cellHasTerrainFlag(x, y, blockingTerrain) || (pmap[x][y].flags & blockingFlags)) {
            return false;
        }
        if (x == x2 && y == y2) {
            return true;
        }
    }
    brogueAssert(false);
    return true; // should never get here
}

boolean openPathBetween(short x1, short y1, short x2, short y2) {
    pos startLoc = (pos){ .x = x1, .y = y1 };
    pos targetLoc = (pos){ .x = x2, .y = y2 };

    pos returnLoc;
    getImpactLoc(&returnLoc, startLoc, targetLoc, DCOLS, false, &boltCatalog[BOLT_NONE]);
    if (returnLoc.x == targetLoc.x && returnLoc.y == targetLoc.y) {
        return true;
    }
    return false;
}

// will return the player if the player is at (x, y).
creature *monsterAtLoc(short x, short y) {
    if (!(pmap[x][y].flags & (HAS_MONSTER | HAS_PLAYER))) {
        return NULL;
    }
    if (player.loc.x == x && player.loc.y == y) {
        return &player;
    }
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (monst->loc.x == x && monst->loc.y == y) {
            return monst;
        }
    }
    // This should be unreachable, since the HAS_MONSTER
    // flag was true at (x, y).
    brogueAssert(0);
    return NULL;
}

creature *dormantMonsterAtLoc(short x, short y) {
    if (!(pmap[x][y].flags & HAS_DORMANT_MONSTER)) {
        return NULL;
    }

    for (creatureIterator it = iterateCreatures(dormantMonsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (monst->loc.x == x && monst->loc.y == y) {
            return monst;
        }
    }
    // This should be unreachable, since the HAS_DORMANT_MONSTER
    // flag was true at (x, y).
    brogueAssert(0);
    return NULL;
}

enum boltType monsterHasBoltEffect(creature *monst, enum boltEffects boltEffectIndex) {
    short i;
    for (i=0; monst->info.bolts[i] != 0; i++) {
        if (boltCatalog[monst->info.bolts[i]].boltEffect == boltEffectIndex) {
            return monst->info.bolts[i];
        }
    }
    return BOLT_NONE;
}

void pathTowardCreature(creature *monst, creature *target) {
    short targetLoc[2], dir;

    if (traversiblePathBetween(monst, target->loc.x, target->loc.y)) {
        if (distanceBetween(monst->loc.x, monst->loc.y, target->loc.x, target->loc.y) <= 2) {
            monst->bookkeepingFlags &= ~MB_GIVEN_UP_ON_SCENT;
        }
        targetLoc[0] = target->loc.x;
        targetLoc[1] = target->loc.y;
        moveMonsterPassivelyTowards(monst, targetLoc, (monst->creatureState != MONSTER_ALLY));
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
    if (distanceBetween(monst->loc.x, monst->loc.y, target->loc.x, target->loc.y) > 10
        || monstersAreEnemies(monst, target)) {

        if (monsterBlinkToPreferenceMap(monst, target->mapToMe, false)) { // if it blinked
            monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
            return;
        }
    }

    // follow the map.
    dir = nextStep(target->mapToMe, monst->loc.x, monst->loc.y, monst, true);
    if (dir == NO_DIRECTION) {
        dir = randValidDirectionFrom(monst, monst->loc.x, monst->loc.y, true);
    }
    if (dir == NO_DIRECTION) {
        return; // monster is blocked
    }
    targetLoc[0] = monst->loc.x + nbDirs[dir][0];
    targetLoc[1] = monst->loc.y + nbDirs[dir][1];

    moveMonsterPassivelyTowards(monst, targetLoc, (monst->creatureState != MONSTER_ALLY));
}

boolean creatureEligibleForSwarming(creature *monst) {
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
enum directions monsterSwarmDirection(creature *monst, creature *enemy) {
    short newX, newY, i;
    enum directions dir, targetDir;
    short dirList[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    boolean alternateDirectionExists;

    if (monst == &player || !creatureEligibleForSwarming(monst)) {
        return NO_DIRECTION;
    }

    if (distanceBetween(monst->loc.x, monst->loc.y, enemy->loc.x, enemy->loc.y) != 1
        || (diagonalBlocked(monst->loc.x, monst->loc.y, enemy->loc.x, enemy->loc.y, false) || (enemy->info.flags & MONST_ATTACKABLE_THRU_WALLS))
        || !monstersAreEnemies(monst, enemy)) {

        return NO_DIRECTION; // Too far from the enemy, diagonally blocked, or not enemies with it.
    }

    // Find a location that is adjacent to you and to the enemy.
    targetDir = NO_DIRECTION;
    shuffleList(dirList, 4);
    shuffleList(&(dirList[4]), 4);
    for (i=0; i<8 && targetDir == NO_DIRECTION; i++) {
        dir = dirList[i];
        newX = monst->loc.x + nbDirs[dir][0];
        newY = monst->loc.y + nbDirs[dir][1];
        if (coordinatesAreInMap(newX, newY)
            && distanceBetween(enemy->loc.x, enemy->loc.y, newX, newY) == 1
            && !(pmap[newX][newY].flags & (HAS_PLAYER | HAS_MONSTER))
            && !diagonalBlocked(monst->loc.x, monst->loc.y, newX, newY, false)
            && (!diagonalBlocked(enemy->loc.x, enemy->loc.y, newX, newY, false) || (enemy->info.flags & MONST_ATTACKABLE_THRU_WALLS))
            && !monsterAvoids(monst, newX, newY)) {

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
            && distanceBetween(monst->loc.x, monst->loc.y, ally->loc.x, ally->loc.y) == 1
            && !diagonalBlocked(monst->loc.x, monst->loc.y, ally->loc.x, ally->loc.y, false)
            && !monsterAvoids(ally, monst->loc.x, monst->loc.y)
            && (distanceBetween(enemy->loc.x, enemy->loc.y, ally->loc.x, ally->loc.y) > 1 || diagonalBlocked(enemy->loc.x, enemy->loc.y, ally->loc.x, ally->loc.y, false))) {

            // Found a prospective ally.
            // Check that there isn't already an open space from which to attack the enemy that is accessible to the ally.
            alternateDirectionExists = false;
            for (dir=0; dir< DIRECTION_COUNT && !alternateDirectionExists; dir++) {
                newX = ally->loc.x + nbDirs[dir][0];
                newY = ally->loc.y + nbDirs[dir][1];
                if (coordinatesAreInMap(newX, newY)
                    && !(pmap[newX][newY].flags & (HAS_PLAYER | HAS_MONSTER))
                    && distanceBetween(enemy->loc.x, enemy->loc.y, newX, newY) == 1
                    && !diagonalBlocked(enemy->loc.x, enemy->loc.y, newX, newY, false)
                    && !diagonalBlocked(ally->loc.x, ally->loc.y, newX, newY, false)
                    && !monsterAvoids(ally, newX, newY)) {

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
                        && distanceBetween(ally->loc.x, ally->loc.y, otherEnemy->loc.x, otherEnemy->loc.y) == 1
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
        nowPreference = preferenceMap[monst->loc.x + nbDirs[i][0]][monst->loc.y + nbDirs[i][1]];

        if (((blinkUphill && nowPreference > bestPreference) || (!blinkUphill && nowPreference < bestPreference))
            && !monsterAvoids(monst, monst->loc.x + nbDirs[i][0], monst->loc.y + nbDirs[i][1])) {

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
            && !monsterAvoids(monst, impact.x, impact.y)) {

            bestTarget = target;
            bestPreference  = nowPreference;

            if ((abs(impact.x - origin.x) > 1 || abs(impact.y - origin.y) > 1)
                // Note: these are deliberately backwards:
                || (cellHasTerrainFlag(impact.x, origin.y, T_OBSTRUCTS_PASSABILITY))
                || (cellHasTerrainFlag(origin.x, impact.y, T_OBSTRUCTS_PASSABILITY))) {
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

boolean fleeingMonsterAwareOfPlayer(creature *monst) {
    if (player.status[STATUS_INVISIBLE]) {
        return (distanceBetween(monst->loc.x, monst->loc.y, player.loc.x, player.loc.y) <= 1);
    } else {
        return (pmap[monst->loc.x][monst->loc.y].flags & IN_FIELD_OF_VIEW) ? true : false;
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
boolean monsterBlinkToSafety(creature *monst) {
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
            if (rogue.depthLevel < DEEPEST_LEVEL) {
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

// Some monsters never make good targets irrespective of what bolt we're contemplating.
// Return false for those. Otherwise, return true.
boolean generallyValidBoltTarget(creature *caster, creature *target) {
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
    return openPathBetween(caster->loc.x, caster->loc.y, target->loc.x, target->loc.y);
}

boolean targetEligibleForCombatBuff(creature *caster, creature *target) {
    if (caster->creatureState == MONSTER_ALLY) {
        if (canDirectlySeeMonster(caster)) {
            boolean handledPlayer = false;
            for (creatureIterator it = iterateCreatures(monsters); !handledPlayer || hasNextCreature(it);) {
                creature *enemy = !handledPlayer ? &player : nextCreature(&it);
                handledPlayer = true;
                if (monstersAreEnemies(&player, enemy)
                    && canSeeMonster(enemy)
                    && (pmap[enemy->loc.x][enemy->loc.y].flags & IN_FIELD_OF_VIEW)) {

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
boolean specificallyValidBoltTarget(creature *caster, creature *target, enum boltType theBoltType) {

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
    if ((target->info.flags & MONST_REFLECT_4)
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
        && burnedTerrainFlagsAtLoc(caster->loc.x, caster->loc.y) & avoidedFlagsForMonster(&(caster->info))) {
        // Don't shoot fireballs if you're standing on a tile that could combust into something that harms you.
        return false;
    }

    // Rules specific to bolt effects:
    switch (boltCatalog[theBoltType].boltEffect) {
        case BE_BECKONING:
            if (distanceBetween(caster->loc.x, caster->loc.y, target->loc.x, target->loc.y) <= 1) {
                return false;
            }
            break;
        case BE_ATTACK:
            if (cellHasTerrainFlag(target->loc.x, target->loc.y, T_OBSTRUCTS_PASSABILITY)
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
                    && cellHasTerrainFlag(target->loc.x, target->loc.y, (T_LAVA_INSTA_DEATH | T_IS_DEEP_WATER | T_AUTO_DESCENT))) {
                    // Drop the target into lava or a chasm if opportunity knocks.
                    return true;
                }
                if (monstersAreTeammates(caster, target)
                    && target->status[STATUS_DISCORDANT]
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

void monsterCastSpell(creature *caster, creature *target, enum boltType boltIndex) {
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
boolean monstUseBolt(creature *monst) {
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
boolean monstUseMagic(creature *monst) {
    if (monsterSummons(monst, (monst->info.flags & MONST_ALWAYS_USE_ABILITY))) {
        return true;
    } else if (monstUseBolt(monst)) {
        return true;
    }
    return false;
}

boolean isLocalScentMaximum(short x, short y) {
    enum directions dir;
    short newX, newY;

    const short baselineScent = scentMap[x][y];

    for (dir=0; dir< DIRECTION_COUNT; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];
        if (coordinatesAreInMap(newX, newY)
            && (scentMap[newX][newY] > baselineScent)
            && !cellHasTerrainFlag(newX, newY, T_OBSTRUCTS_PASSABILITY)
            && !diagonalBlocked(x, y, newX, newY, false)) {

            return false;
        }
    }
    return true;
}

// Returns the direction the player's scent points to from a given cell. Returns -1 if the nose comes up blank.
enum directions scentDirection(creature *monst) {
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
            otherMonst = monsterAtLoc(newX, newY);
            if (coordinatesAreInMap(newX, newY)
                && (scentMap[newX][newY] > bestNearbyScent)
                && (!(pmap[newX][newY].flags & HAS_MONSTER) || (otherMonst && canPass(monst, otherMonst)))
                && !cellHasTerrainFlag(newX, newY, T_OBSTRUCTS_PASSABILITY)
                && !diagonalBlocked(x, y, newX, newY, false)
                && !monsterAvoids(monst, newX, newY)) {

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
boolean resurrectAlly(const short x, const short y) {
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

        getQualifyingPathLocNear(&monToRaise->loc.x, &monToRaise->loc.y, x, y, true,
                                 (T_PATHING_BLOCKER | T_HARMFUL_TERRAIN), 0,
                                 0, (HAS_PLAYER | HAS_MONSTER), false);
        pmap[monToRaise->loc.x][monToRaise->loc.y].flags |= HAS_MONSTER;

        // Restore health etc.
        monToRaise->bookkeepingFlags &= ~(MB_IS_DYING | MB_ADMINISTRATIVE_DEATH | MB_HAS_DIED | MB_IS_FALLING);
        if (!(monToRaise->info.flags & MONST_FIERY)
            && monToRaise->status[STATUS_BURNING]) {

            monToRaise->status[STATUS_BURNING] = 0;
        }
        monToRaise->status[STATUS_DISCORDANT] = 0;
        heal(monToRaise, 100, true);

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

    if (distanceBetween(x, y, defender->loc.x, defender->loc.y) >= 4) {
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

boolean allyFlees(creature *ally, creature *closestEnemy) {
    const short x = ally->loc.x;
    const short y = ally->loc.y;

    if (!closestEnemy) {
        return false; // No one to flee from.
    }

    if (ally->info.maxHP <= 1 || (ally->status[STATUS_LIFESPAN_REMAINING]) > 0) { // Spectral blades and timed allies should never flee.
        return false;
    }

    if (distanceBetween(x, y, closestEnemy->loc.x, closestEnemy->loc.y) < 10
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

void monsterMillAbout(creature *monst, short movementChance) {
    enum directions dir;
    short targetLoc[2];

    const short x = monst->loc.x;
    const short y = monst->loc.y;

    if (rand_percent(movementChance)) {
        dir = randValidDirectionFrom(monst, x, y, true);
        if (dir != -1) {
            targetLoc[0] = x + nbDirs[dir][0];
            targetLoc[1] = y + nbDirs[dir][1];
            moveMonsterPassivelyTowards(monst, targetLoc, false);
        }
    }
}

void moveAlly(creature *monst) {
    creature *closestMonster = NULL;
    short i, j, x, y, dir, shortestDistance, targetLoc[2], leashLength;
    short **enemyMap, **costMap;
    char buf[DCOLS], monstName[DCOLS];

    x = monst->loc.x;
    y = monst->loc.y;

    targetLoc[0] = targetLoc[1] = 0;

    if (!(monst->leader)) {
        monst->leader = &player;
        monst->bookkeepingFlags |= MB_FOLLOWER;
    }

    // If we're standing in harmful terrain and there is a way to escape it, spend this turn escaping it.
    if (cellHasTerrainFlag(x, y, (T_HARMFUL_TERRAIN & ~(T_IS_FIRE | T_CAUSES_DAMAGE | T_CAUSES_PARALYSIS | T_CAUSES_CONFUSION)))
        || (cellHasTerrainFlag(x, y, T_IS_FIRE) && !monst->status[STATUS_IMMUNE_TO_FIRE])
        || (cellHasTerrainFlag(x, y, T_CAUSES_DAMAGE | T_CAUSES_PARALYSIS | T_CAUSES_CONFUSION) && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE)))) {

        if (!rogue.updatedMapToSafeTerrainThisTurn) {
            updateSafeTerrainMap();
        }

        if (monsterBlinkToPreferenceMap(monst, rogue.mapToSafeTerrain, false)) {
            monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
            return;
        }

        dir = nextStep(rogue.mapToSafeTerrain, x, y, monst, true);
        if (dir != -1) {
            targetLoc[0] = x + nbDirs[dir][0];
            targetLoc[1] = y + nbDirs[dir][1];
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
            && distanceBetween(x, y, target->loc.x, target->loc.y) < shortestDistance
            && traversiblePathBetween(monst, target->loc.x, target->loc.y)
            && (!cellHasTerrainFlag(target->loc.x, target->loc.y, T_OBSTRUCTS_PASSABILITY) || (target->info.flags & MONST_ATTACKABLE_THRU_WALLS))
            && (!target->status[STATUS_INVISIBLE] || rand_percent(33))) {

            shortestDistance = distanceBetween(x, y, target->loc.x, target->loc.y);
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
            targetLoc[0] = x + nbDirs[dir][0];
            targetLoc[1] = y + nbDirs[dir][1];
        }
        if (dir == -1
            || (allySafetyMap[targetLoc[0]][targetLoc[1]] >= allySafetyMap[x][y])
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
        && (distanceBetween(x, y, player.loc.x, player.loc.y) < leashLength || (monst->bookkeepingFlags & MB_DOES_NOT_TRACK_LEADER))
        && !(monst->info.flags & MONST_MAINTAINS_DISTANCE)
        && !attackWouldBeFutile(monst, closestMonster)) {

        // Blink toward an enemy?
        if (monsterHasBoltEffect(monst, BE_BLINKING)
            && ((monst->info.flags & MONST_ALWAYS_USE_ABILITY) || rand_percent(30))) {

            enemyMap = allocGrid();
            costMap = allocGrid();

            for (i=0; i<DCOLS; i++) {
                for (j=0; j<DROWS; j++) {
                    if (cellHasTerrainFlag(i, j, T_OBSTRUCTS_PASSABILITY)) {
                        costMap[i][j] = cellHasTerrainFlag(i, j, T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
                        enemyMap[i][j] = 0; // safeguard against OOS
                    } else if (monsterAvoids(monst, i, j)) {
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
                    && distanceBetween(x, y, target->loc.x, target->loc.y) < shortestDistance
                    && traversiblePathBetween(monst, target->loc.x, target->loc.y)
                    && (!monsterAvoids(monst, target->loc.x, target->loc.y) || (target->info.flags & MONST_ATTACKABLE_THRU_WALLS))
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

        targetLoc[0] = closestMonster->loc.x;
        targetLoc[1] = closestMonster->loc.y;
        moveMonsterPassivelyTowards(monst, targetLoc, false);
    } else if (monst->targetCorpseLoc[0]
               && !monst->status[STATUS_POISONED]
               && (!monst->status[STATUS_BURNING] || monst->status[STATUS_IMMUNE_TO_FIRE])) { // Going to start eating a corpse.
        moveMonsterPassivelyTowards(monst, monst->targetCorpseLoc, false);
        if (monst->loc.x == monst->targetCorpseLoc[0]
            && monst->loc.y == monst->targetCorpseLoc[1]
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
               || (distanceBetween(x, y, player.loc.x, player.loc.y) < 3 && (pmap[x][y].flags & IN_FIELD_OF_VIEW))) {

        monst->bookkeepingFlags &= ~MB_GIVEN_UP_ON_SCENT;
        monsterMillAbout(monst, 30);
    } else {
        if (!(monst->bookkeepingFlags & MB_GIVEN_UP_ON_SCENT)
            && distanceBetween(x, y, player.loc.x, player.loc.y) > 10
            && monsterBlinkToPreferenceMap(monst, scentMap, true)) {

            monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
            return;
        }
        dir = scentDirection(monst);
        if (dir == -1 || (monst->bookkeepingFlags & MB_GIVEN_UP_ON_SCENT)) {
            monst->bookkeepingFlags |= MB_GIVEN_UP_ON_SCENT;
            pathTowardCreature(monst, monst->leader);
        } else {
            targetLoc[0] = x + nbDirs[dir][0];
            targetLoc[1] = y + nbDirs[dir][1];
            moveMonsterPassivelyTowards(monst, targetLoc, false);
        }
    }
}

// Returns whether to abort the turn.
boolean updateMonsterCorpseAbsorption(creature *monst) {
    short i;
    char buf[COLS], buf2[COLS];

    if (monst->loc.x == monst->targetCorpseLoc[0]
        && monst->loc.y == monst->targetCorpseLoc[1]
        && (monst->bookkeepingFlags & MB_ABSORBING)) {

        if (--monst->corpseAbsorptionCounter <= 0) {
            monst->targetCorpseLoc[0] = monst->targetCorpseLoc[1] = 0;
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
                    sprintf(buf, "%s now %s!", buf2, monsterBehaviorFlagDescriptions[unflag(monst->absorptionFlags)]);
                } else {
                    sprintf(buf, "%s now %s!", buf2, monsterAbilityFlagDescriptions[unflag(monst->absorptionFlags)]);
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
        monst->targetCorpseLoc[0] = monst->targetCorpseLoc[1] = 0; // lost its chance
        monst->bookkeepingFlags &= ~MB_ABSORBING;
        monst->absorptionFlags = 0;
        monst->absorptionBolt = BOLT_NONE;
    } else if (monst->bookkeepingFlags & MB_ABSORBING) {
        monst->bookkeepingFlags &= ~MB_ABSORBING; // absorbing but not on the corpse
        if (monst->corpseAbsorptionCounter <= 15) {
            monst->targetCorpseLoc[0] = monst->targetCorpseLoc[1] = 0; // lost its chance
            monst->absorptionFlags = 0;
            monst->absorptionBolt = BOLT_NONE;
        }
    }
    return false;
}

void monstersTurn(creature *monst) {
    short x, y, playerLoc[2], targetLoc[2], dir, shortestDistance;
    boolean alreadyAtBestScent;
    creature *closestMonster;

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
                && distanceBetween(x, y, target->loc.x, target->loc.y) < shortestDistance
                && traversiblePathBetween(monst, target->loc.x, target->loc.y)
                && (!monsterAvoids(monst, target->loc.x, target->loc.y) || (target->info.flags & MONST_ATTACKABLE_THRU_WALLS))
                && (!target->status[STATUS_INVISIBLE] || rand_percent(33))) {

                shortestDistance = distanceBetween(x, y, target->loc.x, target->loc.y);
                closestMonster = target;
            }
        }
        if (closestMonster && monstUseMagic(monst)) {
            monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
            return;
        }
        if (closestMonster && !(monst->info.flags & MONST_MAINTAINS_DISTANCE)) {
            targetLoc[0] = closestMonster->loc.x;
            targetLoc[1] = closestMonster->loc.y;
            if (moveMonsterPassivelyTowards(monst, targetLoc, monst->creatureState == MONSTER_ALLY)) {
                return;
            }
        }
    }

    // hunting
    if ((monst->creatureState == MONSTER_TRACKING_SCENT
        || (monst->creatureState == MONSTER_ALLY && monst->status[STATUS_DISCORDANT]))
        // eels don't charge if you're not in the water
        && (!(monst->info.flags & MONST_RESTRICTED_TO_LIQUID) || cellHasTMFlag(player.loc.x, player.loc.y, TM_ALLOWS_SUBMERGING))) {

        // magic users sometimes cast spells
        if (monstUseMagic(monst)
            || (monsterHasBoltEffect(monst, BE_BLINKING)
                && ((monst->info.flags & MONST_ALWAYS_USE_ABILITY) || rand_percent(30))
                && monsterBlinkToPreferenceMap(monst, scentMap, true))) { // if he actually cast a spell

                monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
                return;
            }

        // if the monster is adjacent to an ally and not adjacent to the player, attack the ally
        if (distanceBetween(x, y, player.loc.x, player.loc.y) > 1
            || diagonalBlocked(x, y, player.loc.x, player.loc.y, false)) {
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *ally = nextCreature(&it);
                if (monsterWillAttackTarget(monst, ally)
                    && distanceBetween(x, y, ally->loc.x, ally->loc.y) == 1
                    && (!ally->status[STATUS_INVISIBLE] || rand_percent(33))) {

                    targetLoc[0] = ally->loc.x;
                    targetLoc[1] = ally->loc.y;
                    if (moveMonsterPassivelyTowards(monst, targetLoc, true)) { // attack
                        return;
                    }
                }
            }
        }

        if ((monst->status[STATUS_LEVITATING] || (monst->info.flags & MONST_RESTRICTED_TO_LIQUID) || (monst->bookkeepingFlags & MB_SUBMERGED)
             || ((monst->info.flags & (MONST_IMMUNE_TO_WEBS | MONST_INVULNERABLE) && monsterCanShootWebs(monst))))
            && pmap[x][y].flags & IN_FIELD_OF_VIEW) {

            playerLoc[0] = player.loc.x;
            playerLoc[1] = player.loc.y;
            moveMonsterPassivelyTowards(monst, playerLoc, true); // attack
            return;
        }
        if ((monst->info.flags & MONST_ALWAYS_HUNTING)
            && (monst->bookkeepingFlags & MB_GIVEN_UP_ON_SCENT)) {

            pathTowardCreature(monst, &player);
            return;
        }

        dir = scentDirection(monst);
        if (dir == NO_DIRECTION) {
            alreadyAtBestScent = isLocalScentMaximum(x, y);
            if (alreadyAtBestScent && monst->creatureState != MONSTER_ALLY && !(pmap[x][y].flags & IN_FIELD_OF_VIEW)) {
                if (monst->info.flags & MONST_ALWAYS_HUNTING) {
                    pathTowardCreature(monst, &player);
                    monst->bookkeepingFlags |= MB_GIVEN_UP_ON_SCENT;
                    return;
                }
                monst->creatureState = MONSTER_WANDERING;
                // If we're out of the player's FOV and the scent map is a dead end,
                // wander over to near where we last saw the player.
                wanderToward(monst, monst->lastSeenPlayerAt[0], monst->lastSeenPlayerAt[1]);
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
            targetLoc[0] = x + nbDirs[dir][0];
            targetLoc[1] = y + nbDirs[dir][1];
        }
        if (dir == -1 || (!moveMonster(monst, nbDirs[dir][0], nbDirs[dir][1]) && !moveMonsterPassivelyTowards(monst, targetLoc, true))) {
            boolean handledPlayer = false;
            for (creatureIterator it = iterateCreatures(monsters); !handledPlayer || hasNextCreature(it);) {
                creature *ally = !handledPlayer ? &player : nextCreature(&it);
                handledPlayer = true;
                if (!monst->status[STATUS_MAGICAL_FEAR] // Fearful monsters will never attack.
                    && monsterWillAttackTarget(monst, ally)
                    && distanceBetween(x, y, ally->loc.x, ally->loc.y) <= 1) {

                    moveMonster(monst, ally->loc.x - x, ally->loc.y - y); // attack the player if cornered
                    return;
                }
            }
        }
        return;
    } else if (monst->creatureState == MONSTER_WANDERING
               // eels wander if you're not in water
               || ((monst->info.flags & MONST_RESTRICTED_TO_LIQUID) && !cellHasTMFlag(player.loc.x, player.loc.y, TM_ALLOWS_SUBMERGING))) {

        // if we're standing in harmful terrain and there is a way to escape it, spend this turn escaping it.
        if (cellHasTerrainFlag(x, y, (T_HARMFUL_TERRAIN & ~T_IS_FIRE))
            || (cellHasTerrainFlag(x, y, T_IS_FIRE) && !monst->status[STATUS_IMMUNE_TO_FIRE] && !(monst->info.flags & MONST_INVULNERABLE))) {
            if (!rogue.updatedMapToSafeTerrainThisTurn) {
                updateSafeTerrainMap();
            }

            if (monsterBlinkToPreferenceMap(monst, rogue.mapToSafeTerrain, false)) {
                monst->ticksUntilTurn = monst->attackSpeed * (monst->info.flags & MONST_CAST_SPELLS_SLOWLY ? 2 : 1);
                return;
            }

            dir = nextStep(rogue.mapToSafeTerrain, x, y, monst, true);
            if (dir != -1) {
                targetLoc[0] = x + nbDirs[dir][0];
                targetLoc[1] = y + nbDirs[dir][1];
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

            if (distanceBetween(monst->loc.x, monst->loc.y, monst->leader->loc.x, monst->leader->loc.y) == 1) {
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
                    && distanceBetween(x, y, ally->loc.x, ally->loc.y) == 1
                    && (!ally->status[STATUS_INVISIBLE] || rand_percent(33))) {

                    targetLoc[0] = ally->loc.x;
                    targetLoc[1] = ally->loc.y;
                    if (moveMonsterPassivelyTowards(monst, targetLoc, true)) {
                        return;
                    }
                }
            }
        }

        // if you're a follower, don't get separated from the pack
        if (monst->bookkeepingFlags & MB_FOLLOWER) {
            if (distanceBetween(x, y, monst->leader->loc.x, monst->leader->loc.y) > 2) {
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
                targetLoc[0] = x + nbDirs[dir][0];
                targetLoc[1] = y + nbDirs[dir][1];
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

boolean isPassableOrSecretDoor(short x, short y) {
    return (!cellHasTerrainFlag(x, y, T_OBSTRUCTS_PASSABILITY)
            || (cellHasTMFlag(x, y, TM_IS_SECRET) && !(discoveredTerrainFlagsAtLoc(x, y) & T_OBSTRUCTS_PASSABILITY)));
}

boolean knownToPlayerAsPassableOrSecretDoor(short x, short y) {
    unsigned long tFlags, TMFlags;
    getLocationFlags(x, y, &tFlags, &TMFlags, NULL, true);
    return (!(tFlags & T_OBSTRUCTS_PASSABILITY)
            || ((TMFlags & TM_IS_SECRET) && !(discoveredTerrainFlagsAtLoc(x, y) & T_OBSTRUCTS_PASSABILITY)));
}

void setMonsterLocation(creature *monst, short newX, short newY) {
    unsigned long creatureFlag = (monst == &player ? HAS_PLAYER : HAS_MONSTER);
    pmap[monst->loc.x][monst->loc.y].flags &= ~creatureFlag;
    refreshDungeonCell(monst->loc.x, monst->loc.y);
    monst->turnsSpentStationary = 0;
    monst->loc.x = newX;
    monst->loc.y = newY;
    pmap[newX][newY].flags |= creatureFlag;
    if ((monst->bookkeepingFlags & MB_SUBMERGED) && !cellHasTMFlag(newX, newY, TM_ALLOWS_SUBMERGING)) {
        monst->bookkeepingFlags &= ~MB_SUBMERGED;
    }
    if (playerCanSee(newX, newY)
        && cellHasTMFlag(newX, newY, TM_IS_SECRET)
        && cellHasTerrainFlag(newX, newY, T_OBSTRUCTS_PASSABILITY)) {

        discover(newX, newY); // if you see a monster use a secret door, you discover it
    }
    refreshDungeonCell(newX, newY);
    applyInstantTileEffectsToCreature(monst);
    if (monst == &player) {
        updateVision(true);
        // get any items at the destination location
        if (pmap[player.loc.x][player.loc.y].flags & HAS_ITEM) {
            pickUpItemAt(player.loc.x, player.loc.y);
        }
    }
}

// Tries to move the given monster in the given vector; returns true if the move was legal
// (including attacking player, vomiting or struggling in vain)
// Be sure that dx, dy are both in the range [-1, 1] or the move will sometimes fail due to the diagonal check.
boolean moveMonster(creature *monst, short dx, short dy) {
    short x = monst->loc.x, y = monst->loc.y;
    short newX, newY;
    short i;
    short confusedDirection, swarmDirection;
    creature *defender = NULL;
    creature *hitList[16] = {NULL};
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
    if ((monst->info.flags & MONST_RESTRICTED_TO_LIQUID) && !cellHasTMFlag(newX, newY, TM_ALLOWS_SUBMERGING)) {
        return false;
    }

    // Caught in spiderweb?
    if (monst->status[STATUS_STUCK] && !(pmap[newX][newY].flags & (HAS_PLAYER | HAS_MONSTER))
        && cellHasTerrainFlag(x, y, T_ENTANGLES) && !(monst->info.flags & MONST_IMMUNE_TO_WEBS)) {
        if (!(monst->info.flags & MONST_INVULNERABLE)
            && --monst->status[STATUS_STUCK]) {

            monst->ticksUntilTurn = monst->movementSpeed;
            return true;
        } else if (tileCatalog[pmap[x][y].layers[SURFACE]].flags & T_ENTANGLES) {
            pmap[x][y].layers[SURFACE] = NOTHING;
        }
    }

    if (pmap[newX][newY].flags & (HAS_MONSTER | HAS_PLAYER)) {
        defender = monsterAtLoc(newX, newY);
    } else {
        if (monst->bookkeepingFlags & MB_SEIZED) {
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *defender = nextCreature(&it);
                if ((defender->bookkeepingFlags & MB_SEIZING)
                    && monstersAreEnemies(monst, defender)
                    && distanceBetween(monst->loc.x, monst->loc.y, defender->loc.x, defender->loc.y) == 1
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
         || (isPassableOrSecretDoor(newX, newY)
             && !diagonalBlocked(x, y, newX, newY, false)
             && isPassableOrSecretDoor(x, y)))
        && (!defender || canPass(monst, defender) || monsterWillAttackTarget(monst, defender))) {
            // if it's a legal move

            if (defender) {
                if (canPass(monst, defender)) {

                    // swap places
                    pmap[defender->loc.x][defender->loc.y].flags &= ~HAS_MONSTER;
                    refreshDungeonCell(defender->loc.x, defender->loc.y);

                    pmap[monst->loc.x][monst->loc.y].flags &= ~HAS_MONSTER;
                    refreshDungeonCell(monst->loc.x, monst->loc.y);

                    monst->loc.x = newX;
                    monst->loc.y = newY;
                    pmap[monst->loc.x][monst->loc.y].flags |= HAS_MONSTER;

                    if (monsterAvoids(defender, x, y)) { // don't want a flying monster to swap a non-flying monster into lava!
                        getQualifyingPathLocNear(&(defender->loc.x), &(defender->loc.y), x, y, true,
                                                 forbiddenFlagsForMonster(&(defender->info)), HAS_PLAYER,
                                                 forbiddenFlagsForMonster(&(defender->info)), (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS), false);
                    } else {
                        defender->loc.x = x;
                        defender->loc.y = y;
                    }
                    pmap[defender->loc.x][defender->loc.y].flags |= HAS_MONSTER;

                    refreshDungeonCell(monst->loc.x, monst->loc.y);
                    refreshDungeonCell(defender->loc.x, defender->loc.y);

                    monst->ticksUntilTurn = monst->movementSpeed;
                    return true;
                }

                // Sights are set on an enemy monster. Would we rather swarm than attack?
                swarmDirection = monsterSwarmDirection(monst, defender);
                if (swarmDirection != NO_DIRECTION) {
                    newX = monst->loc.x + nbDirs[swarmDirection][0];
                    newY = monst->loc.y + nbDirs[swarmDirection][1];
                    setMonsterLocation(monst, newX, newY);
                    monst->ticksUntilTurn = monst->movementSpeed;
                    return true;
                } else {
                    // attacking another monster!
                    monst->ticksUntilTurn = monst->attackSpeed;
                    if (!((monst->info.abilityFlags & MA_SEIZES) && !(monst->bookkeepingFlags & MB_SEIZING))) {
                        // Bog monsters and krakens won't surface on the turn that they seize their target.
                        monst->bookkeepingFlags &= ~MB_SUBMERGED;
                    }
                    refreshDungeonCell(x, y);

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
                setMonsterLocation(monst, newX, newY);
                monst->ticksUntilTurn = monst->movementSpeed;
                return true;
            }
        }
    return false;
}

void clearStatus(creature *monst) {
    short i;

    for (i=0; i<NUMBER_OF_STATUS_EFFECTS; i++) {
        monst->status[i] = monst->maxStatus[i] = 0;
    }
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
                    && !monsterAvoids(monst, sCols[i], sRows[j])
                    && !(monst == &player && cellHasTerrainFlag(sCols[i], sRows[j], T_PATHING_BLOCKER))) {

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
                             short x, short y,
                             boolean hallwaysAllowed,
                             char blockingMap[DCOLS][DROWS],
                             unsigned long forbiddenTerrainFlags,
                             unsigned long forbiddenMapFlags,
                             boolean forbidLiquid,
                             boolean deterministic) {
    short candidateLocs = 0;

    // count up the number of candidate locations
    for (int k=0; k<max(DROWS, DCOLS) && !candidateLocs; k++) {
        for (int i = x-k; i <= x+k; i++) {
            for (int j = y-k; j <= y+k; j++) {
                if (coordinatesAreInMap(i, j)
                    && (i == x-k || i == x+k || j == y-k || j == y+k)
                    && (!blockingMap || !blockingMap[i][j])
                    && !cellHasTerrainFlag(i, j, forbiddenTerrainFlags)
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
        for (int i = x-k; i <= x+k; i++) {
            for (int j = y-k; j <= y+k; j++) {
                if (coordinatesAreInMap(i, j)
                    && (i == x-k || i == x+k || j == y-k || j == y+k)
                    && (!blockingMap || !blockingMap[i][j])
                    && !cellHasTerrainFlag(i, j, forbiddenTerrainFlags)
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
                                 short x, short y,
                                 boolean grid[DCOLS][DROWS],
                                 boolean deterministic) {
    short candidateLocs = 0;

    // count up the number of candidate locations
    for (int k=0; k<max(DROWS, DCOLS) && !candidateLocs; k++) {
        for (int i = x-k; i <= x+k; i++) {
            for (int j = y-k; j <= y+k; j++) {
                if (coordinatesAreInMap(i, j)
                    && (i == x-k || i == x+k || j == y-k || j == y+k)
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
        for (int i = x-k; i <= x+k; i++) {
            for (int j = y-k; j <= y+k; j++) {
                if (coordinatesAreInMap(i, j)
                    && (i == x-k || i == x+k || j == y-k || j == y+k)
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
    placeItem(monst->carriedItem, x, y);
    monst->carriedItem = NULL;
    refreshDungeonCell(x, y);
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

    for (int level = 0; level <= DEEPEST_LEVEL; level++) {
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

    for (int level = 0; level <= DEEPEST_LEVEL; level++) {
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
    //short loc[2] = {0, 0};

    if (removeCreature(dormantMonsters, monst)) {
        // Found it! It's dormant. Wake it up.
        // It's been removed from the dormant list.

        // Add it to the normal list.
        prependCreature(monsters, monst);

        pmap[monst->loc.x][monst->loc.y].flags &= ~HAS_DORMANT_MONSTER;

        // Does it need a new location?
        if (pmap[monst->loc.x][monst->loc.y].flags & (HAS_MONSTER | HAS_PLAYER)) { // Occupied!
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

        pmap[monst->loc.x][monst->loc.y].flags |= HAS_MONSTER;
        monst->bookkeepingFlags &= ~MB_IS_DORMANT;
        fadeInMonster(monst);
        return;
    }

    if (removeCreature(monsters, monst)) {
        // Found it! It's alive. Put it into dormancy.
        // Add it to the dormant chain.
        prependCreature(dormantMonsters, monst);
        // Miscellaneous transitional tasks.
        pmap[monst->loc.x][monst->loc.y].flags &= ~HAS_MONSTER;
        pmap[monst->loc.x][monst->loc.y].flags |= HAS_DORMANT_MONSTER;
        monst->bookkeepingFlags |= MB_IS_DORMANT;
        return;
    }
}

boolean staffOrWandEffectOnMonsterDescription(char *newText, item *theItem, creature *monst) {
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
            case BE_DOMINATION:
                if (monst->creatureState != MONSTER_ALLY) {
                    if (monst->info.flags & MONST_INANIMATE) {
                        sprintf(newText, "\n     A wand of domination will have no effect on objects like %s.",
                                monstName);
                    } else if (monst->info.flags & MONST_INVULNERABLE) {
                            sprintf(newText, "\n     A wand of domination will not affect %s.",
                                    monstName);
                    } else if (wandDominate(monst) <= 0) {
                        sprintf(newText, "\n     A wand of domination will fail at %s's current health level.",
                                monstName);
                    } else if (wandDominate(monst) >= 100) {
                        sprintf(newText, "\n     A wand of domination will always succeed at %s's current health level.",
                                monstName);
                    } else {
                        sprintf(newText, "\n     A wand of domination will have a %i%% chance of success at %s's current health level.",
                                wandDominate(monst),
                                monstName);
                    }
                    successfulDescription = true;
                }
                break;
            default:
                strcpy(newText, "");
                break;
        }
    }
    return successfulDescription;
}

void monsterDetails(char buf[], creature *monst) {
    char monstName[COLS], capMonstName[COLS], theItemName[COLS * 3], newText[20*COLS];
    short i, j, combatMath, combatMath2, playerKnownAverageDamage, playerKnownMaxDamage, commaCount, realArmorValue;
    boolean anyFlags, alreadyDisplayedDominationText = false;
    item *theItem;

    buf[0] = '\0';
    commaCount = 0;

    monsterName(monstName, monst, true);
    strcpy(capMonstName, monstName);
    upperCase(capMonstName);

    if (!(monst->info.flags & MONST_RESTRICTED_TO_LIQUID)
         || cellHasTMFlag(monst->loc.x, monst->loc.y, TM_ALLOWS_SUBMERGING)) {
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
        && cellHasTerrainFlag(monst->loc.x, monst->loc.y, T_OBSTRUCTS_PASSABILITY)) {
        // If the monster is trapped in impassible terrain, explain as much.
        sprintf(newText, "%s is trapped %s %s.\n     ",
                capMonstName,
                (tileCatalog[pmap[monst->loc.x][monst->loc.y].layers[layerWithFlag(monst->loc.x, monst->loc.y, T_OBSTRUCTS_PASSABILITY)]].mechFlags & TM_STAND_IN_TILE) ? "in" : "on",
                tileCatalog[pmap[monst->loc.x][monst->loc.y].layers[layerWithFlag(monst->loc.x, monst->loc.y, T_OBSTRUCTS_PASSABILITY)]].description);
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
    if ((monst->info.flags & MONST_RESTRICTED_TO_LIQUID) && !cellHasTMFlag(monst->loc.x, monst->loc.y, TM_ALLOWS_SUBMERGING)) {
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

    for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (staffOrWandEffectOnMonsterDescription(newText, theItem, monst)) {
            if (boltEffectForItem(theItem) == BE_DOMINATION) {
                if (alreadyDisplayedDominationText) {
                    continue;
                } else {
                    alreadyDisplayedDominationText = true;
                }
            }
            i = strlen(buf);
            i = encodeMessageColor(buf, i, &itemMessageColor);
            strcat(buf, newText);
        }
    }

    if (monst->carriedItem) {
        i = strlen(buf);
        i = encodeMessageColor(buf, i, &itemMessageColor);
        itemName(monst->carriedItem, theItemName, true, true, NULL);
        sprintf(newText, "%s has %s.", capMonstName, theItemName);
        upperCase(newText);
        strcat(buf, "\n     ");
        strcat(buf, newText);
    }

    if (monst->wasNegated && monst->newPowerCount == monst->totalPowerCount) {
        i = strlen(buf);
        i = encodeMessageColor(buf, i, &pink);
        sprintf(newText, "%s is stripped of $HISHER special traits.", capMonstName);
        resolvePronounEscapes(newText, monst);
        upperCase(newText);
        strcat(buf, "\n     ");
        strcat(buf, newText);
    }

    strcat(buf, "\n     ");

    i = strlen(buf);
    i = encodeMessageColor(buf, i, &white);

    anyFlags = false;
    sprintf(newText, "%s ", capMonstName);

    if (monst->attackSpeed < 100) {
        strcat(newText, "attacks quickly");
        anyFlags = true;
    } else if (monst->attackSpeed > 100) {
        strcat(newText, "attacks slowly");
        anyFlags = true;
    }

    if (monst->movementSpeed < 100) {
        if (anyFlags) {
            strcat(newText, "& ");
            commaCount++;
        }
        strcat(newText, "moves quickly");
        anyFlags = true;
    } else if (monst->movementSpeed > 100) {
        if (anyFlags) {
            strcat(newText, "& ");
            commaCount++;
        }
        strcat(newText, "moves slowly");
        anyFlags = true;
    }

    if (monst->info.turnsBetweenRegen == 0) {
        if (anyFlags) {
            strcat(newText, "& ");
            commaCount++;
        }
        strcat(newText, "does not regenerate");
        anyFlags = true;
    } else if (monst->info.turnsBetweenRegen < 5000) {
        if (anyFlags) {
            strcat(newText, "& ");
            commaCount++;
        }
        strcat(newText, "regenerates quickly");
        anyFlags = true;
    }

    // bolt flags
    for (i = 0; monst->info.bolts[i] != BOLT_NONE; i++) {
        if (boltCatalog[monst->info.bolts[i]].abilityDescription[0]) {
            if (anyFlags) {
                strcat(newText, "& ");
                commaCount++;
            }
            strcat(newText, boltCatalog[monst->info.bolts[i]].abilityDescription);
            anyFlags = true;
        }
    }

    // ability flags
    for (i=0; i<32; i++) {
        if ((monst->info.abilityFlags & (Fl(i)))
            && monsterAbilityFlagDescriptions[i][0]) {
            if (anyFlags) {
                strcat(newText, "& ");
                commaCount++;
            }
            strcat(newText, monsterAbilityFlagDescriptions[i]);
            anyFlags = true;
        }
    }

    // behavior flags
    for (i=0; i<32; i++) {
        if ((monst->info.flags & (Fl(i)))
            && monsterBehaviorFlagDescriptions[i][0]) {
            if (anyFlags) {
                strcat(newText, "& ");
                commaCount++;
            }
            strcat(newText, monsterBehaviorFlagDescriptions[i]);
            anyFlags = true;
        }
    }

    // bookkeeping flags
    for (i=0; i<32; i++) {
        if ((monst->bookkeepingFlags & (Fl(i)))
            && monsterBookkeepingFlagDescriptions[i][0]) {
            if (anyFlags) {
                strcat(newText, "& ");
                commaCount++;
            }
            strcat(newText, monsterBookkeepingFlagDescriptions[i]);
            anyFlags = true;
        }
    }

    if (anyFlags) {
        strcat(newText, ". ");
        //strcat(buf, "\n\n");
        j = strlen(buf);
        for (i=0; newText[i] != '\0'; i++) {
            if (newText[i] == '&') {
                if (!--commaCount) {
                    buf[j] = '\0';
                    strcat(buf, " and");
                    j += 4;
                } else {
                    buf[j++] = ',';
                }
            } else {
                buf[j++] = newText[i];
            }
        }
        buf[j] = '\0';
    }
    resolvePronounEscapes(buf, monst);
}
