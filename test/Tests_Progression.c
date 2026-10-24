// Tests derived from rules/06-progression-score.md.
//
// These cover the deterministic, headless-observable progression rules: the
// state a new game starts in (depth, gold, turn counts, strength, starting
// equipment) and the reduced damage taken in easy mode.
//
// Omitted are everything that depends on the menus, the high-score file, run
// history, victory/death screens, achievements, stair travel between levels,
// level restoration, and the wizard item and monster creation menus, all of
// which need a screen, saved files, or player-driven menu interaction.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

static int before;

// ============================================================
// Starting state
// ============================================================

static boolean new_game_begins_on_depth_1(void) {
    TEST_ASSERT_EQ(currentDepth(), 1,
                   "a new game should begin on depth 1");
    TEST_ASSERT_EQ(deepestDepth(), 1,
                   "a new game should record depth 1 as the deepest reached");
    return !testFailed;
}

static boolean new_game_starts_with_no_gold(void) {
    TEST_ASSERT_EQ(playerGold(), 0,
                   "a new game should start with no gold in hand");
    return !testFailed;
}

static boolean new_game_starts_at_turn_zero(void) {
    TEST_ASSERT_EQ(playerTurnCount(), 0,
                   "a new game should start at player turn 0");
    TEST_ASSERT_EQ((int)turnNumber(), 0,
                   "a new game should start at absolute turn 0");
    return !testFailed;
}

static boolean player_starts_with_12_strength(void) {
    TEST_ASSERT_EQ(playerStrength(), 12,
                   "the player should start with 12 strength");
    return !testFailed;
}

static boolean player_starts_with_food_ration(void) {
    TEST_ASSERT(playerHasItem(FOOD, RATION),
                "the player should start with a food ration");
    return !testFailed;
}

static boolean player_starts_with_dagger_equipped(void) {
    item *dagger = equippedWeapon();
    TEST_ASSERT(dagger != NULL, "the player should start with a weapon equipped");
    if (dagger == NULL) {
        return !testFailed;
    }
    TEST_ASSERT_EQ(itemKind(dagger), DAGGER,
                   "the starting weapon should be a dagger");
    TEST_ASSERT(!itemIsCursed(dagger),
                "the starting dagger should not be cursed");
    TEST_ASSERT(!itemIsRunic(dagger),
                "the starting dagger should not be runic");
    return !testFailed;
}

static boolean player_starts_with_15_darts(void) {
    TEST_ASSERT_EQ(packUnitCount(WEAPON, DART), 15,
                   "the player should start with 15 darts");
    return !testFailed;
}

static boolean player_starts_with_leather_armor_equipped(void) {
    item *armor = equippedArmor();
    TEST_ASSERT(armor != NULL, "the player should start with armor equipped");
    if (armor == NULL) {
        return !testFailed;
    }
    TEST_ASSERT_EQ(itemKind(armor), LEATHER_ARMOR,
                   "the starting armor should be leather armor");
    TEST_ASSERT(!itemIsCursed(armor),
                "the starting armor should not be cursed");
    TEST_ASSERT(!itemIsRunic(armor),
                "the starting armor should not be runic");
    return !testFailed;
}

// ============================================================
// Easy mode
// ============================================================

static void normal_damage_setup(void) {
    setPlayerHealth(playerMaxHealth());
    before = playerHealth();
    woundPlayer(15);
}

static boolean normal_mode_takes_full_damage(void) {
    TEST_ASSERT_EQ(playerHealth(), before - 15,
                   "in normal mode you should take the full damage");
    return !testFailed;
}

static void easy_mode_damage_setup(void) {
    enterEasyMode();
    setPlayerHealth(playerMaxHealth());
    before = playerHealth();
    woundPlayer(15);
}

static boolean easy_mode_reduces_damage(void) {
    TEST_ASSERT_EQ(playerHealth(), before - 3,
                   "in easy mode you should take only a fifth of the damage");
    return !testFailed;
}

// ============================================================

const testCase tests_progression[] = {
    { "BR19079_new_game_begins_on_depth_1",        NULL,                    NULL, new_game_begins_on_depth_1 },
    { "BR89567_new_game_starts_with_no_gold",      NULL,                    NULL, new_game_starts_with_no_gold },
    { "BR59025_new_game_starts_at_turn_zero",      NULL,                    NULL, new_game_starts_at_turn_zero },
    { "BR74104_player_starts_with_12_strength",    NULL,                    NULL, player_starts_with_12_strength },
    { "BR84421_player_starts_with_food_ration",    NULL,                    NULL, player_starts_with_food_ration },
    { "BR49147_player_starts_with_dagger_equipped", NULL,                   NULL, player_starts_with_dagger_equipped },
    { "BR81759_player_starts_with_15_darts",       NULL,                    NULL, player_starts_with_15_darts },
    { "BR27339_player_starts_with_leather_armor_equipped", NULL,            NULL, player_starts_with_leather_armor_equipped },

    { "BR88010_normal_mode_takes_full_damage",     normal_damage_setup,     NULL, normal_mode_takes_full_damage },
    { "BR88010_easy_mode_reduces_damage",          easy_mode_damage_setup,  NULL, easy_mode_reduces_damage },

    { NULL, NULL, NULL, NULL }
};
