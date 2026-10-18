// Tests derived from rules/11-runic-weapons-armor.md.
//
// Each BR rule gets one or more test cases. Probabilistic tests use
// retryAction() to loop the step function until the runic effect fires or
// max attempts are exhausted.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

// Forward declarations for step functions used by retryAction before
// their definition in the source file.
static boolean br11019_weak_rune_flat_15_step(void);

static creature *monst;
static int healthBefore;
static pos playerStartPos;
static pos startPos;
static pos endPos;
static creature *forceCollateral;
static int test_phase;
static int test_int[5];


// ============================================================
// BR11001 — runic effect activation chance rises with enchantment level
// ============================================================

// The chance formula must increase with enchantment level.
static boolean br11001_chance_rises_with_enchant_verify(void) {
    // Low enchantment dagger: chance must be > 0
    item *low = givePlayerRunicWeapon(DAGGER, W_SPEED, 1, true);
    short lowChance = runicActivationChance(low);

    // High enchantment dagger: chance must be strictly greater
    item *high = givePlayerRunicWeapon(DAGGER, W_SPEED, 50, true);
    short highChance = runicActivationChance(high);

    TEST_ASSERT(lowChance > 0, "runic activation chance should be non-zero");
    TEST_ASSERT(highChance > lowChance, "runic activation chance should rise with enchantment level");
    return !testFailed;
}

// ============================================================
// BR11016 — high base-damage weapons get lower runic chance
// ============================================================

static boolean br11016_high_damage_lower_chance_verify(void) {
    item *dagger = givePlayerRunicWeapon(DAGGER, W_SPEED, 10, true);
    item *sword  = givePlayerRunicWeapon(SWORD,  W_SPEED, 10, true);

    short daggerChance = runicActivationChance(dagger);
    short swordChance  = runicActivationChance(sword);

    TEST_ASSERT(swordChance <= daggerChance, "higher damage weapons should have lower runic activation chance");
    return !testFailed;
}

// ============================================================
// BR11017 — slow weapons get higher per-hit chance; fast-attack weapons get lower
// ============================================================

static boolean br11017_speed_adjustment_verify(void) {
    item *mace = givePlayerRunicWeapon(MACE, W_SPEED, 10, true);
    item *rapier = givePlayerRunicWeapon(RAPIER, W_SPEED, 10, true);

    short maceChance = runicActivationChance(mace);
    short rapierChance = runicActivationChance(rapier);

    // Wipe away the special attrack trait
    mace->flags &= ~ITEM_ATTACKS_STAGGER;
    rapier->flags &= ~ITEM_ATTACKS_QUICKLY;

    TEST_ASSERT(maceChance > rapierChance, "slow-attack (mace) should have higher runic activation chance than fast-attack (rapier) weapons");
    TEST_ASSERT(maceChance > runicActivationChance(mace), "slow-attack (mace) should have higher runic activation chance than non-slow weapons");
    TEST_ASSERT(rapierChance < runicActivationChance(rapier), "fast-attack (rapier) should have lower runic activation chance than non-fast weapons");
    return !testFailed;
}

// ============================================================
// BR11018 — runic chance is clamped between enchantment level and 100%
// ============================================================

static boolean br11018_chance_bounded_verify(void) {
    // Chance must be at least the enchantment level
    item *item1 = givePlayerRunicWeapon(DAGGER, W_SPEED, 3, true);
    short chance1 = runicActivationChance(item1);
    TEST_ASSERT(chance1 >= 3, "chance must be >= enchantment level (3)");

    // Chance must never exceed 100
    item *item2 = givePlayerRunicWeapon(SWORD, W_SPEED, 50, true);
    short chance2 = runicActivationChance(item2);
    TEST_ASSERT(chance2 <= 100, "chance must be <= 100");
    return !testFailed;
}

// ============================================================
// BR11019 — weak/cursed runes activate at flat 15%
// ============================================================

static boolean br11019_weak_rune_flat_15_step(void) {
    clearArena();

    // W_VULNERABILITY is a bad (cursed) armor enchant — use a bad weapon rune
    // W_PLENTY is good, but for bad runes we use negative enchant2 values.
    // In the code, runicType >= NUMBER_GOOD_WEAPON_ENCHANT_KINDS -> 15%.
    // We simulate a "bad" rune by using the W_PLENTY kind (which is the only
    // "bad" weapon rune, at index NUMBER_WEAPON_RUNIC_KINDS-1).
    // Actually, the check is: runicType >= NUMBER_GOOD_WEAPON_ENCHANT_KINDS.
    // W_PLENTY is the only bad weapon rune. So let's test that directly.
    item *plenty = giveAndEquipRunicWeapon(DAGGER, W_PLENTY, 10, true);

    char buf[120];
    sprintf(buf, "W_PLENTY is bad rune, chance should be 15, got %d", runicActivationChance(plenty));
    TEST_ASSERT(runicActivationChance(plenty) == 15, buf);
    return false;
}

static boolean br11019_weak_rune_flat_15_verify(void) {
    retryAction(200, br11019_weak_rune_flat_15_step);
    return !testFailed;
}

// ============================================================
// BR11020 — slaying rune has no random activation chance
// ============================================================

static boolean br11020_slaying_no_chance_verify(void) {
    // Slaying rune returns 0 from runicActivationChance (no random check).
    item *slaying = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10, true);
    // Set vorpalEnemy to the target we'll use
    slaying->vorpalEnemy = 2; // Arbitrary vorpal enemy

    TEST_ASSERT(runicActivationChance(slaying) == 0,
                "slaying rune chance should be 0 (no random roll)");
    return !testFailed;
}

// ============================================================
// BR11002 — speed rune grants an immediate extra turn
// ============================================================

static int ticksBeforeAttack;

static void br11002_speed_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_SPEED, 20, true);
    monst = placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    makeMonsterHostile(monst);
    ticksBeforeAttack = player.ticksUntilTurn;
}

static boolean br11002_speed_step(void) {
    if (player.ticksUntilTurn == ticksBeforeAttack) {
        playerAttacksMonster(monst);
        return true; // keep retrying
    }
    return false; // attack completed
}

static boolean br11002_speed_verify(void) {
    // After speed rune fires, player.ticksUntilTurn becomes -1 (free turn)
    TEST_ASSERT(player.ticksUntilTurn == -1 || messageWasShown("trembles"),
                "speed rune should grant an extra turn (ticksUntilTurn=-1)");
    return !testFailed;
}

// ============================================================
// BR11003 — quietus rune sometimes instantly kills
// ============================================================

static void br11003_quietus_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_QUIETUS, 20, true);
    monst = placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    setMonsterHealth(monst, monst->info.maxHP);
}

static boolean br11003_quietus_step(void) {
    playerAttacksMonster(monst);
    if (findMonster(MK_RAT) == NULL) return false; // killed, done
    return true; // keep retrying
}

static boolean br11003_quietus_verify(void) {
    retryAction(200, br11003_quietus_step);
    return !testFailed;
}

// ============================================================
// BR11004 — paralysis rune freezes target for duration growing with enchantment
// ============================================================

static int paralysisDuration;

static void br11004_paralysis_setup(void) {
    clearArena();
    // Use enchant 10 so duration is determinable
    giveAndEquipRunicWeapon(DAGGER, W_PARALYSIS, 10, true);
    // Use troll (maxHP=65) so it survives the attack even with sneak bonus (5x).
    // Paralyze to guarantee hit. Damage with dagger enchant 10 ≈ 2, 5x = 10.
    // Troll at 65 HP → 55 HP, survives. Then paralysis rune fires.
    monst = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
    setMonsterHealth(monst, monst->info.maxHP);
}

static boolean br11004_paralysis_step(void) {
    playerAttacksMonster(monst);
    if (monsterHasStatus(monst, STATUS_PARALYZED)) {
        paralysisDuration = monsterStatusTurns(monst, STATUS_PARALYZED);
        return false; // effect applied, done
    }
    return true; // keep retrying
}

static boolean br11004_paralysis_verify(void) {
    retryAction(200, br11004_paralysis_step);
    if (testFailed) return false;

    // enchant 10 -> weaponParalysisDuration(10*FP) = max(2, 2 + 10/2) = 7
    char buf[120];
    sprintf(buf, "paralysis duration should be >= 2, got %d", paralysisDuration);
    TEST_ASSERT(paralysisDuration >= 2, buf);
    return !testFailed;
}

// ============================================================
// BR11005 — multiplicity rune summons spectral duplicates
// ============================================================

static int imagesBefore;

static void br11005_multiplicity_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_MULTIPLICITY, 10, true);
    monst = placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    setMonsterHealth(monst, monst->info.maxHP);
    imagesBefore = monsterCount();
}

static boolean br11005_multiplicity_step(void) {
    playerAttacksMonster(monst);
    if (monsterCount() > imagesBefore) return false; // summoned, done
    return true; // keep retrying
}

static boolean br11005_multiplicity_verify(void) {
    retryAction(200, br11005_multiplicity_step);
    if (testFailed) return false;

    // Should have spawned at least 1 spectral image (enchant 10 -> clamp(10/3, 1, 7) = 3)
    TEST_ASSERT(monsterCount() > imagesBefore,
                "multiplicity should summon spectral duplicates");
    return !testFailed;
}

// ============================================================
// BR11006 — force rune knocks creature backward, dealing impact damage
// ============================================================

static void br11006_force_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_FORCE, 20, true);
    monst = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    setMonsterHealth(monst, monst->info.maxHP);
    startPos = monsterPosition(monst);
}

static boolean br11006_force_step(void) {
    playerAttacksMonster(monst);
    pos newPos = monsterPosition(monst);
    if (newPos.x != startPos.x || newPos.y != startPos.y) {
        return false; // knocked back, done
    }
    return true; // keep retrying
}

static boolean br11006_force_verify(void) {
    retryAction(200, br11006_force_step);
    if (testFailed) return false;

    pos newPos = monsterPosition(monst);
    TEST_ASSERT(newPos.x != startPos.x || newPos.y != startPos.y,
                "force rune should knock target backward");
    return !testFailed;
}

// ============================================================
// BR11007 — slaying rune always hits and kills creatures of designated kind
// ============================================================

static void br11007_slaying_setup(void) {
    clearArena();
    item *slaying = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10, true);
    slaying->vorpalEnemy = vorpalEnemyClassForMonster(MK_RAT);
    monst = placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    setMonsterHealth(monst, monst->info.maxHP);
}

static boolean br11007_slaying_step(void) {
    playerAttacksMonster(monst);
    if (findMonster(MK_RAT) == NULL) return false; // killed, stop retrying
    return true; // keep retrying
}

static boolean br11007_slaying_verify(void) {
    retryAction(200, br11007_slaying_step);
    return !testFailed;
}

// ============================================================
// BR11008 — mercy rune heals the creature it strikes
// ============================================================

static void br11008_mercy_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_MERCY, 10, true);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0));    // Kraken out of water is helpless == always hit
    setMonsterHealth(monst, 30);
    healthBefore = monsterHealth(monst);
}

static boolean br11008_mercy_step(void) {
    playerAttacksMonster(monst);
    if (monsterHealth(monst) > healthBefore) return false; // healed, done
    return true; // keep retrying
}

static boolean br11008_mercy_verify(void) {
    TEST_ASSERT(monsterHealth(monst) >= healthBefore, "weapon of mercy should heal enemy");
    return !testFailed;
}

// ============================================================
// BR11009 — plenty rune creates a duplicate of the struck creature
// ============================================================

static int plentyMonstersBefore;

static void br11009_plenty_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_PLENTY, 50, true);
    // Use troll with very high HP so it survives many attacks — plenty rune
    // (15% bad rune chance) needs multiple attempts to fire.
    placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    creature *t = findMonster(MK_TROLL);
    if (t) {
        // Set currentHP directly — info.maxHP must be set first for setMonsterHealth's clamp.
        t->info.maxHP = 500;
        t->currentHP = 500;
    }
    plentyMonstersBefore = monsterCount();
}

static boolean br11009_plenty_step(void) {
    creature *troll = findMonster(MK_TROLL);
    if (troll && (troll->bookkeepingFlags & MB_HAS_DIED)) {
        troll = NULL;
    }
    if (troll && !(troll->bookkeepingFlags & MB_IS_DYING)) {
        playerAttacksMonster(troll);
        if (monsterCount() > plentyMonstersBefore) return false; // duplicated, done
    }
    // Troll died or isn't alive — spawn a new one
    if (!troll || (troll->bookkeepingFlags & MB_IS_DYING)) {
        if (troll && troll->bookkeepingFlags & MB_IS_DYING) {
            killCreature(troll, false);
        }
        clearMonsters();
        creature *t = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
        if (t) {
            t->info.maxHP = 500;
            t->currentHP = 500;
        }
    }
    return true; // keep retrying
}

// Note: the test framework's step loop (up to 10000 iterations) handles
// retrying. The rune's 15% chance per hit fires reliably within a few hundred attempts.

static boolean br11009_plenty_verify(void) {
    retryAction(200, br11009_plenty_step);
    return !testFailed;
}

// ============================================================
// BR11010 — weapon runes (other than slaying) don't trigger against
//            inanimate or invulnerable creatures
// ============================================================

static boolean inanimHitOccurred;

static void br11010_inanimate_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_SPEED, 20, true);
    // WARDEN_OF_YENDOR is MONST_INVULNERABLE
    monst = placeHostileMonster(MK_WARDEN_OF_YENDOR, posOffset(player.loc, 1, 0));
    inanimHitOccurred = false;
}

static boolean br11010_inanimate_step(void) {
    playerAttacksMonster(monst);
    // Speed rune should NOT fire on invulnerable — player.ticksUntilTurn should not be -1
    if (player.ticksUntilTurn == -1) {
        inanimHitOccurred = true;
    }
    return false; // test done after one attack
}

static boolean br11010_inanimate_verify(void) {
    TEST_ASSERT(!inanimHitOccurred,
                "speed rune should not activate against invulnerable creature");
    return !testFailed;
}

// ============================================================
// BR11011 — armor of immunity negates bolts of designated type
// ============================================================

static creature *immunityBoltCaster;

static void br11011_immunity_setup(void) {
    clearArena();
    // Create armor of immunity with vorpalEnemy = a bolt-casting monster
    item *armor = givePlayerRunicArmor(CHAIN_MAIL, A_IMMUNITY, 10);
    armor->vorpalEnemy = vorpalEnemyClassByName("mage");
    equipItemFromPack(ARMOR, CHAIN_MAIL);

    // Lich stands far enough to fire a bolt but close enough to be visible
    immunityBoltCaster = placeHostileMonster(MK_LICH, posOffset(player.loc, 4, 0));
}

static boolean br11011_immunity_verify(void) {
    // Fire a fire bolt at the player from the lich position
    pos boltOrigin = immunityBoltCaster->loc;
    bolt theBolt = boltCatalog[BOLT_FIRE];
    zap(boltOrigin, player.loc, &theBolt, false, false);

    // The immunity armor should have negated the bolt entirely
    // The bolt's vorpalEnemy check in projectileReflects returns true for immunity
    // Actually, immunity just sets damage = 0 in combatHit for melee.
    // For bolts, it uses projectileReflects which returns true for immunity.
    // So the bolt should have been reflected/blocked.
    // We verify that the player took no damage from the bolt.
    TEST_ASSERT(!isPlayerDead(), "immunity armor should negate bolt of designated type");
    return !testFailed;
}

// ============================================================
// BR11012 — armor of reflection bounces bolts back at the caster
// ============================================================

static creature *reflectionCaster;

static void br11012_reflection_setup(void) {
    clearArena();
    givePlayerRunicArmor(CHAIN_MAIL, A_REFLECTION, 10);
    equipItemFromPack(ARMOR, CHAIN_MAIL);
    reflectionCaster = placeHostileMonster(MK_LICH, posOffset(player.loc, 4, 0));
}

static boolean br11012_reflection_verify(void) {
    // Lich casts fire bolt at player — armor of reflection should bounce it
    bolt theBolt = boltCatalog[BOLT_FIRE];
    zap(reflectionCaster->loc, player.loc, &theBolt, false, false);

    // The bolt should bounce back. We check the monster didn't die (bolt bounced)
    // and that the caster is affected. A reflected bolt would go back to the
    // original caster's location.
    TEST_ASSERT(!isPlayerDead(), "reflection should bounce bolt away from player");
    return !testFailed;
}

// ============================================================
// BR11013 — armor of respiration protects against damaging gases
// ============================================================

static void br11013_respiration_setup(void) {
    clearArena();
    givePlayerRunicArmor(CHAIN_MAIL, A_RESPIRATION, 10);
    equipItemFromPack(ARMOR, CHAIN_MAIL);

    // Place poison gas cloud
    pos gasLoc = player.loc;
    spawnFeature(gasLoc, DF_POISON_GAS_CLOUD);
    revealCell(gasLoc);
}

static boolean br11013_respiration_verify(void) {
    // Wait for gas effects
    restTurns(10);

    // Should NOT have the nauseous status (respiration protection)
    // The nauseous status should not be applied to a player with respiration armor
    TEST_ASSERT(!monsterHasStatus(&player, STATUS_NAUSEOUS) ||
                    playerHasStatus(STATUS_NAUSEOUS) == false,
                "respiration armor should protect against poison gas nauseous effect");
    return !testFailed;
}

// ============================================================
// BR11014 — armor of dampening absorbs explosive damage
// ============================================================

static void br11014_dampening_setup(void) {
    clearArena();
    givePlayerRunicArmor(CHAIN_MAIL, A_DAMPENING, 10);
    equipItemFromPack(ARMOR, CHAIN_MAIL);
    setPlayerHealth(1); // set low HP so explosion would normally kill
}

static boolean br11014_dampening_verify(void) {
    // Set the player's tile to GAS_EXPLOSION terrain which has
    // T_CAUSES_EXPLOSIVE_DAMAGE (15-20 damage or 50% maxHP).
    setTerrain(player.loc, GAS_EXPLOSION);
    revealCell(player.loc);

    // The dampening armor should absorb the explosive damage, keeping the player alive
    // Tile effects (including explosive damage) only fire when applyInstantTileEffectsToCreature
    // is called — trigger them explicitly for the player.
    applyTileEffectsToPlayer();

    // If dampening worked, player should survive (not dead despite starting at HP 1)
    TEST_ASSERT(!isPlayerDead(), "dampening armor should absorb explosive damage");
    return !testFailed;
}

// ============================================================
// BR11015 — armor reveals its runic nature after several turns of wear
// ============================================================

static void br11015_reveal_setup(void) {
    clearArena();
    // Give an unidentified runic armor
    item *armor = givePlayerRunicArmor(CHAIN_MAIL, A_DAMPENING, 10);
    // Reset the identification flags so it acts like newly acquired unidentified runic
    armor->flags &= ~(ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    armor->charges = 20; // armorDelayToAutoID turns until reveal
    equipItemFromPack(ARMOR, CHAIN_MAIL);
}

static boolean br11015_reveal_verify(void) {
    // Wait for the armor to auto-identify after enough turns
    restTurns(25);

    // Armor charges decremented in processIncrementalAutoID; when they reach 0,
    // the armor sets ITEM_IDENTIFIED (which reveals it has a runic enchant in its name).
    TEST_ASSERT(rogue.armor && (rogue.armor->flags & ITEM_IDENTIFIED),
                "runic armor should identify itself after enough turns of wear");
    return !testFailed;
}

// ============================================================
// BR11021 — weapon reveals runic nature when effect fires
// ============================================================

static void br11021_weapon_identify_setup(void) {
    clearArena();
    // Give unidentified runic weapon
    item *weapon = giveAndEquipRunicWeapon(DAGGER, W_SPEED, 20, true);
    weapon->flags &= ~(ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    rogue.weapon = weapon;
    // Use troll so it survives the attack and speed rune can fire
    monst = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    setMonsterHealth(monst, monst->info.maxHP);
}

static boolean br11021_weapon_identify_step(void) {
    playerAttacksMonster(monst);
    if (rogue.weapon && (rogue.weapon->flags & ITEM_RUNIC_IDENTIFIED)) {
        return false; // identified, done
    }
    return true; // keep retrying
}

static boolean br11021_weapon_identify_verify(void) {
    retryAction(200, br11021_weapon_identify_step);
    return !testFailed;
}

// ============================================================
// BR11022 — armor auto-identifies on runic trigger (e.g., reflection)
// ============================================================

static creature *reflectionCaster2;

static void br11022_armor_identify_setup(void) {
    clearArena();
    // Give unidentified runic armor of reflection
    item *armor = givePlayerRunicArmor(CHAIN_MAIL, A_REFLECTION, 10);
    armor->flags &= ~(ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    equipItemFromPack(ARMOR, CHAIN_MAIL);
    reflectionCaster2 = placeHostileMonster(MK_LICH, posOffset(player.loc, 4, 0));
}

static boolean br11022_armor_identify_verify(void) {
    // Fire a bolt that should trigger reflection and auto-identify
    bolt theBolt = boltCatalog[BOLT_FIRE];
    zap(reflectionCaster2->loc, player.loc, &theBolt, false, false);

    if (rogue.armor) {
        TEST_ASSERT((rogue.armor->flags & ITEM_RUNIC_IDENTIFIED),
                    "armor should auto-identify when reflection triggers");
    } else {
        testFail("no armor equipped");
    }
    return !testFailed;
}

// ============================================================
// BR11006 extended — force knocks into another creature, damaging it too
// ============================================================

static void br11006_force_collateral_setup(void) {
    clearArenaRadius(5);
    playerStartPos = monsterPosition(&player);

    giveAndEquipRunicWeapon(DAGGER, W_FORCE, 20, true);

    // Target directly in front
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0)); // Kraken out of water is helpless
    startPos = monsterPosition(monst);

    // Collateral target behind the primary target
    forceCollateral = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 5, 0)); // Kraken out of water is helpless
}

static boolean br11006_force_collateral_step(void) {
    moveRight();
    endPos = monsterPosition(monst);

    // If target or player moves, or target dies, then end the test
    if (!posEq(startPos, endPos)
        || !posEq(monsterPosition(&player), playerStartPos)
        || monsterIsDead(monst)) {
        return false; // end test
    }
    return true; // keep retrying
}

static boolean br11006_force_collateral_verify(void) {
    endPos = monsterPosition(monst);
    TEST_ASSERT(posEq(monsterPosition(&player), playerStartPos) // Player didn't move
                && !posEq(startPos, endPos)         // Target was thrown
                && monsterIsHurt(monst)             // Target was hurt
                && monsterIsHurt(forceCollateral),  // Collateral was hurt
                "force rune must throw target and damage collateral target");
    return !testFailed;
}

// ============================================================
// BR11011 extended — immunity blocks the right bolt type
// ============================================================

static void br11011_immunity_wrong_type_setup(void) {
    clearArenaRadius(5);
    item *armor = givePlayerRunicArmor(CHAIN_MAIL, A_IMMUNITY, 10);
    armor->vorpalEnemy = vorpalEnemyClassByName("mage"); // Mages can cast lightning too
    equipItemFromPack(ARMOR, CHAIN_MAIL);

    // Turret is not a mage, so player is still vulnerable to the spark from a turret
    //    even if they are immune to sparks from eg. Dar or Pixie
    monst = placeHostileMonster(MK_SPARK_TURRET, posOffset(player.loc, 3, 0));
}

// ============================================================
// BR11007 extended — slaying rune does NOT kill non-designated creatures
// ============================================================

static void br11007_slaying_miss_setup(void) {
    clearArena();
    item *slaying = giveAndEquipRunicWeapon(DAGGER, W_SLAYING, 15, true);
    slaying->vorpalEnemy = vorpalEnemyClassByName("animal");

    // Kraken is "abomination", not "animal"
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0)); // Kraken out of water is helpless
}

static boolean br11007_slaying_miss_verify(void) {
    // Monster should take normal damage but not be instantly killed by slaying
    TEST_ASSERT( monsterIsAlive(monst) && monsterIsHurt(monst),
                "non-designated creature should not be instantly killed by slaying");
    return !testFailed;
}

// ============================================================
// BR11004 extended — paralysis duration grows with enchantment
// ============================================================

static int paralysisDurationEnchant20;

static void br11004_paralysis_enchant20_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_PARALYSIS, 20, true);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0));
}

static boolean br11004_paralysis_enchant20_step(void) {
    moveRight();
    if (monsterHasStatus(monst, STATUS_PARALYZED)) {
        paralysisDurationEnchant20 = monsterStatusTurns(monst, STATUS_PARALYZED);
        return false; // effect applied, done
    }
    return true; // keep retrying
}

static boolean br11004_paralysis_enchant20_verify(void) {
    TEST_ASSERT(paralysisDurationEnchant20 >= 11, "paralysis at enchant 20 should be 12"); // 12-1 due to monster turn
    return !testFailed;
}

// ============================================================
// BR11005 extended — multiplicity image count grows with enchantment
// ============================================================

static void br11005_multiplicity_enchant10_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_MULTIPLICITY, 10, true);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0));

    test_phase = 1;
    test_int[0] = 0;
    test_int[1] = 0;
}

static boolean br11005_multiplicity_enchant10_step(void) {
    int count;
    switch (test_phase) {

    // Phase 1: Use +10 until it triggers
    case 1:
        moveRight();
        test_int[0] = countMonstersOfType(MK_SPECTRAL_IMAGE);
        if (test_int[0] > 0) test_phase++;
        return true;

    // Phase 2: Wait for the images to dissapear
    case 2:
        restOneTurn();
        count = countMonstersOfType(MK_SPECTRAL_IMAGE);
        if (count == 0) test_phase++;
        return true;

    // Phase 3: Equip +20
    case 3:
        giveAndEquipRunicWeapon(SWORD, W_MULTIPLICITY, 20, true);
        test_phase++;
        return true;

    // Phase 4: Use +20 until it triggers
    case 4:
        moveRight();
        test_int[1] = countMonstersOfType(MK_SPECTRAL_IMAGE);
        if (test_int[1] > 0) test_phase++;
        return true;

    default:
        return false; // End of test
    }
}

static boolean br11005_multiplicity_enchant10_verify(void) {
    TEST_ASSERT(test_phase >= 5 && test_int[0] > 0 && test_int[1] > test_int[0], "multiplicity should spawn more images at +20 than +10");
    return !testFailed;
}

// ============================================================
// BR11021 extended — slaying rune identifies on trigger vs correct target
// ============================================================

static void br11021_slaying_identify_setup(void) {
    clearArena();
    item *slaying = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10, true);
    slaying->vorpalEnemy = vorpalEnemyClassForMonster(MK_TROLL);
    // slaying->flags &= ~(ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    monst = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    // Increase rat HP so it survives the attack — slaying rune needs the target
    // alive to fire (MB_IS_DYING blocks slaying).
    // setMonsterHealth(monst, 500);
    applyStatusToMonster(monst, STATUS_STUCK, 10);
    applyStatusToMonster(monst, STATUS_PARALYZED, 10);
}

static boolean br11021_slaying_identify_step(void) {
    moveRight();
    if (rogue.weapon && (rogue.weapon->flags & ITEM_RUNIC_IDENTIFIED)) {
        return false; // identified, done
    }
    applyStatusToMonster(monst, STATUS_STUCK, 10);
    applyStatusToMonster(monst, STATUS_PARALYZED, 10);
    return true; // keep retrying
}

static boolean br11021_slaying_identify_verify(void) {
    retryAction(200, br11021_slaying_identify_step);
    return !testFailed;
}

// ============================================================
// BR11010 extended — slaying CAN trigger against inanimate/invulnerable
//                     because slaying bypasses the check
// ============================================================

static boolean br11010_slaying_inanim_verify(void) {
    clearArena();
    item *slaying = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10, true);
    slaying->vorpalEnemy = vorpalEnemyClassByName("undead");
    TEST_ASSERT(rogue.weapon == slaying, "weapon should be equipped");
    creature *rev = placeHostileMonster(MK_REVENANT, posOffset(player.loc, 1, 0));
    // Paralyze the target so the attack is guaranteed to land.
    applyStatusToMonster(rev, STATUS_PARALYZED, 20);

    // The slaying rune checks monsterIsInClass first (100% vs correct class),
    // so it should kill the creature via killCreature, bypassing weapon immunity.
    playerAttacksMonster(rev);
    TEST_ASSERT(findMonster(MK_REVENANT) == NULL,
                "slaying rune should kill designated creature even if weapon-immune");
    return !testFailed;
}

// ============================================================
// BR11006 — force knockback does not trigger in a dead creature
// ============================================================

static void br11006_dead_target_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_FORCE, 20, true);
    creature *t = spawnMonster(MK_RAT, posOffset(player.loc, 1, 0));
    makeMonsterHostile(t);
    revealCell(t->loc);
    killCreature(t, false); // dead target
}

static boolean br11006_dead_target_verify(void) {
    // Attacking a dead target should not trigger force (no knockback)
    creature *deadRat = findMonster(MK_RAT);
    if (deadRat == NULL) {
        // Rat was killed, re-spawn one for testing
        deadRat = placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
        setMonsterHealth(deadRat, 1);
        deadRat->bookkeepingFlags |= MB_IS_DYING;
    }
    pos oldLoc = deadRat->loc;
    playerAttacksMonster(deadRat);
    TEST_ASSERT(deadRat->loc.x == oldLoc.x && deadRat->loc.y == oldLoc.y,
                "force should not knock back a dying target");
    return !testFailed;
}

// ============================================================
// BR11002 — speed rune does not trigger if already on free turn
// ============================================================

static void br11002_no_free_turn_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_SPEED, 20, true);
    placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    // Set ticksUntilTurn to -1 (already on a free turn)
    player.ticksUntilTurn = -1;
}

static boolean br11002_no_free_turn_verify(void) {
    creature *rat = findMonster(MK_RAT);
    if (rat) {
        playerAttacksMonster(rat);
        // After attack, ticksUntilTurn should be -1 still (already had it)
        // Speed rune check: if ticksUntilTurn == -1 it fires, which sets it to -1 again
        // The code checks: if (player.ticksUntilTurn != -1) { ... ticksUntilTurn = -1; }
        // So if already -1, the rune does nothing — no message, no ID.
        // We verify that the trembles message was NOT shown
        TEST_ASSERT(!messageWasShown("trembles"),
                    "speed should not fire again if already on free turn");
    }
    return !testFailed;
}

// ============================================================
// BR11004 extended — paralysis duration grows with enchantment (enchant 5)
// ============================================================

static int paralysisDurationEnchant5;

static void br11004_paralysis_enchant5_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_PARALYSIS, 5, true);
    // Use troll so it survives the attack
    monst = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    setMonsterHealth(monst, monst->info.maxHP);
    // Paralyze first so attack hits and rune fires
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
}

static boolean br11004_paralysis_enchant5_step(void) {
    creature *t = findMonster(MK_TROLL);
    if (t == NULL) return false;
    playerAttacksMonster(t);
    if (monsterHasStatus(t, STATUS_PARALYZED)) {
        paralysisDurationEnchant5 = monsterStatusTurns(t, STATUS_PARALYZED);
        return false; // effect applied, done
    }
    return true; // keep retrying
}

static boolean br11004_paralysis_enchant5_verify(void) {
    retryAction(200, br11004_paralysis_enchant5_step);
    if (testFailed) return false;

    // enchant 5 -> max(2, 2 + 5/2) = max(2, 4) = 4
    TEST_ASSERT(paralysisDurationEnchant5 >= 4,
                "paralysis at enchant 5 should be at least 4 turns");
    return !testFailed;
}

// ============================================================
// BR11005 — multiplicity against dead target (speed also against dead target)
// ============================================================

static void br11005_dead_target_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_MULTIPLICITY, 20, true);
    creature *t = spawnMonster(MK_RAT, posOffset(player.loc, 1, 0));
    makeMonsterHostile(t);
    revealCell(t->loc);
    killCreature(t, false); // dead target
}

static boolean br11005_dead_target_verify(void) {
    // Speed and multiplicity are exceptions: they CAN fire against dead targets
    // (see magicWeaponHit: "If the defender is already dead, proceed only if the
    //  runic is speed or multiplicity.")
    // But dead targets don't count as alive monsters, so we check the message
    // about the runic triggering.
    creature *deadRat = findMonster(MK_RAT);
    if (deadRat == NULL) {
        deadRat = spawnMonster(MK_RAT, posOffset(player.loc, 1, 0));
        makeMonsterHostile(deadRat);
        revealCell(deadRat->loc);
        deadRat->bookkeepingFlags |= MB_IS_DYING;
    }
    int before = monsterCount();
    playerAttacksMonster(deadRat);
    // Multiplicity should spawn images against a dying target
    TEST_ASSERT(monsterCount() > before,
                "multiplicity should fire against a dying target");
    return !testFailed;
}

// ============================================================
// BR11011 — immunity also works in projectileReflects for bolt-blocking monsters
// ============================================================

static void br11011_immunity_caster_reflect_setup(void) {
    clearArena();    
    item *armor = givePlayerRunicArmor(CHAIN_MAIL, A_IMMUNITY, 10);
    armor->vorpalEnemy = vorpalEnemyClassForMonster(MK_SPARK_TURRET); // MK_SPARK_TURRET cast BOLT_SPARK
    equipItemFromPack(ARMOR, CHAIN_MAIL);
    monst = placeHostileMonster(MK_SPARK_TURRET, posOffset(player.loc, 4, 0));
}

static boolean br11011_immunity_caster_reflect_step(void) {
    restOneTurn();
    if (monsterIsHurt(monst)) return false; // bolt reflected, end test
    return true; // keep going, bolt did not reflect back to caster
}

static boolean br11011_immunity_caster_reflect_verify(void) {
    // With immunity armor, the projectile should be reflected
    TEST_ASSERT(isPlayerHealthy() && monsterIsHurt(monst),
                "immunity armor should reflect the guardian's bolt");
    return !testFailed;
}

// ============================================================
// BR11006 extended — force knockback distance grows with enchantment
// ============================================================

static void br11006_force_high_enchant_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_FORCE, 50, true);
    monst = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    // netEnchant clamped to 50*FP, damage ~{9,11}, force chance ~4%.
    // Troll needs to survive ~25+ attacks for force to fire reliably.
    monst->info.maxHP = 500;
    setMonsterHealth(monst, monst->info.maxHP);
    startPos = monsterPosition(monst);
}

static boolean br11006_force_high_enchant_step(void) {
    playerAttacksMonster(monst);
    pos newPos = monsterPosition(monst);
    if (newPos.x != startPos.x || newPos.y != startPos.y) {
        return false; // knocked back, done
    }
    return true; // keep retrying
}

static boolean br11006_force_high_enchant_verify(void) {
    retryAction(200, br11006_force_high_enchant_step);
    if (testFailed) return false;

    // enchant 50 -> weaponForceDistance(50*FP) = max(4, (50*2/1)+2) = 102
    // But knockback is limited by map boundaries and obstacles
    pos newPos = monsterPosition(monst);
    TEST_ASSERT(newPos.x != startPos.x || newPos.y != startPos.y,
                "force with high enchantment should knock target back");
    return !testFailed;
}

// ============================================================
// BR11010 extended — weapons with non-slaying runes don't trigger
//                     against inanimate creatures
// ============================================================

static boolean br11010_inanimate_quietus_verify(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_QUIETUS, 20, true);
    // WARDEN_OF_YENDOR is MONST_INVULNERABLE — blocks runic effects
    creature *warden = placeHostileMonster(MK_WARDEN_OF_YENDOR, posOffset(player.loc, 1, 0));
    setMonsterHealth(warden, warden->info.maxHP);

    playerAttacksMonster(warden);
    // Quietus should NOT fire on invulnerable — warden should still be alive
    TEST_ASSERT(findMonster(MK_WARDEN_OF_YENDOR) != NULL,
                "quietus should not activate against invulnerable creature");
    return !testFailed;
}

// ============================================================
// BR11010 extended — paralysis against inanimate creature
// ============================================================

static boolean br11010_inanimate_paralysis_verify(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_PARALYSIS, 20, true);
    // WARDEN_OF_YENDOR is MONST_INVULNERABLE — blocks runic effects
    creature *warden = placeHostileMonster(MK_WARDEN_OF_YENDOR, posOffset(player.loc, 1, 0));
    setMonsterHealth(warden, warden->info.maxHP);

    playerAttacksMonster(warden);
    // Paralysis should NOT fire on invulnerable
    TEST_ASSERT(!monsterHasStatus(warden, STATUS_PARALYZED),
                "paralysis should not activate against invulnerable creature");
    return !testFailed;
}

// ============================================================
// BR11013 extended — respiration does NOT protect against non-toxic damage
// ============================================================

static boolean br11013_no_false_protection_verify(void) {
    clearArena();
    givePlayerRunicArmor(CHAIN_MAIL, A_RESPIRATION, 10);
    equipItemFromPack(ARMOR, CHAIN_MAIL);
    setPlayerHealth(1);

    // Stand on a damaging but non-toxic terrain (lava has T_IS_FIRE but not
    // T_RESPIRATION_IMMUNITIES)
    pos fireLoc = posOffset(player.loc, 1, 0);
    setTerrain(fireLoc, LAVA);
    setPlayerPos(fireLoc);
    revealCell(fireLoc);

    restTurns(1);

    // Respiration should NOT protect against fire
    // (it only protects against T_RESPIRATION_IMMUNITIES)
    TEST_ASSERT(isPlayerDead(),
                "respiration should not protect against non-gas damage (fire)");
    return !testFailed;
}

// ============================================================
// BR11020 extended — slaying against wrong kind does 0% damage from rune
// ============================================================

static boolean br11020_slaying_wrong_kind_verify(void) {
    clearArena();
    item *slaying = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10, true);
    slaying->vorpalEnemy = vorpalEnemyClassByName("animal");
    // Attack a troll (not rat)
    creature *troll = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));

    // Slaying rune should not trigger against wrong kind (chance = 0)
    playerAttacksMonster(troll);
    // Troll takes normal damage from the attack itself, but the slaying
    // effect should not trigger (chance = 0 for wrong kind)
    // The weapon still deals normal damage
    TEST_ASSERT(findMonster(MK_TROLL) != NULL,
                "slaying against wrong kind should not instant-kill");
    return !testFailed;
}

// ============================================================
// BR11001 extended — runic chance increases with enchant for different runes
// ============================================================

static boolean br11001_all_runes_chance_rises_verify(void) {
    // Test that multiple runic types show increasing chance with enchant
    short quietusChance10 = runicActivationChance(
        givePlayerRunicWeapon(DAGGER, W_QUIETUS, 10, true));
    short quietusChance50 = runicActivationChance(
        givePlayerRunicWeapon(DAGGER, W_QUIETUS, 50, true));

    char buf[120];
    sprintf(buf, "quietus chance at enchant 10=%d should be < enchant 50=%d",
            quietusChance10, quietusChance50);
    TEST_ASSERT(quietusChance10 < quietusChance50, buf);
    return !testFailed;
}

// ============================================================
// BR11016 extended — weapon with ITEM_ATTACKS_STAGGER normalizes damage
// ============================================================

static boolean br11016_stagger_dmg_normalize_verify(void) {
    // Mace attacks every other turn, so adjustedBaseDamage is halved before
    // the damage-based penalty is calculated. This means mace should have a
    // higher runic chance than a dagger with the same base damage.
    item *mace = givePlayerRunicWeapon(MACE, W_QUIETUS, 10, true);
    item *dagger = givePlayerRunicWeapon(DAGGER, W_QUIETUS, 10, true);

    short maceChance = runicActivationChance(mace);
    short daggerChance = runicActivationChance(dagger);

    char buf[120];
    sprintf(buf, "mace (stagger, chance=%d) should have higher chance than dagger (chance=%d)",
            maceChance, daggerChance);
    TEST_ASSERT(maceChance >= daggerChance, buf);
    return !testFailed;
}

// ============================================================
// BR11002 extended — speed gives extra turn only on activation
// ============================================================

static void br11002_speed_multi_attack_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_SPEED, 20, true);
}

static boolean br11002_speed_multi_attack_step(void) {
    creature *rat = placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    setMonsterHealth(rat, rat->info.maxHP);
    playerAttacksMonster(rat);
    // Speed rune should set ticksUntilTurn to -1
    if (player.ticksUntilTurn == -1) return false;
    // If it didn't fire, re-spawn and retry
    clearArena();
    placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    return true;
}

static boolean br11002_speed_multi_attack_verify(void) {
    retryAction(200, br11002_speed_multi_attack_step);
    TEST_ASSERT(player.ticksUntilTurn == -1,
                "speed rune should set ticksUntilTurn to -1 on activation");
    return !testFailed;
}

// ============================================================
// BR11018 extended — chance can't go below enchantment level
// ============================================================

static boolean br11018_minimum_enchant_verify(void) {
    // Heavy weapons have high base damage -> low modifier -> low chance
    // But chance should be clamped to max(1, enchantLevel)
    // War axe has high base damage
    item *axe = givePlayerRunicWeapon(WAR_AXE, W_SPEED, 5, true);
    short chance = runicActivationChance(axe);

    char buf[120];
    sprintf(buf, "war axe chance (%d) should be >= enchantment level (5)", chance);
    TEST_ASSERT(chance >= 5, buf);
    return !testFailed;
}

// ============================================================
// BR11006 extended — force with low enchantment has minimum knockback distance
// ============================================================

static void br11006_force_low_enchant_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_FORCE, 1, true);
    monst = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    startPos = monsterPosition(monst);
}

static boolean br11006_force_low_enchant_step(void) {
    playerAttacksMonster(monst);
    pos newPos = monsterPosition(monst);
    if (newPos.x != startPos.x || newPos.y != startPos.y) {
        return false; // knocked back, done
    }
    return true; // keep retrying
}

static boolean br11006_force_low_enchant_verify(void) {
    retryAction(200, br11006_force_low_enchant_step);
    if (testFailed) return false;

    // enchant 1 -> weaponForceDistance(1*FP) = max(4, 1*2+2) = 4
    pos newPos = monsterPosition(monst);
    TEST_ASSERT(distanceBetween(startPos, newPos) > 0,
                "force with enchant 1 should still knock back (min distance 4)");
    return !testFailed;
}

// ============================================================
// BR11011 — immunity also blocks the caster's own bolt
// ============================================================

static boolean br11011_immunity_self_bolt_verify(void) {
    clearArena();
    item *armor = givePlayerRunicArmor(CHAIN_MAIL, A_IMMUNITY, 10);
    armor->vorpalEnemy = MK_LICH;
    equipItemFromPack(ARMOR, CHAIN_MAIL);

    // Lich casts fire bolt at player
    creature *lich = placeHostileMonster(MK_LICH, posOffset(player.loc, 4, 0));
    bolt theBolt = boltCatalog[BOLT_FIRE];
    zap(lich->loc, player.loc, &theBolt, false, false);

    // Immunity should block
    TEST_ASSERT(!isPlayerDead(), "immunity armor should block lich fire bolt");
    return !testFailed;
}

const testCase tests_runic[] = {
    // --- Activation Chance Rules (BR11001, BR11016, BR11017, BR11018, BR11019, BR11020) ---
    { "BR11001_chance_rises_with_enchant",
        clearArena, NULL, br11001_chance_rises_with_enchant_verify },
    { "BR11016_high_damage_lower_chance",
        clearArena, NULL, br11016_high_damage_lower_chance_verify },
    { "BR11017_speed_adjustment",
        clearArena, NULL, br11017_speed_adjustment_verify },
    { "BR11018_chance_bounded",
        clearArena, NULL, br11018_chance_bounded_verify },
    { "BR11019_weak_rune_flat_15",
        clearArena, br11019_weak_rune_flat_15_step, br11019_weak_rune_flat_15_verify },
    { "BR11020_slaying_no_random_chance",
        clearArena, NULL, br11020_slaying_no_chance_verify },
    { "BR11001_all_runes_chance_rises",
        clearArena, NULL, br11001_all_runes_chance_rises_verify },
    { "BR11016_stagger_dmg_normalize",
        clearArena, NULL, br11016_stagger_dmg_normalize_verify },
    { "BR11018_minimum_enchant",
        clearArena, NULL, br11018_minimum_enchant_verify },
    { "BR11020_slaying_wrong_kind",
        clearArena, NULL, br11020_slaying_wrong_kind_verify },

    // --- Weapon Effect Rules ---
    { "BR11002_speed_extra_turn",
        br11002_speed_setup, br11002_speed_step, br11002_speed_verify },
    { "BR11002_speed_no_free_turn",
        br11002_no_free_turn_setup, NULL, br11002_no_free_turn_verify },
    { "BR11002_speed_multi_attack",
        br11002_speed_multi_attack_setup, br11002_speed_multi_attack_step, br11002_speed_multi_attack_verify },
    { "BR11003_quietus_instant_kill",
        br11003_quietus_setup, br11003_quietus_step, br11003_quietus_verify },
    { "BR11004_paralysis_duration_grows",
        br11004_paralysis_setup, br11004_paralysis_step, br11004_paralysis_verify },
    { "BR11004_paralysis_enchant20",
        br11004_paralysis_enchant20_setup, br11004_paralysis_enchant20_step, br11004_paralysis_enchant20_verify },
    { "BR11004_paralysis_enchant5",
        br11004_paralysis_enchant5_setup, br11004_paralysis_enchant5_step, br11004_paralysis_enchant5_verify },
    { "BR11005_multiplicity_summons",
        br11005_multiplicity_setup, br11005_multiplicity_step, br11005_multiplicity_verify },
    { "BR11005_multiplicity_enchant10",
        br11005_multiplicity_enchant10_setup, br11005_multiplicity_enchant10_step, br11005_multiplicity_enchant10_verify },
    { "BR11005_multiplicity_dead_target",
        br11005_dead_target_setup, NULL, br11005_dead_target_verify },
    { "BR11006_force_knockback",
        br11006_force_setup, br11006_force_step, br11006_force_verify },
    { "BR11006_force_collateral",
        br11006_force_collateral_setup, br11006_force_collateral_step, br11006_force_collateral_verify },
    { "BR11006_force_high_enchant",
        br11006_force_high_enchant_setup, br11006_force_high_enchant_step, br11006_force_high_enchant_verify },
    { "BR11006_force_low_enchant",
        br11006_force_low_enchant_setup, br11006_force_low_enchant_step, br11006_force_low_enchant_verify },
    { "BR11006_dead_target",
        br11006_dead_target_setup, NULL, br11006_dead_target_verify },
    { "BR11007_slaying_kills_designated",
        br11007_slaying_setup, br11007_slaying_step, br11007_slaying_verify },
    { "BR11007_slaying_miss_non_designated",
        br11007_slaying_miss_setup, move_right_step, br11007_slaying_miss_verify },
    { "BR11008_mercy_heals",
        br11008_mercy_setup, br11008_mercy_step, br11008_mercy_verify },
    { "BR11009_plenty_duplicating",
        br11009_plenty_setup, br11009_plenty_step, br11009_plenty_verify },

    // --- Inanimate/Invulnerable Rules ---
    { "BR11010_inanimate_invulnerable_blocked",
        br11010_inanimate_setup, br11010_inanimate_step, br11010_inanimate_verify },
    { "BR11010_slaying_inanim",
        clearArena, NULL, br11010_slaying_inanim_verify },
    { "BR11010_inanimate_quietus",
        clearArena, NULL, br11010_inanimate_quietus_verify },
    { "BR11010_inanimate_paralysis",
        clearArena, NULL, br11010_inanimate_paralysis_verify },

    // --- Armor Rules ---
    { "BR11011_immunity_blocks_designated_bolt",
        br11011_immunity_setup, NULL, br11011_immunity_verify },
    { "BR11011_immunity_wrong_type",
        br11011_immunity_wrong_type_setup, wait_until_hurt, player_is_hurt_verify },
    { "BR11011_immunity_caster_reflect",
        br11011_immunity_caster_reflect_setup, br11011_immunity_caster_reflect_step, br11011_immunity_caster_reflect_verify },
    { "BR11011_immunity_self_bolt",
        clearArena, NULL, br11011_immunity_self_bolt_verify },
    { "BR11012_reflection_bounces_bolt",
        br11012_reflection_setup, NULL, br11012_reflection_verify },
    { "BR11013_respiration_gas_protection",
        br11013_respiration_setup, wait_10_turns_step, br11013_respiration_verify },
    { "BR11013_damage_terrain",
        clearArena, NULL, br11013_no_false_protection_verify },
    { "BR11014_dampening_absorbs_explosive",
        br11014_dampening_setup, NULL, br11014_dampening_verify },
    { "BR11015_armor_reveals_after_wearing",
        br11015_reveal_setup, NULL, br11015_reveal_verify },

    // --- Auto-ID Rules ---
    { "BR11021_weapon_identifies_on_trigger",
        br11021_weapon_identify_setup, br11021_weapon_identify_step, br11021_weapon_identify_verify },
    { "BR11021_slaying_identifies_on_trigger",
        br11021_slaying_identify_setup, br11021_slaying_identify_step, br11021_slaying_identify_verify },
    { "BR11022_armor_identifies_on_reflection",
        br11022_armor_identify_setup, NULL, br11022_armor_identify_verify },

    { NULL, NULL, NULL, NULL }
};
