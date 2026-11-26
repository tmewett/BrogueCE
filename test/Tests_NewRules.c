// Tests derived from rules/NewRules.txt — pending new rules that have not
// yet been merged into a numbered rules file.
//
// These cover the deterministic, headless-observable behaviors those rules
// describe: how the game's absolute turn count and your own turn count
// diverge while you are paralyzed, the wording the Change Mode dialog uses
// for Wizard mode, the opacity menu buttons are painted with, the
// decorative random-number stream button painting uses, the reach of a pile
// of unicorn poop's rainbow glow, and where a guardian charm places its
// guardian.
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

// ============================================================
// BR36733 — a guardian charm summons its guardian onto the nearest
// reachable square to you that no creature or staircase occupies and whose
// terrain the guardian does not avoid.  Among equally near squares one is
// chosen at random, and it never appears on your own square.
// ============================================================

static int br36733_chebyshev(pos a, pos b) {
    int dx = a.x - b.x; if (dx < 0) dx = -dx;
    int dy = a.y - b.y; if (dy < 0) dy = -dy;
    return dx > dy ? dx : dy;
}

static creature *br36733_guardian;

static void br36733_summon_guardian(void) {
    givePlayerItem(CHARM, CHARM_GUARDIAN, 0);
    applyPackItem(CHARM, CHARM_GUARDIAN);
    br36733_guardian = findMonster(MK_CHARM_GUARDIAN);
}

// Fill the eight squares around you with sacred glyphs, which the guardian
// avoids as a destination but which do not block the placement path, so the
// distance scan can still reach the ring beyond.  `open` is left as ordinary
// floor when `leaveOpen` is set.
static void br36733_ring(boolean leaveOpen, pos open) {
    pos p = getPlayerPos();
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            pos q = posOffset(p, dx, dy);
            if (leaveOpen && q.x == open.x && q.y == open.y) {
                continue;
            }
            setTerrain(q, SACRED_GLYPH);
        }
    }
}

static void br36733_clear_arena(void) {
    clearMonsters();
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(8, FLOOR);
}

static void br36733_open_setup(void) {
    br36733_clear_arena();
    br36733_summon_guardian();
}

static boolean br36733_open_verify(void) {
    TEST_ASSERT(br36733_guardian != NULL, "the charm should summon a guardian");
    TEST_ASSERT_EQ(countMonstersOfType(MK_CHARM_GUARDIAN), 1,
                   "exactly one guardian should appear");
    pos gp = monsterPosition(br36733_guardian);
    TEST_ASSERT(!playerIsAt(gp), "the guardian must not appear on your square");
    TEST_ASSERT_EQ(br36733_chebyshev(gp, getPlayerPos()), 1,
                   "with open floor all around, the guardian should take an adjacent square");
    TEST_ASSERT(!monsterAvoidsCell(br36733_guardian, gp),
                "the guardian should not appear on terrain it avoids");
    return !testFailed;
}

static void br36733_occupied_setup(void) {
    br36733_clear_arena();
    pos open = posOffset(getPlayerPos(), 1, 0);
    br36733_ring(true, open);
    creature *blocker = spawnMonster(MK_RAT, open);
    makeMonsterAlly(blocker);
    br36733_summon_guardian();
}

static boolean br36733_occupied_verify(void) {
    TEST_ASSERT(br36733_guardian != NULL, "the charm should summon a guardian");
    pos gp = monsterPosition(br36733_guardian);
    TEST_ASSERT_EQ(br36733_chebyshev(gp, getPlayerPos()), 2,
                   "with the only free adjacent square occupied, the guardian "
                   "should appear one ring farther out");
    return !testFailed;
}

static void br36733_stairs_setup(void) {
    br36733_clear_arena();
    pos open = posOffset(getPlayerPos(), 1, 0);
    br36733_ring(true, open);
    placeDownStairs(open);
    br36733_summon_guardian();
}

static boolean br36733_stairs_verify(void) {
    TEST_ASSERT(br36733_guardian != NULL, "the charm should summon a guardian");
    pos gp = monsterPosition(br36733_guardian);
    pos stairs = posOffset(getPlayerPos(), 1, 0);
    TEST_ASSERT(!(gp.x == stairs.x && gp.y == stairs.y),
                "the guardian should not appear on a staircase");
    TEST_ASSERT_EQ(br36733_chebyshev(gp, getPlayerPos()), 2,
                   "with the only free adjacent square a staircase, the guardian "
                   "should appear one ring farther out");
    return !testFailed;
}

static void br36733_terrain_setup(void) {
    br36733_clear_arena();
    br36733_ring(false, posAt(0, 0));
    br36733_summon_guardian();
}

static boolean br36733_terrain_verify(void) {
    TEST_ASSERT(br36733_guardian != NULL, "the charm should summon a guardian");
    pos gp = monsterPosition(br36733_guardian);
    TEST_ASSERT_EQ(br36733_chebyshev(gp, getPlayerPos()), 2,
                   "the guardian should avoid the sacred glyphs around you");
    return !testFailed;
}

// ============================================================
// BR19494 — unicorn poop sparkles with rainbow light over a radius
// of 2 tiles, fading to nothing at the edge of the glow.
//
// An open floor is laid out with the player far enough away that the
// miner's light cannot reach the probes.  The probes one, two and
// three tiles from a pile of unicorn poop are sampled without the
// pile as a baseline, then many times with it: the pile's colours
// are re-rolled on every lighting pass, so any one pass may be dim
// or even subtractive, and only repeated passes can show the glow
// reliably.  The one-tile probe is inside the glow (its shadow is
// lifted and light appears there at least once), while the two- and
// three-tile probes are never lit at all — the two-tile edge fades to
// nothing rather than continuing outward.
// ============================================================

#define BR19494_TRIALS 32

static pos br19494_probes[4];
static int br19494_baseline[4];
static int br19494_maxChange[4];
static boolean br19494_shadowCleared[4];
static boolean br19494_shadowStayed[4];

static void br19494_setup(void) {
    clearMonsters();
    pos home = posAt(levelWidth() / 2, levelHeight() / 2);
    // Strip the whole level to bare floor, so no torch, lava, fungus or other
    // glowing terrain can contribute light to the probes.
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }
    setPlayerPos(home);
    // With the miner's light at its full strength the player's own glow
    // reaches across the whole arena and would swamp the pile's contribution.
    // Turning the multiplier down leaves only the minimum two-tile light,
    // which cannot reach probes eight or more tiles away.
    setLightMultiplier(0);

    pos source = posOffset(getPlayerPos(), 8, 0);
    for (int d = 0; d <= 3; d++) {
        br19494_probes[d] = posOffset(source, d, 0);
    }

    setTerrain(source, FLOOR);
    recomputeLighting();
    for (int d = 0; d <= 3; d++) {
        br19494_baseline[d] = lightAt(br19494_probes[d]);
    }

    setTerrain(source, UNICORN_POOP);
    for (int d = 0; d <= 3; d++) {
        br19494_maxChange[d] = 0;
        br19494_shadowCleared[d] = false;
        br19494_shadowStayed[d] = false;
    }
    for (int trial = 0; trial < BR19494_TRIALS; trial++) {
        recomputeLighting();
        for (int d = 0; d <= 3; d++) {
            int change = lightAt(br19494_probes[d]) - br19494_baseline[d];
            if (change < 0) {
                change = -change;
            }
            if (change > br19494_maxChange[d]) {
                br19494_maxChange[d] = change;
            }
            if (cellIsInShadow(br19494_probes[d])) {
                br19494_shadowStayed[d] = true;
            } else {
                br19494_shadowCleared[d] = true;
            }
        }
    }
}

static boolean br19494_glow_ends_at_two_tiles(void) {
    for (int d = 1; d <= 3; d++) {
        TEST_ASSERT_EQ(br19494_baseline[d], 0,
                       "control: without the poop nothing lights the probe tiles");
    }
    TEST_ASSERT(br19494_maxChange[0] > 0,
                "control: the pile's own tile should glow");
    TEST_ASSERT(br19494_shadowCleared[0],
                "control: the glow should lift the shadow from its own tile");

    TEST_ASSERT(br19494_shadowCleared[1],
                "a tile one step away should lie inside the 2-tile glow");
    TEST_ASSERT(br19494_maxChange[1] > 0,
                "a tile one step away should receive light from the glow");

    TEST_ASSERT(!br19494_shadowCleared[2] && br19494_shadowStayed[2],
                "the glow should reach no farther than two tiles");
    TEST_ASSERT_EQ(br19494_maxChange[2], 0,
                   "the glow should fade to nothing at its two-tile edge");
    TEST_ASSERT_EQ(br19494_maxChange[3], 0,
                   "nothing three tiles from the pile should be lit");
    return !testFailed;
}

const testCase tests_newrules[] = {
    { "BR29087_turn_counts_diverge_while_paralyzed",      br29087_setup, br29087_step, br29087_turn_counts_diverge_while_paralyzed },
    { "BR14549_change_mode_dialog_describes_wizard_mode", br14549_setup, NULL,         br14549_change_mode_dialog_describes_wizard_mode },
    { "BR12483_translucent_buttons_show_screen_through",  br12483_setup, NULL,         br12483_translucent_buttons_show_screen_through },
    { "BR14317_painting_button_leaves_game_rng_alone",    NULL,          NULL,         br14317_painting_button_leaves_game_rng_alone },

    { "BR36733_guardian_takes_nearest_reachable_square", br36733_open_setup,     NULL, br36733_open_verify },
    { "BR36733_guardian_avoids_occupied_square",         br36733_occupied_setup, NULL, br36733_occupied_verify },
    { "BR36733_guardian_avoids_staircase",               br36733_stairs_setup,   NULL, br36733_stairs_verify },
    { "BR36733_guardian_avoids_avoided_terrain",         br36733_terrain_setup,  NULL, br36733_terrain_verify },

    { "BR19494_glow_ends_at_two_tiles", br19494_setup, NULL, br19494_glow_ends_at_two_tiles },

    { NULL, NULL, NULL, NULL }
};
