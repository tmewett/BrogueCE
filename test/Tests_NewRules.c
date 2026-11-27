// Tests derived from rules/NewRules.txt — pending new rules that have not
// yet been merged into a numbered rules file.
//
// These cover the deterministic, headless-observable behaviors those rules
// describe: how the game's absolute turn count and your own turn count
// diverge while you are paralyzed, the wording the Change Mode dialog uses
// for Wizard mode, the opacity menu buttons are painted with, the
// decorative random-number stream button painting uses, the reach of a pile
// of unicorn poop's rainbow glow, the radius and edge brightness of a crystal
// wall's and a sacred glyph's glow, where a guardian charm places its
// guardian, the lich and phylactery summoning cycle, how gas stranded on
// a square that cannot hold it is shared among its neighbours, and how
// vulnerability armor doubles the damage of a thrown weapon.
//
// The recording save rule below is observable through the framework's
// recording helpers, so it is covered too.  Omitted is anything that needs a
// run history, which the pending rules do not describe headlessly.

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


// ============================================================
// BR27383 — a crystal wall glows over a randomly chosen radius of
// 3 to 5 tiles, with the light dimming to half brightness at the
// edge of the glow.
//
// A bare floor is laid out with the player far enough away that the
// miner's light cannot reach the probes, and the probes are sampled
// once without a crystal wall as the control.  Each lighting pass
// rolls a new radius, and the outward field-of-view stops just short
// of the exact radius, so the last lit tile lies at 2, 3 or 4 tiles
// for a radius of 3, 4 or 5.  Over many passes all three extents must
// occur (the radius really is random across its whole range), no
// pass may light a fifth tile (the radius never exceeds 5), and the
// glow must be continuous.  The outermost lit tile's brightness
// relative to the crystal wall's own tile is the edge fade: about
// half, which rules out both a fade to nothing and no fade at all.
// ============================================================

#define BR27383_TRIALS 128

static pos br27383_source;
static int br27383_extentCounts[7];
static int br27383_minEdgePercent;
static int br27383_maxEdgePercent;
static int br27383_baselineLit;
static int br27383_sourceDark;
static int br27383_gaps;

static void br27383_setup(void) {
    clearMonsters();
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    // The miner's light bottoms out at two tiles, which cannot reach the
    // probes eight tiles away.
    setLightMultiplier(0);

    br27383_source = posOffset(getPlayerPos(), 8, 0);
    recomputeLighting();
    br27383_baselineLit = 0;
    for (int d = 0; d <= 6; d++) {
        if (lightAt(posOffset(br27383_source, d, 0)) > 0) {
            br27383_baselineLit++;
        }
    }

    setTerrain(br27383_source, CRYSTAL_WALL);
    for (int d = 0; d <= 6; d++) {
        br27383_extentCounts[d] = 0;
    }
    br27383_minEdgePercent = 1000;
    br27383_maxEdgePercent = -1;
    br27383_sourceDark = 0;
    br27383_gaps = 0;

    for (int trial = 0; trial < BR27383_TRIALS; trial++) {
        int lights[7];
        int far = 0;
        recomputeLighting();
        int sourceLight = lightAt(br27383_source);
        if (sourceLight <= 0) {
            br27383_sourceDark++;
            continue;
        }
        for (int d = 1; d <= 6; d++) {
            lights[d] = lightAt(posOffset(br27383_source, d, 0));
            if (lights[d] > 0) {
                far = d;
            }
        }
        for (int d = 1; d <= far; d++) {
            if (lights[d] == 0) {
                br27383_gaps++;
            }
        }
        if (far > 0) {
            br27383_extentCounts[far]++;
            int edgePercent = lights[far] * 100 / sourceLight;
            if (edgePercent < br27383_minEdgePercent) {
                br27383_minEdgePercent = edgePercent;
            }
            if (edgePercent > br27383_maxEdgePercent) {
                br27383_maxEdgePercent = edgePercent;
            }
        }
    }
}

static boolean br27383_crystal_wall_glow_radius_and_edge(void) {
    TEST_ASSERT_EQ(br27383_baselineLit, 0,
                   "control: without a crystal wall nothing lights the probes");
    TEST_ASSERT_EQ(br27383_sourceDark, 0,
                   "a crystal wall should always glow on its own tile");

    TEST_ASSERT(br27383_extentCounts[2] > 0,
                "the glow should sometimes reach only two tiles past its origin, "
                "which is a three-tile light radius");
    TEST_ASSERT(br27383_extentCounts[3] > 0,
                "the glow should sometimes reach three tiles, a four-tile light radius");
    TEST_ASSERT(br27383_extentCounts[4] > 0,
                "the glow should sometimes reach four tiles, a five-tile light radius");
    TEST_ASSERT_EQ(br27383_extentCounts[1], 0,
                   "the light radius should never fall below three tiles");
    TEST_ASSERT_EQ(br27383_extentCounts[5], 0,
                   "the light radius should never exceed five tiles");
    TEST_ASSERT_EQ(br27383_extentCounts[6], 0,
                   "the light radius should never exceed five tiles");
    TEST_ASSERT_EQ(br27383_gaps, 0,
                   "the glow should fill every tile out to its edge without gaps");

    TEST_ASSERT(br27383_minEdgePercent >= 45,
                "the edge of the glow should still carry about half the source brightness");
    TEST_ASSERT(br27383_maxEdgePercent <= 75,
                "the edge of the glow should dim to about half brightness, not stay near full");
    return !testFailed;
}

// ============================================================
// BR31796 — a sacred glyph glows over a radius of 3 tiles,
// fading to nothing at the edge of the glow.
//
// The level is stripped to bare floor and the miner's light
// reduced to its two-tile minimum, so the probes eight tiles
// away are dark without the glyph.  With the glyph, the tiles
// one and two steps away are lit and dim with distance, while
// the tile three steps away and the one beyond it stay dark:
// the glow reaches three tiles and dies out at its edge.
// ============================================================

#define BR31796_PROBE_TILES 4

static pos br31796_source;
static int br31796_baseline[BR31796_PROBE_TILES + 1];
static int br31796_lit[BR31796_PROBE_TILES + 1];

static void br31796_setup(void) {
    clearMonsters();
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    // The miner's light bottoms out at two tiles, which cannot reach the
    // probes eight tiles away.
    setLightMultiplier(0);

    br31796_source = posOffset(getPlayerPos(), 8, 0);
    recomputeLighting();
    for (int d = 0; d <= BR31796_PROBE_TILES; d++) {
        br31796_baseline[d] = lightAt(posOffset(br31796_source, d, 0));
    }

    setTerrain(br31796_source, SACRED_GLYPH);
    recomputeLighting();
    for (int d = 0; d <= BR31796_PROBE_TILES; d++) {
        br31796_lit[d] = lightAt(posOffset(br31796_source, d, 0));
    }
}

static boolean br31796_sacred_glyph_glow_radius_and_edge(void) {
    for (int d = 0; d <= BR31796_PROBE_TILES; d++) {
        TEST_ASSERT_EQ(br31796_baseline[d], 0,
                       "control: without the glyph nothing lights the probe tiles");
    }

    TEST_ASSERT(br31796_lit[0] > 0,
                "a sacred glyph should glow on its own tile");
    TEST_ASSERT(br31796_lit[1] > 0,
                "a tile one step away should lie inside the three-tile glow");
    TEST_ASSERT(br31796_lit[2] > 0,
                "a tile two steps away should lie inside the three-tile glow");
    TEST_ASSERT(br31796_lit[1] < br31796_lit[0],
                "the glow should fade as it moves away from the glyph");
    TEST_ASSERT(br31796_lit[2] < br31796_lit[1],
                "the glow should keep fading toward its edge");
    TEST_ASSERT_EQ(br31796_lit[3], 0,
                   "a sacred glyph should fade to nothing at its three-tile edge");
    TEST_ASSERT_EQ(br31796_lit[4], 0,
                   "nothing four tiles from the glyph should be lit");
    return !testFailed;
}

// ============================================================
// BR33689 — the first time the vulnerability armor's rune takes
// effect, the game tells you "your <armor> pulses and you are
// wracked with pain!" and the armor is identified; once the rune
// is known, triggering it again produces no message.
//
// The player wears an unidentified suit of vulnerability armor
// and is paralyzed so the adjacent rat's blows cannot miss.  The
// first blow must land, announce the rune and identify it; the
// second blow must land too (health falls again) but must not
// repeat the announcement.  Clearing the message log between the
// blows keeps the second check independent of the first.
// ============================================================

static item *br33689_armor;
static creature *br33689_attacker;
static boolean br33689_firstMessage;
static boolean br33689_secondMessage;
static boolean br33689_firstIdentified;
static int br33689_healthAfterFirst;
static int br33689_healthAfterSecond;

static void br33689_setup(void) {
    clearArena();
    br33689_armor = givePlayerRunicArmor(CHAIN_MAIL, A_VULNERABILITY, 10, false);
    setPlayerHealth(1000);
    br33689_attacker = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 1, 0));
    // A paralyzed defender is hit without a roll, so both blows are guaranteed
    // to reach the armor's rune handler.
    applyStatusToPlayer(STATUS_PARALYZED, 100);

    clearMessages();
    monsterAttacksPlayer(br33689_attacker);
    br33689_firstMessage = messageWasShown("wracked with pain");
    br33689_firstIdentified = runicIsIdentified(br33689_armor);
    br33689_healthAfterFirst = playerHealth();

    // The first blow's morale check shakes off paralysis, so restore it to
    // guarantee that the second blow lands as well.
    applyStatusToPlayer(STATUS_PARALYZED, 100);
    clearMessages();
    monsterAttacksPlayer(br33689_attacker);
    br33689_secondMessage = messageWasShown("wracked with pain");
    br33689_healthAfterSecond = playerHealth();
}

static boolean br33689_first_trigger_announces_and_identifies(void) {
    TEST_ASSERT(br33689_healthAfterFirst < 1000,
                "control: the first blow should land");
    TEST_ASSERT(br33689_firstMessage,
                "the first triggering of the vulnerability rune should announce that you are wracked with pain");
    TEST_ASSERT(br33689_firstIdentified,
                "the first triggering should identify the armor's rune");

    TEST_ASSERT(br33689_healthAfterSecond < br33689_healthAfterFirst,
                "control: the second blow should land");
    TEST_ASSERT(!br33689_secondMessage,
                "once the rune is known, triggering it again should produce no message");
    return !testFailed;
}

// ============================================================
// BR33558 — vulnerability armor doubles the damage its wearer takes
// from a thrown weapon, not just from melee blows.
//
// The thrown-weapon hit path resolves the blow through the armor-rune
// handler with the melee flag clear.  This test feeds a fixed amount of
// damage through that same path while wearing vulnerability armor, so
// the doubling (and not the hit roll or the weapon's own damage roll)
// is what the assertion observes.  A second case wears armor with a
// different rune and requires the damage to pass through unchanged,
// showing the doubling belongs to the vulnerability rune.
// ============================================================

#define BR33558_THROWN_DAMAGE 7

static creature *br33558_thrower;
static int br33558_damageWithVulnerability;
static int br33558_damageWithOtherRune;

static void br33558_vulnerability_setup(void) {
    clearArena();
    givePlayerRunicArmor(CHAIN_MAIL, A_VULNERABILITY, 1, true);
    br33558_thrower = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 2, 0));
    br33558_damageWithVulnerability =
        damageAfterArmorRuneForThrownWeapon(br33558_thrower, BR33558_THROWN_DAMAGE);
}

static boolean br33558_vulnerability_doubles_thrown_damage(void) {
    TEST_ASSERT_EQ(br33558_damageWithVulnerability, BR33558_THROWN_DAMAGE * 2,
                   "vulnerability armor should double the damage taken from a thrown weapon");
    return !testFailed;
}

static void br33558_other_rune_setup(void) {
    clearArena();
    givePlayerRunicArmor(CHAIN_MAIL, A_RESPIRATION, 1, true);
    br33558_thrower = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 2, 0));
    br33558_damageWithOtherRune =
        damageAfterArmorRuneForThrownWeapon(br33558_thrower, BR33558_THROWN_DAMAGE);
}

static boolean br33558_other_armor_rune_leaves_thrown_damage_alone(void) {
    TEST_ASSERT_EQ(br33558_damageWithOtherRune, BR33558_THROWN_DAMAGE,
                   "armor with a different rune should not change the damage from a thrown weapon");
    return !testFailed;
}

// ============================================================
// BR34933 — wall-mounted torches cast a warm orange light
// reaching 10 tiles, and candles on altars and pedestals cast
// the same orange light reaching 2 to 4 tiles; both shades
// flicker at every screen refresh.  Before it is triggered, a
// wired wall torch draws its flame in that same flickering
// orange.
//
// A bare floor keeps every other light source away, and the
// player's light is scaled down to its two-tile minimum far from
// the probes, so each sample shows only the tested terrain.  The
// radius is counted as the number of lit tiles including the
// source tile, matching the way a light radius of 3 lights two
// tiles outward.  64 refreshes of each source expose the random
// re-shading (the light colour must change) and the random candle
// radius (all of 2, 3 and 4 tiles must occur), while a fixed
// torch radius must light the same extent every time.  The drawn
// colours and the light colours are compared by name, so equal
// names mean the same shade.
// ============================================================

#define BR34933_TRIALS 64

static pos br34933_source;
static int br34933_torchExtent[12];
static int br34933_candleExtent[6];
static boolean br34933_torchColorVaries;
static boolean br34933_torchAlwaysOrange;
static boolean br34933_candleColorVaries;
static boolean br34933_candleAlwaysOrange;
static int br34933_baselineLight;

static unsigned long br34933_colorId(int red, int green, int blue) {
    return ((unsigned long) (unsigned short) red << 32)
         | ((unsigned long) (unsigned short) green << 16)
         | (unsigned long) (unsigned short) blue;
}

static void br34933_sampleTerrain(short terrain, int trials,
                                  int *extentCounts, int extentLimit,
                                  boolean *colorVaries, boolean *alwaysOrange) {
    unsigned long firstColor = 0;
    setTerrain(br34933_source, terrain);
    for (int trial = 0; trial < trials; trial++) {
        recomputeLighting();

        int red, green, blue;
        lightColorAt(br34933_source, &red, &green, &blue);
        unsigned long color = br34933_colorId(red, green, blue);
        if (trial == 0) {
            firstColor = color;
            *colorVaries = false;
            *alwaysOrange = true;
        } else if (color != firstColor) {
            *colorVaries = true;
        }
        if (!(red > green && green > blue && red >= 2 * blue)) {
            *alwaysOrange = false;
        }

        int farthest = -1;
        for (int distance = 1; distance < extentLimit; distance++) {
            if (lightAt(posOffset(br34933_source, distance, 0)) > 0) {
                farthest = distance;
            }
        }
        if (farthest >= 0 && farthest < extentLimit) {
            extentCounts[farthest]++;
        }
    }
}

static void br34933_light_setup(void) {
    clearMonsters();
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }
    setPlayerPos(posAt(10, 14));
    setLightMultiplier(0);
    br34933_source = posAt(50, 14);

    for (int distance = 0; distance < 12; distance++) {
        br34933_torchExtent[distance] = 0;
    }
    for (int distance = 0; distance < 6; distance++) {
        br34933_candleExtent[distance] = 0;
    }

    setTerrain(br34933_source, FLOOR);
    recomputeLighting();
    br34933_baselineLight = lightAt(br34933_source);
    for (int distance = 1; distance < 12; distance++) {
        br34933_baselineLight += lightAt(posOffset(br34933_source, distance, 0));
    }

    br34933_sampleTerrain(TORCH_WALL, BR34933_TRIALS,
                          br34933_torchExtent, 12,
                          &br34933_torchColorVaries, &br34933_torchAlwaysOrange);

    boolean altarColorVaries, altarAlwaysOrange;
    br34933_sampleTerrain(ALTAR_INERT, BR34933_TRIALS / 2,
                          br34933_candleExtent, 6,
                          &altarColorVaries, &altarAlwaysOrange);

    boolean pedestalColorVaries, pedestalAlwaysOrange;
    br34933_sampleTerrain(PEDESTAL, BR34933_TRIALS / 2,
                          br34933_candleExtent, 6,
                          &pedestalColorVaries, &pedestalAlwaysOrange);
    br34933_candleColorVaries = altarColorVaries || pedestalColorVaries;
    br34933_candleAlwaysOrange = altarAlwaysOrange && pedestalAlwaysOrange;
}

static boolean br34933_torch_and_candle_light_is_flickering_orange(void) {
    TEST_ASSERT_EQ(br34933_baselineLight, 0,
                   "control: bare floor should cast no light at the probes");

    TEST_ASSERT_EQ(br34933_torchExtent[9], BR34933_TRIALS,
                   "a wall torch should light nine tiles beyond its own tile, "
                   "a reach of ten tiles including the source");
    TEST_ASSERT_EQ(br34933_torchExtent[10], 0,
                   "a wall torch should not reach an eleventh tile");
    TEST_ASSERT(br34933_torchColorVaries,
                "the torch light should be re-shaded on every refresh");
    TEST_ASSERT(br34933_torchAlwaysOrange,
                "the torch light should be warm orange (red over green over blue)");

    TEST_ASSERT(br34933_candleExtent[1] > 0,
                "candle light should sometimes reach only two tiles including its source");
    TEST_ASSERT(br34933_candleExtent[2] > 0,
                "candle light should sometimes reach three tiles including its source");
    TEST_ASSERT(br34933_candleExtent[3] > 0,
                "candle light should sometimes reach all four tiles including its source");
    TEST_ASSERT_EQ(br34933_candleExtent[0], 0,
                   "candle light should always light at least its own tile and one more");
    TEST_ASSERT_EQ(br34933_candleExtent[4], 0,
                   "candle light should never reach a fifth tile");
    TEST_ASSERT(br34933_candleColorVaries,
                "the candle light should be re-shaded on every refresh");
    TEST_ASSERT(br34933_candleAlwaysOrange,
                "the candle light should be the same warm orange as the torch light");
    return !testFailed;
}

static const char *br34933_torchDrawName;
static const char *br34933_wiredTorchDrawName;
static const char *br34933_torchGlowName;
static const char *br34933_altarGlowName;
static const char *br34933_pedestalGlowName;
static const char *br34933_wallDrawName;
static const char *br34933_wallGlowName;
static boolean br34933_torchDrawFlickers;
static boolean br34933_wiredTorchDrawFlickers;
static boolean br34933_wallDrawFlickers;

static void br34933_drawn_setup(void) {
    clearMonsters();
    pos torch = posAt(20, 10);
    pos wiredTorch = posAt(24, 10);
    pos altar = posAt(28, 10);
    pos pedestal = posAt(32, 10);
    pos wall = posAt(36, 10);

    setTerrain(torch, TORCH_WALL);
    setTerrain(wiredTorch, PILOT_LIGHT_DORMANT);
    setTerrain(altar, ALTAR_INERT);
    setTerrain(pedestal, PEDESTAL);
    setTerrain(wall, WALL);

    br34933_torchDrawName = terrainDrawColorName(torch);
    br34933_wiredTorchDrawName = terrainDrawColorName(wiredTorch);
    br34933_torchGlowName = terrainGlowLightColorName(torch);
    br34933_altarGlowName = terrainGlowLightColorName(altar);
    br34933_pedestalGlowName = terrainGlowLightColorName(pedestal);
    br34933_wallDrawName = terrainDrawColorName(wall);
    br34933_wallGlowName = terrainGlowLightColorName(wall);
    br34933_torchDrawFlickers = terrainDrawColorFlickers(torch);
    br34933_wiredTorchDrawFlickers = terrainDrawColorFlickers(wiredTorch);
    br34933_wallDrawFlickers = terrainDrawColorFlickers(wall);
}

static boolean br34933_wired_torch_flame_is_flickering_orange(void) {
    TEST_ASSERT(strcmp(br34933_torchDrawName, "torch orange") == 0,
                "an ordinary wall torch should be drawn as a torch-orange flame");
    TEST_ASSERT(strcmp(br34933_wiredTorchDrawName, "torch orange") == 0,
                "a dormant wired wall torch should draw its flame in the same torch orange");
    TEST_ASSERT(br34933_torchDrawFlickers,
                "the ordinary torch flame should flicker on every refresh");
    TEST_ASSERT(br34933_wiredTorchDrawFlickers,
                "the dormant wired torch flame should flicker on every refresh");

    TEST_ASSERT(strcmp(br34933_torchGlowName, "torch orange") == 0,
                "a wall torch should cast an orange light");
    TEST_ASSERT(strcmp(br34933_altarGlowName, br34933_torchGlowName) == 0,
                "an altar's candles should cast the same orange light as a torch");
    TEST_ASSERT(strcmp(br34933_pedestalGlowName, br34933_torchGlowName) == 0,
                "a pedestal's candles should cast the same orange light as a torch");

    TEST_ASSERT(strcmp(br34933_wallDrawName, "torch orange") != 0,
                "control: a plain wall should not be drawn as a torch flame");
    TEST_ASSERT(!br34933_wallDrawFlickers,
                "control: a plain wall's colour should not flicker");
    TEST_ASSERT(strcmp(br34933_wallGlowName, "other") == 0,
                "control: a plain wall should cast no light of its own");
    return !testFailed;
}

// ============================================================
// BR33736 — a wall-mounted torch sputtering with a dim purple flame
// is re-shaded at every screen refresh and casts a flickering
// purple light reaching 4 to 6 tiles, in place of an ordinary
// torch's orange.
//
// The drawn colours and the light colours are compared by name, so
// equal names mean the same shade.  The light itself is sampled over
// a bare floor with the player's light scaled down far away: the
// haunted torch's own tile and every outward tile are measured on
// each of many refreshes, which must show the whole 4-to-6-tile
// radius range, the re-shaded purple colour, and a hard edge at six.
// ============================================================

#define BR33736_TRIALS 64

static const char *br33736_hauntedDrawName;
static const char *br33736_hauntedGlowName;
static const char *br33736_torchDrawName;
static const char *br33736_torchGlowName;
static const char *br33736_wallDrawName;
static const char *br33736_wallGlowName;
static boolean br33736_hauntedDrawFlickers;
static boolean br33736_torchDrawFlickers;
static boolean br33736_wallDrawFlickers;

static void br33736_drawn_setup(void) {
    clearMonsters();
    pos haunted = posAt(20, 10);
    pos torch = posAt(24, 10);
    pos wall = posAt(28, 10);

    setTerrain(haunted, HAUNTED_TORCH);
    setTerrain(torch, TORCH_WALL);
    setTerrain(wall, WALL);

    br33736_hauntedDrawName = terrainDrawColorName(haunted);
    br33736_hauntedGlowName = terrainGlowLightColorName(haunted);
    br33736_torchDrawName = terrainDrawColorName(torch);
    br33736_torchGlowName = terrainGlowLightColorName(torch);
    br33736_wallDrawName = terrainDrawColorName(wall);
    br33736_wallGlowName = terrainGlowLightColorName(wall);
    br33736_hauntedDrawFlickers = terrainDrawColorFlickers(haunted);
    br33736_torchDrawFlickers = terrainDrawColorFlickers(torch);
    br33736_wallDrawFlickers = terrainDrawColorFlickers(wall);
}

static boolean br33736_haunted_torch_flame_is_flickering_purple(void) {
    TEST_ASSERT(strcmp(br33736_hauntedDrawName, "haunted torch purple") == 0,
                "a sputtering torch should be drawn as a purple flame");
    TEST_ASSERT(strcmp(br33736_hauntedDrawName, br33736_torchDrawName) != 0,
                "the sputtering torch should not be drawn in the ordinary torch's orange");
    TEST_ASSERT(br33736_hauntedDrawFlickers,
                "the sputtering torch's flame should flicker on every refresh");

    TEST_ASSERT(strcmp(br33736_hauntedGlowName, br33736_hauntedDrawName) == 0,
                "the sputtering torch should cast the same purple light as its flame");
    TEST_ASSERT(strcmp(br33736_hauntedGlowName, br33736_torchGlowName) != 0,
                "the sputtering torch's light should differ from the ordinary torch's orange");

    TEST_ASSERT(strcmp(br33736_torchDrawName, "torch orange") == 0,
                "control: an ordinary wall torch should be drawn orange");
    TEST_ASSERT(br33736_torchDrawFlickers,
                "control: the ordinary torch flame should flicker");
    TEST_ASSERT(strcmp(br33736_wallDrawName, "haunted torch purple") != 0,
                "control: a plain wall should not be drawn as a purple flame");
    TEST_ASSERT(!br33736_wallDrawFlickers,
                "control: a plain wall's colour should not flicker");
    TEST_ASSERT(strcmp(br33736_wallGlowName, "other") == 0,
                "control: a plain wall should cast no light of its own");
    return !testFailed;
}

static pos br33736_source;
static int br33736_extentCounts[10];
static boolean br33736_colorVaries;
static boolean br33736_alwaysPurple;
static int br33736_baselineLight;
static int br33736_sourceDark;

static void br33736_light_setup(void) {
    clearMonsters();
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }
    setPlayerPos(posAt(10, 14));
    // The miner's light bottoms out at two tiles, which cannot reach the
    // probes eight or more tiles away.
    setLightMultiplier(0);
    br33736_source = posAt(50, 14);

    for (int distance = 0; distance < 10; distance++) {
        br33736_extentCounts[distance] = 0;
    }

    recomputeLighting();
    br33736_baselineLight = 0;
    for (int distance = 0; distance < 10; distance++) {
        br33736_baselineLight += lightAt(posOffset(br33736_source, distance, 0));
    }

    setTerrain(br33736_source, HAUNTED_TORCH);
    unsigned long firstColor = 0;
    br33736_colorVaries = false;
    br33736_alwaysPurple = true;
    br33736_sourceDark = 0;

    for (int trial = 0; trial < BR33736_TRIALS; trial++) {
        recomputeLighting();

        int red, green, blue;
        lightColorAt(br33736_source, &red, &green, &blue);
        unsigned long color = ((unsigned long) (unsigned short) red << 32)
                            | ((unsigned long) (unsigned short) green << 16)
                            | (unsigned long) (unsigned short) blue;
        if (trial == 0) {
            firstColor = color;
        } else if (color != firstColor) {
            br33736_colorVaries = true;
        }
        if (!(red > green && blue > green)) {
            br33736_alwaysPurple = false;
        }

        if (lightAt(br33736_source) <= 0) {
            br33736_sourceDark++;
        }

        int farthest = -1;
        for (int distance = 1; distance < 10; distance++) {
            if (lightAt(posOffset(br33736_source, distance, 0)) > 0) {
                farthest = distance;
            }
        }
        if (farthest >= 0) {
            br33736_extentCounts[farthest]++;
        }
    }
}

static boolean br33736_haunted_torch_light_reaches_four_to_six_tiles(void) {
    TEST_ASSERT_EQ(br33736_baselineLight, 0,
                   "control: bare floor should cast no light at the probes");
    TEST_ASSERT_EQ(br33736_sourceDark, 0,
                   "the sputtering torch should always glow on its own tile");

    TEST_ASSERT(br33736_extentCounts[3] > 0,
                "the purple light should sometimes reach only four tiles including its source");
    TEST_ASSERT(br33736_extentCounts[4] > 0,
                "the purple light should sometimes reach five tiles including its source");
    TEST_ASSERT(br33736_extentCounts[5] > 0,
                "the purple light should sometimes reach all six tiles including its source");
    TEST_ASSERT_EQ(br33736_extentCounts[2], 0,
                   "the purple light should never reach less than four tiles");
    TEST_ASSERT_EQ(br33736_extentCounts[6], 0,
                   "the purple light should never reach a seventh tile");
    TEST_ASSERT_EQ(br33736_extentCounts[9], 0,
                   "the purple light should never reach ten tiles as an ordinary torch does");
    TEST_ASSERT(br33736_colorVaries,
                "the purple light should be re-shaded on every refresh");
    TEST_ASSERT(br33736_alwaysPurple,
                "the sputtering torch's light should be purple (blue stronger than green)");
    return !testFailed;
}

// ============================================================
// BR35743 — a will-o'-the-wisp is drawn in bright blue that
// flickers with every screen refresh and casts a flickering blue
// light reaching 4 to 8 tiles.
//
// The wisp's drawn colour and its own light colour are compared by
// name, with a goblin as the control for a monster that neither
// flickers nor glows.  For the light's reach, a bare floor and a
// distant, dimmed miner's light isolate the wisp: every refresh must
// re-shade its light, and across many refreshes the light must reach
// every extent from four to eight tiles and never beyond eight.
// ============================================================

#define BR35743_TRIALS 128

static creature *br35743_wisp;
static creature *br35743_goblin;
static const char *br35743_wispDrawName;
static const char *br35743_wispGlowName;
static const char *br35743_goblinDrawName;
static const char *br35743_goblinGlowName;
static boolean br35743_wispDrawFlickers;
static boolean br35743_goblinDrawFlickers;

static void br35743_drawn_setup(void) {
    clearMonsters();
    br35743_wisp = placeHostileMonster(MK_WILL_O_THE_WISP, posAt(20, 10));
    br35743_goblin = placeHostileMonster(MK_GOBLIN, posAt(24, 10));

    br35743_wispDrawName = monsterDrawColorName(br35743_wisp);
    br35743_wispGlowName = monsterGlowLightColorName(br35743_wisp);
    br35743_goblinDrawName = monsterDrawColorName(br35743_goblin);
    br35743_goblinGlowName = monsterGlowLightColorName(br35743_goblin);
    br35743_wispDrawFlickers = monsterDrawColorFlickers(br35743_wisp);
    br35743_goblinDrawFlickers = monsterDrawColorFlickers(br35743_goblin);
}

static boolean br35743_wisp_is_flickering_blue(void) {
    TEST_ASSERT(br35743_wisp != NULL, "the wisp should be placed");
    TEST_ASSERT(br35743_goblin != NULL, "the control goblin should be placed");

    TEST_ASSERT(strcmp(br35743_wispDrawName, "wisp blue") == 0,
                "a will-o'-the-wisp should be drawn in bright blue");
    TEST_ASSERT(br35743_wispDrawFlickers,
                "the wisp's drawn colour should flicker on every refresh");
    TEST_ASSERT(strcmp(br35743_wispGlowName, br35743_wispDrawName) == 0,
                "the wisp should cast the same blue light as its drawn colour");

    TEST_ASSERT(strcmp(br35743_goblinDrawName, "wisp blue") != 0,
                "control: a goblin should not be drawn in the wisp's blue");
    TEST_ASSERT(!br35743_goblinDrawFlickers,
                "control: a goblin's colour should not flicker");
    TEST_ASSERT(strcmp(br35743_goblinGlowName, "other") == 0,
                "control: a goblin should cast no light of its own");
    return !testFailed;
}

static pos br35743_source;
static int br35743_extentCounts[12];
static boolean br35743_colorVaries;
static boolean br35743_alwaysBlue;
static int br35743_baselineLight;
static int br35743_sourceDark;

static void br35743_light_setup(void) {
    clearMonsters();
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }
    setPlayerPos(posAt(10, 14));
    // The miner's light bottoms out at two tiles, which cannot reach the
    // probes eight or more tiles away.
    setLightMultiplier(0);
    br35743_source = posAt(50, 14);

    for (int distance = 0; distance < 12; distance++) {
        br35743_extentCounts[distance] = 0;
    }

    recomputeLighting();
    br35743_baselineLight = 0;
    for (int distance = 0; distance < 12; distance++) {
        br35743_baselineLight += lightAt(posOffset(br35743_source, distance, 0));
    }

    placeHostileMonster(MK_WILL_O_THE_WISP, br35743_source);
    unsigned long firstColor = 0;
    br35743_colorVaries = false;
    br35743_alwaysBlue = true;
    br35743_sourceDark = 0;

    for (int trial = 0; trial < BR35743_TRIALS; trial++) {
        recomputeLighting();

        int red, green, blue;
        lightColorAt(br35743_source, &red, &green, &blue);
        unsigned long color = ((unsigned long) (unsigned short) red << 32)
                            | ((unsigned long) (unsigned short) green << 16)
                            | (unsigned long) (unsigned short) blue;
        if (trial == 0) {
            firstColor = color;
        } else if (color != firstColor) {
            br35743_colorVaries = true;
        }
        if (!(blue > red && blue > green)) {
            br35743_alwaysBlue = false;
        }

        if (lightAt(br35743_source) <= 0) {
            br35743_sourceDark++;
        }

        int farthest = -1;
        for (int distance = 1; distance < 12; distance++) {
            if (lightAt(posOffset(br35743_source, distance, 0)) > 0) {
                farthest = distance;
            }
        }
        if (farthest >= 0) {
            br35743_extentCounts[farthest]++;
        }
    }
}

static boolean br35743_wisp_light_reaches_four_to_eight_tiles(void) {
    TEST_ASSERT_EQ(br35743_baselineLight, 0,
                   "control: bare floor should cast no light at the probes");
    TEST_ASSERT_EQ(br35743_sourceDark, 0,
                   "the wisp should always glow on its own tile");

    TEST_ASSERT(br35743_extentCounts[3] > 0,
                "the blue light should sometimes reach only four tiles including its source");
    TEST_ASSERT(br35743_extentCounts[4] > 0,
                "the blue light should sometimes reach five tiles including its source");
    TEST_ASSERT(br35743_extentCounts[5] > 0,
                "the blue light should sometimes reach six tiles including its source");
    TEST_ASSERT(br35743_extentCounts[6] > 0,
                "the blue light should sometimes reach seven tiles including its source");
    TEST_ASSERT(br35743_extentCounts[7] > 0,
                "the blue light should sometimes reach all eight tiles including its source");
    TEST_ASSERT_EQ(br35743_extentCounts[2], 0,
                   "the blue light should never reach less than four tiles");
    TEST_ASSERT_EQ(br35743_extentCounts[8], 0,
                   "the blue light should never reach a ninth tile");
    TEST_ASSERT_EQ(br35743_extentCounts[11], 0,
                   "the blue light should never reach twelve tiles");
    TEST_ASSERT(br35743_colorVaries,
                "the blue light should be re-shaded on every refresh");
    TEST_ASSERT(br35743_alwaysBlue,
                "the wisp's light should be blue (blue stronger than red and green)");
    return !testFailed;
}

// ============================================================
// BR39783 — An ifrit carries a blue light reaching 3 to 6 tiles
// around it that flickers with every screen refresh.
//
// The light the ifrit's catalog entry names is asked about
// directly: it must be red-and-green-poor blue, reach at least
// three tiles, and re-shade on refresh.  The light actually
// painted on open floor is then measured around a lone ifrit:
// across many refreshes it must reach every extent from three to
// six tiles, never less or more, and must always keep a blue
// shade.
// ============================================================

#define BR39783_TRIALS 128

static creature *br39783_ifrit;
static pos br39783_source;
static int br39783_extentCounts[12];
static boolean br39783_colorVaries;
static boolean br39783_alwaysBlue;
static int br39783_sourceDark;
static int br39783_baselineLight;

static void br39783_setup(void) {
    clearMonsters();
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }
    // Keep the miner's light away from the probes and tiny.
    setPlayerPos(posAt(10, 14));
    setLightMultiplier(0);
    br39783_source = posAt(50, 14);

    for (int distance = 0; distance < 12; distance++) {
        br39783_extentCounts[distance] = 0;
    }

    recomputeLighting();
    br39783_baselineLight = 0;
    for (int distance = 0; distance < 12; distance++) {
        br39783_baselineLight += lightAt(posOffset(br39783_source, distance, 0));
    }

    br39783_ifrit = placeHostileMonster(MK_IFRIT, br39783_source);
    unsigned long firstColor = 0;
    br39783_colorVaries = false;
    br39783_alwaysBlue = true;
    br39783_sourceDark = 0;

    for (int trial = 0; trial < BR39783_TRIALS; trial++) {
        recomputeLighting();

        int red, green, blue;
        lightColorAt(br39783_source, &red, &green, &blue);
        unsigned long color = ((unsigned long) (unsigned short) red << 32)
                            | ((unsigned long) (unsigned short) green << 16)
                            | (unsigned long) (unsigned short) blue;
        if (trial == 0) {
            firstColor = color;
        } else if (color != firstColor) {
            br39783_colorVaries = true;
        }
        if (!(blue > red && blue > green)) {
            br39783_alwaysBlue = false;
        }
        if (lightAt(br39783_source) <= 0) {
            br39783_sourceDark++;
        }

        int farthest = -1;
        for (int distance = 1; distance < 12; distance++) {
            if (lightAt(posOffset(br39783_source, distance, 0)) > 0) {
                farthest = distance;
            }
        }
        if (farthest >= 0) {
            br39783_extentCounts[farthest]++;
        }
    }
}

static boolean br39783_ifrit_light_is_flickering_blue_three_to_six_tiles(void) {
    TEST_ASSERT(br39783_ifrit != NULL, "the ifrit should be placed");

    int red, green, blue;
    monsterGlowLightColorComponents(br39783_ifrit, &red, &green, &blue);
    TEST_ASSERT(blue > red && blue > green,
                "the ifrit's light should be blue (blue stronger than red and green)");
    TEST_ASSERT_EQ((int) (monsterGlowLightRadius(br39783_ifrit) * 100 + 0.5), 300,
                   "the ifrit's light should reach at least three tiles");
    TEST_ASSERT(monsterGlowLightFlickers(br39783_ifrit),
                "the ifrit's light should flicker on every refresh");

    TEST_ASSERT_EQ(br39783_baselineLight, 0,
                   "control: bare floor should cast no light at the probes");
    TEST_ASSERT_EQ(br39783_sourceDark, 0,
                   "the ifrit should always glow on its own tile");

    TEST_ASSERT(br39783_extentCounts[2] > 0,
                "the blue light should sometimes reach only three tiles including its source");
    TEST_ASSERT(br39783_extentCounts[3] > 0,
                "the blue light should sometimes reach four tiles including its source");
    TEST_ASSERT(br39783_extentCounts[4] > 0,
                "the blue light should sometimes reach five tiles including its source");
    TEST_ASSERT(br39783_extentCounts[5] > 0,
                "the blue light should sometimes reach all six tiles including its source");
    TEST_ASSERT_EQ(br39783_extentCounts[1], 0,
                   "the blue light should never reach less than three tiles");
    TEST_ASSERT_EQ(br39783_extentCounts[6], 0,
                   "the blue light should never reach a seventh tile");
    TEST_ASSERT_EQ(br39783_extentCounts[11], 0,
                   "the blue light should never reach twelve tiles");
    TEST_ASSERT(br39783_colorVaries,
                "the blue light should be re-shaded on every refresh");
    TEST_ASSERT(br39783_alwaysBlue,
                "the ifrit's light should always be blue (blue stronger than red and green)");
    return !testFailed;
}

// ============================================================
// BR29777 — Spectral images that appear at your side cast a steady
// pink-red light reaching 3.5 tiles, lighting the squares where
// they fight.
//
// Multiplicity armor is made to flash, and the images it conjures
// are asked about the light they carry: it must be red-dominant
// with a pink tint (blue above green), reach exactly 3.5 tiles,
// and keep the same shade on every refresh.  A second test puts a
// spectral image on open floor and measures the light actually
// painted on the squares around it.
// ============================================================

static creature *br29777_attacker;
static int br29777_imagesBefore;
static int br29777_attempts;
static pos br29777_source;
static int br29777_baseline[5];
static int br29777_lit[5];
static unsigned long br29777_firstColor;
static boolean br29777_colorVaries;

static void br29777_conjure_setup(void) {
    clearArena();
    givePlayerRunicArmor(LEATHER_ARMOR, A_MULTIPLICITY, 20, true);
    br29777_imagesBefore = countMonstersOfType(MK_SPECTRAL_IMAGE);
    br29777_attacker = placeHostileMonster(MK_GOBLIN, posOffset(getPlayerPos(), 1, 0));
    // A paralyzed defender is always hit, so each blow gives the armor its
    // chance to flash without depending on the goblin's accuracy.
    applyStatusToPlayer(STATUS_PARALYZED, 1000);
    br29777_attempts = 0;
}

static boolean br29777_conjure_step(void) {
    setPlayerHealth(playerMaxHealth());
    monsterAttacksPlayer(br29777_attacker);
    br29777_attempts++;
    return countMonstersOfType(MK_SPECTRAL_IMAGE) <= br29777_imagesBefore;
}

static boolean br29777_conjured_image_light_is_pink_red_steady_3_5_tiles(void) {
    creature *image = findMonster(MK_SPECTRAL_IMAGE);

    TEST_ASSERT(br29777_attempts > 0 && br29777_attempts < 10000,
                "control: multiplicity armor should flash within the step limit");
    TEST_ASSERT(image != NULL,
                "multiplicity armor should conjure at least one spectral image");
    if (image == NULL) {
        return !testFailed;
    }

    TEST_ASSERT(strcmp(monsterGlowLightColorName(image), "spectral image pink-red") == 0,
                "a spectral image's light should be the pink-red conjured-image light");

    int red, green, blue;
    monsterGlowLightColorComponents(image, &red, &green, &blue);
    TEST_ASSERT(red > green && red > blue,
                "the spectral image's light should be red-dominant");
    TEST_ASSERT(blue > green,
                "the spectral image's red light should have a pink (blue) tint");

    TEST_ASSERT_EQ((int) (monsterGlowLightRadius(image) * 100 + 0.5), 350,
                   "the spectral image's light should reach 3.5 tiles");
    TEST_ASSERT(!monsterGlowLightFlickers(image),
                "the spectral image's light should be steady, not re-rolled on refresh");
    return !testFailed;
}

static void br29777_illumination_setup(void) {
    clearMonsters();
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }
    // Keep the miner's light away from the probes and tiny.
    setPlayerPos(posAt(10, 14));
    setLightMultiplier(0);
    br29777_source = posAt(50, 14);

    recomputeLighting();
    for (int distance = 0; distance <= 4; distance++) {
        br29777_baseline[distance] = lightAt(posOffset(br29777_source, distance, 0));
    }

    placeHostileMonster(MK_SPECTRAL_IMAGE, br29777_source);

    br29777_firstColor = 0;
    br29777_colorVaries = false;
    for (int trial = 0; trial < 64; trial++) {
        recomputeLighting();
        int red, green, blue;
        lightColorAt(br29777_source, &red, &green, &blue);
        unsigned long color = ((unsigned long) (unsigned short) red << 32)
                            | ((unsigned long) (unsigned short) green << 16)
                            | (unsigned long) (unsigned short) blue;
        if (trial == 0) {
            br29777_firstColor = color;
        } else if (color != br29777_firstColor) {
            br29777_colorVaries = true;
        }
    }
    for (int distance = 0; distance <= 4; distance++) {
        br29777_lit[distance] = lightAt(posOffset(br29777_source, distance, 0));
    }
}

static boolean br29777_spectral_image_light_reaches_three_tiles(void) {
    TEST_ASSERT_EQ(br29777_baseline[0] + br29777_baseline[1] + br29777_baseline[2]
                       + br29777_baseline[3] + br29777_baseline[4], 0,
                   "control: bare floor should cast no light at the probes");

    for (int distance = 0; distance <= 3; distance++) {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "a spectral image should light the square %d tiles from it", distance);
        TEST_ASSERT(br29777_lit[distance] > br29777_baseline[distance], msg);
    }
    TEST_ASSERT_EQ(br29777_lit[4], br29777_baseline[4],
                   "a 3.5-tile light should not reach a square four tiles away");
    TEST_ASSERT(!br29777_colorVaries,
                "the spectral image's light should keep the same shade on every refresh");
    return !testFailed;
}

// ============================================================
// BR29041 — a bolt from a staff of poison poisons the creature it hits
// for 5 turns at +2 enchantment or lower, and each enchantment level
// above that lengthens the poisoning by about 30% (5 × 1.3^(enchantment
// − 2), rounded down, so +3 gives 6 turns and +10 gives 40).
//
// Each enchantment is fired at a fresh target: the staff's enchantment is
// the bolt magnitude the engine's formula reads, so the duration left on
// the struck creature is the rule's whole observable result.
// ============================================================

static int br29041_turnsAt1;
static int br29041_turnsAt2;
static int br29041_turnsAt3;
static int br29041_turnsAt10;

static int br29041_poisonTurnsFor(int enchant) {
    clearMonsters();
    creature *target = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 4, 0));
    item *staff = givePlayerItem(STAFF, STAFF_POISON, enchant);
    playerZapsStaffAt(staff, target);
    return monsterStatusTurns(target, STATUS_POISONED);
}

static void br29041_setup(void) {
    setArenaRadius(8, FLOOR);
    br29041_turnsAt1 = br29041_poisonTurnsFor(1);
    br29041_turnsAt2 = br29041_poisonTurnsFor(2);
    br29041_turnsAt3 = br29041_poisonTurnsFor(3);
    br29041_turnsAt10 = br29041_poisonTurnsFor(10);
}

static boolean br29041_poison_staff_duration_scales_with_enchantment(void) {
    TEST_ASSERT_EQ(br29041_turnsAt1, 5,
                   "a +1 staff of poison should poison for 5 turns (the floor)");
    TEST_ASSERT_EQ(br29041_turnsAt2, 5,
                   "a +2 staff of poison should poison for 5 turns");
    TEST_ASSERT_EQ(br29041_turnsAt3, 6,
                   "+3 staff of poison should poison for 6 turns");
    TEST_ASSERT_EQ(br29041_turnsAt10, 40,
                   "+10 staff of poison should poison for 40 turns");
    TEST_ASSERT(br29041_turnsAt3 > br29041_turnsAt2,
                "each enchantment above +2 should lengthen the poisoning");
    return !testFailed;
}

// ============================================================
// BR38509 — the Game Stats screen's Current Win Streak and Current
// Mastery Streak rows each show only the recent figure, with no
// all-time counterpart, unlike the Longest streak rows just above
// them, which carry an All Time and a Recent value.
//
// showGameStatsScreen() holds the real stats frame.  The Longest
// rows are built as two right-aligned 16-column values, while the
// Current rows are built as one right-aligned 32-column value.  So
// the Current value ends in the Recent column and begins after the
// All Time column has ended.  Reading the digit runs' columns proves
// both that the Recent figure is present and that no All Time figure
// shares its row.
// ============================================================

// Record the first and last digit of every run of digits on a screen line.
// Returns the number of runs.  Positions are column indexes in the line.
static int br38509_digitRuns(const char *line,
                             int *firstStart, int *firstEnd,
                             int *lastStart, int *lastEnd) {
    int runs = 0;
    int i = 0;
    *firstStart = *firstEnd = *lastStart = *lastEnd = -1;
    while (line[i] != '\0') {
        if (line[i] >= '0' && line[i] <= '9') {
            const int start = i;
            while (line[i] >= '0' && line[i] <= '9') {
                i++;
            }
            if (runs == 0) {
                *firstStart = start;
                *firstEnd = i - 1;
            }
            *lastStart = start;
            *lastEnd = i - 1;
            runs++;
        } else {
            i++;
        }
    }
    return runs;
}

static void br38509_check_row(const char *currentLabel, const char *longestLabel) {
    int longestFirstStart, longestFirstEnd, longestLastStart, longestLastEnd, longestRuns;
    int currentFirstStart, currentFirstEnd, currentLastStart, currentLastEnd, currentRuns;
    const char *longest = screenTextLineContaining(longestLabel);
    const char *current = screenTextLineContaining(currentLabel);
    char msg[300];

    longestRuns = br38509_digitRuns(longest, &longestFirstStart, &longestFirstEnd,
                                    &longestLastStart, &longestLastEnd);
    currentRuns = br38509_digitRuns(current, &currentFirstStart, &currentFirstEnd,
                                    &currentLastStart, &currentLastEnd);

    snprintf(msg, sizeof(msg), "the \"%s\" row should show an All Time and a Recent value, as \"%s\" does (line \"%s\")",
             longestLabel, currentLabel, longest);
    TEST_ASSERT(longestRuns == 2, msg);

    snprintf(msg, sizeof(msg), "the \"%s\" row should show a single Recent value, with no All Time value (line \"%s\")",
             currentLabel, current);
    TEST_ASSERT(currentRuns == 1, msg);

    snprintf(msg, sizeof(msg), "the \"%s\" value should sit in the Recent column, at the same right edge as the \"%s\" Recent value",
             currentLabel, longestLabel);
    TEST_ASSERT_EQ(currentLastEnd, longestLastEnd, msg);

    snprintf(msg, sizeof(msg), "the \"%s\" row should hold no value in the All Time column, which ends at column %d",
             currentLabel, longestFirstEnd);
    TEST_ASSERT(currentFirstStart > longestFirstEnd, msg);
}

static void br38509_setup(void) {
    showGameStatsScreen();
}

static boolean br38509_current_streaks_show_only_recent(void) {
    TEST_ASSERT(screenTextContains("Longest Win Streak"),
                "the Game Stats screen should show the Longest Win Streak row");
    TEST_ASSERT(screenTextContains("Longest Mastery Streak"),
                "the Game Stats screen should show the Longest Mastery Streak row");
    TEST_ASSERT(screenTextContains("Current Win Streak"),
                "the Game Stats screen should show the Current Win Streak row");
    TEST_ASSERT(screenTextContains("Current Mastery Streak"),
                "the Game Stats screen should show the Current Mastery Streak row");
    br38509_check_row("Current Win Streak", "Longest Win Streak");
    br38509_check_row("Current Mastery Streak", "Longest Mastery Streak");
    return !testFailed;
}

// ============================================================
// BR40200 — when a blow lands on a paralyzed defender, the combat
// message explains that the defender is struck while paralyzed.
//
// A paralyzed rat is always hit and outlasts the blow, so the
// surviving-defender message is the one that reaches the log.  The
// message is buffered until the turn ends, so a rest flushes it
// before the clause is read back.
// ============================================================

static void br40200_setup(void) {
    clearMonsters();
    setArenaRadius(4, FLOOR);
    const pos targetPos = posOffset(getPlayerPos(), 1, 0);
    creature *target = placeHostileMonster(MK_RAT, targetPos);
    revealCell(targetPos);
    setMonsterHealth(target, 100); // survive the guaranteed triple-damage blow
    applyStatusToMonster(target, STATUS_PARALYZED, 100);
    playerAttacksMonster(target);
    restOneTurn(); // the turn's end flushes the buffered combat message
}

static boolean br40200_message_explains_paralyzed_defender(void) {
    TEST_ASSERT(messageWasShown("while it is paralyzed"),
                "a blow on a paralyzed defender should report that it was struck while paralyzed");
    return !testFailed;
}

// ============================================================
// BR18636 — when a blow lands on a sleeping defender, the combat
// message explains that the defender was struck in its sleep.
//
// A sleeping rat is always hit and, with plenty of health, survives
// the guaranteed triple-damage blow, so the surviving-defender
// message is the one that reaches the log.  The message is buffered
// until the turn ends, so a rest flushes it before the clause is
// read back.
// ============================================================

static void br18636_setup(void) {
    clearMonsters();
    setArenaRadius(4, FLOOR);
    const pos targetPos = posOffset(getPlayerPos(), 1, 0);
    creature *target = placeHostileMonster(MK_RAT, targetPos);
    revealCell(targetPos);
    setMonsterHealth(target, 100); // survive the guaranteed triple-damage blow
    putMonsterToSleep(target);
    playerAttacksMonster(target);
    restOneTurn(); // the turn's end flushes the buffered combat message
}

static boolean br18636_message_explains_sleeping_defender(void) {
    TEST_ASSERT(messageWasShown("in its sleep"),
                "a blow on a sleeping defender should report that it was struck in its sleep");
    return !testFailed;
}

// ============================================================
// BR17360 — randomizing the order of a list produces a fair
// shuffle in which every possible ordering is equally likely.
//
// shuffleTestSequence() samples the game's own shuffle of three
// entries 100,000 times.  A fair shuffle gives each of the six
// orderings about 16,667 turns; the chi-square statistic has five
// degrees of freedom, and a fair shuffle exceeds 60 with probability
// around 1e-11.  A shuffle that swaps each position with any other
// position skews those six counts by hundreds, and a shuffle that
// only produces cyclic orderings leaves three of them at zero.
// ============================================================

#define BR17360_TRIALS 100000

static int br17360_counts[6];

// The lexicographic rank of one ordering of the values 0, 1, and 2.
static int br17360_permutation(const short *values) {
    if (values[0] == 0) {
        return values[1] == 1 ? 0 : 1;
    }
    if (values[0] == 1) {
        return values[1] == 0 ? 2 : 3;
    }
    return values[1] == 0 ? 4 : 5;
}

static void br17360_setup(void) {
    for (int i = 0; i < 6; i++) {
        br17360_counts[i] = 0;
    }
    for (int trial = 0; trial < BR17360_TRIALS; trial++) {
        short values[3];
        shuffleTestSequence(values, 3);
        br17360_counts[br17360_permutation(values)]++;
    }
}

static boolean br17360_shuffle_is_fair(void) {
    const double expected = BR17360_TRIALS / 6.0;
    double chiSquare = 0.0;
    int total = 0;

    for (int i = 0; i < 6; i++) {
        const double deviation = br17360_counts[i] - expected;
        chiSquare += deviation * deviation / expected;
        total += br17360_counts[i];
    }

    TEST_ASSERT_EQ(total, BR17360_TRIALS,
                   "every sampled shuffle should produce exactly one of the six orderings");
    for (int i = 0; i < 6; i++) {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "the shuffled list should be able to take ordering %d, but it never did", i);
        TEST_ASSERT(br17360_counts[i] > 0, msg);
    }

    char msg[240];
    snprintf(msg, sizeof(msg),
             "all six orderings of three entries should be about equally likely (chi-square %.1f over 5 degrees of freedom)",
             chiSquare);
    TEST_ASSERT(chiSquare < 60.0, msg);
    return !testFailed;
}

// ============================================================
// BR33997 — a draw from a range with a clumping factor above one
// adds that many separate uniform rolls, so its values cluster
// toward the middle of the range instead of being equally likely;
// when the range's width is not evenly divisible by the factor, the
// first few rolls may each reach one higher so the top of the range
// can still come up.
//
// drawClumpedRange() samples the game's own roll of 0..10 with a
// clumping factor of three: one roll of 0..4 plus two rolls of 0..3.
// That sum puts 17.5% on the middle value 5 and 1.25% on each end,
// so the five middle values 3..7 take three quarters of the draws.
// An even spread would put 9.1% on every value and only 45% in those
// five values, so the checks below separate the two shapes by a wide
// margin.  A second sample uses a clumping factor of four on the same
// range, where two of the four rolls get the extra side; the upper
// bound 10 is reachable only because of that extra side, since four
// base-sided rolls would stop at 8.
// ============================================================

#define BR33997_TRIALS 100000
#define BR33997_LOW 0
#define BR33997_HIGH 10
#define BR33997_VALUES (BR33997_HIGH - BR33997_LOW + 1)

static int br33997_counts[BR33997_VALUES];
static int br33997_out_of_range;
static int br33997_top_of_range_count;
static int br33997_clump_four_out_of_range;

static void br33997_clump_three_setup(void) {
    br33997_out_of_range = 0;
    for (int i = 0; i < BR33997_VALUES; i++) {
        br33997_counts[i] = 0;
    }
    for (int trial = 0; trial < BR33997_TRIALS; trial++) {
        const int value = drawClumpedRange(BR33997_LOW, BR33997_HIGH, 3);
        if (value < BR33997_LOW || value > BR33997_HIGH) {
            br33997_out_of_range++;
        } else {
            br33997_counts[value - BR33997_LOW]++;
        }
    }
}

static boolean br33997_clumped_rolls_cluster_toward_the_middle(void) {
    int total = 0;
    int middleMass = 0;
    int edgeMass = 0;

    for (int i = 0; i < BR33997_VALUES; i++) {
        total += br33997_counts[i];
    }
    for (int value = 3; value <= 7; value++) {
        middleMass += br33997_counts[value - BR33997_LOW];
    }
    for (int value = 0; value <= 2; value++) {
        edgeMass += br33997_counts[value - BR33997_LOW];
    }
    for (int value = 8; value <= 10; value++) {
        edgeMass += br33997_counts[value - BR33997_LOW];
    }

    TEST_ASSERT_EQ(br33997_out_of_range, 0,
                   "a clumped roll must stay inside its bounds");
    TEST_ASSERT_EQ(total, BR33997_TRIALS,
                   "every clumped roll should land somewhere in the range");

    for (int i = 0; i < BR33997_VALUES; i++) {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "a clumped roll should be able to take value %d, but it never did",
                 i + BR33997_LOW);
        TEST_ASSERT(br33997_counts[i] > 0, msg);
    }

    char msg[240];
    snprintf(msg, sizeof(msg),
             "the middle of the range should be far more likely than either end "
             "(middle %d, low %d, high %d)",
             br33997_counts[5], br33997_counts[0], br33997_counts[10]);
    TEST_ASSERT(br33997_counts[5] > 4 * br33997_counts[0]
                    && br33997_counts[5] > 4 * br33997_counts[10],
                msg);

    snprintf(msg, sizeof(msg),
             "clumped rolls should pile into the middle of the range "
             "(values 3-7: %d, values 0-2 and 8-10: %d)",
             middleMass, edgeMass);
    TEST_ASSERT(middleMass > 2 * edgeMass, msg);
    return !testFailed;
}

static void br33997_clump_four_setup(void) {
    br33997_top_of_range_count = 0;
    br33997_clump_four_out_of_range = 0;
    for (int trial = 0; trial < BR33997_TRIALS; trial++) {
        const int value = drawClumpedRange(BR33997_LOW, BR33997_HIGH, 4);
        if (value < BR33997_LOW || value > BR33997_HIGH) {
            br33997_clump_four_out_of_range++;
        } else if (value == BR33997_HIGH) {
            br33997_top_of_range_count++;
        }
    }
}

static boolean br33997_top_of_range_survives_a_remainder(void) {
    TEST_ASSERT_EQ(br33997_clump_four_out_of_range, 0,
                   "a clumped roll must stay inside its bounds");

    char msg[200];
    snprintf(msg, sizeof(msg),
             "the top of a range whose width is not divisible by the clumping factor "
             "should still come up (saw %d top rolls in %d draws)",
             br33997_top_of_range_count, BR33997_TRIALS);
    // With a clumping factor of four over 0..10, two rolls reach 0..3 and two
    // reach 0..2, so the top comes up about 1 draw in 144.  Four base-sided
    // rolls would top out at 8 and this count would be zero.
    TEST_ASSERT(br33997_top_of_range_count > 200, msg);
    return !testFailed;
}

// ============================================================
// BR30955 — when you catch fire, the light you carry turns fiery
// red-orange and your figure is drawn in flickering torch-orange;
// both return to normal once the fire is extinguished.
//
// Standing on a fire tile and applying the tile effects runs the
// engine's own catch-fire path.  The carried light is measured as
// its RGB components (red over green over blue), the figure's colour
// is read by name, and both flicker flags are read.  The water branch
// of the same tile-effect routine then puts the fire out, and both
// colours must return to exactly the values recorded beforehand.
// ============================================================

static const char *br30955_normalDrawName;
static const char *br30955_normalLightName;
static unsigned long br30955_normalDrawId;
static const char *br30955_burningDrawName;
static const char *br30955_burningLightName;
static unsigned long br30955_burningDrawId;
static boolean br30955_burningDrawFlickers;
static boolean br30955_burningLightFlickers;
static int br30955_burningLightRed;
static int br30955_burningLightGreen;
static int br30955_burningLightBlue;
static const char *br30955_extinguishedDrawName;
static const char *br30955_extinguishedLightName;
static unsigned long br30955_extinguishedDrawId;

static void br30955_setup(void) {
    clearMonsters();
    clearArena();
    setTerrain(getPlayerPos(), FLOOR);
    recomputeLighting();
    updatePlayerDrawColor();

    br30955_normalDrawName = playerDrawColorName();
    br30955_normalLightName = playerLightColorName();
    br30955_normalDrawId = playerDrawColorId();

    // Catch fire through the engine's ordinary standing-in-fire path.
    spawnFeature(getPlayerPos(), DF_PLAIN_FIRE);
    applyTileEffectsToPlayer();

    br30955_burningDrawName = playerDrawColorName();
    br30955_burningLightName = playerLightColorName();
    br30955_burningDrawId = playerDrawColorId();
    br30955_burningDrawFlickers = playerDrawColorFlickers();
    br30955_burningLightFlickers = playerLightColorFlickers();
    playerLightColorComponents(&br30955_burningLightRed,
                               &br30955_burningLightGreen,
                               &br30955_burningLightBlue);

    // Water uses the same tile-effect routine to put the fire out.
    setTerrain(getPlayerPos(), SHALLOW_WATER);
    applyTileEffectsToPlayer();

    br30955_extinguishedDrawName = playerDrawColorName();
    br30955_extinguishedLightName = playerLightColorName();
    br30955_extinguishedDrawId = playerDrawColorId();
}

static boolean br30955_burning_colors_are_red_orange_and_restore(void) {
    TEST_ASSERT(strcmp(br30955_normalLightName, "miners light") == 0,
                "control: an unburned player should carry the ordinary miners light");
    TEST_ASSERT(strcmp(br30955_normalDrawName, "torch orange") != 0,
                "control: an unburned player should not be drawn in the torch colour");

    TEST_ASSERT(strcmp(br30955_burningLightName, "fire") == 0,
                "catching fire should change the carried light to the fire colour");
    TEST_ASSERT(br30955_burningLightFlickers,
                "the fire light should be re-shaded on every refresh");
    TEST_ASSERT(br30955_burningLightRed > br30955_burningLightGreen
                    && br30955_burningLightGreen > br30955_burningLightBlue,
                "the fire light should be a fiery red-orange (red over green over blue)");

    TEST_ASSERT(strcmp(br30955_burningDrawName, "torch orange") == 0,
                "a burning player should be drawn in the torch-orange colour");
    TEST_ASSERT(br30955_burningDrawFlickers,
                "the burning player's torch-orange figure should flicker on every refresh");
    TEST_ASSERT(br30955_normalDrawId != br30955_burningDrawId,
                "the burning figure colour should differ from the normal figure colour");

    TEST_ASSERT(!isPlayerBurning(), "the water should have extinguished the fire");
    TEST_ASSERT(strcmp(br30955_extinguishedLightName, "miners light") == 0,
                "extinguishing the fire should restore the ordinary carried light");
    TEST_ASSERT(strcmp(br30955_extinguishedLightName, br30955_normalLightName) == 0,
                "the restored carried light should match the light from before the fire");
    TEST_ASSERT(strcmp(br30955_extinguishedDrawName, br30955_normalDrawName) == 0,
                "extinguishing the fire should restore the figure's normal colour");
    TEST_ASSERT(br30955_extinguishedDrawId == br30955_normalDrawId,
                "the restored figure colour should be exactly the colour from before the fire");
    return !testFailed;
}

// ============================================================
// BR38512 — the message archive is a fixed-length rolling history:
// once it is full, each new message overwrites the oldest archived
// message, so the earliest messages of a very long game can no
// longer be reviewed.
//
// The game is restarted to begin from an empty archive.  A marker
// message is followed by exactly enough distinct fillers to fill
// every slot; the marker is then the oldest entry and is still
// present.  One more message rolls the archive forward: the marker
// is gone, the first filler is the oldest entry, and the newest
// message is the one just emitted.
// ============================================================

#define BR38512_MARKER "the oldest archived message"
#define BR38512_NEWEST "the newest archived message"

static int br38512_capacity;
static int br38512_countAtFull;
static int br38512_countAfterRollover;
static boolean br38512_markerIsOldest;
static boolean br38512_lastFillerIsNewest;
static boolean br38512_markerSurvivesWhenFull;
static boolean br38512_markerIsGone;
static boolean br38512_firstFillerIsOldest;
static boolean br38512_newestIsNewest;

static void br38512_filler(int index, char *buffer, size_t size) {
    snprintf(buffer, size, "archived filler %d", index);
}

static void br38512_setup(void) {
    restartGame();
    br38512_capacity = messageArchiveCapacity();
    TEST_ASSERT(br38512_capacity > 1,
                "the message archive should have room for more than one message");
    TEST_ASSERT_EQ(messageArchiveEntryCount(), 0,
                   "a freshly started game should have an empty message archive");

    emitArchivedMessage(BR38512_MARKER);
    char filler[64];
    for (int i = 0; i < br38512_capacity - 1; i++) {
        br38512_filler(i, filler, sizeof(filler));
        emitArchivedMessage(filler);
    }

    br38512_countAtFull = messageArchiveEntryCount();
    br38512_markerIsOldest = strcmp(messageArchiveEntry(br38512_capacity),
                                    BR38512_MARKER) == 0;
    br38512_filler(br38512_capacity - 2, filler, sizeof(filler));
    br38512_lastFillerIsNewest = strcmp(messageArchiveEntry(1), filler) == 0;

    br38512_markerSurvivesWhenFull = false;
    for (int back = 1; back <= br38512_capacity; back++) {
        if (strcmp(messageArchiveEntry(back), BR38512_MARKER) == 0) {
            br38512_markerSurvivesWhenFull = true;
        }
    }

    // One more message has to overwrite the oldest entry.
    emitArchivedMessage(BR38512_NEWEST);

    br38512_countAfterRollover = messageArchiveEntryCount();
    br38512_markerIsGone = true;
    for (int back = 1; back <= br38512_capacity; back++) {
        if (strcmp(messageArchiveEntry(back), BR38512_MARKER) == 0) {
            br38512_markerIsGone = false;
        }
    }
    br38512_filler(0, filler, sizeof(filler));
    br38512_firstFillerIsOldest = strcmp(messageArchiveEntry(br38512_capacity),
                                         filler) == 0;
    br38512_newestIsNewest = strcmp(messageArchiveEntry(1), BR38512_NEWEST) == 0;
}

static boolean br38512_archive_rolls_over_at_capacity(void) {
    TEST_ASSERT_EQ(br38512_countAtFull, br38512_capacity,
                   "filling every slot should fill the archive to its capacity");
    TEST_ASSERT(br38512_markerIsOldest,
                "before the archive is full, the first message should be its oldest entry");
    TEST_ASSERT(br38512_lastFillerIsNewest,
                "the last filler written should be the newest entry");
    TEST_ASSERT(br38512_markerSurvivesWhenFull,
                "the oldest message should still be archived while the archive is full");

    TEST_ASSERT_EQ(br38512_countAfterRollover, br38512_capacity,
                   "one more message should not grow the archive past its capacity");
    TEST_ASSERT(br38512_markerIsGone,
                "one more message should overwrite the oldest archived message");
    TEST_ASSERT(br38512_firstFillerIsOldest,
                "after the overwrite, the first filler should be the oldest entry");
    TEST_ASSERT(br38512_newestIsNewest,
                "the message just emitted should be the newest entry");
    return !testFailed;
}

// ============================================================
// BR37725 — the ambient light is neutral grey near the surface
// and shifts gradually toward a darker blue-grey as you descend,
// reaching its deepest shade at the depth of the amulet.
//
// The miner's light is the ambient light, and the game exposes it
// as RGB components.  At depth 1 the red and green channels are
// equal and blue is barely above them: neutral grey.  One floor
// down, red has fallen and blue has fallen less, so the blue-grey
// tint has grown.  On the amulet floor red is lower still and the
// tint is at its strongest, and one floor deeper changes nothing,
// so the deepest shade was reached at the amulet and not before.
// A fixed colour would leave the depth-2 tint equal to the surface
// tint; a step change at a fixed depth would leave the amulet tint
// equal to the depth-2 tint or would still change below the amulet.
// ============================================================

static void br37725_ambient_light(int *red, int *green, int *blue) {
    playerLightColorComponents(red, green, blue);
}

static boolean br37725_ambient_light_darkens_to_blue_grey_with_depth(void) {
    int surfaceRed, surfaceGreen, surfaceBlue;
    int shallowRed, shallowGreen, shallowBlue;
    int amuletRed, amuletGreen, amuletBlue;
    int deeperRed, deeperGreen, deeperBlue;

    TEST_ASSERT_EQ(currentDepth(), 1,
                   "control: the test should start on the surface floor");
    TEST_ASSERT(strcmp(playerLightColorName(), "miners light") == 0,
                "control: the ambient light should be the ordinary miner's light");

    br37725_ambient_light(&surfaceRed, &surfaceGreen, &surfaceBlue);
    TEST_ASSERT_EQ(surfaceRed, surfaceGreen,
                   "near the surface the ambient light should be neutral (red equals green)");
    TEST_ASSERT(surfaceBlue - surfaceRed <= 2,
                "near the surface the ambient light should not yet be noticeably blue-grey");

    descendToNextLevel();
    TEST_ASSERT_EQ(currentDepth(), 2, "control: one descent should reach the second floor");
    br37725_ambient_light(&shallowRed, &shallowGreen, &shallowBlue);
    TEST_ASSERT_EQ(shallowRed, shallowGreen,
                   "the ambient light should stay neutral in its red and green channels");
    TEST_ASSERT(shallowRed < surfaceRed,
                "the ambient light should darken after descending one floor");
    TEST_ASSERT(shallowBlue - shallowRed > surfaceBlue - surfaceRed,
                "the ambient light should grow more blue-grey after descending one floor");

    descendWithFailedAmuletAltar();
    TEST_ASSERT(currentDepth() > 2,
                "control: the amulet should lie below the second floor");
    TEST_ASSERT(strcmp(playerLightColorName(), "miners light") == 0,
                "control: reaching the amulet floor should not change the ambient light");
    br37725_ambient_light(&amuletRed, &amuletGreen, &amuletBlue);
    TEST_ASSERT_EQ(amuletRed, amuletGreen,
                   "at the amulet depth the ambient light should have equal red and green channels");
    TEST_ASSERT(amuletRed < shallowRed,
                "the ambient light should keep darkening all the way to the amulet");
    TEST_ASSERT(amuletBlue - amuletRed > shallowBlue - shallowRed,
                "the blue-grey tint should be stronger at the amulet than one floor down");

    descendToNextLevel();
    br37725_ambient_light(&deeperRed, &deeperGreen, &deeperBlue);
    TEST_ASSERT(deeperRed == amuletRed
                    && deeperGreen == amuletGreen
                    && deeperBlue == amuletBlue,
                "the ambient light should reach its deepest shade at the amulet and change no further");
    return !testFailed;
}

// ============================================================
// BR24247 — a light flash is drawn on top of the lighting that
// existed before the flash, and that earlier lighting is
// restored after every frame, so the flash leaves no permanent
// glow and the dungeon returns to its normal illumination when
// the animation ends.
//
// The engine's own flare animation is played for a generic flash
// centred on the player.  The peak brightness recorded while the
// frames were drawn must exceed the starting brightness (the
// flash really was laid over the existing light), and once the
// animation returns both the centre tile and its neighbour must
// carry exactly the brightness they had beforehand (the flash
// left nothing behind).
// ============================================================

static pos br24247_flashLoc;
static pos br24247_neighborLoc;
static int br24247_before;
static int br24247_neighborBefore;
static int br24247_peak;
static int br24247_after;
static int br24247_neighborAfter;

static void br24247_setup(void) {
    clearMonsters();
    setArenaRadius(6, FLOOR);
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    recomputeLighting();

    br24247_flashLoc = getPlayerPos();
    br24247_neighborLoc = posOffset(br24247_flashLoc, 1, 0);
    br24247_before = lightAt(br24247_flashLoc);
    br24247_neighborBefore = lightAt(br24247_neighborLoc);

    br24247_peak = flashLightAtAndReportPeak(br24247_flashLoc);

    br24247_after = lightAt(br24247_flashLoc);
    br24247_neighborAfter = lightAt(br24247_neighborLoc);
}

static boolean br24247_flash_restores_lighting(void) {
    TEST_ASSERT(br24247_before > 0,
                "control: the centre tile should be lit before the flash");
    TEST_ASSERT(br24247_peak > br24247_before,
                "the flash should brighten the tile while it is being drawn");
    TEST_ASSERT_EQ(br24247_after, br24247_before,
                   "the flash should leave no permanent glow on the centre tile");
    TEST_ASSERT_EQ(br24247_neighborAfter, br24247_neighborBefore,
                   "the flash should leave the surrounding lighting unchanged");
    return !testFailed;
}

// ============================================================
// BR40052 — when the game's master seed fits in its lower 32
// bits, every level's seed comes from the backward-compatible
// small range: a value from 0 to 9999 plus 10,000 times another
// value from 0 to 9999.  A level seed of zero is never kept;
// it is replaced by the level's depth plus one.
//
// The first test starts a game from a master seed that fits in
// 32 bits and checks every level seed the dungeon reserved: all
// are within the old scheme's maximum (99,999,999), none is
// zero, and at least one draws the upper 10,000-multiple term,
// so both rolls really contribute.  If the game drew a full
// 64-bit seed instead, the chance that all of the dungeon's
// level seeds stayed below 100 million is negligible.
//
// The second test uses master seed 25,179,883, whose backward-
// compatible draw produces a zero level seed on depth 3.  That
// zero must not survive: the level's seed must be its depth plus
// one, proving both the rejection and the depth-based
// replacement value.
// ============================================================

#define BR40052_MAX_SMALL_LEVEL_SEED (9999ULL + 10000ULL * 9999ULL)
#define BR40052_SEED_CAPACITY 64

static unsigned long long br40052_seeds[BR40052_SEED_CAPACITY];
static int br40052_seedCount;

static void br40052_start_with_master_seed(unsigned long long seed) {
    startGameWithSeed(seed);
    TEST_ASSERT(dungeonSeed() == seed,
                "control: the game should run from the requested master seed");
    br40052_seedCount = dungeonLevelSeeds(br40052_seeds, BR40052_SEED_CAPACITY);
}

static void br40052_range_setup(void) {
    // Fits in the lower 32 bits, so every level must use the backward-
    // compatible draw.
    br40052_start_with_master_seed(20221016ULL);
}

static boolean br40052_level_seeds_stay_in_small_range(void) {
    TEST_ASSERT(br40052_seedCount >= 2,
                "control: the dungeon should reserve a seed for every level");
    boolean upperTermSeen = false;
    for (int i = 0; i < br40052_seedCount; i++) {
        TEST_ASSERT(br40052_seeds[i] != 0,
                    "no level may keep a zero seed");
        TEST_ASSERT(br40052_seeds[i] <= BR40052_MAX_SMALL_LEVEL_SEED,
                    "a level seed should stay within the backward-compatible 32-bit scheme");
        if (br40052_seeds[i] >= 10000) {
            upperTermSeen = true;
        }
    }
    TEST_ASSERT(upperTermSeen,
                "level seeds should include the 10,000-times roll, not just the 0-to-9999 roll");
    return !testFailed;
}

static void br40052_zero_setup(void) {
    // The backward-compatible draw yields a zero level seed at depth 3.
    br40052_start_with_master_seed(25179883ULL);
}

static boolean br40052_zero_level_seed_becomes_depth_plus_one(void) {
    TEST_ASSERT(br40052_seedCount >= 3,
                "control: the dungeon should have a third level to inspect");
    TEST_ASSERT_EQ(br40052_seeds[2], 3,
                   "a zero seed drawn for depth 3 should be replaced by depth plus one");
    for (int i = 0; i < br40052_seedCount; i++) {
        TEST_ASSERT(br40052_seeds[i] != 0,
                    "a zero level seed should never be kept");
    }
    return !testFailed;
}

// ============================================================
// BR37990 — a game started from a seed larger than 32 bits uses
// the whole seed, so two games whose seeds differ only in their
// upper 32 bits still produce different dungeon layouts.
//
// Two master seeds share their lower 32 bits and differ only
// above bit 32, so both take the 64-bit level-seed draw.  Each
// game's level seeds and the terrain of its first level are
// recorded.  If the game truncated the master seed to 32 bits,
// both games would replay the same draws: their level seeds and
// their first dungeon would be identical.  The test therefore
// requires the level seeds to differ somewhere and the first
// level's terrain to differ, both of which fail under a 32-bit
// truncation.
// ============================================================

#define BR37990_LOW_SEED_BITS 4242ULL
#define BR37990_SEED_CAPACITY 64

static unsigned long long br37990_seedsA[BR37990_SEED_CAPACITY];
static unsigned long long br37990_seedsB[BR37990_SEED_CAPACITY];
static int br37990_countA, br37990_countB;
static unsigned long br37990_layoutA, br37990_layoutB;

static void br37990_start(unsigned long long seed,
                          unsigned long long *seeds, int *count,
                          unsigned long *layout) {
    startGameWithSeed(seed);
    TEST_ASSERT(dungeonSeed() == seed,
                "control: the game should run from the requested master seed");
    *count = dungeonLevelSeeds(seeds, BR37990_SEED_CAPACITY);
    *layout = levelTerrainFingerprint();
}

static void br37990_setup(void) {
    unsigned long long seedA = (1ULL << 32) | BR37990_LOW_SEED_BITS;
    unsigned long long seedB = (2ULL << 32) | BR37990_LOW_SEED_BITS;

    TEST_ASSERT((seedA & 0xffffffffULL) == (seedB & 0xffffffffULL),
                "control: the two master seeds should differ only above bit 32");
    br37990_start(seedA, br37990_seedsA, &br37990_countA, &br37990_layoutA);
    br37990_start(seedB, br37990_seedsB, &br37990_countB, &br37990_layoutB);
}

static boolean br37990_upper_seed_bits_change_dungeon_layout(void) {
    TEST_ASSERT_EQ(br37990_countA, br37990_countB,
                   "both games should reserve the same number of level seeds");
    TEST_ASSERT(br37990_countA >= 2,
                "control: the dungeon should reserve seeds for several levels");

    boolean levelSeedDiffers = false;
    for (int i = 0; i < br37990_countA && i < br37990_countB; i++) {
        if (br37990_seedsA[i] != br37990_seedsB[i]) {
            levelSeedDiffers = true;
        }
    }
    TEST_ASSERT(levelSeedDiffers,
                "the upper 32 bits of the master seed should change the levels' seeds");
    TEST_ASSERT(br37990_layoutA != br37990_layoutB,
                "two seeds differing only in their upper 32 bits should build different dungeons");
    return !testFailed;
}

// ============================================================
// BR24594 — the down stairs are placed at a random position
// within the map's interior, and the placement is repeated
// until the down stairs are at least one third of the map's
// width away from the up stairs.
//
// The dungeon reserves a staircase square for every depth when
// the run starts, before any of those lower levels is built and
// its stairs are nudged onto legal wall tiles.  Sampling those
// reserved squares across several seeds and every depth from 3
// down isolates the placement draw itself.
//
// The first test checks that every reserved down staircase lies
// on an interior square, never on the border, and that the
// sampled squares appear in both halves of the map in each
// direction (a fixed or one-sided placement would fail).  The
// second test checks the separation constraint: removing it
// would leave roughly half the sampled placements too close to
// the up stairs.
// ============================================================

#define BR24594_SAMPLE_CAPACITY 256
#define BR24594_SEED_COUNT 3

static const unsigned long long br24594_seeds[BR24594_SEED_COUNT] = {
    20221016ULL, 7ULL, 424242ULL
};

static pos br24594_up[BR24594_SAMPLE_CAPACITY];
static pos br24594_down[BR24594_SAMPLE_CAPACITY];
static int br24594_separations[BR24594_SAMPLE_CAPACITY];
static int br24594_sampleCount;

static void br24594_collect_samples(void) {
    br24594_sampleCount = 0;
    for (int s = 0; s < BR24594_SEED_COUNT; s++) {
        startGameWithSeed(br24594_seeds[s]);
        TEST_ASSERT(dungeonSeed() == br24594_seeds[s],
                    "control: the game should run from the requested master seed");

        for (int depth = 3;
             depth <= dungeonDeepestLevel() && br24594_sampleCount < BR24594_SAMPLE_CAPACITY;
             depth++) {
            pos upstairs, downstairs;
            if (!dungeonLevelStairLocations(depth, &upstairs, &downstairs)) {
                continue;
            }
            int separation = dungeonLevelStairSeparation(depth);

            br24594_up[br24594_sampleCount] = upstairs;
            br24594_down[br24594_sampleCount] = downstairs;
            br24594_separations[br24594_sampleCount] = separation;
            br24594_sampleCount++;
        }
    }
    TEST_ASSERT(br24594_sampleCount >= 2 * dungeonDeepestLevel(),
                "control: the dungeon should reserve staircases on every sampled depth");
}

static void br24594_setup(void) {
    br24594_collect_samples();
}

static boolean br24594_down_stairs_are_interior_and_varied(void) {
    boolean leftHalf = false, rightHalf = false;
    boolean topHalf = false, bottomHalf = false;
    boolean positionsDiffer = false;

    for (int i = 0; i < br24594_sampleCount; i++) {
        pos down = br24594_down[i];
        TEST_ASSERT(down.x >= 1 && down.x <= levelWidth() - 2,
                    "the down stairs should never be placed on the map's left or right border");
        TEST_ASSERT(down.y >= 1 && down.y <= levelHeight() - 2,
                    "the down stairs should never be placed on the map's top or bottom border");

        if (down.x <= levelWidth() / 2) {
            leftHalf = true;
        } else {
            rightHalf = true;
        }
        if (down.y <= levelHeight() / 2) {
            topHalf = true;
        } else {
            bottomHalf = true;
        }
        if (down.x != br24594_down[0].x || down.y != br24594_down[0].y) {
            positionsDiffer = true;
        }
    }

    TEST_ASSERT(leftHalf && rightHalf,
                "down stairs should be drawn from the whole width of the interior, not one side");
    TEST_ASSERT(topHalf && bottomHalf,
                "down stairs should be drawn from the whole height of the interior, not one side");
    TEST_ASSERT(positionsDiffer,
                "down stairs should not always occupy the same square");
    return !testFailed;
}

static boolean br24594_down_stairs_keep_a_third_width_from_up(void) {
    TEST_ASSERT(br24594_sampleCount > 0,
                "control: the dungeon should have reserved staircases to inspect");

    for (int i = 0; i < br24594_sampleCount; i++) {
        TEST_ASSERT(br24594_separations[i] >= levelWidth() / 3,
                    "the down stairs should be at least a third of the map's width from the up stairs");
    }
    return !testFailed;
}

// ============================================================
// BR35125 — while the title screen waits for your choice, the
// flames are recomputed and the menu redrawn on every pass of
// the input loop only when the game's window is active; if the
// window is inactive, the display is left as it was.
//
// The framework drives the real title menu and reports the
// window active or inactive on each input-loop pass.  The first
// test requires a different frame on every active pass, so a
// menu that stopped animating fails it.  The second test lets
// one active pass paint the title screen, then reports the
// window inactive for several passes and requires the display
// to remain exactly that frame; a menu that kept redrawing
// while inactive fails it.  The second test also requires the
// title screen to have been drawn at all, so a menu that never
// drew anything cannot pass by standing still.
// ============================================================

#define BR35125_PASSES 3

static boolean br35125_activeRedrewEveryPass;

static void br35125_active_setup(void) {
    br35125_activeRedrewEveryPass = titleScreenRedrawsOnEveryActivePass(BR35125_PASSES);
}

static boolean br35125_redraws_on_every_active_pass(void) {
    TEST_ASSERT(screenTextContains("New Game"),
                "control: the title screen should have been drawn");
    TEST_ASSERT(br35125_activeRedrewEveryPass,
                "an active title screen should redraw the flames on every input-loop pass");
    return !testFailed;
}

static boolean br35125_inactiveHeldFrame;

static void br35125_inactive_setup(void) {
    br35125_inactiveHeldFrame = titleScreenLeavesDisplayAloneWhileApplicationInactive(BR35125_PASSES);
}

static boolean br35125_leaves_display_alone_while_inactive(void) {
    TEST_ASSERT(br35125_inactiveHeldFrame,
                "an inactive title screen should not recompute or redraw its flames");
    return !testFailed;
}

// ============================================================
// BR36302 — examining an identified weapon of multiplicity
// also states how many spectral images it would produce if
// enchanted further, how often they would appear, and how many
// turns they would last.
// ============================================================

static const char *br36302_description;

static void br36302_setup(void) {
    // Matching the player's strength to the sword's requirement makes the
    // net enchantment exactly the item's +5, and one more scroll of
    // enchanting (power 1, at the 1.25 increment for a handled weapon)
    // previews +6.25: one spectral image now, two after the scroll, and
    // three turns either way.
    item *theItem = giveAndEquipRunicWeapon(SWORD, W_MULTIPLICITY, 5, true);
    br36302_description = itemExamineText(theItem);
}

static boolean br36302_examine_previews_stronger_images(void) {
    int previewChance = 0;

    TEST_ASSERT(br36302_description != NULL,
                "the multiplicity weapon should have an examine description");
    TEST_ASSERT(strstr(br36302_description,
                       "1 spectral sword will spring into being") != NULL,
                "the examine text should state the current spectral image count");
    TEST_ASSERT(strstr(br36302_description, "will dissipate 3 turns later") != NULL,
                "the examine text should state how long the current images last");
    TEST_ASSERT(strstr(br36302_description,
                       "(If the sword is enchanted, 2 images will appear") != NULL,
                "the examine text should preview the image count after further enchanting");
    TEST_ASSERT(strstr(br36302_description,
                       "of the time, and will last 3 turns.)") != NULL,
                "the examine text should preview the appearance chance and duration after further enchanting");

    const char *preview = strstr(br36302_description, "images will appear ");
    TEST_ASSERT(preview != NULL
                && sscanf(preview, "images will appear %d%% of the time", &previewChance) == 1,
                "the preview should state a numeric chance for the images to appear");
    TEST_ASSERT(previewChance > 0 && previewChance <= 100,
                "the previewed chance should be a real percentage");
    return !testFailed;
}

// ============================================================
// BR18004 — a phylactery summons a lich and hides inside it; killing
// that lich only reveals the phylactery, which can summon a fresh
// lich and hide again, so the lich keeps returning until the
// phylactery itself is destroyed.
//
// The phylactery is placed alone in an open arena.  Forcing the
// engine's own summon routine makes it produce its single lich and
// disappear from the level (it is now carried inside that lich).
// Killing the lich through the ordinary death path returns the
// phylactery to the floor, alive and on the lich's old tile.  The
// same cycle runs a second time, proving the phylactery can repeat
// it; destroying the phylactery then ends the cycle for good.
// ============================================================

static creature *br18004_phylactery;

static void br18004_setup(void) {
    setArenaRadius(8, FLOOR);
    br18004_phylactery = placeHostileMonster(MK_PHYLACTERY, posOffset(getPlayerPos(), 3, 0));
}

static boolean br18004_phylactery_cycle_ends_only_when_destroyed(void) {
    creature *phylactery = br18004_phylactery;
    TEST_ASSERT(phylactery != NULL && monsterIsAlive(phylactery),
                "setup: the phylactery should be alive in the arena");
    TEST_ASSERT(monsterHasAbility(phylactery, MA_CAST_SUMMON),
                "control: the phylactery's catalog entry carries the summon ability");
    if (phylactery == NULL) {
        return false;
    }

    // First cycle: summon a lich, then destroy that lich.
    TEST_ASSERT(forceMonsterSummon(phylactery),
                "the phylactery should summon a lich");
    creature *firstLich = findMonster(MK_LICH);
    TEST_ASSERT(firstLich != NULL && monsterIsAlive(firstLich),
                "the summoned lich should stand alive on the level");
    if (firstLich == NULL) {
        return false;
    }
    TEST_ASSERT_EQ(countMonstersOfType(MK_PHYLACTERY), 0,
                   "the phylactery should leave the level when it hides inside its lich");
    TEST_ASSERT(monsterCarriesMonsterOfType(firstLich, MK_PHYLACTERY),
                "the summoned lich should be carrying the phylactery inside it");
    pos firstLichPos = monsterPosition(firstLich);

    makeMonsterDying(firstLich);
    TEST_ASSERT(!monsterIsAlive(firstLich),
                "the lich should die when killed");
    clearDeadMonsters();
    phylactery = findMonster(MK_PHYLACTERY);
    TEST_ASSERT(phylactery != NULL && monsterIsAlive(phylactery),
                "killing the lich should reveal the phylactery, not destroy it");
    if (phylactery == NULL) {
        return false;
    }
    TEST_ASSERT_POS_EQ(monsterPosition(phylactery), firstLichPos,
                       "the revealed phylactery should appear where the lich fell");

    // Second cycle: the revealed phylactery can summon and hide again.
    TEST_ASSERT(forceMonsterSummon(phylactery),
                "the revealed phylactery should be able to summon again");
    creature *secondLich = findMonster(MK_LICH);
    TEST_ASSERT(secondLich != NULL && monsterIsAlive(secondLich),
                "the phylactery should produce another living lich");
    if (secondLich == NULL) {
        return false;
    }
    TEST_ASSERT_EQ(countMonstersOfType(MK_PHYLACTERY), 0,
                   "the phylactery should hide inside the replacement lich too");
    TEST_ASSERT(monsterCarriesMonsterOfType(secondLich, MK_PHYLACTERY),
                "the replacement lich should carry the phylactery inside it");
    pos secondLichPos = monsterPosition(secondLich);

    makeMonsterDying(secondLich);
    clearDeadMonsters();
    phylactery = findMonster(MK_PHYLACTERY);
    TEST_ASSERT(phylactery != NULL && monsterIsAlive(phylactery),
                "killing the replacement lich should reveal the phylactery again");
    if (phylactery == NULL) {
        return false;
    }
    TEST_ASSERT_POS_EQ(monsterPosition(phylactery), secondLichPos,
                       "the phylactery should reappear where the replacement lich fell");

    // Destroying the phylactery itself stops the cycle.
    makeMonsterDying(phylactery);
    clearDeadMonsters();
    TEST_ASSERT_EQ(countMonstersOfType(MK_LICH), 0,
                   "destroying the phylactery should leave no lich behind");
    TEST_ASSERT_EQ(countMonstersOfType(MK_PHYLACTERY), 0,
                   "the destroyed phylactery should not return");
    return !testFailed;
}

// ============================================================
// BR35620 — when gas is stranded on a square that cannot hold it,
// its volume is split as evenly as possible among the neighbouring
// squares that can hold gas, each of which takes on that same kind
// of gas, and any volume that does not divide evenly is lost.
//
// A granite square (which blocks gas) holds 25 units of explosive
// gas, surrounded by an eight-tile ring of floor sealed inside
// granite.  Integer division over the eight neighbours gives each
// three units, copying the gas kind with them, and discards the
// remaining one.  The floor ring is closed, so the second gas pass
// in the same update leaves those equal shares unchanged; methane
// does not dissipate, so the measured volumes stay exact.
// ============================================================

static pos br35620_source;
static pos br35620_neighbours[8];
static int br35620_sourceVolume;
static short br35620_sourceType;

static void br35620_setup(void) {
    static const int dx[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
    static const int dy[8] = {-1, 0, 1, 1, 1, 0, -1, -1};

    setArenaRadius(10, FLOOR);
    br35620_source = posOffset(getPlayerPos(), 6, 0);

    // The stranded square itself is granite; the eight neighbours are
    // gas-permeable floor.  A granite border keeps the gas from escaping
    // beyond that ring.
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            setTerrain(posOffset(br35620_source, x, y),
                       (x == 0 && y == 0) ? GRANITE : FLOOR);
        }
    }
    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            if (x == -2 || x == 2 || y == -2 || y == 2) {
                setTerrain(posOffset(br35620_source, x, y), GRANITE);
            }
        }
    }
    for (int i = 0; i < 8; i++) {
        br35620_neighbours[i] = posOffset(br35620_source, dx[i], dy[i]);
    }

    setGasAt(br35620_source, METHANE_GAS, 25);
}

static boolean br35620_step(void) {
    br35620_sourceVolume = gasVolumeAt(br35620_source);
    br35620_sourceType = gasTypeAt(br35620_source);
    runEnvironmentUpdate();
    return false;
}

static boolean br35620_stranded_gas_splits_evenly_and_loses_remainder(void) {
    TEST_ASSERT_EQ(br35620_sourceVolume, 25,
                   "control: the stranded square should start with 25 units of gas");
    TEST_ASSERT_EQ(br35620_sourceType, METHANE_GAS,
                   "control: the stranded gas should be the explosive kind");

    TEST_ASSERT_EQ(gasVolumeAt(br35620_source), 0,
                   "the square that cannot hold gas should be emptied");
    TEST_ASSERT_EQ(gasTypeAt(br35620_source), NOTHING,
                   "the emptied square should no longer hold a gas kind");

    int total = 0;
    for (int i = 0; i < 8; i++) {
        TEST_ASSERT_EQ(gasVolumeAt(br35620_neighbours[i]), 3,
                       "each of the eight neighbours should receive 25 / 8, rounded down");
        TEST_ASSERT_EQ(gasTypeAt(br35620_neighbours[i]), METHANE_GAS,
                       "each neighbour should take on the stranded gas's kind");
        total += gasVolumeAt(br35620_neighbours[i]);
    }
    TEST_ASSERT_EQ(total, 24,
                   "the one unit that does not divide evenly should be lost, not redistributed");
    return !testFailed;
}

// ============================================================
// BR35726 — pink jellies are drawn pink, underworms and pools of worm
// entrails tan, sentinels dark blue, goblin mystics blue, ifrits purple,
// and phoenixes and their eggs red.
//
// Each creature is placed on open floor and its drawn colour is read by
// role name; a pool of worm entrails is painted on a floor tile and read the
// same way.  The rule's plain colour word is the exact expected name, so a
// catalog row that changes colour fails the test even if another monster
// keeps the old colour.  An ordinary goblin is the control for a monster
// that has none of these role colours.
// ============================================================

static creature *br35726_jelly, *br35726_underworm, *br35726_sentinel,
                *br35726_mystic, *br35726_ifrit, *br35726_phoenix,
                *br35726_egg, *br35726_goblin;
static const char *br35726_jellyColor, *br35726_underwormColor,
                  *br35726_sentinelColor, *br35726_mysticColor,
                  *br35726_ifritColor, *br35726_phoenixColor,
                  *br35726_eggColor, *br35726_entrailsColor,
                  *br35726_goblinColor;

static void br35726_setup(void) {
    clearArena(); // 5x5 of open floor around the player
    pos origin = getPlayerPos();

    br35726_jelly = placeHostileMonster(MK_PINK_JELLY, posOffset(origin, -2, -2));
    br35726_underworm = placeHostileMonster(MK_UNDERWORM, posOffset(origin, -1, -2));
    br35726_sentinel = placeHostileMonster(MK_SENTINEL, posOffset(origin, 0, -2));
    br35726_mystic = placeHostileMonster(MK_GOBLIN_MYSTIC, posOffset(origin, 1, -2));
    br35726_ifrit = placeHostileMonster(MK_IFRIT, posOffset(origin, 2, -2));
    br35726_phoenix = placeHostileMonster(MK_PHOENIX, posOffset(origin, -2, -1));
    br35726_egg = placeHostileMonster(MK_PHOENIX_EGG, posOffset(origin, -1, -1));
    br35726_goblin = placeHostileMonster(MK_GOBLIN, posOffset(origin, 2, -1));

    pos entrails = posOffset(origin, -2, 2);
    setTerrain(entrails, WORM_BLOOD);

    br35726_jellyColor = monsterDrawColorName(br35726_jelly);
    br35726_underwormColor = monsterDrawColorName(br35726_underworm);
    br35726_sentinelColor = monsterDrawColorName(br35726_sentinel);
    br35726_mysticColor = monsterDrawColorName(br35726_mystic);
    br35726_ifritColor = monsterDrawColorName(br35726_ifrit);
    br35726_phoenixColor = monsterDrawColorName(br35726_phoenix);
    br35726_eggColor = monsterDrawColorName(br35726_egg);
    br35726_goblinColor = monsterDrawColorName(br35726_goblin);
    br35726_entrailsColor = terrainDrawColorName(entrails);
}

static boolean br35726_monsters_and_entrails_have_role_colors(void) {
    TEST_ASSERT(br35726_jelly && br35726_underworm && br35726_sentinel
                && br35726_mystic && br35726_ifrit && br35726_phoenix
                && br35726_egg && br35726_goblin,
                "precondition: every creature should be placed");

    TEST_ASSERT(strcmp(br35726_jellyColor, "pink") == 0,
                "a pink jelly should be drawn pink");
    TEST_ASSERT(strcmp(br35726_underwormColor, "tan") == 0,
                "an underworm should be drawn tan");
    TEST_ASSERT(strcmp(br35726_entrailsColor, "tan") == 0,
                "a pool of worm entrails should be drawn tan");
    TEST_ASSERT(strcmp(br35726_sentinelColor, "dark blue") == 0,
                "a sentinel should be drawn dark blue");
    TEST_ASSERT(strcmp(br35726_mysticColor, "blue") == 0,
                "a goblin mystic should be drawn blue");
    TEST_ASSERT(strcmp(br35726_ifritColor, "purple") == 0,
                "an ifrit should be drawn purple");
    TEST_ASSERT(strcmp(br35726_phoenixColor, "red") == 0,
                "a phoenix should be drawn red");
    TEST_ASSERT(strcmp(br35726_eggColor, "red") == 0,
                "a phoenix egg should be drawn the same red as the phoenix");
    TEST_ASSERT(strcmp(br35726_goblinColor, "other") == 0,
                "control: an ordinary goblin should not be drawn in any of those role colours");
    return !testFailed;
}

// ============================================================
// BR11201 — pink jellies, sentinels, ifrits, phoenixes, and phoenix eggs
// flicker (their shade shifts slightly with every screen refresh), while
// underworms, goblin mystics, and pools of worm entrails keep a completely
// steady colour.
//
// Every creature is placed on open floor and its drawn-colour flicker flag is
// read through the framework; a pool of worm entrails is painted and its
// terrain-colour flag read the same way.  The steady cases are asserted
// negatively, so a catalog row that starts flickering fails the test even if
// the other colours stay correct.
// ============================================================

static creature *br11201_jelly, *br11201_sentinel, *br11201_ifrit,
                *br11201_phoenix, *br11201_egg, *br11201_underworm,
                *br11201_mystic;
static boolean br11201_jellyFlickers, br11201_sentinelFlickers,
               br11201_ifritFlickers, br11201_phoenixFlickers,
               br11201_eggFlickers, br11201_underwormFlickers,
               br11201_mysticFlickers, br11201_entrailsFlicker;

static void br11201_setup(void) {
    clearArena();
    pos origin = getPlayerPos();

    br11201_jelly = placeHostileMonster(MK_PINK_JELLY, posOffset(origin, -2, -2));
    br11201_sentinel = placeHostileMonster(MK_SENTINEL, posOffset(origin, -1, -2));
    br11201_ifrit = placeHostileMonster(MK_IFRIT, posOffset(origin, 0, -2));
    br11201_phoenix = placeHostileMonster(MK_PHOENIX, posOffset(origin, 1, -2));
    br11201_egg = placeHostileMonster(MK_PHOENIX_EGG, posOffset(origin, 2, -2));
    br11201_underworm = placeHostileMonster(MK_UNDERWORM, posOffset(origin, -2, 2));
    br11201_mystic = placeHostileMonster(MK_GOBLIN_MYSTIC, posOffset(origin, -1, 2));

    pos entrails = posOffset(origin, 2, 2);
    setTerrain(entrails, WORM_BLOOD);

    br11201_jellyFlickers = monsterDrawColorFlickers(br11201_jelly);
    br11201_sentinelFlickers = monsterDrawColorFlickers(br11201_sentinel);
    br11201_ifritFlickers = monsterDrawColorFlickers(br11201_ifrit);
    br11201_phoenixFlickers = monsterDrawColorFlickers(br11201_phoenix);
    br11201_eggFlickers = monsterDrawColorFlickers(br11201_egg);
    br11201_underwormFlickers = monsterDrawColorFlickers(br11201_underworm);
    br11201_mysticFlickers = monsterDrawColorFlickers(br11201_mystic);
    br11201_entrailsFlicker = terrainDrawColorFlickers(entrails);
}

static boolean br11201_only_the_listed_monsters_and_entrails_flicker(void) {
    TEST_ASSERT(br11201_jelly && br11201_sentinel && br11201_ifrit
                && br11201_phoenix && br11201_egg && br11201_underworm
                && br11201_mystic,
                "precondition: every creature should be placed");

    TEST_ASSERT(br11201_jellyFlickers, "a pink jelly's colour should flicker");
    TEST_ASSERT(br11201_sentinelFlickers, "a sentinel's colour should flicker");
    TEST_ASSERT(br11201_ifritFlickers, "an ifrit's colour should flicker");
    TEST_ASSERT(br11201_phoenixFlickers, "a phoenix's colour should flicker");
    TEST_ASSERT(br11201_eggFlickers, "a phoenix egg's colour should flicker");

    TEST_ASSERT(!br11201_underwormFlickers,
                "an underworm's colour should stay steady");
    TEST_ASSERT(!br11201_mysticFlickers,
                "a goblin mystic's colour should stay steady");
    TEST_ASSERT(!br11201_entrailsFlicker,
                "a pool of worm entrails should keep a steady colour");
    return !testFailed;
}

// ============================================================
// BR23709 — the inventory list and its item detail box each keep
// invisible previous and next controls so the up and down arrow
// keys page through the items.
//
// The controls have no label, so the observable behavior is the
// paging itself.  Three identified potions give a first, middle,
// and last item.  Down in the list opens the first item; down in
// the detail box advances to the middle; up in the detail box
// wraps from the first to the last; and up in the list opens the
// last item directly.  Asserting the item named on each page
// makes a dropped or reversed control fail.
// ============================================================

static char br23709_first[4096];
static char br23709_second[4096];
static char br23709_last[4096];
static char br23709_wrapped[4096];

static void br23709_setup(void) {
    clearArena();
    // Leave only the three potions, so their order is deterministic.
    removePackFood();
    removePackItems(WEAPON, DAGGER);
    removePackItems(WEAPON, DART);
    removePackItems(ARMOR, LEATHER_ARMOR);
    givePlayerItem(POTION, POTION_LIFE, 0);
    givePlayerItem(POTION, POTION_STRENGTH, 0);
    givePlayerItem(POTION, POTION_TELEPATHY, 0);
}

static boolean br23709_arrows_page_item_details(void) {
    snprintf(br23709_first, sizeof(br23709_first), "%s",
             inventoryDetailAfterKeys(DOWN_ARROW, DOWN_ARROW, 0));
    snprintf(br23709_second, sizeof(br23709_second), "%s",
             inventoryDetailAfterKeys(DOWN_ARROW, DOWN_ARROW, 1));
    snprintf(br23709_last, sizeof(br23709_last), "%s",
             inventoryDetailAfterKeys(UP_ARROW, UP_ARROW, 0));
    snprintf(br23709_wrapped, sizeof(br23709_wrapped), "%s",
             inventoryDetailAfterKeys(DOWN_ARROW, UP_ARROW, 1));

    TEST_ASSERT(strstr(br23709_first, "potion of life") != NULL,
                "the down arrow in the inventory should open the first item's details");
    TEST_ASSERT(strstr(br23709_second, "potion of strength") != NULL,
                "the down arrow in the detail box should advance to the next item");
    TEST_ASSERT(strstr(br23709_last, "potion of telepathy") != NULL,
                "the up arrow in the inventory should open the last item's details");
    TEST_ASSERT(strstr(br23709_wrapped, "potion of telepathy") != NULL,
                "the up arrow in the detail box should wrap from the first item to the last");
    TEST_ASSERT(strcmp(br23709_first, br23709_second) != 0,
                "paging down should change which item's details are shown");
    TEST_ASSERT(strcmp(br23709_first, br23709_wrapped) != 0,
                "paging up from the first item should change which details are shown");
    return !testFailed;
}

// ============================================================
// BR34530 — a staff of blinking's targeting line reaches two
// spaces plus twice its net enchantment once the staff is
// identified or its maximum charges are known, and is unlimited
// until then.
//
// The test stands in an open, revealed corridor and aims eleven
// cells east with the staff's own targeting prompt.  The frame
// captured while the line is drawn shows which cells it lit: a
// cell inside the cap must be lit and a cell beyond it must not.
// The unlimited control lights a cell the capped staff leaves
// dark, so a missing or over-long line fails either arm.
// ============================================================

static const int br34530_targetDistance = 11;

static void br34530_arena_setup(void) {
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(12, FLOOR);
    clearFloorItems();
    for (int i = 1; i <= br34530_targetDistance; i++) {
        revealCell(posOffset(getPlayerPos(), i, 0));
    }
}

static void br34530_identified_one_setup(void) {
    br34530_arena_setup();
    givePlayerItem(STAFF, STAFF_BLINKING, 1);
}

static void br34530_identified_three_setup(void) {
    br34530_arena_setup();
    givePlayerItem(STAFF, STAFF_BLINKING, 3);
}

static void br34530_unidentified_setup(void) {
    br34530_arena_setup();
    item *staff = givePlayerUnidentifiedItem(STAFF, STAFF_BLINKING, 1);
    TEST_ASSERT(staff != NULL, "setup: the unidentified staff should be created");
    if (staff != NULL) {
        TEST_ASSERT(!itemIsIdentified(staff),
                    "precondition: the staff's kind should still be unknown");
    }
}

static void br34530_max_charges_known_setup(void) {
    br34530_arena_setup();
    item *staff = givePlayerUnidentifiedItem(STAFF, STAFF_BLINKING, 1);
    TEST_ASSERT(staff != NULL, "setup: the staff should be created");
    if (staff != NULL) {
        TEST_ASSERT(!itemIsIdentified(staff),
                    "precondition: only the maximum charges should be known, not the staff's identity");
        markItemMaxChargesKnown(staff);
    }
}

static boolean br34530_aim_step(void) {
    for (int i = 0; i < br34530_targetDistance; i++) {
        queueKeystroke(RIGHT_KEY, false, false);
    }
    applyPackItem(STAFF, STAFF_BLINKING);
    // The last frame the targeting prompt captured has the line drawn.
    rememberScreenColors();
    runQueuedInputThroughMainLoop();
    return false;
}

static boolean br34530_identified_one_has_four_space_line(void) {
    TEST_ASSERT(screenCellColorsChanged(posOffset(getPlayerPos(), 2, 0)),
                "the identified staff's line should light the cells inside its range");
    TEST_ASSERT(screenCellColorsChanged(posOffset(getPlayerPos(), 4, 0)),
                "an enchantment of one should allow two plus twice one, four spaces");
    TEST_ASSERT(!screenCellColorsChanged(posOffset(getPlayerPos(), 5, 0)),
                "the line should stop at four spaces for an enchantment of one");
    return !testFailed;
}

static boolean br34530_identified_three_has_eight_space_line(void) {
    TEST_ASSERT(screenCellColorsChanged(posOffset(getPlayerPos(), 7, 0)),
                "the identified staff's line should light the cells inside its range");
    TEST_ASSERT(screenCellColorsChanged(posOffset(getPlayerPos(), 8, 0)),
                "an enchantment of three should allow two plus twice three, eight spaces");
    TEST_ASSERT(!screenCellColorsChanged(posOffset(getPlayerPos(), 9, 0)),
                "the line should stop at eight spaces for an enchantment of three");
    return !testFailed;
}

static boolean br34530_unidentified_has_unlimited_line(void) {
    TEST_ASSERT(screenCellColorsChanged(posOffset(getPlayerPos(), 2, 0)),
                "the unidentified staff's line should be drawn");
    TEST_ASSERT(screenCellColorsChanged(posOffset(getPlayerPos(), 10, 0)),
                "an unidentified staff should not limit the line to its enchantment-based range");
    return !testFailed;
}

static boolean br34530_known_max_charges_has_capped_line(void) {
    TEST_ASSERT(screenCellColorsChanged(posOffset(getPlayerPos(), 2, 0)),
                "the line should light the cells inside the known range");
    TEST_ASSERT(!screenCellColorsChanged(posOffset(getPlayerPos(), 5, 0)),
                "learning the staff's maximum charges should restore the enchantment-based cap");
    return !testFailed;
}

// ============================================================
// BR41110 — dismissing the help screen puts the previous display
// back exactly as it was
//
// The framework opens the help screen over the dungeon, checks that its
// command list was actually drawn in the frame the engine waits on, then
// dismisses it and compares every screen cell with the frame from before.
// The "help was shown" assertion keeps the restore check from passing when
// the help screen never appeared.
// ============================================================

static boolean br41110_helpWasShown;
static boolean br41110_displayRestored;

static void br41110_setup(void) {
    setArenaRadius(4, FLOOR);
    restOneTurn(); // settle the map and message display before the snapshot
}

static boolean br41110_step(void) {
    br41110_displayRestored = helpScreenRestoresPreviousDisplay();
    br41110_helpWasShown = helpScreenWasShown();
    return false;
}

static boolean br41110_verify(void) {
    TEST_ASSERT(br41110_helpWasShown,
                "the help screen should draw its command list while it waits");
    TEST_ASSERT(br41110_displayRestored,
                "dismissing the help screen should put the previous display back exactly");
    return !testFailed;
}

// ============================================================
// BR40113 — when the help screen opens, its command list is drawn over the
// current game screen, one command per line starting at the top-left of the
// display window.
//
// The framework opens the help screen over the dungeon and holds the frame it
// waits on.  That frame must still show the game underneath (the depth line)
// as well as the command list.  The first command block must begin near the
// top row, each of its commands must occupy its own consecutive row, and the
// unindented move/attack line must start one column inside the display
// window's left edge.
// ============================================================

static boolean br40113_helpWasShown, br40113_overGameScreen,
               br40113_startsAtTopLeft, br40113_oneCommandPerLine;

static void br40113_setup(void) {
    setArenaRadius(4, FLOOR);
    restOneTurn(); // give the map and its depth line a settled frame
}

static boolean br40113_step(void) {
    helpScreenRestoresPreviousDisplay();
    br40113_helpWasShown = helpScreenWasShown();

    int headerRow = screenTextRowContaining("-- Commands --");
    int cursorRow = screenTextRowContaining("mouse  move cursor");
    int travelRow = screenTextRowContaining("click  travel");
    int advanceRow = screenTextRowContaining("control-click  advance one space");
    int returnRow = screenTextRowContaining("<return>  enable keyboard cursor control");
    int cancelRow = screenTextRowContaining("<space/esc>  disable keyboard cursor control");
    int moveRow = screenTextRowContaining("hjklyubn, arrow keys, or numpad  move or attack");

    br40113_overGameScreen = screenTextContains("-- Commands --")
                             && screenTextContains("-- Depth:");
    br40113_startsAtTopLeft =
        headerRow >= displayWindowTopRow() && headerRow <= displayWindowTopRow() + 2
        && screenTextColumnContaining("hjklyubn, arrow keys, or numpad")
               == displayWindowLeftColumn() + 1;
    br40113_oneCommandPerLine =
        cursorRow == headerRow + 2
        && travelRow == cursorRow + 1
        && advanceRow == travelRow + 1
        && returnRow == advanceRow + 1
        && cancelRow == returnRow + 1
        && moveRow == cancelRow + 1;
    return false;
}

static boolean br40113_help_commands_overlay_from_top_left(void) {
    TEST_ASSERT(br40113_helpWasShown,
                "the help screen should draw its command list while it waits");
    TEST_ASSERT(br40113_overGameScreen,
                "the help command list should be drawn over the current game screen");
    TEST_ASSERT(br40113_startsAtTopLeft,
                "the command list should start at the top-left of the display window");
    TEST_ASSERT(br40113_oneCommandPerLine,
                "the first command block should list one command per line");
    return !testFailed;
}

// ============================================================
// BR20494 — while a recording is being played back, saving the
// recording without prompting writes nothing at all: no file is
// created or replaced and no message appears.
//
// The same automatic save outside playback is the control: it writes a
// recording file, proving the helper can observe a save that happens.  With
// the game in playback the save must write no file and leave the message log
// empty.  Both attempts use the framework's recording helpers, which hide the
// engine's file naming and playback state.
// ============================================================

static boolean br20494_savedOutsidePlayback;
static boolean br20494_savedDuringPlayback;

static void br20494_control_setup(void) {
    clearMessages();
    beginTestRecording();
    br20494_savedOutsidePlayback = saveRecordingWithoutPromptWritesFile();
}

static boolean br20494_control_verify(void) {
    TEST_ASSERT(br20494_savedOutsidePlayback,
                "control: outside playback the same save should write a recording file");
    return !testFailed;
}

static void br20494_playback_setup(void) {
    beginTestRecording();
    enterPlaybackMode();
    clearMessages();
    br20494_savedDuringPlayback = saveRecordingWithoutPromptWritesFile();
}

static boolean br20494_playback_verify(void) {
    TEST_ASSERT(isInPlaybackMode(),
                "precondition: the game should still be replaying a recording");
    TEST_ASSERT(!br20494_savedDuringPlayback,
                "saving a recording without a prompt during playback should create or replace no file");
    TEST_ASSERT(!anyMessageWasShown(),
                "saving a recording without a prompt during playback should show no message");
    return !testFailed;
}

// ============================================================
// BR93785 — a floor tile stays floor during a cellular automata
// generation when its live-neighbour count matches a survival
// condition
//
// Two runs differ only in the survival rule for the centre tile of a solid
// 3x3 block, whose eight neighbours are all live.  With the matching rule
// the centre stays floor; with the rule not matching that count it is not
// kept.  The second run is the control that keeps the first from passing
// when the generation fails to remove any floor tile.
// ============================================================

static boolean br93785_survivedWithRule;
static boolean br93785_survivedWithoutRule;

static void br93785_setup(void) {
    setArenaRadius(2, FLOOR);
    br93785_survivedWithRule = floorTileSurvivesGeneration(true);
    br93785_survivedWithoutRule = floorTileSurvivesGeneration(false);
}

static boolean br93785_verify(void) {
    TEST_ASSERT(br93785_survivedWithRule,
                "a floor tile with eight live neighbours should stay floor when the survival rule matches them");
    TEST_ASSERT(!br93785_survivedWithoutRule,
                "the same floor tile should not stay floor when the survival rule does not match its neighbours");
    return !testFailed;
}

// ============================================================
// BR32744 — a fleeing ally moves toward whichever neighbouring
// square is farthest by walking distance from the nearest
// hostile, not from you.
//
// The arena is open floor.  You stand west of the ally and a
// hostile rat stands east of it, so a step measured from you
// would carry the ally east toward the rat, while a step
// measured from the rat carries it west.  Dropping the ally to
// one hit point makes it flee.  The assertion therefore fails
// both if the ally does not flee and if it flees from the
// player instead of from the enemy.  The distance to the rat
// must also grow, proving the square chosen was strictly safer.
// ============================================================

static creature *br32744_ally;
static creature *br32744_enemy;
static pos br32744_allyStart;
static pos br32744_enemyLoc;
static pos br32744_sacredLoc;
static int br32744_startDistanceFromEnemy;

static void br32744_setup(void) {
    clearMonsters();
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(6, FLOOR);

    pos playerLoc = getPlayerPos();
    br32744_allyStart = posOffset(playerLoc, 3, 0);
    br32744_enemyLoc = posOffset(playerLoc, 6, 0);
    br32744_sacredLoc = posOffset(playerLoc, 2, 0);

    // The straight step away from the enemy would be the best one, so put
    // sacred ground there: the ally must take a diagonal step instead.
    setTerrain(br32744_sacredLoc, SACRED_GLYPH);

    br32744_ally = spawnMonster(MK_RAT, br32744_allyStart);
    makeMonsterAlly(br32744_ally);
    makeMonsterFollowPlayer(br32744_ally);
    setMonsterHealth(br32744_ally, 1);

    br32744_enemy = placeHostileMonster(MK_RAT, br32744_enemyLoc);

    setPlayerHealth(playerMaxHealth());
    br32744_startDistanceFromEnemy = pathingDistanceBetween(br32744_allyStart, br32744_enemyLoc, 0);
}

static boolean br32744_step(void) {
    monsterTakesTurn(br32744_ally);
    return false;
}

static boolean br32744_fleeing_ally_runs_from_the_enemy(void) {
    pos allyLoc = monsterPosition(br32744_ally);

    TEST_ASSERT(monsterIsAlive(br32744_ally),
                "control: the ally should survive its own turn");
    TEST_ASSERT(monsterIsAlive(br32744_enemy),
                "control: the enemy should still be present");
    TEST_ASSERT(!posEq(allyLoc, br32744_allyStart),
                "a weak ally beside an enemy should move rather than stand still");
    TEST_ASSERT(allyLoc.x < br32744_allyStart.x,
                "the ally should retreat away from the enemy to its east, not toward the player to its west");
    TEST_ASSERT(!posEq(allyLoc, getPlayerPos()),
                "the ally should not step onto your square");
    TEST_ASSERT(!posEq(allyLoc, br32744_enemyLoc),
                "the ally should not step onto the enemy's square");
    TEST_ASSERT(!posEq(allyLoc, br32744_sacredLoc),
                "the ally should not step onto sacred ground, even when it is the best square");
    TEST_ASSERT(pathingDistanceBetween(allyLoc, br32744_enemyLoc, 0)
                    > br32744_startDistanceFromEnemy,
                "the square the ally moved to should be strictly farther from the enemy");
    return !testFailed;
}

// ============================================================
// BR34188 — when its mechanism is powered, glowing glass pipes
// char and become inert.
//
// The pipe is installed wired into a fresh mechanism.  Before the
// mechanism is powered it must be glowing and not charred; after the
// engine's own machine activation it must be charred and no longer
// glowing.
// ============================================================

static pos br34188_pipeLoc;
static boolean br34188_glowingBefore;
static boolean br34188_charredBefore;

static void br34188_setup(void) {
    clearArena();
    br34188_pipeLoc = posOffset(getPlayerPos(), 2, 0);
    placeGlowingGlassPipeWiredToMechanism(br34188_pipeLoc);
    br34188_glowingBefore = cellIsGlowingGlassPipe(br34188_pipeLoc);
    br34188_charredBefore = cellIsCharredGlassPipe(br34188_pipeLoc);
    powerMechanismIncluding(br34188_pipeLoc);
}

static boolean br34188_powered_pipe_chars_and_goes_inert(void) {
    TEST_ASSERT(br34188_glowingBefore,
                "control: the glass pipes should be glowing before their mechanism is powered");
    TEST_ASSERT(!br34188_charredBefore,
                "control: the glass pipes should not be charred before their mechanism is powered");
    TEST_ASSERT(cellIsCharredGlassPipe(br34188_pipeLoc),
                "powering the mechanism should char the glowing glass pipes");
    TEST_ASSERT(!cellIsGlowingGlassPipe(br34188_pipeLoc),
                "the charred glass pipes should no longer glow");
    return !testFailed;
}

// ============================================================
// BR35576 — a menu button that the mouse is held down on is
// painted darker, with a dimmed label, and that pressed look
// replaces the hover tint the same button would otherwise wear.
//
// The framework paints one button through the engine's own menu
// painter three times: with the pointer away, resting on it, and
// held down on it.  Holding the mouse down leaves the button both
// focused and depressed, exactly the state a real menu reaches,
// so the pressed colours are the engine's decision.  The middle
// of the background and a label character are measured in each
// state: hover must differ from normal (the control that the
// pointer state reaches the painter), and pressed must be
// strictly darker than both normal and hover.
// ============================================================

static boolean br35576_pressed_button_replaces_hover(void) {
    const int normalBackground = menuButtonBackgroundBrightness(MENU_BUTTON_POINTER_AWAY);
    const int hoverBackground = menuButtonBackgroundBrightness(MENU_BUTTON_POINTER_OVER);
    const int pressedBackground = menuButtonBackgroundBrightness(MENU_BUTTON_POINTER_HELD);
    const int normalLabel = menuButtonLabelBrightness(MENU_BUTTON_POINTER_AWAY);
    const int hoverLabel = menuButtonLabelBrightness(MENU_BUTTON_POINTER_OVER);
    const int pressedLabel = menuButtonLabelBrightness(MENU_BUTTON_POINTER_HELD);

    TEST_ASSERT(hoverBackground > normalBackground,
                "control: a menu button under the pointer should be painted brighter than an ordinary one");
    TEST_ASSERT(pressedBackground < normalBackground,
                "holding the mouse down on a menu button should darken its background");
    TEST_ASSERT(pressedBackground < hoverBackground,
                "the pressed background should take the place of the hover tint when the button is held down");
    TEST_ASSERT(pressedLabel < normalLabel,
                "holding the mouse down on a menu button should dim its label");
    TEST_ASSERT(pressedLabel < hoverLabel,
                "the pressed label should be dimmer than the hover label on the same button");
    return !testFailed;
}

// ============================================================
// BR11468 — every discharge of a wand is counted, and a wand
// whose charges are not known reports that count in its
// description as "used once", "used twice", or "used N times".
//
// An unidentified wand of teleportation is fired three times at
// open floor, each time through the ordinary apply command with
// a queued target, so each shot takes the engine's discharge path.
// The description is read after each shot: it must name the use
// count that shot produced, and the wand must still be unknown so
// the count is the only place the history can come from.  Finally
// the maximum charges are made known: the description must then
// show the remaining charge count and no use count at all.
// ============================================================

#define BR11468_TEXT_SIZE 8192

static item *br11468_wand;
static int br11468_startCharges;
static int br11468_chargesAfterOnce;
static int br11468_chargesAfterTwice;
static int br11468_chargesAfterThrice;
static char br11468_before[BR11468_TEXT_SIZE];
static char br11468_afterOnce[BR11468_TEXT_SIZE];
static char br11468_afterTwice[BR11468_TEXT_SIZE];
static char br11468_afterThrice[BR11468_TEXT_SIZE];
static char br11468_afterChargesKnown[BR11468_TEXT_SIZE];

static void br11468_snapshot(char *dest, size_t size) {
    snprintf(dest, size, "%s", itemExamineText(br11468_wand));
}

// Fire the wand at open floor two tiles east.  The two clicks first move the
// targeting cursor onto the tile and then confirm it, as a player's click
// would.
static void br11468_discharge(void) {
    const pos target = posOffset(getPlayerPos(), 2, 0);
    queueMapClick(target);
    queueMapClick(target);
    applyPackItem(WAND, WAND_TELEPORT);
}

static void br11468_setup(void) {
    setPlayerPos(posAt(levelWidth() / 2, levelHeight() / 2));
    setArenaRadius(4, FLOOR);
    br11468_wand = givePlayerUnidentifiedItem(WAND, WAND_TELEPORT, 0);
    TEST_ASSERT(br11468_wand != NULL, "setup: the wand should be created");
    if (br11468_wand == NULL) {
        return;
    }
    br11468_startCharges = itemCharges(br11468_wand);
    TEST_ASSERT(br11468_startCharges >= 3,
                "setup: the wand should start with at least three charges");
    TEST_ASSERT(!itemIsIdentified(br11468_wand),
                "precondition: the wand's kind should still be unknown");
    br11468_snapshot(br11468_before, sizeof(br11468_before));

    br11468_discharge();
    br11468_chargesAfterOnce = itemCharges(br11468_wand);
    br11468_snapshot(br11468_afterOnce, sizeof(br11468_afterOnce));

    br11468_discharge();
    br11468_chargesAfterTwice = itemCharges(br11468_wand);
    br11468_snapshot(br11468_afterTwice, sizeof(br11468_afterTwice));

    br11468_discharge();
    br11468_chargesAfterThrice = itemCharges(br11468_wand);
    br11468_snapshot(br11468_afterThrice, sizeof(br11468_afterThrice));

    markItemMaxChargesKnown(br11468_wand);
    br11468_snapshot(br11468_afterChargesKnown, sizeof(br11468_afterChargesKnown));
}

static boolean br11468_uses_recorded_until_charges_known(void) {
    char expectedCharges[32];

    TEST_ASSERT(br11468_wand != NULL, "setup: the wand should have been created");
    TEST_ASSERT(br11468_startCharges >= 3,
                "setup: the wand should have had at least three charges");

    TEST_ASSERT(strstr(br11468_before, "(used") == NULL,
                "control: a wand that has not been discharged should report no use count");
    TEST_ASSERT(strstr(br11468_afterOnce, "(used once)") != NULL,
                "the first discharge should be reported as \"used once\"");
    TEST_ASSERT(strstr(br11468_afterTwice, "(used twice)") != NULL,
                "the second discharge should be reported as \"used twice\"");
    TEST_ASSERT(strstr(br11468_afterThrice, "(used 3 times)") != NULL,
                "the third discharge should be reported as \"used 3 times\"");

    TEST_ASSERT_EQ(br11468_chargesAfterOnce, br11468_startCharges - 1,
                   "one discharge should use up one charge");
    TEST_ASSERT_EQ(br11468_chargesAfterTwice, br11468_startCharges - 2,
                   "a second discharge should use up a second charge");
    TEST_ASSERT_EQ(br11468_chargesAfterThrice, br11468_startCharges - 3,
                   "a third discharge should use up a third charge");

    snprintf(expectedCharges, sizeof(expectedCharges), "[%d]", br11468_chargesAfterThrice);
    TEST_ASSERT(strstr(br11468_afterChargesKnown, "(used") == NULL,
                "once the charges are known the description should stop reporting the use count");
    TEST_ASSERT(strstr(br11468_afterChargesKnown, expectedCharges) != NULL,
                "once the charges are known the description should show the remaining charge count");
    TEST_ASSERT(!itemIsIdentified(br11468_wand),
                "the wand's kind should stay unknown throughout, so the use count is not hidden by identification");
    return !testFailed;
}

// ============================================================
// BR12820 — A creature that is already stuck in entangling terrain
// is not stuck again, so its remaining 3-7 turns of entanglement
// are not refreshed by standing in further entangling terrain.
//
// The player first steps into a web: the fresh entanglement must
// last 3-7 turns.  Then the countdown is replaced with a
// distinctive 20 turns and the web's tile effects are applied
// again while the player is still standing in it.  Without the
// already-stuck guard the effect would roll a new 3-7 duration.
// ============================================================

static int br12820_freshTurns, br12820_remainingBefore, br12820_remainingAfter;

static void br12820_setup(void) {
    setArenaRadius(4, FLOOR);
    setTerrain(getPlayerPos(), SPIDERWEB);
}

static boolean br12820_step(void) {
    applyTileEffectsToPlayer();
    br12820_freshTurns = playerStatusTurns(STATUS_STUCK);

    applyStatusToPlayer(STATUS_STUCK, 20);
    br12820_remainingBefore = playerStatusTurns(STATUS_STUCK);
    applyTileEffectsToPlayer();
    br12820_remainingAfter = playerStatusTurns(STATUS_STUCK);
    return false;
}

static boolean br12820_already_stuck_keeps_its_countdown(void) {
    TEST_ASSERT(isPlayerStuck(),
                "standing in a web should entangle the player");
    TEST_ASSERT(br12820_freshTurns >= 3 && br12820_freshTurns <= 7,
                "a fresh entanglement should last 3 to 7 turns");
    TEST_ASSERT_EQ(br12820_remainingBefore, 20,
                   "control: the distinctive countdown should be in place");
    TEST_ASSERT_EQ(br12820_remainingAfter, 20,
                   "an already-stuck creature should keep its remaining countdown");
    return !testFailed;
}

// ============================================================
// BR27751 — glowing embers light a radius of 2 tiles, with the
// light dimming to half brightness at the edge of the glow.
//
// The level is stripped to bare floor and the miner's light reduced
// to its minimum, so the probes eight tiles away are dark without the
// embers.  The embers are placed at the source and the lighting is
// recomputed.  The source's own tile is lit, the tiles one and two
// tiles away are lit, and the tile three tiles away is dark.  The
// edge of the glow (two tiles out) is about a quarter of the source's
// brightness: the engine paints the source tile twice (once in the
// radial loop and once as the light's origin), so the 50% radial fade
// at the edge reads as a quarter of the doubled source brightness.
// ============================================================

static pos br27751_probes[4];
static int br27751_baseline[4];
static int br27751_lit[4];

static void br27751_setup(void) {
    clearMonsters();
    pos home = posAt(levelWidth() / 2, levelHeight() / 2);
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }
    setPlayerPos(home);
    setLightMultiplier(0);

    pos source = posOffset(home, 8, 0);
    for (int d = 0; d <= 3; d++) {
        br27751_probes[d] = posOffset(source, d, 0);
    }

    setTerrain(source, FLOOR);
    recomputeLighting();
    for (int d = 0; d <= 3; d++) {
        br27751_baseline[d] = lightAt(br27751_probes[d]);
    }

    setTerrain(source, EMBERS);
    recomputeLighting();
    for (int d = 0; d <= 3; d++) {
        br27751_lit[d] = lightAt(br27751_probes[d]);
    }
}

static boolean br27751_embers_glow_radius_and_edge(void) {
    for (int d = 1; d <= 3; d++) {
        TEST_ASSERT_EQ(br27751_baseline[d], 0,
                       "control: without embers nothing lights the probe tiles");
    }
    TEST_ASSERT(br27751_lit[0] > 0,
                "the embers' own tile should be lit");
    TEST_ASSERT(br27751_lit[1] > 0,
                "the tile one away should lie inside the 2-tile glow");
    TEST_ASSERT_EQ(br27751_lit[2], 0,
                   "the glow should reach no farther than two tiles");
    TEST_ASSERT_EQ(br27751_lit[3], 0,
                   "nothing three tiles from the embers should be lit");

    int edgePercent = br27751_lit[1] * 100 / br27751_lit[0];
    TEST_ASSERT(edgePercent >= 60 && edgePercent <= 85,
                "the edge of the glow should dim to about three-quarters brightness");
    return !testFailed;
}

const testCase tests_newrules[] = {
    { "BR35726_monsters_and_entrails_have_role_colors", br35726_setup, NULL, br35726_monsters_and_entrails_have_role_colors },
    { "BR11201_only_the_listed_monsters_and_entrails_flicker", br11201_setup, NULL, br11201_only_the_listed_monsters_and_entrails_flicker },
    { "BR40113_help_commands_overlay_from_top_left", br40113_setup, br40113_step, br40113_help_commands_overlay_from_top_left },
    { "BR41110_help_dismiss_restores_previous_display", br41110_setup, br41110_step, br41110_verify },
    { "BR93785_floor_survives_matching_cellular_automata_count", br93785_setup, NULL, br93785_verify },
    { "BR20494_no_recording_saved_during_playback", br20494_playback_setup, NULL, br20494_playback_verify },
    { "BR20494_save_without_prompt_writes_recording", br20494_control_setup, NULL, br20494_control_verify },
    { "BR29087_turn_counts_diverge_while_paralyzed",      br29087_setup, br29087_step, br29087_turn_counts_diverge_while_paralyzed },
    { "BR14549_change_mode_dialog_describes_wizard_mode", br14549_setup, NULL,         br14549_change_mode_dialog_describes_wizard_mode },
    { "BR12483_translucent_buttons_show_screen_through",  br12483_setup, NULL,         br12483_translucent_buttons_show_screen_through },
    { "BR14317_painting_button_leaves_game_rng_alone",    NULL,          NULL,         br14317_painting_button_leaves_game_rng_alone },

    { "BR36733_guardian_takes_nearest_reachable_square", br36733_open_setup,     NULL, br36733_open_verify },
    { "BR36733_guardian_avoids_occupied_square",         br36733_occupied_setup, NULL, br36733_occupied_verify },
    { "BR36733_guardian_avoids_staircase",               br36733_stairs_setup,   NULL, br36733_stairs_verify },
    { "BR36733_guardian_avoids_avoided_terrain",         br36733_terrain_setup,  NULL, br36733_terrain_verify },

    { "BR19494_glow_ends_at_two_tiles", br19494_setup, NULL, br19494_glow_ends_at_two_tiles },

    { "BR27383_crystal_wall_glow_radius_and_edge", br27383_setup, NULL, br27383_crystal_wall_glow_radius_and_edge },

    { "BR31796_sacred_glyph_glow_radius_and_edge", br31796_setup, NULL, br31796_sacred_glyph_glow_radius_and_edge },

    { "BR33689_vulnerability_rune_first_trigger", br33689_setup, NULL, br33689_first_trigger_announces_and_identifies },

    { "BR33558_vulnerability_doubles_thrown_damage",
      br33558_vulnerability_setup, NULL, br33558_vulnerability_doubles_thrown_damage },
    { "BR33558_other_armor_rune_leaves_thrown_damage_alone",
      br33558_other_rune_setup, NULL, br33558_other_armor_rune_leaves_thrown_damage_alone },

    { "BR34933_torch_and_candle_light_is_flickering_orange",
      br34933_light_setup, NULL, br34933_torch_and_candle_light_is_flickering_orange },
    { "BR34933_wired_torch_flame_is_flickering_orange",
      br34933_drawn_setup, NULL, br34933_wired_torch_flame_is_flickering_orange },

    { "BR33736_haunted_torch_flame_is_flickering_purple",
      br33736_drawn_setup, NULL, br33736_haunted_torch_flame_is_flickering_purple },
    { "BR33736_haunted_torch_light_reaches_four_to_six_tiles",
      br33736_light_setup, NULL, br33736_haunted_torch_light_reaches_four_to_six_tiles },

    { "BR35743_wisp_is_flickering_blue",
      br35743_drawn_setup, NULL, br35743_wisp_is_flickering_blue },
    { "BR35743_wisp_light_reaches_four_to_eight_tiles",
      br35743_light_setup, NULL, br35743_wisp_light_reaches_four_to_eight_tiles },

    { "BR39783_ifrit_light_is_flickering_blue_three_to_six_tiles",
      br39783_setup, NULL, br39783_ifrit_light_is_flickering_blue_three_to_six_tiles },

    { "BR29777_spectral_image_light_is_pink_red_steady_3_5_tiles",
      br29777_conjure_setup, br29777_conjure_step, br29777_conjured_image_light_is_pink_red_steady_3_5_tiles },
    { "BR29777_spectral_image_light_reaches_three_tiles",
      br29777_illumination_setup, NULL, br29777_spectral_image_light_reaches_three_tiles },

    { "BR29041_poison_staff_duration_scales_with_enchantment",
      br29041_setup, NULL, br29041_poison_staff_duration_scales_with_enchantment },

    { "BR23709_arrows_page_item_details",
      br23709_setup, NULL, br23709_arrows_page_item_details },

    { "BR34530_identified_one_has_four_space_line",
      br34530_identified_one_setup, br34530_aim_step, br34530_identified_one_has_four_space_line },
    { "BR34530_identified_three_has_eight_space_line",
      br34530_identified_three_setup, br34530_aim_step, br34530_identified_three_has_eight_space_line },
    { "BR34530_unidentified_has_unlimited_line",
      br34530_unidentified_setup, br34530_aim_step, br34530_unidentified_has_unlimited_line },
    { "BR34530_known_max_charges_has_capped_line",
      br34530_max_charges_known_setup, br34530_aim_step, br34530_known_max_charges_has_capped_line },

    { "BR38509_current_streaks_show_only_recent_value",
      br38509_setup, NULL, br38509_current_streaks_show_only_recent },

    { "BR40200_paralyzed_defender_hit_message",
      br40200_setup, NULL, br40200_message_explains_paralyzed_defender },

    { "BR18636_sleeping_defender_hit_message",
      br18636_setup, NULL, br18636_message_explains_sleeping_defender },

    { "BR17360_list_shuffle_is_fair",
      br17360_setup, NULL, br17360_shuffle_is_fair },

    { "BR33997_clumped_rolls_cluster_toward_the_middle",
      br33997_clump_three_setup, NULL, br33997_clumped_rolls_cluster_toward_the_middle },
    { "BR33997_top_of_range_survives_a_remainder",
      br33997_clump_four_setup, NULL, br33997_top_of_range_survives_a_remainder },

    { "BR30955_burning_light_and_figure_are_red_orange",
      br30955_setup, NULL, br30955_burning_colors_are_red_orange_and_restore },

    { "BR24247_flash_restores_lighting",
      br24247_setup, NULL, br24247_flash_restores_lighting },

    { "BR40052_level_seeds_stay_in_small_range",
      br40052_range_setup, NULL, br40052_level_seeds_stay_in_small_range },
    { "BR40052_zero_level_seed_becomes_depth_plus_one",
      br40052_zero_setup, NULL, br40052_zero_level_seed_becomes_depth_plus_one },

    { "BR37990_upper_seed_bits_change_dungeon_layout",
      br37990_setup, NULL, br37990_upper_seed_bits_change_dungeon_layout },

    { "BR24594_down_stairs_are_interior_and_varied",
      br24594_setup, NULL, br24594_down_stairs_are_interior_and_varied },
    { "BR24594_down_stairs_keep_a_third_width_from_up",
      br24594_setup, NULL, br24594_down_stairs_keep_a_third_width_from_up },

    { "BR35125_title_screen_redraws_on_every_active_pass",
      br35125_active_setup, NULL, br35125_redraws_on_every_active_pass },
    { "BR35125_title_screen_leaves_display_alone_while_inactive",
      br35125_inactive_setup, NULL, br35125_leaves_display_alone_while_inactive },

    { "BR36302_multiplicity_examine_previews_images",
      br36302_setup, NULL, br36302_examine_previews_stronger_images },

    { "BR38512_archive_overwrites_oldest_message_when_full",
      br38512_setup, NULL, br38512_archive_rolls_over_at_capacity },

    { "BR37725_ambient_light_darkens_to_blue_grey_with_depth",
      NULL, NULL, br37725_ambient_light_darkens_to_blue_grey_with_depth },

    { "BR18004_phylactery_cycle_ends_only_when_destroyed",
      br18004_setup, NULL, br18004_phylactery_cycle_ends_only_when_destroyed },
    { "BR35620_stranded_gas_splits_evenly_and_loses_remainder",
      br35620_setup, br35620_step, br35620_stranded_gas_splits_evenly_and_loses_remainder },

    { "BR32744_fleeing_ally_runs_from_enemy_not_player",
      br32744_setup, br32744_step, br32744_fleeing_ally_runs_from_the_enemy },

    { "BR34188_powered_pipe_chars_and_goes_inert",
      br34188_setup, NULL, br34188_powered_pipe_chars_and_goes_inert },

    { "BR35576_pressed_button_replaces_hover",
      NULL, NULL, br35576_pressed_button_replaces_hover },

    { "BR11468_wand_reports_uses_until_charges_known",
      br11468_setup, NULL, br11468_uses_recorded_until_charges_known },

    { "BR12820_already_stuck_keeps_its_countdown",
      br12820_setup, br12820_step, br12820_already_stuck_keeps_its_countdown },

    { "BR27751_embers_glow_radius_two_tiles_and_half_edge",
      br27751_setup, NULL, br27751_embers_glow_radius_and_edge },

    { NULL, NULL, NULL, NULL }
};
