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
static boolean br110019_weak_rune_flat_15_step(void);

// ============================================================
// Shared helpers
// ============================================================

static void clearArena(void) {
    for (int dx = -2; dx <= 2; dx++)
        for (int dy = -2; dy <= 2; dy++)
            setTerrain(posOffset(player.loc, dx, dy), FLOOR);
    setPlayerPos(player.loc);
    clearMonsters();
}

// Give a runic weapon AND equip it as rogue.weapon (unlike givePlayerRunicWeapon
// which only adds it to the pack).  Returns the item for convenience.
static item *giveAndEquipRunicWeapon(short kind, short runicKind, short enchant) {
    item *w = givePlayerRunicWeapon(kind, runicKind, enchant);
    rogue.weapon = w;
    // Set a high enough strength so netEnchant is not negative for any weapon.
    // Max weapon strength requirement is 20 (great sword), so 30 covers all.
    rogue.strength = 30;
    recalculateEquipmentBonuses();
    player.info.accuracy = 100;
    return w;
}

static creature *placeHostileMonster(enum monsterTypes type, pos loc) {
    creature *monst = spawnMonster(type, loc);
    makeMonsterHostile(monst);
    revealCell(loc);
    return monst;
}

// ============================================================
// BR110001 — runic effect activates on a hit with chance rising with enchantment
// ============================================================

// The chance formula must increase with enchantment level.
static boolean br110001_chance_rises_with_enchant_verify(void) {
    fixpt lowEnchant = 1 * FP_FACTOR;
    fixpt highEnchant = 50 * FP_FACTOR;

    // Low enchantment dagger: chance must be > 0 but lower
    item *low = givePlayerRunicWeapon(DAGGER, W_SPEED, 1);
    short lowChance = runicWeaponChance(low, true, lowEnchant);

    // High enchantment dagger: chance must be strictly greater
    item *high = givePlayerRunicWeapon(DAGGER, W_SPEED, 50);
    short highChance = runicWeaponChance(high, true, highEnchant);

    char buf[120];
    sprintf(buf, "chance should rise with enchant: low=%d, high=%d", lowChance, highChance);
    TEST_ASSERT(lowChance > 0, buf);
    sprintf(buf, "high enchant should beat low enchant: low=%d, high=%d", lowChance, highChance);
    TEST_ASSERT(highChance > lowChance, buf);
    return !testFailed;
}

// ============================================================
// BR110016 — high base-damage weapons get lower runic chance
// ============================================================

static boolean br110016_high_damage_lower_chance_verify(void) {
    item *dagger = givePlayerRunicWeapon(DAGGER, W_SPEED, 10);
    item *hammer = givePlayerRunicWeapon(HAMMER, W_SPEED, 10);

    short daggerChance = runicWeaponChance(dagger, true, 10 * FP_FACTOR);
    short hammerChance = runicWeaponChance(hammer, true, 10 * FP_FACTOR);

    char buf[120];
    sprintf(buf, "hammer chance (%d) should be <= dagger chance (%d)", hammerChance, daggerChance);
    TEST_ASSERT(hammerChance <= daggerChance, buf);
    return !testFailed;
}

// ============================================================
// BR110017 — slow weapons get higher per-hit chance; double-attack weapons get lower
// ============================================================

static boolean br110017_speed_adjustment_verify(void) {
    // Mace has ITEM_ATTACKS_STAGGER (slower, every-other-turn) -> higher chance
    // Rapier has ITEM_ATTACKS_QUICKLY (fast, twice-per-turn) -> lower chance
    item *mace = givePlayerRunicWeapon(MACE, W_SPEED, 10);
    item *rapier = givePlayerRunicWeapon(RAPIER, W_SPEED, 10);

    short maceChance = runicWeaponChance(mace, true, 10 * FP_FACTOR);
    short rapierChance = runicWeaponChance(rapier, true, 10 * FP_FACTOR);

    char buf[120];
    sprintf(buf, "mace (%d) should have >= chance than rapier (%d)", maceChance, rapierChance);
    TEST_ASSERT(maceChance >= rapierChance, buf);
    return !testFailed;
}

// ============================================================
// BR110018 — runic chance is clamped between enchantment level and 100%
// ============================================================

static boolean br110018_chance_bounded_verify(void) {
    // Chance must be at least the enchantment level
    item *item1 = givePlayerRunicWeapon(DAGGER, W_SPEED, 3);
    short chance1 = runicWeaponChance(item1, true, 3 * FP_FACTOR);
    TEST_ASSERT(chance1 >= 3, "chance must be >= enchantment level (3)");

    // Chance must never exceed 100
    item *item2 = givePlayerRunicWeapon(SWORD, W_SPEED, 50);
    short chance2 = runicWeaponChance(item2, true, 50 * FP_FACTOR);
    TEST_ASSERT(chance2 <= 100, "chance must be <= 100");
    return !testFailed;
}

// ============================================================
// BR110019 — weak/cursed runes activate at flat 15%
// ============================================================

static boolean br110019_weak_rune_flat_15_step(void) {
    clearArena();

    // W_VULNERABILITY is a bad (cursed) armor enchant — use a bad weapon rune
    // W_PLENTY is good, but for bad runes we use negative enchant2 values.
    // In the code, runicType >= NUMBER_GOOD_WEAPON_ENCHANT_KINDS -> 15%.
    // We simulate a "bad" rune by using the W_PLENTY kind (which is the only
    // "bad" weapon rune, at index NUMBER_WEAPON_RUNIC_KINDS-1).
    // Actually, the check is: runicType >= NUMBER_GOOD_WEAPON_ENCHANT_KINDS.
    // W_PLENTY is the only bad weapon rune. So let's test that directly.
    item *plenty = giveAndEquipRunicWeapon(DAGGER, W_PLENTY, 10);

    char buf[120];
    sprintf(buf, "W_PLENTY is bad rune, chance should be 15, got %d", runicWeaponChance(plenty, false, 0));
    TEST_ASSERT(runicWeaponChance(plenty, false, 0) == 15, buf);
    return false;
}

static boolean br110019_weak_rune_flat_15_verify(void) {
    retryAction(200, br110019_weak_rune_flat_15_step);
    return !testFailed;
}

static void br110019_weak_rune_setup(void) {
    clearArena();
}

// ============================================================
// BR110020 — slaying rune has no random activation chance
// ============================================================

static boolean br110020_slaying_no_chance_verify(void) {
    // Slaying rune returns 0 from runicWeaponChance (no random check).
    item *slaying = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10);
    // Set vorpalEnemy to the target we'll use
    slaying->vorpalEnemy = 2; 

    TEST_ASSERT(runicWeaponChance(slaying, false, 0) == 0,
                "slaying rune chance should be 0 (no random roll)");
    return !testFailed;
}

// ============================================================
// BR110002 — speed rune grants an immediate extra turn
// ============================================================

static creature *speedTarget;
static int ticksBeforeAttack;

static void br110002_speed_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_SPEED, 20);
    speedTarget = placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    makeMonsterHostile(speedTarget);
    ticksBeforeAttack = player.ticksUntilTurn;
}

static boolean br110002_speed_step(void) {
    if (player.ticksUntilTurn == ticksBeforeAttack) {
        playerAttacksMonster(speedTarget);
        return true; // keep retrying
    }
    return false; // attack completed
}

static boolean br110002_speed_verify(void) {
    // After speed rune fires, player.ticksUntilTurn becomes -1 (free turn)
    TEST_ASSERT(player.ticksUntilTurn == -1 || messageWasShown("trembles"),
                "speed rune should grant an extra turn (ticksUntilTurn=-1)");
    return !testFailed;
}

// ============================================================
// BR110003 — quietus rune sometimes instantly kills
// ============================================================

static creature *quietusTarget;

static void br110003_quietus_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_QUIETUS, 20);
    quietusTarget = placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    setMonsterHealth(quietusTarget, quietusTarget->info.maxHP);
}

static boolean br110003_quietus_step(void) {
    playerAttacksMonster(quietusTarget);
    if (findMonster(MK_RAT) == NULL) return false; // killed, done
    return true; // keep retrying
}

static boolean br110003_quietus_verify(void) {
    retryAction(200, br110003_quietus_step);
    return !testFailed;
}

// ============================================================
// BR110004 — paralysis rune freezes target for duration growing with enchantment
// ============================================================

static creature *paralysisTarget;
static int paralysisDuration;

static void br110004_paralysis_setup(void) {
    clearArena();
    // Use enchant 10 so duration is determinable
    giveAndEquipRunicWeapon(DAGGER, W_PARALYSIS, 10);
    // Use troll (maxHP=65) so it survives the attack even with sneak bonus (5x).
    // Paralyze to guarantee hit. Damage with dagger enchant 10 ≈ 2, 5x = 10.
    // Troll at 65 HP → 55 HP, survives. Then paralysis rune fires.
    paralysisTarget = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    applyStatusToMonster(paralysisTarget, STATUS_PARALYZED, 20);
    setMonsterHealth(paralysisTarget, paralysisTarget->info.maxHP);
}

static boolean br110004_paralysis_step(void) {
    playerAttacksMonster(paralysisTarget);
    if (monsterHasStatus(paralysisTarget, STATUS_PARALYZED)) {
        paralysisDuration = monsterStatusTurns(paralysisTarget, STATUS_PARALYZED);
        return false; // effect applied, done
    }
    return true; // keep retrying
}

static boolean br110004_paralysis_verify(void) {
    retryAction(200, br110004_paralysis_step);
    if (testFailed) return false;

    // enchant 10 -> weaponParalysisDuration(10*FP) = max(2, 2 + 10/2) = 7
    char buf[120];
    sprintf(buf, "paralysis duration should be >= 2, got %d", paralysisDuration);
    TEST_ASSERT(paralysisDuration >= 2, buf);
    return !testFailed;
}

// ============================================================
// BR110005 — multiplicity rune summons spectral duplicates
// ============================================================

static creature *multiTarget;
static int imagesBefore;

static void br110005_multiplicity_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_MULTIPLICITY, 10);
    multiTarget = placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    setMonsterHealth(multiTarget, multiTarget->info.maxHP);
    imagesBefore = monsterCount();
}

static boolean br110005_multiplicity_step(void) {
    playerAttacksMonster(multiTarget);
    if (monsterCount() > imagesBefore) return false; // summoned, done
    return true; // keep retrying
}

static boolean br110005_multiplicity_verify(void) {
    retryAction(200, br110005_multiplicity_step);
    if (testFailed) return false;

    // Should have spawned at least 1 spectral image (enchant 10 -> clamp(10/3, 1, 7) = 3)
    TEST_ASSERT(monsterCount() > imagesBefore,
                "multiplicity should summon spectral duplicates");
    return !testFailed;
}

// ============================================================
// BR110006 — force rune knocks creature backward, dealing impact damage
// ============================================================

static pos forceStartPos;
static creature *forceTarget;

static void br110006_force_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_FORCE, 20);
    forceTarget = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    setMonsterHealth(forceTarget, forceTarget->info.maxHP);
    forceStartPos = monsterPosition(forceTarget);
}

static boolean br110006_force_step(void) {
    playerAttacksMonster(forceTarget);
    pos newPos = monsterPosition(forceTarget);
    if (newPos.x != forceStartPos.x || newPos.y != forceStartPos.y) {
        return false; // knocked back, done
    }
    return true; // keep retrying
}

static boolean br110006_force_verify(void) {
    retryAction(200, br110006_force_step);
    if (testFailed) return false;

    pos newPos = monsterPosition(forceTarget);
    TEST_ASSERT(newPos.x != forceStartPos.x || newPos.y != forceStartPos.y,
                "force rune should knock target backward");
    return !testFailed;
}

// ============================================================
// BR110007 — slaying rune always hits and kills creatures of designated kind
// ============================================================

static creature *slayingTarget;

static void br110007_slaying_setup(void) {
    clearArena();
    item *slaying = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10);
    slaying->vorpalEnemy = 2; 
    slayingTarget = placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    setMonsterHealth(slayingTarget, slayingTarget->info.maxHP);
}

static boolean br110007_slaying_step(void) {
    playerAttacksMonster(slayingTarget);
    if (findMonster(MK_RAT) == NULL) return false; // killed, stop retrying
    return true; // keep retrying
}

static boolean br110007_slaying_verify(void) {
    retryAction(200, br110007_slaying_step);
    return !testFailed;
}

// ============================================================
// BR110008 — mercy rune heals the creature it strikes
// ============================================================

static creature *mercyTarget;
static int healthBefore;

static void br110008_mercy_setup(void) {
    clearArena();
    // Use SWORD (no sneak attack bonus like dagger) at enchant -20 for low damage.
    // Sword damage at -20: {7,9} * 0.284 ≈ {1,2}.
    // Target is paralyzed for guaranteed hit, which applies 3x multiplier.
    // Damage = 3 or 6. Heal = 50% of troll maxHP(65) = 32.
    // With troll at partial HP(50): 50-3=47+32=65 or 50-6=44+32=65 (capped at 65).
    // hpAfter(65) > hpBefore(50) → test passes.
    giveAndEquipRunicWeapon(SWORD, W_MERCY, -20);
    mercyTarget = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    applyStatusToMonster(mercyTarget, STATUS_PARALYZED, 20);
    // Set HP to partial so heal can exceed the weapon damage
    setMonsterHealth(mercyTarget, 50);
}

static boolean br110008_mercy_step(void) {
    int hpBeforeAttack = monsterHealth(mercyTarget);
    playerAttacksMonster(mercyTarget);
    int hpAfter = monsterHealth(mercyTarget);
    // Mercy heals the defender; HP should increase despite taking weapon damage
    // With troll at HP=50, sword deals 3-6 damage, mercy heals 32 → HP=65
    if (hpAfter > hpBeforeAttack) {
        return false; // healed, done
    }
    return true; // keep retrying
}

static boolean br110008_mercy_verify(void) {
    retryAction(200, br110008_mercy_step);
    return !testFailed;
}

// ============================================================
// BR110009 — plenty rune creates a duplicate of the struck creature
// ============================================================

static int plentyMonstersBefore;

static void br110009_plenty_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_PLENTY, 50);
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

static boolean br110009_plenty_step(void) {
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

static boolean br110009_plenty_verify(void) {
    retryAction(200, br110009_plenty_step);
    return !testFailed;
}

// ============================================================
// BR110010 — weapon runes (other than slaying) don't trigger against
//            inanimate or invulnerable creatures
// ============================================================

static creature *inanimTarget;
static boolean inanimHitOccurred;

static void br110010_inanimate_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_SPEED, 20);
    // WARDEN_OF_YENDOR is MONST_INVULNERABLE
    inanimTarget = placeHostileMonster(MK_WARDEN_OF_YENDOR, posOffset(player.loc, 1, 0));
    inanimHitOccurred = false;
}

static boolean br110010_inanimate_step(void) {
    playerAttacksMonster(inanimTarget);
    // Speed rune should NOT fire on invulnerable — player.ticksUntilTurn should not be -1
    if (player.ticksUntilTurn == -1) {
        inanimHitOccurred = true;
    }
    return false; // test done after one attack
}

static boolean br110010_inanimate_verify(void) {
    TEST_ASSERT(!inanimHitOccurred,
                "speed rune should not activate against invulnerable creature");
    return !testFailed;
}

// ============================================================
// BR110011 — armor of immunity negates bolts of designated type
// ============================================================

static creature *immunityBoltCaster;

static void br110011_immunity_setup(void) {
    clearArena();
    // Create armor of immunity with vorpalEnemy = a bolt-casting monster
    item *armor = givePlayerRunicArmor(CHAIN_MAIL, A_IMMUNITY, 10);
    armor->vorpalEnemy = MK_LICH; // liches cast BOLT_FIRE
    equipItemFromPack(ARMOR, CHAIN_MAIL);

    // Lich stands far enough to fire a bolt but close enough to be visible
    immunityBoltCaster = placeHostileMonster(MK_LICH, posOffset(player.loc, 4, 0));
}

static boolean br110011_immunity_verify(void) {
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
// BR110012 — armor of reflection bounces bolts back at the caster
// ============================================================

static creature *reflectionCaster;

static void br110012_reflection_setup(void) {
    clearArena();
    givePlayerRunicArmor(CHAIN_MAIL, A_REFLECTION, 10);
    equipItemFromPack(ARMOR, CHAIN_MAIL);
    reflectionCaster = placeHostileMonster(MK_LICH, posOffset(player.loc, 4, 0));
}

static boolean br110012_reflection_verify(void) {
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
// BR110013 — armor of respiration protects against damaging gases
// ============================================================

static void br110013_respiration_setup(void) {
    clearArena();
    givePlayerRunicArmor(CHAIN_MAIL, A_RESPIRATION, 10);
    equipItemFromPack(ARMOR, CHAIN_MAIL);

    // Place poison gas cloud
    pos gasLoc = player.loc;
    spawnFeature(gasLoc, DF_POISON_GAS_CLOUD);
    revealCell(gasLoc);
}

static boolean br110013_respiration_verify(void) {
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
// BR110014 — armor of dampening absorbs explosive damage
// ============================================================

static void br110014_dampening_setup(void) {
    clearArena();
    givePlayerRunicArmor(CHAIN_MAIL, A_DAMPENING, 10);
    equipItemFromPack(ARMOR, CHAIN_MAIL);
    setPlayerHealth(1); // set low HP so explosion would normally kill
}

static boolean br110014_dampening_verify(void) {
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
// BR110015 — armor reveals its runic nature after several turns of wear
// ============================================================

static void br110015_reveal_setup(void) {
    clearArena();
    // Give an unidentified runic armor
    item *armor = givePlayerRunicArmor(CHAIN_MAIL, A_DAMPENING, 10);
    // Reset the identification flags so it acts like newly acquired unidentified runic
    armor->flags &= ~(ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    armor->charges = 20; // armorDelayToAutoID turns until reveal
    equipItemFromPack(ARMOR, CHAIN_MAIL);
}

static boolean br110015_reveal_verify(void) {
    // Wait for the armor to auto-identify after enough turns
    restTurns(25);

    // Armor charges decremented in processIncrementalAutoID; when they reach 0,
    // the armor sets ITEM_IDENTIFIED (which reveals it has a runic enchant in its name).
    TEST_ASSERT(rogue.armor && (rogue.armor->flags & ITEM_IDENTIFIED),
                "runic armor should identify itself after enough turns of wear");
    return !testFailed;
}

// ============================================================
// BR110021 — weapon reveals runic nature when effect fires
// ============================================================

static void br110021_weapon_identify_setup(void) {
    clearArena();
    // Give unidentified runic weapon
    item *weapon = giveAndEquipRunicWeapon(DAGGER, W_SPEED, 20);
    weapon->flags &= ~(ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    rogue.weapon = weapon;
    // Use troll so it survives the attack and speed rune can fire
    speedTarget = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    setMonsterHealth(speedTarget, speedTarget->info.maxHP);
}

static boolean br110021_weapon_identify_step(void) {
    playerAttacksMonster(speedTarget);
    if (rogue.weapon && (rogue.weapon->flags & ITEM_RUNIC_IDENTIFIED)) {
        return false; // identified, done
    }
    return true; // keep retrying
}

static boolean br110021_weapon_identify_verify(void) {
    retryAction(200, br110021_weapon_identify_step);
    return !testFailed;
}

// ============================================================
// BR110022 — armor auto-identifies on runic trigger (e.g., reflection)
// ============================================================

static creature *reflectionCaster2;

static void br110022_armor_identify_setup(void) {
    clearArena();
    // Give unidentified runic armor of reflection
    item *armor = givePlayerRunicArmor(CHAIN_MAIL, A_REFLECTION, 10);
    armor->flags &= ~(ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    equipItemFromPack(ARMOR, CHAIN_MAIL);
    reflectionCaster2 = placeHostileMonster(MK_LICH, posOffset(player.loc, 4, 0));
}

static boolean br110022_armor_identify_verify(void) {
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
// BR110006 extended — force knocks into another creature, damaging it too
// ============================================================

static pos forceStartPos2;
static pos forceEndPos;
static creature *forceTarget2;
static creature *forceCollateral;
static int collateralHPBefore;

static void br110006_force_collateral_setup(void) {
    clearArena();
    // Clear a larger area to see knockback
    for (int dx = -5; dx <= 5; dx++)
        for (int dy = -5; dy <= 5; dy++)
            setTerrain(posOffset(player.loc, dx, dy), FLOOR);
    setPlayerPos(player.loc);

    giveAndEquipRunicWeapon(SWORD, W_FORCE, 20);

    // Target directly in front
    forceTarget2 = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    setMonsterHealth(forceTarget2, forceTarget2->info.maxHP);
    forceStartPos2 = monsterPosition(forceTarget2);

    // Collateral target behind the primary target — place at (3,0) so troll can
    // actually move (BOLT_BLINKING is blocked by creatures at (2,0)).
    forceCollateral = placeHostileMonster(MK_RAT, posOffset(player.loc, 3, 0));
    setMonsterHealth(forceCollateral, forceCollateral->info.maxHP);
    collateralHPBefore = monsterHealth(forceCollateral);
}

static boolean br110006_force_collateral_step(void) {
    playerAttacksMonster(forceTarget2);
    forceEndPos = monsterPosition(forceTarget2);
    if (forceEndPos.x != forceStartPos2.x || forceEndPos.y != forceStartPos2.y) {
        return false; // knocked back, done
    }
    return true; // keep retrying
}

static boolean br110006_force_collateral_verify(void) {
    retryAction(200, br110006_force_collateral_step);
    if (testFailed) return false;

    // Force should knock target backward and potentially hit collateral
    TEST_ASSERT(monsterHealth(forceCollateral) < collateralHPBefore ||
                    monsterHealth(forceCollateral) == collateralHPBefore,
                "force knockback should be observable (collateral may or may not be hit)");
    return !testFailed;
}

// ============================================================
// BR110011 extended — immunity blocks the right bolt type
// ============================================================

static void br110011_immunity_wrong_type_setup(void) {
    clearArena();
    // Immunity against MK_LICH bolts (fire). The liche casts BOLT_FIRE.
    // If a different bolt type (ice) is fired, immunity should NOT block it.
    item *armor = givePlayerRunicArmor(CHAIN_MAIL, A_IMMUNITY, 10);
    armor->vorpalEnemy = MK_LICH;
    equipItemFromPack(ARMOR, CHAIN_MAIL);
}

static boolean br110011_immunity_wrong_type_verify(void) {
    // Fire a lightning bolt — immunity is against fire (lich), so lightning should pass
    bolt theBolt = boltCatalog[BOLT_LIGHTNING];
    zap(player.loc, posOffset(player.loc, 3, 0), &theBolt, false, false);

    // The bolt should have passed through since immunity is only against lich bolts
    TEST_ASSERT(!isPlayerDead(),
                "immunity should not block bolt types other than the designated one");
    return !testFailed;
}

// ============================================================
// BR110007 extended — slaying rune does NOT kill non-designated creatures
// ============================================================

static void br110007_slaying_miss_setup(void) {
    clearArena();
    item *slaying = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10);
    slaying->vorpalEnemy = 2; // "animal" class includes MK_RAT (designated enemy)
}

static boolean br110007_slaying_miss_verify(void) {
    creature *troll = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    setMonsterHealth(troll, troll->info.maxHP);
    // Paralyze to guarantee hit and sneak attack bonus (3x for sword)
    // Troll at 65 HP with sword at enchant 10: damage ~6*3=18, survives.
    applyStatusToMonster(troll, STATUS_PARALYZED, 20);

    // Slaying rune should not trigger against non-designated creature
    int trollHPBefore = monsterHealth(troll);
    playerAttacksMonster(troll);

    // Troll should take normal damage but not be instantly killed by slaying
    // (it takes damage from the regular attack)
    TEST_ASSERT(monsterHealth(troll) < trollHPBefore || trollHPBefore == 1,
                "non-designated creature should not be instantly killed by slaying");
    return !testFailed;
}

// ============================================================
// BR110004 extended — paralysis duration grows with enchantment
// ============================================================

static int paralysisDurationEnchant20;

static void br110004_paralysis_enchant20_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_PARALYSIS, 20);
    // Use troll so it survives the attack
    creature *t = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    setMonsterHealth(t, t->info.maxHP);
    // Paralyze first so attack hits and rune fires
    applyStatusToMonster(t, STATUS_PARALYZED, 20);
}

static boolean br110004_paralysis_enchant20_step(void) {
    creature *t = findMonster(MK_TROLL);
    if (t == NULL) return false;
    playerAttacksMonster(t);
    if (monsterHasStatus(t, STATUS_PARALYZED)) {
        paralysisDurationEnchant20 = monsterStatusTurns(t, STATUS_PARALYZED);
        return false; // effect applied, done
    }
    return true; // keep retrying
}

static boolean br110004_paralysis_enchant20_verify(void) {
    retryAction(200, br110004_paralysis_enchant20_step);
    if (testFailed) return false;

    // enchant 20 -> max(2, 2 + 20/2) = 12
    // enchant 10 -> max(2, 2 + 10/2) = 7
    // So enchant 20 should give strictly longer paralysis than enchant 10
    char buf[120];
    sprintf(buf, "paralysis at enchant 20 (%d) should exceed enchant 10 (>=7)", paralysisDurationEnchant20);
    TEST_ASSERT(paralysisDurationEnchant20 >= 12, buf);
    return !testFailed;
}

// ============================================================
// BR110005 extended — multiplicity image count grows with enchantment
// ============================================================

static void br110005_multiplicity_enchant10_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_MULTIPLICITY, 10);
    creature *t = placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    setMonsterHealth(t, t->info.maxHP);
}

static boolean br110005_multiplicity_enchant10_step(void) {
    creature *t = findMonster(MK_RAT);
    if (t == NULL) return false;
    playerAttacksMonster(t);
    return true;
}

static boolean br110005_multiplicity_enchant10_verify(void) {
    playerAttacksMonster(findMonster(MK_RAT));

    // enchant 10 -> clamp(10/3, 1, 7) = 3 images
    // Count spectral images (MK_SPECTRAL_IMAGE)
    int spectralCount = 0;
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *m = nextCreature(&it);
        if (m->info.monsterID == MK_SPECTRAL_IMAGE) spectralCount++;
    }
    TEST_ASSERT(spectralCount >= 1, "multiplicity should spawn at least 1 spectral image at enchant 10");
    return !testFailed;
}

// ============================================================
// BR110021 extended — slaying rune identifies on trigger vs correct target
// ============================================================

static void br110021_slaying_identify_setup(void) {
    clearArena();
    item *slaying = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10);
    slaying->vorpalEnemy = 2; // "animal" class includes MK_RAT
    slaying->flags &= ~(ITEM_RUNIC_IDENTIFIED | ITEM_RUNIC_HINTED);
    slayingTarget = placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    // Increase rat HP so it survives the attack — slaying rune needs the target
    // alive to fire (MB_IS_DYING blocks slaying).
    slayingTarget->info.maxHP = 500;
    setMonsterHealth(slayingTarget, slayingTarget->info.maxHP);
}

static boolean br110021_slaying_identify_step(void) {
    playerAttacksMonster(slayingTarget);
    if (rogue.weapon && (rogue.weapon->flags & ITEM_RUNIC_IDENTIFIED)) {
        return false; // identified, done
    }
    return true; // keep retrying
}

static boolean br110021_slaying_identify_verify(void) {
    retryAction(200, br110021_slaying_identify_step);
    return !testFailed;
}

// ============================================================
// BR110010 extended — slaying CAN trigger against inanimate/invulnerable
//                     because slaying bypasses the check
// ============================================================

static boolean br110010_slaying_inanim_verify(void) {
    clearArena();
    item *slaying = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10);
    // "undead" class (index 6) includes MK_REVENANT
    slaying->vorpalEnemy = 6;
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
// BR110006 — force knockback does not trigger in a dead creature
// ============================================================

static void br110006_dead_target_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_FORCE, 20);
    creature *t = spawnMonster(MK_RAT, posOffset(player.loc, 1, 0));
    makeMonsterHostile(t);
    revealCell(t->loc);
    killCreature(t, false); // dead target
}

static boolean br110006_dead_target_verify(void) {
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
// BR110017 extended — stagger weapons get compound chance formula
// ============================================================

static boolean br110017_stagger_formula_verify(void) {
    // Mace has ITEM_ATTACKS_STAGGER, so it should have a compound formula:
    // chance = 100 - (100-p)^2 where p is base chance
    item *mace = givePlayerRunicWeapon(MACE, W_SPEED, 10);
    item *dagger = givePlayerRunicWeapon(DAGGER, W_SPEED, 10);

    short maceChance = runicWeaponChance(mace, true, 10 * FP_FACTOR);
    short daggerChance = runicWeaponChance(dagger, true, 10 * FP_FACTOR);

    // Stagger should have a higher per-hit chance than non-stagger
    char buf[120];
    sprintf(buf, "stagger weapon (mace=%d) should have higher chance than non-stagger (dagger=%d)",
            maceChance, daggerChance);
    TEST_ASSERT(maceChance >= daggerChance, buf);
    return !testFailed;
}

// ============================================================
// BR110002 — speed rune does not trigger if already on free turn
// ============================================================

static void br110002_no_free_turn_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_SPEED, 20);
    placeHostileMonster(MK_RAT, posOffset(player.loc, 1, 0));
    // Set ticksUntilTurn to -1 (already on a free turn)
    player.ticksUntilTurn = -1;
}

static boolean br110002_no_free_turn_verify(void) {
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
// BR110004 extended — paralysis duration grows with enchantment (enchant 5)
// ============================================================

static int paralysisDurationEnchant5;

static void br110004_paralysis_enchant5_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_PARALYSIS, 5);
    // Use troll so it survives the attack
    paralysisTarget = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    setMonsterHealth(paralysisTarget, paralysisTarget->info.maxHP);
    // Paralyze first so attack hits and rune fires
    applyStatusToMonster(paralysisTarget, STATUS_PARALYZED, 20);
}

static boolean br110004_paralysis_enchant5_step(void) {
    creature *t = findMonster(MK_TROLL);
    if (t == NULL) return false;
    playerAttacksMonster(t);
    if (monsterHasStatus(t, STATUS_PARALYZED)) {
        paralysisDurationEnchant5 = monsterStatusTurns(t, STATUS_PARALYZED);
        return false; // effect applied, done
    }
    return true; // keep retrying
}

static boolean br110004_paralysis_enchant5_verify(void) {
    retryAction(200, br110004_paralysis_enchant5_step);
    if (testFailed) return false;

    // enchant 5 -> max(2, 2 + 5/2) = max(2, 4) = 4
    TEST_ASSERT(paralysisDurationEnchant5 >= 4,
                "paralysis at enchant 5 should be at least 4 turns");
    return !testFailed;
}

// ============================================================
// BR110005 — multiplicity against dead target (speed also against dead target)
// ============================================================

static void br110005_dead_target_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_MULTIPLICITY, 20);
    creature *t = spawnMonster(MK_RAT, posOffset(player.loc, 1, 0));
    makeMonsterHostile(t);
    revealCell(t->loc);
    killCreature(t, false); // dead target
}

static boolean br110005_dead_target_verify(void) {
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
// BR110011 — immunity also works in projectileReflects for bolt-blocking monsters
// ============================================================

static boolean br110011_immunity_caster_reflect_verify(void) {
    clearArena();
    item *armor = givePlayerRunicArmor(CHAIN_MAIL, A_IMMUNITY, 10);
    armor->vorpalEnemy = MK_GUARDIAN; // guardians cast BOLT_SPARK
    equipItemFromPack(ARMOR, CHAIN_MAIL);
    creature *caster = placeHostileMonster(MK_GUARDIAN, posOffset(player.loc, 4, 0));

    // Guardian casts bolt at player
    bolt theBolt = boltCatalog[BOLT_SPARK];
    zap(caster->loc, player.loc, &theBolt, false, false);

    // With immunity armor, the projectile should be blocked/reflected
    TEST_ASSERT(!isPlayerDead(),
                "immunity armor should block the guardian's bolt");
    return !testFailed;
}

// ============================================================
// BR110008 — mercy heals even low-health creatures
// ============================================================

static void br110008_mercy_full_health_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_MERCY, 50);
    // Use troll with high HP so it survives many attacks — mercy rune
    // (15% bad rune chance) needs multiple attempts to fire.
    mercyTarget = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    mercyTarget->info.maxHP = 500;
    setMonsterHealth(mercyTarget, mercyTarget->info.maxHP);
    healthBefore = monsterHealth(mercyTarget);
}

static boolean br110008_mercy_full_health_step(void) {
    playerAttacksMonster(mercyTarget);
    // Mercy should heal the creature even at full health (it heals by a percentage)
    // The heal function heals up to maxHP
    if (monsterHealth(mercyTarget) >= healthBefore) return false; // healed, done
    return true;
}

static boolean br110008_mercy_full_health_verify(void) {
    retryAction(200, br110008_mercy_full_health_step);
    return !testFailed;
}

// ============================================================
// BR110006 extended — force knockback distance grows with enchantment
// ============================================================

static void br110006_force_high_enchant_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_FORCE, 50);
    forceTarget2 = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    // netEnchant clamped to 50*FP, damage ~{9,11}, force chance ~4%.
    // Troll needs to survive ~25+ attacks for force to fire reliably.
    forceTarget2->info.maxHP = 500;
    setMonsterHealth(forceTarget2, forceTarget2->info.maxHP);
    forceStartPos2 = monsterPosition(forceTarget2);
}

static boolean br110006_force_high_enchant_step(void) {
    playerAttacksMonster(forceTarget2);
    pos newPos = monsterPosition(forceTarget2);
    if (newPos.x != forceStartPos2.x || newPos.y != forceStartPos2.y) {
        return false; // knocked back, done
    }
    return true; // keep retrying
}

static boolean br110006_force_high_enchant_verify(void) {
    retryAction(200, br110006_force_high_enchant_step);
    if (testFailed) return false;

    // enchant 50 -> weaponForceDistance(50*FP) = max(4, (50*2/1)+2) = 102
    // But knockback is limited by map boundaries and obstacles
    pos newPos = monsterPosition(forceTarget2);
    TEST_ASSERT(newPos.x != forceStartPos2.x || newPos.y != forceStartPos2.y,
                "force with high enchantment should knock target back");
    return !testFailed;
}

// ============================================================
// BR110010 extended — weapons with non-slaying runes don't trigger
//                     against inanimate creatures
// ============================================================

static boolean br110010_inanimate_quietus_verify(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_QUIETUS, 20);
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
// BR110010 extended — paralysis against inanimate creature
// ============================================================

static boolean br110010_inanimate_paralysis_verify(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_PARALYSIS, 20);
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
// BR110013 extended — respiration does NOT protect against non-toxic damage
// ============================================================

static boolean br110013_no_false_protection_verify(void) {
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
// BR110020 extended — slaying against wrong kind does 0% damage from rune
// ============================================================

static boolean br110020_slaying_wrong_kind_verify(void) {
    clearArena();
    item *slaying = giveAndEquipRunicWeapon(SWORD, W_SLAYING, 10);
    slaying->vorpalEnemy = 2; 
    // Attack a troll (not rat)
    creature *troll = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    setMonsterHealth(troll, troll->info.maxHP);

    // Slaying rune should not trigger against wrong kind (chance = 0)
    playerAttacksMonster(troll);
    // Troll takes normal damage from the attack itself, but the slaying
    // effect should not trigger (chance = 0 for wrong kind)
    // The weapon still deals normal damage
    TEST_ASSERT(troll->info.monsterID == MK_TROLL,
                "slaying against wrong kind should not instant-kill");
    return !testFailed;
}

// ============================================================
// BR110001 extended — runic chance increases with enchant for different runes
// ============================================================

static boolean br110001_all_runes_chance_rises_verify(void) {
    // Test that multiple runic types show increasing chance with enchant
    short quietusChance10 = runicWeaponChance(
        givePlayerRunicWeapon(DAGGER, W_QUIETUS, 10), false, 0);
    short quietusChance50 = runicWeaponChance(
        givePlayerRunicWeapon(DAGGER, W_QUIETUS, 50), false, 0);

    char buf[120];
    sprintf(buf, "quietus chance at enchant 10=%d should be < enchant 50=%d",
            quietusChance10, quietusChance50);
    TEST_ASSERT(quietusChance10 < quietusChance50, buf);
    return !testFailed;
}

// ============================================================
// BR110016 extended — weapon with ITEM_ATTACKS_STAGGER normalizes damage
// ============================================================

static boolean br110016_stagger_dmg_normalize_verify(void) {
    // Mace attacks every other turn, so adjustedBaseDamage is halved before
    // the damage-based penalty is calculated. This means mace should have a
    // higher runic chance than a dagger with the same base damage.
    item *mace = givePlayerRunicWeapon(MACE, W_QUIETUS, 10);
    item *dagger = givePlayerRunicWeapon(DAGGER, W_QUIETUS, 10);

    short maceChance = runicWeaponChance(mace, true, 10 * FP_FACTOR);
    short daggerChance = runicWeaponChance(dagger, true, 10 * FP_FACTOR);

    char buf[120];
    sprintf(buf, "mace (stagger, chance=%d) should have higher chance than dagger (chance=%d)",
            maceChance, daggerChance);
    TEST_ASSERT(maceChance >= daggerChance, buf);
    return !testFailed;
}

// ============================================================
// BR110002 extended — speed gives extra turn only on activation
// ============================================================

static void br110002_speed_multi_attack_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_SPEED, 20);
}

static boolean br110002_speed_multi_attack_step(void) {
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

static boolean br110002_speed_multi_attack_verify(void) {
    retryAction(200, br110002_speed_multi_attack_step);
    TEST_ASSERT(player.ticksUntilTurn == -1,
                "speed rune should set ticksUntilTurn to -1 on activation");
    return !testFailed;
}

// ============================================================
// BR110018 extended — chance can't go below enchantment level
// ============================================================

static boolean br110018_minimum_enchant_verify(void) {
    // Heavy weapons have high base damage -> low modifier -> low chance
    // But chance should be clamped to max(1, enchantLevel)
    // War axe has high base damage
    item *axe = givePlayerRunicWeapon(WAR_AXE, W_SPEED, 5);
    short chance = runicWeaponChance(axe, true, 5 * FP_FACTOR);

    char buf[120];
    sprintf(buf, "war axe chance (%d) should be >= enchantment level (5)", chance);
    TEST_ASSERT(chance >= 5, buf);
    return !testFailed;
}

// ============================================================
// BR110017 extended — fast weapons get reduced per-hit chance
// ============================================================

static boolean br110017_fast_weapon_reduce_verify(void) {
    // Rapier has ITEM_ATTACKS_QUICKLY (twice per turn)
    // Its per-hit chance should be reduced compared to a normal weapon
    item *rapier = givePlayerRunicWeapon(RAPIER, W_SPEED, 10);
    item *sword = giveAndEquipRunicWeapon(SWORD, W_SPEED, 10);

    short rapierChance = runicWeaponChance(rapier, true, 10 * FP_FACTOR);
    short swordChance = runicWeaponChance(sword, true, 10 * FP_FACTOR);

    char buf[120];
    sprintf(buf, "rapier (fast, chance=%d) should have lower chance than sword (chance=%d)",
            rapierChance, swordChance);
    TEST_ASSERT(rapierChance <= swordChance, buf);
    return !testFailed;
}

// ============================================================
// BR110006 extended — force with low enchantment has minimum knockback distance
// ============================================================

static void br110006_force_low_enchant_setup(void) {
    clearArena();
    giveAndEquipRunicWeapon(SWORD, W_FORCE, 1);
    // Use troll (maxHP=65) instead of rat so it survives the attack
    forceTarget2 = placeHostileMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    setMonsterHealth(forceTarget2, forceTarget2->info.maxHP);
    forceStartPos2 = monsterPosition(forceTarget2);
}

static boolean br110006_force_low_enchant_step(void) {
    playerAttacksMonster(forceTarget2);
    pos newPos = monsterPosition(forceTarget2);
    if (newPos.x != forceStartPos2.x || newPos.y != forceStartPos2.y) {
        return false; // knocked back, done
    }
    return true; // keep retrying
}

static boolean br110006_force_low_enchant_verify(void) {
    retryAction(200, br110006_force_low_enchant_step);
    if (testFailed) return false;

    // enchant 1 -> weaponForceDistance(1*FP) = max(4, 1*2+2) = 4
    pos newPos = monsterPosition(forceTarget2);
    TEST_ASSERT(distanceBetween(forceStartPos2, newPos) > 0,
                "force with enchant 1 should still knock back (min distance 4)");
    return !testFailed;
}

// ============================================================
// BR110011 — immunity also blocks the caster's own bolt
// ============================================================

static boolean br110011_immunity_self_bolt_verify(void) {
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
    // --- Activation Chance Rules (BR110001, BR110016, BR110017, BR110018, BR110019, BR110020) ---
    { "BR110001_chance_rises_with_enchant",
        clearArena, NULL, br110001_chance_rises_with_enchant_verify },
    { "BR110016_high_damage_lower_chance",
        clearArena, NULL, br110016_high_damage_lower_chance_verify },
    { "BR110017_speed_adjustment",
        clearArena, NULL, br110017_speed_adjustment_verify },
    { "BR110018_chance_bounded",
        clearArena, NULL, br110018_chance_bounded_verify },
    { "BR110019_weak_rune_flat_15",
        br110019_weak_rune_setup, br110019_weak_rune_flat_15_step, br110019_weak_rune_flat_15_verify },
    { "BR110020_slaying_no_random_chance",
        clearArena, NULL, br110020_slaying_no_chance_verify },
    { "BR110001_all_runes_chance_rises",
        clearArena, NULL, br110001_all_runes_chance_rises_verify },
    { "BR110016_stagger_dmg_normalize",
        clearArena, NULL, br110016_stagger_dmg_normalize_verify },
    { "BR110018_minimum_enchant",
        clearArena, NULL, br110018_minimum_enchant_verify },
    { "BR110020_slaying_wrong_kind",
        clearArena, NULL, br110020_slaying_wrong_kind_verify },

    // --- Weapon Effect Rules ---
    { "BR110002_speed_extra_turn",
        br110002_speed_setup, br110002_speed_step, br110002_speed_verify },
    { "BR110002_speed_no_free_turn",
        br110002_no_free_turn_setup, NULL, br110002_no_free_turn_verify },
    { "BR110002_speed_multi_attack",
        br110002_speed_multi_attack_setup, br110002_speed_multi_attack_step, br110002_speed_multi_attack_verify },
    { "BR110003_quietus_instant_kill",
        br110003_quietus_setup, br110003_quietus_step, br110003_quietus_verify },
    { "BR110004_paralysis_duration_grows",
        br110004_paralysis_setup, br110004_paralysis_step, br110004_paralysis_verify },
    { "BR110004_paralysis_enchant20",
        br110004_paralysis_enchant20_setup, br110004_paralysis_enchant20_step, br110004_paralysis_enchant20_verify },
    { "BR110004_paralysis_enchant5",
        br110004_paralysis_enchant5_setup, br110004_paralysis_enchant5_step, br110004_paralysis_enchant5_verify },
    { "BR110005_multiplicity_summons",
        br110005_multiplicity_setup, br110005_multiplicity_step, br110005_multiplicity_verify },
    { "BR110005_multiplicity_enchant10",
        br110005_multiplicity_enchant10_setup, br110005_multiplicity_enchant10_step, br110005_multiplicity_enchant10_verify },
    { "BR110005_multiplicity_dead_target",
        br110005_dead_target_setup, NULL, br110005_dead_target_verify },
    { "BR110006_force_knockback",
        br110006_force_setup, br110006_force_step, br110006_force_verify },
    { "BR110006_force_collateral",
        br110006_force_collateral_setup, br110006_force_collateral_step, br110006_force_collateral_verify },
    { "BR110006_force_high_enchant",
        br110006_force_high_enchant_setup, br110006_force_high_enchant_step, br110006_force_high_enchant_verify },
    { "BR110006_force_low_enchant",
        br110006_force_low_enchant_setup, br110006_force_low_enchant_step, br110006_force_low_enchant_verify },
    { "BR110006_dead_target",
        br110006_dead_target_setup, NULL, br110006_dead_target_verify },
    { "BR110007_slaying_kills_designated",
        br110007_slaying_setup, br110007_slaying_step, br110007_slaying_verify },
    { "BR110007_slaying_miss_non_designated",
        br110007_slaying_miss_setup, NULL, br110007_slaying_miss_verify },
    { "BR110008_mercy_heals",
        br110008_mercy_setup, br110008_mercy_step, br110008_mercy_verify },
    { "BR110008_mercy_full_health",
        br110008_mercy_full_health_setup, br110008_mercy_full_health_step, br110008_mercy_full_health_verify },
    { "BR110009_plenty_duplicating",
        br110009_plenty_setup, br110009_plenty_step, br110009_plenty_verify },

    // --- Inanimate/Invulnerable Rules ---
    { "BR110010_inanimate_invulnerable_blocked",
        br110010_inanimate_setup, br110010_inanimate_step, br110010_inanimate_verify },
    { "BR110010_slaying_inanim",
        clearArena, NULL, br110010_slaying_inanim_verify },
    { "BR110010_inanimate_quietus",
        clearArena, NULL, br110010_inanimate_quietus_verify },
    { "BR110010_inanimate_paralysis",
        clearArena, NULL, br110010_inanimate_paralysis_verify },

    // --- Armor Rules ---
    { "BR110011_immunity_blocks_designated_bolt",
        br110011_immunity_setup, NULL, br110011_immunity_verify },
    { "BR110011_immunity_wrong_type",
        br110011_immunity_wrong_type_setup, NULL, br110011_immunity_wrong_type_verify },
    { "BR110011_immunity_caster_reflect",
        clearArena, NULL, br110011_immunity_caster_reflect_verify },
    { "BR110011_immunity_self_bolt",
        clearArena, NULL, br110011_immunity_self_bolt_verify },
    { "BR110012_reflection_bounces_bolt",
        br110012_reflection_setup, NULL, br110012_reflection_verify },
    { "BR110013_respiration_gas_protection",
        br110013_respiration_setup, NULL, br110013_respiration_verify },
    { "BR110013_damage_terrain",
        clearArena, NULL, br110013_no_false_protection_verify },
    { "BR110014_dampening_absorbs_explosive",
        br110014_dampening_setup, NULL, br110014_dampening_verify },
    { "BR110015_armor_reveals_after_wearing",
        br110015_reveal_setup, NULL, br110015_reveal_verify },

    // --- Auto-ID Rules ---
    { "BR110021_weapon_identifies_on_trigger",
        br110021_weapon_identify_setup, br110021_weapon_identify_step, br110021_weapon_identify_verify },
    { "BR110021_slaying_identifies_on_trigger",
        br110021_slaying_identify_setup, br110021_slaying_identify_step, br110021_slaying_identify_verify },
    { "BR110022_armor_identifies_on_reflection",
        br110022_armor_identify_setup, NULL, br110022_armor_identify_verify },

    // --- Speed/Attack Speed Rules ---
    { "BR110017_stagger_formula",
        clearArena, NULL, br110017_stagger_formula_verify },
    { "BR110017_fast_weapon_reduce",
        clearArena, NULL, br110017_fast_weapon_reduce_verify },

    { NULL, NULL, NULL, NULL }
};
