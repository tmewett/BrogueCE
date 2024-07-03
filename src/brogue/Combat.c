/*
 *  Combat.c
 *  Brogue
 *
 *  Created by Brian Walker on 6/11/09.
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


/* Combat rules:
 * Each combatant has an accuracy rating. This is the percentage of their attacks that will ordinarily hit;
 * higher numbers are better for them. Numbers over 100 are permitted.
 *
 * Each combatant also has a defense rating. The "hit probability" is calculated as given by this formula:
 *
 *          hit probability = (accuracy) * 0.987 ^ (defense)
 *
 * when hit determinations are made. Negative numbers and numbers over 100 are permitted.
 * The hit is then randomly determined according to this final percentage.
 *
 * Some environmental factors can modify these numbers. An unaware, sleeping, stuck or paralyzed
 * combatant is always hit. An unaware, sleeping or paralyzed combatant also takes triple damage.
 *
 * If the hit lands, damage is calculated in the range provided. However, the clumping factor affects the
 * probability distribution. If the range is 0-10 with a clumping factor of 1, it's a uniform distribution.
 * With a clumping factor of 2, it's calculated as 2d5 (with d5 meaing a die numbered from 0 through 5).
 * With 3, it's 3d3, and so on. Note that a range not divisible by the clumping factor is defective,
 * as it will never be resolved in the top few numbers of the range. In fact, the top
 * (rangeWidth % clumpingFactor) will never succeed. Thus we increment the maximum of the first
 * (rangeWidth % clumpingFactor) die by 1, so that in fact 0-10 with a CF of 3 would be 1d4 + 2d3. Similarly,
 * 0-10 with CF 4 would be 2d3 + 2d2. By playing with the numbers, one can approximate a gaussian
 * distribution of any mean and standard deviation.
 *
 * Player combatants take their base defense value of their actual armor. Their accuracy is a combination of weapon, armor
 * and strength.
 *
 * Players have a base accuracy value of 100 throughout the game. Each point of weapon enchantment (net of
 * strength penalty/benefit) increases
 */

fixpt strengthModifier(item *theItem) {
    int difference = (rogue.strength - player.weaknessAmount) - theItem->strengthRequired;
    if (difference > 0) {
        return difference * FP_FACTOR / 4; // 0.25x
    } else {
        return difference * FP_FACTOR * 5/2; // 2.5x
    }
}

fixpt netEnchant(item *theItem) {
    fixpt retval = theItem->enchant1 * FP_FACTOR;
    if (theItem->category & (WEAPON | ARMOR)) {
        retval += strengthModifier(theItem);
    }
    // Clamp all net enchantment values to [-20, 50].
    return clamp(retval, -20 * FP_FACTOR, 50 * FP_FACTOR);
}

fixpt monsterDamageAdjustmentAmount(const creature *monst) {
    if (monst == &player) {
        // Handled through player strength routines elsewhere.
        return FP_FACTOR;
    } else {
        return damageFraction(monst->weaknessAmount * FP_FACTOR * -3/2);
    }
}

short monsterDefenseAdjusted(const creature *monst) {
    short retval;
    if (monst == &player) {
        // Weakness is already taken into account in recalculateEquipmentBonuses() for the player.
        retval = monst->info.defense;
    } else {
        retval = monst->info.defense - 25 * monst->weaknessAmount;
    }
    return max(retval, 0);
}

short monsterAccuracyAdjusted(const creature *monst) {
    short retval = monst->info.accuracy * accuracyFraction(monst->weaknessAmount * FP_FACTOR * -3/2) / FP_FACTOR;
    return max(retval, 0);
}

// does NOT account for auto-hit from sleeping or unaware defenders; does account for auto-hit from
// stuck or captive defenders and from weapons of slaying.
short hitProbability(creature *attacker, creature *defender) {
    short accuracy = monsterAccuracyAdjusted(attacker);
    short defense = monsterDefenseAdjusted(defender);
    short hitProbability;

    if (defender->status[STATUS_STUCK] || (defender->bookkeepingFlags & MB_CAPTIVE)) {
        return 100;
    }
    if ((defender->bookkeepingFlags & MB_SEIZED)
        && (attacker->bookkeepingFlags & MB_SEIZING)) {

        return 100;
    }
    if (attacker == &player && rogue.weapon) {
        if ((rogue.weapon->flags & ITEM_RUNIC)
            && rogue.weapon->enchant2 == W_SLAYING
            && monsterIsInClass(defender, rogue.weapon->vorpalEnemy)) {

            return 100;
        }
        accuracy = player.info.accuracy * accuracyFraction(netEnchant(rogue.weapon)) / FP_FACTOR;
    }
    hitProbability = accuracy * defenseFraction(defense * FP_FACTOR) / FP_FACTOR;
    if (hitProbability > 100) {
        hitProbability = 100;
    } else if (hitProbability < 0) {
        hitProbability = 0;
    }
    return hitProbability;
}

boolean attackHit(creature *attacker, creature *defender) {
    // automatically hit if the monster is sleeping or captive or stuck in a web
    if (defender->status[STATUS_STUCK]
        || defender->status[STATUS_PARALYZED]
        || (defender->bookkeepingFlags & MB_CAPTIVE)) {

        return true;
    }

    return rand_percent(hitProbability(attacker, defender));
}

static void addMonsterToContiguousMonsterGrid(short x, short y, creature *monst, char grid[DCOLS][DROWS]) {
    short newX, newY;
    enum directions dir;
    creature *tempMonst;

    grid[x][y] = true;
    for (dir=0; dir<4; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];

        if (coordinatesAreInMap(newX, newY) && !grid[newX][newY]) {
            tempMonst = monsterAtLoc((pos){ newX, newY });
            if (tempMonst && monstersAreTeammates(monst, tempMonst)) {
                addMonsterToContiguousMonsterGrid(newX, newY, monst, grid);
            }
        }
    }
}

static short alliedCloneCount(creature *monst) {
    short count = 0;
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *temp = nextCreature(&it);
        if (temp != monst
            && temp->info.monsterID == monst->info.monsterID
            && monstersAreTeammates(temp, monst)) {

            count++;
        }
    }
    if (rogue.depthLevel > 1) {
        for (creatureIterator it = iterateCreatures(&levels[rogue.depthLevel - 2].monsters); hasNextCreature(it);) {
            creature *temp = nextCreature(&it);
            if (temp != monst
                && temp->info.monsterID == monst->info.monsterID
                && monstersAreTeammates(temp, monst)) {

                count++;
            }
        }
    }
    if (rogue.depthLevel < gameConst->deepestLevel) {
        for (creatureIterator it = iterateCreatures(&levels[rogue.depthLevel].monsters); hasNextCreature(it);) {
            creature *temp = nextCreature(&it);
            if (temp != monst
                && temp->info.monsterID == monst->info.monsterID
                && monstersAreTeammates(temp, monst)) {

                count++;
            }
        }
    }
    return count;
}

// Splits a monster in half.
// The split occurs only if there is a spot adjacent to the contiguous
// group of monsters that the monster would not avoid.
// The contiguous group is supplemented with the given (x, y) coordinates, if any;
// this is so that jellies et al. can spawn behind the player in a hallway.
void splitMonster(creature *monst, creature *attacker) {
    char buf[DCOLS * 3];
    char monstName[DCOLS];
    char monsterGrid[DCOLS][DROWS], eligibleGrid[DCOLS][DROWS];
    creature *clone;
    pos loc = INVALID_POS;

    if ((monst->info.abilityFlags & MA_CLONE_SELF_ON_DEFEND) && alliedCloneCount(monst) < 100
        && monst->currentHP > 0 && !(monst->bookkeepingFlags & MB_IS_DYING)) {

        if (distanceBetween(monst->loc, attacker->loc) <= 1) {
            loc = attacker->loc;
        }
    } else {
        return;
    }

    zeroOutGrid(monsterGrid);
    zeroOutGrid(eligibleGrid);
    int eligibleLocationCount = 0;

    // Add the (x, y) location to the contiguous group, if any.
    if (isPosInMap(loc)) {
        monsterGrid[loc.x][loc.y] = true;
    }

    // Find the contiguous group of monsters.
    addMonsterToContiguousMonsterGrid(monst->loc.x, monst->loc.y, monst, monsterGrid);

    // Find the eligible edges around the group of monsters.
    for (int i=0; i<DCOLS; i++) {
        for (int j=0; j<DROWS; j++) {
            if (monsterGrid[i][j]) {
                for (int dir=0; dir<4; dir++) {
                    const int newX = i + nbDirs[dir][0];
                    const int newY = j + nbDirs[dir][1];
                    if (coordinatesAreInMap(newX, newY)
                        && !eligibleGrid[newX][newY]
                        && !monsterGrid[newX][newY]
                        && !(pmap[newX][newY].flags & (HAS_PLAYER | HAS_MONSTER))
                        && !monsterAvoids(monst, (pos){ newX, newY })) {

                        eligibleGrid[newX][newY] = true;
                        eligibleLocationCount++;
                    }
                }
            }
        }
    }
//    DEBUG {
//        hiliteCharGrid(eligibleGrid, &green, 75);
//        hiliteCharGrid(monsterGrid, &blue, 75);
//        temporaryMessage("Jelly spawn possibilities (green = eligible, blue = monster):", REQUIRE_ACKNOWLEDGMENT);
//        displayLevel();
//    }

    // Pick a random location on the eligibleGrid and add the clone there.
    if (eligibleLocationCount) {
        int randIndex = rand_range(1, eligibleLocationCount);
        for (int i=0; i<DCOLS; i++) {
            for (int j=0; j<DROWS; j++) {
                if (eligibleGrid[i][j] && !--randIndex) {
                    // Found the spot!

                    monsterName(monstName, monst, true);
                    monst->currentHP = (monst->currentHP + 1) / 2;
                    clone = cloneMonster(monst, false, false);

                    // Split monsters don't inherit the learnings of their parents.
                    // Sorry, but self-healing jelly armies are too much.
                    // Mutation effects can be inherited, however; they're not learned abilities.
                    if (monst->mutationIndex >= 0) {
                        clone->info.flags           &= (monsterCatalog[clone->info.monsterID].flags | mutationCatalog[monst->mutationIndex].monsterFlags);
                        clone->info.abilityFlags    &= (monsterCatalog[clone->info.monsterID].abilityFlags | mutationCatalog[monst->mutationIndex].monsterAbilityFlags);
                    } else {
                        clone->info.flags           &= monsterCatalog[clone->info.monsterID].flags;
                        clone->info.abilityFlags    &= monsterCatalog[clone->info.monsterID].abilityFlags;
                    }
                    for (int b = 0; b < 20; b++) {
                        clone->info.bolts[b] = monsterCatalog[clone->info.monsterID].bolts[b];
                    }

                    if (!(clone->info.flags & MONST_FLIES)
                        && clone->status[STATUS_LEVITATING] == 1000) {

                        clone->status[STATUS_LEVITATING] = 0;
                    }

                    clone->loc = (pos){.x = i, .y = j};
                    pmap[i][j].flags |= HAS_MONSTER;
                    clone->ticksUntilTurn = max(clone->ticksUntilTurn, 101);
                    fadeInMonster(clone);
                    refreshSideBar(-1, -1, false);

                    if (canDirectlySeeMonster(monst)) {
                        sprintf(buf, "%s splits in two!", monstName);
                        message(buf, 0);
                    }

                    return;
                }
            }
        }
    }
}

// This function is called whenever one creature acts aggressively against another in a way that directly causes damage.
// This can be things like melee attacks, fire/lightning attacks or throwing a weapon.
void moralAttack(creature *attacker, creature *defender) {

    if (defender->currentHP > 0
        && !(defender->bookkeepingFlags & MB_IS_DYING)) {

        if (defender->status[STATUS_PARALYZED]) {
            defender->status[STATUS_PARALYZED] = 0;
             // Paralyzed creature gets a turn to react before the attacker moves again.
            defender->ticksUntilTurn = min(attacker->attackSpeed, 100) - 1;
        }
        if (defender->status[STATUS_MAGICAL_FEAR]) {
            defender->status[STATUS_MAGICAL_FEAR] = 1;
        }
        defender->status[STATUS_ENTRANCED] = 0;

        if ((defender->info.abilityFlags & MA_AVOID_CORRIDORS)) {
            defender->status[STATUS_ENRAGED] = defender->maxStatus[STATUS_ENRAGED] = 4;
        }

        if (attacker == &player
            && defender->creatureState == MONSTER_ALLY
            && !defender->status[STATUS_DISCORDANT]
            && !attacker->status[STATUS_CONFUSED]
            && !(attacker->bookkeepingFlags & MB_IS_DYING)) {

            unAlly(defender);
        }

        if ((attacker == &player || attacker->creatureState == MONSTER_ALLY)
            && defender != &player
            && defender->creatureState != MONSTER_ALLY) {

            alertMonster(defender); // this alerts the monster that you're nearby
        }
    }
}

/// @brief Determine if the action forfeits the paladin feat. In general, the player fails the feat if they attempt
/// to deal direct damage to a non-hunting creature that they are aware of and the creature would be damaged by the attack.
/// @param attacker 
/// @param defender 
void handlePaladinFeat(creature *defender) {
    if (rogue.featRecord[FEAT_PALADIN]
        && defender->creatureState != MONSTER_TRACKING_SCENT
        && (player.status[STATUS_TELEPATHIC] || canSeeMonster(defender))
        && !(defender->info.flags & (MONST_INANIMATE | MONST_TURRET | MONST_IMMOBILE | MONST_INVULNERABLE))
        && !(player.bookkeepingFlags & MB_SEIZED)
        && defender != &player
        ) {
        rogue.featRecord[FEAT_PALADIN] = false;
    }
}

static boolean playerImmuneToMonster(creature *monst) {
    if (monst != &player
        && rogue.armor
        && (rogue.armor->flags & ITEM_RUNIC)
        && (rogue.armor->enchant2 == A_IMMUNITY)
        && monsterIsInClass(monst, rogue.armor->vorpalEnemy)) {

        return true;
    } else {
        return false;
    }
}

static void specialHit(creature *attacker, creature *defender, short damage) {
    short itemCandidates, randItemIndex, stolenQuantity;
    item *theItem = NULL, *itemFromTopOfStack;
    char buf[COLS], buf2[COLS], buf3[COLS];

    if (!(attacker->info.abilityFlags & SPECIAL_HIT)) {
        return;
    }

    // Special hits that can affect only the player:
    if (defender == &player) {
        if (playerImmuneToMonster(attacker)) {
            return;
        }

        if (attacker->info.abilityFlags & MA_HIT_DEGRADE_ARMOR
            && defender == &player
            && rogue.armor
            && !(rogue.armor->flags & ITEM_PROTECTED)
            && (rogue.armor->enchant1 + rogue.armor->armor/10 > -10)) {

            rogue.armor->enchant1--;
            equipItem(rogue.armor, true, NULL);
            itemName(rogue.armor, buf2, false, false, NULL);
            sprintf(buf, "your %s weakens!", buf2);
            messageWithColor(buf, &itemMessageColor, 0);
            checkForDisenchantment(rogue.armor);
        }
        if (attacker->info.abilityFlags & MA_HIT_HALLUCINATE) {
            if (!player.status[STATUS_HALLUCINATING]) {
                combatMessage("you begin to hallucinate", 0);
            }
            if (!player.status[STATUS_HALLUCINATING]) {
                player.maxStatus[STATUS_HALLUCINATING] = 0;
            }
            player.status[STATUS_HALLUCINATING] += gameConst->onHitHallucinateDuration;
            player.maxStatus[STATUS_HALLUCINATING] = max(player.maxStatus[STATUS_HALLUCINATING], player.status[STATUS_HALLUCINATING]);
        }
        if (attacker->info.abilityFlags & MA_HIT_BURN
             && !defender->status[STATUS_IMMUNE_TO_FIRE]) {

            exposeCreatureToFire(defender);
        }

        if (attacker->info.abilityFlags & MA_HIT_STEAL_FLEE
            && !(attacker->carriedItem)
            && (packItems->nextItem)
            && attacker->currentHP > 0
            && !attacker->status[STATUS_CONFUSED] // No stealing from the player if you bump him while confused.
            && attackHit(attacker, defender)) {

            itemCandidates = numberOfMatchingPackItems(ALL_ITEMS, 0, (ITEM_EQUIPPED), false);
            if (itemCandidates) {
                randItemIndex = rand_range(1, itemCandidates);
                for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
                    if (!(theItem->flags & (ITEM_EQUIPPED))) {
                        if (randItemIndex == 1) {
                            break;
                        } else {
                            randItemIndex--;
                        }
                    }
                }
                if (theItem) {
                    if (theItem->category & WEAPON) { // Monkeys will steal half of a stack of weapons, and one of any other stack.
                        if (theItem->quantity > 3) {
                            stolenQuantity = (theItem->quantity + 1) / 2;
                        } else {
                            stolenQuantity = theItem->quantity;
                        }
                    } else {
                        stolenQuantity = 1;
                    }
                    if (stolenQuantity < theItem->quantity) { // Peel off stolen item(s).
                        itemFromTopOfStack = generateItem(ALL_ITEMS, -1);
                        *itemFromTopOfStack = *theItem; // Clone the item.
                        theItem->quantity -= stolenQuantity;
                        itemFromTopOfStack->quantity = stolenQuantity;
                        theItem = itemFromTopOfStack; // Redirect pointer.
                    } else {
                        if (rogue.swappedIn == theItem || rogue.swappedOut == theItem) {
                            rogue.swappedIn = NULL;
                            rogue.swappedOut = NULL;
                        }
                        removeItemFromChain(theItem, packItems);
                    }
                    theItem->flags &= ~ITEM_PLAYER_AVOIDS; // Explore will seek the item out if it ends up on the floor again.
                    attacker->carriedItem = theItem;
                    attacker->creatureMode = MODE_PERM_FLEEING;
                    attacker->creatureState = MONSTER_FLEEING;
                    monsterName(buf2, attacker, true);
                    itemName(theItem, buf3, false, true, NULL);
                    sprintf(buf, "%s stole %s!", buf2, buf3);
                    messageWithColor(buf, &badMessageColor, 0);
                    rogue.autoPlayingLevel = false;
                }
            }
        }
    }
    if ((attacker->info.abilityFlags & MA_POISONS)
        && damage > 0
        && !(defender->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {

        addPoison(defender, damage, 1);
    }
    if ((attacker->info.abilityFlags & MA_CAUSES_WEAKNESS)
        && damage > 0
        && !(defender->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {

        weaken(defender, gameConst->onHitWeakenDuration);
    }
    if (attacker->info.abilityFlags & MA_ATTACKS_STAGGER) {
        processStaggerHit(attacker, defender);
    }
}

static boolean forceWeaponHit(creature *defender, item *theItem) {
    short forceDamage;
    char buf[DCOLS*3], buf2[COLS], monstName[DCOLS];
    creature *otherMonster = NULL;
    boolean knowFirstMonsterDied = false, autoID = false;
    bolt theBolt;

    monsterName(monstName, defender, true);

    pos oldLoc = defender->loc;
    pos newLoc = (pos){
        .x = defender->loc.x + clamp(defender->loc.x - player.loc.x, -1, 1),
        .y = defender->loc.y + clamp(defender->loc.y - player.loc.y, -1, 1)
    };
    if (canDirectlySeeMonster(defender)
        && !cellHasTerrainFlag(newLoc, T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION)
        && !(pmapAt(newLoc)->flags & (HAS_MONSTER | HAS_PLAYER))) {
        sprintf(buf, "you launch %s backward with the force of your blow", monstName);
        buf[DCOLS] = '\0';
        combatMessage(buf, messageColorFromVictim(defender));
        autoID = true;
    }
    theBolt = boltCatalog[BOLT_BLINKING];
    theBolt.magnitude = max(1, netEnchant(theItem) / FP_FACTOR);
    zap(oldLoc, newLoc, &theBolt, false, false);
    if (!(defender->bookkeepingFlags & MB_IS_DYING)
        && distanceBetween(oldLoc, defender->loc) > 0
        && distanceBetween(oldLoc, defender->loc) < weaponForceDistance(netEnchant(theItem))) {

        if (pmap[defender->loc.x + newLoc.x - oldLoc.x][defender->loc.y + newLoc.y - oldLoc.y].flags & (HAS_MONSTER | HAS_PLAYER)) {
            otherMonster = monsterAtLoc((pos){ defender->loc.x + newLoc.x - oldLoc.x, defender->loc.y + newLoc.y - oldLoc.y });
            monsterName(buf2, otherMonster, true);
        } else {
            otherMonster = NULL;
            strcpy(buf2, tileCatalog[pmap[defender->loc.x + newLoc.x - oldLoc.x][defender->loc.y + newLoc.y - oldLoc.y].layers[highestPriorityLayer(defender->loc.x + newLoc.x - oldLoc.x, defender->loc.y + newLoc.y - oldLoc.y, true)]].description);
        }

        forceDamage = distanceBetween(oldLoc, defender->loc);

        if (!(defender->info.flags & (MONST_IMMUNE_TO_WEAPONS | MONST_INVULNERABLE))
            && inflictDamage(NULL, defender, forceDamage, &white, false)) {

            if (canDirectlySeeMonster(defender)) {
                knowFirstMonsterDied = true;
                sprintf(buf, "%s %s on impact with %s",
                        monstName,
                        (defender->info.flags & MONST_INANIMATE) ? "is destroyed" : "dies",
                        buf2);
                buf[DCOLS] = '\0';
                combatMessage(buf, messageColorFromVictim(defender));
                autoID = true;
            }
            killCreature(defender, false);
        } else {
            if (canDirectlySeeMonster(defender)) {
                sprintf(buf, "%s slams against %s",
                        monstName,
                        buf2);
                buf[DCOLS] = '\0';
                combatMessage(buf, messageColorFromVictim(defender));
                autoID = true;
            }
        }
        moralAttack(&player, defender);
        splitMonster(defender, &player);

        if (otherMonster
            && !(defender->info.flags & (MONST_IMMUNE_TO_WEAPONS | MONST_INVULNERABLE))) {

            if (inflictDamage(NULL, otherMonster, forceDamage, &white, false)) {
                if (canDirectlySeeMonster(otherMonster)) {
                    sprintf(buf, "%s %s%s when %s slams into $HIMHER",
                            buf2,
                            (knowFirstMonsterDied ? "also " : ""),
                            (defender->info.flags & MONST_INANIMATE) ? "is destroyed" : "dies",
                            monstName);
                    resolvePronounEscapes(buf, otherMonster);
                    buf[DCOLS] = '\0';
                    combatMessage(buf, messageColorFromVictim(otherMonster));
                    autoID = true;
                }
                killCreature(otherMonster, false);
            }
            if (otherMonster->creatureState != MONSTER_ALLY) {
                // Allies won't defect if you throw another monster at them, even though it hurts.
                moralAttack(&player, otherMonster);
                splitMonster(defender, &player);
            }
        }
    }
    return autoID;
}

void magicWeaponHit(creature *defender, item *theItem, boolean backstabbed) {
    char buf[DCOLS*3], monstName[DCOLS], theItemName[DCOLS];

    const color *effectColors[NUMBER_WEAPON_RUNIC_KINDS] = {&white, &black,
        &yellow, &pink, &green, &confusionGasColor, NULL, NULL, &darkRed, &rainbow};
    //  W_SPEED, W_QUIETUS, W_PARALYSIS, W_MULTIPLICITY, W_SLOWING, W_CONFUSION, W_FORCE, W_SLAYING, W_MERCY, W_PLENTY
    short chance, i;
    fixpt enchant;
    enum weaponEnchants enchantType = theItem->enchant2;
    creature *newMonst;
    boolean autoID = false;

    // If the defender is already dead, proceed only if the runic is speed or multiplicity.
    // (Everything else acts on the victim, which would literally be overkill.)
    if ((defender->bookkeepingFlags & MB_IS_DYING)
        && theItem->enchant2 != W_SPEED
        && theItem->enchant2 != W_MULTIPLICITY) {
        return;
    }

    enchant = netEnchant(theItem);

    if (theItem->enchant2 == W_SLAYING) {
        chance = (monsterIsInClass(defender, theItem->vorpalEnemy) ? 100 : 0);
    } else if (defender->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE)) {
        chance = 0;
    } else {
        chance = runicWeaponChance(theItem, false, 0);
        if (backstabbed && chance < 100) {
            chance = min(chance * 2, (chance + 100) / 2);
        }
    }
    if (chance > 0 && rand_percent(chance)) {
        if (!(defender->bookkeepingFlags & MB_SUBMERGED)) {
            switch (enchantType) {
                case W_SPEED:
                    createFlare(player.loc.x, player.loc.y, SCROLL_ENCHANTMENT_LIGHT);
                    break;
                case W_QUIETUS:
                    createFlare(defender->loc.x, defender->loc.y, QUIETUS_FLARE_LIGHT);
                    break;
                case W_SLAYING:
                    createFlare(defender->loc.x, defender->loc.y, SLAYING_FLARE_LIGHT);
                    break;
                default:
                    flashMonster(defender, effectColors[enchantType], 100);
                    break;
            }
            autoID = true;
        }
        rogue.disturbed = true;
        monsterName(monstName, defender, true);
        itemName(theItem, theItemName, false, false, NULL);

        switch (enchantType) {
            case W_SPEED:
                if (player.ticksUntilTurn != -1) {
                    sprintf(buf, "your %s trembles and time freezes for a moment", theItemName);
                    buf[DCOLS] = '\0';
                    combatMessage(buf, 0);
                    player.ticksUntilTurn = -1; // free turn!
                    autoID = true;
                }
                break;
            case W_SLAYING:
            case W_QUIETUS:
                inflictLethalDamage(&player, defender);
                sprintf(buf, "%s suddenly %s",
                        monstName,
                        (defender->info.flags & MONST_INANIMATE) ? "shatters" : "dies");
                buf[DCOLS] = '\0';
                combatMessage(buf, messageColorFromVictim(defender));
                killCreature(defender, false);
                autoID = true;
                break;
            case W_PARALYSIS:
                defender->status[STATUS_PARALYZED] = max(defender->status[STATUS_PARALYZED], weaponParalysisDuration(enchant));
                defender->maxStatus[STATUS_PARALYZED] = defender->status[STATUS_PARALYZED];
                if (canDirectlySeeMonster(defender)) {
                    sprintf(buf, "%s is frozen in place", monstName);
                    buf[DCOLS] = '\0';
                    combatMessage(buf, messageColorFromVictim(defender));
                    autoID = true;
                }
                break;
            case W_MULTIPLICITY:
                sprintf(buf, "Your %s emits a flash of light, and %sspectral duplicate%s appear%s!",
                        theItemName,
                        (weaponImageCount(enchant) == 1 ? "a " : ""),
                        (weaponImageCount(enchant) == 1 ? "" : "s"),
                        (weaponImageCount(enchant) == 1 ? "s" : ""));
                buf[DCOLS] = '\0';

                for (i = 0; i < (weaponImageCount(enchant)); i++) {
                    newMonst = generateMonster(MK_SPECTRAL_IMAGE, true, false);
                    getQualifyingPathLocNear(&(newMonst->loc.x), &(newMonst->loc.y), defender->loc.x, defender->loc.y, true,
                                             T_DIVIDES_LEVEL & avoidedFlagsForMonster(&(newMonst->info)), HAS_PLAYER,
                                             avoidedFlagsForMonster(&(newMonst->info)), (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS), false);
                    newMonst->bookkeepingFlags |= (MB_FOLLOWER | MB_BOUND_TO_LEADER | MB_DOES_NOT_TRACK_LEADER | MB_TELEPATHICALLY_REVEALED);
                    newMonst->bookkeepingFlags &= ~MB_JUST_SUMMONED;
                    newMonst->leader = &player;
                    newMonst->creatureState = MONSTER_ALLY;
                    if (theItem->flags & ITEM_ATTACKS_STAGGER) {
                        newMonst->info.attackSpeed *= 2;
                        newMonst->info.abilityFlags |= MA_ATTACKS_STAGGER;
                    }
                    if (theItem->flags & ITEM_ATTACKS_QUICKLY) {
                        newMonst->info.attackSpeed /= 2;
                    }
                    if (theItem->flags & ITEM_ATTACKS_PENETRATE) {
                        newMonst->info.abilityFlags |= MA_ATTACKS_PENETRATE;
                    }
                    if (theItem->flags & ITEM_ATTACKS_ALL_ADJACENT) {
                        newMonst->info.abilityFlags |= MA_ATTACKS_ALL_ADJACENT;
                    }
                    if (theItem->flags & ITEM_ATTACKS_EXTEND) {
                        newMonst->info.abilityFlags |= MA_ATTACKS_EXTEND;
                    }
                    newMonst->ticksUntilTurn = 100;
                    newMonst->info.accuracy = player.info.accuracy + (5 * netEnchant(theItem) / FP_FACTOR);
                    newMonst->info.damage = player.info.damage;
                    newMonst->status[STATUS_LIFESPAN_REMAINING] = newMonst->maxStatus[STATUS_LIFESPAN_REMAINING] = weaponImageDuration(enchant);
                    if (strLenWithoutEscapes(theItemName) <= 8) {
                        sprintf(newMonst->info.monsterName, "spectral %s", theItemName);
                    } else {
                        switch (rogue.weapon->kind) {
                            case BROADSWORD:
                                strcpy(newMonst->info.monsterName, "spectral sword");
                                break;
                            case HAMMER:
                                strcpy(newMonst->info.monsterName, "spectral hammer");
                                break;
                            case PIKE:
                                strcpy(newMonst->info.monsterName, "spectral pike");
                                break;
                            case WAR_AXE:
                                strcpy(newMonst->info.monsterName, "spectral axe");
                                break;
                            default:
                                strcpy(newMonst->info.monsterName, "spectral weapon");
                                break;
                        }
                    }
                    pmapAt(newMonst->loc)->flags |= HAS_MONSTER;
                    fadeInMonster(newMonst);
                }
                updateVision(true);

                message(buf, 0);
                autoID = true;
                break;
            case W_SLOWING:
                slow(defender, weaponSlowDuration(enchant));
                if (canDirectlySeeMonster(defender)) {
                    sprintf(buf, "%s slows down", monstName);
                    buf[DCOLS] = '\0';
                    combatMessage(buf, messageColorFromVictim(defender));
                    autoID = true;
                }
                break;
            case W_CONFUSION:
                defender->status[STATUS_CONFUSED] = max(defender->status[STATUS_CONFUSED], weaponConfusionDuration(enchant));
                defender->maxStatus[STATUS_CONFUSED] = defender->status[STATUS_CONFUSED];
                if (canDirectlySeeMonster(defender)) {
                    sprintf(buf, "%s looks very confused", monstName);
                    buf[DCOLS] = '\0';
                    combatMessage(buf, messageColorFromVictim(defender));
                    autoID = true;
                }
                break;
            case W_FORCE:
                autoID = forceWeaponHit(defender, theItem);
                break;
            case W_MERCY:
                heal(defender, gameConst->onHitMercyHealPercent, false);
                if (canSeeMonster(defender)) {
                    autoID = true;
                }
                break;
            case W_PLENTY:
                newMonst = cloneMonster(defender, true, true);
                if (newMonst) {
                    flashMonster(newMonst, effectColors[enchantType], 100);
                    if (canSeeMonster(newMonst)) {
                        autoID = true;
                    }
                }
                break;
            default:
                break;
        }
    }
    if (autoID) {
        autoIdentify(theItem);
    }
}

static void attackVerb(char returnString[DCOLS], creature *attacker, short hitPercentile) {
    short verbCount, increment;

    if (attacker != &player && (player.status[STATUS_HALLUCINATING] || !canSeeMonster(attacker))) {
        strcpy(returnString, "hits");
        return;
    }

    if (attacker == &player && !rogue.weapon) {
        strcpy(returnString, "punch");
        return;
    }

    for (verbCount = 0; verbCount < 4 && monsterText[attacker->info.monsterID].attack[verbCount + 1][0] != '\0'; verbCount++);
    increment = (100 / (verbCount + 1));
    hitPercentile = max(0, min(hitPercentile, increment * (verbCount + 1) - 1));
    strcpy(returnString, monsterText[attacker->info.monsterID].attack[hitPercentile / increment]);
    resolvePronounEscapes(returnString, attacker);
}

void applyArmorRunicEffect(char returnString[DCOLS], creature *attacker, short *damage, boolean melee) {
    char armorName[DCOLS], attackerName[DCOLS], monstName[DCOLS], buf[DCOLS * 3];
    boolean runicKnown;
    boolean runicDiscovered;
    short newDamage, dir, newX, newY, count, i;
    fixpt enchant;
    creature *monst, *hitList[8];

    returnString[0] = '\0';

    if (!(rogue.armor && rogue.armor->flags & ITEM_RUNIC)) {
        return; // just in case
    }

    enchant = netEnchant(rogue.armor);

    runicKnown = rogue.armor->flags & ITEM_RUNIC_IDENTIFIED;
    runicDiscovered = false;

    itemName(rogue.armor, armorName, false, false, NULL);

    monsterName(attackerName, attacker, true);

    switch (rogue.armor->enchant2) {
        case A_MULTIPLICITY:
            if (melee && !(attacker->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE)) && rand_percent(33)) {
                for (i = 0; i < armorImageCount(enchant); i++) {
                    monst = cloneMonster(attacker, false, true);
                    monst->bookkeepingFlags |= (MB_FOLLOWER | MB_BOUND_TO_LEADER | MB_DOES_NOT_TRACK_LEADER | MB_TELEPATHICALLY_REVEALED);
                    monst->info.flags |= MONST_DIES_IF_NEGATED;
                    monst->bookkeepingFlags &= ~(MB_JUST_SUMMONED | MB_SEIZED | MB_SEIZING);
                    monst->info.abilityFlags &= ~(MA_CAST_SUMMON | MA_DF_ON_DEATH); // No summoning by spectral images. Gotta draw the line!
                                                                                    // Also no exploding or infecting by spectral clones.
                    monst->leader = &player;
                    monst->creatureState = MONSTER_ALLY;
                    monst->status[STATUS_DISCORDANT] = 0; // Otherwise things can get out of control...
                    monst->ticksUntilTurn = 100;
                    monst->info.monsterID = MK_SPECTRAL_IMAGE;
                    if (monst->carriedMonster) {
                        creature *carried = monst->carriedMonster;
                        monst->carriedMonster = NULL;
                        killCreature(carried, true); // Otherwise you can get infinite phoenices from a discordant phoenix.
                    }

                    // Give it the glowy red light and color.
                    monst->info.intrinsicLightType = SPECTRAL_IMAGE_LIGHT;
                    monst->info.foreColor = &spectralImageColor;

                    // Temporary guest!
                    monst->status[STATUS_LIFESPAN_REMAINING] = monst->maxStatus[STATUS_LIFESPAN_REMAINING] = 3;
                    monst->currentHP = monst->info.maxHP = 1;
                    monst->info.defense = 0;

                    if (strLenWithoutEscapes(attacker->info.monsterName) <= 6) {
                        sprintf(monst->info.monsterName, "spectral %s", attacker->info.monsterName);
                    } else {
                        strcpy(monst->info.monsterName, "spectral clone");
                    }
                    fadeInMonster(monst);
                }
                updateVision(true);

                runicDiscovered = true;
                sprintf(returnString, "Your %s flashes, and spectral images of %s appear!", armorName, attackerName);
            }
            break;
        case A_MUTUALITY:
            if (*damage > 0) {
                count = 0;
                for (i=0; i<8; i++) {
                    hitList[i] = NULL;
                    dir = i % 8;
                    newX = player.loc.x + nbDirs[dir][0];
                    newY = player.loc.y + nbDirs[dir][1];
                    if (coordinatesAreInMap(newX, newY) && (pmap[newX][newY].flags & HAS_MONSTER)) {
                        monst = monsterAtLoc((pos){ newX, newY });
                        if (monst
                            && monst != attacker
                            && monstersAreEnemies(&player, monst)
                            && !(monst->info.flags & (MONST_IMMUNE_TO_WEAPONS | MONST_INVULNERABLE))
                            && !(monst->bookkeepingFlags & MB_IS_DYING)) {

                            hitList[i] = monst;
                            count++;
                        }
                    }
                }
                if (count) {
                    for (i=0; i<8; i++) {
                        if (hitList[i] && !(hitList[i]->bookkeepingFlags & MB_IS_DYING)) {
                            monsterName(monstName, hitList[i], true);
                            if (inflictDamage(&player, hitList[i], (*damage + count) / (count + 1), &blue, true)) {
                                if (canSeeMonster(hitList[i])) {
                                    sprintf(buf, "%s %s", monstName, ((hitList[i]->info.flags & MONST_INANIMATE) ? "is destroyed" : "dies"));
                                    combatMessage(buf, messageColorFromVictim(hitList[i]));
                                }
                                killCreature(hitList[i], false);
                            }
                        }
                    }
                    runicDiscovered = true;
                    if (!runicKnown) {
                        sprintf(returnString, "Your %s pulses, and the damage is shared with %s!",
                                armorName,
                                (count == 1 ? monstName : "the other adjacent enemies"));
                    }
                    *damage = (*damage + count) / (count + 1);
                }
            }
            break;
        case A_ABSORPTION:
            *damage -= rand_range(1, armorAbsorptionMax(enchant));
            if (*damage <= 0) {
                *damage = 0;
                runicDiscovered = true;
                if (!runicKnown) {
                    sprintf(returnString, "your %s pulses and absorbs the blow!", armorName);
                }
            }
            break;
        case A_REPRISAL:
            if (melee && !(attacker->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
                newDamage = max(1, armorReprisalPercent(enchant) * (*damage) / 100); // 5% reprisal per armor level
                if (inflictDamage(&player, attacker, newDamage, &blue, true)) {
                    if (canSeeMonster(attacker)) {
                        sprintf(returnString, "your %s pulses and %s drops dead!", armorName, attackerName);
                        runicDiscovered = true;
                    }
                    killCreature(attacker, false);
                } else if (!runicKnown) {
                    if (canSeeMonster(attacker)) {
                        sprintf(returnString, "your %s pulses and %s shudders in pain!", armorName, attackerName);
                        runicDiscovered = true;
                    }
                }
            }
            break;
        case A_IMMUNITY:
            if (monsterIsInClass(attacker, rogue.armor->vorpalEnemy)) {
                *damage = 0;
                runicDiscovered = true;
            }
            break;
        case A_BURDEN:
            if (rand_percent(10)) {
                rogue.armor->strengthRequired++;
                sprintf(returnString, "your %s suddenly feels heavier!", armorName);
                equipItem(rogue.armor, true, NULL);
                runicDiscovered = true;
            }
            break;
        case A_VULNERABILITY:
            *damage *= 2;
            if (!runicKnown) {
                sprintf(returnString, "your %s pulses and you are wracked with pain!", armorName);
                runicDiscovered = true;
            }
            break;
        case A_IMMOLATION:
            if (rand_percent(10)) {
                sprintf(returnString, "flames suddenly explode out of your %s!", armorName);
                message(returnString, runicKnown ? 0 : REQUIRE_ACKNOWLEDGMENT);
                returnString[0] = '\0';
                spawnDungeonFeature(player.loc.x, player.loc.y, &(dungeonFeatureCatalog[DF_ARMOR_IMMOLATION]), true, false);
                runicDiscovered = true;
            }
        default:
            break;
    }

    if (runicDiscovered && !runicKnown) {
        autoIdentify(rogue.armor);
    }
}

static void decrementWeaponAutoIDTimer() {
    char buf[COLS*3], buf2[COLS*3];

    if (rogue.weapon
        && !(rogue.weapon->flags & ITEM_IDENTIFIED)
        && !--rogue.weapon->charges) {

        rogue.weapon->flags |= ITEM_IDENTIFIED;
        updateIdentifiableItems();
        messageWithColor("you are now familiar enough with your weapon to identify it.", &itemMessageColor, 0);
        itemName(rogue.weapon, buf2, true, true, NULL);
        sprintf(buf, "%s %s.", (rogue.weapon->quantity > 1 ? "they are" : "it is"), buf2);
        messageWithColor(buf, &itemMessageColor, 0);
    }
}

void processStaggerHit(creature *attacker, creature *defender) {
    if ((defender->info.flags & (MONST_INVULNERABLE | MONST_IMMOBILE | MONST_INANIMATE))
        || (defender->bookkeepingFlags & MB_CAPTIVE)
        || cellHasTerrainFlag(defender->loc, T_OBSTRUCTS_PASSABILITY)) {

        return;
    }
    short newX = clamp(defender->loc.x - attacker->loc.x, -1, 1) + defender->loc.x;
    short newY = clamp(defender->loc.y - attacker->loc.y, -1, 1) + defender->loc.y;
    if (coordinatesAreInMap(newX, newY)
        && !cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)
        && !(pmap[newX][newY].flags & (HAS_MONSTER | HAS_PLAYER))) {

        setMonsterLocation(defender, (pos){ newX, newY });
    }
}

// returns whether the attack hit
boolean attack(creature *attacker, creature *defender, boolean lungeAttack) {
    short damage, specialDamage, poisonDamage;
    char buf[COLS*2], buf2[COLS*2], attackerName[COLS], defenderName[COLS], verb[DCOLS], explicationClause[DCOLS] = "", armorRunicString[DCOLS*3];
    boolean sneakAttack, defenderWasAsleep, defenderWasParalyzed, degradesAttackerWeapon, sightUnseen;

    // Check paladin feat before creatureState is changed
    if (attacker == &player && !(defender->info.flags & MONST_IMMUNE_TO_WEAPONS)) {
        handlePaladinFeat(defender);
    }

    if (attacker == &player && rogue.weapon && rogue.featRecord[FEAT_PURE_MAGE] && canSeeMonster(defender)) {
        rogue.featRecord[FEAT_PURE_MAGE] = false;
    }

    if (attacker->info.abilityFlags & MA_KAMIKAZE) {
        killCreature(attacker, false);
        return true;
    }

    armorRunicString[0] = '\0';

    poisonDamage = 0;

    degradesAttackerWeapon = (defender->info.flags & MONST_DEFEND_DEGRADE_WEAPON ? true : false);

    sightUnseen = !canSeeMonster(attacker) && !canSeeMonster(defender);

    if (defender->status[STATUS_LEVITATING] && (attacker->info.flags & MONST_RESTRICTED_TO_LIQUID)) {
        return false; // aquatic or other liquid-bound monsters cannot attack flying opponents
    }

    if ((attacker == &player || defender == &player) && !rogue.blockCombatText) {
        rogue.disturbed = true;
    }

    defender->status[STATUS_ENTRANCED] = 0;
    if (defender->status[STATUS_MAGICAL_FEAR]) {
        defender->status[STATUS_MAGICAL_FEAR] = 1;
    }

    if (attacker != &player && defender == &player && attacker->creatureState == MONSTER_WANDERING) {
        attacker->creatureState = MONSTER_TRACKING_SCENT;
    }

    if (defender->info.flags & MONST_INANIMATE) {
        sneakAttack = false;
        defenderWasAsleep = false;
        defenderWasParalyzed = false;
    } else {
        sneakAttack = (defender != &player && attacker == &player && (defender->creatureState == MONSTER_WANDERING) ? true : false);
        defenderWasAsleep = (defender != &player && (defender->creatureState == MONSTER_SLEEPING) ? true : false);
        defenderWasParalyzed = defender->status[STATUS_PARALYZED] > 0;
    }

    monsterName(attackerName, attacker, true);
    monsterName(defenderName, defender, true);

    if ((attacker->info.abilityFlags & MA_SEIZES)
        && (!(attacker->bookkeepingFlags & MB_SEIZING) || !(defender->bookkeepingFlags & MB_SEIZED))
        && (distanceBetween(attacker->loc, defender->loc) == 1
            && !diagonalBlocked(attacker->loc.x, attacker->loc.y, defender->loc.x, defender->loc.y, false))) {

        attacker->bookkeepingFlags |= MB_SEIZING;
        defender->bookkeepingFlags |= MB_SEIZED;

        // if the player is seized by a submerged monster they can see (i.e. player is also submerged), 
        // it immediately surfaces so it can be targeted with staffs/wands
        if (defender == &player && (attacker->bookkeepingFlags & MB_SUBMERGED) && canSeeMonster(attacker)) {            
            attacker->bookkeepingFlags &= ~MB_SUBMERGED;
            monsterName(attackerName, attacker, true);
        }

        if (canSeeMonster(attacker) || canSeeMonster(defender)) {
            sprintf(buf, "%s seizes %s!", attackerName, (defender == &player ? "your legs" : defenderName));
            messageWithColor(buf, &white, 0);
        }
        return false;
    }

    if (sneakAttack || defenderWasAsleep || defenderWasParalyzed || lungeAttack || attackHit(attacker, defender)) {
        // If the attack hit:
        damage = (defender->info.flags & (MONST_IMMUNE_TO_WEAPONS | MONST_INVULNERABLE)
                  ? 0 : randClump(attacker->info.damage) * monsterDamageAdjustmentAmount(attacker) / FP_FACTOR);

        if (sneakAttack || defenderWasAsleep || defenderWasParalyzed) {
            if (defender != &player) {
                // The non-player defender doesn't hit back this turn because it's still flat-footed.
                defender->ticksUntilTurn += max(defender->movementSpeed, defender->attackSpeed);
                if (defender->creatureState != MONSTER_ALLY) {
                    defender->creatureState = MONSTER_TRACKING_SCENT; // Wake up!
                }
            }
        }
        if (sneakAttack || defenderWasAsleep || defenderWasParalyzed || lungeAttack) {
            if (attacker == &player
                && rogue.weapon
                && (rogue.weapon->flags & ITEM_SNEAK_ATTACK_BONUS)) {

                damage *= 5; // 5x damage for dagger sneak attacks.
            } else {
                damage *= 3; // Triple damage for general sneak attacks.
            }
        }

        if (defender == &player && rogue.armor && (rogue.armor->flags & ITEM_RUNIC)) {
            applyArmorRunicEffect(armorRunicString, attacker, &damage, true);
        }

        if (attacker == &player
            && rogue.reaping
            && !(defender->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {

            specialDamage = min(damage, defender->currentHP) * rogue.reaping; // Maximum reaped damage can't exceed the victim's remaining health.
            if (rogue.reaping > 0) {
                specialDamage = rand_range(0, specialDamage);
            } else {
                specialDamage = rand_range(specialDamage, 0);
            }
            if (specialDamage) {
                rechargeItemsIncrementally(specialDamage);
            }
        }

        if (damage == 0) {
            sprintf(explicationClause, " but %s no damage", (attacker == &player ? "do" : "does"));
            if (attacker == &player) {
                rogue.disturbed = true;
            }
        } else if (lungeAttack) {
            strcpy(explicationClause, " with a vicious lunge attack");
        } else if (defenderWasParalyzed) {
            sprintf(explicationClause, " while $HESHE %s paralyzed", (defender == &player ? "are" : "is"));
        } else if (defenderWasAsleep) {
            strcpy(explicationClause, " in $HISHER sleep");
        } else if (sneakAttack) {
            strcpy(explicationClause, ", catching $HIMHER unaware");
        } else if (defender->status[STATUS_STUCK] || defender->bookkeepingFlags & MB_CAPTIVE) {
            sprintf(explicationClause, " while %s dangle%s helplessly",
                    (canSeeMonster(defender) ? "$HESHE" : "it"),
                    (defender == &player ? "" : "s"));
        }
        resolvePronounEscapes(explicationClause, defender);

        if ((attacker->info.abilityFlags & MA_POISONS) && damage > 0) {
            poisonDamage = damage;
            damage = 1;
        }

        if (inflictDamage(attacker, defender, damage, &red, false)) { // if the attack killed the defender
            if (defenderWasAsleep || sneakAttack || defenderWasParalyzed || lungeAttack) {
                sprintf(buf, "%s %s %s%s", attackerName,
                        ((defender->info.flags & MONST_INANIMATE) ? "destroyed" : "dispatched"),
                        defenderName,
                        explicationClause);
            } else {
                sprintf(buf, "%s %s %s%s",
                        attackerName,
                        ((defender->info.flags & MONST_INANIMATE) ? "destroyed" : "defeated"),
                        defenderName,
                        explicationClause);
            }
            if (sightUnseen) {
                if (defender->info.flags & MONST_INANIMATE) {
                    combatMessage("you hear something get destroyed in combat", 0);
                } else {
                    combatMessage("you hear something die in combat", 0);
                }
            } else {
                combatMessage(buf, (damage > 0 ? messageColorFromVictim(defender) : &white));
            }
            killCreature(defender, false);
            if (&player == defender) {
                gameOver(attacker->info.monsterName, false);
                return true;
            } else if (&player == attacker
                       && defender->info.monsterID == MK_DRAGON) {

                rogue.featRecord[FEAT_DRAGONSLAYER] = true;
            }
        } else { // if the defender survived
            if (!rogue.blockCombatText && (canSeeMonster(attacker) || canSeeMonster(defender))) {
                attackVerb(verb, attacker, max(damage - (attacker->info.damage.lowerBound * monsterDamageAdjustmentAmount(attacker) / FP_FACTOR), 0) * 100
                           / max(1, (attacker->info.damage.upperBound - attacker->info.damage.lowerBound) * monsterDamageAdjustmentAmount(attacker) / FP_FACTOR));
                sprintf(buf, "%s %s %s%s", attackerName, verb, defenderName, explicationClause);
                if (sightUnseen) {
                    if (!rogue.heardCombatThisTurn) {
                        rogue.heardCombatThisTurn = true;
                        combatMessage("you hear combat in the distance", 0);
                    }
                } else {
                    combatMessage(buf, messageColorFromVictim(defender));
                }
            }
            if (attacker == &player && rogue.weapon && (rogue.weapon->flags & ITEM_ATTACKS_STAGGER)) {
                processStaggerHit(attacker, defender);
            }
            if (attacker->info.abilityFlags & SPECIAL_HIT) {
                specialHit(attacker, defender, (attacker->info.abilityFlags & MA_POISONS) ? poisonDamage : damage);
            }
            if (armorRunicString[0]) {
                message(armorRunicString, 0);
                if (rogue.armor && (rogue.armor->flags & ITEM_RUNIC) && rogue.armor->enchant2 == A_BURDEN) {
                    strengthCheck(rogue.armor, true);
                }
            }
        }

        moralAttack(attacker, defender);
        
        if (attacker == &player && rogue.weapon && (rogue.weapon->flags & ITEM_RUNIC)) {
            magicWeaponHit(defender, rogue.weapon, sneakAttack || defenderWasAsleep || defenderWasParalyzed);
        }

        splitMonster(defender, attacker);

        if (attacker == &player
            && (defender->bookkeepingFlags & MB_IS_DYING)
            && (defender->bookkeepingFlags & MB_WEAPON_AUTO_ID)) {

            decrementWeaponAutoIDTimer();
        }

        if (degradesAttackerWeapon
            && attacker == &player
            && rogue.weapon
            && !(rogue.weapon->flags & ITEM_PROTECTED)
                // Can't damage a Weapon of Acid Mound Slaying by attacking an acid mound... just ain't right!
            && !((rogue.weapon->flags & ITEM_RUNIC) && rogue.weapon->enchant2 == W_SLAYING && monsterIsInClass(defender, rogue.weapon->vorpalEnemy))
            && rogue.weapon->enchant1 >= -10) {

            rogue.weapon->enchant1--;
            if (rogue.weapon->quiverNumber) {
                rogue.weapon->quiverNumber = rand_range(1, 60000);
            }
            equipItem(rogue.weapon, true, NULL);
            itemName(rogue.weapon, buf2, false, false, NULL);
            sprintf(buf, "your %s weakens!", buf2);
            messageWithColor(buf, &itemMessageColor, 0);
            checkForDisenchantment(rogue.weapon);
        }

        return true;
    } else { // if the attack missed
        if (!rogue.blockCombatText) {
            if (sightUnseen) {
                if (!rogue.heardCombatThisTurn) {
                    rogue.heardCombatThisTurn = true;
                    combatMessage("you hear combat in the distance", 0);
                }
            } else {
                sprintf(buf, "%s missed %s", attackerName, defenderName);
                combatMessage(buf, 0);
            }
        }
        return false;
    }
}

// Gets the length of a string without the four-character color escape sequences, since those aren't displayed.
short strLenWithoutEscapes(const char *str) {
    short i, count;

    count = 0;
    for (i=0; str[i];) {
        if (str[i] == COLOR_ESCAPE) {
            i += 4;
            continue;
        }
        count++;
        i++;
    }
    return count;
}

// Buffer messages generated by combat until flushed by displayCombatText().
// Messages in the buffer are delimited by newlines.
void combatMessage(char *theMsg, const color *theColor) {
    short length;
    char newMsg[COLS * 2 - 1]; // -1 for the newline when appending later

    if (theColor == 0) {
        theColor = &white;
    }

    newMsg[0] = '\0';
    encodeMessageColor(newMsg, 0, theColor);
    length = strlen(newMsg);
    strncat(&newMsg[length], theMsg, (COLS * 2 - 1) - length - 1);

    length = strlen(combatText);

    // Buffer combat messages here just for timing; otherwise player combat
    // messages appear after monsters, rather than before.  The -2 is for the
    // newline and terminator.
    if (length + strlen(newMsg) > COLS * 2 - 2) {
        displayCombatText();
    }

    if (combatText[0]) {
        snprintf(&combatText[length], COLS * 2 - length, "\n%s", newMsg);
    } else {
        strcpy(combatText, newMsg);
    }
}

// Flush any buffered, newline-delimited combat messages, passing each to
// message().  These messages are "foldable", meaning that if space permits
// they may be joined together by semi-colons.  Notice that combat messages may
// be flushed by a number of different callers.  One is message() itself
// creating a recursion, which this function is responsible for terminating.
void displayCombatText() {
    char buf[COLS * 2];
    char *start, *end;

    // message itself will call displayCombatText.  For this guard to terminate
    // the recursion, we need to copy combatText out and empty it before
    // calling message.
    if (combatText[0] == '\0') {
        return;
    }

    strcpy(buf, combatText);
    combatText[0] = '\0';

    start = buf;
    for (end = start; *end != '\0'; end++) {
        if (*end == '\n') {
            *end = '\0';
            message(start, FOLDABLE | (rogue.cautiousMode ? REQUIRE_ACKNOWLEDGMENT : 0));
            start = end + 1;
        }
    }

    message(start, FOLDABLE | (rogue.cautiousMode ? REQUIRE_ACKNOWLEDGMENT : 0));

    rogue.cautiousMode = false;
}

void flashMonster(creature *monst, const color *theColor, short strength) {
    if (!theColor) {
        return;
    }
    if (!(monst->bookkeepingFlags & MB_WILL_FLASH) || monst->flashStrength < strength) {
        monst->bookkeepingFlags |= MB_WILL_FLASH;
        monst->flashStrength = strength;
        monst->flashColor = *theColor;
        rogue.creaturesWillFlashThisTurn = true;
    }
}

static boolean canAbsorb(creature *ally, boolean ourBolts[], creature *prey, short **grid) {
    short i;

    if (ally->creatureState == MONSTER_ALLY
        && ally->newPowerCount > 0
        && (!isPosInMap(ally->targetCorpseLoc))
        && !((ally->info.flags | prey->info.flags) & (MONST_INANIMATE | MONST_IMMOBILE))
        && !monsterAvoids(ally, prey->loc)
        && grid[ally->loc.x][ally->loc.y] <= 10) {

        if (~(ally->info.abilityFlags) & prey->info.abilityFlags & LEARNABLE_ABILITIES) {
            return true;
        } else if (~(ally->info.flags) & prey->info.flags & LEARNABLE_BEHAVIORS) {
            return true;
        } else {
            for (i = 0; i < gameConst->numberBoltKinds; i++) {
                ourBolts[i] = false;
            }
            for (i = 0; ally->info.bolts[i] != BOLT_NONE; i++) {
                ourBolts[ally->info.bolts[i]] = true;
            }

            for (i=0; prey->info.bolts[i] != BOLT_NONE; i++) {
                if (!(boltCatalog[prey->info.bolts[i]].flags & BF_NOT_LEARNABLE)
                    && !ourBolts[prey->info.bolts[i]]) {

                    return true;
                }
            }
        }
    }
    return false;
}

static boolean anyoneWantABite(creature *decedent) {
    short candidates, randIndex, i;
    short **grid;
    boolean success = false;
    boolean *ourBolts;

    ourBolts = (boolean *)calloc(gameConst->numberBoltKinds, sizeof(boolean));

    candidates = 0;
    if ((!(decedent->info.abilityFlags & LEARNABLE_ABILITIES)
         && !(decedent->info.flags & LEARNABLE_BEHAVIORS)
         && decedent->info.bolts[0] == BOLT_NONE)
        || (cellHasTerrainFlag(decedent->loc, T_OBSTRUCTS_PASSABILITY))
        || decedent->info.monsterID == MK_SPECTRAL_IMAGE
        || (decedent->info.flags & (MONST_INANIMATE | MONST_IMMOBILE))) {

        return false;
    }

    grid = allocGrid();
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *ally = nextCreature(&it);
        if (ally->creatureState == MONSTER_ALLY) {
            fillGrid(grid, 0);
            calculateDistances(grid, decedent->loc.x, decedent->loc.y, forbiddenFlagsForMonster(&(ally->info)), NULL, true, true);
        }
        if (canAbsorb(ally, ourBolts, decedent, grid)) {
            candidates++;
        }
    }
    if (candidates > 0) {
        randIndex = rand_range(1, candidates);
        creature *firstAlly = NULL;
        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *ally = nextCreature(&it);
            // CanAbsorb() populates ourBolts if it returns true and there are no learnable behaviors or flags:
            if (canAbsorb(ally, ourBolts, decedent, grid) && !--randIndex) {
                firstAlly = ally;
                break;
            }
        }
        if (firstAlly) {
            firstAlly->targetCorpseLoc = decedent->loc;
            strcpy(firstAlly->targetCorpseName, decedent->info.monsterName);
            firstAlly->corpseAbsorptionCounter = 20; // 20 turns to get there and start eating before he loses interest

            // Choose a superpower.
            // First, select from among learnable ability or behavior flags, if one is available.
            candidates = 0;
            for (i=0; i<32; i++) {
                if (Fl(i) & ~(firstAlly->info.abilityFlags) & decedent->info.abilityFlags & LEARNABLE_ABILITIES) {
                    candidates++;
                }
            }
            for (i=0; i<32; i++) {
                if (Fl(i) & ~(firstAlly->info.flags) & decedent->info.flags & LEARNABLE_BEHAVIORS) {
                    candidates++;
                }
            }
            if (candidates > 0) {
                randIndex = rand_range(1, candidates);
                for (i=0; i<32; i++) {
                    if ((Fl(i) & ~(firstAlly->info.abilityFlags) & decedent->info.abilityFlags & LEARNABLE_ABILITIES)
                        && !--randIndex) {

                        firstAlly->absorptionFlags = Fl(i);
                        firstAlly->absorbBehavior = false;
                        success = true;
                        break;
                    }
                }
                for (i=0; i<32 && !success; i++) {
                    if ((Fl(i) & ~(firstAlly->info.flags) & decedent->info.flags & LEARNABLE_BEHAVIORS)
                        && !--randIndex) {

                        firstAlly->absorptionFlags = Fl(i);
                        firstAlly->absorbBehavior = true;
                        success = true;
                        break;
                    }
                }
            } else if (decedent->info.bolts[0] != BOLT_NONE) {
                // If there are no learnable ability or behavior flags, pick a learnable bolt.
                candidates = 0;
                for (i=0; decedent->info.bolts[i] != BOLT_NONE; i++) {
                    if (!(boltCatalog[decedent->info.bolts[i]].flags & BF_NOT_LEARNABLE)
                        && !ourBolts[decedent->info.bolts[i]]) {

                        candidates++;
                    }
                }
                if (candidates > 0) {
                    randIndex = rand_range(1, candidates);
                    for (i=0; decedent->info.bolts[i] != BOLT_NONE; i++) {
                        if (!(boltCatalog[decedent->info.bolts[i]].flags & BF_NOT_LEARNABLE)
                            && !ourBolts[decedent->info.bolts[i]]
                            && !--randIndex) {

                            firstAlly->absorptionBolt = decedent->info.bolts[i];
                            success = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    freeGrid(grid);
    free(ourBolts);
    return success;
}

#define MIN_FLASH_STRENGTH  50

void inflictLethalDamage(creature *attacker, creature *defender) {
    inflictDamage(attacker, defender, defender->currentHP, NULL, true);
}

// returns true if this was a killing stroke; does NOT call killCreature
// flashColor indicates the color that the damage will cause the creature to flash
boolean inflictDamage(creature *attacker, creature *defender,
                      short damage, const color *flashColor, boolean ignoresProtectionShield) {
    dungeonFeature theBlood;
    short transferenceAmount;

    if (damage == 0
        || (defender->info.flags & MONST_INVULNERABLE)) {

        return false;
    }

    if (!ignoresProtectionShield
        && defender->status[STATUS_SHIELDED]) {

        if (defender->status[STATUS_SHIELDED] > damage * 10) {
            defender->status[STATUS_SHIELDED] -= damage * 10;
            damage = 0;
        } else {
            damage -= (defender->status[STATUS_SHIELDED] + 9) / 10;
            defender->status[STATUS_SHIELDED] = defender->maxStatus[STATUS_SHIELDED] = 0;
        }
    }

    defender->bookkeepingFlags &= ~MB_ABSORBING; // Stop eating a corpse if you are getting hurt.

    // bleed all over the place, proportionately to damage inflicted:
    if (damage > 0 && defender->info.bloodType) {
        theBlood = dungeonFeatureCatalog[defender->info.bloodType];
        theBlood.startProbability = (theBlood.startProbability * (15 + min(damage, defender->currentHP) * 3 / 2) / 100);
        if (theBlood.layer == GAS) {
            theBlood.startProbability *= 100;
        }
        spawnDungeonFeature(defender->loc.x, defender->loc.y, &theBlood, true, false);
    }

    if (defender != &player && defender->creatureState == MONSTER_SLEEPING) {
        wakeUp(defender);
    }

    if (defender == &player
        && rogue.easyMode
        && damage > 0) {
        damage = max(1, damage/5);
    }

    if (((attacker == &player && rogue.transference) || (attacker && attacker != &player && (attacker->info.abilityFlags & MA_TRANSFERENCE)))
        && !(defender->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {

        transferenceAmount = min(damage, defender->currentHP); // Maximum transferred damage can't exceed the victim's remaining health.

        if (attacker == &player) {
            transferenceAmount = transferenceAmount * rogue.transference / gameConst->playerTransferenceRatio;
            if (transferenceAmount == 0) {
                transferenceAmount = ((rogue.transference > 0) ? 1 : -1);
            }
        } else if (attacker->creatureState == MONSTER_ALLY) {
            transferenceAmount = transferenceAmount * 4 / 10; // allies get 40% recovery rate
        } else {
            transferenceAmount = transferenceAmount * 9 / 10; // enemies get 90% recovery rate, deal with it
        }

        attacker->currentHP += transferenceAmount;

        if (attacker == &player && player.currentHP <= 0) {
            gameOver("Drained by a cursed ring", true);
            return false;
        }
    }

    if (defender->currentHP <= damage) { // killed
        return true;
    } else { // survived
        if (damage < 0 && defender->currentHP - damage > defender->info.maxHP) {
            defender->currentHP = max(defender->currentHP, defender->info.maxHP);
        } else {
            defender->currentHP -= damage; // inflict the damage!
        }

        if (defender != &player && defender->creatureState != MONSTER_ALLY
            && defender->info.flags & MONST_FLEES_NEAR_DEATH
            && defender->info.maxHP / 4 >= defender->currentHP) {

            defender->creatureState = MONSTER_FLEEING;
        }
        if (flashColor && damage > 0) {
            flashMonster(defender, flashColor, MIN_FLASH_STRENGTH + (100 - MIN_FLASH_STRENGTH) * damage / defender->info.maxHP);
        }
    }

    refreshSideBar(-1, -1, false);
    return false;
}

void addPoison(creature *monst, short durationIncrement, short concentrationIncrement) {
    extern const color poisonColor;
    if (durationIncrement > 0) {
        if (monst == &player && !player.status[STATUS_POISONED]) {
            combatMessage("scalding poison fills your veins", &badMessageColor);
        }
        if (!monst->status[STATUS_POISONED]) {
            monst->maxStatus[STATUS_POISONED] = 0;
        }
        monst->poisonAmount += concentrationIncrement;
        if (monst->poisonAmount == 0) {
            monst->poisonAmount = 1;
        }
        monst->status[STATUS_POISONED] += durationIncrement;
        monst->maxStatus[STATUS_POISONED] = monst->info.maxHP / monst->poisonAmount;

        if (canSeeMonster(monst)) {
            flashMonster(monst, &poisonColor, 100);
        }
    }
}


// Marks the decedent as dying, but does not remove it from the monster chain to avoid iterator invalidation;
// that is done in `removeDeadMonsters`.
// Use "administrativeDeath" if the monster is being deleted for administrative purposes, as opposed to dying as a result of physical actions.
// AdministrativeDeath means the monster simply disappears, with no messages, dropped item, DFs or other effect.
void killCreature(creature *decedent, boolean administrativeDeath) {
    short x, y;
    char monstName[DCOLS], buf[DCOLS * 3];

    if (decedent->bookkeepingFlags & (MB_IS_DYING | MB_HAS_DIED)) {
        // monster has already been killed; let's avoid overkill
        return;
    }

    if (decedent != &player) {
        decedent->bookkeepingFlags |= MB_IS_DYING;
    }

    if (rogue.lastTarget == decedent) {
        rogue.lastTarget = NULL;
    }
    if (rogue.yendorWarden == decedent) {
        rogue.yendorWarden = NULL;
    }

    if (decedent->carriedItem) {
        if (administrativeDeath) {
            deleteItem(decedent->carriedItem);
            decedent->carriedItem = NULL;
        } else {
            makeMonsterDropItem(decedent);
        }
    }

    if (!administrativeDeath && (decedent->info.abilityFlags & MA_DF_ON_DEATH)
        && !(decedent->bookkeepingFlags & MB_IS_FALLING)) {
        spawnDungeonFeature(decedent->loc.x, decedent->loc.y, &dungeonFeatureCatalog[decedent->info.DFType], true, false);

        if (monsterText[decedent->info.monsterID].DFMessage[0] && canSeeMonster(decedent)) {
            monsterName(monstName, decedent, true);
            snprintf(buf, DCOLS * 3, "%s %s", monstName, monsterText[decedent->info.monsterID].DFMessage);
            resolvePronounEscapes(buf, decedent);
            message(buf, 0);
        }
    }

    if (decedent == &player) { // the player died
        // game over handled elsewhere
    } else {
        if (!administrativeDeath
            && decedent->creatureState == MONSTER_ALLY
            && !canSeeMonster(decedent)
            && (!(decedent->info.flags & MONST_INANIMATE) 
                || (monsterCatalog[decedent->info.monsterID].abilityFlags & MA_ENTER_SUMMONS))
            && !(decedent->bookkeepingFlags & MB_BOUND_TO_LEADER)
            && !decedent->carriedMonster) {

            messageWithColor("you feel a sense of loss.", &badMessageColor, 0);
        }
        x = decedent->loc.x;
        y = decedent->loc.y;
        if (decedent->bookkeepingFlags & MB_IS_DORMANT) {
            pmap[x][y].flags &= ~HAS_DORMANT_MONSTER;
        } else {
            pmap[x][y].flags &= ~HAS_MONSTER;
        }

        // This must be done at the same time as removing the HAS_MONSTER flag, or game state might
        // end up inconsistent.
        decedent->bookkeepingFlags |= MB_HAS_DIED;
        if (administrativeDeath) {
            decedent->bookkeepingFlags |= MB_ADMINISTRATIVE_DEATH;
        }

        if (!administrativeDeath && !(decedent->bookkeepingFlags & MB_IS_DORMANT)) {
            // Was there another monster inside?
            if (decedent->carriedMonster) {
                // Insert it into the chain.
                creature *carriedMonster = decedent->carriedMonster;
                decedent->carriedMonster = NULL;
                prependCreature(monsters, carriedMonster);

                carriedMonster->loc.x = x;
                carriedMonster->loc.y = y;
                carriedMonster->ticksUntilTurn = 200;
                pmap[x][y].flags |= HAS_MONSTER;
                fadeInMonster(carriedMonster);

                if (canSeeMonster(carriedMonster)) {
                    monsterName(monstName, carriedMonster, true);
                    sprintf(buf, "%s appears", monstName);
                    combatMessage(buf, NULL);
                }

                applyInstantTileEffectsToCreature(carriedMonster);
            }
            anyoneWantABite(decedent);
            refreshDungeonCell((pos){ x, y });
        }
    }
    decedent->currentHP = 0;
    demoteMonsterFromLeadership(decedent);
    if (decedent->leader) {
        checkForContinuedLeadership(decedent->leader);
    }
}

void buildHitList(const creature **hitList, const creature *attacker, creature *defender, const boolean sweep) {
    short i, x, y, newX, newY, newestX, newestY;
    enum directions dir, newDir;

    x = attacker->loc.x;
    y = attacker->loc.y;
    newX = defender->loc.x;
    newY = defender->loc.y;

    dir = NO_DIRECTION;
    for (i = 0; i < DIRECTION_COUNT; i++) {
        if (nbDirs[i][0] == newX - x
            && nbDirs[i][1] == newY - y) {

            dir = i;
            break;
        }
    }

    if (sweep) {
        if (dir == NO_DIRECTION) {
            dir = UP; // Just pick one.
        }
        for (i=0; i<8; i++) {
            newDir = (dir + i) % DIRECTION_COUNT;
            newestX = x + cDirs[newDir][0];
            newestY = y + cDirs[newDir][1];
            if (coordinatesAreInMap(newestX, newestY) && (pmap[newestX][newestY].flags & (HAS_MONSTER | HAS_PLAYER))) {
                defender = monsterAtLoc((pos){ newestX, newestY });
                if (defender
                    && monsterWillAttackTarget(attacker, defender)
                    && (!cellHasTerrainFlag(defender->loc, T_OBSTRUCTS_PASSABILITY) || (defender->info.flags & MONST_ATTACKABLE_THRU_WALLS))) {

                    hitList[i] = defender;
                }
            }
        }
    } else {
        hitList[0] = defender;
    }
}
