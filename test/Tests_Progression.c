// Tests derived from rules/06-progression-score.md.
//
// These cover the deterministic, headless-observable progression rules: the
// state a new game starts in (depth, gold, turn counts, strength, starting
// equipment), the reduced damage taken in easy mode, the amulet fallback, and
// the high-score gate on victory.
//
// Omitted are everything that depends on the run history, the achievement
// screens, stair travel between levels, level restoration, and the wizard
// monster creation menu, all of which need a screen or player-driven menu
// interaction.  The wizard item creation dialog is driven headlessly from
// queued keystrokes.

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
// BR73625 — if level generation fails to place the amulet altar on the
// amulet's floor, the game places an amulet somewhere on that floor as
// a fallback.
// ============================================================

// The framework rigs the altar machine to fail before descending, so the
// fallback placement is the only way an amulet can appear.
static void amulet_altar_fails_setup(void) {
    descendWithFailedAmuletAltar();
}

static boolean amulet_fallback_placed(void) {
    TEST_ASSERT_EQ(currentDepth(), 26,
                   "the amulet's floor should be depth 26");
    TEST_ASSERT(!playerIsCarryingAmulet(),
                "the player should not be carrying the amulet at this point");
    TEST_ASSERT(findFloorItem(AMULET, 0) != NULL,
                "with the altar failed, an amulet should be placed somewhere on the floor");
    return !testFailed;
}

// Negative test: when an amulet already exists (in the pack), the failed
// altar must not cause another amulet to be placed on the floor.
static void amulet_altar_fails_while_carrying_setup(void) {
    givePlayerItem(AMULET, 0, 0);
    descendWithFailedAmuletAltar();
}

static boolean no_fallback_amulet_when_already_carrying(void) {
    TEST_ASSERT_EQ(currentDepth(), 26,
                   "the amulet's floor should be depth 26");
    TEST_ASSERT_EQ(packUnitCount(AMULET, 0), 1,
                   "the player should still carry the one amulet");
    TEST_ASSERT(findFloorItem(AMULET, 0) == NULL,
                "the fallback should not place an amulet when one already exists");
    return !testFailed;
}

// ============================================================
// BR87073 — a victory saves a high score only when not in wizard
// mode and not during playback.
// ============================================================

static int victoryGold;
static int expectedScore;

// Shared setup: at depth 1, carrying the amulet, with up stairs ready to
// step onto.  Stepping onto them ends the game with an escape victory.  Each
// test picks a distinctive purse, so the score its victory would record is
// unique and can be told apart from every other entry in the file.
static void victory_common_setup(void) {
    clearArena();
    givePlayerItem(AMULET, 0, 0);
    givePlayerGold(victoryGold);
    placeUpStairs(posOffset(getPlayerPos(), 0, -1));
    expectedScore = expectedVictoryScore();
}

static void normal_victory_setup(void) {
    victoryGold = 8888888;
    victory_common_setup();
}

static void wizard_victory_setup(void) {
    victoryGold = 7777777;
    victory_common_setup();
    enterWizardMode();
    // Wizard mode turns off the test-mode input plumbing, so the victory's
    // prompts must never run: server mode drops the save-recording prompt and
    // the score screen, and autopilot skips the --MORE-- waits.
    enableServerMode();
    enableAutopilot();
}

static void playback_victory_setup(void) {
    victoryGold = 6666666;
    victory_common_setup();
    enterPlaybackMode();
}

static boolean escape_the_dungeon_step(void) {
    moveUp();
    return false;
}

static boolean normal_victory_saves_score(void) {
    TEST_ASSERT_EQ(currentDepth(), 0,
                   "stepping onto the up stairs with the amulet should escape the dungeon");
    TEST_ASSERT(highScoreHasScore(expectedScore),
                "a normal-mode victory should save its score to the high-score list");
    return !testFailed;
}

static boolean wizard_victory_saves_no_score(void) {
    TEST_ASSERT_EQ(currentDepth(), 0,
                   "stepping onto the up stairs with the amulet should escape the dungeon");
    TEST_ASSERT(!highScoreHasScore(expectedScore),
                "a wizard-mode victory must not be saved to the high-score list");
    return !testFailed;
}

static boolean playback_victory_saves_no_score(void) {
    TEST_ASSERT_EQ(currentDepth(), 0,
                   "stepping onto the up stairs with the amulet should escape the dungeon");
    TEST_ASSERT(!highScoreHasScore(expectedScore),
                "a victory during playback must not be saved to the high-score list");
    return !testFailed;
}

// ============================================================
// BR22777 — wizard item creation refuses to start when the pack is full
// ============================================================

static int br22777_pack_before;

static void br22777_full_pack_setup(void) {
    fillPackToCapacity();
    br22777_pack_before = packItemCount();
}

static boolean br22777_full_pack_step(void) {
    // "a" picks "Item" in the wizard's "Create:" dialog; with the pack full
    // the creation flow must refuse before any further prompt appears.
    queueKeystroke('a', false, false);
    attemptWizardItemCreation();
    return false;
}

static boolean br22777_creation_refused_when_pack_full(void) {
    TEST_ASSERT_EQ(br22777_pack_before, 26,
                   "the pack should be at its capacity before the attempt");
    TEST_ASSERT_MSG(messageWasShown("Your pack is already full."),
                    "wizard item creation should refuse a full pack");
    TEST_ASSERT_EQ(packItemCount(), br22777_pack_before,
                   "a refused creation must add nothing to the pack");
    TEST_ASSERT(!messageWasShown("you now have"),
                "a refused creation must not report a created item");
    return !testFailed;
}

// ============================================================
// BR64450 — a wizard-created item can be identified immediately on request
// ============================================================

// Walk the wizard's creation dialogs from queued keystrokes: "Item" (a),
// the "potion" category (d), its first kind (a), and finally the answer to
// the "Identify the item?" prompt the creation flow ends with.
static void br64450_create_potion(char identifyAnswer) {
    queueKeystroke('a', false, false);
    queueKeystroke('d', false, false);
    queueKeystroke('a', false, false);
    queueKeystroke(identifyAnswer, false, false);
    attemptWizardItemCreation();
}

static void br64450_accept_identify_setup(void) {
    br64450_create_potion('y');
}

static boolean br64450_created_item_is_identified(void) {
    item *created = findInventoryItem(POTION, 0);
    TEST_ASSERT(created != NULL, "the wizard should have created a potion");
    TEST_ASSERT(messageWasShown("you now have"),
                "creation should report the item you now have");
    TEST_ASSERT(itemIsIdentified(created),
                "answering yes should identify the created item at once");
    return !testFailed;
}

static void br64450_decline_identify_setup(void) {
    br64450_create_potion('n');
}

static boolean br64450_created_item_stays_unknown(void) {
    item *created = findInventoryItem(POTION, 0);
    TEST_ASSERT(created != NULL, "the wizard should have created a potion");
    TEST_ASSERT(!itemIsIdentified(created),
                "declining the request should leave the created item unidentified");
    return !testFailed;
}

// ============================================================
// BR78012 — the welcome message tells the player to retrieve the
// Amulet of Yendor from a specific floor of the dungeon and escape
// with it, naming that floor with an ordinal number
// ============================================================

static boolean welcome_message_names_amulet_floor(void) {
    const char *msg = startupMessageWith("Retrieve the");
    TEST_ASSERT(msg != NULL,
                "the game should greet the player with a message about retrieving something");
    if (msg == NULL) {
        return !testFailed;
    }
    TEST_ASSERT(strstr(msg, "Amulet of Yendor") != NULL,
                "the welcome message should name the Amulet of Yendor");
    TEST_ASSERT(strstr(msg, "escape with it") != NULL,
                "the welcome message should tell the player to escape with it");

    const char *floorText = strstr(msg, "from the ");
    TEST_ASSERT(floorText != NULL,
                "the welcome message should say which floor holds the amulet");
    if (floorText == NULL) {
        return !testFailed;
    }

    int floorNumber = 0;
    char suffix[4] = "";
    int fields = sscanf(floorText, "from the %d%3s floor", &floorNumber, suffix);
    TEST_ASSERT(fields == 2,
                "the welcome message should read \"from the <number> floor\"");
    if (fields != 2) {
        return !testFailed;
    }

    TEST_ASSERT(floorNumber >= 1,
                "the welcome message should name a real dungeon floor");
    int lastTwoDigits = floorNumber % 100;
    int lastDigit = floorNumber % 10;
    const char *expectedSuffix;
    if (lastTwoDigits >= 11 && lastTwoDigits <= 13) {
        expectedSuffix = "th";
    } else if (lastDigit == 1) {
        expectedSuffix = "st";
    } else if (lastDigit == 2) {
        expectedSuffix = "nd";
    } else if (lastDigit == 3) {
        expectedSuffix = "rd";
    } else {
        expectedSuffix = "th";
    }
    TEST_ASSERT(strcmp(suffix, expectedSuffix) == 0,
                "the amulet floor should be named with its correct ordinal number");
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

    { "BR73625_failed_altar_fallback_places_amulet", amulet_altar_fails_setup, NULL, amulet_fallback_placed },
    { "BR73625_failed_altar_spares_a_carried_amulet", amulet_altar_fails_while_carrying_setup, NULL, no_fallback_amulet_when_already_carrying },

    { "BR87073_normal_victory_saves_high_score",  normal_victory_setup,  escape_the_dungeon_step, normal_victory_saves_score },
    { "BR87073_wizard_victory_saves_no_score",    wizard_victory_setup,  escape_the_dungeon_step, wizard_victory_saves_no_score },
    { "BR87073_playback_victory_saves_no_score",  playback_victory_setup, escape_the_dungeon_step, playback_victory_saves_no_score },

    { "BR22777_creation_refused_when_pack_full",  br22777_full_pack_setup, br22777_full_pack_step, br22777_creation_refused_when_pack_full },

    { "BR64450_created_item_identified_on_request", br64450_accept_identify_setup, NULL, br64450_created_item_is_identified },
    { "BR64450_created_item_unidentified_when_declined", br64450_decline_identify_setup, NULL, br64450_created_item_stays_unknown },

    { "BR78012_welcome_message_names_amulet_floor", NULL, NULL, welcome_message_names_amulet_floor },

    { NULL, NULL, NULL, NULL }
};
