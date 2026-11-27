// Tests derived from rules/03-status-effects.md.
//
// These exercise the status effects that can be observed headlessly: the
// counters that tick down on the periodic environmental update, searching,
// weakening, haste and slow, poison, starvation, paralysis, entrancement, the
// shield, being stuck ending as soon as a creature is off an entangling tile,
// levitation keeping a creature off pressure plates, and a discordant
// creature attacking the nearest eligible creature.  Rules
// about rendering (light radius, hallucinated names), combat prompts, and
// item/bolt magnitudes other than the staff-scaled entrancement duration
// are omitted.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

// ============================================================
// BR97529 — a player status loses 1 point on the periodic update
// ============================================================

static enum statusEffects decrementingStatus;
static int decrementStart;
static creature *monst;
static int healthBefore;
static pos startPos;

static void status_decrement_setup(enum statusEffects status, int turns) {
    decrementingStatus = status;
    decrementStart = turns;
    applyStatusToPlayer(decrementingStatus, decrementStart);
}

static boolean rest_one_turn_step(void) {
    restOneTurn();
    return false;   // done
}

static boolean status_decremented_by_one(void) {
    TEST_ASSERT_EQ(playerStatusTurns(decrementingStatus), decrementStart - 1,
                   "the status should decrease by 1 on the periodic update");
    return !testFailed;
}

static void telepathy_decrement_setup(void) {
    status_decrement_setup(STATUS_TELEPATHIC,5);
}

static void hallucination_decrement_setup(void) {
    status_decrement_setup(STATUS_HALLUCINATING,5);
}

static void levitation_decrement_setup(void) {
    status_decrement_setup(STATUS_LEVITATING,5);
}

static void confusion_decrement_setup(void) {
    status_decrement_setup(STATUS_CONFUSED,5);
}

static void nausea_decrement_setup(void) {
    status_decrement_setup(STATUS_NAUSEOUS,5);
}

static void paralysis_decrement_setup(void) {
    // A paralyzed player fast-forwards through several turns at once, so the
    // per-update decrement is observed on a monster instead.
    clearMonsters();
    monst = spawnMonster(MK_TROLL, posOffset(player.loc, 4, 0));
    applyStatusToMonster(monst, STATUS_PARALYZED, 50);
}

static boolean monster_paralysis_decremented(void) {
    TEST_ASSERT_EQ(monsterStatusTurns(monst, STATUS_PARALYZED), 49,
                   "a monster's paralysis should decrease by 1 per update");
    return !testFailed;
}

static void darkness_decrement_setup(void) {
    status_decrement_setup(STATUS_DARKNESS,5);
}

static void invisibility_decrement_setup(void) {
    status_decrement_setup(STATUS_INVISIBLE,5);
}

// ============================================================
// BR48230 — the player's invisibility wears off with a message
// ============================================================

static void invisibility_fade_setup(void) {
    applyStatusToPlayer(STATUS_INVISIBLE, 1);
}

static boolean invisibility_faded_message(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_INVISIBLE), 0, "invisibility should have worn off");
    TEST_ASSERT(messageWasShown("you are no longer invisible"),
                "wearing off should produce its message");
    return !testFailed;
}

static void aggravation_decrement_setup(void) {
    status_decrement_setup(STATUS_AGGRAVATING,5);
}

static void fire_immunity_decrement_setup(void) {
    status_decrement_setup(STATUS_IMMUNE_TO_FIRE,5);
}

static void explosion_immunity_decrement_setup(void) {
    status_decrement_setup(STATUS_EXPLOSION_IMMUNITY,5);
}

// ============================================================
// BR91554 — aggravation wakes and alerts creatures
// BR10268 — aggravation strips distance-keeping and
//           corridor-avoiding behaviors
//
// Two monsters of a kind that both keeps its distance and avoids
// fighting in corridors are placed near the player: one asleep and
// one wandering.  The scroll reaches the whole level, so both are
// inside the radius.  Setup remembers that they really carried both
// behaviors, so the clears below cannot pass vacuously.
// ============================================================

static creature *aggravation_sleeper;
static creature *aggravation_wanderer;
static boolean aggravation_startedKeepingDistance;
static boolean aggravation_startedAvoidingCorridors;

static void aggravation_setup(void) {
    setArenaRadius(4, FLOOR);
    enum monsterTypes type = chooseMonsterTypeByProperty(BOLT_NONE,
                                                          MONST_MAINTAINS_DISTANCE, 0,
                                                          MA_AVOID_CORRIDORS, 0);

    aggravation_sleeper = spawnMonster(type, posOffset(getPlayerPos(), 2, 0));
    makeMonsterHostile(aggravation_sleeper);
    putMonsterToSleep(aggravation_sleeper);

    aggravation_wanderer = spawnMonster(type, posOffset(getPlayerPos(), -2, 0));
    makeMonsterHostile(aggravation_wanderer);
    makeMonsterUnaware(aggravation_wanderer);

    aggravation_startedKeepingDistance =
        monsterHasTrait(aggravation_sleeper, MONST_MAINTAINS_DISTANCE)
        && monsterHasTrait(aggravation_wanderer, MONST_MAINTAINS_DISTANCE);
    aggravation_startedAvoidingCorridors =
        monsterHasAbility(aggravation_sleeper, MA_AVOID_CORRIDORS)
        && monsterHasAbility(aggravation_wanderer, MA_AVOID_CORRIDORS);

    queueConfirmYes();
    readScrollOfKind(SCROLL_AGGRAVATE_MONSTER);
}

static boolean br91554_aggravation_wakes_and_alerts(void) {
    TEST_ASSERT(!monsterIsAsleep(aggravation_sleeper),
                "aggravation should wake a sleeping monster");
    TEST_ASSERT(monsterIsTrackingPlayer(aggravation_sleeper),
                "aggravation should alert the awakened monster to the player");
    TEST_ASSERT(!monsterIsWandering(aggravation_wanderer),
                "aggravation should alert a wandering monster");
    TEST_ASSERT(monsterIsTrackingPlayer(aggravation_wanderer),
                "the wandering monster should start tracking the player");
    return !testFailed;
}

static boolean br10268_aggravation_strips_avoidance(void) {
    TEST_ASSERT(aggravation_startedKeepingDistance,
                "setup: both monsters should start out keeping their distance");
    TEST_ASSERT(aggravation_startedAvoidingCorridors,
                "setup: both monsters should start out avoiding corridors");

    TEST_ASSERT(!monsterHasTrait(aggravation_sleeper, MONST_MAINTAINS_DISTANCE),
                "aggravation should clear the awakened monster's distance-keeping");
    TEST_ASSERT(!monsterHasTrait(aggravation_wanderer, MONST_MAINTAINS_DISTANCE),
                "aggravation should clear the wandering monster's distance-keeping");
    TEST_ASSERT(!monsterHasAbility(aggravation_sleeper, MA_AVOID_CORRIDORS),
                "aggravation should clear the awakened monster's corridor avoidance");
    TEST_ASSERT(!monsterHasAbility(aggravation_wanderer, MA_AVOID_CORRIDORS),
                "aggravation should clear the wandering monster's corridor avoidance");
    return !testFailed;
}

// ============================================================
// BR50175 — while the aggravation counter is positive and you are
// not invisible, it adds that many points to your stealth range.
//
// The range is read before and after applying seven points of
// aggravation, so the exact size of the bonus proves the counter is
// added; clearing the counter returns the range to its starting
// value, and invisibility pins the range at 1 no matter how
// aggravated you are.
// ============================================================

static int br50175_baseRange;
static int br50175_aggravatedRange;
static int br50175_clearedRange;
static int br50175_invisibleRange;

static void br50175_aggravation_setup(void) {
    setArenaRadius(4, FLOOR);
    removeStatusFromPlayer(STATUS_AGGRAVATING);
    removeStatusFromPlayer(STATUS_INVISIBLE);
    recomputeLighting();

    br50175_baseRange = playerStealthRange();
    applyStatusToPlayer(STATUS_AGGRAVATING, 7);
    br50175_aggravatedRange = playerStealthRange();
    removeStatusFromPlayer(STATUS_AGGRAVATING);
    br50175_clearedRange = playerStealthRange();
}

static boolean br50175_aggravation_adds_to_stealth_range(void) {
    TEST_ASSERT_EQ(br50175_aggravatedRange - br50175_baseRange, 7,
                   "seven points of aggravation should add seven points to the stealth range");
    TEST_ASSERT_EQ(br50175_clearedRange, br50175_baseRange,
                   "a cleared aggravation counter should add nothing to the stealth range");
    return !testFailed;
}

static void br50175_invisible_setup(void) {
    setArenaRadius(4, FLOOR);
    recomputeLighting();
    applyStatusToPlayer(STATUS_AGGRAVATING, 7);
    applyStatusToPlayer(STATUS_INVISIBLE, 50);
    br50175_invisibleRange = playerStealthRange();
}

static boolean br50175_invisibility_ignores_aggravation(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_AGGRAVATING), 7,
                   "precondition: seven points of aggravation should be in effect");
    TEST_ASSERT_EQ(br50175_invisibleRange, 1,
                   "an invisible player's stealth range should stay 1 even while aggravated");
    return !testFailed;
}

// ============================================================
// BR28518 — telepathy fades with a message
// ============================================================

static void telepathy_fades_setup(void) {
    applyStatusToPlayer(STATUS_TELEPATHIC, 1);
}

static boolean telepathy_faded_message(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_TELEPATHIC), 0, "telepathy should have faded");
    TEST_ASSERT(messageWasShown("preternatural mental sensitivity fades"),
                "fading telepathy should produce its message");
    return !testFailed;
}

// ============================================================
// BR41654 — darkness fades with a message
// ============================================================

static void darkness_fades_setup(void) {
    applyStatusToPlayer(STATUS_DARKNESS, 1);
}

static boolean darkness_faded_message(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_DARKNESS), 0, "darkness should have lifted");
    TEST_ASSERT(messageWasShown("cloak of darkness lifts"),
                "lifting darkness should produce its message");
    return !testFailed;
}

// ============================================================
// BR76412 — consecutive searches accumulate, capped at 5
// ============================================================

static void search_accumulates_setup(void) {
    searchOneTurn();
    searchOneTurn();
    searchOneTurn();
}

static boolean search_counter_is_three(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_SEARCHING), 3,
                   "three consecutive searches should charge the counter to 3");
    return !testFailed;
}

// ============================================================
// BR76412 — a turn without searching resets the counter
// ============================================================

static void search_then_stop_setup(void) {
    searchOneTurn();
    searchOneTurn();
    restOneTurn();
}

static boolean search_counter_reset(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_SEARCHING), 0,
                   "a turn without searching should reset the search counter");
    return !testFailed;
}

// ============================================================
// BR41784 — the fifth consecutive search resets the counter
// ============================================================

static void five_searches_setup(void) {
    for (int i = 0; i < 5; i++) {
        searchOneTurn();
    }
}

static boolean search_counter_cleared(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_SEARCHING), 0,
                   "the fifth consecutive search should reset the counter");
    return !testFailed;
}

// ============================================================
// BR95942 — donning armor sets the counter to a tenth of its armor value
// ============================================================

static int donningExpected;

static void donning_setup(void) {
    wearArmor(LEATHER_ARMOR);
    donningExpected = playerArmorValue() / 10;
}

static boolean donning_counter_set(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_DONNING), donningExpected,
                   "donning should start at one tenth of the armor value");
    return !testFailed;
}

// ============================================================
// BR37051 — removing armor resets the donning counter
// ============================================================

static void removing_armor_setup(void) {
    wearArmor(LEATHER_ARMOR);
    takeOffArmor();
}

static boolean donning_counter_reset(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_DONNING), 0,
                   "removing armor should reset the donning counter");
    return !testFailed;
}

// ============================================================
// BR49074 — weakening adds one weakness and extends the duration
// ============================================================

static void weakness_stacks_setup(void) {
    weakenPlayer(10);
    weakenPlayer(20);
}

static boolean weakness_stacked(void) {
    TEST_ASSERT_EQ(playerWeaknessAmount(), 2, "weakness should stack by 1 each time");
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_WEAKENED), 20,
                   "the weakened duration should keep the larger value");
    return !testFailed;
}

// ============================================================
// BR55648 — when the weakened counter expires the weakness resets
// ============================================================

static void weakness_expires_setup(void) {
    weakenPlayer(1);
}

static boolean weakness_cleared(void) {
    TEST_ASSERT_EQ(playerWeaknessAmount(), 0,
                   "expiring weakness should reset the weakness amount");
    TEST_ASSERT(messageWasShown("strength returns to your muscles"),
                "expiring weakness should produce its message");
    return !testFailed;
}

// ============================================================
// BR50918 — a slowed monster acts at half its normal speed
// ============================================================

static int monsterBaseSpeed;

static void slow_halves_speed_setup(void) {
    clearMonsters();
    monst = spawnMonster(MK_TROLL, posOffset(getPlayerPos(), 2, 0));
    monsterBaseSpeed = monsterMovementSpeed(monst);
    slowMonster(monst, 10);
}

static boolean slow_halves_speed(void) {
    TEST_ASSERT_EQ(monsterMovementSpeed(monst), monsterBaseSpeed * 2,
                   "a slowed monster should take twice as long to act");
    return !testFailed;
}

// ============================================================
// BR28481 — slow ends haste
// ============================================================

static void slow_clears_haste_setup(void) {
    clearMonsters();
    monst = spawnMonster(MK_TROLL, posOffset(getPlayerPos(), 2, 0));
    hasteMonster(monst, 10);
    slowMonster(monst, 10);
}

static boolean slow_clears_haste(void) {
    TEST_ASSERT_EQ(monsterStatusTurns(monst, STATUS_HASTED), 0,
                   "slow should clear haste");
    TEST_ASSERT_EQ(monsterStatusTurns(monst, STATUS_SLOWED), 10, "slow should be set");
    return !testFailed;
}

// ============================================================
// BR25804 — haste ends slow
// ============================================================

static void haste_clears_slow_setup(void) {
    clearMonsters();
    monst = spawnMonster(MK_TROLL, posOffset(getPlayerPos(), 2, 0));
    slowMonster(monst, 10);
    hasteMonster(monst, 10);
}

static boolean haste_clears_slow(void) {
    TEST_ASSERT_EQ(monsterStatusTurns(monst, STATUS_SLOWED), 0,
                   "haste should clear slow");
    TEST_ASSERT_EQ(monsterStatusTurns(monst, STATUS_HASTED), 10, "haste should be set");
    return !testFailed;
}

// ============================================================
// BR40736 — haste sets haste, clears slow, and halves the time
// ============================================================

static void haste_sets_haste_setup(void) {
    clearMonsters();
    monst = spawnMonster(MK_TROLL, posOffset(player.loc, 2, 0));
    monsterBaseSpeed = monsterMovementSpeed(monst);
    slowMonster(monst, 10);
    hasteMonster(monst, 10);
}

static boolean haste_applied(void) {
    TEST_ASSERT_EQ(monsterStatusTurns(monst, STATUS_HASTED), 10, "haste should be set");
    TEST_ASSERT_EQ(monsterStatusTurns(monst, STATUS_SLOWED), 0, "haste should clear slow");
    TEST_ASSERT_EQ(monsterMovementSpeed(monst), monsterBaseSpeed / 2,
                   "a hasted monster should take half as long to act");
    return !testFailed;
}

// ============================================================
// BR22853 — when slow expires the monster returns to its base speed
// ============================================================

static void slow_expires_setup(void) {
    clearMonsters();
    monst = spawnMonster(MK_TROLL, posOffset(player.loc, 2, 0));
    monsterBaseSpeed = monsterMovementSpeed(monst);
    slowMonster(monst, 1);
}

static boolean speed_restored(void) {
    TEST_ASSERT_EQ(monsterStatusTurns(monst, STATUS_SLOWED), 0, "slow should have expired");
    TEST_ASSERT_EQ(monsterMovementSpeed(monst), monsterBaseSpeed,
                   "expiring slow should restore the monster's normal speed");
    return !testFailed;
}

// ============================================================
// BR71884 — when the haste counter reaches 0 the creature's normal
// movement and attack speeds are restored, and for the player the
// time state is resynchronized.
// ============================================================

static int monsterNormalMove, monsterNormalAttack;
static int monsterHastedMove, monsterHastedAttack;
static int playerNormalMove, playerNormalAttack;
static int playerHastedMove, playerHastedAttack;
static int hasteRestsRemaining;

// Status counters tick once per 100-tick environment cycle, but a hasted
// player's rest lasts only 50 ticks, so two rests are needed before one
// full cycle elapses and the haste counter ticks down.
static boolean rest_twice_step(void) {
    restOneTurn();
    return --hasteRestsRemaining > 0;
}

static void monster_haste_expires_setup(void) {
    clearMonsters();
    monst = spawnMonster(MK_TROLL, posOffset(player.loc, 2, 0));
    monsterNormalMove = monsterMovementSpeed(monst);
    monsterNormalAttack = monsterAttackSpeed(monst);
    hasteMonster(monst, 1);
    monsterHastedMove = monsterMovementSpeed(monst);
    monsterHastedAttack = monsterAttackSpeed(monst);
}

static boolean monster_haste_expired(void) {
    TEST_ASSERT_EQ(monsterHastedMove, monsterNormalMove / 2,
                   "precondition: haste should halve the monster's movement time");
    TEST_ASSERT_EQ(monsterHastedAttack, monsterNormalAttack / 2,
                   "precondition: haste should halve the monster's attack time");
    TEST_ASSERT_EQ(monsterStatusTurns(monst, STATUS_HASTED), 0,
                   "the haste counter should have reached 0");
    TEST_ASSERT_EQ(monsterMovementSpeed(monst), monsterNormalMove,
                   "expiring haste should restore the monster's normal movement speed");
    TEST_ASSERT_EQ(monsterAttackSpeed(monst), monsterNormalAttack,
                   "expiring haste should restore the monster's normal attack speed");
    return !testFailed;
}

static void player_haste_expires_setup(void) {
    clearMonsters();
    playerNormalMove = playerMovementSpeed();
    playerNormalAttack = playerAttackSpeed();
    hastePlayer(1);
    playerHastedMove = playerMovementSpeed();
    playerHastedAttack = playerAttackSpeed();
    hasteRestsRemaining = 2;
}

static boolean player_haste_expired(void) {
    TEST_ASSERT_EQ(playerHastedMove, playerNormalMove / 2,
                   "precondition: haste should halve the player's movement time");
    TEST_ASSERT_EQ(playerHastedAttack, playerNormalAttack / 2,
                   "precondition: haste should halve the player's attack time");
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_HASTED), 0,
                   "the haste counter should have reached 0");
    TEST_ASSERT_EQ(playerMovementSpeed(), playerNormalMove,
                   "expiring haste should restore the player's normal movement speed");
    TEST_ASSERT_EQ(playerAttackSpeed(), playerNormalAttack,
                   "expiring haste should restore the player's normal attack speed");
    TEST_ASSERT(messageWasShown("your supernatural speed fades"),
                "expiring haste should announce that the player's speed has faded");
    // The expiry re-anchors the environment-update clock to the player's own
    // action clock; without the resynchronization it would stay on the old
    // cycle (a full 100) instead of matching the player's remaining time.
    TEST_ASSERT_EQ(ticksUntilEnvironmentUpdate(), playerHastedMove,
                   "expiring haste should resynchronize the player's time state");
    return !testFailed;
}

// Negative test: while turns of haste remain, the raised speeds stay in
// effect and no fade message is shown -- restoration happens only at 0.
static void player_haste_still_running_setup(void) {
    clearMonsters();
    playerNormalMove = playerMovementSpeed();
    playerNormalAttack = playerAttackSpeed();
    hastePlayer(2);
    playerHastedMove = playerMovementSpeed();
    playerHastedAttack = playerAttackSpeed();
    hasteRestsRemaining = 2;
}

static boolean player_speeds_still_hasted(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_HASTED), 1,
                   "one rest should leave one turn of haste remaining");
    TEST_ASSERT_EQ(playerMovementSpeed(), playerHastedMove,
                   "movement time should stay halved while haste remains");
    TEST_ASSERT_EQ(playerAttackSpeed(), playerHastedAttack,
                   "attack time should stay halved while haste remains");
    TEST_ASSERT(!messageWasShown("your supernatural speed fades"),
                "no fade message should appear while haste remains");
    return !testFailed;
}

// ============================================================
// BR37844 — recomputing encumbrance keeps the player's slow multiplier
// ============================================================

static int br37844BaseMove;
static int br37844BaseAttack;
static int br37844HastedMove;

// Equipping armor recomputes the player's encumbrance.  The recomputation
// must reapply the haste/slow multiplier rather than resetting the player to
// their base speeds.
static void hasted_encumbrance_recompute_setup(void) {
    clearMonsters();
    br37844BaseMove = playerMovementSpeed();
    br37844BaseAttack = playerAttackSpeed();
    hastePlayer(50);
    br37844HastedMove = playerMovementSpeed();
    wearArmor(LEATHER_ARMOR);
}

static boolean hasted_speed_survives_encumbrance_recompute(void) {
    TEST_ASSERT_EQ(br37844HastedMove, br37844BaseMove / 2,
                   "precondition: haste should halve the player's movement time");
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_HASTED), 50,
                   "equipping armor should not end the player's haste");
    TEST_ASSERT_EQ(playerMovementSpeed(), br37844BaseMove / 2,
                   "recomputing encumbrance while hasted should keep movement time halved");
    TEST_ASSERT_EQ(playerAttackSpeed(), br37844BaseAttack / 2,
                   "recomputing encumbrance while hasted should keep attack time halved");
    return !testFailed;
}

static void slowed_encumbrance_recompute_setup(void) {
    clearMonsters();
    br37844BaseMove = playerMovementSpeed();
    br37844BaseAttack = playerAttackSpeed();
    slowPlayer(50);
    wearArmor(LEATHER_ARMOR);
}

static boolean slowed_speed_survives_encumbrance_recompute(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_SLOWED), 50,
                   "equipping armor should not end the player's slow");
    TEST_ASSERT_EQ(playerMovementSpeed(), br37844BaseMove * 2,
                   "recomputing encumbrance while slowed should double movement time");
    TEST_ASSERT_EQ(playerAttackSpeed(), br37844BaseAttack * 2,
                   "recomputing encumbrance while slowed should double attack time");
    return !testFailed;
}

// Control: without haste or slow, recomputing encumbrance leaves the
// player's base speeds unchanged, so the multiplier comes from the status.
static void normal_encumbrance_recompute_setup(void) {
    clearMonsters();
    br37844BaseMove = playerMovementSpeed();
    br37844BaseAttack = playerAttackSpeed();
    wearArmor(LEATHER_ARMOR);
}

static boolean normal_speed_survives_encumbrance_recompute(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_HASTED), 0,
                   "the control player should not be hasted");
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_SLOWED), 0,
                   "the control player should not be slowed");
    TEST_ASSERT_EQ(playerMovementSpeed(), br37844BaseMove,
                   "recomputing encumbrance without haste or slow should keep normal movement time");
    TEST_ASSERT_EQ(playerAttackSpeed(), br37844BaseAttack,
                   "recomputing encumbrance without haste or slow should keep normal attack time");
    return !testFailed;
}

// ============================================================
// BR64646 — poison adds concentration and duration
// ============================================================

static void poison_added_setup(void) {
    poisonPlayer(10, 1);
}

static boolean poison_added(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_POISONED), 10, "poison duration should be added");
    TEST_ASSERT_EQ(playerPoisonConcentration(), 1, "poison concentration should increase by 1");
    return !testFailed;
}

// ============================================================
// BR75638 — poison damages by its concentration and ticks down
// ============================================================

static void poison_damages_setup(void) {
    setPlayerHealth(playerMaxHealth());
    healthBefore = playerHealth();
    poisonPlayer(3, 1);
}

static boolean poison_damaged_and_ticked(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_POISONED), 2,
                   "the poison counter should decrease by 1");
    TEST_ASSERT_EQ(playerHealth(), healthBefore - 1,
                   "the player should take damage equal to the poison concentration");
    return !testFailed;
}

// ============================================================
// BR52651 — a panacea-style heal clears poison
// ============================================================

static void panacea_clears_poison_setup(void) {
    poisonPlayer(10, 2);
    healPlayerPanacea();
}

static boolean poison_cleared(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_POISONED), 0, "a panacea should clear poison duration");
    TEST_ASSERT_EQ(playerPoisonConcentration(), 0, "a panacea should clear poison concentration");
    return !testFailed;
}

// ============================================================
// BR33833 — a panacea-style heal caps several statuses
// ============================================================

static void panacea_caps_statuses_setup(void) {
    applyStatusToPlayer(STATUS_HALLUCINATING, 10);
    applyStatusToPlayer(STATUS_CONFUSED, 10);
    applyStatusToPlayer(STATUS_NAUSEOUS, 10);
    applyStatusToPlayer(STATUS_DARKNESS, 10);
    healPlayerPanacea();
}

static boolean statuses_capped(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_HALLUCINATING), 1, "hallucination should cap at 1");
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_CONFUSED), 1, "confusion should cap at 1");
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_NAUSEOUS), 1, "nausea should cap at 1");
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_DARKNESS), 0, "darkness should be cleared");
    return !testFailed;
}

// ============================================================
// BR51327 — fire immunity extinguishes burning
// ============================================================

static void immunity_extinguishes_setup(void) {
    applyStatusToPlayer(STATUS_BURNING, 10);
    drink(POTION_FIRE_IMMUNITY);
}

static boolean burning_extinguished(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_BURNING), 0,
                   "gaining fire immunity should extinguish burning");
    return !testFailed;
}

// ============================================================
// BR25686 — nutrition starts at the stomach size
// ============================================================

// The setup is deliberately empty: the test must observe the nutrition a new
// game actually gives the player, not a value the test wrote itself.
static boolean nutrition_is_full(void) {
    TEST_ASSERT_EQ(playerNutrition(), 2150,
                   "a new game should start the player at full nutrition (2150)");
    return !testFailed;
}

// ============================================================
// BR49432 — nutrition decreases on the periodic update
// ============================================================

static void nutrition_decreases_setup(void) {
    setPlayerNutrition(1000);
}

static boolean nutrition_decreased(void) {
    TEST_ASSERT_EQ(playerNutrition(), 999, "nutrition should decrease by 1");
    return !testFailed;
}

// ============================================================
// BR24204 — a paralyzed player expends no nutrition
// ============================================================

static void paralyzed_no_nutrition_setup(void) {
    applyStatusToPlayer(STATUS_PARALYZED, 50);
    setPlayerNutrition(1000);
}

static boolean nutrition_unchanged(void) {
    TEST_ASSERT_EQ(playerNutrition(), 1000,
                   "a paralyzed player should not expend nutrition");
    return !testFailed;
}

// ============================================================
// BR80877 — starvation costs 1 health per turn
// ============================================================

static void starvation_setup(void) {
    removePackFood();
    setPlayerNutrition(0);
    healthBefore = playerHealth();
}

static boolean starved_one_point(void) {
    TEST_ASSERT_EQ(playerHealth(), healthBefore - 1,
                   "starvation should cost 1 health per turn");
    return !testFailed;
}

// ============================================================
// BR32368 — crossing the hunger threshold warns the player
// ============================================================

static void hunger_warning_setup(void) {
    removePackFood();
    setPlayerNutrition(HUNGER_THRESHOLD + 1);
}

static boolean hunger_warning_shown(void) {
    TEST_ASSERT(messageWasShown("you are hungry"),
                "crossing the hunger threshold should warn the player");
    return !testFailed;
}

// ============================================================
// BR10266 — while levitating, the player does not discover the secrets of
// the tile they stand on.
//
// The player is placed on a hidden gas vent: a passable secret tile that is
// neither a trap nor an obstruction, so the only engine path that can reveal
// it is the "discover the secrets of the tile you stand on" rule.  A
// levitating player must leave the vent hidden; the identical non-levitating
// control must reveal it, so the levitating case is not passing vacuously.
// ============================================================

static pos br10266SecretLoc;

static void br10266_setup(boolean levitating) {
    br10266SecretLoc = posOffset(getPlayerPos(), 1, 0);
    setTerrain(br10266SecretLoc, MACHINE_POISON_GAS_VENT_HIDDEN);
    // The tile has to be in view for its secret to be discovered.
    revealCell(br10266SecretLoc);
    setPlayerPos(br10266SecretLoc);
    if (levitating) {
        applyStatusToPlayer(STATUS_LEVITATING, 20);
    }
    applyTileEffectsToPlayer();
}

static void br10266_levitating_setup(void) {
    br10266_setup(true);
}

static boolean br10266_levitating_keeps_secret(void) {
    TEST_ASSERT(isPlayerLevitating(), "setup: the player should be levitating");
    TEST_ASSERT(playerIsAt(br10266SecretLoc),
                "setup: the player should stand on the hidden vent");
    TEST_ASSERT(cellIsTerrain(br10266SecretLoc, MACHINE_POISON_GAS_VENT_HIDDEN),
                "a levitating player should not discover the hidden vent they stand on");
    return !testFailed;
}

static void br10266_control_setup(void) {
    br10266_setup(false);
}

static boolean br10266_discovers_secret(void) {
    TEST_ASSERT(!isPlayerLevitating(),
                "setup: the control player should not be levitating");
    TEST_ASSERT(playerIsAt(br10266SecretLoc),
                "setup: the player should stand on the hidden vent");
    TEST_ASSERT(!cellIsTerrain(br10266SecretLoc, MACHINE_POISON_GAS_VENT_HIDDEN),
                "a non-levitating player should discover the hidden vent they stand on");
    TEST_ASSERT(cellIsTerrain(br10266SecretLoc, MACHINE_POISON_GAS_VENT_DORMANT),
                "the discovered vent should be revealed as an inactive gas vent");
    return !testFailed;
}

// ============================================================
// BR27834 — a levitating creature does not extinguish its own burning by
// entering fire-extinguishing terrain such as water.  The control proves the
// water really does extinguish an otherwise identical burning player who is
// not levitating, so the levitating case is not passing vacuously.
// ============================================================

static void br27834_levitating_setup(void) {
    setTerrain(getPlayerPos(), SHALLOW_WATER);
    applyStatusToPlayer(STATUS_BURNING, 10);
    applyStatusToPlayer(STATUS_LEVITATING, 20);
    applyTileEffectsToPlayer();
}

static boolean br27834_levitating_keeps_burning(void) {
    TEST_ASSERT(isPlayerLevitating(), "setup: the player should be levitating");
    TEST_ASSERT(isPlayerBurning(),
                "a levitating player should not be extinguished by water");
    return !testFailed;
}

static void br27834_control_setup(void) {
    setTerrain(getPlayerPos(), SHALLOW_WATER);
    applyStatusToPlayer(STATUS_BURNING, 10);
    applyTileEffectsToPlayer();
}

static boolean br27834_water_extinguishes(void) {
    TEST_ASSERT(!isPlayerLevitating(),
                "setup: the control player should not be levitating");
    TEST_ASSERT(!isPlayerBurning(),
                "water should extinguish a non-levitating player's burning");
    return !testFailed;
}

// ============================================================
// BR47057 — a creature exposed to fire catches fire, announced
// as "you catch fire" for the player, unless it is already
// dying, immune to fire, invulnerable, submerged, or standing
// on fire-extinguishing terrain without levitating.
//
// Each exemption test spawns an otherwise identical control
// creature beside the exempt one and exposes both to the same
// flame.  The control must catch fire, so an exemption that
// simply skipped the exposure cannot pass.
// ============================================================

static creature *br47057_subject;
static creature *br47057_control;

static void br47057_spawn_pair(void) {
    clearMonsters();
    setArenaRadius(4, FLOOR);
    br47057_subject = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 2, 0));
    br47057_control = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 0, 2));
    TEST_ASSERT(br47057_subject != NULL && br47057_control != NULL,
                "setup: both creatures should spawn");
}

static void br47057_player_setup(void) {
    clearMonsters();
    setArenaRadius(4, FLOOR);
    clearMessages();
    exposePlayerToFire();
    flushCombatMessages();
}

static boolean br47057_player_catches_fire_and_is_told(void) {
    TEST_ASSERT(isPlayerBurning(), "exposure to fire should set the player alight");
    TEST_ASSERT(messageWasShown("you catch fire"),
                "the player should be told \"you catch fire\"");
    return !testFailed;
}

static void br47057_ordinary_setup(void) {
    br47057_spawn_pair();
}

static boolean br47057_ordinary_creature_catches_fire(void) {
    exposeMonsterToFire(br47057_subject);
    TEST_ASSERT(monsterHasStatus(br47057_subject, STATUS_BURNING),
                "fire should set an ordinary creature burning");
    return !testFailed;
}

static void br47057_dying_setup(void) {
    br47057_spawn_pair();
    makeMonsterDying(br47057_subject);
    TEST_ASSERT(monsterIsDead(br47057_subject),
                "setup: the subject should already be dying");
}

static boolean br47057_dying_creature_does_not_catch_fire(void) {
    exposeMonsterToFire(br47057_subject);
    exposeMonsterToFire(br47057_control);
    TEST_ASSERT(!monsterHasStatus(br47057_subject, STATUS_BURNING),
                "fire should not set an already dying creature burning");
    TEST_ASSERT(monsterHasStatus(br47057_control, STATUS_BURNING),
                "the same fire should set the control creature burning");
    return !testFailed;
}

static void br47057_immune_setup(void) {
    br47057_spawn_pair();
    applyStatusToMonster(br47057_subject, STATUS_IMMUNE_TO_FIRE, 100);
    TEST_ASSERT(monsterHasStatus(br47057_subject, STATUS_IMMUNE_TO_FIRE),
                "setup: the subject should be immune to fire");
}

static boolean br47057_fire_immune_creature_does_not_catch_fire(void) {
    exposeMonsterToFire(br47057_subject);
    exposeMonsterToFire(br47057_control);
    TEST_ASSERT(!monsterHasStatus(br47057_subject, STATUS_BURNING),
                "fire should not set a fire-immune creature burning");
    TEST_ASSERT(monsterHasStatus(br47057_control, STATUS_BURNING),
                "the same fire should set the control creature burning");
    return !testFailed;
}

static void br47057_invulnerable_setup(void) {
    clearMonsters();
    setArenaRadius(4, FLOOR);
    br47057_subject = placeHostileMonster(MK_WARDEN_OF_YENDOR,
                                          posOffset(getPlayerPos(), 2, 0));
    br47057_control = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 0, 2));
    TEST_ASSERT(br47057_subject != NULL && br47057_control != NULL,
                "setup: both creatures should spawn");
    TEST_ASSERT(monsterHasTrait(br47057_subject, MONST_INVULNERABLE),
                "setup: the subject should be invulnerable");
}

static boolean br47057_invulnerable_creature_does_not_catch_fire(void) {
    exposeMonsterToFire(br47057_subject);
    exposeMonsterToFire(br47057_control);
    TEST_ASSERT(!monsterHasStatus(br47057_subject, STATUS_BURNING),
                "fire should not set an invulnerable creature burning");
    TEST_ASSERT(monsterHasStatus(br47057_control, STATUS_BURNING),
                "the same fire should set the control creature burning");
    return !testFailed;
}

static void br47057_submerged_setup(void) {
    br47057_spawn_pair();
    setMonsterSubmerged(br47057_subject, true);
    TEST_ASSERT(monsterIsSubmerged(br47057_subject),
                "setup: the subject should be submerged");
}

static boolean br47057_submerged_creature_does_not_catch_fire(void) {
    exposeMonsterToFire(br47057_subject);
    exposeMonsterToFire(br47057_control);
    TEST_ASSERT(!monsterHasStatus(br47057_subject, STATUS_BURNING),
                "fire should not set a submerged creature burning");
    TEST_ASSERT(monsterHasStatus(br47057_control, STATUS_BURNING),
                "the same fire should set the control creature burning");
    return !testFailed;
}

static void br47057_water_setup(void) {
    clearMonsters();
    setArenaRadius(4, FLOOR);
    pos subjectPos = posOffset(getPlayerPos(), 2, 0);
    pos controlPos = posOffset(getPlayerPos(), 0, 2);
    setTerrain(subjectPos, SHALLOW_WATER);
    setTerrain(controlPos, SHALLOW_WATER);
    br47057_subject = placeHostileMonster(MK_RAT, subjectPos);
    br47057_control = placeHostileMonster(MK_RAT, controlPos);
    applyStatusToMonster(br47057_control, STATUS_LEVITATING, 20);
    TEST_ASSERT(br47057_subject != NULL && br47057_control != NULL,
                "setup: both creatures should spawn");
    TEST_ASSERT(cellIsTerrain(monsterPosition(br47057_subject), SHALLOW_WATER),
                "setup: the subject should stand in fire-extinguishing terrain");
    TEST_ASSERT(monsterHasStatus(br47057_control, STATUS_LEVITATING),
                "setup: the control should be levitating");
}

static boolean br47057_water_exempts_non_levitating_creature(void) {
    exposeMonsterToFire(br47057_subject);
    exposeMonsterToFire(br47057_control);
    TEST_ASSERT(!monsterHasStatus(br47057_subject, STATUS_BURNING),
                "fire should not set alight a creature standing on fire-extinguishing terrain");
    TEST_ASSERT(monsterHasStatus(br47057_control, STATUS_BURNING),
                "the same fire should set alight a levitating creature above that terrain");
    return !testFailed;
}

// ============================================================
// BR38788 — while levitating, a creature does not trigger
// pressure plates.  The control proves an otherwise identical
// non-levitating player does depress the same plate, so the
// levitating case is not passing vacuously.
// ============================================================

static pos br38788_plate;

static void br38788_setup(boolean levitating) {
    clearArena();
    br38788_plate = posOffset(getPlayerPos(), 1, 0);
    setTerrain(br38788_plate, MACHINE_PRESSURE_PLATE);
    setPlayerPos(br38788_plate);
    if (levitating) {
        applyStatusToPlayer(STATUS_LEVITATING, 20);
    }
    applyTileEffectsToPlayer();
}

static void br38788_levitating_setup(void) {
    br38788_setup(true);
}

static boolean br38788_levitating_leaves_plate_up(void) {
    TEST_ASSERT(isPlayerLevitating(), "setup: the player should be levitating");
    TEST_ASSERT(playerIsAt(br38788_plate), "setup: the player should stand on the plate");
    TEST_ASSERT(!isTrapTriggered(br38788_plate),
                "a levitating creature should not trigger a pressure plate");
    return !testFailed;
}

static void br38788_control_setup(void) {
    br38788_setup(false);
}

static boolean br38788_control_depresses_plate(void) {
    TEST_ASSERT(!isPlayerLevitating(),
                "setup: the control player should not be levitating");
    TEST_ASSERT(playerIsAt(br38788_plate), "setup: the player should stand on the plate");
    TEST_ASSERT(isTrapTriggered(br38788_plate),
                "control: a non-levitating creature should trigger a pressure plate");
    return !testFailed;
}

// ============================================================
// BR73170 — a levitating player is not underwater over deep water
// ============================================================

static void levitating_over_water_setup(void) {
    setTerrain(player.loc, DEEP_WATER);
    applyStatusToPlayer(STATUS_LEVITATING, 20);
    applyTileEffectsToPlayer();
}

static boolean not_underwater(void) {
    TEST_ASSERT(!isPlayerSubmerged(),
                "a levitating player should not be underwater over deep water");
    return !testFailed;
}

// ============================================================
// BR98623 — while levitating, location flavor text is shown
// instead of terrain flavor text.
//
// The player stands on marble, whose terrain flavor is the long
// sentence about crystals; a levitating player instead sees the
// location description "you are hovering above the marble ground."
// The non-levitating control proves the same tile shows its terrain
// flavor, so the levitating case is not passing vacuously.
// ============================================================

static void br98623_levitating_setup(void) {
    setTerrain(getPlayerPos(), MARBLE_FLOOR);
    applyStatusToPlayer(STATUS_LEVITATING, 20);
    refreshFlavorText();
}

static boolean br98623_levitating_shows_location_flavor(void) {
    const char *flavor = flavorTextLine();
    TEST_ASSERT(strstr(flavor, "hovering above the marble ground") != NULL,
                "a levitating player should see the location's flavor text");
    TEST_ASSERT(strstr(flavor, "grain of the lavish marble") == NULL,
                "a levitating player should not see the terrain's flavor text");
    return !testFailed;
}

static void br98623_terrain_setup(void) {
    setTerrain(getPlayerPos(), MARBLE_FLOOR);
    refreshFlavorText();
}

static boolean br98623_shows_terrain_flavor(void) {
    const char *flavor = flavorTextLine();
    TEST_ASSERT(strstr(flavor, "grain of the lavish marble") != NULL,
                "a non-levitating player should see the terrain's flavor text");
    TEST_ASSERT(strstr(flavor, "hovering above") == NULL,
                "a non-levitating player should not see the location's flavor text");
    return !testFailed;
}

// ============================================================
// BR62028 — a paralyzed monster takes no action
// ============================================================

static void paralyzed_monster_idle_setup(void) {
    clearArena();
    monst = spawnMonster(MK_TROLL, posOffset(player.loc, 2, 0));
    makeMonsterHostile(monst);
    applyStatusToMonster(monst, STATUS_PARALYZED, 50);
    startPos = monsterPosition(monst);
}

static boolean paralyzed_monster_did_nothing(void) {
    TEST_ASSERT_POS_EQ(monsterPosition(monst), startPos,
                       "a paralyzed monster should not move");
    return !testFailed;
}

// ============================================================
// BR34867 — an entranced monster skips its turns
// ============================================================

static void entranced_monster_idle_setup(void) {
    clearMonsters();
    monst = spawnMonster(MK_TROLL, posOffset(player.loc, 2, 0));
    makeMonsterHostile(monst);
    setTerrainAroundLoc(player.loc, FLOOR);
    applyStatusToMonster(monst, STATUS_ENTRANCED, 50);
    startPos = monsterPosition(monst);
}

static boolean entranced_monster_did_nothing(void) {
    TEST_ASSERT_POS_EQ(monsterPosition(monst), startPos,
                       "an entranced monster should not move");
    return !testFailed;
}

// ============================================================
// BR73717 — attacking a paralyzed creature clears its paralysis
// ============================================================

static void attack_paralyzed_setup(void) {
    clearArena();
    monst = placeHostileMonster(MK_KRAKEN, posOffset(player.loc, 1, 0));
    applyStatusToMonster(monst, STATUS_PARALYZED, 50);
}

static boolean attack_paralyzed_step(void) {
    moveRight();
    return false; // end test
}

static boolean paralysis_cleared(void) {
    TEST_ASSERT(!monsterHasStatus(monst, STATUS_PARALYZED),
                "hitting a paralyzed creature should clear its paralysis");
    return !testFailed;
}

// ============================================================
// BR68621 — the shield loses a twentieth of its maximum per update
// ============================================================

static void shield_decays_setup(void) {
    applyStatusToPlayer(STATUS_SHIELDED, 200);
}

static boolean shield_decayed(void) {
    TEST_ASSERT_EQ(playerShield(), 190,
                   "the shield should lose one twentieth of its maximum");
    return !testFailed;
}

// ============================================================
// BR23913 — being stuck ends immediately for a creature no longer standing
// on an entangling tile
// ============================================================

static creature *br23913_subject;

static void br23913_player_setup(void) {
    clearArena();
    pos here = getPlayerPos();
    setTerrain(here, SPIDERWEB);
    setPlayerPos(here);
    applyStatusToPlayer(STATUS_STUCK, 10);
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_STUCK), 10,
                   "setup: the player should start out stuck in the web");
}

static boolean br23913_player_step(void) {
    // Step off the web onto the open floor beside it, then let a turn pass.
    setPlayerPos(posOffset(getPlayerPos(), 1, 0));
    restOneTurn();
    return false;
}

static boolean br23913_player_stuck_ends(void) {
    TEST_ASSERT(!playerIsOnTerrain(SPIDERWEB),
                "control: the player is no longer standing on the web");
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_STUCK), 0,
                   "a player no longer on an entangling tile stops being stuck at once");
    return !testFailed;
}

static void br23913_monster_setup(void) {
    setArenaRadius(4, FLOOR);
    pos webLoc = posOffset(getPlayerPos(), 3, 0);
    setTerrain(webLoc, SPIDERWEB);
    br23913_subject = spawnMonster(MK_RAT, webLoc);
    makeMonsterHostile(br23913_subject);
    applyStatusToMonster(br23913_subject, STATUS_STUCK, 10);
    TEST_ASSERT_EQ(monsterStatusTurns(br23913_subject, STATUS_STUCK), 10,
                   "setup: the rat should start out stuck in the web");
}

static boolean br23913_monster_step(void) {
    // Walk the rat off the web onto the open floor beside it, then let a
    // turn pass.  The web lies behind the rat, away from the player, so its
    // own turn cannot carry it back onto the tile.
    setMonsterPos(br23913_subject, posOffset(monsterPosition(br23913_subject), -1, 0));
    restOneTurn();
    return false;
}

static boolean br23913_monster_stuck_ends(void) {
    TEST_ASSERT(monsterIsAlive(br23913_subject), "control: the rat survives the wait");
    TEST_ASSERT(!cellIsTerrain(monsterPosition(br23913_subject), SPIDERWEB),
                "control: the rat is no longer standing on an entangling tile");
    TEST_ASSERT_EQ(monsterStatusTurns(br23913_subject, STATUS_STUCK), 0,
                   "a monster no longer on an entangling tile stops being stuck at once");
    return !testFailed;
}

// ============================================================
// BR45966 — the player refuses to step into lava with the message
// "that would be certain death!" unless levitating or immune to
// fire.
//
// The negative test approaches a known lava tile with neither
// exception: the step must be cancelled and the warning shown.
// The two positive tests grant each exception the rule names —
// levitation, then fire immunity — and must walk straight onto
// the same tile with no warning.  The fire-immunity case doubles
// as a survival check, since lava also kills on contact.
// ============================================================

static pos br45966_start;
static pos br45966_lava;

static void br45966_lava_setup(void) {
    clearArena();
    br45966_start = getPlayerPos();
    br45966_lava = posOffset(br45966_start, 1, 0);
    setTerrain(br45966_lava, LAVA);
    revealCell(br45966_lava); // the refusal only applies to known lava
}

static void br45966_levitating_setup(void) {
    br45966_lava_setup();
    applyStatusToPlayer(STATUS_LEVITATING, 20);
}

static void br45966_fire_immune_setup(void) {
    br45966_lava_setup();
    applyStatusToPlayer(STATUS_IMMUNE_TO_FIRE, 20);
}

static boolean br45966_step_right(void) {
    moveRight();
    return false;
}

static boolean br45966_refused_without_an_exception(void) {
    TEST_ASSERT(playerIsAt(br45966_start),
                "without levitation or fire immunity, the step into lava should be refused");
    TEST_ASSERT(messageWasShown("certain death"),
                "the refusal should warn that the lava would be certain death");
    return !testFailed;
}

static boolean br45966_exception_steps_onto_lava(void) {
    TEST_ASSERT(playerIsAt(br45966_lava),
                "the exception the rule names should let the step onto the lava go through");
    TEST_ASSERT(!messageWasShown("certain death"),
                "no certain-death warning should fire when the exception applies");
    TEST_ASSERT(!isPlayerDead(),
                "the player should still be alive after stepping onto the lava");
    return !testFailed;
}

// ============================================================

// ============================================================
// BR25391 — A potion or charm of fire immunity sets the
// fire-immunity counter to its rolled or scaled duration.
//
// The potion's rolled duration is its fixed 150-turn magnitude,
// and drinking it directly does not end a turn, so the counter
// reads exactly 150.  The charm's duration scales with its
// enchantment: base 10 times 1.25 per point of enchant,
// truncated, so +3 sets 19 and +1 sets 12 — and using a charm
// does end a turn, which costs the freshly set counter one tick
// before the test can read it (18 and 11).
// ============================================================

static int br25391_charmHigh, br25391_charmLow, br25391_potionTurns;

static void br25391_setup(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_IMMUNE_TO_FIRE), 0,
                   "setup: the player should start with no fire immunity");
    givePlayerItem(CHARM, CHARM_FIRE_IMMUNITY, 3);
    givePlayerItem(CHARM, CHARM_FIRE_IMMUNITY, 1);
}

static boolean br25391_step(void) {
    applyPackItem(CHARM, CHARM_FIRE_IMMUNITY); // the +3 charm (first in the pack)
    br25391_charmHigh = playerStatusTurns(STATUS_IMMUNE_TO_FIRE);
    removePackItems(CHARM, CHARM_FIRE_IMMUNITY);
    givePlayerItem(CHARM, CHARM_FIRE_IMMUNITY, 1);
    applyPackItem(CHARM, CHARM_FIRE_IMMUNITY);
    br25391_charmLow = playerStatusTurns(STATUS_IMMUNE_TO_FIRE);
    removePackItems(CHARM, CHARM_FIRE_IMMUNITY);
    drink(POTION_FIRE_IMMUNITY);
    br25391_potionTurns = playerStatusTurns(STATUS_IMMUNE_TO_FIRE);
    return false;
}

static boolean br25391_fire_immunity_counter_set(void) {
    TEST_ASSERT_EQ(br25391_charmHigh, 18,
                   "a +3 charm of fire immunity should set the counter to its scaled duration");
    TEST_ASSERT_EQ(br25391_charmLow, 11,
                   "a +1 charm of fire immunity should set a shorter scaled duration");
    TEST_ASSERT_EQ(br25391_potionTurns, 150,
                   "a potion of fire immunity should set the counter to its rolled duration");
    return !testFailed;
}

// ============================================================
// BR91998 — the player must confirm before diving into a
// chasm, and levitation removes the confirmation.
//
// The control case shows a known chasm one step east asking
// "Dive into the depths?"; the queued "no" cancels the move,
// so the player stays put.  The levitating case queues the
// same "no" as a trap: if the game still asked, the move would
// be cancelled and the position assertion would fail.  With
// levitation running the unconfirmed step goes through and the
// player ends up hovering over the chasm, not falling.
// ============================================================

static pos br91998_start;
static pos br91998_chasm;
static int br91998_depth;

static void br91998_chasm_setup(void) {
    clearArena();
    br91998_start = getPlayerPos();
    br91998_depth = currentDepth();
    br91998_chasm = posOffset(br91998_start, 1, 0);
    setTerrain(br91998_chasm, CHASM);
    revealCell(br91998_chasm); // the confirmation only applies to a known chasm
    queueConfirmNo();          // answered "no" only if the game asks at all
}

static void br91998_levitating_setup(void) {
    br91998_chasm_setup();
    applyStatusToPlayer(STATUS_LEVITATING, 20);
}

static boolean br91998_step_right(void) {
    moveRight();
    return false;
}

static boolean br91998_dive_asks_for_confirmation(void) {
    TEST_ASSERT(messageWasShown("Dive into the depths?"),
                "the player should be asked to confirm before diving into a chasm");
    TEST_ASSERT(playerIsAt(br91998_start),
                "declining the dive confirmation should cancel the step");
    return !testFailed;
}

static boolean br91998_levitation_removes_confirmation(void) {
    TEST_ASSERT(isPlayerLevitating(), "setup: the player should be levitating");
    TEST_ASSERT(!messageWasShown("Dive into the depths?"),
                "levitating, the player should not be asked to confirm the dive");
    TEST_ASSERT(playerIsAt(br91998_chasm),
                "levitating, the unconfirmed step should carry the player over the chasm");
    TEST_ASSERT_EQ(currentDepth(), br91998_depth,
                   "levitating over the chasm should leave the player on the same level");
    return !testFailed;
}

// ============================================================
// BR24556 — a weapon of confusion sets the target's confusion
// duration from the weapon's enchantment.
//
// The duration is three halves of the enchantment, never less
// than 3.  The target is a beached kraken, which cannot act, and
// each blow is struck without ending the turn, so the counter
// cannot tick down between the hit and the check.  Only the hit
// is random: the number of blows is bounded so a broken runic
// fails the test instead of looping forever.
// ============================================================

static int br24556_duration;
static int br24556_expected;
static int br24556_attempts;

static void br24556_prepare(int enchant) {
    clearArena();
    giveAndEquipRunicWeapon(DAGGER, W_CONFUSION, enchant, true);
    monst = placeHostileMonster(MK_KRAKEN, posOffset(getPlayerPos(), 1, 0));
    br24556_duration = 0;
    br24556_attempts = 0;
}

static boolean br24556_step(void) {
    setMonsterHealth(monst, monsterMaxHealth(monst)); // keep the target alive to be struck again
    playerAttacksMonster(monst);
    if (monsterHasStatus(monst, STATUS_CONFUSED)) {
        br24556_duration = monsterStatusTurns(monst, STATUS_CONFUSED);
        return false; // runic fired, done
    }
    return ++br24556_attempts < 2000;
}

static boolean br24556_confusion_duration_is(void) {
    TEST_ASSERT(br24556_duration > 0, "the confusion rune should have triggered");
    TEST_ASSERT_EQ(br24556_duration, br24556_expected,
                   "a weapon of confusion should set the target's confusion duration from its enchantment");
    return !testFailed;
}

static void br24556_enchant10_setup(void) {
    br24556_prepare(10);
    br24556_expected = 15; // 10 * 3 / 2
}

static void br24556_enchant4_setup(void) {
    br24556_prepare(4);
    br24556_expected = 6; // 4 * 3 / 2
}

// ============================================================
// BR14011 — confusion gas sets the confusion counter to at least
// 25 on a creature that is neither inanimate nor invulnerable.
// ============================================================

static void br14011_player_setup(void) {
    clearArena();
    spawnFeature(getPlayerPos(), DF_CONFUSION_GAS_CLOUD_POTION);
    applyTileEffectsToPlayer();
}

static boolean br14011_player_confused(void) {
    TEST_ASSERT(isPlayerConfused(), "confusion gas should confuse the player");
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_CONFUSED), 25,
                   "confusion gas should set the confusion counter to at least 25");
    return !testFailed;
}

static void br14011_monster_setup(enum monsterTypes type) {
    clearArena();
    monst = placeHostileMonster(type, posOffset(getPlayerPos(), 1, 0));
    spawnFeature(monsterPosition(monst), DF_CONFUSION_GAS_CLOUD_POTION);
    TEST_ASSERT(cellHasGas(monsterPosition(monst)),
                "setup: the monster should be standing in confusion gas");
    monsterTakesTurn(monst);
}

static void br14011_ordinary_setup(void) {
    br14011_monster_setup(MK_TROLL);
}

static boolean br14011_ordinary_confused(void) {
    TEST_ASSERT(monsterHasStatus(monst, STATUS_CONFUSED),
                "confusion gas should confuse an ordinary creature");
    TEST_ASSERT_EQ(monsterStatusTurns(monst, STATUS_CONFUSED), 25,
                   "confusion gas should set the creature's confusion counter to at least 25");
    return !testFailed;
}

static void br14011_inanimate_setup(void) {
    br14011_monster_setup(MK_GOBLIN_TOTEM);
}

static boolean br14011_inanimate_not_confused(void) {
    TEST_ASSERT(cellHasGas(monsterPosition(monst)),
                "setup: the inanimate creature should be standing in confusion gas");
    TEST_ASSERT(!monsterHasStatus(monst, STATUS_CONFUSED),
                "confusion gas should not confuse an inanimate creature");
    return !testFailed;
}

static void br14011_invulnerable_setup(void) {
    br14011_monster_setup(MK_WARDEN_OF_YENDOR);
}

static boolean br14011_invulnerable_not_confused(void) {
    TEST_ASSERT(cellHasGas(monsterPosition(monst)),
                "setup: the invulnerable creature should be standing in confusion gas");
    TEST_ASSERT(!monsterHasStatus(monst, STATUS_CONFUSED),
                "confusion gas should not confuse an invulnerable creature");
    return !testFailed;
}

// ============================================================
// BR18500 — Discord sets the discord counter (30 for the blast; a
// scaled duration for a staff).
//
// The scroll's blast catches every visible monster within its radius, so a
// single nearby rat receives the fixed 30-turn counter.  Reading the scroll
// through the engine's scroll routine without ending the turn keeps the
// counter at the value the blast set rather than one periodic tick later.
// The staff's bolt carries the staff's enchantment as its magnitude, and its
// duration is four turns per point of enchantment; two different
// enchantments make a fixed duration or the wrong scale fail.  The staff is
// zapped without spending a turn, so its counter is also read at the value
// the bolt set.
// ============================================================

static creature *br18500_target;

static void br18500_scroll_setup(void) {
    setArenaRadius(5, FLOOR);
    br18500_target = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 2, 0));
    // The blast only affects monsters in the field of view, which a real turn
    // establishes.  Paralyze the rat so it stays put during that turn.
    applyStatusToMonster(br18500_target, STATUS_PARALYZED, 100);
    restOneTurn();
    readScrollOfKindWithoutEndingTurn(SCROLL_DISCORD);
}

static boolean br18500_scroll_sets_thirty(void) {
    TEST_ASSERT(br18500_target != NULL,
                "setup: the blast's target should exist");
    TEST_ASSERT(monsterHasStatus(br18500_target, STATUS_DISCORDANT),
                "a discord blast should make the visible monster discordant");
    TEST_ASSERT_EQ(monsterStatusTurns(br18500_target, STATUS_DISCORDANT), 30,
                   "a discord blast should set the discord counter to 30");
    return !testFailed;
}

static int br18500_expectedTurns;

static void br18500_staff_setup(int enchant) {
    setArenaRadius(5, FLOOR);
    br18500_target = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 2, 0));
    br18500_expectedTurns = enchant * 4;
    item *staff = givePlayerItem(STAFF, STAFF_DISCORD, enchant);
    playerZapsStaffAt(staff, br18500_target);
}

static void br18500_staff_enchant2_setup(void) {
    br18500_staff_setup(2);
}

static void br18500_staff_enchant5_setup(void) {
    br18500_staff_setup(5);
}

static boolean br18500_staff_duration_scales_with_enchantment(void) {
    TEST_ASSERT(br18500_target != NULL,
                "setup: the staff's target should exist");
    TEST_ASSERT(monsterHasStatus(br18500_target, STATUS_DISCORDANT),
                "a discord staff should make its target discordant");
    TEST_ASSERT_EQ(monsterStatusTurns(br18500_target, STATUS_DISCORDANT),
                   br18500_expectedTurns,
                   "a discord staff should last four turns per point of enchantment");
    return !testFailed;
}

// ============================================================
// BR49341 — a discordant monster attacks whatever eligible
// creature is nearest rather than respecting allegiance
// ============================================================

static creature *br49341_subject;
static creature *br49341_nearest;
static pos br49341_startPos;
static pos br49341_afterFirstTurn;
static int br49341_playerHealthBefore;

static void br49341_setup(void) {
    setArenaRadius(6, FLOOR);
    pos p = getPlayerPos();

    // The nearest creature is two squares east of the subject and shares its
    // allegiance; the player is three squares west and is an enemy.  A
    // discordant subject must step east toward the nearer ally, whereas its
    // ordinary hunting would carry it west toward the player.  The neighbour
    // is asleep so the attack it receives on the next turn lands for certain.
    br49341_subject = spawnMonster(MK_RAT, posOffset(p, 3, 0));
    makeMonsterHostile(br49341_subject);
    applyStatusToMonster(br49341_subject, STATUS_DISCORDANT, 100);
    br49341_startPos = monsterPosition(br49341_subject);

    br49341_nearest = spawnMonster(MK_RAT, posOffset(p, 5, 0));
    makeMonsterHostile(br49341_nearest);
    putMonsterToSleep(br49341_nearest);

    br49341_playerHealthBefore = playerHealth();
}

static boolean br49341_step(void) {
    monsterTakesTurn(br49341_subject);
    br49341_afterFirstTurn = monsterPosition(br49341_subject);
    monsterTakesTurn(br49341_subject);
    return false;
}

static boolean br49341_attacks_nearest_creature(void) {
    TEST_ASSERT_POS_EQ(br49341_afterFirstTurn, posOffset(br49341_startPos, 1, 0),
                       "a discordant monster should step toward the nearest creature rather than the player");
    TEST_ASSERT(!monsterIsHealthy(br49341_nearest),
                "a discordant monster should attack the nearest creature even though it shares the monster's allegiance");
    TEST_ASSERT_EQ(playerHealth(), br49341_playerHealthBefore,
                   "the discordant monster should not attack the farther player");
    return !testFailed;
}

// ============================================================
// BR50793 — an entrancement bolt sets the entranced counter on a
// non-inanimate monster (a scaled staff duration) and wakes it.
//
// The staff's bolt magnitude is its enchantment, so the entrancement
// lasts three turns per point of enchantment.  Two different
// enchantments are used so a fixed duration cannot pass, and the
// target is put to sleep first so the waking half of the rule is
// observable.  The staff is zapped without spending a turn, so the
// counter can be read at the value the bolt set rather than one
// periodic-update tick later.  A separate inanimate target shows the
// effect is limited to creatures that can be entranced at all.
// ============================================================

static creature *br50793_target;
static int br50793_expectedTurns;
static boolean br50793_wasAsleep;

static void br50793_animate_setup(int enchant) {
    setArenaRadius(5, FLOOR);
    br50793_target = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 3, 0));
    putMonsterToSleep(br50793_target);
    br50793_wasAsleep = monsterIsAsleep(br50793_target);
    br50793_expectedTurns = enchant * 3;
    item *staff = givePlayerItem(STAFF, STAFF_ENTRANCEMENT, enchant);
    playerZapsStaffAt(staff, br50793_target);
}

static void br50793_enchant5_setup(void) {
    br50793_animate_setup(5);
}

static void br50793_enchant2_setup(void) {
    br50793_animate_setup(2);
}

static void br50793_inanimate_setup(void) {
    setArenaRadius(5, FLOOR);
    br50793_target = placeHostileMonster(MK_OGRE_TOTEM, posOffset(getPlayerPos(), 3, 0));
    item *staff = givePlayerItem(STAFF, STAFF_ENTRANCEMENT, 5);
    playerZapsStaffAt(staff, br50793_target);
}

static boolean br50793_entrances_and_wakes(void) {
    TEST_ASSERT(br50793_target != NULL, "setup: the target should exist");
    TEST_ASSERT(br50793_wasAsleep, "setup: the target should start asleep");
    TEST_ASSERT(monsterIsAlive(br50793_target), "the bolt should not kill the target");
    TEST_ASSERT(!monsterIsAsleep(br50793_target),
                "an entrancement bolt should wake the monster");
    TEST_ASSERT_EQ(monsterStatusTurns(br50793_target, STATUS_ENTRANCED),
                   br50793_expectedTurns,
                   "entrancement should last three turns per point of staff enchantment");
    TEST_ASSERT(messageWasShown("is entranced!"),
                "the visible target should be reported as entranced");
    return !testFailed;
}

static boolean br50793_inanimate_not_entranced(void) {
    TEST_ASSERT(br50793_target != NULL, "setup: the inanimate target should exist");
    TEST_ASSERT(!monsterHasStatus(br50793_target, STATUS_ENTRANCED),
                "an inanimate creature should not be entranced");
    TEST_ASSERT_EQ(monsterStatusTurns(br50793_target, STATUS_ENTRANCED), 0,
                   "an inanimate creature should have no entrancement counter");
    TEST_ASSERT(!messageWasShown("is entranced!"),
                "an inanimate creature should not be reported as entranced");
    return !testFailed;
}

// ============================================================
// BR36538 — a paralyzed defender is always hit, takes triple
//            damage, and does not get to retaliate
// ============================================================

#define BR36538_TRIALS 20

static creature *br36538_target;

static void br36538_setup(enum monsterTypes type, int health) {
    clearArena();
    // Triple damage is measured against an unarmed blow, whose base damage is
    // 1-2.  Take off the starting dagger so the expected range is exact.
    if (equippedWeapon() != NULL) {
        unequipItemFromPack(WEAPON, itemKind(equippedWeapon()));
    }
    br36538_target = placeHostileMonster(type, posOffset(getPlayerPos(), 1, 0));
    setMonsterHealth(br36538_target, health);
    applyStatusToMonster(br36538_target, STATUS_PARALYZED, 100);
}

static void br36538_damage_setup(void) {
    // A troll's defense of 70 turns a normal blow aside most of the time, so
    // the damage assertions also rest on the paralyzed defender being hit.
    br36538_setup(MK_TROLL, 500);
}

static void br36538_jackal_setup(void) {
    // A hasted jackal attacks twice as fast as the player, so delaying it past
    // its own attack interval proves it was not allowed to retaliate.
    br36538_setup(MK_JACKAL, 500);
    hasteMonster(br36538_target, 100);
}

static boolean br36538_triple_damage(void) {
    int before = monsterHealth(br36538_target);
    TEST_ASSERT(playerAttacksMonster(br36538_target),
                "an attack should always hit a paralyzed defender");
    int damage = before - monsterHealth(br36538_target);
    TEST_ASSERT(damage == 3 || damage == 6,
                "a paralyzed defender should take triple the 1-2 damage of an unarmed blow");
    return !testFailed;
}

static boolean br36538_always_hit(void) {
    for (int i = 0; i < BR36538_TRIALS; i++) {
        setMonsterHealth(br36538_target, monsterMaxHealth(br36538_target));
        // Attacking clears paralysis, so restore it before each trial.
        applyStatusToMonster(br36538_target, STATUS_PARALYZED, 100);
        if (!playerAttacksMonster(br36538_target)) {
            testFail("a paralyzed defender should be hit by every attack");
            break;
        }
    }
    return !testFailed;
}

static boolean br36538_no_retaliation(void) {
    TEST_ASSERT(monsterHasStatus(br36538_target, STATUS_PARALYZED),
                "setup: the target should be paralyzed");
    TEST_ASSERT(playerAttacksMonster(br36538_target),
                "an attack should always hit a paralyzed defender");
    TEST_ASSERT(monsterTicksUntilTurn(br36538_target) > monsterAttackSpeed(br36538_target),
                "a paralyzed defender should be delayed past its own attack instead of retaliating");
    return !testFailed;
}

// ============================================================
// BR98338 — a monster that avoids corridors is enraged for 4 turns
// by an attack that directly damages it
//
// The target is asleep so the blow is guaranteed to land, and its
// health is raised so the blow cannot kill it before the reaction
// is recorded.  The control monster lacks the corridor-avoidance
// ability, so the same attack must leave it calm: that keeps the
// check from passing on any attack at all.
// ============================================================

static creature *br98338_avoider;
static creature *br98338_control;
static boolean br98338_avoiderWasHit;
static boolean br98338_controlWasHit;

static void br98338_setup(void) {
    clearArena();
    pos targetLoc = posOffset(getPlayerPos(), 1, 0);
    enum monsterTypes avoiderType = chooseMonsterTypeByProperty(
        BOLT_NONE, 0, 0, MA_AVOID_CORRIDORS, 0);

    br98338_avoider = spawnMonster(avoiderType, targetLoc);
    putMonsterToSleep(br98338_avoider);
    setMonsterHealth(br98338_avoider, 1000);

    br98338_control = spawnMonster(MK_RAT, posOffset(getPlayerPos(), -1, 0));
    putMonsterToSleep(br98338_control);
    setMonsterHealth(br98338_control, 1000);
}

static boolean br98338_attack_avoider_step(void) {
    br98338_avoiderWasHit = playerAttacksMonster(br98338_avoider);
    return false;
}

static boolean br98338_attack_control_step(void) {
    br98338_controlWasHit = playerAttacksMonster(br98338_control);
    return false;
}

static boolean br98338_avoider_enraged(void) {
    TEST_ASSERT(monsterHasAbility(br98338_avoider, MA_AVOID_CORRIDORS),
                "setup: the target must normally avoid corridors");
    TEST_ASSERT(br98338_avoiderWasHit,
                "a sleeping target should be hit by the attack");
    TEST_ASSERT(monsterHasStatus(br98338_avoider, STATUS_ENRAGED),
                "direct damage should enrage a corridor-avoiding monster");
    TEST_ASSERT_EQ(monsterStatusTurns(br98338_avoider, STATUS_ENRAGED), 4,
                   "the enraged counter should be set to 4 turns");
    return !testFailed;
}

static boolean br98338_control_not_enraged(void) {
    TEST_ASSERT(!monsterHasAbility(br98338_control, MA_AVOID_CORRIDORS),
                "setup: the control must not avoid corridors");
    TEST_ASSERT(br98338_controlWasHit,
                "a sleeping control should be hit by the attack");
    TEST_ASSERT(!monsterHasStatus(br98338_control, STATUS_ENRAGED),
                "direct damage should not enrage a monster that does not avoid corridors");
    return !testFailed;
}

// ============================================================
// BR28722 — while the player is invisible their scent fades faster
// (scent advances 10 per turn instead of 3).
//
// The player is made invisible and one turn is taken; the scent
// counter should advance by 10.  Invisibility is then removed and
// another turn is taken; the scent counter should advance by 3.
// ============================================================

static int br28722_before;
static int br28722_afterInvisible;
static int br28722_afterVisible;

static void br28722_setup(void) {
    clearArena();
    clearMonsters();
    applyStatusToPlayer(STATUS_INVISIBLE, 100);
    br28722_before = playerScentTurnNumber();
}

static boolean br28722_step(void) {
    restOneTurn();
    br28722_afterInvisible = playerScentTurnNumber();
    removeStatusFromPlayer(STATUS_INVISIBLE);
    restOneTurn();
    br28722_afterVisible = playerScentTurnNumber();
    return false;
}

static boolean br28722_scent_advances_faster_while_invisible(void) {
    TEST_ASSERT_EQ(br28722_afterInvisible - br28722_before, 10,
                   "the scent counter should advance by 10 per turn while invisible");
    TEST_ASSERT_EQ(br28722_afterVisible - br28722_afterInvisible, 3,
                   "the scent counter should advance by 3 per turn while visible");
    return !testFailed;
}

const testCase tests_status_effects[] = {
    { "BR97529_telepathy_counter_decrements",   telepathy_decrement_setup,      rest_one_turn_step, status_decremented_by_one },
    { "BR97529_hallucination_counter_decrements", hallucination_decrement_setup, rest_one_turn_step, status_decremented_by_one },
    { "BR35089_levitation_counter_decrements",  levitation_decrement_setup,     rest_one_turn_step, status_decremented_by_one },
    { "BR84218_confusion_counter_decrements",   confusion_decrement_setup,      rest_one_turn_step, status_decremented_by_one },
    { "BR48502_nausea_counter_decrements",      nausea_decrement_setup,         rest_one_turn_step, status_decremented_by_one },
    { "BR74294_paralysis_counter_decrements",   paralysis_decrement_setup,      rest_one_turn_step, monster_paralysis_decremented },
    { "BR41654_darkness_counter_decrements",    darkness_decrement_setup,       rest_one_turn_step, status_decremented_by_one },
    { "BR97529_invisibility_counter_decrements", invisibility_decrement_setup,  rest_one_turn_step, status_decremented_by_one },
    { "BR48230_invisibility_fades_message",     invisibility_fade_setup,        rest_one_turn_step, invisibility_faded_message },
    { "BR18867_aggravation_counter_decrements", aggravation_decrement_setup,    rest_one_turn_step, status_decremented_by_one },
    { "BR91554_aggravation_wakes_and_alerts",   aggravation_setup,              NULL, br91554_aggravation_wakes_and_alerts },
    { "BR10268_aggravation_strips_avoidance",   aggravation_setup,              NULL, br10268_aggravation_strips_avoidance },
    { "BR50175_aggravation_adds_to_stealth_range", br50175_aggravation_setup,   NULL, br50175_aggravation_adds_to_stealth_range },
    { "BR50175_invisibility_ignores_aggravation",  br50175_invisible_setup,     NULL, br50175_invisibility_ignores_aggravation },
    { "BR75439_fire_immunity_counter_decrements",fire_immunity_decrement_setup, rest_one_turn_step, status_decremented_by_one },
    { "BR31504_explosion_immunity_counter_decrements", explosion_immunity_decrement_setup, rest_one_turn_step, status_decremented_by_one },

    { "BR28518_telepathy_fades_message",        telepathy_fades_setup,          rest_one_turn_step, telepathy_faded_message },
    { "BR41654_darkness_fades_message",         darkness_fades_setup,           rest_one_turn_step, darkness_faded_message },

    { "BR76412_searches_accumulate",            search_accumulates_setup,       NULL, search_counter_is_three },
    { "BR76412_search_counter_resets",          search_then_stop_setup,         NULL, search_counter_reset },
    { "BR41784_fifth_search_resets_counter",    five_searches_setup,            NULL, search_counter_cleared },

    { "BR95942_donning_counter_set",            donning_setup,                  NULL, donning_counter_set },
    { "BR37051_removing_armor_resets_donning",  removing_armor_setup,           NULL, donning_counter_reset },

    { "BR49074_weakness_stacks",                weakness_stacks_setup,          NULL, weakness_stacked },
    { "BR55648_weakness_expiry_clears_amount",  weakness_expires_setup,         rest_one_turn_step, weakness_cleared },

    { "BR50918_slow_halves_speed",              slow_halves_speed_setup,        NULL, slow_halves_speed },
    { "BR28481_slow_clears_haste",              slow_clears_haste_setup,        NULL, slow_clears_haste },
    { "BR25804_haste_clears_slow",              haste_clears_slow_setup,        NULL, haste_clears_slow },
    { "BR40736_haste_sets_haste_clears_slow",   haste_sets_haste_setup,         NULL, haste_applied },
    { "BR22853_slow_expiry_restores_speed",     slow_expires_setup,             rest_one_turn_step, speed_restored },
    { "BR71884_haste_expiry_restores_monster_speed", monster_haste_expires_setup, rest_one_turn_step, monster_haste_expired },
    { "BR71884_haste_expiry_restores_player_speed_and_time", player_haste_expires_setup, rest_twice_step, player_haste_expired },
    { "BR71884_haste_still_running_keeps_speeds_raised", player_haste_still_running_setup, rest_twice_step, player_speeds_still_hasted },

    { "BR37844_hasted_speed_survives_encumbrance_recompute", hasted_encumbrance_recompute_setup, NULL, hasted_speed_survives_encumbrance_recompute },
    { "BR37844_slowed_speed_survives_encumbrance_recompute", slowed_encumbrance_recompute_setup, NULL, slowed_speed_survives_encumbrance_recompute },
    { "BR37844_normal_speed_survives_encumbrance_recompute", normal_encumbrance_recompute_setup, NULL, normal_speed_survives_encumbrance_recompute },

    { "BR64646_poison_adds_concentration",      poison_added_setup,             NULL, poison_added },
    { "BR75638_poison_damages_and_ticks",       poison_damages_setup,           rest_one_turn_step, poison_damaged_and_ticked },
    { "BR52651_panacea_clears_poison",          panacea_clears_poison_setup,    NULL, poison_cleared },
    { "BR33833_panacea_caps_statuses",          panacea_caps_statuses_setup,    NULL, statuses_capped },

    { "BR51327_fire_immunity_extinguishes",     immunity_extinguishes_setup,    NULL, burning_extinguished },
    { "BR25391_fire_immunity_counter_set",      br25391_setup,                  br25391_step, br25391_fire_immunity_counter_set },

    { "BR25686_nutrition_starts_full",          NULL,                           NULL, nutrition_is_full },
    { "BR49432_nutrition_decrements",           nutrition_decreases_setup,      rest_one_turn_step, nutrition_decreased },
    { "BR24204_no_nutrition_while_paralyzed",   paralyzed_no_nutrition_setup,   wait_10_turns_step, nutrition_unchanged },
    { "BR80877_starvation_costs_health",        starvation_setup,               rest_one_turn_step, starved_one_point },
    { "BR32368_hunger_threshold_warning",       hunger_warning_setup,           rest_one_turn_step, hunger_warning_shown },

    { "BR73170_levitating_not_underwater",      levitating_over_water_setup,    NULL, not_underwater },

    { "BR10266_levitating_keeps_secret_hidden", br10266_levitating_setup,       NULL, br10266_levitating_keeps_secret },
    { "BR10266_non_levitating_discovers_secret", br10266_control_setup,         NULL, br10266_discovers_secret },

    { "BR98623_levitating_shows_location_flavor",    br98623_levitating_setup, NULL, br98623_levitating_shows_location_flavor },
    { "BR98623_non_levitating_shows_terrain_flavor",  br98623_terrain_setup,    NULL, br98623_shows_terrain_flavor },

    { "BR27834_levitating_burning_survives_water", br27834_levitating_setup,    NULL, br27834_levitating_keeps_burning },
    { "BR27834_water_extinguishes_non_levitating", br27834_control_setup,       NULL, br27834_water_extinguishes },

    { "BR47057_player_catches_fire_and_is_told", br47057_player_setup, NULL, br47057_player_catches_fire_and_is_told },
    { "BR47057_ordinary_creature_catches_fire", br47057_ordinary_setup, NULL, br47057_ordinary_creature_catches_fire },
    { "BR47057_dying_creature_does_not_catch_fire", br47057_dying_setup, NULL, br47057_dying_creature_does_not_catch_fire },
    { "BR47057_fire_immune_creature_does_not_catch_fire", br47057_immune_setup, NULL, br47057_fire_immune_creature_does_not_catch_fire },
    { "BR47057_invulnerable_creature_does_not_catch_fire", br47057_invulnerable_setup, NULL, br47057_invulnerable_creature_does_not_catch_fire },
    { "BR47057_submerged_creature_does_not_catch_fire", br47057_submerged_setup, NULL, br47057_submerged_creature_does_not_catch_fire },
    { "BR47057_water_exempts_non_levitating_creature", br47057_water_setup, NULL, br47057_water_exempts_non_levitating_creature },

    { "BR38788_levitating_creature_leaves_plate_up", br38788_levitating_setup,  NULL, br38788_levitating_leaves_plate_up },
    { "BR38788_non_levitating_creature_depresses_plate", br38788_control_setup, NULL, br38788_control_depresses_plate },

    { "BR62028_paralyzed_monster_takes_no_action", paralyzed_monster_idle_setup, wait_10_turns_step, paralyzed_monster_did_nothing },
    { "BR34867_entranced_monster_skips_turn",   entranced_monster_idle_setup,   wait_10_turns_step, entranced_monster_did_nothing },

    { "BR50793_entrancement_duration_scales_with_enchant5", br50793_enchant5_setup, NULL, br50793_entrances_and_wakes },
    { "BR50793_entrancement_duration_scales_with_enchant2", br50793_enchant2_setup, NULL, br50793_entrances_and_wakes },
    { "BR50793_entrancement_skips_inanimate",               br50793_inanimate_setup, NULL, br50793_inanimate_not_entranced },
    { "BR73717_attacking_paralyzed_clears_it",  attack_paralyzed_setup,         attack_paralyzed_step, paralysis_cleared },
    { "BR36538_paralyzed_defender_takes_triple_damage", br36538_damage_setup, NULL, br36538_triple_damage },
    { "BR36538_paralyzed_defender_always_hit",          br36538_damage_setup, NULL, br36538_always_hit },
    { "BR36538_paralyzed_defender_does_not_retaliate",  br36538_jackal_setup, NULL, br36538_no_retaliation },

    { "BR68621_shield_decays_by_twentieth",     shield_decays_setup,            rest_one_turn_step, shield_decayed },

    { "BR23913_player_stuck_ends_off_entangling_tile", br23913_player_setup, br23913_player_step, br23913_player_stuck_ends },
    { "BR23913_monster_stuck_ends_off_entangling_tile", br23913_monster_setup, br23913_monster_step, br23913_monster_stuck_ends },

    { "BR45966_lava_step_refused_without_exception", br45966_lava_setup,     br45966_step_right, br45966_refused_without_an_exception },
    { "BR45966_levitation_steps_into_lava",          br45966_levitating_setup, br45966_step_right, br45966_exception_steps_onto_lava },
    { "BR45966_fire_immunity_steps_into_lava",       br45966_fire_immune_setup, br45966_step_right, br45966_exception_steps_onto_lava },

    { "BR91998_chasm_dive_asks_for_confirmation",    br91998_chasm_setup,       br91998_step_right, br91998_dive_asks_for_confirmation },
    { "BR91998_levitation_removes_dive_confirmation", br91998_levitating_setup,  br91998_step_right, br91998_levitation_removes_confirmation },

    { "BR24556_confusion_weapon_duration_enchant10", br24556_enchant10_setup, br24556_step, br24556_confusion_duration_is },
    { "BR24556_confusion_weapon_duration_enchant4",  br24556_enchant4_setup,  br24556_step, br24556_confusion_duration_is },

    { "BR14011_confusion_gas_sets_player_counter",   br14011_player_setup,      NULL, br14011_player_confused },
    { "BR14011_confusion_gas_confuses_ordinary_creature", br14011_ordinary_setup, NULL, br14011_ordinary_confused },
    { "BR14011_confusion_gas_skips_inanimate_creature",   br14011_inanimate_setup, NULL, br14011_inanimate_not_confused },
    { "BR14011_confusion_gas_skips_invulnerable_creature", br14011_invulnerable_setup, NULL, br14011_invulnerable_not_confused },

    { "BR18500_scroll_discord_sets_thirty_turns",       br18500_scroll_setup,        NULL, br18500_scroll_sets_thirty },
    { "BR18500_staff_discord_duration_scales_with_enchant2", br18500_staff_enchant2_setup, NULL, br18500_staff_duration_scales_with_enchantment },
    { "BR18500_staff_discord_duration_scales_with_enchant5", br18500_staff_enchant5_setup, NULL, br18500_staff_duration_scales_with_enchantment },

    { "BR49341_discordant_attacks_nearest_creature", br49341_setup, br49341_step, br49341_attacks_nearest_creature },

    { "BR98338_corridor_avoider_enraged_by_damage", br98338_setup, br98338_attack_avoider_step, br98338_avoider_enraged },
    { "BR98338_other_monster_not_enraged_by_damage", br98338_setup, br98338_attack_control_step, br98338_control_not_enraged },

    { "BR28722_scent_advances_faster_while_invisible", br28722_setup, br28722_step, br28722_scent_advances_faster_while_invisible },

    { NULL, NULL, NULL, NULL }
};
