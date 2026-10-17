// Tests derived from rules/02-combat.md.
//
// These exercise the parts of combat that are observable headlessly: hit
// resolution, damage, rune-free weapon behaviour, monster states and death.
// Rules about specific weapon/armor runes (which depend on random
// activation chances) and about combat messages that require a rendering
// state are omitted.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

static pos startPos;
static pos targetPos;

// --- shared setup helpers ---

static void clearArena(void) {
    // Clear a wide area so knockback and clone destinations are open floor.
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            setTerrain(posOffset(player.loc, dx, dy), FLOOR);
        }
    }
    setPlayerPos(player.loc);
    clearMonsters();
}

static creature *placeHostileMonster(enum monsterTypes type, pos loc) {
    creature *monst = spawnMonster(type, loc);
    makeMonsterHostile(monst);
    revealCell(loc);
    return monst;
}

static creature *placeVisibleMonster(enum monsterTypes type, pos loc) {
    creature *monst = spawnMonster(type, loc);
    revealCell(loc);
    return monst;
}

// ============================================================
// BR65933 — an attack always hits a paralyzed defender
// ============================================================

static creature *attackTarget;
static int targetHealthBefore;

static void paralyzed_target_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    attackTarget = placeHostileMonster(MK_TROLL, targetPos);
    setMonsterHealth(attackTarget, attackTarget->info.maxHP);
    applyStatusToMonster(attackTarget, STATUS_PARALYZED, 20);
    targetHealthBefore = monsterHealth(attackTarget);
}

static boolean attack_once_step(void) {
    moveRight();
    return false;
}

static boolean target_was_wounded(void) {
    TEST_ASSERT(monsterHealth(attackTarget) < targetHealthBefore,
                "a paralyzed defender should be hit");
    return !testFailed;
}

// ============================================================
// BR66846 — a sleeping defender is caught flat-footed and always hit
// ============================================================

static creature *sleepingTarget;

static void sleeping_target_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    sleepingTarget = placeVisibleMonster(MK_TROLL, targetPos);
    setMonsterHealth(sleepingTarget, sleepingTarget->info.maxHP);
    putMonsterToSleep(sleepingTarget);
    targetHealthBefore = monsterHealth(sleepingTarget);
}

static boolean sleeping_target_was_wounded(void) {
    TEST_ASSERT(monsterHealth(sleepingTarget) < targetHealthBefore,
                "a sleeping defender should be hit");
    return !testFailed;
}

// ============================================================
// BR89965 — a struck sleeping monster wakes and tracks the player
// ============================================================

static boolean struck_monster_woke(void) {
    TEST_ASSERT(!monsterIsAsleep(sleepingTarget), "the struck monster should wake");
    TEST_ASSERT(monsterIsTrackingPlayer(sleepingTarget),
                "the woken monster should begin tracking the player");
    return !testFailed;
}

// ============================================================
// BR62243 — attacking a wandering monster alerts it
// ============================================================

static creature *unawareTarget;

static void unaware_target_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    unawareTarget = placeVisibleMonster(MK_TROLL, targetPos);
    setMonsterHealth(unawareTarget, unawareTarget->info.maxHP);
    makeMonsterUnaware(unawareTarget);
}

static boolean unaware_target_step(void) {
    moveRight();
    return false;
}

static boolean monster_was_alerted(void) {
    TEST_ASSERT(!monsterIsWandering(unawareTarget), "the monster should no longer be unaware");
    TEST_ASSERT(monsterIsTrackingPlayer(unawareTarget),
                "the alerted monster should track the player");
    return !testFailed;
}

// ============================================================
// BR33297 — a sweeping weapon strikes every adjacent enemy
// ============================================================

static creature *sweepTargets[3];
static int sweepHealthBefore[3];

static void sweep_setup(void) {
    clearArena();
    givePlayerItem(WEAPON, AXE, 0);
    equipItemFromPack(WEAPON, AXE);
    static const int offsets[3][2] = { {1, 0}, {0, -1}, {0, 1} };
    for (int i = 0; i < 3; i++) {
        pos loc = posOffset(player.loc, offsets[i][0], offsets[i][1]);
        sweepTargets[i] = placeHostileMonster(MK_RAT, loc);
        setMonsterHealth(sweepTargets[i], sweepTargets[i]->info.maxHP);
        // Paralysis guarantees the hits land so the test only measures reach.
        applyStatusToMonster(sweepTargets[i], STATUS_PARALYZED, 20);
        sweepHealthBefore[i] = monsterHealth(sweepTargets[i]);
    }
}

static boolean sweep_step(void) {
    moveRight();
    return false;
}

static boolean all_adjacent_were_hit(void) {
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT(monsterHealth(sweepTargets[i]) < sweepHealthBefore[i],
                    "the sweeping attack should hit every adjacent enemy");
    }
    return !testFailed;
}

// ============================================================
// BR77279 — a creature immune to weapons takes no melee damage
// ============================================================

static creature *immuneTarget;

static void weapon_immune_setup(void) {
    clearArena();
    givePlayerItem(WEAPON, SWORD, 0);
    equipItemFromPack(WEAPON, SWORD);
    targetPos = posOffset(player.loc, 1, 0);
    immuneTarget = placeHostileMonster(MK_REVENANT, targetPos);
    setMonsterHealth(immuneTarget, immuneTarget->info.maxHP);
    applyStatusToMonster(immuneTarget, STATUS_PARALYZED, 20);
    targetHealthBefore = monsterHealth(immuneTarget);
}

static boolean immune_target_unharmed(void) {
    TEST_ASSERT(monsterHealth(immuneTarget) == targetHealthBefore,
                "a weapon-immune creature should take no melee damage");
    return !testFailed;
}

// ============================================================
// BR81065 — an invulnerable creature cannot be damaged by an attack
// ============================================================

static creature *invulnerableTarget;

static void invulnerable_setup(void) {
    clearArena();
    givePlayerItem(WEAPON, SWORD, 0);
    equipItemFromPack(WEAPON, SWORD);
    targetPos = posOffset(player.loc, 1, 0);
    invulnerableTarget = placeHostileMonster(MK_WARDEN_OF_YENDOR, targetPos);
    setMonsterHealth(invulnerableTarget, invulnerableTarget->info.maxHP);
    applyStatusToMonster(invulnerableTarget, STATUS_PARALYZED, 20);
    targetHealthBefore = monsterHealth(invulnerableTarget);
}

static boolean invulnerable_target_unharmed(void) {
    TEST_ASSERT(monsterHealth(invulnerableTarget) == targetHealthBefore,
                "an invulnerable creature should take no damage");
    return !testFailed;
}

// ============================================================
// BR76528 — a blow that reduces a creature to zero health kills it
// ============================================================

static void lethal_blow_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    attackTarget = placeHostileMonster(MK_RAT, targetPos);
    setMonsterHealth(attackTarget, 1);
    applyStatusToMonster(attackTarget, STATUS_PARALYZED, 20);
}

static boolean monster_was_killed(void) {
    TEST_ASSERT(findMonster(MK_RAT) == NULL, "the rat should have been killed");
    return !testFailed;
}

// ============================================================
// BR31952 — a slain monster drops whatever it was carrying
// ============================================================

static void drop_item_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    attackTarget = placeHostileMonster(MK_RAT, targetPos);
    giveMonsterItem(attackTarget, FOOD, RATION);
    setMonsterHealth(attackTarget, 1);
    applyStatusToMonster(attackTarget, STATUS_PARALYZED, 20);
}

static boolean item_was_dropped(void) {
    TEST_ASSERT(findFloorItem(FOOD, RATION) != NULL,
                "the slain monster should have dropped its item");
    return !testFailed;
}

// ============================================================
// BR20491 — attacking a visible monster with a weapon forfeits the pure mage feat
// ============================================================

static void pure_mage_setup(void) {
    clearArena();
    givePlayerItem(WEAPON, SWORD, 0);
    equipItemFromPack(WEAPON, SWORD);
    targetPos = posOffset(player.loc, 1, 0);
    attackTarget = placeHostileMonster(MK_RAT, targetPos);
}

static boolean pure_mage_forfeited(void) {
    TEST_ASSERT(!playerHasFeat(FEAT_PURE_MAGE),
                "attacking with a weapon should forfeit the pure mage feat");
    return !testFailed;
}

// ============================================================
// BR89326 — attacking an unaware creature forfeits the paladin feat
// ============================================================

static void paladin_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    unawareTarget = placeVisibleMonster(MK_TROLL, targetPos);
    setMonsterHealth(unawareTarget, unawareTarget->info.maxHP);
    makeMonsterUnaware(unawareTarget);
}

static boolean paladin_forfeited(void) {
    TEST_ASSERT(!playerHasFeat(FEAT_PALADIN),
                "attacking an unaware creature should forfeit the paladin feat");
    return !testFailed;
}

// ============================================================
// BR86360 — a staggering weapon knocks its target back one square
// ============================================================

static creature *staggerTarget;

static void stagger_setup(void) {
    clearArena();
    givePlayerItem(WEAPON, MACE, 0);
    equipItemFromPack(WEAPON, MACE);
    targetPos = posOffset(player.loc, 1, 0);
    staggerTarget = placeHostileMonster(MK_TROLL, targetPos);
    setMonsterHealth(staggerTarget, staggerTarget->info.maxHP);
    // A paralyzed defender cannot dodge, so the blow is guaranteed to land.
    applyStatusToMonster(staggerTarget, STATUS_PARALYZED, 20);
    startPos = player.loc;
}

static boolean stagger_step(void) {
    // Attack without ending the turn so the knockback is not undone by the
    // target walking back into place.
    playerAttacksMonster(staggerTarget);
    return false;
}

static boolean target_was_knocked_back(void) {
    pos expected = posOffset(startPos, 2, 0);
    TEST_ASSERT_POS_EQ(monsterPosition(staggerTarget), expected,
                       "a staggering weapon should push the target back one square");
    return !testFailed;
}

// ============================================================
// BR95398 — a monster that flees does so at a quarter health
// ============================================================

static creature *fleeingTarget;

static void flee_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    fleeingTarget = placeHostileMonster(MK_WRAITH, targetPos);
    setMonsterHealth(fleeingTarget, fleeingTarget->info.maxHP);
    // Drop it to exactly a quarter of its maximum health.
    damageMonster(fleeingTarget, fleeingTarget->info.maxHP - fleeingTarget->info.maxHP / 4);
}

static boolean monster_fled(void) {
    TEST_ASSERT(monsterIsFleeing(fleeingTarget),
                "a monster wounded to a quarter health should flee");
    return !testFailed;
}

// ============================================================
// BR74866 — a monster that divides splits in two when struck
// ============================================================

static int monsterCountBefore;

static void splitting_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    attackTarget = placeHostileMonster(MK_BLACK_JELLY, targetPos);
    setMonsterHealth(attackTarget, attackTarget->info.maxHP);
    applyStatusToMonster(attackTarget, STATUS_PARALYZED, 20);
    monsterCountBefore = monsterCount();
}

static boolean monster_divided(void) {
    TEST_ASSERT(monsterCount() > monsterCountBefore,
                "striking a dividing monster should create a clone");
    return !testFailed;
}

// ============================================================
// BR82949 — a weakening attack weakens its victim
// ============================================================

static creature *weakeningAttacker;

static void weakening_setup(void) {
    clearArena();
    // A paralyzed player cannot dodge, so the centipede's blow lands.
    applyStatusToPlayer(STATUS_PARALYZED, 30);
    targetPos = posOffset(player.loc, 1, 0);
    weakeningAttacker = placeHostileMonster(MK_CENTIPEDE, targetPos);
    setMonsterHealth(weakeningAttacker, weakeningAttacker->info.maxHP);
}

static boolean player_was_weakened(void) {
    TEST_ASSERT(isPlayerWeakened(), "a weakening attack should weaken the player");
    return !testFailed;
}

// ============================================================
// BR61676 — an exploding attacker dies in the act of attacking
// ============================================================

static creature *explodingAttacker;

static void kamikaze_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    explodingAttacker = spawnMonster(MK_BLOAT, targetPos);
    makeMonsterHostile(explodingAttacker);
    revealCell(targetPos);
}

static boolean kamikaze_step(void) {
    // Make the bloat attack directly; it dies in the act, and this avoids
    // depending on the random wanderings of a flitting monster.
    monsterAttacksPlayer(explodingAttacker);
    return false;
}

static boolean bloat_was_destroyed(void) {
    TEST_ASSERT(findMonster(MK_BLOAT) == NULL,
                "an exploding attacker should die when it attacks");
    return !testFailed;
}

// ============================================================
// BR57747 — killing a dragon earns the dragonslayer feat
// ============================================================

static creature *dragonTarget;

static void dragonslayer_setup(void) {
    clearArena();
    givePlayerItem(WEAPON, SWORD, 0);
    equipItemFromPack(WEAPON, SWORD);
    targetPos = posOffset(player.loc, 1, 0);
    dragonTarget = placeHostileMonster(MK_DRAGON, targetPos);
    setMonsterHealth(dragonTarget, 1);
    applyStatusToMonster(dragonTarget, STATUS_PARALYZED, 20);
}

static boolean dragonslayer_earned(void) {
    TEST_ASSERT(playerHasFeat(FEAT_DRAGONSLAYER),
                "killing a dragon should earn the dragonslayer feat");
    return !testFailed;
}

// ============================================================
// BR86062 — a blow that kills the player ends the game
// ============================================================

static void player_death_setup(void) {
    clearArena();
    setPlayerHealth(1);
    // A paralyzed player cannot dodge, so the ogre's blow is guaranteed.
    applyStatusToPlayer(STATUS_PARALYZED, 30);
    targetPos = posOffset(player.loc, 1, 0);
    spawnMonster(MK_OGRE, targetPos);
    revealCell(targetPos);
}

static boolean rest_once_step(void) {
    restOneTurn();
    return false;
}

static boolean game_ended(void) {
    TEST_ASSERT(isPlayerDead() || rogue.gameHasEnded,
                "the player's death should end the game");
    return !testFailed;
}

// ============================================================

const testCase tests_combat[] = {
    { "BR65933_paralyzed_defender_always_hit",  paralyzed_target_setup,  attack_once_step,        target_was_wounded },
    { "BR66846_sleeping_defender_always_hit",   sleeping_target_setup,   attack_once_step,        sleeping_target_was_wounded },
    { "BR89965_struck_monster_wakes",           sleeping_target_setup,   attack_once_step,        struck_monster_woke },
    { "BR62243_attacking_alerts_monster",       unaware_target_setup,    unaware_target_step,     monster_was_alerted },
    { "BR33297_sweeping_attack_hits_adjacent",  sweep_setup,             sweep_step,              all_adjacent_were_hit },
    { "BR77279_weapon_immune_takes_no_damage",  weapon_immune_setup,     attack_once_step,        immune_target_unharmed },
    { "BR81065_invulnerable_takes_no_damage",   invulnerable_setup,      attack_once_step,        invulnerable_target_unharmed },
    { "BR76528_lethal_blow_kills_monster",      lethal_blow_setup,       attack_once_step,        monster_was_killed },
    { "BR31952_slain_monster_drops_item",       drop_item_setup,         attack_once_step,        item_was_dropped },
    { "BR20491_attack_forfeits_pure_mage",      pure_mage_setup,         attack_once_step,        pure_mage_forfeited },
    { "BR89326_attack_unaware_forfeits_paladin",paladin_setup,           attack_once_step,        paladin_forfeited },
    { "BR86360_staggering_weapon_knocks_back",  stagger_setup,           stagger_step,            target_was_knocked_back },
    { "BR95398_monster_flees_at_quarter_health",flee_setup,              NULL,                    monster_fled },
    { "BR74866_dividing_monster_splits",        splitting_setup,         attack_once_step,        monster_divided },
    { "BR82949_weakening_attack_weakens_player",weakening_setup,         rest_once_step,          player_was_weakened },
    { "BR61676_exploding_attacker_dies",        kamikaze_setup,          kamikaze_step,           bloat_was_destroyed },
    { "BR57747_killing_dragon_earns_feat",      dragonslayer_setup,      attack_once_step,        dragonslayer_earned },
    { "BR86062_player_death_ends_game",         player_death_setup,      rest_once_step,          game_ended },

    { NULL, NULL, NULL, NULL }
};
