// Tests derived from rules/NewRules.txt — pending new rules that have not
// yet been merged into a numbered rules file.
//
// These cover the deterministic, headless-observable behaviors those rules
// describe: how the game's absolute turn count and your own turn count
// diverge while you are paralyzed, the wording the Change Mode dialog uses
// for Wizard mode, the opacity menu buttons are painted with, the
// decorative random-number stream button painting uses, the reach of a pile
// of unicorn poop's rainbow glow, the radius and edge brightness of a crystal
// wall's glow, and where a guardian charm places its guardian.
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

    { "BR27383_crystal_wall_glow_radius_and_edge", br27383_setup, NULL, br27383_crystal_wall_glow_radius_and_edge },

    { "BR33689_vulnerability_rune_first_trigger", br33689_setup, NULL, br33689_first_trigger_announces_and_identifies },

    { "BR34933_torch_and_candle_light_is_flickering_orange",
      br34933_light_setup, NULL, br34933_torch_and_candle_light_is_flickering_orange },
    { "BR34933_wired_torch_flame_is_flickering_orange",
      br34933_drawn_setup, NULL, br34933_wired_torch_flame_is_flickering_orange },

    { "BR29041_poison_staff_duration_scales_with_enchantment",
      br29041_setup, NULL, br29041_poison_staff_duration_scales_with_enchantment },

    { NULL, NULL, NULL, NULL }
};
