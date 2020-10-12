/*
 *  PowerTables.c
 *  Brogue
 *
 *  Created by Brian Walker on 4/9/17.
 *  Copyright 2017. All rights reserved.
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

#define LAST_INDEX(a) (sizeof(a) / sizeof(fixpt) - 1)

// As of v1.7.5, Brogue does not use floating-point math in any calculations
// that have an effect on substantive gameplay. The two operations that were
// annoying to convert were sqrt() (handled by an open source fixed point sqrt
// implementation in Sqrt.c) and especially pow(). I could not find a fixed point
// pow implementation that was good enough for the wide range of fractional bases
// and exponents. Fortunately, all uses of pow() involved a fixed base and an exponent
// that varied by increments of at least 0.25 (or really 0.1 for armor calculations but
// I fudged that), or a fixed exponent and base that varied similarly. The one exception
// were runic weapon activation chances, which modified the base by the damage of the weapon
// and varied the exponent by the level, but I moved the damage modification into the exponent
// without much affecting the results. So now pow() has been replaced by lookup tables.
// Hopefully this will help with out of sync errors for saved games and recordings...

// game data formulae:

short wandDominate(creature *monst)             { return (((monst)->currentHP * 5 < (monst)->info.maxHP) ? 100 :
                                                    max(0, 100 * ((monst)->info.maxHP - (monst)->currentHP) / (monst)->info.maxHP)); }

short staffDamageLow(fixpt enchant)             { return fp_round((enchant + fp(2)) * 3/4); } // (enchant + 2) * 3/4
short staffDamageHigh(fixpt enchant)            { return fp_round(enchant * 5/2) + 4; } // (enchant * 5/2) + 4
short staffDamage(fixpt enchant)                { return randClumpedRange(staffDamageLow(enchant), staffDamageHigh(enchant), 1 + fp_round(enchant / 3)); }
short staffBlinkDistance(fixpt enchant)         { return fp_round(enchant * 2) + 2; } // (enchant * 2) + 2
short staffHasteDuration(fixpt enchant)         { return fp_round(enchant * 4) + 2; } // (enchant * 4) + 2
short staffBladeCount(fixpt enchant)            { return fp_round(enchant * 3/2); } // enchant * 3/2
short staffDiscordDuration(fixpt enchant)       { return fp_round(enchant * 4); } // enchant * 4
short staffEntrancementDuration(fixpt enchant)  { return fp_round(enchant * 3); } // enchant * 3
int   staffProtection(fixpt enchant)            { return fp_round(fp_ipow(fp_ratio(14, 10), fp_round(enchant) - 2) * 130); } // (1.4 ^ (round(enchant) - 2)) * 130
int   staffPoison(fixpt enchant)                { return fp_round(fp_ipow(fp_ratio(13, 10), fp_round(enchant) - 2) * 5); } // (1.3 ^ (round(enchant) - 2)) * 5

fixpt ringWisdomMultiplier(fixpt enchant)       { return fp_ipow(fp_ratio(13, 10), min(27, fp_round(enchant))); } // 1.3 ^ min(27, round(enchant))

short charmHealing(fixpt enchant)               { return clamp(fp_round(enchant * 20), 0, 100); } // (enchant * 20), between 0 and 100
short charmShattering(fixpt enchant)            { return fp_round(enchant) + 4; } // enchant + 4
short charmGuardianLifespan(fixpt enchant)      { return fp_round(enchant * 2) + 4; } // (enchant * 2) + 4
short charmNegationRadius(fixpt enchant)        { return fp_round(enchant * 3) + 1; } // (enchant * 3) + 1
int   charmProtection(fixpt enchant)            { return fp_round(fp_ipow(fp_ratio(135, 100), fp_round(enchant) - 1) * 150); } // (1.35 ^ (round(enchant) - 1)) * 150

short weaponParalysisDuration(fixpt enchant)    { return max(2, fp_round(enchant / 2) + 2); } // (enchant / 2) + 2, minimum 2
short weaponConfusionDuration(fixpt enchant)    { return max(3, fp_round(enchant * 3/2)); } // (enchant * 3/2), minimum 3
short weaponForceDistance(fixpt enchant)        { return max(4, fp_round(enchant * 2) + 2); } // (enchant * 2) + 2, minimum 4; related to staffBlinkDistance()
short weaponSlowDuration(fixpt enchant)         { return max(3, fp_round(fp_ipow(enchant + fp(2), 2) / 3)); } // (enchant + 2)^2 / 3, minimum 3
short weaponImageCount(fixpt enchant)           { return clamp(fp_round(enchant / 3), 1, 7); } // (enchant / 3), between 1 and 7
short weaponImageDuration(fixpt enchant)        { return 3; } // constant

short armorReprisalPercent(fixpt enchant)       { return max(5, fp_round(enchant * 5)); } // (enchant * 5), minimum 5
short armorAbsorptionMax(fixpt enchant)         { return max(1, fp_round(enchant)); } // enchant, minimum 1
short armorImageCount(fixpt enchant)            { return clamp(fp_round(enchant / 3), 1, 5); } // (enchant / 3), between 1 and 5

short reflectionChance(fixpt enchant) {
    // 100 - 100 * (0.85 ^ enchant), between 1 and 100
    enchant = fp_quantize(enchant, 4); // in 0.25 increments
    return clamp(100 - fp_round(100 * fp_pow(fp_ratio(85, 100), enchant)), 1, 100);
}

long turnsForFullRegenInThousandths(fixpt bonus) {
    // (0.75 ^ round(bonus)) * 1000 * TURNS_FOR_FULL_REGEN + 2000
    fixpt t = fp_ipow(fp_ratio(75, 100), fp_round(bonus));
    return fp_round(t * 1000 * TURNS_FOR_FULL_REGEN) + 2000;
}

fixpt damageFraction(fixpt netEnchant) {
    // 1.065 ^ netEnchant
    netEnchant = fp_quantize(netEnchant, 4); // in 0.25 increments
    return fp_pow(fp_ratio(1065, 1000), netEnchant);
}

fixpt accuracyFraction(fixpt netEnchant) {
    // 1.065 ^ netEnchant
    netEnchant = fp_quantize(netEnchant, 4); // in 0.25 increments
    return fp_pow(fp_ratio(1065, 1000), netEnchant);
}

fixpt defenseFraction(fixpt netDefense) {
    // 0.877347 ^ (netDefense / 10)
    fixpt expn = netDefense / 10;
    expn = fp_quantize(expn, 4); // in 0.25 increments
    return fp_pow(fp_ratio(701, 799), expn);
}

short charmEffectDuration(short charmKind, short enchant) {
    const short duration[NUMBER_CHARM_KINDS] = {
        3,  // Health
        20, // Protection
        7,  // Haste
        10, // Fire immunity
        5,  // Invisibility
        25, // Telepathy
        10, // Levitation
        0,  // Shattering
        18, // Guardian
        0,  // Teleportation
        0,  // Recharging
        0,  // Negation
    };
    const fixpt increment[NUMBER_CHARM_KINDS] = {
        0,    // Health
        0,    // Protection
        fp_ratio(120, 100), // Haste
        fp_ratio(125, 100), // Fire immunity
        fp_ratio(120, 100), // Invisibility
        fp_ratio(125, 100), // Telepathy
        fp_ratio(125, 100), // Levitation
        0,    // Shattering
        0,    // Guardian
        0,    // Teleportation
        0,    // Recharging
        0,    // Negation
    };
    return fp_round(duration[charmKind] * fp_ipow(increment[charmKind], enchant));
}

short charmRechargeDelay(short charmKind, short enchant) {
    const short duration[NUMBER_CHARM_KINDS] = {
        2500,   // Health
        1000,   // Protection
        800,    // Haste
        800,    // Fire immunity
        800,    // Invisibility
        800,    // Telepathy
        800,    // Levitation
        2500,   // Shattering
        700,    // Guardian
        920,    // Teleportation
        10000,  // Recharging
        2500,   // Negation
    };

    const fixpt base[NUMBER_CHARM_KINDS] = {
        fp_ratio(55, 100), // Health
        fp_ratio(60, 100), // Protection
        fp_ratio(65, 100), // Haste
        fp_ratio(60, 100), // Fire immunity
        fp_ratio(65, 100), // Invisibility
        fp_ratio(65, 100), // Telepathy
        fp_ratio(65, 100), // Levitation
        fp_ratio(60, 100), // Shattering
        fp_ratio(70, 100), // Guardian
        fp_ratio(60, 100), // Teleportation
        fp_ratio(55, 100), // Recharging
        fp_ratio(60, 100), // Negation
    };

    short delay = charmEffectDuration(charmKind, enchant)
        + fp_round(duration[charmKind] * fp_ipow(base[charmKind], enchant));
    return max(1, delay);
}

short runicWeaponChance(item *theItem, boolean customEnchantLevel, fixpt enchantLevel) {
    const fixpt effectChances[NUMBER_WEAPON_RUNIC_KINDS] = {
        fp_ratio(84, 100), // W_SPEED
        fp_ratio(94, 100), // W_QUIETUS
        fp_ratio(93, 100), // W_PARALYSIS
        fp_ratio(85, 100), // W_MULTIPLICITY
        fp_ratio(86, 100), // W_SLOWING
        fp_ratio(89, 100), // W_CONFUSION
        fp_ratio(85, 100), // W_FORCE
        0,    // W_SLAYING
        0,    // W_MERCY
        0};   // W_PLENTY

    short runicType = theItem->enchant2;
    short chance, adjustedBaseDamage;

    if (runicType == W_SLAYING) return 0;
    if (runicType >= NUMBER_GOOD_WEAPON_ENCHANT_KINDS) return 15; // bad runic
    if (!customEnchantLevel) enchantLevel = netEnchant(theItem);

    // Innately high-damage weapon types are less likely to trigger runic effects.
    adjustedBaseDamage = (tableForItemCategory(theItem->category, NULL)[theItem->kind].range.lowerBound
                          + tableForItemCategory(theItem->category, NULL)[theItem->kind].range.upperBound) / 2;

    if (theItem->flags & ITEM_ATTACKS_STAGGER) {
        adjustedBaseDamage /= 2; // Normalize as though they attacked once per turn instead of every other turn.
    }

    fixpt modifier = FP_ONE - min(fp_ratio(99, 100), fp(adjustedBaseDamage) / 18);

    if (enchantLevel < 0) {
        chance = 0;
    } else {
        fixpt expn = clamp(fp_mul(enchantLevel, modifier), fp(0), fp(50));
        expn = fp_quantize(expn, 4); // in 0.25 increments
        chance = 100 - fp_round(100 * fp_pow(effectChances[runicType], expn)); // good runic
    }

    // Slow weapons get an adjusted chance of 1 - (1-p)^2 to reflect two bites at the apple instead of one.
    if (theItem->flags & ITEM_ATTACKS_STAGGER) {
        chance = 100 - (100 - chance) * (100 - chance) / 100;
    }
    // Fast weapons get an adjusted chance of 1 - sqrt(1-p) to reflect one bite at the apple instead of two.
    if (theItem->flags & ITEM_ATTACKS_QUICKLY) {
        chance = fp_round(100 * (FP_ONE - fp_sqrt(FP_ONE - fp_ratio(chance, 100))));
    }

    // The lowest percent change that a weapon will ever have is its enchantment level (if greater than 0).
    // That is so that even really heavy weapons will improve at least 1% per enchantment.
    chance = clamp(chance, max(1, fp_round(enchantLevel)), 100);

    return chance;
}
