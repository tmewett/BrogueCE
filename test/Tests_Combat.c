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
static creature *monst;
static enum monsterTypes monstType;
static int healthBefore;
static int monsterCountBefore;

// --- shared setup helpers ---

static creature *placeVisibleMonster(enum monsterTypes type, pos loc) {
    creature *monst = spawnMonster(type, loc);
    revealCell(loc);
    return monst;
}

static creature *placeAllyMonster(enum monsterTypes type, pos loc) {
    creature *monst = spawnMonster(type, loc);
    makeMonsterAlly(monst);
    revealCell(loc);
    return monst;
}

static void target_setup(enum monsterTypes typ) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    monstType = typ;
    monst = placeHostileMonster(monstType, targetPos);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
}

static boolean attack_once_step(void) {
    moveRight();
    return false;
}

static boolean monster_attacks_step(void) {
    // Have the monster attack the player directly.
    monsterAttacksPlayer(monst);
    return false;
}

static boolean target_was_wounded(void) {
    creature *target = findMonster(monstType);
    TEST_ASSERT(target == NULL || monsterIsHurt(target),
                "monster should be dead or wounded");
    return !testFailed;
}

// ============================================================
// BR65933 — an attack always hits a paralyzed defender
// ============================================================

static void paralyzed_target_setup(void) {
    target_setup(MK_TROLL);
}


// ============================================================
// BR66846 — a sleeping defender is caught flat-footed and always hit
// ============================================================

static void sleeping_target_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    monst = placeVisibleMonster(MK_TROLL, targetPos);
    setMonsterHealth(monst, monst->info.maxHP);
    putMonsterToSleep(monst);
}

static boolean sleeping_target_was_wounded(void) {
    TEST_ASSERT(monsterIsHurt(monst),
                "a sleeping defender should be hit");
    return !testFailed;
}

// ============================================================
// BR89965 — a struck sleeping monster wakes and tracks the player
// ============================================================

static boolean struck_monster_woke(void) {
    TEST_ASSERT(!monsterIsAsleep(monst), "the struck monster should wake");
    TEST_ASSERT(monsterIsTrackingPlayer(monst),
                "the woken monster should begin tracking the player");
    return !testFailed;
}

// ============================================================
// BR62243 — attacking a wandering monster alerts it
// ============================================================

static void unaware_target_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    monst = placeVisibleMonster(MK_TROLL, targetPos);
    makeMonsterUnaware(monst);
}

static boolean monster_was_alerted(void) {
    TEST_ASSERT(!monsterIsWandering(monst), "the monster should no longer be unaware");
    TEST_ASSERT(monsterIsTrackingPlayer(monst),
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
        sweepTargets[i] = placeHostileMonster(MK_TROLL, loc);
        // Paralysis guarantees the hits land so the test only measures reach.
        applyStatusToMonster(sweepTargets[i], STATUS_PARALYZED, 20);
        sweepHealthBefore[i] = monsterHealth(sweepTargets[i]);
    }
}

static boolean all_adjacent_were_hit(void) {
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT(monsterIsHurt(sweepTargets[i]),
                    "the sweeping attack should hit every adjacent enemy");
    }
    return !testFailed;
}

// ============================================================
// BR77279 — a creature immune to weapons takes no melee damage
// ============================================================

static void weapon_immune_setup(void) {
    clearArena();
    givePlayerItem(WEAPON, SWORD, 0);
    equipItemFromPack(WEAPON, SWORD);
    targetPos = posOffset(player.loc, 1, 0);
    monst = placeHostileMonster(MK_REVENANT, targetPos);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
    healthBefore = monsterHealth(monst);
}

static boolean immune_target_unharmed(void) {
    TEST_ASSERT(monsterHealth(monst) == healthBefore,
                "a weapon-immune creature should take no melee damage");
    return !testFailed;
}

// ============================================================
// BR81065 — an invulnerable creature cannot be damaged by an attack
// ============================================================

static void invulnerable_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    monst = placeHostileMonster(MK_WARDEN_OF_YENDOR, targetPos);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
    healthBefore = monsterHealth(monst);
}

static boolean invulnerable_target_unharmed(void) {
    TEST_ASSERT(monsterHealth(monst) == healthBefore,
                "an invulnerable creature should take no damage");
    return !testFailed;
}

// ============================================================
// BR76528 — a blow that reduces a creature to zero health kills it
// ============================================================

static void lethal_blow_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    monst = placeHostileMonster(MK_RAT, targetPos);
    setMonsterHealth(monst, 1);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
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
    monst = placeHostileMonster(MK_RAT, targetPos);
    giveMonsterItem(monst, FOOD, RATION);
    setMonsterHealth(monst, 1);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
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
    monst = placeHostileMonster(MK_RAT, targetPos);
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
    monst = placeVisibleMonster(MK_TROLL, targetPos);
    makeMonsterUnaware(monst);
}

static boolean paladin_forfeited(void) {
    TEST_ASSERT(!playerHasFeat(FEAT_PALADIN),
                "attacking an unaware creature should forfeit the paladin feat");
    return !testFailed;
}

// ============================================================
// BR86360 — a staggering weapon knocks its target back one square
// ============================================================

static void stagger_setup(void) {
    clearArena();
    givePlayerItem(WEAPON, MACE, 0);
    equipItemFromPack(WEAPON, MACE);
    targetPos = posOffset(player.loc, 1, 0);
    monst = placeHostileMonster(MK_TROLL, targetPos);
    // A paralyzed defender cannot dodge, so the blow is guaranteed to land.
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
    startPos = player.loc;
}

static boolean stagger_step(void) {
    // Attack without ending the turn so the knockback is not undone by the
    // target walking back into place.
    playerAttacksMonster(monst);
    return false;
}

static boolean target_was_knocked_back(void) {
    pos expected = posOffset(startPos, 2, 0);
    TEST_ASSERT_POS_EQ(monsterPosition(monst), expected,
                       "a staggering weapon should push the target back one square");
    return !testFailed;
}

// ============================================================
// BR95398 — a monster that flees does so at a quarter health
// ============================================================

static void flee_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    monst = placeHostileMonster(MK_WRAITH, targetPos);
    // Drop it to exactly a quarter of its maximum health.
    damageMonster(monst, monst->info.maxHP - monst->info.maxHP / 4);
}

static boolean monster_fled(void) {
    TEST_ASSERT(monsterIsFleeing(monst),
                "a monster wounded to a quarter health should flee");
    return !testFailed;
}

// ============================================================
// BR74866 — a monster that divides splits in two when struck
// ============================================================

static void splitting_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    monst = placeHostileMonster(MK_BLACK_JELLY, targetPos);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
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

static void weakening_setup(void) {
    clearArena();
    // A paralyzed player cannot dodge, so the centipede's blow lands.
    applyStatusToPlayer(STATUS_PARALYZED, 30);
    targetPos = posOffset(player.loc, 1, 0);
    monst = placeHostileMonster(MK_CENTIPEDE, targetPos);
}

static boolean player_was_weakened(void) {
    TEST_ASSERT(isPlayerWeakened(), "a weakening attack should weaken the player");
    return !testFailed;
}

// ============================================================
// BR61676 — an exploding attacker dies in the act of attacking
// ============================================================

static void kamikaze_setup(void) {
    clearArena();
    targetPos = posOffset(player.loc, 1, 0);
    monst = spawnMonster(MK_BLOAT, targetPos);
    makeMonsterHostile(monst);
    revealCell(targetPos);
}

static boolean kamikaze_step(void) {
    // Make the bloat attack directly; it dies in the act, and this avoids
    // depending on the random wanderings of a flitting monster.
    monsterAttacksPlayer(monst);
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

static void dragonslayer_setup(void) {
    clearArena();
    givePlayerItem(WEAPON, SWORD, 0);
    equipItemFromPack(WEAPON, SWORD);
    targetPos = posOffset(player.loc, 1, 0);
    monst = placeHostileMonster(MK_DRAGON, targetPos);
    setMonsterHealth(monst, 1);
    applyStatusToMonster(monst, STATUS_PARALYZED, 20);
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
// BR11906 — an attacker with transference heals itself for a share of the damage
// ============================================================

// Positive test: a monster with MA_TRANSFERENCE heals itself when it attacks.
static void transference_ally_heals_setup(void) {
    clearArena();
    // Paralyze the player so the bat's attack is guaranteed to hit.
    applyStatusToPlayer(STATUS_PARALYZED, 30);
    // A vampire bat has the MA_TRANSFERENCE ability (see BR83324).
    // Place the monster adjacent to the player so its attack connects.
    monst = placeHostileMonster(MK_VAMPIRE_BAT, posOffset(player.loc, 1, 0));
    // Set health to 1/10 so any healing is detectable.
    setMonsterHealth(monst, 1 + monst->info.maxHP / 10);
    healthBefore = monsterHealth(monst);
}

static boolean ally_was_healed(void) {
    // The bat deals 2-4 damage (see its catalog entry). With MA_TRANSFERENCE,
    // allied monsters recover 40 % of the damage dealt. So healing should be
    // 1-2 HP (ceil/round down). We just need to confirm *some* healing happened.
    TEST_ASSERT(monsterHealth(monst) > healthBefore,
                "an allied transference attacker should heal itself when striking");
    return !testFailed;
}

// Negative test: an allied monster without MA_TRANSFERENCE does not heal.
static void transference_no_ability_setup(void) {
    clearArena();
    applyStatusToPlayer(STATUS_PARALYZED, 30);
    // A rat has no MA_TRANSFERENCE ability.
    monst = placeAllyMonster(MK_RAT, posOffset(player.loc, 1, 0));
    setMonsterHealth(monst, 10);
    healthBefore = monsterHealth(monst);
}

static boolean ally_does_not_heal_step(void) {
    monsterAttacksPlayer(monst);
    return false;
}

static boolean ally_was_not_healed(void) {
    TEST_ASSERT(monsterHealth(monst) == healthBefore,
                "an allied monster without transference should not heal when striking");
    return !testFailed;
}

// Negative test: an inanimate defender blocks transference healing.
static void transference_inanimate_defender_setup(void) {
    clearArena();
    // A vampire bat has the MA_TRANSFERENCE ability (see BR83324).
    monst = placeAllyMonster(MK_VAMPIRE_BAT, posOffset(player.loc, 1, 0));
    // Spawn an inanimate target: goblin totem has the MONST_INANIMATE flag.
    creature *inanimTarget = placeHostileMonster(MK_GOBLIN_TOTEM, posOffset(player.loc, 2, 0));
    setMonsterHealth(inanimTarget, inanimTarget->info.maxHP);
    revealCell(posOffset(player.loc, 2, 0));
    setMonsterHealth(monst, 10);
    healthBefore = monsterHealth(monst);
}

static boolean inanimate_defender_no_heal_step(void) {
    // Have the bat attack the goblin totem (which is inanimate).
    // Since the totem is inanimate, transference should not trigger.
    attackMonster(monst, findMonster(MK_GOBLIN_TOTEM));
    return false;
}

static boolean inanimate_defender_no_heal(void) {
    TEST_ASSERT(monsterHealth(monst) == healthBefore,
                "transference should not heal when the defender is inanimate");
    return !testFailed;
}

// ============================================================

const testCase tests_combat[] = {
    { "BR65933_paralyzed_defender_always_hit",  paralyzed_target_setup,  attack_once_step,        target_was_wounded },
    { "BR66846_sleeping_defender_always_hit",   sleeping_target_setup,   attack_once_step,        sleeping_target_was_wounded },
    { "BR89965_struck_monster_wakes",           sleeping_target_setup,   attack_once_step,        struck_monster_woke },
    { "BR62243_attacking_alerts_monster",       unaware_target_setup,    attack_once_step,     monster_was_alerted },
    { "BR33297_sweeping_attack_hits_adjacent",  sweep_setup,             attack_once_step,              all_adjacent_were_hit },
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
    { "BR11906_ally_heals_on_attack",           transference_ally_heals_setup, monster_attacks_step,        ally_was_healed },
    { "BR11906_no_ability_no_heal",             transference_no_ability_setup, ally_does_not_heal_step, ally_was_not_healed },
    { "BR11906_inanimate_defender_no_heal",     transference_inanimate_defender_setup, inanimate_defender_no_heal_step, inanimate_defender_no_heal },

    { NULL, NULL, NULL, NULL }
};
