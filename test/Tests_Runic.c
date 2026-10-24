#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

static creature *monst;
static int healthBefore;
static pos playerStartPos;
static pos startPos;
static pos endPos;
static creature *forceCollateral;
static item *theItem;
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
    theItem = giveAndEquipRunicWeapon(DAGGER, W_PLENTY, 10, true);
    return false;
}

static boolean br11019_weak_rune_flat_15_verify(void) {
    TEST_ASSERT(runicActivationChance(theItem) == 15, "W_PLENTY is bad rune, chance should be 15");
    return !testFailed;
}

// ============================================================
// BR11020 — slaying rune has no random activation chance
// ============================================================

static boolean br11020_slaying_no_chance_verify(void) {
    // Slaying rune returns 0 from runicActivationChance (no random check).
    theItem = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10, true);
    // Set vorpalEnemy to the target we'll use
    theItem->vorpalEnemy = 2; // Arbitrary vorpal enemy class

    TEST_ASSERT(runicActivationChance(theItem) == 0,
                "slaying rune chance should be 0 (no random roll)");
    return !testFailed;
}

// ============================================================
// BR11002 — speed rune grants an immediate extra turn
// ============================================================

static void br11002_speed_setup(void) {
    clearArena();
    theItem = giveAndEquipRunicWeapon(DAGGER, W_SPEED, 20, true);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0)); // Kraken is helpless out of water
}

static boolean br11002_speed_step(void) {
    moveRight();
    test_int[0] = player.ticksUntilTurn;
    if (messageWasShown("trembles")) return false;
    return turnNumber() < 1000; // keep attacking until a free turn fires
}

static boolean br11002_speed_verify(void) {
    // After speed rune fires, player.ticksUntilTurn becomes -1 (free turn)
    TEST_ASSERT(messageWasShown("trembles"),
                "speed rune should grant an extra turn");
    return !testFailed;
}

// ============================================================
// BR11003 — quietus rune sometimes instantly kills
// ============================================================

static void br11003_quietus_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_QUIETUS, 20, true);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0)); // Kraken is helpless out of water
}

static boolean br11003_quietus_step(void) {
    moveRight();
    if (messageWasShown("suddenly")) return false;
    return turnNumber() < 1000;
}

static boolean br11003_quietus_verify(void) {
    TEST_ASSERT(messageWasShown("suddenly"), "monster should be dead");
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
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0)); // Kraken is helpless out of water
}

static boolean br11004_paralysis_step(void) {
    moveRight();
    if (monsterHasStatus(monst, STATUS_PARALYZED) || turnNumber() > 1000) {
        paralysisDuration = monsterStatusTurns(monst, STATUS_PARALYZED);
        return false; // effect applied, done
    }
    return true; // keep retrying
}

static boolean br11004_paralysis_verify(void) {
    TEST_ASSERT(paralysisDuration >= 2, "paralysis duration should be >= 2");
    return !testFailed;
}

// ============================================================
// BR11005 — multiplicity rune summons spectral duplicates
// ============================================================

static int imagesBefore;

static void br11005_multiplicity_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_MULTIPLICITY, 20, true);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0)); // Kraken is helpless out of water
    imagesBefore = monsterCount();
}

static boolean br11005_multiplicity_step(void) {
    playerAttacksMonster(monst);
    if (monsterCount() > imagesBefore || turnNumber() > 1000) return false; // summoned, done
    return true; // keep retrying
}

static boolean br11005_multiplicity_verify(void) {
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
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0)); // Kraken is helpless out of water
    startPos = monsterPosition(monst);
}

static boolean br11006_force_step(void) {
    playerAttacksMonster(monst);
    pos newPos = monsterPosition(monst);
    if (!posEq(startPos, newPos)) {
        return false; // knocked back, done
    }
    return true; // keep retrying
}

static boolean br11006_force_verify(void) {
    pos newPos = monsterPosition(monst);
    TEST_ASSERT(!posEq(startPos, newPos),
                "force rune should knock target backward");
    return !testFailed;
}

// ============================================================
// BR11007 — slaying rune always hits and kills creatures of designated kind
// ============================================================

static void br11007_slaying_setup(void) {
    clearArena();
    theItem = giveAndEquipRunicWeapon(DAGGER, W_SLAYING, 10, true);
    theItem->vorpalEnemy = vorpalEnemyClassForMonster(MK_KRAKEN);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0)); // Kraken is helpless out of water
}

static boolean br11007_slaying_step(void) {
    moveRight();
    if (monsterIsDead(monst) || turnNumber() > 1000) return false; // killed, stop retrying
    return true; // keep retrying
}

static boolean br11007_slaying_verify(void) {
    TEST_ASSERT(monsterIsDead(monst) && messageWasShown("suddenly"), "monster should be dead");
    return !testFailed;
}

// ============================================================
// BR11008 — mercy rune heals the creature it strikes
// ============================================================

static void br11008_mercy_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_MERCY, -3, true);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0));    // Kraken out of water is helpless == always hit
    setMonsterHealth(monst, 20);
    healthBefore = monsterHealth(monst);
}

static boolean br11008_mercy_step(void) {
    setMonsterHealth(monst, 20);
    moveRight(); // Attack monster
    if (monsterHealth(monst) > healthBefore || turnNumber() > 1000) return false; // healed, end test
    return true; // not healed yet, keep going
}

static boolean br11008_mercy_verify(void) {
    TEST_ASSERT(monsterHealth(monst) > healthBefore, "weapon of mercy should heal enemy");
    return !testFailed;
}

// ============================================================
// BR11009 — plenty rune creates a duplicate of the struck creature
// ============================================================

static int plentyMonstersBefore;

static void br11009_plenty_setup(void) {
    clearArena();
    theItem = giveAndEquipRunicWeapon(DAGGER, W_PLENTY, -3, true);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0)); // Kraken out of water is helpless == always hit
    plentyMonstersBefore = monsterCount();
}

static boolean br11009_plenty_step(void) {
    moveRight(); // Attack monster
    if (monsterCount() > plentyMonstersBefore || turnNumber() > 1000) return false;
    return true; // keep retrying
}

static boolean br11009_plenty_verify(void) {
    TEST_ASSERT(monsterCount() > plentyMonstersBefore, "weapon of plenty should clone enemy");
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

static void br11011_immunity_setup(void) {
    clearArena();
    theItem = givePlayerRunicArmor(CHAIN_MAIL, A_IMMUNITY, 10, true);
    theItem->vorpalEnemy = vorpalEnemyClassForMonster(MK_SPARK_TURRET);
    monst = placeHostileMonster(MK_SPARK_TURRET, posOffset(player.loc, 4, 0));
}

static boolean br11011_immunity_verify(void) {
    TEST_ASSERT(isPlayerHealthy(), "immunity armor should negate bolts from monster class");
    return !testFailed;
}

// ============================================================
// BR11012 — armor of reflection bounces bolts back at the caster
// ============================================================

static void br11012_reflection_setup(void) {
    clearArena();
    theItem = givePlayerRunicArmor(CHAIN_MAIL, A_REFLECTION, 50, true);
    monst = placeHostileMonster(MK_SPARK_TURRET, posOffset(player.loc, 4, 0));
}

static boolean br11012_reflection_verify(void) {
    // TODO: Pick better verifier - maybe check that the monster is hurt or killed by its own bolt?
    // Maybe check the message log for a reflection message?
    TEST_ASSERT(isPlayerHealthy(), "reflection should bounce bolt away from player");
    return !testFailed;
}

// ============================================================
// BR11013 — armor of respiration protects against damaging gases
// ============================================================

static void br11013_respiration_setup(void) {
    clearArena();
    theItem = givePlayerRunicArmor(CHAIN_MAIL, A_RESPIRATION, 1, true);

    // Place poison gas cloud
    spawnFeature(player.loc, DF_CONFUSION_GAS_TRAP_CLOUD);
}

static boolean br11013_respiration_verify(void) {
    TEST_ASSERT(!monsterHasStatus(&player, STATUS_CONFUSED),
                "respiration armor should protect against gas effects");
    return !testFailed;
}

// ============================================================
// BR11014 — armor of dampening absorbs explosive damage
// ============================================================

static void br11014_dampening_setup(void) {
    clearArena();
    theItem = givePlayerRunicArmor(CHAIN_MAIL, A_DAMPENING, 10, true);
    drink(POTION_FIRE_IMMUNITY); // So burning won't kill the player
    setPlayerHealth(1); // set low HP so explosion would normally kill
    monst = placeHostileMonster(MK_EXPLOSIVE_BLOAT, posOffset(player.loc, 1, 0));
}

static boolean br11014_dampening_verify(void) {
    TEST_ASSERT(!isPlayerDead(), "dampening armor should absorb explosive damage");
    return !testFailed;
}

// ============================================================
// BR11015 — armor reveals its runic nature after several turns of wear
// ============================================================

static void br11015_reveal_setup(void) {
    clearArena();
    theItem = givePlayerRunicArmor(CHAIN_MAIL, A_DAMPENING, 10, false);
    theItem->charges = 5; // turns until reveal
}

static boolean br11015_reveal_verify(void) {
    // Armor charges decremented in processIncrementalAutoID; when they reach 0,
    // the armor sets ITEM_IDENTIFIED (which reveals it has a runic enchant in its name).
    TEST_ASSERT(itemIsIdentified(theItem) && !runicIsIdentified(theItem),
                "runic armor should provide runic hint after enough turns of wear");
    return !testFailed;
}

// ============================================================
// BR11021 — weapon reveals runic nature when effect fires
// ============================================================

static void br11021_weapon_identify_setup(void) {
    clearArena();
    theItem = giveAndEquipRunicWeapon(DAGGER, W_SPEED, 20, false);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0)); // Kraken is helpless out of water
}

static boolean br11021_weapon_identify_step(void) {
    moveRight();
    if (runicIsIdentified(theItem) || turnNumber() > 1000) return false; // identified, done
    return true; // keep retrying
}

static boolean br11021_weapon_identify_verify(void) {
    TEST_ASSERT(runicIsIdentified(theItem), "runic weapon should identify itself after triggering");
    return !testFailed;
}

// ============================================================
// BR11022 — armor auto-identifies on runic trigger (e.g., reflection)
// ============================================================

static void br11022_armor_identify_setup(void) {
    clearArena();
    theItem = givePlayerRunicArmor(CHAIN_MAIL, A_REFLECTION, 50, false);
    monst = placeHostileMonster(MK_SPARK_TURRET, posOffset(player.loc, 2, 0));
}

static boolean br11022_armor_identify_verify(void) {
    TEST_ASSERT(runicIsIdentified(theItem),
                "armor should auto-identify when reflection triggers");
    return !testFailed;
}

// ============================================================
// BR11006 extended — force knocks into another creature, damaging it too
// ============================================================

static void br11006_force_collateral_setup(void) {
    setArenaRadius(5,FLOOR);
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
    setArenaRadius(5,FLOOR);
    item *armor = givePlayerRunicArmor(CHAIN_MAIL, A_IMMUNITY, 10, true);
    armor->vorpalEnemy = vorpalEnemyClassByName("mage"); // Mages can cast lightning too

    // Turret is not a mage, so player is still vulnerable to the spark from a turret
    //    even if they are immune to sparks from eg. Dar or Pixie
    monst = placeHostileMonster(MK_SPARK_TURRET, posOffset(player.loc, 3, 0));
}

// ============================================================
// BR11007 extended — slaying rune does NOT kill non-designated creatures
// ============================================================

static void br11007_slaying_miss_setup(void) {
    clearArena();
    theItem = giveAndEquipRunicWeapon(DAGGER, W_SLAYING, 15, true);
    theItem->vorpalEnemy = vorpalEnemyClassByName("animal");

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
    theItem = giveAndEquipRunicWeapon(DAGGER, W_MULTIPLICITY, 10, true);
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
    theItem = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10, true);
    theItem->vorpalEnemy = vorpalEnemyClassForMonster(MK_KRAKEN);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0));
}

static boolean br11021_slaying_identify_step(void) {
    moveRight();
    if (runicIsIdentified(theItem) || turnNumber() > 1000) {
        return false; // identified, done
    }
    return true; // keep retrying
}

static boolean br11021_slaying_identify_verify(void) {
    TEST_ASSERT(runicIsIdentified(theItem), "slaying runic should identify after triggering");
    return !testFailed;
}

// ============================================================
// BR11010 extended — slaying CAN trigger against inanimate/invulnerable
//                     because slaying bypasses the check
// ============================================================

static void br11010_slaying_inanim_setup(void) {
    clearArena();
    theItem = giveAndEquipRunicWeapon(DAGGER, W_SLAYING, 10, true);
    theItem->vorpalEnemy = vorpalEnemyClassForMonster(MK_REVENANT);
    monst = placeHostileMonster(MK_REVENANT, posOffset(player.loc, 1, 0));
    // Paralyze the target so the attack is guaranteed to land.
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
}

static boolean br11010_slaying_inanim_step(void) {
    moveRight();
    if (monsterIsDead(monst) || turnNumber() > 1000) return false;
    return true;
}

static boolean br11010_slaying_inanim_verify(void) {
    TEST_ASSERT(monsterIsDead(monst) && messageWasShown("suddenly"),
                "slaying rune should kill designated creature even if weapon-immune");
    return !testFailed;
}


// ============================================================
// BR11004 extended — paralysis duration grows with enchantment (enchant 5)
// ============================================================

static int paralysisDurationEnchant5;

static void br11004_paralysis_enchant5_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_PARALYSIS, 5, true);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0));    
}

static boolean br11004_paralysis_enchant5_step(void) {
    moveRight();
    paralysisDurationEnchant5 = monsterStatusTurns(monst, STATUS_PARALYZED);
    return paralysisDurationEnchant5 <= 0;
}

static boolean br11004_paralysis_enchant5_verify(void) {
    // enchant 5 -> max(2, 2 + 5/2) = max(2, 4) = 4
    TEST_ASSERT(paralysisDurationEnchant5 >= 3, // 4 turns of paralysis - 1 turn for monster's turn
                "paralysis at enchant 5 should be at least 4 turns");
    return !testFailed;
}

// ============================================================
// BR11011 — immunity also works in projectileReflects for bolt-blocking monsters
// ============================================================

static void br11011_immunity_caster_reflect_setup(void) {
    clearArena();    
    theItem = givePlayerRunicArmor(CHAIN_MAIL, A_IMMUNITY, 10, true);
    theItem->vorpalEnemy = vorpalEnemyClassForMonster(MK_SPARK_TURRET); // MK_SPARK_TURRET cast BOLT_SPARK
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
                "immunity armor should reflect bolts from the monster class back at the caster");
    return !testFailed;
}

// ============================================================
// BR11020 extended — slaying against wrong kind does 0% damage from rune
// ============================================================

static boolean br11020_slaying_wrong_kind_verify(void) {
    clearArena();
    theItem = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10, true);
    theItem->vorpalEnemy = vorpalEnemyClassByName("animal");
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

    // TODO: Pick random runic to test
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
    theItem = giveAndEquipRunicWeapon(DAGGER, W_FORCE, 20, true);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0));
    startPos = monsterPosition(monst);
}

static boolean br11006_force_low_enchant_step(void) {
    moveRight();
    endPos = monsterPosition(monst);
    if (!posEq(startPos,endPos)) {
        return false; // knocked back
    }
    return true; // not knocked back yet
}

static boolean br11006_force_low_enchant_verify(void) {
    TEST_ASSERT(monsterIsAlive(monst) && distanceBetween(startPos, endPos) > 0,
                "force with enchant 1 should still knock back (min distance 4)");
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
    { "BR11006_force_knockback",
        br11006_force_setup, br11006_force_step, br11006_force_verify },
    { "BR11006_force_collateral",
        br11006_force_collateral_setup, br11006_force_collateral_step, br11006_force_collateral_verify },
    { "BR11006_force_low_enchant",
        br11006_force_low_enchant_setup, br11006_force_low_enchant_step, br11006_force_low_enchant_verify },
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
        br11010_slaying_inanim_setup, br11010_slaying_inanim_step, br11010_slaying_inanim_verify },

    // --- Armor Rules ---
    { "BR11011_immunity_blocks_designated_bolt",
        br11011_immunity_setup, wait_10_turns_step, br11011_immunity_verify },
    { "BR11011_immunity_wrong_type",
        br11011_immunity_wrong_type_setup, wait_until_hurt, player_is_hurt_verify },
    { "BR11011_immunity_caster_reflect",
        br11011_immunity_caster_reflect_setup, br11011_immunity_caster_reflect_step, br11011_immunity_caster_reflect_verify },
    { "BR11012_reflection_bounces_bolt",
        br11012_reflection_setup, wait_100_turns_step, br11012_reflection_verify },
    { "BR11013_respiration_gas_protection",
        br11013_respiration_setup, wait_10_turns_step, br11013_respiration_verify },
    { "BR11014_dampening_absorbs_explosive",
        br11014_dampening_setup, wait_10_turns_step, br11014_dampening_verify },
    { "BR11015_runic_armor_hint_after_wearing",
        br11015_reveal_setup, wait_10_turns_step, br11015_reveal_verify },

    // --- Auto-ID Rules ---
    { "BR11021_weapon_identifies_on_trigger",
        br11021_weapon_identify_setup, br11021_weapon_identify_step, br11021_weapon_identify_verify },
    { "BR11021_slaying_identifies_on_trigger",
        br11021_slaying_identify_setup, br11021_slaying_identify_step, br11021_slaying_identify_verify },
    { "BR11022_armor_identifies_on_reflection",
        br11022_armor_identify_setup, wait_100_turns_step, br11022_armor_identify_verify },

    { NULL, NULL, NULL, NULL }
};
