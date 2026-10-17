// Tests derived from rules/03-status-effects.md.
//
// These exercise the status effects that can be observed headlessly: the
// counters that tick down on the periodic environmental update, searching,
// weakening, haste and slow, poison, starvation, paralysis, entrancement and
// the shield.  Rules about rendering (light radius, hallucinated names),
// combat prompts, and specific item/bolt magnitudes are omitted.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

// ============================================================
// BR97529 — a player status loses 1 point on the periodic update
// ============================================================

static enum statusEffects decrementingStatus;
static int decrementStart;
static creature *decrementingMonster;

static void status_decrement_setup(void) {
    applyStatusToPlayer(decrementingStatus, decrementStart);
    restTurns(1);
}

static boolean status_decremented_by_one(void) {
    TEST_ASSERT_EQ(playerStatusTurns(decrementingStatus), decrementStart - 1,
                   "the status should decrease by 1 on the periodic update");
    return !testFailed;
}

static void telepathy_decrement_setup(void) {
    decrementingStatus = STATUS_TELEPATHIC;
    decrementStart = 5;
    status_decrement_setup();
}

static void hallucination_decrement_setup(void) {
    decrementingStatus = STATUS_HALLUCINATING;
    decrementStart = 5;
    status_decrement_setup();
}

static void levitation_decrement_setup(void) {
    decrementingStatus = STATUS_LEVITATING;
    decrementStart = 5;
    status_decrement_setup();
}

static void confusion_decrement_setup(void) {
    decrementingStatus = STATUS_CONFUSED;
    decrementStart = 5;
    status_decrement_setup();
}

static void nausea_decrement_setup(void) {
    decrementingStatus = STATUS_NAUSEOUS;
    decrementStart = 5;
    status_decrement_setup();
}

static void paralysis_decrement_setup(void) {
    // A paralyzed player fast-forwards through several turns at once, so the
    // per-update decrement is observed on a monster instead.
    clearMonsters();
    decrementingMonster = spawnMonster(MK_TROLL, posOffset(player.loc, 4, 0));
    applyStatusToMonster(decrementingMonster, STATUS_PARALYZED, 50);
    restTurns(1);
}

static boolean monster_paralysis_decremented(void) {
    TEST_ASSERT_EQ(monsterStatusTurns(decrementingMonster, STATUS_PARALYZED), 49,
                   "a monster's paralysis should decrease by 1 per update");
    return !testFailed;
}

static void darkness_decrement_setup(void) {
    decrementingStatus = STATUS_DARKNESS;
    decrementStart = 5;
    status_decrement_setup();
}

static void invisibility_decrement_setup(void) {
    decrementingStatus = STATUS_INVISIBLE;
    decrementStart = 5;
    status_decrement_setup();
}

static void aggravation_decrement_setup(void) {
    decrementingStatus = STATUS_AGGRAVATING;
    decrementStart = 5;
    status_decrement_setup();
}

static void fire_immunity_decrement_setup(void) {
    decrementingStatus = STATUS_IMMUNE_TO_FIRE;
    decrementStart = 5;
    status_decrement_setup();
}

// ============================================================
// BR28518 — telepathy fades with a message
// ============================================================

static void telepathy_fades_setup(void) {
    applyStatusToPlayer(STATUS_TELEPATHIC, 1);
    restTurns(1);
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
    restTurns(1);
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
    restTurns(1);
}

static boolean weakness_cleared(void) {
    TEST_ASSERT_EQ(playerWeaknessAmount(), 0,
                   "expiring weakness should reset the weakness amount");
    TEST_ASSERT(messageWasShown("strength returns to your muscles"),
                "expiring weakness should produce its message");
    return !testFailed;
}

// ============================================================
// BR50918 — slowing sets slow, clears haste, and doubles the time
// ============================================================

static creature *slowedMonster;
static int monsterBaseSpeed;

static void slow_sets_slow_setup(void) {
    clearMonsters();
    slowedMonster = spawnMonster(MK_TROLL, posOffset(player.loc, 2, 0));
    monsterBaseSpeed = monsterMovementSpeed(slowedMonster);
    hasteMonster(slowedMonster, 10);
    slowMonster(slowedMonster, 10);
}

static boolean slow_applied(void) {
    TEST_ASSERT_EQ(monsterStatusTurns(slowedMonster, STATUS_SLOWED), 10, "slow should be set");
    TEST_ASSERT_EQ(monsterStatusTurns(slowedMonster, STATUS_HASTED), 0, "slow should clear haste");
    TEST_ASSERT_EQ(monsterMovementSpeed(slowedMonster), monsterBaseSpeed * 2,
                   "a slowed monster should take twice as long to act");
    return !testFailed;
}

// ============================================================
// BR40736 — haste sets haste, clears slow, and halves the time
// ============================================================

static void haste_sets_haste_setup(void) {
    clearMonsters();
    slowedMonster = spawnMonster(MK_TROLL, posOffset(player.loc, 2, 0));
    monsterBaseSpeed = monsterMovementSpeed(slowedMonster);
    slowMonster(slowedMonster, 10);
    hasteMonster(slowedMonster, 10);
}

static boolean haste_applied(void) {
    TEST_ASSERT_EQ(monsterStatusTurns(slowedMonster, STATUS_HASTED), 10, "haste should be set");
    TEST_ASSERT_EQ(monsterStatusTurns(slowedMonster, STATUS_SLOWED), 0, "haste should clear slow");
    TEST_ASSERT_EQ(monsterMovementSpeed(slowedMonster), monsterBaseSpeed / 2,
                   "a hasted monster should take half as long to act");
    return !testFailed;
}

// ============================================================
// BR22853 — when slow expires the monster returns to its base speed
// ============================================================

static void slow_expires_setup(void) {
    clearMonsters();
    slowedMonster = spawnMonster(MK_TROLL, posOffset(player.loc, 2, 0));
    monsterBaseSpeed = monsterMovementSpeed(slowedMonster);
    slowMonster(slowedMonster, 1);
    restTurns(1);
}

static boolean speed_restored(void) {
    TEST_ASSERT_EQ(monsterStatusTurns(slowedMonster, STATUS_SLOWED), 0, "slow should have expired");
    TEST_ASSERT_EQ(monsterMovementSpeed(slowedMonster), monsterBaseSpeed,
                   "expiring slow should restore the monster's normal speed");
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

static int healthBeforePoison;

static void poison_damages_setup(void) {
    setPlayerHealth(playerMaxHealth());
    healthBeforePoison = playerHealth();
    poisonPlayer(3, 1);
    restTurns(1);
}

static boolean poison_damaged_and_ticked(void) {
    TEST_ASSERT_EQ(playerStatusTurns(STATUS_POISONED), 2,
                   "the poison counter should decrease by 1");
    TEST_ASSERT_EQ(playerHealth(), healthBeforePoison - 1,
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

static void nutrition_starts_full_setup(void) {
    setPlayerNutrition(STOMACH_SIZE);
}

static boolean nutrition_is_full(void) {
    TEST_ASSERT_EQ(playerNutrition(), STOMACH_SIZE,
                   "nutrition should start at the stomach size");
    return !testFailed;
}

// ============================================================
// BR49432 — nutrition decreases on the periodic update
// ============================================================

static void nutrition_decreases_setup(void) {
    setPlayerNutrition(1000);
    restTurns(1);
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
    restTurns(1);
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
    removeAllFood();
    setPlayerNutrition(0);
    healthBeforePoison = playerHealth();
    restTurns(1);
}

static boolean starved_one_point(void) {
    TEST_ASSERT_EQ(playerHealth(), healthBeforePoison - 1,
                   "starvation should cost 1 health per turn");
    return !testFailed;
}

// ============================================================
// BR58287 — crossing the hunger threshold warns the player
// ============================================================

static void hunger_warning_setup(void) {
    removeAllFood();
    setPlayerNutrition(HUNGER_THRESHOLD + 1);
    restTurns(1);
}

static boolean hunger_warning_shown(void) {
    TEST_ASSERT(messageWasShown("you are hungry"),
                "crossing the hunger threshold should warn the player");
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
// BR62028 — a paralyzed monster takes no action
// ============================================================

static creature *idleMonster;
static pos idleStart;

static void paralyzed_monster_idle_setup(void) {
    clearMonsters();
    idleMonster = spawnMonster(MK_TROLL, posOffset(player.loc, 2, 0));
    makeMonsterHostile(idleMonster);
    setTerrainAroundLoc(player.loc, FLOOR);
    applyStatusToMonster(idleMonster, STATUS_PARALYZED, 50);
    idleStart = monsterPosition(idleMonster);
    restTurns(1);
}

static boolean paralyzed_monster_did_nothing(void) {
    TEST_ASSERT_POS_EQ(monsterPosition(idleMonster), idleStart,
                       "a paralyzed monster should not move");
    return !testFailed;
}

// ============================================================
// BR34867 — an entranced monster skips its turns
// ============================================================

static void entranced_monster_idle_setup(void) {
    clearMonsters();
    idleMonster = spawnMonster(MK_TROLL, posOffset(player.loc, 2, 0));
    makeMonsterHostile(idleMonster);
    setTerrainAroundLoc(player.loc, FLOOR);
    applyStatusToMonster(idleMonster, STATUS_ENTRANCED, 50);
    idleStart = monsterPosition(idleMonster);
    restTurns(1);
}

static boolean entranced_monster_did_nothing(void) {
    TEST_ASSERT_POS_EQ(monsterPosition(idleMonster), idleStart,
                       "an entranced monster should not move");
    return !testFailed;
}

// ============================================================
// BR73717 — attacking a paralyzed creature clears its paralysis
// ============================================================

static creature *paralyzedVictim;

static void attack_paralyzed_setup(void) {
    clearMonsters();
    paralyzedVictim = spawnMonster(MK_TROLL, posOffset(player.loc, 1, 0));
    makeMonsterHostile(paralyzedVictim);
    setMonsterHealth(paralyzedVictim, paralyzedVictim->info.maxHP);
    applyStatusToMonster(paralyzedVictim, STATUS_PARALYZED, 50);
    playerAttacksMonster(paralyzedVictim);
}

static boolean paralysis_cleared(void) {
    TEST_ASSERT(!monsterHasStatus(paralyzedVictim, STATUS_PARALYZED),
                "hitting a paralyzed creature should clear its paralysis");
    return !testFailed;
}

// ============================================================
// BR68621 — the shield loses a twentieth of its maximum per update
// ============================================================

static void shield_decays_setup(void) {
    applyStatusToPlayer(STATUS_SHIELDED, 200);
    restTurns(1);
}

static boolean shield_decayed(void) {
    TEST_ASSERT_EQ(playerShield(), 190,
                   "the shield should lose one twentieth of its maximum");
    return !testFailed;
}

// ============================================================

const testCase tests_status_effects[] = {
    { "BR97529_telepathy_counter_decrements",   telepathy_decrement_setup,      NULL, status_decremented_by_one },
    { "BR97529_hallucination_counter_decrements", hallucination_decrement_setup, NULL, status_decremented_by_one },
    { "BR35089_levitation_counter_decrements",  levitation_decrement_setup,     NULL, status_decremented_by_one },
    { "BR84218_confusion_counter_decrements",   confusion_decrement_setup,      NULL, status_decremented_by_one },
    { "BR48502_nausea_counter_decrements",      nausea_decrement_setup,         NULL, status_decremented_by_one },
    { "BR74294_paralysis_counter_decrements",   paralysis_decrement_setup,      NULL, monster_paralysis_decremented },
    { "BR41654_darkness_counter_decrements",    darkness_decrement_setup,       NULL, status_decremented_by_one },
    { "BR48230_invisibility_counter_decrements",invisibility_decrement_setup,   NULL, status_decremented_by_one },
    { "BR18867_aggravation_counter_decrements", aggravation_decrement_setup,    NULL, status_decremented_by_one },
    { "BR75439_fire_immunity_counter_decrements",fire_immunity_decrement_setup, NULL, status_decremented_by_one },

    { "BR28518_telepathy_fades_message",        telepathy_fades_setup,          NULL, telepathy_faded_message },
    { "BR41654_darkness_fades_message",         darkness_fades_setup,           NULL, darkness_faded_message },

    { "BR76412_searches_accumulate",            search_accumulates_setup,       NULL, search_counter_is_three },
    { "BR76412_search_counter_resets",          search_then_stop_setup,         NULL, search_counter_reset },
    { "BR41784_fifth_search_resets_counter",    five_searches_setup,            NULL, search_counter_cleared },

    { "BR95942_donning_counter_set",            donning_setup,                  NULL, donning_counter_set },
    { "BR37051_removing_armor_resets_donning",  removing_armor_setup,           NULL, donning_counter_reset },

    { "BR49074_weakness_stacks",                weakness_stacks_setup,          NULL, weakness_stacked },
    { "BR55648_weakness_expiry_clears_amount",  weakness_expires_setup,         NULL, weakness_cleared },

    { "BR50918_slow_sets_slow_clears_haste",    slow_sets_slow_setup,           NULL, slow_applied },
    { "BR40736_haste_sets_haste_clears_slow",   haste_sets_haste_setup,         NULL, haste_applied },
    { "BR22853_slow_expiry_restores_speed",     slow_expires_setup,             NULL, speed_restored },

    { "BR64646_poison_adds_concentration",      poison_added_setup,             NULL, poison_added },
    { "BR75638_poison_damages_and_ticks",       poison_damages_setup,           NULL, poison_damaged_and_ticked },
    { "BR52651_panacea_clears_poison",          panacea_clears_poison_setup,    NULL, poison_cleared },
    { "BR33833_panacea_caps_statuses",          panacea_caps_statuses_setup,    NULL, statuses_capped },

    { "BR51327_fire_immunity_extinguishes",     immunity_extinguishes_setup,    NULL, burning_extinguished },

    { "BR25686_nutrition_starts_full",          nutrition_starts_full_setup,    NULL, nutrition_is_full },
    { "BR49432_nutrition_decrements",           nutrition_decreases_setup,      NULL, nutrition_decreased },
    { "BR24204_no_nutrition_while_paralyzed",   paralyzed_no_nutrition_setup,   NULL, nutrition_unchanged },
    { "BR80877_starvation_costs_health",        starvation_setup,               NULL, starved_one_point },
    { "BR58287_hunger_threshold_warning",       hunger_warning_setup,           NULL, hunger_warning_shown },

    { "BR73170_levitating_not_underwater",      levitating_over_water_setup,    NULL, not_underwater },

    { "BR62028_paralyzed_monster_takes_no_action", paralyzed_monster_idle_setup, NULL, paralyzed_monster_did_nothing },
    { "BR34867_entranced_monster_skips_turn",   entranced_monster_idle_setup,   NULL, entranced_monster_did_nothing },
    { "BR73717_attacking_paralyzed_clears_it",  attack_paralyzed_setup,         NULL, paralysis_cleared },

    { "BR68621_shield_decays_by_twentieth",     shield_decays_setup,            NULL, shield_decayed },

    { NULL, NULL, NULL, NULL }
};
