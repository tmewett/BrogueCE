// Tests derived from rules/NewRules.txt — pending new rules that have not
// yet been merged into a numbered rules file.
//
// These cover the deterministic, headless-observable behaviors those rules
// describe: how the game's absolute turn count and your own turn count
// diverge while you are paralyzed, the wording the Change Mode dialog uses
// for Wizard mode, the opacity menu buttons are painted with, and the
// decorative random-number stream button painting uses.
//
// Omitted are everything that needs a recording or a run history, neither of
// which the pending rules describe headlessly.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

// ============================================================
// BR29087 — the game's absolute turn count rises by one every
// turn, even while you are paralyzed, but your own turn count
// rises only on turns when you are not paralyzed, so time spent
// paralyzed never adds a turn to your count.
//
// A plain rest moves both counts by exactly one (the control
// that proves both counters advance together normally).  A rest
// taken while paralyzed runs the whole stretch of paralysis in
// that single call — the turn loop repeats while paralysis
// lasts, decrementing it once per iteration — so five paralytic
// iterations advance the absolute count five times and your own
// not at all, and the two counts diverge.
// ============================================================

static int br29087_absBefore, br29087_playerBefore;
static int br29087_absAfterNormal, br29087_playerAfterNormal;

static void br29087_setup(void) {
    setArenaRadius(4, FLOOR); // open floor and no monsters: only the turn loop runs
    br29087_absBefore = (int) turnNumber();
    br29087_playerBefore = playerTurnCount();
}

static boolean br29087_step(void) {
    restOneTurn(); // a normal turn: both counts should advance together
    br29087_absAfterNormal = (int) turnNumber();
    br29087_playerAfterNormal = playerTurnCount();

    applyStatusToPlayer(STATUS_PARALYZED, 5);
    restOneTurn(); // burns out the whole stretch of paralysis in one call
    return false;
}

static boolean br29087_turn_counts_diverge_while_paralyzed(void) {
    TEST_ASSERT_EQ(br29087_absAfterNormal - br29087_absBefore, 1,
                   "a normal turn should advance the absolute turn count by one");
    TEST_ASSERT_EQ(br29087_playerAfterNormal - br29087_playerBefore, 1,
                   "a normal turn should advance your own turn count by one");
    TEST_ASSERT_EQ((int) turnNumber() - br29087_absAfterNormal, 5,
                   "the absolute turn count should rise on every paralyzed turn");
    TEST_ASSERT_EQ(playerTurnCount() - br29087_playerAfterNormal, 0,
                   "time spent paralyzed should never add a turn to your own count");
    TEST_ASSERT((int) turnNumber() - br29087_absAfterNormal
                    > playerTurnCount() - br29087_playerAfterNormal,
                "the absolute turn count and your own should diverge while paralyzed");
    TEST_ASSERT(!isPlayerParalyzed(),
                "the rest should run through the whole stretch of paralysis");
    TEST_ASSERT(messageWasShown("you can move again"),
                "the paralysis should wear off during the rest");
    return !testFailed;
}

// ============================================================
// BR14549 — the Change Mode dialog explains that Wizard mode
// makes the player an invincible wizard starting with legendary
// items who is magically reborn after every death, and who can
// summon monsters as friend or foe and conjure any item out of
// thin air.
//
// showModeDialog() drives the real main menu (Play flyout ->
// Change Mode) and answers with Escape, so the mode in effect is
// left alone; in test mode printTextBox() hands the dialog's
// full text to the message log, where each phrase of the Wizard
// blurb can be asserted on its own.  The "Wizard Mode" heading
// assertion doubles as the control that the dialog was shown at
// all.
// ============================================================

static void br14549_setup(void) {
    showModeDialog(ESCAPE_KEY);
}

static boolean br14549_change_mode_dialog_describes_wizard_mode(void) {
    TEST_ASSERT(messageWasShown("Wizard Mode"),
                "control: the Change Mode dialog should have been shown");
    TEST_ASSERT(messageWasShown("invincible wizard"),
                "the dialog should describe Wizard mode as an invincible wizard");
    TEST_ASSERT(messageWasShown("starts with legendary items"),
                "the dialog should say Wizard mode starts with legendary items");
    TEST_ASSERT(messageWasShown("magically reborn after every death"),
                "the dialog should say Wizard mode is reborn after every death");
    TEST_ASSERT(messageWasShown("Summon monsters and make them friend or foe"),
                "the dialog should say Wizard mode summons monsters as friend or foe");
    TEST_ASSERT(messageWasShown("Conjure any item out of thin air"),
                "the dialog should say Wizard mode conjures any item out of thin air");
    return !testFailed;
}

// ============================================================
// BR12483 — Every screen cell a button covers is marked with
// that button's opacity when the menu is drawn as an overlay,
// so a translucent button lets the screen behind it show
// through rather than blocking it; buttons appear at 95% opacity
// in menus such as the actions menu and the inventory, and at
// 75% along the bottom row.
//
// The first three checks paint one button at 50%, 95% and 75%
// opacity over a solid white screen: because the button is
// black, the white that survives is exactly the percentage the
// button lets through, so a missing opacity (fully transparent)
// or a forced 100 (fully opaque) is distinguished from a real
// blend.  The last two checks read the opacity the game's own
// inventory menu and bottom-row buttons actually painted with.
// ============================================================

static void br12483_setup(void) {
    clearArena();
}

static boolean br12483_translucent_buttons_show_screen_through(void) {
    TEST_ASSERT_EQ(buttonBackdropVisibility(50), 50,
                   "a 50%-opacity button should let half the screen behind it show through");
    TEST_ASSERT_EQ(buttonBackdropVisibility(95), 5,
                   "a 95%-opacity button should let 5% of the screen behind it show through");
    TEST_ASSERT_EQ(buttonBackdropVisibility(75), 25,
                   "a 75%-opacity button should let a quarter of the screen behind it show through");
    TEST_ASSERT_EQ(inventoryButtonOpacity(), 95,
                   "the inventory's buttons should be painted at 95% opacity");
    TEST_ASSERT_EQ(bottomRowButtonOpacity(), 75,
                   "the bottom row's buttons should be painted at 75% opacity");
    return !testFailed;
}

// ============================================================
// BR14317 — Painting a menu button never disturbs the game's
// own random-number sequence: any randomness used while the
// button is drawn comes from the decorative stream, and the
// ordinary sequence is switched back in the moment the button
// is finished, so redrawing menus cannot change the game's
// outcomes.
//
// paintMenuButton() paints a button directly, outside any menu
// loop that might already hold the decorative stream, so the
// button painter has to make the switch itself.  The control
// then draws one number from the game's own sequence and shows
// that the counter does advance when gameplay randomness is
// really consumed.
// ============================================================

static boolean br14317_painting_button_leaves_game_rng_alone(void) {
    const unsigned long before = gameplayRandomDraws();
    paintMenuButton("Sample button");
    TEST_ASSERT_EQ(gameplayRandomDraws(), before,
                   "painting a menu button should not consume the game's own random numbers");
    TEST_ASSERT(gameplayRandomStreamActive(),
                "the ordinary random-number sequence should be back in effect after the button is painted");

    const unsigned long controlBefore = gameplayRandomDraws();
    drawGameplayRandomNumber();
    TEST_ASSERT_EQ(gameplayRandomDraws(), controlBefore + 1,
                   "control: drawing from the game's own sequence should consume one number");
    return !testFailed;
}

const testCase tests_newrules[] = {
    { "BR29087_turn_counts_diverge_while_paralyzed",      br29087_setup, br29087_step, br29087_turn_counts_diverge_while_paralyzed },
    { "BR14549_change_mode_dialog_describes_wizard_mode", br14549_setup, NULL,         br14549_change_mode_dialog_describes_wizard_mode },
    { "BR12483_translucent_buttons_show_screen_through",  br12483_setup, NULL,         br12483_translucent_buttons_show_screen_through },
    { "BR14317_painting_button_leaves_game_rng_alone",    NULL,          NULL,         br14317_painting_button_leaves_game_rng_alone },

    { NULL, NULL, NULL, NULL }
};
